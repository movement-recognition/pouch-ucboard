### setup
- install arduino IDE
- add `https://dl.espressif.com/dl/package_esp32_index.json` to Board-List (see Ctrl+Comma, textbox below)

### useful commands for arduino-cli
- list all installed esp32-compiler-environments `arduino-cli board search esp32`
- board-id `esp32:esp32:esp32doit-devkit-v1`
- compile: `arduino-cli compile -b esp32:esp32:esp32doit-devkit-v1 main.ino`
- upload: `arduino-cli upload -b esp32:esp32:esp32doit-devkit-v1 -p /dev/ttyUSB0 main.ino`