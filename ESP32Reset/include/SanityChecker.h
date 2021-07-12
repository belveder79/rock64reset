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

#ifndef _SANITYCHECKER_H_INCLUDED_
#define _SANITYCHECKER_H_INCLUDED_

#include "Singleton.h"

class SanityChecker : public Singleton <SanityChecker>
{
   friend class Singleton <SanityChecker>;
   public:
      ~SanityChecker () { }
      bool init(unsigned long nowTime, uint32_t interval = 1000); // in ms...
      void setState(bool enabled);
      void iterate(unsigned long currentTime);
      void sendPower(unsigned long timePullDown, bool ignorePowerStatus = true);
      void sendReset(unsigned long timePullDown);
      int lastHeatBeatVal() const { return m_lastHeartBeatValue; }
      int currentPowerStatus() const { return m_lastPowerValue; }
   protected:
      SanityChecker () { }
   private:
      void convertMillis(unsigned long milli, unsigned long& hour, unsigned long &minute, unsigned long &second, unsigned long &remainder);
      bool coolDownActive(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder);
      bool readHeartBeat(unsigned long currentTime, unsigned long hour, unsigned long minute, unsigned long second, unsigned long remainder);

      uint32_t                 m_pollingInterval;
      unsigned long            m_nextPoll;

      bool                      m_enabled;

      unsigned long             m_coolDownEnd; // time when cooldown started
      bool                      m_resetApplied; // reset was last action taken
      unsigned long             m_lastTimeHeartBeatChanged; // last time value changed
      unsigned long             m_lastTimeLoopIteration; // general
      int                       m_lastHeartBeatValue; // default to off
      int                       m_lastPowerValue;
      int                       m_heartBeatCounter;

      int                       m_heartBeatCountTrigger;
      unsigned long             m_coolDownTimeTrigger;
      unsigned long             m_lockupTimeTrigger;

};

#endif
