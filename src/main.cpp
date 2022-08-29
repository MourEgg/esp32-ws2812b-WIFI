
#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <main.h>
#include <htmlCode.h>
#include <esp_wifi.h>
#include <esp32-hal-cpu.h>
#include <Preferences.h>
#include "time.h"


#define DATA_PIN  22
#define NUM_LEDS  180
#define LED_TYPE  WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define VOLTAGE 5
#define MAX_MILIAMPS 5000
#define BRIGHTNESS  96
#define FRAMES_PER_SECOND 120
uint8_t gHue = 0;
String ledTheme;
bool ledState = true;
uint32_t solidColor;
uint8_t brightness = 255;
uint8_t speed = 50;
String alarmTime = "0";
uint32_t alarmOffsetTime = 0;
bool alarmWeekdays = true;
bool alarmDisableOnce = false;

// wifi credentials
const char* ssid = "skacel.jmnet.cz";
const char* password = "kiov-vracov*";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 69);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 4, 1);

unsigned long previousMillis = 0;
unsigned long interval = 60001;
bool intervalPassed = false;

unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 300;
// Set web server port number to 80
WiFiServer server(80);

Preferences preferences;

struct tm timeinfo;
bool fakeSunActive = false;

String header;

#define ONBOARD_LED  2


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  pinMode(ONBOARD_LED,OUTPUT);

  getPreferences();

  setupWifi();
 
  getTime();

   // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTAGE, MAX_MILIAMPS);
  //FastLED.setBrightness(BRIGHTNESS);


  
}

void loop(){
  intervalCounter();
  reconnectWifi();
  handleLeds();
  handleServer();
  handleFakeSun();
  intervalPassed = false;
}

void getPreferences() {
  preferences.begin("ledControl", false);
  alarmTime = preferences.getString("alarmTime", "");
  alarmOffsetTime = preferences.getUInt("alarmOffsetTime");
  alarmWeekdays = preferences.getBool("alarmWeekdays");
  preferences.end();
  Serial.println("-----------ters");
  Serial.println(alarmWeekdays);
}

void setupWifi(){
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  digitalWrite(ONBOARD_LED, HIGH);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(ONBOARD_LED, LOW);
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
}

void getTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
}

void reconnectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ONBOARD_LED, HIGH);
    if (intervalPassed){
      Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
  else {
    digitalWrite(ONBOARD_LED, LOW);
  }
}

void handleServer(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,  
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
    currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.print("CurrentTime: ");
            timeinfo.tm_hour;
            client.print(timeinfo.tm_hour);
            client.print(":");
            client.println(timeinfo.tm_min);
            client.print("AlarmTime: ");
            client.println(alarmTime);
            client.print("OffsetTime: ");
            client.println(alarmOffsetTime);
            client.print("WeekdaysAlarm: ");
            client.println(alarmWeekdays);
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /sinelon") >= 0) {
              ledTheme = "sinelon";
              ledState = true;
            }
            if (header.indexOf("GET /rainbow") >= 0) {
              ledTheme = "rainbow";
              ledState = true;
            }
            if (header.indexOf("GET /brightness") >= 0) {
              brightness = header.substring(16, 19).toInt();
              FastLED.setBrightness(brightness);
              ledState = true;
            }
            if (header.indexOf("GET /speed") >= 0) {
              speed = header.substring(11, 14).toInt();
            }
            if (header.indexOf("GET /color") >= 0) {
              String hexColor = "#";
              hexColor += header.substring(11,17);
              solidColor = strtol( &hexColor[1], NULL, 16);
              ledTheme = "solid";
              ledState = true;
            }
            if (header.indexOf("GET /white") >= 0) {
              ledTheme = "solid";
              solidColor = 0xffbe24;
              ledState = true;
            }
            if (header.indexOf("GET /off") >= 0) {
              ledState = !ledState;
            }
            if (header.indexOf("GET /disable-once") >= 0) {
              alarmDisableOnce = true;
            }
            if (header.indexOf("GET /time") >= 0) {
              setAlarmTime(header.substring(10, 15));
            }
            if (header.indexOf("GET /offset") >= 0) {
              setAlarmOffsetTime(header.substring(12,14).toInt());
            }
            if (header.indexOf("GET /weekdays") >= 0) {
              setAlarmWeekdays(header.substring(14,15).toInt());
            }
            if (header.indexOf("GET / HTTP") >= 0) {
              client.println(htmlCode);
            }
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  else {
    client.stop();
  }

}

