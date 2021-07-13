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

#include <SanityChecker.h>
#include <MemLogger.h>
#include <Constants.h>
#include <ConfigManager.h>

bool SanityChecker::init(unsigned long nowTime, uint32_t interval)
{
  BoardConfig* boardcfg = reinterpret_cast<BoardConfig*>(ConfigManager::instance()->getConfig(CONFIG_TYPE::BOARD));

  m_heartBeatCountTrigger = boardcfg->heartBeatCnt;
  m_coolDownTimeTrigger = boardcfg->cooldownTime;
  m_lockupTimeTrigger = boardcfg->lockupTime;

  m_coolDownEnd = boardcfg->cooldownTime;

  m_resetApplied = false;
  m_lastTimeHeartBeatChanged = 0;
  m_lastTimeLoopIteration = 0;
  m_lastHeartBeatValue = 0;
  m_heartBeatCounter = 0;

  m_enabled = boardcfg->enabled;

  MemLogger::instance()->logMessage("=SC: Initializing Sanity Checker...\n");
  // define input heartbeat pin...
  pinMode(HEARTBEAT, INPUT);
  pinMode(POWERWATCH, INPUT);

  // define pulldowns to be down by default
  pinMode(ROCKRESET, OUTPUT);
  pinMode(ROCKPOWER, OUTPUT);
  digitalWrite(ROCKRESET, LOW);
  digitalWrite(ROCKPOWER, LOW);

  m_pollingInterval = interval;
  m_nextPoll = nowTime + interval;
  return true;
}

void SanityChecker::convertMillis(unsigned long milli, unsigned long& hour, unsigned long &minute, unsigned long &second, unsigned long &remainder)
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

void SanityChecker::setState(bool enabled)
{
    MemLogger::instance()->logMessage(enabled ? "=SC: Enable WD\n" : "=SC: Disable WD\n");
    m_enabled = enabled;
    ConfigManager::instance()->setState(enabled);
}

bool SanityChecker::coolDownActive(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder)
{
  bool active = currentTime < m_coolDownEnd;
  if(active)
  {
    unsigned long seconds2 = (m_coolDownEnd - currentTime) / 1000;

    char buf[256];
    sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Cooldown active for another %lu seconds\n", hour, minute, second, remainder, seconds2);
    MemLogger::instance()->logMessage(buf);
  }
  return active;
}

bool SanityChecker::readHeartBeat(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder)
{
  m_lastPowerValue = digitalRead(POWERWATCH);
#if PRINT_VERBOSE
  char buf[256];
  sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Current Power Watch Status: %s\n", hour, minute, second, remainder, m_lastPowerValue ? "on" : "off");
  MemLogger::instance()->logMessage(buf);
#endif
  int currentHeartBeatValue = digitalRead(HEARTBEAT);
  // value changed!
  if(currentHeartBeatValue != m_lastHeartBeatValue)
  {
    m_lastHeartBeatValue = currentHeartBeatValue;
    m_lastTimeHeartBeatChanged = currentTime;
    if(m_heartBeatCounter < m_heartBeatCountTrigger) {
      m_heartBeatCounter++;
    }
    else if(m_heartBeatCounter == m_heartBeatCountTrigger) {
      char buf[256];
      sprintf(buf,"=SC: [%02lu:%02lu:%02lu.%03lu] Resetting cooldown timer!\n", hour, minute, second, remainder);
      MemLogger::instance()->logMessage(buf);
      m_coolDownEnd = currentTime;
      m_heartBeatCounter++;
    }
    return false;
  }
  else // value did not change
  {
    // locked up!
    if(((currentTime > (m_lastTimeHeartBeatChanged + m_lockupTimeTrigger)) && !coolDownActive(currentTime, hour, minute, second, remainder)) || !m_lastPowerValue)
    {
      delay(500);
      m_heartBeatCounter = 0;
      return true;
    }
  }
  return false;
}

// sends a reset signal to the RockPro64
void SanityChecker::sendReset(unsigned long timePullDown)
{
  unsigned long currentTime = millis();
  unsigned long doneTime = currentTime+timePullDown;
  MemLogger::instance()->logMessage("=SC: Executing RESET message\n");
  digitalWrite(ROCKRESET, HIGH);
  while(doneTime > currentTime)
  {
    delay(50);
    yield();
    currentTime = millis();
  }
  digitalWrite(ROCKRESET, LOW);
  delay(200);
}

void SanityChecker::sendPower(unsigned long timePullDown, bool ignorePowerStatus)
{
  unsigned long currentTime = millis();
  unsigned long doneTime = currentTime+timePullDown;
  if(ignorePowerStatus)
  {
    MemLogger::instance()->logMessage("=SC: Executing POWER message\n");
    digitalWrite(ROCKPOWER, HIGH);
    while(doneTime > currentTime)
    {
      delay(50);
      yield();
      currentTime = millis();
    }
    digitalWrite(ROCKPOWER, LOW);
    delay(200);
  }
  else
  {
    MemLogger::instance()->logMessage("=SC: Ignoring POWER message\n");
    // CHECK CURRENT POWER STATUS AND DECIDE ON PULLDOWN TIMEOUT
    // work with m_lastPowerValue





  }
}

void SanityChecker::iterate(unsigned long currentTime)
{
  if(currentTime <= m_nextPoll)
  {
    yield();
    return;
  }
  m_nextPoll += m_pollingInterval;

  // safeguard overflow after 50 days or so...
  if(m_lastTimeLoopIteration > currentTime)
  {
    m_lastTimeLoopIteration = currentTime;
    m_coolDownEnd = currentTime;
    return;
  }

  unsigned long hour, minute, second, remainder;
  convertMillis(currentTime, hour, minute, second, remainder);

  // m_lastHeartBeatValue is the heartbeat
  // it is 0 (and does not change if board is off)
  // it is 1 (and does not change if board is on and locked up)
  // it alternates between 0 and 1 otherwise

  // returns true if board is off or locked up
  if(readHeartBeat(currentTime, hour, minute, second, remainder))
  {
    if(m_enabled)
    {
      char buf[256];
      sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Board locked up!\n", hour, minute, second, remainder);
      MemLogger::instance()->logMessage(buf);

      sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Status is %s!\n", hour,
        minute, second, remainder, m_lastHeartBeatValue > 0 ? "on" : "off");
      MemLogger::instance()->logMessage(buf);

      sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Send power/reset combi!\n", hour, minute, second, remainder);
      MemLogger::instance()->logMessage(buf);

      sendPower(2000);
      yield();
      delay(1000);
      yield();
      sendReset(2000);
    }
    // sets back the timer for eval against RESET_TIME secs
    m_lastTimeHeartBeatChanged = currentTime;
    // cooldown should start now!
    m_coolDownEnd = currentTime + m_coolDownTimeTrigger;
  }
  else
  {
    // board is alive, so reset flags
#if PRINT_VERBOSE
    char buf[256];
    sprintf(buf,"=SC:[%02lu:%02lu:%02lu.%03lu] Last value: %d - changed %lu ms ago!\n", hour,
      minute, second, remainder, m_lastHeartBeatValue, currentTime - m_lastTimeHeartBeatChanged );
    MemLogger::instance()->logMessage(buf);
#endif
  }
}
