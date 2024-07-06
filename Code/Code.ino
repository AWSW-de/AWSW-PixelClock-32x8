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


// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project:
// # - Adafruit NeoPixel      // by Adafruit:                     https://github.com/adafruit/Adafruit_NeoPixel
// # - Adafruit GFX           // by Adafruit:                     https://github.com/adafruit/Adafruit-GFX-Library
// # - Adafruit NeoMatrix     // by Adafruit:                     https://github.com/adafruit/Adafruit_NeoMatrix
// # - Adafruit BusIO         // by Adafruit:                     https://github.com/adafruit/Adafruit_BusIO
// # - ESP32Time              // by fbiego:                       https://github.com/fbiego/ESP32Time
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI/archive/refs/tags/2.2.3.zip
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// #
// ###########################################################################################################################################
#include <WiFi.h>                // Used to connect the ESP32 to your WiFi
#include <WebServer.h>           // ESP32 OTA update function
#include <Update.h>              // ESP32 OTA update function
#include <Adafruit_GFX.h>        // Used to drive the NeoPixel LEDs
#include <Adafruit_NeoMatrix.h>  // Used to drive the NeoPixel LEDs
#include <Adafruit_NeoPixel.h>   // Used to drive the NeoPixel LEDs
#include "time.h"                // Used for NTP time requests
#include <AsyncTCP.h>            // Used for the internal web server
#include <ESPAsyncWebServer.h>   // Used for the internal web server
#include <DNSServer.h>           // Used for the internal web server
#include <ESPUI.h>               // Used for the internal web server
#include "esp_log.h"             // Disable WiFi debug warnings
#include <ESP32Time.h>           // Used for the Offline Mode ESP32 time function
#include <Preferences.h>         // Used to save the configuration to the ESP32 flash
#include "settings.h"            // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* CLOCK_VERSION = "V1.1.2";


// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);       // Web server for config
WebServer otaserver(8080);       // Web OTA ESP32 update server
AsyncWebServer ledserver(2023);  // Web server for HTML commands
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;


// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;  // ESP32 flash storage
ESP32Time rtc;            // Setup Offline ESP32 time function
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
bool WiFIsetup = false;
bool dots = true;
int iHour, iMinute, iSecond, iDay, iMonth, iYear, intensity, intensity_day, intensity_night, set_web_intensity, intensity_web;
int redVal_back, greenVal_back, blueVal_back, redVal_time, greenVal_time, blueVal_time, DayNightSectionID, LEDsettingsSectionID, usetestNumbers;
int usenightmode, statusNightMode, statusNightModeWarnID, useshowip, useStartupText, statusLabelID, statusNightModeID, intensity_web_HintID, RandomColor, switchRandomColorID;
int sliderBrightnessDayID, switchNightModeID, sliderBrightnessNightID, call_day_time_startID, call_day_time_stopID, mySetTimeZone, mySetTimeZoneID, mySetTimeServer, mySetTimeServerID;
int UseOnlineMode, OfflineCurrentHourOffset, iHourOffset, statusTimeFromDevice, statusTimeSetOffline, OfflineModeHint1, OfflineModeHint2, OfflineModeHint3, showOMhints;
uint16_t text_colour_background, text_colour_time, timeId;
String iStartTime, myTimeZone, myTimeServer, Timezone, NTPserver, day_time_start, day_time_stop, statusNightModeIDtxt;
// WiFi REconnect function:
unsigned long WiFi_currentMillis = 0;
unsigned long WiFi_previousMillis = 0;
unsigned long WiFi_interval = 60000;
int WiFi_retry_counter = 0;
int useWiFiReCon, WiFiReConHint;
int WiFi_FlashLEDs = 10;
int WiFi_Restart = 30;



// ###########################################################################################################################################
// # Setup function:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("######################################################################");
  Serial.println("# PixelClock startup of version: " + String(CLOCK_VERSION));
  Serial.println("######################################################################");
  preferences.begin("pixelclock", false);  // Init ESP32 flash
  getFlashValues();                        // Read settings from flash
  intensity = intensity_day;               // Set the intenity to day mode for startup
  strip.begin();                           // Init the LEDs
  strip.show();                            // Init the LEDs --> Set them to OFF
  strip.setBrightness(intensity);          // Set LED brightness
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(intensity);
  if (UseOnlineMode == 1) WIFI_SETUP();          // ONLINE MODE WiFi login and startup of web services
  if (UseOnlineMode == 0) {                      // OFFLINE MODE
    iHour = 9;                                   // Default hour in Offline Mode
    iMinute = 41;                                // Default minute in Offline Mode
    OfflinePotalSetup();                         // Offline mode setup access point
    rtc.setTime(0, iMinute, iHour, 1, 1, 2024);  // Set time: (ss, mm, hh, DD, MM, YYYY) --> 17th Jan 2021 09:41:00
    updatenow = true;                            // Update the display 1x after startup
    update_display();                            // Update LED display
    Serial.println("######################################################################");
    Serial.println("# PixelClock startup in OFFLINE MODE finished...");
    Serial.println("######################################################################");
  }
  updatedevice = true;
}


// ###########################################################################################################################################
// # Loop function:
// ###########################################################################################################################################
void loop() {
  if (UseOnlineMode == 1) {  // Online Mode actions:
    if (WiFIsetup == true) {
      WiFi_currentMillis = millis();
      if ((useWiFiReCon == true) && (WiFIsetup == true) && (WiFi.status() != WL_CONNECTED) && (WiFi_currentMillis - WiFi_previousMillis >= WiFi_interval)) {  // Device offline during runtime? > Reconnect now!
        WiFi_Lost();
      }
      if ((useWiFiReCon == true) && (WiFIsetup == true) && (WiFi.status() == WL_CONNECTED) && (WiFi_retry_counter >= 1)) {  // Device was offline during runtime? > Reconnect was successfull!
        Serial.println("Reconnecting to WiFi = SUCCESS");
        WiFi_retry_counter = 0;
        updatedevice = true;
        updatenow = true;
      }
      printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
      if (updatedevice == true) {                     // Allow display updates (normal usage)
        if (changedvalues == true) setFlashValues();  // Write settings to flash
        update_display();                             // Update display (1x per minute regulary)
      }
      if (updatemode == true) otaserver.handleClient();  // ESP32 OTA update
    }
  } else {                                               // Offline Mode actions:
    if (debugtexts == 1) Serial.println(rtc.getTime());  // Time string as e.g. 15:24:38
    struct tm timeinfo = rtc.getTimeStruct();
    iHour = timeinfo.tm_hour;
    iMinute = timeinfo.tm_min;
    iSecond = timeinfo.tm_sec;
    if (updatedevice == true) {
      if (changedvalues == true) setFlashValues();  // Write settings to flash
      update_display();
    }
    delay(1000);
  }
  dnsServer.processNextRequest();  // Update the web server
}


// ###########################################################################################################################################
// # Set LED function:
// ###########################################################################################################################################
void setLED(int ledNrFrom, int ledNrTo, uint32_t color) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, color);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS)) {
        strip.setPixelColor(i, color);
      }
    }
  }
}


