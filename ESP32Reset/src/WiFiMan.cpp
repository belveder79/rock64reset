// ESP32 Reset
// Copyright 2021 AR4 GmbH. All rights reserved.
// https://www.ar4.io
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of AR4 GmbH nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: clemens@ar4.io (Clemens Arth)

#include <WiFiMan.h>
#include <Constants.h>
#include <SanityChecker.h>
#include <ConfigManager.h>
#include <MemLogger.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <LITTLEFS.h>
#include <SD.h>

#include <WebSerialPro.h>

extern int lastHeartBeatValue;

// INTERFACES FOR SERVING WEB
const char* sendResetMsg(unsigned long timePullDown)
{
  SanityChecker::instance()->sendReset(timePullDown);
  return "";
}

const char* sendPowerMsg(unsigned long timePullDown)
{
  SanityChecker::instance()->sendPower(timePullDown);
  return "";
}

const char* restartESP()
{
  ESP.restart();
  return "";
}

void saveConfig(const char* val)
{
  ConfigManager::instance()->setConfigJson(val);
}

void setState(const bool state)
{
  SanityChecker::instance()->setState(state);
}

const char* getConfig()
{
  size_t sz;
  return ConfigManager::instance()->getConfigJson(sz);
}

const char* getFWVersion()
{
  return FW_VERSION;
}

#define BUFSZ 2048
char blubber [BUFSZ];
const char* popLogMsg()
{
  const char* x = MemLogger::instance()->popLog();
  unsigned stl = strlen(x);
  unsigned int len2copy = stl > (BUFSZ-1) ? (BUFSZ-1) : stl;
  memcpy(blubber,x,len2copy);
  blubber[len2copy] = '\0';
  return blubber;
}

const char* currentPowerStatus()
{
  char b[8];
  sprintf(b,"%d",SanityChecker::instance()->currentPowerStatus());
  return std::string(b).c_str();
}

const char* currentHBVal()
{
  char b[8];
  sprintf(b,"%d",SanityChecker::instance()->lastHeatBeatVal());
  return std::string(b).c_str();
}

bool WiFiMan::init()
{
  // Trigger reset from previously saved config file...
  BoardConfig* boardcfg = reinterpret_cast<BoardConfig*>(ConfigManager::instance()->getConfig(CONFIG_TYPE::BOARD));
  if(boardcfg->resetWifiSettings) {
    // fall back to default
    boardcfg->resetWifiSettings = false;
    ConfigManager::instance()->markConfigDirty();
  }

  // Connect to Wi-Fi
  int tries = 10;
  WiFi.begin(boardcfg->wifiName.c_str(), boardcfg->wifiPwd.c_str());
  while (WiFi.status() != WL_CONNECTED && tries > 0) {
    delay(1000);
    MemLogger::instance()->logMessage("=WM: Connecting to WiFi...\n");
    tries--;
  }

  if(WiFi.status() != WL_CONNECTED)
    return false;

  return startServe();
}

void WiFiMan::iterate()
{
  handleSerialData();

  if(!m_servelocal)
    return;
}

void WiFiMan::spawnHotSpot()
{
  BoardConfig* boardcfg = reinterpret_cast<BoardConfig*>(ConfigManager::instance()->getConfig(CONFIG_TYPE::BOARD));

  MemLogger::instance()->logMessage("=WM: SPAWNING HOTSPOT!\n");
  m_servelocal = true;
  WiFi.softAP(boardcfg->hotSpotName.c_str(), boardcfg->hotSpotPwd.c_str());

  IPAddress IP = WiFi.softAPIP();
  MemLogger::instance()->logMessage("=WM: AP IP address: ");
  MemLogger::instance()->logMessage(IP.toString().c_str());
  MemLogger::instance()->logMessage("\n");

  startServe();
}


bool WiFiMan::startServe()
{
  BoardConfig* boardcfg = reinterpret_cast<BoardConfig*>(ConfigManager::instance()->getConfig(CONFIG_TYPE::BOARD));

#if !SERVEFROMSD
  if(LITTLEFS.begin())
  {
#endif

#if PRINT_DEBUG
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
#endif

    m_server = new AsyncWebServer(boardcfg->serverPort);

    // Route for root / web page
    m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  #if SERVEFROMSD
      request->send(SD, "/index.html");
  #else
      request->send(LITTLEFS, "/index.html");
  #endif
    });
    m_server->on("/icon.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LITTLEFS, "/icon.css");
    });
    m_server->on("/googlematerials.woff2", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LITTLEFS, "/googlematerials.woff2");
    });
    m_server->on("/css/materialize.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LITTLEFS, "/css/materialize.min.css");
    });
    m_server->on("/js/materialize.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LITTLEFS, "/js/materialize.min.js");
    });
    m_server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LITTLEFS, "/favicon.ico");
    });
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/saveconfig", [](AsyncWebServerRequest *request, JsonVariant &json) {
      StaticJsonDocument<512> data;
      if (json.is<JsonArray>())
      {
        data = json.as<JsonArray>();
      }
      else if (json.is<JsonObject>())
      {
        data = json.as<JsonObject>();
      }
      String cfg;
      serializeJson(data, cfg);
      saveConfig(cfg.c_str());
      request->send(200, "text/plain", "OK!");
    });
    m_server->addHandler(handler);

    AsyncCallbackJsonWebHandler* handler2 = new AsyncCallbackJsonWebHandler("/wdstate", [](AsyncWebServerRequest *request, JsonVariant &json) {
      StaticJsonDocument<512> data;
      if (json.is<JsonArray>())
      {
        data = json.as<JsonArray>();
      }
      else if (json.is<JsonObject>())
      {
        data = json.as<JsonObject>();
      }
      setState(data["state"]);
      request->send(200, "text/plain", "OK!");
    });
    m_server->addHandler(handler2);


    /*
    m_server->on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request){
      Serial.printf("SIZE OF PARAMS: %d\n",request->params());

      String message = "";

      if (request->hasParam(PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM_MESSAGE, true)->value();
      } else {
          message = "No message sent";
      }

      request->send(200, "text/plain", "Hello, POST: " + message);
    });
    */
    // m_server->on("/hbval", HTTP_GET, [](AsyncWebServerRequest *request){
    //   request->send_P(200, "text/plain", currentHBVal());
    // });
    m_server->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", sendResetMsg(500));
    });

    m_server->on("/resetESP", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", restartESP());
    });
    // m_server->on("/shutdown", HTTP_GET, [](AsyncWebServerRequest *request){
    //   request->send_P(200, "text/plain", sendPowerMsg(6000));
    // });
    m_server->on("/getconfig", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "application/json", getConfig());
    });
    m_server->on("/fwversion", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", getFWVersion());
    });
    m_server->on("/powerstatus", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", currentPowerStatus());
    });
    m_server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", popLogMsg());
    });


    // start update server
    AsyncElegantOTA.begin(m_server);//m_updateServer);
    WebSerialPro.begin(m_server);//m_serialserver);
    WebSerialPro.msgCallback(WiFiMan::recvMsg);

    // Start server
    m_server->begin();

    return true;
#if !SERVEFROMSD
  }
  return false;
#endif
}

void WiFiMan::recvMsg(uint8_t *data, size_t len)
{
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  //WebSerialPro.println(d);
  Serial.println(d);

  /*
  WebSerialPro.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerialPro.println(d);
  */
}

void WiFiMan::handleSerialData()
{
  #if !DEBUG_PRINT
    if(Serial.available())
    {
      String data = Serial.readStringUntil('\n');
      WebSerialPro.print(data);
    }
  #endif
}
