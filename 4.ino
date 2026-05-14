#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <AHT20.h>
AHT20 aht20;

const char* ssid = "Ruc2hxd";       // ชืÉอ WiFi
const char* password = "ruch1232";  // รหสัผา่ น WiFi
// ตังÊคา่ ตวัแปร Telegram
String telegramToken = "8676113181:AAFTRZLRmuiqbA5QYIMc9iiGY3cQBze7Bh0";
String chatID = "8782397663";                                             
String tokenab = "";                                                      
String message = "";                                                     
#define testSwitch_T 23
#define testSwitch_H 5
void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.begin();
  aht20.begin();
  Serial.println("ConnecƟng to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // ทดสอบการส่งข้อความ
  sendHttpRequest("ทดสอบการแจ้งเตือน Telegram");  // เรียกใช้ฟังก์ชนั ส่งข้อความ
  pinMode(testSwitch_T, INPUT_PULLUP);
  pinMode(testSwitch_H, INPUT_PULLUP);
}
char dataString[99];
void loop() {
  float tempp = aht20.getTemperature();
  float humid = aht20.getHumidity();
  long randNumber;
  if (digitalRead(testSwitch_H) == LOW) {
    sprintf(dataString, "now Humidity = %0.2f %%", humid);
    sendHttpRequest(dataString);
    delay(5000);
  }
  if (digitalRead(testSwitch_T) == LOW) {
    sprintf(dataString, "now Temperature = %0.2f 'C", tempp);
    sendHttpRequest(dataString);
    delay(5000);
  }
  if ((tempp < 22 && tempp > 26) || humid < 40 && humid > 60) {
    sprintf(dataString, "now Temperature = %0.2f 'C and Humidity = %0.2f %%", tempp, humid);
    sendHttpRequest(dataString);
    delay(5000);
  }
}
// ฟังก์ชนั ส่ง HTTP Request โดยรบัพารามิเตอรเ์ป็นข้อความ
void sendHttpRequest(String message) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = "https://api.telegram.org/bot8676113181:AAFTRZLRmuiqbA5QYIMc9iiGY3cQBze7Bh0/sendMessage?chat_id=" + chatID + "&text=" + urlEncode(message);

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0) {

      String payload = http.getString();

      Serial.println("Response:");
      Serial.println(payload);

    } else {

      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }

    http.end();
  }
}
// ฟังก์ชนั สาํ หรบั แปลงข้อความเป็น URL Encoding (สาํ หรบัรองรบัการใช้หลายคาํ )
String urlEncode(String message) {
  String encoded = "";
  char c;
  for (int i = 0; i < message.length(); i++) {
    c = message[i];
    if (c == ' ') {
      encoded += "%20";  // เปลีÉยนช่องว่างเป็น %20
    } else if (c == '&') {
      encoded += "%26";  // เปลีÉยนเครืÉองหมาย & เป็น %26
    } else if (c == '=') {
      encoded += "%3D";  // เปลีÉยนเครืÉองหมาย = เป็น %3D
    } else {
      encoded += c;  // ตวัอกัษรอืÉนๆ ไม่ต้องแปลง
    }
  }
  return encoded;
}