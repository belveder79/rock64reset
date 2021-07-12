#ifndef WebSerialPro_h
#define WebSerialPro_h

#include "Arduino.h"
#include "stdio.h"
#include "stdarg.h"
#include <functional>

#if defined(ESP8266)
#include "stdlib_noniso.h"
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#endif

#include "webserial_webpage.h"

#define DEBUG_ENABLED 0

#if DEBUG_ENABLED == 1
#define DEBUG_WEBSERIAL(x) Serial.println("[WebSerialPro] " + String(x))
#else
#define DEBUG_WEBSERIAL(x)
#endif

#define WEBSERIAL_PRINTF_BUFFER_SIZE 512

typedef std::function<void(uint8_t * data, size_t len)> RecvMsgHandler;

class WebSerialProClass {
  public:
    void setID(const char* id){
      identity = id;
    }

    void begin(AsyncWebServer * server, const char * url = "/webserial") {

      if(identity == ""){
        #if defined(ESP8266)
          identity = String(ESP.getChipId(), HEX);
          identity.toUpperCase();
          identity = "ESP8266-"+identity;
        #elif defined(ESP32)
          uint32_t chipId = 0;
          for(int i=0; i<17; i=i+8) {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
          }
          identity = "ESP32-"+String(chipId);
        #endif
      }

      _server = server;
      _ws = new AsyncWebSocket("/webserialws");

      _server->on(url, HTTP_GET, [](AsyncWebServerRequest * request) {
        // Send Webpage
        AsyncWebServerResponse * response = request->beginResponse_P(200, "text/html", WEBSERIAL_HTML, sizeof(WEBSERIAL_HTML));
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      });

      _server->on("/webserial-id", HTTP_GET, [&](AsyncWebServerRequest * request) {
        request->send(200, "application/json", "{\"identity\": \""+identity+"\"}");
      });

      _ws->onEvent([ & ](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len) -> void {
        if (type == WS_EVT_CONNECT) {
          DEBUG_WEBSERIAL("Client connection received");
        } else if (type == WS_EVT_DISCONNECT) {
          DEBUG_WEBSERIAL("Client disconnected");
        } else if (type == WS_EVT_DATA) {
          DEBUG_WEBSERIAL("Received Websocket Data");
          if (_RecvFunc != NULL) {
            _RecvFunc(data, len);
          }
        }
      });

      _server->addHandler(_ws);
      DEBUG_WEBSERIAL("Attached AsyncWebServer along with Websockets");
    }

    void msgCallback(RecvMsgHandler _recv) {
      _RecvFunc = _recv;
    }

    // Print

    void print(String m = "") {
      _ws->textAll(m);
    }

    void print(const char * m) {
      _ws->textAll(m);
    }

    void print(char * m) {
      _ws->textAll(m);
    }

    void print(int m) {
      _ws->textAll(String(m));
    }

    void print(uint8_t m) {
      _ws->textAll(String(m));
    }

    void print(uint16_t m) {
      _ws->textAll(String(m));
    }

    void print(uint32_t m) {
      _ws->textAll(String(m));
    }

    void print(double m) {
      _ws->textAll(String(m));
    }

    void print(float m) {
      _ws->textAll(String(m));
    }

    // Print with New Line

    void println(String m = "") {
      _ws->textAll(m + "\n");
    }

    void println(const char * m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(char * m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(int m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(uint8_t m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(uint16_t m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(uint32_t m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(float m) {
      _ws->textAll(String(m) + "\n");
    }

    void println(double m) {
      _ws->textAll(String(m) + "\n");
    }

    void printf(const char* format, ...){
      va_list args;
      va_start(args, format);
      char payload[WEBSERIAL_PRINTF_BUFFER_SIZE];
      uint16_t len = vsnprintf(payload, WEBSERIAL_PRINTF_BUFFER_SIZE, format, args);
      _ws->textAll(payload, len);
    }

  private:
    AsyncWebServer * _server;
    AsyncWebSocket * _ws;

    String identity = "";

    RecvMsgHandler _RecvFunc = NULL;
};

WebSerialProClass WebSerialPro;

#endif
