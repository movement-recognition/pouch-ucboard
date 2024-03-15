#ifndef GYRO_H
#define GYRO_H


#include <stdint.h>
#include <Arduino.h>
#include <esp_attr.h>
#include <Wire.h>

// initialize hardware
#define HW_GYRO_SDA_PIN D21
#define HW_GYRO_SCL_PIN D22
#define HW_GYRO_I2C_ADR 0x68

#define GYRO_BUFFER_SIZE 128

class Gyro {
    public:

        const uint8_t bufsize = GYRO_BUFFER_SIZE;

        int16_t meas_X[GYRO_BUFFER_SIZE];
        int16_t meas_Y[GYRO_BUFFER_SIZE];
        int16_t meas_Z[GYRO_BUFFER_SIZE];

        int16_t meas_R[GYRO_BUFFER_SIZE]; // temperature data

        int16_t meas_A[GYRO_BUFFER_SIZE];
        int16_t meas_B[GYRO_BUFFER_SIZE];
        int16_t meas_C[GYRO_BUFFER_SIZE];

        uint32_t meas_T[GYRO_BUFFER_SIZE];
        uint8_t meas_ptr = 0;


        void interrupt();

        void setup();

        void process();

    private:
        int16_t convSignedInt(uint8_t msb, uint8_t lsb);

};


#endif