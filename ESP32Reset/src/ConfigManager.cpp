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

#include <ConfigManager.h>
#include <MemLogger.h>

#include <ArduinoJson.h>
#include <LITTLEFS.h>

void ConfigManager::init()
{
  // init all functions
  m_configDirty = false;
  DynamicJsonDocument config(CONFIGFILE_DEFAULT_SIZE);
  bool configFileSane = false;
  if (LITTLEFS.begin())
  {
    MemLogger::instance()->logMessage("=CM: Mounted file system\n");
    // parse json config file
    File configFile = LITTLEFS.open(CONFIGFILE_DEFAULT_NAME, "r");
    if (configFile)
    {
        // Allocate a buffer to store contents of the file.
        MemLogger::instance()->logMessage("=CM: JSON Config File found!\n");

        // parse
        size_t size = configFile.size();

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        auto error = deserializeJson(config, buf.get());
        if (error)
        {
          char buf[128];
          sprintf(buf, "=CM: deserialize JSON Config failed with code: %s\n",error.c_str());
          MemLogger::instance()->logMessage(buf);
        }
        else
        {
          // set variables from config file
          //try
          //{
            m_BoardConfig.chipId            = config["BoardConfig"].containsKey("chipId")             ? config["BoardConfig"]["chipId"] : BOARD_DEFAULT_CHIPID ;
            m_BoardConfig.configVersion     = config["BoardConfig"].containsKey("configVersion")      ? config["BoardConfig"]["configVersion"] : 0 ;
            m_BoardConfig.resetWifiSettings = config["BoardConfig"].containsKey("resetWifiSettings")  ? config["BoardConfig"]["resetWifiSettings"] : WIFI_DEFAULT_RESET ;

            m_BoardConfig.serverPort        = config["BoardConfig"].containsKey("serverPort")         ? config["BoardConfig"]["serverPort"] : DEFAULT_SERVERPORT ;

            m_BoardConfig.hotSpotName       = config["BoardConfig"].containsKey("hotSpotName")        ? config["BoardConfig"]["hotSpotName"].as<String>() : DEFAULT_HOTSPOTSSID ;
            m_BoardConfig.hotSpotPwd        = config["BoardConfig"].containsKey("hotSpotPwd")         ? config["BoardConfig"]["hotSpotPwd"].as<String>() : DEFAULT_HOTSPOTPWD ;
            m_BoardConfig.wifiName          = config["BoardConfig"].containsKey("wifiName")           ? config["BoardConfig"]["wifiName"].as<String>() : DEFAULT_WIFISSID ;
            m_BoardConfig.wifiPwd           = config["BoardConfig"].containsKey("wifiPwd")            ? config["BoardConfig"]["wifiPwd"].as<String>() : DEFAULT_WIFIPWD ;

            m_BoardConfig.lockupTime        = config["BoardConfig"].containsKey("lockupTime")         ? config["BoardConfig"]["lockupTime"] : DEFAULT_LOCKUP_TIME;
            m_BoardConfig.cooldownTime      = config["BoardConfig"].containsKey("cooldownTime")       ? config["BoardConfig"]["cooldownTime"] : DEFAULT_COOLDOWN_TIME;
            m_BoardConfig.heartBeatCnt      = config["BoardConfig"].containsKey("heartBeatCnt")       ? config["BoardConfig"]["heartBeatCnt"] : DEFAULT_HEARTBEAT_COUNT;

            printConfig();

            MemLogger::instance()->logMessage("=CM: JSON Config file successfully loaded!\n");
            configFileSane = true;
          //}
          //catch(...)
          //{
          //  Serial.println("=CM: Error parsing JSON file!");
          //}
        }
        configFile.close();
    }
    else
      MemLogger::instance()->logMessage("=CM: JSON Config file load failed!\n");

    LITTLEFS.end();
  }
  else
  {
    MemLogger::instance()->logMessage("=CM: File System issue!\n");
  }
  if(!configFileSane)
    MemLogger::instance()->logMessage("=CM: Using configuration from firmware defaults\n");
}

void ConfigManager::printConfig()
{
#if PRINT_DEBUG
  char buf[256];
  MemLogger::instance()->logMessage("=CM: ================ CURRENT CONFIG ===================\n");
  sprintf(buf,"=CM: BoardConfig.chipId:            %d \n", m_BoardConfig.chipId); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.configVersion      %d \n", m_BoardConfig.configVersion); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.resetWifiSettings  %d \n", m_BoardConfig.resetWifiSettings); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.serverPort         %d \n", m_BoardConfig.serverPort); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.hotSpotName:       %s \n", m_BoardConfig.hotSpotName.c_str()); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.hotSpotPwd         %s \n", m_BoardConfig.hotSpotPwd.c_str()); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.wifiName           %s \n", m_BoardConfig.wifiName.c_str()); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.wifiPwd            %s \n", m_BoardConfig.wifiPwd.c_str()); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.lockupTime:        %d \n", m_BoardConfig.lockupTime); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.cooldownTime       %d \n", m_BoardConfig.cooldownTime); MemLogger::instance()->logMessage(buf);
  sprintf(buf,"=CM: BoardConfig.heartBeatCnt       %d \n", m_BoardConfig.heartBeatCnt); MemLogger::instance()->logMessage(buf);
  MemLogger::instance()->logMessage("=CM: ================ CURRENT CONFIG ===================\n");
#endif
}

