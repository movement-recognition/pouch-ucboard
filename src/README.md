### setup
- install arduino IDE
- add `https://dl.espressif.com/dl/package_esp32_index.json` to Board-List (see Ctrl+Comma, textbox below)

### stuff for CI/CD
- arduino-cli core install esp32:esp32…
- arduino-cli core update-index

### useful commands for arduino-cli
- list all installed esp32-compiler-environments `arduino-cli board search esp32`
- board-id `esp32:esp32:esp32doit-devkit-v1`
- compile: `arduino-cli compile -b esp32:esp32:esp32doit-devkit-v1 main.ino`
- upload: `arduino-cli upload -b esp32:esp32:esp32doit-devkit-v1 -p /dev/ttyUSB0 main.ino`


### ESP: Power consumption
- current is more or less independent of drive voltage
- idle with "default-code" => around ~55mA
- switching WiFi off => ~45mA
- switching BT off + reducing Frequency => ~25mA
for more intensive sleeping-modes see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#id1

