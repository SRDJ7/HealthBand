/* 
  TEAM ENIGMA (MuleSoft Hackathon)
  Health Band v3.2 Firmware
=======================================================================================================
  Firmware Author: J DHANUSH AKA SRDJ7
=======================================================================================================
  -------TEAM ENIGMA:-----------------
  BHARATDEEP HAZARIKA
  J DHANUSH
  LOKESH BISWAS M
  KARTHIK S
  ------------------------------------
  Author Notes (As of on 04-04-2022) : # The code can be further optimized to enhance the performance of the Health Band device.
 */

#include <Wire.h>
#include <SPI.h>
#include "NTPClient.h"
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>  // ESP8266 WiFi MODULE LIBRARY
#include <Fonts/FreeSerifBoldItalic9pt7b.h> // Used in BootUp
#include <Fonts/FreeSansBold9pt7b.h>  // Used in BootUp
#include <Fonts/FreeSansBoldOblique12pt7b.h> // Used in BootUp
#include <Fonts/FreeMonoBold9pt7b.h> // Used for Showing Date-Time
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h> //Library used to prepare and parse JSONs 

// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "gpio.h"
}
extern "C" {
#include "user_interface.h"
}


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SECRET_SSID "AndroidAP9147"    // replace MySSID with your WiFi network name
#define SECRET_PASS "samsunga5071"  // replace MyPassword with your WiFi password

#define LOGO_HEIGHT   16    //Heart Logo Height
#define LOGO_WIDTH    16   //Heart Logo Width


//#define Button2 16 // Button2 is linked to GPIO 16 i.e D0 pin // Commented this as it may cause ESP Chip to wake up
#define Button1 14 // Button1 is linked to GPIO 14 i.e D5 pin
#define Button2 12 // Button2 is linked to GPIO 12 i.e D6 pin
#define Button3 D7 // Button3 is linked to GPIO 13 i.e D7 pin

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Declaration for MLX90614 Thermopile Sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
int i = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 19800,60000);

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
String date_time; // You can specify the time server pool and the offset (in seconds, can be // changed later with setTimeOffset() ). Additionaly you can specify the // update interval (in milliseconds, can be changed using setUpdateInterval() ).

//Mule App URL (for Weather Service)
const char* serverName1 = "http://weatherinfo.us-e2.cloudhub.io/weatherInfo/12.9394/77.6952";

String weatherUpdates;
float weatherReadingsArr[3];
float maxtemp=0, mintemp=0, humidity=0;
String clouds="";
unsigned long lastTimeW=0;
unsigned long timerDelayW=60000; // The time interval of 60 Secs after which Weather Data gets updated.
bool newWeather=true;

// Mule API URL (for Health Parameters Updates
const char* serverName2="http://health-info.us-e2.cloudhub.io";
String healthUpdate;
unsigned long lastTimeH=0;
unsigned long timerDelayH=60000; // The time interval of 60 Secs after which Health Data is updated via the API

// Mule API URL (for getting Notice)

const char* serverName3="http://health-info.us-e2.cloudhub.io/notice/1NH19EC010";
String notice="";
unsigned long lastTimeN=0;
unsigned long timerDelayN=60000; // The time interval of 1 min (60 secs) for which the notice is being fetched by the device

// Initializing Global Variables
long int entry = 0;
int heartrate=77;
float  spo2=99,gluco=0,temperatureF = 88,temperatureC = 0,atempF = 0;
int switchState1=0, oldSwitchState1=1,toggle1=0;
int switchState2=0, oldSwitchState2=1,toggle2=0;
int switchState3=0, oldSwitchState3=1,toggle3=0;
bool healthBool=true;

int batteryLvl=100; 
 //This is an bitmap array for the heart logo/animation
const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

 // This is a bitmap array for the WiFi Symbol
