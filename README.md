# epaper_imageclient_spectra6
A simple ESP32 program that downloads a c-array image from a http url and display it on a 7.3 inch Spectra 6 epaper display.

-An ESP32 with extenden PSRAM is required. I used an ESP32-WROVER myself.
-You should solder off the power led from the ESP32 to save a lot of battery life.

This program does the following:
When booting up it connects to wifi.
Then downloads a .txt file and a .h file from a specified url.

The .txt file should contain 1 line that sets the hours to wake up.
For example if the file contains:
6,8,14,20
Then the ESP will set wakeup times for 6AM 8AM 2PM and 8PM.
You can set a maximum of 10 different hours.

The .h file is the actual image in a c-array format.
Use the 6color73i.py script to convert a jpg image to a c-array.
You can use the script as follows: python3 6color73i.py input.jpg output.h

After the prgram has finished displaying an image it will go to deep sleep and wake up at the first upcoming hour from the .txt file.
If it runs into any errors the program will go to deep sleep for 2 hours and then wake up and try again.

Before flashing the program to the ESP32 you should edit the following lines in the .ino file

const char* ssid = "YOURSSID";
const char* password = "YOURWIFIPASSWORD";
const char* image_url = "http://YOURURL.COM/output.h"; // URL to image.
const char* wakeHoursUrl = "http://YOURURL.COM/timer.txt"; // URL to wakehours txt file.

