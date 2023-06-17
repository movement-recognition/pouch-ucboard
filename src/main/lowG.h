#ifndef ADXL345_H
#define ADXL345_H


#include <stdint.h>
#include <Arduino.h>
#include <esp_attr.h>
#include <Wire.h>

// initialize hardware
#define HW_LOWACCEL_SDA_PIN D21
#define HW_LOWACCEL_SCL_PIN D22
#define HW_LOCACCEL_I2C_ADR 0x53

#define LOW_G_BUFFER_SIZE 100

class LowG {
    public:

        const uint8_t bufsize = LOW_G_BUFFER_SIZE;

        int16_t meas_X[LOW_G_BUFFER_SIZE];
        int16_t meas_Y[LOW_G_BUFFER_SIZE];
        int16_t meas_Z[LOW_G_BUFFER_SIZE];
        uint32_t meas_T[LOW_G_BUFFER_SIZE];
        uint8_t meas_ptr = 0;


        void interrupt();

        void setup();

        void process();

    private:
        int16_t convSignedInt(uint8_t msb, uint8_t lsb);

};


#endif