void setAlarmTime(String time) {
  preferences.begin("ledControl", false);
  alarmTime = time;
  preferences.putString("alarmTime", time);
  preferences.end();
}

void setAlarmOffsetTime(uint32_t time) {
  preferences.begin("ledControl", false);
  alarmOffsetTime = time;
  preferences.putUInt("alarmOffsetTime", time);
  preferences.end();
}

void setAlarmWeekdays(uint8_t weekdays) {
  preferences.begin("ledControl", false);
  bool week = false;
  if (weekdays == 1) {
    week = true;
  }
  preferences.putBool("alarmWeekdays", week);
  preferences.end();

}

void handleFakeSun() {
  if (alarmOffsetTime != 0) {
    String currTime;
    if(intervalPassed) {
      getTime();
      if (alarmWeekdays && (timeinfo.tm_wday == 6 || timeinfo.tm_wday == 0)){
        return;
      }

      if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
      }
      uint8_t hours = timeinfo.tm_hour;
      uint8_t minutes = timeinfo.tm_min;

      uint8_t ahour = alarmTime.substring(0,2).toInt();
      uint8_t amin = alarmTime.substring(3).toInt();

      if(alarmOffsetTime > amin) {
        ahour--;
        amin += 60;
      }
      amin -= alarmOffsetTime;
      String atime = ahour + (String)":" + amin;
      currTime = hours + (String)":" + minutes;
      if (currTime == atime) {
        if(alarmDisableOnce) {
          alarmDisableOnce = false;
          return;
        }
        fakeSunActive = true;
        ledTheme = "solid";
        solidColor = 0xffbe24;
        ledState = true;
        brightness = 0;
        FastLED.setBrightness(brightness);
      }
    }
    if(fakeSunActive) {
      if(FastLED.getBrightness() < 255){
        EVERY_N_SECONDS((alarmOffsetTime * 60) / 255){
          Serial.println(brightness);
          brightness++;
          FastLED.setBrightness(brightness);
        }
      }
      uint8_t ohour = alarmTime.substring(0,2).toInt();
      uint8_t omin = alarmTime.substring(3).toInt();
      if(omin + 30 >= 60) {
        ohour++;
        omin -= 60;
      }
      omin += 30;
      String atime = ohour + (String)":" + omin;
      if (currTime == atime) {
        ledState = false;
      }
    }
  }
}

void handleLeds() {
  if (ledState) {
    if (ledTheme == "sinelon") {
      sinelon();
    }
    else if (ledTheme == "solid") {
      parseColor(solidColor);
    }
    else if (ledTheme == "white") {
      return;
    }
    else if (ledTheme == "rainbow") {
      fill_rainbow( leds, NUM_LEDS, gHue, 1);
      FastLED.show();
      EVERY_N_MILLISECONDS_I( timingObj, 20 ) {
        timingObj.setPeriod(speed);
        gHue++;  // slowly cycle the "base color" through the rainbow
      }
    }
    else {
      fill_solid( leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
  else {
    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}

void parseColor(uint32_t color) {

  fill_solid( leds, NUM_LEDS, CRGB(color));
  FastLED.show();
}

void sinelon()
{

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 25);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
}

void intervalCounter() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >=interval){
    previousMillis = currentMillis;
    intervalPassed = true;
    return;
  }
}
