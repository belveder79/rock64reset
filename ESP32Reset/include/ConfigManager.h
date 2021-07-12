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

#ifndef _CONFIGMANAGER_H_INCLUDED_
#define _CONFIGMANAGER_H_INCLUDED_

#include <Arduino.h>

#include <Singleton.h>
#include <Constants.h>

class ConfigManager : public Singleton <ConfigManager>
{
  friend class Singleton <ConfigManager>;
public:
  ~ConfigManager () { }
  void markConfigDirty() { m_configDirty = true; }
  void deleteConfigFile();
  bool saveConfigToFile();
  void setState(bool enabled);

  static Config* getConfigFunction(CONFIG_TYPE type);
  Config* getConfig(CONFIG_TYPE type);

  char* getConfigJson(size_t &size);
  void setConfigJson(const char* val);
protected:
  ConfigManager()
  {
    init();
  }
private:

  void init();
  void printConfig();

  bool m_configDirty;

  // all config values required...
  BoardConfig m_BoardConfig;

};

#endif // _CONFIGMANAGER_H_INCLUDED_
