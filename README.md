# ESP32 Reset Circuit

This repository contains a collection of scripts, tools and other documentations for the ESP32-based reset watchdog by [AR4 GmbH](https://www.ar4.io).

![ESP32 Reset Board](https://gitlab.icg.tugraz.at/ar4/rock64reset/-/blob/master/images/board.png)

Features at a glance:
- 5-12V input voltage
- ESP32 programmable interface with onboard UART2Serial converter with USB-C connector
- 3 programmable buttons
- 2 pull-down outputs, 2 pull-up outputs
- 2 input pins
- Thermoresistor to measure temperature from ESP32
- isolated circuit with Sensirion SHT31 temperature and humidity sensor

---
## Purpose

The main purpose of this board to act as a smart watchdog

---
## Wiring

---
## Temperature Sensor


### Node-Red Integration

---
## ESP32 Firmware


---
## Heatbeat over GPIO

Obviously there are multiple different ways to interface the GPIO of a RockPro64 or RPi.

### Using Python on RockPro64

We are using ```python``` and the [GPIO library](https://github.com/ThroneVault/Rock64-R64.GPIO/) together with an according script to output a heartbeat on a particular GPIO with a 1 second interval. The according file ```heartbeat.py``` looks somewhat like this:

```python
#!/usr/bin/env python

# Clemens Arth, 2021, LGPLv3 License
# based on...
# Frank Mankel, 2018, LGPLv3 License
# Rock 64 GPIO Library for Python
# Thanks Allison! Thanks smartdave!

import R64.GPIO as GPIO
from time import sleep

print("Output Test R64.GPIO Module...")

# Set Variables -> in this case we are at pin 8, aka GPIO #148
var_gpio_out = 148

# GPIO Setup
GPIO.setwarnings(True)
GPIO.setmode(GPIO.ROCK)
GPIO.setup(var_gpio_out, GPIO.OUT, initial=GPIO.HIGH)       # Set up GPIO as an output, with an initial state of HIGH

# Test Output
print("")
print("Testing GPIO Input/Output:")

while True:
        var_gpio_state = GPIO.input(var_gpio_out)       # Return State of GPIO
        print("Output State: " + str(var_gpio_state))
        if var_gpio_state == 1:
                GPIO.output(var_gpio_out,0)                         # Set GPIO to LOW
                print("Turning off")              # Print results
        else:
                GPIO.output(var_gpio_out,1)                         # Set GPIO to LOW
                print("Turning on")
        sleep(1.0)
exit()
```

This sample is largely taken from [Frank Mankel's homepage](https://forum.frank-mankel.org/topic/292/rockpro64-rp64-gpio/6). The pinout configuration is like this for RockPro64:

![RockPro 64 Pinout](https://forum.frank-mankel.org/assets/uploads/files/1537460598918-rockpro64_gpio_reference-resized.png)

### Service File

The easiest way to get the heartbeat working is to install a file for ```systemd```. Here is an example script ```heartbeat.service``` that should go into ```/etc/systemd/system```:

```
[Unit]
Description=HeartBeatService
After=network.target

[Service]
Environment=PYTHONUNBUFFERED=1
ExecStart=/usr/bin/python3 /home/<user>/heartbeat.py
ExecReload=/bin/kill -HUP $MAINPID
KillMode=process
Restart=always

[Install]
WantedBy=multi-user.target
```

Naturally, you can start the script from hand by using something like ```systemctl start heartbeat```. If you want to start it at every boot, you have to enable the service accordingly using ```systemctl enable heartbeat```.
