#include <TM1638plus.h>
#include <WiFi.h>
#include <WebServer.h>

#define STB 18
#define CLK 19
#define DIO 21

const char* ssid = "Ruc2hxd";
const char* pass = "ruch1232";

WebServer server(80);
TM1638plus tm(STB, CLK, DIO, true);

bool led[8];

void showLED() {
  for (int i = 0; i < 8; i++)
    tm.setLED(i, led[i]);
}

void web() {
  String s = "<meta http-equiv='refresh' content='1'><h1>TM1638</h1>";
  for (int i = 0; i < 8; i++) {
    s += "LED" + String(i + 1);
    s += " <a href='/t" + String(i) + "'>";
    s += "<button>";
    s += led[i] ? "ON" : "OFF";
    s += "</button></a><br><br>";
  }
  server.send(200, "text/html", s);
}

void toggle(int i) {
  led[i] = !led[i];
  showLED();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  tm.displayBegin();
  tm.reset();
  tm.brightness(7);

  // แสดงสถานะ connecting บน Serial
  Serial.print("\nConnecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // แสดง IP และ URL ใน Serial
  Serial.println("\n------------------------------");
  Serial.print("Connected!  IP : ");
  Serial.println(WiFi.localIP());
  Serial.print("Open browser  : http://");
  Serial.println(WiFi.localIP());
  Serial.println("------------------------------");

  server.on("/", web);
  for (int i = 0; i < 8; i++) {
    String p = "/t" + String(i);
    server.on(p.c_str(), [i]() { toggle(i); });
  }

  server.begin();
  Serial.println("Server started.");

  showLED();
}

void loop() {
  server.handleClient();

  byte b = tm.readButtons();
  for (int i = 0; i < 8; i++) {
    if (b & (1 << i)) {
      led[i] = !led[i];
      showLED();
      Serial.print("Button ");
      Serial.print(i + 1);
      Serial.print(" → LED");
      Serial.print(i + 1);
      Serial.println(led[i] ? " ON" : " OFF");
      delay(250);
    }
  }
}
