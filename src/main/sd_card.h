#ifndef SDCARD_H
#define SDCARD_H

#include <cstdint>
#include <stdint.h>
#include <Arduino.h>
#include <SD.h>
#include <FS.h>


#define HW_SDCARD_CS_PIN 5
#define SDCARD_FLUSH_LIMIT 100

class SD_Card {
    public:
        void write(char* data);
        void setup();

        File dataFile;
        void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
        void writeFile(fs::FS &fs, const char * path, const char * message);
        void appendFile(fs::FS &fs, const char * path, const char * message);
        void readFile(fs::FS &fs, const char * path);
    private:
        uint_fast8_t write_count = 0;
};

#endif