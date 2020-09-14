#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Settings.h"

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

const char *ssid = storedSSID;
const char *password = storedPASSWORD;
IPAddress ip;
HTTPClient http;
String url = String(storedURL);
int errCount = 0;

void display(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  alpha4.writeDigitAscii(0, a);
  alpha4.writeDigitAscii(1, b);
  alpha4.writeDigitAscii(2, c);
  alpha4.writeDigitAscii(3, d);
  alpha4.writeDisplay();
}

void displayC(const char *s) {
  for (int i = 0; i < 4; i++) {
    if (s[i] == 0x00) {
      alpha4.writeDigitAscii(i, ' ');
    } else {
      alpha4.writeDigitAscii(i, s[i]); 
    }
  }
  alpha4.writeDisplay();
}

void setup()
{
  Serial.begin(115200);
  Serial.print("START");
  alpha4.begin(0x70);
  alpha4.setBrightness(brightness);


  //////////////////////
  // WIFI CONNECTION
  //////////////////////
  display('C', 'O', 'N', 'N');
  WiFi.begin(ssid, password);

  int wifiRetry = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    wifiRetry++;

    if (wifiRetry > 100) {
      ESP.restart();
    }
  }

  //////////////////////
  // PRINT IP CONNECTION
  //////////////////////
  display('W', 'I', 'F', 'I');
  ip = WiFi.localIP();

  char fip[3];
  itoa(ip[3], fip, 10);
  char veb[4] = {'-', fip[0], fip[1], fip[2]};
  displayC(veb);

  delay(1000);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      const size_t capacity = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(5) + 940;
      DynamicJsonDocument doc(capacity);

      deserializeJson(doc, http.getString());

      JsonObject spotify = doc["spotify"];
      const char *spotify_format = spotify["format"]; // "OGG"
      const char *spotify_state = spotify["state"];   // "paused"

      JsonObject swinsian = doc["swinsian"];
      const char *swinsian_format = swinsian["format"]; // "MP3"
      const char *swinsian_state = swinsian["state"];   // "playing"

      if (strcmp(swinsian_state, "playing")==0) {
        displayC(swinsian_format);
      } else if (strcmp(spotify_state, "playing")==0) {
        displayC(spotify_format);
      } else {
        display(' ', ' ', ' ', ' ');
      }
      errCount = 0;
    } else {
      errCount++;
    }
    http.end();
  }
  else
  {
    display('E', 'N', 'E', 'T');
  }

  if (errCount > 10) {
    display('E', 'R', 'R', 'D');
  }

  delay(500);
}
