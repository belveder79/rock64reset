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


#ifndef _CONSTANTS_H_INCLUDED_
#define _CONSTANTS_H_INCLUDED_

#include <string>

#define FW_VERSION                  "21.07.12.21"

#define BOARD_DEFAULT_CHIPID        1
#define WIFI_DEFAULT_RESET          0

#define PRINT_DEBUG                 0        // DO NOT ENABLE THIS IF USB-C IS WIRED TO ROCKPRO!!!
#define PRINT_VERBOSE               0
#define DEFAULT_WIFISSID            "unknown"
#define DEFAULT_WIFIPWD             "unknown"

#define DEFAULT_WD_ENABLED          1

#define DEFAULT_SERVERPORT          80
#define SERVEFROMSD                 0

#define DEFAULT_HOTSPOTSSID         "rock64reset"
#define DEFAULT_HOTSPOTPWD          "rock64reset"

#define TIMEOFFSETINSECS            3600

#define RSTBUTTONPIN                5
#define FLASHBUTTONPIN              0

#define FLASH_RESET_PERIOD          5000 // 5sec

#define HEARTBEAT                   16
#define POWERWATCH                  17
#define ROCKRESET                   14
#define ROCKPOWER                   13

#define DEFAULT_LOCKUP_TIME                       10000 // time board is allowed to not send heartbeat
#define DEFAULT_COOLDOWN_TIME                     120000 // time after action with no further action to be taken
#define DEFAULT_HEARTBEAT_COUNT                   10

#define CONFIGFILE_DEFAULT_SIZE           1024
#define CONFIGFILE_DEFAULT_NAME           "/config.json"

typedef enum : uint8_t
{
  BOARD = 0,
  MAXTYPES
} CONFIG_TYPE;

typedef struct
{} Config;

typedef struct : public Config
{
    int chipId = BOARD_DEFAULT_CHIPID;
    int configVersion = 0;
    bool resetWifiSettings = WIFI_DEFAULT_RESET;
    uint16_t serverPort = DEFAULT_SERVERPORT;
    String hotSpotName = DEFAULT_HOTSPOTSSID;
    String hotSpotPwd = DEFAULT_HOTSPOTPWD;
    String wifiName = DEFAULT_WIFISSID;
    String wifiPwd = DEFAULT_WIFIPWD;
    int lockupTime = DEFAULT_LOCKUP_TIME;
    int cooldownTime = DEFAULT_COOLDOWN_TIME;
    int heartBeatCnt = DEFAULT_HEARTBEAT_COUNT;
    bool enabled = DEFAULT_WD_ENABLED;
} BoardConfig;

typedef Config*(*ConfigAccessFunction)(CONFIG_TYPE type);

#endif
