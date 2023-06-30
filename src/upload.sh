#!/bin/bash
./compile.sh
arduino-cli upload -b esp32:esp32:esp32doit-devkit-v1 -p /dev/ttyUSB1 main/main.ino
./serial.sh
