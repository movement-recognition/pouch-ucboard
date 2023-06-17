#!/bin/bash
./compile.sh
arduino-cli upload -b esp32:esp32:esp32doit-devkit-v1 -p /dev/ttyUSB0 main/main.ino
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
