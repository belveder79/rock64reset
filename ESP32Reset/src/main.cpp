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

#include <WifiMan.h>

#define BUTTONPIN 5
#define HEARTBEAT 16
#define POWERWATCH 17
#define ROCKRESET 14
#define ROCKPOWER 13

bool in = false;
void IRAM_ATTR HandleButtonInterrupt() {
    Serial.println("Interrupt from Button!");
    if(!in)
    {
      // set 100ms reset...
      in = true;
      digitalWrite(ROCKRESET, HIGH);
      delay(100);
      digitalWrite(ROCKRESET, LOW);
    }
    in = false;
}

void convertMillis(unsigned long milli, unsigned long& hour, unsigned long &minute, unsigned long &second, unsigned long &remainder)
{
  //3600000 milliseconds in an hour
  hour = milli / 3600000;
  milli = milli - 3600000 * hour;
  //60000 milliseconds in a minute
  minute = milli / 60000;
  milli = milli - 60000 * minute;

  //1000 milliseconds in a second
  second = milli / 1000;
  remainder = milli - 1000 * second;
}

void setup() {

  Serial.begin(115200);
  // attach an interrupt to the middle button for fun...
  attachInterrupt(BUTTONPIN, HandleButtonInterrupt, FALLING);

  // define input heartbeat pin...
  pinMode(HEARTBEAT, INPUT);
  pinMode(POWERWATCH, INPUT);

  // define pulldowns to be down by default
  pinMode(ROCKRESET, OUTPUT);
  pinMode(ROCKPOWER, OUTPUT);
  digitalWrite(ROCKRESET, LOW);
  digitalWrite(ROCKPOWER, LOW);

  if(!WiFiMan::instance()->init())
  {
    Serial.println("Connecting to existing WLAN failed... Will spawn hotspot!");
    WiFiMan::instance()->spawnHotSpot();
  }
  delay(2000);
}

//====================================================================

int LOCKUP_TIME = 10 * 1000; // time board is allowed to not send heartbeat
int COOLDOWN_TIME = 120 * 1000; // time after action with no further action to be taken
int HEARTBEAT_COUNT = 10;

unsigned long coolDownEnd = COOLDOWN_TIME; // time when cooldown started
bool resetApplied = false; // reset was last action taken
unsigned long lastTimeHeartBeatChanged = 0; // last time value changed
unsigned long lastTimeLoopIteration = 0; // general
int lastHeartBeatValue = 0; // default to off
int heartBeatCounter = 0;

bool coolDownActive(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder)
{
  bool active = currentTime < coolDownEnd;
  if(active) {
    unsigned long seconds2 = (coolDownEnd - currentTime) / 1000;
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] Cooldown active for another %lu seconds\n", hour, minute, second, remainder, seconds2);
  }
  return active;
}

bool readHeartBeat(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder)
{
  int currentPowerStatus = digitalRead(POWERWATCH);
  Serial.printf("[%02lu:%02lu:%02lu.%03lu] Current Power Watch Status: %s\n", hour, minute, second, remainder, currentPowerStatus ? "on" : "off");
  int currentHeartBeatValue = digitalRead(HEARTBEAT);
  // value changed!
  if(currentHeartBeatValue != lastHeartBeatValue)
  {
    lastHeartBeatValue = currentHeartBeatValue;
    lastTimeHeartBeatChanged = currentTime;
    if(heartBeatCounter < HEARTBEAT_COUNT) {
      heartBeatCounter++;
    }
    else if(heartBeatCounter == HEARTBEAT_COUNT) {
      Serial.printf("[%02lu:%02lu:%02lu.%03lu] Resetting cooldown timer!\n", hour, minute, second, remainder);
      coolDownEnd = currentTime;
      heartBeatCounter++;
    }
    return false;
  }
  else // value did not change
  {
    // locked up!
    if(((currentTime > (lastTimeHeartBeatChanged + LOCKUP_TIME)) && !coolDownActive(currentTime, hour, minute, second, remainder)) || !currentPowerStatus)
    {
      delay(500);
      heartBeatCounter = 0;
      return true;
    }
  }
  return false;
}

// sends a reset signal to the RockPro64
void sendReset(unsigned long timePullDown)
{
  digitalWrite(ROCKRESET, HIGH);
  delay(timePullDown);
  digitalWrite(ROCKRESET, LOW);
  delay(200);
}

void sendPower(unsigned long timePullDown)
{
  digitalWrite(ROCKPOWER, HIGH);
  delay(timePullDown);
  digitalWrite(ROCKPOWER, LOW);
  delay(200);
}

void sanityCheck()
{
  unsigned long currentTime = millis();
  // safeguard overflow after 50 days or so...
  if(lastTimeLoopIteration > currentTime)
  {
    lastTimeLoopIteration = currentTime;
    coolDownEnd = currentTime;
    delay(1000);
    return;
  }

  unsigned long hour, minute, second, remainder;
  convertMillis(currentTime, hour, minute, second, remainder);

  // lastHeartBeatValue is the heartbeat
  // it is 0 (and does not change if board is off)
  // it is 1 (and does not change if board is on and locked up)
  // it alternates between 0 and 1 otherwise

  // returns true if board is off or locked up
  if(readHeartBeat(currentTime, hour, minute, second, remainder))
  {
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] Board locked up!\n", hour, minute, second, remainder);
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] Status is %s!\n", hour,
      minute, second, remainder, lastHeartBeatValue > 0 ? "on" : "off");

    Serial.printf("[%02lu:%02lu:%02lu.%03lu] Send power/reset combi!\n", hour, minute, second, remainder);
    sendPower(2000);
    delay(1000);
    sendReset(2000);

    // sets back the timer for eval against RESET_TIME secs
    lastTimeHeartBeatChanged = currentTime;
    // cooldown should start now!
    coolDownEnd = currentTime + COOLDOWN_TIME;
  }
  else
  {
    // board is alive, so reset flags
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] Last value: %d - changed %lu ms ago!\n", hour,
      minute, second, remainder, lastHeartBeatValue, currentTime - lastTimeHeartBeatChanged );
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  sanityCheck();
  delay(400);
  WiFiMan::instance()->iterate();
  yield();
}