const unsigned char wifiSymbol [512] PROGMEM = {
  // 'WiFi Symbol, 59x64px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xff, 0x00, 0x1f, 0xff, 0xe0, 0x00, 
  0x03, 0xff, 0xf0, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x1f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 
  0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x7f, 0x80, 0x00, 0xff, 0xe0, 0x00, 0x3f, 0x80, 
  0x3f, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x1f, 0x00, 0x1e, 0x00, 0x3f, 0xff, 0xff, 0x80, 0x0e, 0x00, 
  0x0c, 0x00, 0xff, 0xff, 0xff, 0xe0, 0x06, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x1f, 0xfe, 0x00, 0x00, 
  0x00, 0x3f, 0xf8, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0xff, 0x80, 0x00, 
  0x00, 0x3f, 0x80, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 
  0x00, 0x0c, 0x00, 0x0e, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0xe0, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x0c, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 
  0x3f, 0xff, 0xff, 0x9f, 0x3f, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xfc, 0xe7, 0x27, 0xf8, 0x67, 0xff, 0xc0, 
  0x7f, 0xfc, 0xe7, 0x27, 0xf8, 0x27, 0xff, 0xc0, 0x7f, 0xfc, 0xc3, 0x7f, 0xf9, 0xff, 0xff, 0xc0, 
  0x7f, 0xfc, 0xc2, 0x67, 0xfb, 0xe7, 0xff, 0xc0, 0x7f, 0xfe, 0x52, 0x67, 0xf8, 0x67, 0xff, 0xc0, 
  0x7f, 0xfe, 0x52, 0x66, 0x38, 0x67, 0xff, 0xc0, 0x7f, 0xfe, 0x18, 0xe6, 0x3b, 0xe7, 0xff, 0xc0, 
  0x7f, 0xfe, 0x18, 0xe7, 0xf9, 0xe7, 0xff, 0xc0, 0x7f, 0xff, 0x38, 0xe7, 0xf9, 0xe7, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xff, 0xfb, 0xf7, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

const unsigned char weatherIcon [576] PROGMEM = {
  // 'Weather icon2 rs, 66x64px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x06, 
  0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x0e, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0c, 0x06, 0x06, 0x0c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x03, 0x06, 0x0c, 
  0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0x02, 0x1c, 0x07, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 
  0x00, 0x08, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xe0, 0x7f, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0xff, 0xf8, 0x70, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x23, 0xc0, 0x1e, 0x20, 0x07, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00, 0x07, 0x00, 
  0x1e, 0x00, 0x00, 0x07, 0xc0, 0x0e, 0x03, 0x83, 0x80, 0x7c, 0x00, 0x00, 0x01, 0xf0, 0x18, 0x00, 
  0xe1, 0xc3, 0xe0, 0x00, 0x00, 0x00, 0x3e, 0x38, 0x00, 0x10, 0xc7, 0x80, 0x00, 0x00, 0x00, 0x0e, 
  0x30, 0x00, 0x08, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x04, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x60, 0x00, 0x02, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x02, 0x30, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x02, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x01, 
  0x30, 0x00, 0x00, 0x00, 0x07, 0xfc, 0xc0, 0x00, 0x01, 0x33, 0xff, 0x00, 0x00, 0x07, 0xfc, 0xc0, 
  0x00, 0x00, 0x31, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x60, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x3c, 0x00, 0x00, 0x67, 
  0x00, 0x00, 0x00, 0x03, 0x80, 0x0e, 0x00, 0x00, 0xc7, 0xc0, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00, 
  0x01, 0xc0, 0xf8, 0x00, 0x00, 0x0e, 0x00, 0x03, 0xfe, 0x03, 0x80, 0x3e, 0x00, 0x00, 0x0c, 0x00, 
  0x01, 0xff, 0x87, 0x00, 0x07, 0x00, 0x00, 0x18, 0x00, 0x00, 0x01, 0xde, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x00, 0x00, 0x00, 0xfc, 0x70, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x70, 0x30, 0x00, 
  0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x1c, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 
  0x0e, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x06, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 
  0x00, 0x3f, 0xe3, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x3f, 0xf1, 0x00, 0x00, 0x1c, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 
  0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 
  0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x03, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x60, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x60, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 
  0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x1c, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char batterySymbol [126] PROGMEM = {
  // 'Battery Symbol Hollow, 44x21px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 
  0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x03, 0xc0, 
  0x10, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x10, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x10, 0x00, 0x00, 0x00, 
  0x03, 0xc0, 0x10, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x10, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x10, 0x00, 
  0x00, 0x00, 0x03, 0xc0, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 
  0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0xff, 0xff, 0xff, 
  0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void onBeatDetected()
{
  Serial.println("Beat!");
  display.clearDisplay();
  display.drawBitmap( 50, 20, bitmap, 28, 28, 1);
  display.display();
  delay(500);
}

void callback() {
  Serial1.println("Callback");
  Serial.flush();
}

void displayData()
{
  // Write code to get the sensor readings from sensors
    //mlx.begin();
   // delay(250);
  //temperatureF = mlx.readObjectTempF();// Gets Object's temp readings in F....have to troubleshoot this
    temperatureF=random(89,100); // using random function to generate random values between the given min and max values. 
    heartrate=random(70,95); // using random function to generate random values between the given min and max values.
    spo2=random(94,100); // using random function to generate random values between the given min and max values.
    atempF=random(80,100); // using random function to generate random values between the given min and max values.
  Serial.println(" Reading temperatureF:"+String(temperatureF));
  if(((millis()-lastTimeH>timerDelayH)&(WiFi.status()==WL_CONNECTED))|healthBool){
    //Write code here to POST the data to the server
    WiFiClient client;
    HTTPClient http;
    healthUpdate = serverName2+String("/empUpdate?empID=1NH19EC010")+String("&bpm=")+String(heartrate)+String("&temp=")+String(temperatureF)+String("&bo=")+String(spo2);
    Serial.print(healthUpdate);
    http.begin(client,healthUpdate);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    // Specify content-type header
    Serial.println(healthUpdate);
    auto httpResponseCode=http.POST(healthUpdate); // was int
    Serial.print("HTTP Response Code: "); 
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    http.end(); // Free resources
    notice=httpGETRequest(serverName3);
    Serial.println(notice);
    JSONVar myObject = JSON.parse(notice);
  
       //JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);

      // myObject.keys() can be used to get an array of all the keys in the object
     JSONVar keys = myObject.keys();
    notice =JSON.stringify(myObject[keys[0]]);
    Serial.println(notice);
    healthBool=false;
    lastTimeH=millis(); 
  }
  display.stopscroll();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.print("Heart BPM | A.T");
  
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 14);
  display.println(String(heartrate)+"        |"+String(atempF)+"F");

  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 28);
  display.println("SpO2% | B.T");
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,43);
 display.print(String(spo2)+"%");
 display.print(" |"+String(temperatureF)+"F");
 
 display.setTextSize(1);
 display.setTextColor(WHITE);
 display.setCursor(0,57);
 //notice="Jai Balayya"; // Testing Notice with hardcoded string
 display.print("MSG:"+notice+"|");
 display.display();
 display.startscrollleft(0x0F,0x0F); // scroll the notice
 
  //Serial.println(" Reading temperatureF:"+String(temperatureF));
  
  delay(4000);
  
  
}

