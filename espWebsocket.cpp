#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "ESP8266_AP";
const char* password = "12345678";

WebSocketsServer webSocket = WebSocketsServer(82);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println();
  Serial.print("Access Point started.\nIP address: ");
  Serial.println(IP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket server started!");
}

void loop() {
  webSocket.loop();

  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();
    Serial.print("Sending message: ");
    Serial.println(message);
    webSocket.broadcastTXT(message);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
      webSocket.sendTXT(num, "Connected to ESP8266 WebSocket!");
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Received: %s\n", num, payload);
      break;
  }
}
