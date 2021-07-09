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

#include <Arduino.h>

#include <WiFi.h>
#include <WifiMan.h>
#include <SanityChecker.h>
#include <MemLogger.h>
#include <ConfigManager.h>
#include <Constants.h>

//====================================================================
// INTERRUPT DRIVEN RESET ROUTINE
bool reset_in = false;
void IRAM_ATTR HandleResetButtonInterrupt() {
    MemLogger::instance()->logMessage("=MAIN: Interrupt from Reset Button!\n");
    if(!reset_in)
    {
      // set 250ms reset...
      reset_in = true;
      digitalWrite(ROCKRESET, HIGH);
      delay(250);
      digitalWrite(ROCKRESET, LOW);
    }
    reset_in = false;
}

int flash_changes = 0;
unsigned long flash_OnTime = 0;
bool flash_ison = false;
void ICACHE_RAM_ATTR HandleFlashButtonInterrupt()
{
    MemLogger::instance()->logMessage("=MAIN: Interrupt from Flash Button!\n");
    flash_changes++;
    flash_ison = flash_changes % 2;
}

void resetBoardToFactorySettings()
{
    ConfigManager::instance()->deleteConfigFile();

    WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
    WiFi.persistent(true);
    WiFi.disconnect(true,true);
    WiFi.persistent(false);
}

void setup()
{
  Serial.begin(115200);

  unsigned long currentTime = millis();

  //==========================================================
  // BASIC BOARD SETUP TO ALLOW POWER UP
  // attach an interrupt to the middle button for fun...
  pinMode(RSTBUTTONPIN, INPUT_PULLUP);
  attachInterrupt(RSTBUTTONPIN, HandleResetButtonInterrupt, CHANGE);

  pinMode(FLASHBUTTONPIN, INPUT_PULLUP);
  attachInterrupt(FLASHBUTTONPIN, HandleFlashButtonInterrupt, CHANGE);

  // INIT CONFIG MANAGER
  ConfigManager::instance();

  // INIT SANITY CHECKER
  SanityChecker::instance()->init(currentTime);

  //==========================================================
  // BASIC WIFI SETUP
  if(!WiFiMan::instance()->init())
  {
    MemLogger::instance()->logMessage("=MAIN: Connecting to existing WLAN failed... Will spawn hotspot!\n");
    WiFiMan::instance()->spawnHotSpot();
  }
  delay(2000);
}

void loop()
{
  unsigned long currentTime = millis();

  if(flash_ison && flash_OnTime == 0)
  {
    flash_OnTime = currentTime;
  }
  if(!flash_ison && flash_OnTime > 0)
  {
    unsigned long len = currentTime - flash_OnTime;
    char buf[128];
    sprintf(buf,"=MAIN: Time Flash button pressed for %lu ms! Press %d secs to reset!\n", len, FLASH_RESET_PERIOD);
    MemLogger::instance()->logMessage(buf);
    if(len > FLASH_RESET_PERIOD)
    {
      resetBoardToFactorySettings();
      ESP.restart();
    }
    flash_OnTime = 0;
  }

  // put your main code here, to run repeatedly:
  SanityChecker::instance()->iterate(currentTime);
  yield();
  WiFiMan::instance()->iterate();
  yield();
  delay(10);
}
