ESP32 pinout:<br/>
<br/>
VCC = 3.3v<br/>
GND = GND<br/>
DIN/SDI = GPIO-23<br/>
SCLK/SCK = GPIO-18<br/>
CS = GPIO-27<br/>
DC = GPIO-14<br/>
RST = GPIO-12<br/>
BUSY = GPIO-13<br/>


# epaper_imageclient_spectra6
A simple ESP32 program that downloads a c-array image from a http url and display it on a 7.3 inch Spectra 6 epaper display.

-An ESP32 with extenden PSRAM is required. I used an ESP32-WROVER myself.<br/>
-You should solder off the power led from the ESP32 to save a lot of battery life.

This program does the following:<br/>
When booting up it connects to wifi.<br/>
Then downloads a .txt file and a .h file from a specified url.<br/>

The .txt file should contain 1 line that sets the hours to wake up.<br/>
For example if the file contains:<br/>
6,8,14,20<br/>
Then the ESP will set wakeup times for 6AM 8AM 2PM and 8PM.<br/>
You can set a maximum of 10 different hours.<br/>

The .h file is the actual image in a c-array format.<br/>
Use the 6color73i.py script to convert a jpg image to a c-array.<br/>
You can use the script as follows: python3 6color73i.py input.jpg output.h<br/>

After the prgram has finished displaying an image it will go to deep sleep and wake up at the first upcoming hour from the .txt file.<br/>
If it runs into any errors the program will go to deep sleep for 2 hours and then wake up and try again.<br/>

Before flashing the program to the ESP32 you should edit the following lines in the epaper_imageclient_spectra6.ino file<br/>

const char* ssid = "YOURSSID";<br/>
const char* password = "YOURWIFIPASSWORD";<br/>
const char* image_url = "http://YOURURL.COM/output.h"; // URL to image.<br/>
const char* wakeHoursUrl = "http://YOURURL.COM/timer.txt"; // URL to wakehours txt file.<br/>

Below there are the timezone settings. You should set those to the right timezone.
