from PIL import Image
import numpy as np
import sys

def main():
    # Check if there are exactly 3 arguments (script name, input file, and output file)
    if len(sys.argv) != 3:
        print("Usage: python 6color73i.py input_image.jpg output_file.c")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    # Open and convert the image to RGB
    image = Image.open(input_file).convert('RGB')
    pixels = np.array(image, dtype=np.float32)
    height, width, channels = pixels.shape

    # Define the color palette
    palette = [
        (0, 0, 0),       # Black
        (255, 255, 255), # White
        (255, 0, 0),     # Red
        (255, 255, 0),   # Yellow
        (0, 255, 0),     # Green
        (0, 0, 255)      # Blue
    ]

    def find_closest_color(r, g, b):
        min_distance = float('inf')
        closest_color = palette[0]
        for color in palette:
            distance = (color[0] - r) **2 + (color[1] - g) **2 + (color[2] - b) **2
            if distance < min_distance:
                min_distance = distance
                closest_color = color
        return closest_color

    def color_to_epd_color(r, g, b):
        if r > 200 and g > 200 and b > 200:
            return 0xFF  # White
        if r < 50 and g < 50 and b < 50:
            return 0x00  # Black
        if r > 200 and g < 50 and b < 50:
            return 0xE0  # Red
        if r < 50 and g > 200 and b < 50:
            return 0x1C  # Green
        if r < 50 and g < 50 and b > 200:
            return 0x03  # Blue
        if r > 200 and g > 200 and b < 50:
            return 0xFC  # Yellow
        return 0xFF  # Default to White

    # Perform Floyd-Steinberg dithering
    for y in range(height):
        for x in range(width):
            old_pixel = pixels[y, x].copy()
            new_pixel = find_closest_color(*old_pixel)
            pixels[y, x] = new_pixel
            quant_error = old_pixel - new_pixel

            if x + 1 < width:
                pixels[y, x + 1] += quant_error * 7 / 16
            if x - 1 >= 0 and y + 1 < height:
                pixels[y + 1, x - 1] += quant_error * 3 / 16
            if y + 1 < height:
                pixels[y + 1, x] += quant_error * 5 / 16
            if x + 1 < width and y + 1 < height:
                pixels[y + 1, x + 1] += quant_error * 1 / 16

    # Clip the pixel values to be in [0, 255]
    pixels = np.clip(pixels, 0, 255)

    # Generate the data string
    image_size = width * height
    data_string = 'const unsigned char imageData[{}] = {{\n'.format(image_size)
    counter = 0
    for y in range(height):
        for x in range(width):
            r, g, b = pixels[y, x]
            epd_color = color_to_epd_color(r, g, b)
            data_string += '0x{:02X},'.format(int(epd_color))
            counter += 1
            if counter % 16 == 0:
                data_string += '\n'

    data_string += '\n};'

    # Save to specified output file
    with open(output_file, 'w') as f:
        f.write(data_string)

    print("Data array saved to", output_file)

if __name__ == '__main__':
    main()
