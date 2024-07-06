// ###########################################################################################################################################
// #
// # Code for the printables.com AWSW PixelClock 32x8 LED matrix project:
// # https://www.printables.com/de/model/896884-pixelclock-32x8
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/AWSW-PixelClock-32x8?tab=GPL-3.0-1-ov-file
// # Important: NonCommercial — You may not use the material for commercial purposes !
// #
// ###########################################################################################################################################
// #
// # Compatible with PixelClock version: V1.1.2
// #
// ###########################################################################################################################################


// ###########################################################################################################################################
// # Hardware settings:
// ###########################################################################################################################################
#define LEDPIN 32      // ESP32 pin connected to the NeoPixels matrix
#define NUMPIXELS 256  // How many NeoPixels are attached to the Arduino
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, LEDPIN, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);


// ###########################################################################################################################################
// # LED defaults:
// ###########################################################################################################################################
int redVal_back_default = 0;              // Default background color RED
int greenVal_back_default = 255;          // Default background color GREEN
int blueVal_back_default = 255;           // Default background color BLUE
int redVal_time_default = 255;            // Default time color RED
int greenVal_time_default = 128;          // Default time color GREEN
int blueVal_time_default = 0;             // Default time color BLUE
int intensity_day_default = 20;           // LED intensity (0..255) in day mode   - Important note: Check power consumption and used power supply capabilities!
int intensity_night_default = 5;          // LED intensity (0..255) in day mode   - Important note: Check power consumption and used power supply capabilities!
int usenightmode_default = 1;             // Use the night mode to reduce LED intensity during set times
String day_time_start_default = "06:15";  // Define day mode start --> time before is then night mode if used
String day_time_stop_default = "22:14";   // Define day mode end --> time after is then night mode if used


// ###########################################################################################################################################
// # Various default settings:
// ###########################################################################################################################################
int useshowip_default = 1;        // Show the current ip at boot
int RandomColor_default = 0;      // Change text color every minute in random color
int useStartupText_default = 1;   // Show the "PixelClock" text at boot
int maxWiFiconnctiontries = 100;  // Maximum connection tries to logon to the set WiFi. After the amount of tries is reached the WiFi settings will be deleted!
int iHourOffset_default = 1;      // Offset of hours in Offline Mode
int useWiFiReCon_default = 1;     // Use the WiFi reconnect function during runtime
int usetestNumbers_default = 0;   // Test the pixel numbers on startup


// ###########################################################################################################################################
// # Variables declaration in Online Mode:
// ###########################################################################################################################################
#define DEFAULT_AP_NAME "PixelClock"  // WiFi access point name of the ESP32


// ###########################################################################################################################################
// # Variables declaration in Offline Mode:
// ###########################################################################################################################################
// NOTE: When you try this setting "UseOfflineModeWithPassword" you may need to remove the known WiFi entries to see the change after some
// seconds and meanwhile disabling WiFi on your device. Some operating systems seem to cache the settings, so you might not see the change first
int UseOfflineModeWithPassword = 1;                       // Choose to open the interal WiFi access point with (1) or without (0) password protection
const char* Offline_SSID = "PixelClock in Offline Mode";  // SSID of the internal WiFi access point used in Offline Mode
const char* Offline_PW = "Matrix-32x8";                   // Access point with password protection. Minimum 8 characters needed!
int showOMhints_default = 0;                              // Show or hide the Operation Mode hints on the page


// ###########################################################################################################################################
// # Special NTP time server and time zone settings:
// ###########################################################################################################################################
// Time server:
String NTPserver_default = "pool.ntp.org";  // Here you can set your own specific time server address if needed. Many others can be selected in the configuraration portal
// Choose the closest one to you here: https://gist.github.com/mutin-sa/eea1c396b1e610a2da1e5550d94b0453
// PS: The closest NTP time server to you might be your local router which can be selected in the configuraration portal too =)
//
// Time zone:
String Timezone_default = "CET-1CEST,M3.5.0,M10.5.0/3";  // Here you can set your own specific time zone if needed. Many others can be selected in the configuraration portal
// Please send me a message with your time zone settings to add them to this list in future releases. Thanks in advance! =)
// You can check a list of timezone string variables here:  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv


// ###########################################################################################################################################
// # Test functions:
// ###########################################################################################################################################
int debugtexts = 0;  // Show more debug information in serial monitor


// ###########################################################################################################################################
// #
// # !!! DANGER ZONE: DO NOT PLAY WITH THIS VALUE IF YOU ARE NOT AWARE WHAT YOU ARE DOING - SERIOSLY !!!
// #
// # !!! PLEASE READ THIS CAREFULLY TO AVOID DAMAGE TO THE ESP32 AND THE OTHER COMPONENTS !!! YOU HAVE BEEN WARNED !!!
// #
// ###########################################################################################################################################
// # LED intensity setting:
// ###########################################################################################################################################
#define LEDintensityLIMIT 50  // of 255
                              /* Do NOT change this if you don't know how much power drain this may cause to avoid damage !!! 
                              Limit the intensity level to be able to select in the configuration to avoid to much power drain and to avoid hardware failures over time. 
                              In case you want to use the device over longer times with a higher intensity or even higher, you will need to consider a much more powerful 
                              power supply, a better cable to connect the device and you have to power the matrix directly to 5V of the power supply, not over the ESP32! 
                              Otherwise the components fail directly or over time. Therefore this setting is limited and should not be raised. 
                              In case you really need a higher value, you need to make sure to power the matrix directly so that the power is not served over the ESP32 
                              to the matrix to avoid its damage over time or directly. 
                              Specifications for these matrix say that "60mA (per pixel at full brightness" (white color) is used. Multiple this by 256 and it takes 15.36A 
                              Calculation: 16x16 = 256 LEDs >>> 256x60mA = 15.36A (without the rest of the electronics = ESP32) >>> ~16A * 5V = ~80W power drain... 
                              Think of the heat emited by the LEDs then too... 
                              Conclusion: 
                              - You should not go higher here. 
                              - Measuremets showed that with the set intensity limit of 50 the 5V/3A power supply is enough to use with THIS software. That does not mean that this 
                                device will not fail, if you try to use it with other software "to shortly test other LED software" to see the result. Keep this in mind too. 
                              - Please think about if you really need such a high intensity value. The PixelClock's used from me run at a maximum intensity of 22 which is 
                                really bright enoigh in my eyes and all levels above 48 i could not see really an advantage anymore that the display is better able to view... 
                              - Also useing white background color does not seem to be a good idea because to white set LEDs take the most power... 
                              - Make sure to use propper components and settings! 
                              - As allways: You are using this device at your own risk! */


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################