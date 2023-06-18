#ifndef OTHER_H
#define OTHER_H


#include <stdint.h>
#include <Arduino.h>
#include <esp_attr.h>
#include <Wire.h>

#define HW_TEMPERATURE_PIN 36
#define HW_VIBRATION_PIN 32

#define OTHER_BUFFER_SIZE 128

class Other {
    public:

        const uint8_t bufsize = OTHER_BUFFER_SIZE;

        uint16_t meas_Temp[OTHER_BUFFER_SIZE];
        uint16_t meas_Vibration[OTHER_BUFFER_SIZE];
        uint32_t meas_T[OTHER_BUFFER_SIZE];
        uint8_t meas_ptr = 0;

        void interrupt();

        void setup();

        void process();

};


#endif