void ConfigManager::deleteConfigFile()
{
  MemLogger::instance()->logMessage("=CM: Searching config file...\n");
  if (LITTLEFS.begin())
  {
    MemLogger::instance()->logMessage("=CM: Config file deleted!\n");
    if(LITTLEFS.exists(CONFIGFILE_DEFAULT_NAME))
      LITTLEFS.remove(CONFIGFILE_DEFAULT_NAME);
    else
      MemLogger::instance()->logMessage("=CM: Config file not found!\n");

    LITTLEFS.end();
  }
  else
  {
    MemLogger::instance()->logMessage("=CM: File System issue!\n");
  }
}

Config* ConfigManager::getConfigFunction(CONFIG_TYPE type)
{
  return ConfigManager::instance()->getConfig(type);
}

Config* ConfigManager::getConfig(CONFIG_TYPE type)
{
  Config * ptr = NULL;
  switch(type)
  {
    case BOARD:
      ptr = &m_BoardConfig;
      break;
    default:
      ptr = NULL;
  }
  return ptr;
}

bool ConfigManager::saveConfigToFile()
{
  MemLogger::instance()->logMessage("=CM: Saving Config File...\n");
  if (LITTLEFS.begin())
  {
    MemLogger::instance()->logMessage("=CM: Mounted file system\n");
    // parse json config file
    File configFile = LITTLEFS.open(CONFIGFILE_DEFAULT_NAME, "w");
    if (configFile)
    {
      //try
      //{
        size_t size;
        char* buf = getConfigJson(size);
        configFile.write(reinterpret_cast<uint8_t*>(buf), size);
      //}
      //catch(...)
      //{
      //  Serial.println("=CM: Error writing JSON file!");
      //}
      configFile.close();
    }
    else {
      MemLogger::instance()->logMessage("=CM: JSON Config file write failed!\n");
      return false;
    }
    LITTLEFS.end();
  }
  else
  {
    MemLogger::instance()->logMessage("=CM: File System issue!\n");
    return false;
  }
  return true;
}

char* ConfigManager::getConfigJson(size_t &size){

  DynamicJsonDocument config(CONFIGFILE_DEFAULT_SIZE);

  config["BoardConfig"]["chipId"] =             m_BoardConfig.chipId;
  config["BoardConfig"]["configVersion"] =      m_BoardConfig.configVersion + 1;
  config["BoardConfig"]["resetWifiSettings"] =  m_BoardConfig.resetWifiSettings;
  config["BoardConfig"]["serverPort"] =         m_BoardConfig.serverPort;
  config["BoardConfig"]["hotSpotName"] =        m_BoardConfig.hotSpotName;
  config["BoardConfig"]["hotSpotPwd"] =         m_BoardConfig.hotSpotPwd;
  config["BoardConfig"]["wifiName"] =           m_BoardConfig.wifiName;
  config["BoardConfig"]["wifiPwd"] =            m_BoardConfig.wifiPwd;
  config["BoardConfig"]["lockupTime"] =         m_BoardConfig.lockupTime;
  config["BoardConfig"]["cooldownTime"] =       m_BoardConfig.cooldownTime;
  config["BoardConfig"]["heartBeatCnt"] =       m_BoardConfig.heartBeatCnt;


  std::unique_ptr<char[]> buf(new char[CONFIGFILE_DEFAULT_SIZE]);
  size = serializeJson(config,buf.get(),CONFIGFILE_DEFAULT_SIZE);

  return buf.get();
}

void ConfigManager::setConfigJson(const char* val)
{
  try
  {
    DynamicJsonDocument config(CONFIGFILE_DEFAULT_SIZE);
    if(DeserializationError::Ok == deserializeJson(config, val))
    {
      config["BoardConfig"].containsKey("serverPort")         ? m_BoardConfig.serverPort   = config["BoardConfig"]["serverPort"] : 0 ;

      config["BoardConfig"].containsKey("hotSpotName")        ? m_BoardConfig.hotSpotName  = config["BoardConfig"]["hotSpotName"].as<String>() : "" ;
      config["BoardConfig"].containsKey("hotSpotPwd")         ? m_BoardConfig.hotSpotPwd   = config["BoardConfig"]["hotSpotPwd"].as<String>() : "" ;
      config["BoardConfig"].containsKey("wifiName")           ? m_BoardConfig.wifiName     = config["BoardConfig"]["wifiName"].as<String>() : "" ;
      config["BoardConfig"].containsKey("wifiPwd")            ? m_BoardConfig.wifiPwd      = config["BoardConfig"]["wifiPwd"].as<String>() : "" ;

      config["BoardConfig"].containsKey("lockupTime")         ? m_BoardConfig.lockupTime   = config["BoardConfig"]["lockupTime"] : 0;
      config["BoardConfig"].containsKey("cooldownTime")       ? m_BoardConfig.cooldownTime = config["BoardConfig"]["cooldownTime"] : 0;
      config["BoardConfig"].containsKey("heartBeatCnt")       ? m_BoardConfig.heartBeatCnt = config["BoardConfig"]["heartBeatCnt"] : 0;

      saveConfigToFile();
      MemLogger::instance()->logMessage("=CM: JSON Config file written successfully!\n");
    }
    else
    {
      MemLogger::instance()->logMessage("=CM: Parsing JSON Config failed!\n");
    }
  }
  catch (...)
  {
    MemLogger::instance()->logMessage("=CM: JSON Config file write failed!\n");
  }
}
