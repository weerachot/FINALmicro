#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ================= WIFI =================
const char* ssid = "Ruc2hxd";
const char* pass = "ruch1232";

// ================= TELEGRAM =================
#define BOTtoken "8676113181:AAH8oCMRZGVbyDMA0ILYbkY_f_CudnzfqyQ"
#define CHAT_ID "8782397663"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================= WEB SERVER =================
WebServer server(80);

// ================= HARDWARE =================
int led[4] = { 15, 2, 4, 5 };
int sw[2] = { 22, 23 };

// ================= STATES =================
bool ledState[4] = { 0, 0, 0, 0 };

bool swState1 = 0;
bool swState2 = 0;

bool oldSW1 = 0;
bool oldSW2 = 0;

// ================= DEBOUNCE =================
unsigned long lastChange1 = 0;
unsigned long lastChange2 = 0;

const unsigned long debounceDelay = 20;

// ================= TELEGRAM QUEUE =================
QueueHandle_t telegramQueue;

// =================================================
// UPDATE LED
// =================================================
void updateLED() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(led[i], ledState[i]);
  }
}

// =================================================
// TELEGRAM TASK
// =================================================
void telegramTask(void* parameter) {

  char msg[100];

  while (true) {

    if (xQueueReceive(telegramQueue,
                      &msg,
                      portMAX_DELAY)
        == pdTRUE) {

      bot.sendMessage(CHAT_ID,
                      String(msg),
                      "");

      Serial.println(msg);
    }
  }
}

// =================================================
// SEND TELEGRAM
// =================================================
void sendTelegram(const char* text) {

  char msg[100];

  strcpy(msg, text);

  xQueueSend(telegramQueue,
             &msg,
             0);
}

// =================================================
// WEB PAGE
// =================================================
void web() {

  String s;

  s += "<!DOCTYPE html><html><head>";

  s += "<meta charset='UTF-8'>";
  s += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

  s += "<style>";

  s += "body{font-family:Arial;text-align:center;}";

  s += "button{";
  s += "padding:15px;";
  s += "width:120px;";
  s += "font-size:18px;";
  s += "margin:10px;";
  s += "border:none;";
  s += "border-radius:10px;";
  s += "cursor:pointer;";
  s += "}";

  s += ".on{background:#4CAF50;color:white;}";
  s += ".off{background:#f44336;color:white;}";

  s += "</style>";

  // ================= JAVASCRIPT =================
  s += "<script>";

  s += "function toggleLED(id){";

  s += "fetch('/t'+id)";
  s += ".then(res=>res.text())";
  s += ".then(state=>{";

  s += "let btn=document.getElementById('btn'+id);";

  s += "btn.innerText=state;";
  s += "btn.className=(state=='ON')?'on':'off';";

  s += "});";

  s += "}";

  // realtime switch update
  s += "setInterval(function(){";

  s += "fetch('/status')";
  s += ".then(res=>res.json())";
  s += ".then(data=>{";

  s += "document.getElementById('sw1').innerText=data.s1?'ON':'OFF';";
  s += "document.getElementById('sw2').innerText=data.s2?'ON':'OFF';";

  s += "});";

  s += "},200);";

  s += "</script>";

  s += "</head><body>";

  s += "<h1>ESP32 Control Panel</h1>";

  // ================= LED =================
  for (int i = 0; i < 4; i++) {

    s += "<div>";

    s += "LED " + String(i + 1);

    s += "<br>";

    s += "<button id='btn" + String(i) + "' ";

    s += "class='";
    s += (ledState[i] ? "on" : "off");
    s += "' ";

    s += "onclick='toggleLED(" + String(i) + ")'>";

    s += (ledState[i] ? "ON" : "OFF");

    s += "</button>";

    s += "</div>";
  }

  // ================= SWITCH =================
  s += "<h2>Switch Status</h2>";

  s += "SW1 : <b id='sw1'>";
  s += (swState1 ? "ON" : "OFF");
  s += "</b>";

  s += "<br><br>";

  s += "SW2 : <b id='sw2'>";
  s += (swState2 ? "ON" : "OFF");
  s += "</b>";

  s += "</body></html>";

  server.send(200,
              "text/html",
              s);
}

// =================================================
// TOGGLE LED
// =================================================
void handleToggle(int i) {

  ledState[i] = !ledState[i];

  updateLED();

  server.send(200,
              "text/plain",
              ledState[i] ? "ON" : "OFF");
}

// =================================================
// STATUS JSON
// =================================================
void handleStatus() {

  String json;

  json = "{";

  json += "\"s1\":";
  json += String(swState1);

  json += ",\"s2\":";
  json += String(swState2);

  json += "}";

  server.send(200,
              "application/json",
              json);
}

// =================================================
// SETUP
// =================================================
void setup() {

  Serial.begin(115200);

  // ================= PINMODE =================
  for (int i = 0; i < 4; i++) {
    pinMode(led[i], OUTPUT);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(sw[i], INPUT_PULLUP);
  }

  updateLED();

  // ================= WIFI =================
  WiFi.begin(ssid, pass);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  // ================= TELEGRAM =================
  client.setInsecure();

  // ================= WEB =================
  server.on("/", web);

  server.on("/status", handleStatus);

  for (int i = 0; i < 4; i++) {

    String p = "/t" + String(i);

    server.on(p.c_str(),
              [i]() {
                handleToggle(i);
              });
  }

  server.begin();

  Serial.println("Web Started");

  // ================= QUEUE =================
  telegramQueue = xQueueCreate(10,
                               sizeof(char[100]));

  // ================= TASK =================
  xTaskCreatePinnedToCore(
    telegramTask,
    "TelegramTask",
    10000,
    NULL,
    1,
    NULL,
    0);
}

// =================================================
// LOOP
// =================================================
void loop() {

  server.handleClient();

  bool s1 = !digitalRead(sw[0]);
  bool s2 = !digitalRead(sw[1]);

  // ================= SW1 =================
  if (s1 != oldSW1 && millis() - lastChange1 > debounceDelay) {

    swState1 = s1;

    if (s1) {
      sendTelegram("SW1 : Pressed (ON)");
    } else {
      sendTelegram("SW1 : Released (OFF)");
    }

    oldSW1 = s1;

    lastChange1 = millis();
  }

  // ================= SW2 =================
  if (s2 != oldSW2 && millis() - lastChange2 > debounceDelay) {

    swState2 = s2;

    if (s2) {
      sendTelegram("SW2 : Pressed (ON)");
    } else {
      sendTelegram("SW2 : Released (OFF)");
    }

    oldSW2 = s2;

    lastChange2 = millis();
  }
}