// ###########################################################################################################################################
// # AWSW BIG DIGITS: Number output function:
// ###########################################################################################################################################
void shownumber(int num, int pos, uint32_t c) {

  int x = 0;
  if (pos == 1) x = 0;
  if (pos == 2) x = 64;
  if (pos == 3) x = 144;
  if (pos == 4) x = 208;

  if (num == 0) {
    setLED(0 + x, 17 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(30 + x, 47 + x, c);
  }

  if (num == 1) {
    setLED(32 + x, 47 + x, c);
  }

  if (num == 2) {
    setLED(0 + x, 1 + x, c);
    setLED(3 + x, 12 + x, c);
    setLED(14 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 36 + x, c);
    setLED(38 + x, 41 + x, c);
    setLED(43 + x, 47 + x, c);
  }

  if (num == 3) {
    setLED(0 + x, 1 + x, c);
    setLED(3 + x, 4 + x, c);
    setLED(6 + x, 9 + x, c);
    setLED(11 + x, 12 + x, c);
    setLED(14 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 47 + x, c);
  }

  if (num == 4) {
    setLED(0 + x, 4 + x, c);
    setLED(11 + x, 15 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(32 + x, 47 + x, c);
  }

  if (num == 5) {
    setLED(0 + x, 4 + x, c);
    setLED(6 + x, 9 + x, c);
    setLED(11 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 33 + x, c);
    setLED(35 + x, 44 + x, c);
    setLED(46 + x, 47 + x, c);
  }

  if (num == 6) {
    setLED(0 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 33 + x, c);
    setLED(35 + x, 44 + x, c);
    setLED(46 + x, 47 + x, c);
  }

  if (num == 7) {
    setLED(0 + x, 1 + x, c);
    setLED(14 + x, 17 + x, c);
    setLED(30 + x, 47 + x, c);
  }

  if (num == 8) {
    setLED(0 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 47 + x, c);
  }

  if (num == 9) {
    setLED(0 + x, 4 + x, c);
    setLED(6 + x, 9 + x, c);
    setLED(11 + x, 17 + x, c);
    setLED(19 + x, 20 + x, c);
    setLED(22 + x, 25 + x, c);
    setLED(27 + x, 28 + x, c);
    setLED(30 + x, 47 + x, c);
  }
}


// ###########################################################################################################################################
// # AWSW BIG DIGITS: Show dots output function:
// ###########################################################################################################################################
void showDots(uint32_t c) {
  if (dots == true) {
    setLED(121, 122, c);
    setLED(125, 126, c);
    setLED(129, 130, c);
    setLED(133, 134, c);
    dots = false;
  } else {
    dots = true;
  }
}


// ###########################################################################################################################################
// # Test the numbers:
// ###########################################################################################################################################
const uint32_t colorsfront[] = {
  strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(255, 255, 255), strip.Color(0, 255, 255), strip.Color(255, 128, 0)
};
uint32_t testcolor;
void testNumbers() {
  strip.clear();  // Set all pixel colors to 'off'
  for (int y = 0; y <= 5; y++) {
    testcolor = colorsfront[y];
    for (int i = 0; i < 10; i++) {
      strip.clear();  // Set all pixel colors to 'off'
      shownumber(i, 1, testcolor);
      shownumber(i, 2, testcolor);
      shownumber(i, 3, testcolor);
      shownumber(i, 4, testcolor);
      showDots(testcolor);  // Show dots or not
      strip.show();
      delay(500);
    }
  }
}


// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);
  if (UseOnlineMode == 0) ESPUI.captivePortal = true;  // Offline Mode Captive Portal


  // Section General:
  // ################
  ESPUI.separator("General:");

  // Welcome label:
  ESPUI.label("PixelClock configuration", ControlColor::None, "Welcome to the PixelClock configuration. Here you can adjust your PixelClock settings to your personal needs. Enjoy your PixelClock =)");

  // PixelClock version:
  ESPUI.label("PixelClock software version", ControlColor::None, CLOCK_VERSION);



  // Section Offline Mode:
  // #####################
  ESPUI.separator("Operation mode:");

  // Status label:
  if (UseOnlineMode == 0) {
    statusLabelID = ESPUI.label("Operation mode", ControlColor::None, "PixelClock is used in Offline Mode. Please check time value.");
  } else {
    statusLabelID = ESPUI.label("Operation mode", ControlColor::Dark, "PixelClock is used in Online Mode. All functions are available.");
  }

  // Add the invisible "Time" control to update to the current time of your device (PC, smartphone, tablet, ...)
  timeId = ESPUI.addControl(Time, "", "", None, 0, timeCallback);

  // Use online or offline mode:
  ESPUI.switcher("Use PixelClock in Online Mode (change forces restart)", &switchOffline, ControlColor::Dark, UseOnlineMode);

  if (UseOnlineMode == 0) {
    // Time read from your device:
    ESPUI.button("Get time from your device", &getTimeCallback, ControlColor::Dark, "Get time from your computer, phone, tablet");
    statusTimeFromDevice = ESPUI.label("Time received from your device", ControlColor::Dark, "-");

    // Hour offset:
    OfflineCurrentHourOffset = ESPUI.addControl(ControlType::Select, "Current hour offset", String(iHourOffset), ControlColor::Dark, Control::noParent, SetOfflineHourOffset);
    ESPUI.addControl(ControlType::Option, "- 9 hours", "-9", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 8 hours", "-8", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 7 hours", "-7", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 6 hours", "-6", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 5 hours", "-5", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 4 hours", "-4", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 3 hours", "-3", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 2 hours", "-2", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 1 hours", "-1", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 0 hours", "0", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 1 hours", "1", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 2 hours", "2", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 3 hours", "3", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 4 hours", "4", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 5 hours", "5", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 6 hours", "6", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 7 hours", "7", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 8 hours", "8", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 9 hours", "9", ControlColor::Alizarin, OfflineCurrentHourOffset);

    // Time set on PixelClock with the calculated offset:
    statusTimeSetOffline = ESPUI.label("Time set on PixelClock", ControlColor::Dark, "-");
  }

  // Operation Mode hints:
  ESPUI.switcher("Show the Operation Mode hints", &switchOMhints, ControlColor::Dark, showOMhints);
  OfflineModeHint1 = ESPUI.label("What is 'Online Mode' ?", ControlColor::Dark, "PixelClock uses your set local WiFi to update the time and can use all of the smart functions in your local network. (Normal and recommended operation mode)");
  ESPUI.setPanelStyle(OfflineModeHint1, "width: 95%;");
  OfflineModeHint2 = ESPUI.label("What is 'Offline Mode' ?", ControlColor::Dark, "PixelClock does not use your WiFi and it sets up an own, internal access point '" + String(Offline_SSID) + "' you can connect to to control all functions that do not require your network. All smart functions will be disabled and you need to set the time manually after each startup, but you can use the time piece in environments without a local WiFi.");
  ESPUI.setPanelStyle(OfflineModeHint2, "width: 95%;");
  OfflineModeHint3 = ESPUI.label("General usage hints", ControlColor::Dark, "You can switch between both modes without lost of data. In case your browser does not open the PixelClock configuration page automatically in any mode after connecting to the access point, please navigate to this URL manually: http://" + IpAddress2String(WiFi.softAPIP()));
  ESPUI.setPanelStyle(OfflineModeHint3, "width: 95%;");
  if (showOMhints == 0) {
    ESPUI.updateVisibility(OfflineModeHint1, false);
    ESPUI.updateVisibility(OfflineModeHint2, false);
    ESPUI.updateVisibility(OfflineModeHint3, false);
  }



  // Section LED settings:
  // #####################
  LEDsettingsSectionID = ESPUI.separator("LED settings:");

  // Time color selector:
  char hex_time[7] = { 0 };
  sprintf(hex_time, "#%02X%02X%02X", redVal_time, greenVal_time, blueVal_time);
  text_colour_time = ESPUI.text("Time", colCallTIME, ControlColor::Dark, hex_time);
  ESPUI.setInputType(text_colour_time, "color");

  // Background color selector:
  char hex_back[7] = { 0 };
  sprintf(hex_back, "#%02X%02X%02X", redVal_back, greenVal_back, blueVal_back);
  text_colour_background = ESPUI.text("Background", colCallBACK, ControlColor::Dark, hex_back);
  ESPUI.setInputType(text_colour_background, "color");

  // Use random color mode:
  switchRandomColorID = ESPUI.switcher("Use random text color every new minute", &switchRandomColor, ControlColor::Dark, RandomColor);
  if (RandomColor == 1) {
    ESPUI.updateVisibility(text_colour_time, false);
    ESPUI.updateVisibility(text_colour_background, false);
  }

  // Show note when intensity is currently controlled via web-url usage and these internal settings get disabled:
  intensity_web_HintID = ESPUI.label("Manual settings disabled due to web URL usage:", ControlColor::Alizarin, "Restart PixelClock or deactivate web control usage via <a href='http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?INTENSITYviaWEB=0' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?INTENSITYviaWEB=0</a>");
  ESPUI.updateVisibility(intensity_web_HintID, false);



  // Section LED night mode settings:
  // ################################
  DayNightSectionID = ESPUI.separator("Day/Night LED brightness mode settings:");

  // Use night mode function:
  switchNightModeID = ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity DAY slider selector:
  sliderBrightnessDayID = ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, LEDintensityLIMIT);

  // Intensity NIGHT slider selector:
  sliderBrightnessNightID = ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, LEDintensityLIMIT);

  // Warning if day/night time setting is wrong:
  statusNightModeWarnID = ESPUI.label("Night mode settings error", ControlColor::Alizarin, "Wrong setting! 'Day time starts at' needs to be before 'Day time ends after'. Please correct values.");
  ESPUI.updateVisibility(statusNightModeWarnID, false);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  call_day_time_startID = ESPUI.text("Day time starts at", call_day_time_start, ControlColor::Dark, String(day_time_start));
  ESPUI.setInputType(call_day_time_startID, "time");

  // Day mode stop time:
  call_day_time_stopID = ESPUI.text("Day time ends after", call_day_time_stop, ControlColor::Dark, String(day_time_stop));
  ESPUI.setInputType(call_day_time_stopID, "time");



  // Section Startup:
  // ################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Startup:");

    // Startup PixelClock text function :
    ESPUI.switcher("Show the 'Pixel Clock' text on startup", &switchStartupText, ControlColor::Dark, useStartupText);

    // Show IP-address on startup:
    ESPUI.switcher("Show IP-address on startup", &switchShowIP, ControlColor::Dark, useshowip);

    // Use the number test function on startup
    ESPUI.switcher("Test the pixel numbers on startup", &switchtestNumbers, ControlColor::Dark, usetestNumbers);
  }



  // Section WiFi:
  // #############
  if (UseOnlineMode == 1) {
    ESPUI.separator("WiFi:");

    // WiFi SSID:
    ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

    // WiFi signal strength:
    ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

    // Hostname:
    ESPUI.label("Hostname in your router", ControlColor::Dark, "<a href='http://" + String(WiFi.getHostname()) + "' target='_blank' style='color:#ffffff;'>" + String(WiFi.getHostname()) + "</a>");

    // WiFi MAC-address:
    ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());

    // WiFi ip-address:
    ESPUI.label("IP-address", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + "' target='_blank' style='color:#ffffff;'>" + IpAddress2String(WiFi.localIP()) + "</a>");

    // WiFi DNS address:
    ESPUI.label("DNS address", ControlColor::Dark, IpAddress2String(WiFi.dnsIP()));

    // WiFi Gateway address:
    ESPUI.label("Gateway address", ControlColor::Dark, IpAddress2String(WiFi.gatewayIP()));

    // Maximum retries to reach the configured WiFi:
    ESPUI.label("Max. tries on startup until the WiFi config is reset", ControlColor::Dark, String(maxWiFiconnctiontries));
  }



  // Section WiFi reconnect:
  // #######################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Active WiFi reconnect during runtime:");

    // Use WIFi reconnect function during runtime:
    ESPUI.switcher("Use active WIFi reconnect function during runtime", &switchWiFiReConnect, ControlColor::Dark, useWiFiReCon);

    // Notes about active WiFi reconnect:
    WiFiReConHint = ESPUI.label("Notes about active WiFi reconnect", ControlColor::Dark, "PixelClock will check its connection to your WiFi every " + String(WiFi_interval / 1000) + " seconds (" + String((WiFi_interval / 1000) / 60) + " minute(s)). If the WiFi cannot be reached anymore it will continue to work offline for " + String((WiFi_interval / 1000) * WiFi_FlashLEDs) + " seconds (" + String(((WiFi_interval / 1000) * WiFi_FlashLEDs) / 60) + " minutes), but will flash the WiFi LEDs in blue, yellow and red then. If after " + String((WiFi_interval / 1000) * WiFi_Restart) + " seconds (" + String(((WiFi_interval / 1000) * WiFi_Restart) / 60) + " minutes) the WiFi can still not be reconnected, PixelClock will reboot automatically to solve the connection problem. In case the WiFi then still not is able to reach for " + String(maxWiFiconnctiontries) + " tries, the configuration will be set to default. It is expected then, that the WiFi will not be available anymore, e.g. due to a change of your router, etc...");
    ESPUI.setPanelStyle(WiFiReConHint, "width: 60%;");
  }



  // Section Time settings:
  // ######################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Time settings:");

    // PixelClock startup time:
    ESPUI.label("PixelClock startup time", ControlColor::Dark, iStartTime);

    // NTP time server: (All of these were successfully tested on 06.01.2024)
    mySetTimeServer = ESPUI.addControl(ControlType::Select, "Choose your time server (change forces restart)", String(myTimeServer), ControlColor::Dark, Control::noParent, SetMyTimeServer);
    ESPUI.addControl(ControlType::Option, "Best choice: Your local router (WiFi gateway address)", "Your local router", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Use value set in 'settings.h'", NTPserver_default, ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Your local 'Speedport IP' router", "speedport.ip", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Your local 'Fritz!Box' router", "fritz.box", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Default: pool.ntp.org", "pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "AS: asia.pool.ntp.org", "asia.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "AT: asynchronos.iiss.at", "asynchronos.iiss.at", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "BE: ntp1.oma.be", "ntp1.oma.be", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "BR: ntps1.pads.ufrj.br", "ntps1.pads.ufrj.br", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CA: time.nrc.ca", "time.nrc.ca", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CH: ntp.neel.ch", "ntp.neel.ch", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CL: ntp.shoa.cl", "ntp.shoa.cl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CN: ntp.neu.edu.cn", "ntp.neu.edu.cn", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CZ: ntp.nic.cz", "ntp.nic.cz", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "DE: time.fu-berlin.de", "time.fu-berlin.de", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "ES: hora.roa.es", "hora.roa.es", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "EU: europe.pool.ntp.org", "europe.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "EUS: ntp.i2t.ehu.eus", "ntp.i2t.ehu.eus", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "HU: ntp.atomki.mta.hu", "ntp.atomki.mta.hu", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "IT: ntp1.inrim.it", "ntp1.inrim.it", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "JP: ntp.nict.jp", "ntp.nict.jp", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "MX: cronos.cenam.mx", "cronos.cenam.mx", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "NL: ntp.vsl.nl", "ntp.vsl.nl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "PL: tempus1.gum.gov.pl", "tempus1.gum.gov.pl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "RO: ntp1.usv.ro", "ntp1.usv.ro", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "SE: time1.stupi.se", "time1.stupi.se", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "UK: uk.pool.ntp.org", "uk.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "US: north-america.pool.ntp.org", "north-america.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    mySetTimeServerID = ESPUI.label("Used NTP time server", ControlColor::Dark, NTPserver);

    // Time zone:
    mySetTimeZone = ESPUI.addControl(ControlType::Select, "Choose your time zone (change forces restart)", String(myTimeZone), ControlColor::Dark, Control::noParent, SetMyTimeZone);
    ESPUI.addControl(ControlType::Option, "Use value set in 'settings.h'", Timezone_default, ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Asia: EET-2EEST,M3.5.5/0,M10.5.5/0", "EET-2EEST,M3.5.5/0,M10.5.5/0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Australia: ACST-9:30ACDT,M10.1.0,M4.1.0/3", "ACST-9:30ACDT,M10.1.0,M4.1.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Central Europe: CET-1CEST,M3.5.0,M10.5.0/3 (Austria,Denmark,France,Germany,Italy,Netherlands,Poland,Switzerland)", "CET-1CEST,M3.5.0,M10.5.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Most of Europe: MET-2METDST,M3.5.0/01,M10.5.0/02", "MET-2METDST,M3.5.0/01,M10.5.0/02", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "New Zealand: NZST-12NZDT,M9.5.0,M4.1.0/3 (Auckland)", "NZST-12NZDT,M9.5.0,M4.1.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "UK: GMT0BST,M3.5.0/01,M10.5.0/02 (London)", "GMT0BST,M3.5.0/01,M10.5.0/02", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA EST: EST5EDT,M3.2.0,M11.1.0", "EST5EDT,M3.2.0,M11.1.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA CST: CST6CDT,M3.2.0,M11.1.0", "CST6CDT,M3.2.0,M11.1.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA MST: MST7MDT,M4.1.0,M10.5.0", "MST7MDT,M4.1.0,M10.5.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Vietnam: ICT-7", "ICT-7", ControlColor::Alizarin, mySetTimeZone);
    mySetTimeZoneID = ESPUI.label("Used time zone value", ControlColor::Dark, Timezone);
  }



  // Section smart home control via web URLs:
  // ########################################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Smart home control via web URLs:");

    // About note:
    ESPUI.label("About note", ControlColor::Dark, "Control PixelClock from your smart home environment via web URLs.");

    // Functions note:
    ESPUI.label("Functions", ControlColor::Dark, "You can turn the LEDs off or on via http commands to reduce energy consumption.");

    // Usage note:
    ESPUI.label("Usage hints and examples", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + ":2023' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":2023</a> or <a href='http://" + String(WiFi.getHostname()) + ":2023' target='_blank' style='color:#ffffff;'>http://" + String(WiFi.getHostname()) + ":2023</a>");
  }



  // Section Update:
  // ###############
  if (UseOnlineMode == 1) {
    ESPUI.separator("Update:");

    // Update PixelClock:
    ESPUI.button("Activate update mode", &buttonUpdate, ControlColor::Dark, "Activate update mode", (void*)1);

    // Update URL
    ESPUI.label("Update URLs", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":8080</a> or <a href='http://" + String(WiFi.getHostname()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + String(WiFi.getHostname()) + ":8080</a>");

    // AWSW software GitHub repository:
    ESPUI.label("Download newer software updates here", ControlColor::Dark, "<a href='https://github.com/AWSW-de/AWSW-PixelClock-32x8' target='_blank' style='color:#ffffff;'>https://github.com/AWSW-de/AWSW-PixelClock-32x8</a>");
  }


  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart PixelClock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset WiFi settings:
  if (UseOnlineMode == 1) {
    ESPUI.button("Reset WiFi settings", &buttonWiFiReset, ControlColor::Dark, "Reset WiFi settings", (void*)2);
  }

  // Reset PixelClock settings:
  ESPUI.button("Reset settings (except WiFi & operation mode)", &buttonPixelClockReset, ControlColor::Dark, "Reset PixelClock settings", (void*)3);



  // Section License:
  // ####################
  ESPUI.separator("License information:");

  // License information:
  ESPUI.label("License information", ControlColor::Dark, "NonCommercial — You may not use the project for commercial purposes! © 2024 Copyright by <a href='https://github.com/AWSW-de/AWSW-PixelClock-32x8' target='_blank' style='color:#ffffff;'>AWSW</a>");



  checkforNightMode();             // Check for night mode settings on startup
  ESPUI.begin("AWSW PixelClock");  // Deploy the page
}


// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  if (debugtexts == 1) Serial.println("Read settings from flash: START");
  myTimeZone = preferences.getString("myTimeZone", Timezone_default);
  Timezone = myTimeZone;
  myTimeServer = preferences.getString("myTimeServer", NTPserver_default);
  NTPserver = myTimeServer;
  UseOnlineMode = preferences.getUInt("UseOnlineMode", 1);
  showOMhints = preferences.getUInt("showOMhints", showOMhints_default);
  redVal_time = preferences.getUInt("redVal_time", redVal_time_default);
  greenVal_time = preferences.getUInt("greenVal_time", greenVal_time_default);
  blueVal_time = preferences.getUInt("blueVal_time", blueVal_time_default);
  redVal_back = preferences.getUInt("redVal_back", redVal_back_default);
  greenVal_back = preferences.getUInt("greenVal_back", greenVal_back_default);
  blueVal_back = preferences.getUInt("blueVal_back", blueVal_back_default);
  intensity_day = preferences.getUInt("intensity_day", intensity_day_default);
  intensity_night = preferences.getUInt("intensity_night", intensity_night_default);
  usenightmode = preferences.getUInt("usenightmode", usenightmode_default);
  day_time_start = preferences.getString("day_time_start", day_time_start_default);
  day_time_stop = preferences.getString("day_time_stop", day_time_stop_default);
  iHourOffset = preferences.getUInt("iHourOffset", iHourOffset_default);
  useshowip = preferences.getUInt("useshowip", useshowip_default);
  useStartupText = preferences.getUInt("useStartupText", useStartupText_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  useWiFiReCon = preferences.getUInt("useWiFiReCon", useWiFiReCon_default);
  usetestNumbers = preferences.getUInt("usetestNumbers", usetestNumbers_default);
  if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  if (debugtexts == 1) Serial.println("Write settings to flash: START");
  changedvalues = false;
  preferences.putString("myTimeZone", myTimeZone);
  preferences.putString("myTimeServer", myTimeServer);
  preferences.putUInt("UseOnlineMode", UseOnlineMode);
  preferences.putUInt("showOMhints", showOMhints);
  preferences.putUInt("redVal_time", redVal_time);
  preferences.putUInt("greenVal_time", greenVal_time);
  preferences.putUInt("blueVal_time", blueVal_time);
  preferences.putUInt("redVal_back", redVal_back);
  preferences.putUInt("greenVal_back", greenVal_back);
  preferences.putUInt("blueVal_back", blueVal_back);
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putString("day_time_start", day_time_start);
  preferences.putString("day_time_stop", day_time_stop);
  preferences.putUInt("iHourOffset", iHourOffset);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("useStartupText", useStartupText);
  preferences.putUInt("RandomColor", RandomColor);
  preferences.putUInt("useWiFiReCon", useWiFiReCon);
  preferences.putUInt("usetestNumbers", usetestNumbers);
  if (debugtexts == 1) Serial.println("Write settings to flash: END");
  checkforNightMode();
  updatenow = true;  // Update display now...
}


// ###########################################################################################################################################
// # GUI: Reset the PixelClock settings:
// ###########################################################################################################################################
void buttonPixelClockReset(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(100);
  TextOnMatrix("RESET", 0, 255, 0);
  updatedevice = false;
  delay(1000);
  Serial.println("Status: PIXELCLOCK SETTINGS RESET REQUEST EXECUTED");
  // Save stored values for WiFi and Operation Mode:
  String tempDelWiFiSSID = preferences.getString("WIFIssid");
  String tempDelWiFiPASS = preferences.getString("WIFIpass");
  int tempOfflineMode = preferences.getUInt("UseOnlineMode");
  preferences.clear();
  delay(100);
  preferences.putUInt("UseOnlineMode", tempOfflineMode);  // Restore Operation Mode
  preferences.putString("WIFIssid", tempDelWiFiSSID);     // Restore entered WiFi SSID
  preferences.putString("WIFIpass", tempDelWiFiPASS);     // Restore entered WiFi password
  preferences.putUInt("showOMhints", showOMhints_default);
  preferences.putUInt("redVal_time", redVal_time_default);
  preferences.putUInt("greenVal_time", greenVal_time_default);
  preferences.putUInt("blueVal_time", blueVal_time_default);
  preferences.putUInt("redVal_back", redVal_back_default);
  preferences.putUInt("greenVal_back", greenVal_back_default);
  preferences.putUInt("blueVal_back", blueVal_back_default);
  preferences.putUInt("intensity_day", intensity_day_default);
  preferences.putUInt("intensity_night", intensity_night_default);
  preferences.putUInt("useshowip", useshowip_default);
  preferences.putUInt("useStartupText", useStartupText_default);
  preferences.putUInt("usenightmode", usenightmode_default);
  preferences.putString("day_time_stop", day_time_stop_default);
  preferences.putString("day_time_stop", day_time_stop_default);
  preferences.putUInt("RandomColor", RandomColor_default);
  preferences.putString("myTimeZone", Timezone_default);
  preferences.putString("myTimeServer", NTPserver_default);
  preferences.putUInt("useWiFiReCon", useWiFiReCon_default);
  delay(100);
  preferences.end();
  Serial.println("######################################################################################################");
  Serial.println("# PIXELCLOCK SETTING WERE SET TO DEFAULT... PIXELCLOCK WILL NOW RESTART... PLEASE CONFIGURE AGAIN... #");
  Serial.println("######################################################################################################");
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # Clear the display:
// ###########################################################################################################################################
void ClearDisplay() {
  uint32_t c0 = strip.Color(0, 0, 0);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  // Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
  int ipdelay = 2000;
  TextOnMatrix("IP:", redVal_time, greenVal_time, blueVal_time);  // IP:
  delay(ipdelay);
  TextOnMatrix(String(WiFi.localIP()[0]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 1
  delay(ipdelay);
  TextOnMatrix(String(WiFi.localIP()[1]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 2
  delay(ipdelay);
  TextOnMatrix(String(WiFi.localIP()[2]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 3
  delay(ipdelay);
  TextOnMatrix(String(WiFi.localIP()[3]), redVal_time, greenVal_time, blueVal_time);  // Octet 4
  delay(ipdelay);
}


// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}


// ###########################################################################################################################################
// # GUI: Restart the PixelClock:
// ###########################################################################################################################################
void buttonRestart(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(250);
  TextOnMatrix("RESET", 0, 255, 0);
  updatedevice = false;
  if (changedvalues == true) setFlashValues();  // Write settings to flash
  preferences.end();
  delay(100);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Reset the WiFi settings of the PixelClock:
// ###########################################################################################################################################
void buttonWiFiReset(Control* sender, int type, void* param) {
  updatedevice = false;
  Serial.println("Status: WIFI SETTINGS RESET REQUEST");
  TextOnMatrix("RESET", 0, 255, 0);
  updatedevice = false;
  WiFi.disconnect();  // DISCONNECT FROM WIFI
  delay(1000);
  preferences.putString("WIFIssid", "");                // Reset WiFi SSID
  preferences.putString("WIFIpass", "");                // Reste WiFi password
  preferences.putUInt("useshowip", useshowip_default);  // Show IP-address again
  preferences.end();
  Serial.println("Status: WIFI SETTINGS RESET REQUEST EXECUTED");
  Serial.println("####################################################################################################");
  Serial.println("# WIFI SETTING WERE SET TO DEFAULT... PixelClock WILL NOW RESTART... PLEASE CONFIGURE WIFI AGAIN... #");
  Serial.println("####################################################################################################");
  delay(500);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Update the PixelClock:
// ###########################################################################################################################################
void buttonUpdate(Control* sender, int type, void* param) {
  preferences.end();
  useWiFiReCon = 0;  // Temporary set "Active WiFi reconnect to off during the update to avoid problems during the update"
  updatedevice = false;
  delay(1000);
  ESPUI.updateButton(sender->id, "Update mode active now - Use the update url: >>>");
  if (updatemode == false) {
    updatemode = true;
    TextOnMatrix("SW UP", 0, 0, 255);
    delay(1000);
    updatedevice = false;
    Serial.println("Status: Update request");
  }
}


// ###########################################################################################################################################
// # Show a LED output for a text string:
// ###########################################################################################################################################
void TextOnMatrix(String txt, int r, int g, int b) {
  updatedevice = false;
  delay(1000);
  uint32_t color = matrix.Color(r, g, b);
  matrix.setTextColor(color);
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.print(txt);
  matrix.show();
}

// ###########################################################################################################################################
// # GUI: Night mode switch:
// ###########################################################################################################################################
void switchNightMode(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usenightmode = 1;
      break;
    case S_INACTIVE:
      intensity = intensity_day;
      usenightmode = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use random color mode:
// ###########################################################################################################################################
void switchRandomColor(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      RandomColor = 1;
      ESPUI.updateVisibility(text_colour_background, false);
      ESPUI.updateVisibility(text_colour_time, false);
      redVal_back = 0;
      greenVal_back = 0;
      blueVal_back = 0;
      break;
    case S_INACTIVE:
      RandomColor = 0;
      ESPUI.updateVisibility(text_colour_background, true);
      ESPUI.updateVisibility(text_colour_time, true);
      ESPUI.jsonReload();
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Show IP-ADdress switch:
// ###########################################################################################################################################
void switchShowIP(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useshowip = 1;
      break;
    case S_INACTIVE:
      useshowip = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Test numbers on startup switch:
// ###########################################################################################################################################
void switchtestNumbers(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usetestNumbers = 1;
      break;
    case S_INACTIVE:
      usetestNumbers = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use WIFi reconnect function during runtime:
// ###########################################################################################################################################
void switchWiFiReConnect(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useWiFiReCon = 1;
      break;
    case S_INACTIVE:
      useWiFiReCon = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Show PixelClock text switch:
// ###########################################################################################################################################
void switchStartupText(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useStartupText = 1;
      break;
    case S_INACTIVE:
      useStartupText = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - TIME:
// ###########################################################################################################################################
void getRGBTIME(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal_time = red;
  greenVal_time = green;
  blueVal_time = blue;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - BACKGROUND:
// ###########################################################################################################################################
void getRGBBACK(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal_back = red;
  greenVal_back = green;
  blueVal_back = blue;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - helper function:
// ###########################################################################################################################################
int hexcolorToInt(char upper, char lower) {
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  return uVal + lVal;
}


// ###########################################################################################################################################
// # GUI: Color change for time color:
// ###########################################################################################################################################
#define SLIDER_UPDATE_TIME 150  // Wait at least 100 ms before allowing another slider update --> Bug fix for color slider crashing ESP32
void colCallTIME(Control* sender, int type) {
  static unsigned long last_slider_update = 0;  // Track the time of the last slider update
  if ((millis() - last_slider_update >= SLIDER_UPDATE_TIME)) {
    getRGBTIME(sender->value);
    last_slider_update = millis();
    if (debugtexts == 1) {
      Serial.println(type);
      Serial.println(sender->value);
    }
  }
  return;
}


// ###########################################################################################################################################
// # GUI: Color change for background color:
// ###########################################################################################################################################
#define SLIDER_UPDATE_BACK 150  // Wait at least 100 ms before allowing another slider update --> Bug fix for color slider crashing ESP32
void colCallBACK(Control* sender, int type) {
  static unsigned long last_slider_update = 0;  // Track the time of the last slider update
  if ((type == 10) && (millis() - last_slider_update >= SLIDER_UPDATE_BACK)) {
    getRGBBACK(sender->value);
    last_slider_update = millis();
    if (debugtexts == 1) {
      Serial.println(type);
      Serial.println(sender->value);
    }
  }
  return;
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: DAY
// ###########################################################################################################################################
void sliderBrightnessDay(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_day = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: NIGHT
// ###########################################################################################################################################
void sliderBrightnessNight(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_night = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Start
// ###########################################################################################################################################
void call_day_time_start(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_start = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Stop
// ###########################################################################################################################################
void call_day_time_stop(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_stop = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}


// ###########################################################################################################################################
// # GUI: Use Offline Mode switch:
// ###########################################################################################################################################
void switchOffline(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  Serial.println("Offline Mode change: Restart request");
  switch (value) {
    case S_ACTIVE:
      UseOnlineMode = 1;  // Online
      useshowip = 1;
      break;
    case S_INACTIVE:
      UseOnlineMode = 0;  // Offline
      useshowip = 1;
      break;
  }
  changedvalues = true;
  setFlashValues();  // Save values!
  TextOnMatrix("RESET", 0, 255, 0);
  Serial.println("Offline Mode change: Perform restart now");
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Show or hide the Operation Mode hints:
// ###########################################################################################################################################
void switchOMhints(Control* sender, int value) {
  updatedevice = false;
  switch (value) {
    case S_ACTIVE:
      showOMhints = 1;  // Show
      ESPUI.updateVisibility(OfflineModeHint1, true);
      ESPUI.updateVisibility(OfflineModeHint2, true);
      ESPUI.updateVisibility(OfflineModeHint3, true);
      ESPUI.jsonReload();
      break;
    case S_INACTIVE:
      showOMhints = 0;  // Hide
      ESPUI.updateVisibility(OfflineModeHint1, false);
      ESPUI.updateVisibility(OfflineModeHint2, false);
      ESPUI.updateVisibility(OfflineModeHint3, false);
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Manual Offline Mode offset hour setting
// ###########################################################################################################################################
void SetOfflineHourOffset(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  iHourOffset = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Zone selection:
// ###########################################################################################################################################
void SetMyTimeZone(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  myTimeZone = sender->value;
  changedvalues = true;
  updatedevice = true;
  setFlashValues();  // Save values!
  TextOnMatrix("RESET", 0, 255, 0);
  Serial.println("Time Zone change: Perform restart now");
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Time Server selection:
// ###########################################################################################################################################
void SetMyTimeServer(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  if (sender->value == "Your local router") {
    myTimeServer = IpAddress2String(WiFi.gatewayIP());
  } else {
    myTimeServer = sender->value;
  }
  changedvalues = true;
  updatedevice = true;
  setFlashValues();  // Save values!
  TextOnMatrix("RESET", 0, 255, 0);
  Serial.println("Time Server change: Perform restart now");
  Serial.println("Time Server set to: " + String(sender->value));
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Manual time setting from device (PC, tablet, smartphone) as long as the Offline Mode web portal is opened:
// ###########################################################################################################################################
void getTimeCallback(Control* sender, int type) {
  if (type == B_UP) {
    ESPUI.updateTime(timeId);
  }
}



// ###########################################################################################################################################
// # GUI: Manual time setting from device (PC, tablet, smartphone):
// ###########################################################################################################################################
void timeCallback(Control* sender, int type) {
  updatedevice = false;
  if (type == TM_VALUE) {
    // Serial.print("Auto Time by device: ");
    // Serial.println(sender->value);
    // ESPUI.print(statusTimeFromDevice, String(sender->value));

    String calTimestamp = String(sender->value);
    struct tm tm;
    // Test String to get all values:
    // Serial.println("Parsing " + calTimestamp);  // 2024-01-04T18:33:37.294Z
    // #######################################################
    // for (int i = 0; i <= 25; i++) {
    //   Serial.print(i);
    //   Serial.print(" = ");
    //   Serial.println(calTimestamp.substring(i).toInt());
    // }
    // #######################################################

    // Date not used yet:
    // String year = calTimestamp.substring(0, 4);
    // String month = calTimestamp.substring(4, 6);
    // if (month.startsWith("0")) {
    //   month = month.substring(1);
    // }
    // String day = calTimestamp.substring(6, 8);
    // if (day.startsWith("0")) {
    //   month = day.substring(1);
    // }
    // tm.tm_year = year.toInt() - 1900;
    // tm.tm_mon = month.toInt() - 1;
    // tm.tm_mday = day.toInt();
    tm.tm_hour = calTimestamp.substring(11).toInt();  // Hour from string
    tm.tm_min = calTimestamp.substring(14).toInt();   // Minute from string
    tm.tm_sec = calTimestamp.substring(17).toInt();   // Second from string
    iHour = tm.tm_hour;
    iMinute = tm.tm_min;
    iSecond = tm.tm_sec;

    ESPUI.print(statusTimeFromDevice, String(iHour) + ":" + String(iMinute) + ":" + String(iSecond));  // Update GUI: Time from device
    if (iHourOffset >= 0) {
      // Serial.println("iHour OLD: " + String(iHour));
      // Serial.println("iHour + Offset: " + String(iHourOffset));  // Set stored hour offset to received hour
      iHour = iHour + iHourOffset;
      // Serial.println("iHour NEW: " + String(iHour));
    }
    if ((iHourOffset <= -1)) {
      // Serial.println("iHour OLD: " + String(iHour));
      // Serial.println("iHour - Offset: " + String(iHourOffset));
      iHour = iHourOffset + iHour;
      // Serial.println("iHour NEW: " + String(iHour));
    }
    int xHour = iHour;
    if (xHour >= 24) xHour = xHour - 24;                                                                 // Mid night corrections because of offset calculation
    if (xHour < 0) xHour = xHour + 24;                                                                   // Mid night corrections because of offset calculation
    ESPUI.print(statusTimeSetOffline, String(xHour) + ":" + String(iMinute) + ":" + String(iSecond));    // Update GUI: Calculated time with offset for negative offset
    if (debugtexts == 1) Serial.println(String(xHour) + ":" + String(iMinute) + ":" + String(iSecond));  // Test output
    rtc.setTime(iSecond, iMinute, xHour, 1, 1, 2024);                                                    // Set time on device RTC: (ss, mm, hh, DD, MM, YYYY) --> Date not used yet
    checkforNightMode();                                                                                 // Night Mode check
    updatenow = true;
    updatedevice = true;
  }
  delay(1000);
}


// ###########################################################################################################################################
// # GUI: Check if Night Mode needs to be set depending on the time:
// ###########################################################################################################################################
void checkforNightMode() {
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_start H part = " + day_time_startH);
  if (debugtexts == 1) Serial.println("day_time_start M part = " + day_time_startM);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  if (debugtexts == 1) Serial.println("dt_start_HM = " + String(dt_start_HM));
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_stop H part = " + day_time_stopH);
  if (debugtexts == 1) Serial.println("day_time_stop M part = " + day_time_stopM);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  if (debugtexts == 1) Serial.println("dt_stop_HM = " + String(dt_stop_HM));
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;
  if (debugtexts == 1) Serial.println("now_HM = " + String(now_HM));

  // Check if start time is before stop time:
  if ((dt_start_HM > dt_stop_HM) || (dt_start_HM == dt_stop_HM)) {
    if (debugtexts == 1) Serial.println("Wrong setting! 'Day time starts' needs to be before 'Day time ends after'. Please correct values.");
    ESPUI.updateVisibility(statusNightModeWarnID, true);  // Show warning
    ESPUI.jsonReload();
  } else {
    ESPUI.updateVisibility(statusNightModeWarnID, false);  // Hide warning
    // Day or Night time:
    if (usenightmode == 1) {
      if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
        if (debugtexts == 1) Serial.println("Day Time");
        statusNightModeIDtxt = "Day Time";
        ESPUI.print(statusNightModeID, "Day time");
        if ((iHour == 0) && (day_time_startH.toInt() == 23)) {  // Special function if day_time_stop set to 23 and time is 24, so 0...
          statusNightModeIDtxt = "Night time";
          ESPUI.print(statusNightModeID, "Night time");
        }
      } else {
        if (debugtexts == 1) Serial.println("Night Time");
        statusNightModeIDtxt = "Night time";
        ESPUI.print(statusNightModeID, "Night time");
      }
    } else {
      ESPUI.print(statusNightModeID, "Night mode not used");
    }
  }
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
  show_time(iHour, iMinute, iSecond);
}


// ###########################################################################################################################################
// # Display hours and minutes text function:
// ###########################################################################################################################################
uint32_t colorRGB;
static int lastHourSet = -1;
static int lastMinutesSet = -1;
static int lastSecondSet = -1;
static int lastMinutesRandomeSet = -1;
void show_time(int hours, int minutes, int seconds) {
  if ((lastHourSet == hours && lastMinutesSet == minutes && lastSecondSet == seconds) && updatenow == false) {  // Reduce display updates to new minutes and new config updates
    return;
  }

  updatenow = false;
  lastHourSet = hours;
  lastMinutesSet = minutes;
  lastSecondSet = seconds;

  // Show current time of display update:
  // if (debugtexts == 1) Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));
  // Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));

  // Set LED intensity + DAY/NIGHT MDOE:
  // ##################
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;

  // Set intensity:
  if ((usenightmode == 1) && (set_web_intensity == 0)) {
    if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
      intensity = intensity_day;
      if (statusNightModeIDtxt != "Day time") {
        statusNightModeIDtxt = "Day time";
        ESPUI.print(statusNightModeID, "Day time");
      }
      if ((iHour == 0) && (day_time_startH.toInt() == 23)) {  // Special function if day_time_stop set to 23 and time is 24, so 0...
        intensity = intensity_night;
        if (statusNightModeIDtxt != "Night time") {
          statusNightModeIDtxt = "Night time";
          ESPUI.print(statusNightModeID, "Night time");
        }
      }
    } else {
      intensity = intensity_night;
      if (statusNightModeIDtxt != "Night time") {
        statusNightModeIDtxt = "Night time";
        ESPUI.print(statusNightModeID, "Night time");
      }
    }
  } else {
    if (set_web_intensity == 0) intensity = intensity_day;
    if (set_web_intensity == 1) intensity = intensity_web;
  }
  strip.setBrightness(intensity);
  matrix.setBrightness(intensity);

  // Set background color:
  back_color();

  // Static text color or random color mode:
  if (RandomColor == 0) colorRGB = strip.Color(redVal_time, greenVal_time, blueVal_time);
  if (RandomColor == 1) {
    if (lastMinutesRandomeSet != minutes) {  // Update 1x per minute only
      lastMinutesRandomeSet = minutes;
      colorRGB = strip.Color(random(255), random(255), random(255));
    }
  }

  // Display time:
  iHour = hours;
  iMinute = minutes;

  // Show the time numbers:
  shownumber(getDigit(iHour, 1), 1, colorRGB);
  shownumber(getDigit(iHour, 0), 2, colorRGB);
  shownumber(getDigit(iMinute, 1), 3, colorRGB);
  shownumber(getDigit(iMinute, 0), 4, colorRGB);

  // Show dots or not:
  showDots(colorRGB);

  // Update the display:
  strip.show();
}


// ###########################################################################################################################################
// # Background color function: SET ALL LEDs OFF
// ###########################################################################################################################################
void back_color() {
  uint32_t c0 = strip.Color(redVal_back, greenVal_back, blueVal_back);  // Background color
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
}


// ###########################################################################################################################################
// # NTP time functions:
// ###########################################################################################################################################
void configNTPTime() {
  initTime(Timezone);
  printLocalTime();
}
// ###########################################################################################################################################
void setTimezone(String timezone) {
  Serial.printf("Setting timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
  delay(1000);
}
// ###########################################################################################################################################
void initTime(String timezone) {
  Serial.println("Setting up time from: " + NTPserver);
  TextOnMatrix("TIME", 0, 0, 255);

  struct tm timeinfo;
  configTime(0, 0, NTPserver.c_str());
  delay(500);

  while (!getLocalTime(&timeinfo)) {
    delay(1000);
    TextOnMatrix("TIME", 255, 0, 0);
    delay(1000);
    TextOnMatrix("RESET", 0, 255, 0);
    Serial.println("! Failed to obtain time - Time server could not be reached ! --> RESTART THE DEVICE NOW...");
    delay(250);
    ESP.restart();
  }

  // Time successfully received:
  TextOnMatrix("TIME", 0, 255, 0);
  delay(1000);
  Serial.println("Got the time from NTP server: " + NTPserver);
  setTimezone(timezone);
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
}
// ###########################################################################################################################################
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iStartTime = String(timeStringBuff);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  delay(500);
}
// ###########################################################################################################################################
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {
  struct tm tm;
  tm.tm_year = yr - 1900;  // Set date
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;  // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}
// ###########################################################################################################################################


// ###########################################################################################################################################
// # HTML command web server:
// ###########################################################################################################################################
int ew = 0;  // Current extra word
String ledstatus = "ON";
void handleLEDupdate() {  // LED server pages urls:

  ledserver.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show a manual how to use these links:
    String message = "PixelClock web configuration and querry options examples:\n\n";
    message = message + "General:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023 --> Shows this text\n\n";
    message = message + "Get the status of the PixelClock LEDs:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/status --> Show the status of the LEDs (0 = OFF and 1 = ON).\n\n";
    message = message + "Turn the LEDs OFF or ON:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=0 --> LED intensity is set to OFF which will turn the display off.\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1 --> LED intensity is set to ON which will turn the display on again...\n";
    request->send(200, "text/plain", message);
  });

  ledserver.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {  // Configure background and time texts color and intensity:
    int paramsNr = request->params();
    // Serial.println(paramsNr);
    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      // Serial.print("Param name: ");
      // Serial.println(p->name());
      // Serial.print("Param value: ");
      // Serial.println(p->value());
      // Serial.println("------------------");
      if ((p->value().toInt() >= 0) && (p->value().toInt() <= 1)) {
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 0)) {
          set_web_intensity = 1;
          ledstatus = "OFF";
          ESPUI.updateVisibility(intensity_web_HintID, true);
          ESPUI.updateVisibility(statusNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessDayID, false);
          ESPUI.updateVisibility(switchNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessNightID, false);
          ESPUI.updateVisibility(call_day_time_startID, false);
          ESPUI.updateVisibility(call_day_time_stopID, false);
          ESPUI.updateVisibility(text_colour_time, false);
          ESPUI.updateVisibility(text_colour_background, false);
          ESPUI.updateVisibility(switchRandomColorID, false);
          ESPUI.updateVisibility(DayNightSectionID, false);
          ESPUI.jsonReload();
        }
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 1)) {
          set_web_intensity = 0;
          ledstatus = "ON";
          ESPUI.updateVisibility(intensity_web_HintID, false);
          ESPUI.updateVisibility(statusNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessDayID, true);
          ESPUI.updateVisibility(switchNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessNightID, true);
          ESPUI.updateVisibility(call_day_time_startID, true);
          ESPUI.updateVisibility(call_day_time_stopID, true);
          ESPUI.updateVisibility(text_colour_time, true);
          ESPUI.updateVisibility(text_colour_background, true);
          ESPUI.updateVisibility(switchRandomColorID, true);
          ESPUI.updateVisibility(DayNightSectionID, true);
        }
        changedvalues = true;
        updatenow = true;
      } else {
        request->send(200, "text/plain", "INVALID VALUES - MUST BE BETWEEN 0 and 1");
      }
    }
    request->send(200, "text/plain", "PixelClock LEDs set to: " + ledstatus);
  });

  ledserver.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show the status of all extra words and the color for the background and time texts:
    String message = ledstatus;
    request->send(200, "text/plain", message);
  });

  ledserver.begin();
}

// ###########################################################################################################################################
// # Startup LED test function
// ###########################################################################################################################################
void callStartText() {
  // Serial.println("Show 'PixelClock' startup text...");
  TextOnMatrix("AWSW", redVal_time, greenVal_time, blueVal_time);
  TextOnMatrix("PIXEL", redVal_time, greenVal_time, blueVal_time);
  TextOnMatrix("CLOCK", redVal_time, greenVal_time, blueVal_time);
}


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>PixelClock Setup</title>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 15px;
        box-shadow: 0 9px #999;
      }
      button:hover {
        background-color: #3e8e41;
      }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script type="text/javascript">
      function disableButtonAndSubmit() {
        var btn = document.getElementById("submitButton");
        btn.disabled = true;
        setTimeout(function() {
          document.forms["myForm"].submit();
        }, 100);
      }
    </script>
  </head>
  <body>
    <form action="/start" name="myForm">
      <center>
        <h1>Welcome to the PixelClock setup</h1>
        <p>Please add your local WiFi credentials on the next page</p>
        <p><button id="submitButton" type="submit" onclick="disableButtonAndSubmit()">Configure PixelClock</button></p>
      </center>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>PixelClock Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script language="JavaScript">
      function updateSSIDInput() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect && ssidSelect.options.length > 0) {
          document.getElementById("mySSID").value = ssidSelect.options[ssidSelect.selectedIndex].value;
        }
      }
       function validateForm() {
        var errorParagraph = document.querySelector('.error');
        errorParagraph.style.display = 'none';
        errorParagraph.innerHTML = '';
        if (document.forms["myForm"]["mySSID"].value == "") {
          errorParagraph.innerHTML = "WiFi SSID must be set. ";
          errorParagraph.style.display = 'block';
          return false;
        }
        if (document.forms["myForm"]["myPW"].value == "") {
          errorParagraph.innerHTML = "WiFi password must be set. ";
          errorParagraph.style.display = 'block'; 
          return false;
        }
      }
      function disableButtonAndSubmit() {
        if (validateForm()) {
          var btn = document.getElementById("submitButton");
          btn.innerText = 'Restarting PixelClock...';
          btn.disabled = true;
          setTimeout(function() {
            document.forms["myForm"].submit();
          }, 1000);
        }
      }
      window.onload = function() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect) {
          ssidSelect.addEventListener('change', updateSSIDInput);
        }
      };
    </script>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      p.error { 
        color: #ff0000; 
        display: none;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 15px;
        box-shadow: 0 9px #999;
      }
      button:hover { background-color: #3e8e41 }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
  </head>
  <body>
    <form action="/get" name="myForm" onsubmit="return validateForm()">
      <h1>Initial PixelClock setup:</h1>
      <!-- Select element will be dynamically added here -->
      <p>
        <label for="mySSID">Enter your WiFi SSID:</label><br />
        <input id="mySSID" name="mySSID" value="" />
      </p>
      <p>
        <label for="myPW">Enter your WiFi password:</label><br/>
        <input type="text" id="myPW" name="myPW" value="" />
      </p>
      <p class="error">Errors will be displayed here!</p>
      <p>
        <button id="submitButton" onclick="disableButtonAndSubmit()">Save values</button>
      </p>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>PixelClock Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
    </style>
  </head>
  <body>
    <h1>Settings saved!</h1>
    <p>PixelClock is now trying to connect to the selected WiFi.</p>
    <p>First the WiFi leds will be lit blue and change to green in case of a successful WiFi connection.</p>
    <p>If the connection fails the WiFi leds will flash red. Then please reconnect to the temporary access point again.</p>
    <p>Please close this page now, rejoin your selected WiFi and enjoy your PixelClock. =)</p>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
String ScanWiFi() {
  String html = config_html;
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println("Scan WiFi networks - END");
  Serial.println(" ");
  if (n > 0) {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    String ssidList = "<p><label for=\"mySSISelect\">Found these networks:</label><br /><select id=\"mySSIDSelect\" name=\"mySSIDSelect\"><option value=\"\" disabled selected>Choose...</option>";
    for (int i = 0; i < n; ++i) {
      ssidList += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*");
    }
    ssidList += "</select></p>";
    html.replace("<!-- Select element will be dynamically added here -->", ssidList);
  } else {
    Serial.println("WIFI networks found");
  }
  return html;
}


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePortalSetup() {
  String htmlConfigContent = ScanWiFi();
  const char* temp_ssid = "PixelClock";
  const char* temp_password = "";
  WiFi.softAP(temp_ssid, temp_password);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("#################################################################################################################################################################################");
  Serial.print("# Temporary WiFi access point initialized. Please connect to the WiFi access point now and set your local WiFi credentials. Access point name: ");
  Serial.println(temp_ssid);
  Serial.print("# In case your browser does not open the PixelClock setup page automatically after connecting to the access point, please navigate to this URL manually to http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("#################################################################################################################################################################################");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      // Serial.println(inputMessage);
      preferences.putString("WIFIssid", inputMessage);  // Save entered WiFi SSID
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      // Serial.println(inputMessage);
      preferences.putString("WIFIpass", inputMessage);  // Save entered WiFi password
      delay(250);
      preferences.end();
    } else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send_P(200, "text/html", saved_html);
    TextOnMatrix("RESET", 0, 255, 0);
    delay(1000);
    ESP.restart();
  });

  server.on("/start", HTTP_GET, [htmlConfigContent](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", htmlConfigContent.c_str());
  });

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("microsoft.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("URL not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  Serial.println("PixelClock Captive Portal web server started");
}


// ###########################################################################################################################################
// # Wifi setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_SETUP() {
  Serial.println(" ");
  esp_log_level_set("wifi", ESP_LOG_WARN);  // Disable WiFi debug warnings
  String WIFIssid = preferences.getString("WIFIssid");
  bool WiFiConfigEmpty = false;
  if (WIFIssid == "") {
    WiFiConfigEmpty = true;
  }
  String WIFIpass = preferences.getString("WIFIpass");
  if (WIFIpass == "") {
    WiFiConfigEmpty = true;
  }
  if (WiFiConfigEmpty == true) {
    Serial.println("Show SETUP text...");
    TextOnMatrix("AWSW", 0, 255, 255);
    TextOnMatrix("PIXEL", 0, 255, 255);
    TextOnMatrix("CLOCK", 0, 255, 255);
    TextOnMatrix("SETUP", 0, 255, 255);
    Serial.println("Show SETUP WIFI NOW...");
    CaptivePortalSetup();
    for (int i = 0; i <= 3; i++) {
      TextOnMatrix("NO", 0, 255, 255);
      TextOnMatrix("WIFI", 0, 255, 255);
      TextOnMatrix("SET->", 0, 255, 255);
      TextOnMatrix("SETUP", 0, 255, 255);
      TextOnMatrix("WIFI", 0, 255, 255);
      TextOnMatrix("NOW", 0, 255, 255);
    }
    TextOnMatrix("WIFI", 0, 255, 255);
  } else {
    Serial.println("Try to connect to found WiFi configuration: ");
    WiFi.disconnect();
    int tryCount = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
    Serial.println("Connecting to WiFi: " + String(WIFIssid));
    while (WiFi.status() != WL_CONNECTED) {
      TextOnMatrix("WIFI", 0, 0, 255);
      tryCount = tryCount + 1;
      Serial.print("Connection try #: ");
      Serial.println(tryCount);
      if (tryCount >= maxWiFiconnctiontries - 10) {
        TextOnMatrix("WIFI", 255, 0, 0);
      }
      if (tryCount == maxWiFiconnctiontries) {
        Serial.println("\n\nWIFI CONNECTION ERROR: If the connection still can not be established please check the WiFi settings or location of the device.\n\n");
        preferences.putString("WIFIssid", "");  // Reset entered WiFi ssid
        preferences.putString("WIFIpass", "");  // Reset entered WiFi password
        preferences.end();
        delay(250);
        Serial.println("WiFi settings deleted because in " + String(maxWiFiconnctiontries) + " tries the WiFi connection could not be established. Temporary PixelClock access point will be started to reconfigure WiFi again.");
        ESP.restart();
      }
      delay(1000);
      TextOnMatrix("WIFI", 0, 0, 0);
      delay(500);
    }
    Serial.println(" ");
    WiFIsetup = true;
    Serial.print("Successfully connected now to WiFi SSID: ");
    Serial.println(WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("DNS: " + WiFi.dnsIP().toString());
    Serial.println("GW: " + WiFi.gatewayIP().toString());
    Serial.println("ESP32 hostname: " + String(WiFi.getHostname()));
    TextOnMatrix("WIFI", 0, 255, 0);
    delay(1000);
    configNTPTime();      // NTP time setup
    setupWebInterface();  // Generate the configuration page
    handleLEDupdate();    // LED update via web
    setupOTAupate();      // ESP32 OTA update
    Serial.println("######################################################################");
    Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
    Serial.println("# Web interface online at: http://" + String(WiFi.getHostname()));
    Serial.println("# HTTP controls online at: http://" + IpAddress2String(WiFi.localIP()) + ":2023");
    Serial.println("# HTTP controls online at: http://" + String(WiFi.getHostname()) + ":2023");
    Serial.println("######################################################################");
    Serial.println("# PixelClock startup finished...");
    Serial.println("######################################################################");
    if (useStartupText == 1) callStartText();  // Show "PixelClock" startup text
    if (useshowip == 1) ShowIPaddress();       // Display the current IP-address
    if (usetestNumbers == 1) testNumbers();    // Test the numbers
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    updatedevice == true;
    updatenow = true;  // Update the display 1x after startup
    update_display();  // Update LED display
  }
}

// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char otaserverIndex[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>PixelClock</title></head>
      <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <body>
    <form method='POST' action='/update' enctype='multipart/form-data'>
      <center><b><h1>PixelClock software update</h1></b>
      <h2>Please select the in the Arduino IDE > "Sketch" ><br/>"Export Compiled Binary (Alt+Ctrl+S)"<br/>to generate the required "Code.ino.bin" file.<br/><br/>
      Select the "Code.ino.bin" file with the "Search" button.<br/><br/>
      Use the "Update" button to start the update.<br/><br/>PixelClock will restart automatically.</h2><br/>
      <input type='file' name='update'>       <input type='submit' value='Update'>
     </center></form></body>
  </html>
 )=====";


const char otaNOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>PixelClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>PixelClock software update</h1></b>
      <h2>ERROR: Software update FAILED !!!<br/><br/>PixelClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


const char otaOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>PixelClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>PixelClock software update</h1></b>
      <h2>Software update done =)<br/><br/>PixelClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


void setupOTAupate() {
  otaserver.on("/", HTTP_GET, []() {
    otaserver.sendHeader("Connection", "close");
    otaserver.send(200, "text/html", otaserverIndex);
  });

  otaserver.on(
    "/update", HTTP_POST, []() {
      otaserver.sendHeader("Connection", "close");
      if (Update.hasError()) {
        otaserver.send(200, "text/html", otaNOK);
        TextOnMatrix("SW UP", 255, 0, 0);
        delay(1000);
        TextOnMatrix("RESET", 255, 0, 0);
        delay(1000);
      } else {
        otaserver.send(200, "text/html", otaOK);
        TextOnMatrix("SW UP", 0, 255, 0);
        delay(1000);
        TextOnMatrix("RESET", 0, 255, 0);
        delay(1000);
      }
      delay(3000);
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = otaserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        TextOnMatrix("LOAD", 0, 0, 255);
        delay(1000);
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          TextOnMatrix("LOAD", 0, 255, 0);
          delay(1000);
          Serial.printf("Update success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update failed unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  otaserver.begin();
}


// ###########################################################################################################################################
// # OFFLINE MODE Captive Portal by AWSW
// ###########################################################################################################################################
void OfflinePotalSetup() {
  if (debugtexts == 1) Serial.println("\nCreating PixelClock Offline Mode access point...");
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (UseOfflineModeWithPassword == 1) {  // Access point with password or not
    WiFi.softAP(Offline_SSID, Offline_PW);
  } else {
    WiFi.softAP(Offline_SSID);
  }
  Serial.println("##############################################################################################################################################################################################################");
  Serial.print("# Offline Mode WiFi access point initialized. Please connect to the WiFi access point and set the current time now. Access point name: '");
  Serial.print(Offline_SSID);
  if (UseOfflineModeWithPassword == 1) {
    Serial.print("' using the password: '");
    Serial.print(Offline_PW);
  }
  Serial.println("'");
  Serial.print("# In case your browser does not open the PixelClock configuration page automatically after connecting to the access point, please navigate to this URL manually: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("##############################################################################################################################################################################################################");

  setupWebInterface();  // Generate the configuration page

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /connecttest.txt");
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served msftconnecttest.com");
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /fwlink");
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served wpad.dat");
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /gernerate_204");
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /redirect");
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /hotspot-detect.html");
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /cannonical.html");
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /success.txt");
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /ncsi.txt");
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /chrome-variations/seed");
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /service/update2/json");
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /chat");
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /startpage");
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /favicon.ico");
  });


  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    // AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_offline_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    // request->redirect(captiveportalURL);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("Web page not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  if (debugtexts == 1) Serial.println("PixelClock OFFLINE MODE captive portal web server started");
  ShowOfflineIPaddress();  // Display the current Offline Mode IP-address every time on startup
}


// ###########################################################################################################################################
// # Show the Offline Mode IP-address on the display:
// ###########################################################################################################################################
void ShowOfflineIPaddress() {
  if (useshowip == 1) {
    // Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.softAPIP()));
    int ipdelay = 2000;
    TextOnMatrix("IP:", redVal_time, greenVal_time, blueVal_time);  // IP:
    delay(ipdelay);
    TextOnMatrix(String(WiFi.softAPIP()[0]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 1
    delay(ipdelay);
    TextOnMatrix(String(WiFi.softAPIP()[1]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 2
    delay(ipdelay);
    TextOnMatrix(String(WiFi.softAPIP()[2]) + ".", redVal_time, greenVal_time, blueVal_time);  // Octet 3
    delay(ipdelay);
    TextOnMatrix(String(WiFi.softAPIP()[3]), redVal_time, greenVal_time, blueVal_time);  // Octet 4
    delay(ipdelay);
  }
}


// ###########################################################################################################################################
// # Split String into an Array (DayNightModeFunction)
// ###########################################################################################################################################
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// ###########################################################################################################################################
// # // PixelClock is offline during runtime! --> Restart and reconnect now!
// ###########################################################################################################################################
void WiFi_Lost() {
  Serial.print("PixelClock is offline during runtime! ");
  Serial.print(millis());
  Serial.print(" - Reconnecting to WiFi... ");
  WiFi.disconnect();
  WiFi.reconnect();
  WiFi_previousMillis = WiFi_currentMillis;
  WiFi_retry_counter = WiFi_retry_counter + 1;
  Serial.print(" Retry count: " + String(WiFi_retry_counter));

  if ((WiFi.status() != WL_CONNECTED)) {
    Serial.println(" - Reconnecting to WiFi = FAILED - Next try in " + String(WiFi_interval / 1000) + " seconds...");
  }

  if (WiFi_retry_counter >= WiFi_FlashLEDs) {
    Serial.println("PixelClock is offline during runtime for 10+ tries! --> Flash LEDs 3x now!");
    for (int i = 0; i < 3; i++) {
      updatedevice = false;
      TextOnMatrix("WIFI", 0, 0, 255);
      delay(1000);
      TextOnMatrix("WIFI", 255, 255, 0);
      delay(1000);
      TextOnMatrix("WIFI", 255, 0, 0);
      delay(1000);
    }
  }

  if (WiFi_retry_counter >= WiFi_Restart) {
    Serial.println("PixelClock is offline during runtime for 30 tries! --> Restart and reconnect now!");
    updatedevice = false;
    delay(250);
    TextOnMatrix("RESET", 0, 255, 0);
    if (changedvalues == true) setFlashValues();  // Write settings to flash
    preferences.end();
    delay(1000);
    ESP.restart();
  }
}


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################