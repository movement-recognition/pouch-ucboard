#ifndef ADXL377_H
#define ADXL377_H

#include <stdint.h>
#include <Arduino.h>
#include <esp_attr.h>


//https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/


// initialize hardware
// D34/ ADC1 CH6 = Z
// D35/ ADC1 CH7 = Y
// D33/ ADC1 CH5 = X
#define HW_HIGHACCEL_X_PIN 33
#define HW_HIGHACCEL_Z_PIN 34
#define HW_HIGHACCEL_Y_PIN 35

#define HIGH_G_BUFFER_SIZE 128

class HighG {
    public:

        const uint8_t bufsize = HIGH_G_BUFFER_SIZE;

        uint16_t meas_X[HIGH_G_BUFFER_SIZE];
        uint16_t meas_Y[HIGH_G_BUFFER_SIZE];
        uint16_t meas_Z[HIGH_G_BUFFER_SIZE];
        uint32_t meas_T[HIGH_G_BUFFER_SIZE];
        uint8_t meas_ptr = 0;


        void interrupt();

        void setup();

        void process();

};

#endif