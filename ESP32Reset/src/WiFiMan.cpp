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

#include "WiFiMan.h"
#include "Constants.h"

#include <WiFi.h>
#include <LITTLEFS.h>
#include <SD.h>

extern void sendReset(unsigned long timePullDown);
extern void sendPower(unsigned long timePullDown);
extern const char* popLog();
extern int lastHeartBeatValue;

const char* sendResetI(unsigned long timePullDown)
{
  sendReset(timePullDown);
  return "";
}

const char* sendPowerI(unsigned long timePullDown)
{
  sendPower(timePullDown);
  return "";
}

#define BUFSZ 2048
char blubber [BUFSZ];
const char* popLogI()
{
  const char* x = popLog();
  unsigned stl = strlen(x);
  unsigned int len2copy = stl > (BUFSZ-1) ? (BUFSZ-1) : stl;
  memcpy(blubber,x,len2copy);
  blubber[len2copy] = '\0';
  return blubber;
}

const char* currentHBVal()
{
  char b[8];
  sprintf(b,"%d",lastHeartBeatValue);
  return std::string(b).c_str();
}

bool WiFiMan::init()
{
  // Connect to Wi-Fi
  int tries = 10;
  WiFi.begin(WIFISSID, WIFIPWD);
  while (WiFi.status() != WL_CONNECTED && tries > 0) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    tries--;
  }

  if(WiFi.status() != WL_CONNECTED)
    return false;

  return startServe();
}

void WiFiMan::iterate()
{
  if(!m_servelocal)
    return;

}

void WiFiMan::spawnHotSpot()
{
  Serial.println("SPAWNING HOTSPOT!");
  m_servelocal = true;
  WiFi.softAP(HOTSPOTSSID, HOTSPOTPWD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // LEDManager::instance()->turnOnHotspotLight();

  startServe();
}


bool WiFiMan::startServe()
{
#if !SERVEFROMSD
  if(LITTLEFS.begin())
  {
#endif
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    m_server = new AsyncWebServer(SERVERPORT);

    // Route for root / web page
    m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  #if SERVEFROMSD
      request->send(SD, "/index.html");
  #else
      request->send(LITTLEFS, "/index.html");
  #endif
    });
    /*
    m_server->on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
  #if SERVEFROMSD
      request->send(SD, "/highcharts.js");
  #else
      request->send(LITTLEFS, "/highcharts.js");
  #endif
    });
    */
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
/*
    m_server->on("/tvoc", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", SGP30Manager::instance()->getLastTVOC());
    });
//    m_server->on("/last60mins", HTTP_GET, [](AsyncWebServerRequest *request){
//      int32_t timeSince1970 = RTCManager::instance()->getTimeSize1970() + TIMEOFFSETINSECS;
//      DBManager::instance()->getLast60Minutes(timeSince1970);
//    });
    m_server->on("/co2", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", SGP30Manager::instance()->getLastCO2());
    });
    */

    m_server->on("/hbval", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", currentHBVal());
    });
    m_server->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", sendResetI(500));
    });
    m_server->on("/shutdown", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", sendPowerI(6000));
    });
    m_server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", popLogI());
    });

    // Start server
    m_server->begin();

    return true;
#if !SERVEFROMSD
  }
  return false;
#endif
}