String httpGETRequest(const char* serverName) { // was serverName1
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void displayWeather()
{ 

if(WiFi.status()== WL_CONNECTED){
              
      if((millis()-lastTimeW>timerDelayW) | newWeather){
        
      weatherUpdates = httpGETRequest(serverName1);
      Serial.println(weatherUpdates);
      JSONVar myObject = JSON.parse(weatherUpdates);
  
       //JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);

      // myObject.keys() can be used to get an array of all the keys in the object
     JSONVar keys = myObject.keys();
    
      for (int i = 0; i < keys.length()-1; i++) {
        JSONVar value = myObject[keys[i]];
        weatherReadingsArr[i] = double(value);
      }

       clouds =JSON.stringify(myObject[keys[3]]);
      int cLength = clouds.length();
       clouds.remove(0,2);
       clouds.remove(cLength-4,2);
     
 
       maxtemp = weatherReadingsArr[0];
       mintemp = weatherReadingsArr[1];
       humidity = weatherReadingsArr[2];
     
   
    Serial.println("Maxtemp:"+String(maxtemp));
    Serial.println("Mintemp:"+String(mintemp));
    Serial.println("Humidity:"+String(humidity));
    lastTimeW=millis();
       }
       // Clear the buffer.
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0,0);
      display.print("Max Temp  | Min Tenp");
  
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 16);
      display.println(String(maxtemp)+"C"+"    |"+String(mintemp)+"C");

      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 30);
      display.println("Humidity% | Clouds");
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0,45);
      display.print(String(humidity)+"%");
      display.print(" |"+clouds);
      display.display();
      delay(1000);
      
      newWeather=false;
    }
    
    else {
      Serial.println("WiFi Disconnected");
    }
}



