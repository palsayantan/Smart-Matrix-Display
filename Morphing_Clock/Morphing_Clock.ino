//
// Thanks to:
// Dominic Buchstaller for PxMatrix
// Hari Wiguna aka HariFun for Morphing Digits
// Brian Lough aka WitnessMeNow for tutorials on the matrix

// ESP8266 WiFi main library
#include <ESP8266WiFi.h>

// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library

#include <PxMatrix.h>

#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

#ifdef ESP8266

#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#endif

// Pins for LED MATRIX
PxMATRIX display(32, 16, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// set Wi-Fi SSID and password
const char *ssid     = "SSID";
const char *password = "PASSWORD";

WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 19800, 60000); //GMT+5:30 : 5*3600+30*60=19800

byte _Second, _Minute, _Hour;
unsigned long _Epoch;

#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(70);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  //isplay.display(70);
  display.displayTestPattern(70);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

//=== SEGMENTS ===
#include "Digit.h"
Digit digit0(&display, 2, 32 - 0 - 5 * 1, 5, display.color565(0, 0, 255));
Digit digit1(&display, 2, 32 - 1 - 5 * 2, 5, display.color565(0, 0, 255));
Digit digit2(&display, 2, 32 - 2 - 5 * 3, 5, display.color565(0, 255, 0));
Digit digit3(&display, 2, 32 - 3 - 5 * 4, 5, display.color565(0, 255, 0));
Digit digit4(&display, 2, 32 - 4 - 5 * 5, 5, display.color565(255, 0, 0));
Digit digit5(&display, 2, 32 - 5 - 5 * 6, 5, display.color565(255, 0, 0));
//int changeSpeed = 500;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  display.begin(4);
  display.setScanPattern(ZAGZIG);

#ifdef ESP8266
  display_ticker.attach(0.002, display_updater);
#endif

#ifdef ESP32
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &display_updater, true);
  timerAlarmWrite(timer, 2000, true);
  timerAlarmEnable(timer);
#endif

  WiFi.begin(ssid, password);
  Serial.print("Connecting.");
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");
  timeClient.begin();
  delay(10);

  display.fillScreen(display.color565(0, 0, 0));
  digit1.DrawColon(display.color565(255, 255, 255));
  digit3.DrawColon(display.color565(255, 255, 255));
}


void loop() {
  if (WiFi.status() == WL_CONNECTED)  // check WiFi connection status
  {
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
    if (unix_epoch != _Epoch) {
      int Second = second(unix_epoch);      // get seconds from the UNIX Epoch time
      int Minute = minute(unix_epoch);    // get minutes (0 - 59)
      int Hour   = hour(unix_epoch);        // get hours   (0 - 23)

      if (Hour > 12) {
        Hour = Hour - 12;
      }
      else
        Hour = Hour;

      if (_Epoch == 0) { // If we didn't have a previous time. Just draw it without morphing.
        digit0.Draw(Second % 10);
        digit1.Draw(Second / 10);
        digit2.Draw(Minute % 10);
        digit3.Draw(Minute / 10);
        digit4.Draw(Hour % 10);
        if (Hour >= 10) digit5.Draw(Hour / 10);
      }
      else
      {
        // epoch changes every miliseconds, we only want to draw when digits actually change.
        if (Second != _Second) {
          digit1.DrawColon(display.color565(0, 0, 0));
          digit3.DrawColon(display.color565(0, 0, 0));
          int s0 = Second % 10;
          int s1 = Second / 10;
          if (s0 != digit0.Value()) digit0.Morph(s0);
          if (s1 != digit1.Value()) digit1.Morph(s1);
          digit1.DrawColon(display.color565(255, 255, 255));
          digit3.DrawColon(display.color565(255, 255, 255));
          _Second = Second;
        }

        if (Minute != _Minute) {
          int m0 = Minute % 10;
          int m1 = Minute / 10;
          if (m0 != digit2.Value()) digit2.Morph(m0);
          if (m1 != digit3.Value()) digit3.Morph(m1);
          _Minute = Minute;
        }

        if (Hour != _Hour) {
          int h0 = Hour % 10;
          int h1 = Hour / 10;
          if (h0 != digit4.Value()) digit4.Morph(h0);
          if (h1 > 0) {
            if (h1 != digit5.Value()) digit5.Morph(h1);
          }
          _Hour = Hour;
        }
      }
      _Epoch = unix_epoch;
    }
  }
}
