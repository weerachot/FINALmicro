#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 0;

const char* ssid = "Ruc2hxd";
const char* password = "ruch1232";

String GOOGLE_SCRIPT_ID = "AKfycbzuc1zbN8pZEWz-zdVbqZIMQTTS00SVguleD0ljYwplAhTSR2IOqKCnMvZqeSsuPWAh6A";

char timeStringBuff[50];
int nCounter = 0;

void setup() {                                          
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.print("Connecting to WiFi : ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y%m%d_%H%M%S", &timeinfo);
    String DateTime = String(timeStringBuff);
    String wifiName = WiFi.SSID();
    String macAddr = WiFi.macAddress();
    float internalTemp = temperatureRead();  // อุณหภูมิภายใน ESP32
    int rand1 = random(0, 300);
    int rand2 = random(400, 600);
    int rand3 = random(700, 1000);
    float randf1 = random(0, 50000) / 1000.0;
    float randf2 = random(50000, 100000) / 1000.0;
    String urlFinal =
      "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"
                                                                 "Item="
      + String(nCounter) + "&Date_Time=" + DateTime + "&WiFi_Name=" + wifiName + "&MAC_Address=" + macAddr + "&ESP_Temp=" + String(internalTemp) + "&intData_1=" + String(rand1) + "&intData_2=" + String(rand2) + "&intData_3=" + String(rand3) + "&floatData_1=" + String(randf1, 3) + "&floatData_2=" + String(randf2, 3);

    Serial.println("Send data to Google Sheet");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }

    http.end();
  }

  nCounter++;

  delay(60000);  // ส่งทุก 60 วินาที
}