void displayTime()
{
  timeClient.update();
display.clearDisplay();
//Serial.println(timeClient.getFormattedTime());
date_time = timeClient.getFormattedTime();
 time_t epochTime = timeClient.getEpochTime(); // was unsigned long
//Get a time structure
 struct tm *ptm = gmtime ((time_t *)&epochTime);
display.setTextSize(1); 
//display.setFont(&FreeMonoBold9pt7b);
//display.setFont(&FreeSerif9pt7b);
display.setTextColor(WHITE);
display.setCursor(0,0);

int hh = timeClient.getHours();
int mm = timeClient.getMinutes();
int ss = timeClient.getSeconds();
String weekDay = weekDays[timeClient.getDay()];
int monthDay = ptm->tm_mday;
int currentMonth = ptm->tm_mon+1;
String currentMonthName = months[currentMonth-1];
int currentYear = ptm->tm_year+1900;
String currentDate = String(weekDay) + " " + String(currentMonthName) + " " + String(monthDay) + " " + String(currentYear);

display.println(currentDate);
display.setCursor(32,15);
if(hh>12)
{
hh=hh-12;
display.print(hh);
display.print(":");
display.print(mm);
display.print(":");
display.print(ss);
display.println(" PM");
}

else
{
display.print(hh);
display.print(":");
display.print(mm);
display.print(":");
display.print(ss);
display.println(" AM");
}

display.setCursor(0,30);
display.drawLine(0, 28, 128, 28, WHITE);
display.drawBitmap(0,38,batterySymbol,44,21,1);
display.setCursor(9,45); // Places the cursor inside the hollow battery symbol
display.print(batteryLvl);
display.println("%");
display.display();
}

void buttonEvent1()
{
  while(true)
  {
  
  displayData(); // Calls the function to display Health Parameters
    switchState1=digitalRead(Button1);// Read the Push Button State of Button1
  if(switchState1!=oldSwitchState1)
   {
    oldSwitchState1=switchState1;
    if(switchState1==LOW)
    { // toggle the value in toggle1 variable
      toggle1=!toggle1;
      if(!toggle1){
       display.stopscroll();
       display.clearDisplay();
      break;
      }
    }
   }
  }
}


void buttonEvent2()
{
  while(true)
  {  
  displayWeather(); // Calls the function to display Health Parameters
  //delay(1500); // Delay to display the Health  Parameters on screen for 1.5 secs
    switchState2=digitalRead(Button2);// Read the Push Button State of Button2
  if(switchState2!=oldSwitchState2)
   {
    oldSwitchState2=switchState2;
    if(switchState2==LOW)
    { // toggle the value in toggle1 variable
      toggle2=!toggle2;
      if(!toggle2){
      break;
      }
    }
   }
  }
}


void sleepNow() {
  Serial.println("going to light sleep...");
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); //light sleep mode
  gpio_pin_wakeup_enable(GPIO_ID_PIN(D7), GPIO_PIN_INTR_LOLEVEL); //set the interrupt to look for LOW pulses on Pin 0 (the PIR).
  wifi_fpm_open();
  delay(1000);
  wifi_fpm_set_wakeup_cb(wakeupFromMotion); //wakeup callback
  wifi_fpm_do_sleep(0xFFFFFFF);
  delay(1000);
}

void wakeupFromMotion(void) {
  wifi_set_sleep_type(NONE_SLEEP_T);
  gpio_pin_wakeup_disable();
  wifi_fpm_close();
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();
  Serial.println("Woke up from sleep");
}


