#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL5Dlajt5"
#define BLYNK_TEMPLATE_NAME "lab4"
#define BLYNK_AUTH_TOKEN "NgR4wTtCoIE9qTfpl-dLsi9q5fyrHBM0"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <AHT20.h>
AHT20 aht20;
char ssid[] = "Ruc2hxd";
char pass[] = "ruch1232";
#define LED_22 4
#define LED_23 23
#define SWitch_18 18
#define SWitch_19 19
BlynkTimer timer;
bool blynk_led0 = 0;
bool blynk_led1 = 0;
char sBuffer[100];

BLYNK_WRITE(V0) {
  blynk_led0 = param.asInt();
}

BLYNK_WRITE(V1) {
  blynk_led1 = param.asInt();
}

void updateLED() {
  int sw0 = digitalRead(SWitch_18);
  int sw1 = digitalRead(SWitch_19);

  bool led0 = blynk_led0 || !sw0;
  bool led1 = blynk_led1 || !sw1;

  digitalWrite(LED_22, led0);
  digitalWrite(LED_23, led1);

  Blynk.virtualWrite(V2, !sw0);
  Blynk.virtualWrite(V3, !sw1);

  Blynk.virtualWrite(V0, led0);
  Blynk.virtualWrite(V1, led1);
}

void readSensor() {
  int value18 = digitalRead(SWitch_18);
  int value19 = digitalRead(SWitch_19);
  float Tempp = aht20.getTemperature();
  float Humid = aht20.getHumidity();
  Blynk.virtualWrite(V4, Tempp);
  Blynk.virtualWrite(V5, Humid);
  sprintf(sBuffer, "Data: S=%d,%d, T=%0.2f, H=%0.2f",value18, value19, Tempp, Humid);
  Serial.println(sBuffer);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  aht20.begin();
  pinMode(LED_22, OUTPUT);
  pinMode(LED_23, OUTPUT);
  pinMode(SWitch_18, INPUT_PULLUP);
  pinMode(SWitch_19, INPUT_PULLUP);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, readSensor);
}

void loop() {
  Blynk.run();
  timer.run();
  updateLED();
}