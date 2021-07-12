#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialPro.h>

AsyncWebServer server(80);

const char* ssid = ""; // Your WiFi SSID
const char* password = ""; // Your WiFi Password


void recvMsg(uint8_t *data, size_t len){
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerialPro.printf("Received Data: %s\n", d.c_str());
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Connection Failed!\n");
      return;
  }
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* WebSerialPro is accessible at "<IP Address>/webserial" in browser */
  WebSerialPro.begin(&server);
  
  /* Attach Message Callback */
  WebSerialPro.msgCallback(recvMsg);

  /* Start Webserver */
  server.begin();
}

void loop() {
  /* Nothing needed to be done here */
}