void setup() {
  Serial.begin(9600);

// Uncomment the Multi-line comment below once the MLX90614 sensor is connected to the hardware 
  while (!Serial);

  //Serial.println("Adafruit MLX90614 test");

  /*if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  }; */

  //Serial.print("Emissivity = "); Serial.println(mlx.readEmissivity());
  //Serial.println("================================================");
  

  Serial.print("initializing GPIO D7 for Sleep Mode");
  gpio_init();
  pinMode(D7, INPUT_PULLUP); // this is the Sleep/Wake up pin.
WiFi.mode(WIFI_STA);
  timeClient.begin();
  gpio_init();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000); // Pause for 1 seconds
     
  // Clear the buffer
  display.clearDisplay();

 // initialize with the I2C addr 0x3C
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
 pinMode(Button1,INPUT_PULLUP); 
 pinMode(Button2,INPUT_PULLUP);
 
  // Clear the buffer.
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,36);
  display.setTextSize(1);
  display.setFont(&FreeSansBoldOblique12pt7b);
  display.println("ENIGMA");
  display.println("");
  display.display();
  display.startscrollright(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);

 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(1);
 display.setFont(&FreeSansBold9pt7b);
 display.setCursor(15,32);
//display.print("\n");
 display.print("Health \n    Band");// 2 Spaces before Health and 3 before Band
 display.drawBitmap( 87,22, bitmap, 28, 28, 1);
 display.display();
 delay(2000);
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    while (WiFi.status() != WL_CONNECTED) {
      //WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network // This was shifted to inside the while loop
      Serial.print(".");
      delay(1000);
    }
   display.clearDisplay();
   display.drawBitmap(32, 0, wifiSymbol, 59, 64, 1);
   display.display();
    delay(1000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(25,25);
    display.setTextColor(WHITE);
    display.setFont(&FreeSerifBoldItalic9pt7b);
    display.print("Connected!");
    //display.setTextSize(1);
    display.setCursor(0,52);
    //display.setFont(&FreeMonoBold9pt7b);
    display.print(SECRET_SSID);
    display.setFont();
    display.display();
    delay(1000);
  }
  
// loop() is a function in which the code inside the curly bracket runs over and over as long as the maker board is on.
//WiFi.Status() returns the connection status,  WL_CONNECTED is assigned when connected to a Wi-Fi network.
//Serial.print prints data to the serial porta as human readable ASCII text.
//WIFI.begin(SSID,PASS) initializes the WiFi library’s network settings and provides the current status.
//[Here ESP module connects to wifi network.] 
 
// Clear the buffer.
 display.clearDisplay();
  //Uncomment the below lines once the MLX90614 sensor is connected.
  //Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
  //Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  //Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF());
  //Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");

  displayTime(); // Calls the function to display Date-Time
  switchState1=digitalRead(Button1);// Read the Push Button State of Button1
  if(switchState1!=oldSwitchState1)
  {
    oldSwitchState1=switchState1;
    if(switchState1==LOW)
    {
      // toggle the value in toggle variable
      toggle1=!toggle1;
      if(toggle1)
      {
        onBeatDetected();// Test the Heart Logo
        buttonEvent1();
      }
    }
  }

    switchState2=digitalRead(Button2);// Read the Push Button State of Button2
  if(switchState2!=oldSwitchState2)
  {
    oldSwitchState2=switchState2;
    if(switchState2==LOW)
    {
      // toggle the value in toggle variable
      toggle2=!toggle2;
      if(toggle2)
      { //write code here to display weather icon
        display.clearDisplay();
        display.drawBitmap(30,0, weatherIcon, 66, 64, 1); 
        display.display();
        delay(500);
        buttonEvent2();
      }
    }
  }
     switchState3=digitalRead(Button3);// Read the Push Button State of Button3
  if(switchState3!=oldSwitchState3)
  {
    oldSwitchState3=switchState3;
    if(switchState3==LOW)
    {
      // toggle the value in togge1 variable
      toggle3=!toggle3;
      if(toggle3)
      { 
      Serial.println("Ready to go into light sleep...");
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(30,32);
      display.print("Sleep...");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.display();
      delay(1000);
      Serial.println("3...");
      delay(1000);
      Serial.println("2...");
      delay(1000);
       Serial.println("1...");
       sleepNow();
      }
    }
  }
  // temperatureC = mlx.readObjectTempC(); // Gets Object's temp readings in C
   
  // atempF =  mlx.readAmbientTempF(); // Gets Ambient temp readings in C

// The following code is to get the sensor readings as BPM and SPO2, displayed on the OLED screen


}
  
