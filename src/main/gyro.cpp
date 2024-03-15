#include "gyro.h"

#include <Arduino.h>
#include <cstdint>
#include <esp_attr.h>


void Gyro::interrupt() {
    Wire.beginTransmission(HW_GYRO_I2C_ADR);
    Wire.write(0x3B);
    Wire.endTransmission(0);
    // delay(1);
    Wire.requestFrom(HW_GYRO_I2C_ADR, 14, 1);
    // while (Wire.available() == 0){};
    
    meas_X[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_Y[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_Z[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_R[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_A[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_B[meas_ptr] = Wire.read()<<8 | Wire.read();
    meas_C[meas_ptr] = Wire.read()<<8 | Wire.read();

    meas_T[meas_ptr] = millis();

    meas_ptr = (meas_ptr+1) % GYRO_BUFFER_SIZE;
}

void Gyro::process() {

}


void Gyro::setup() {

    Wire.setClock(100000);
    Wire.begin(21, 22); // SDA, SCL

    // Set the Power Management Register
    Wire.beginTransmission(HW_GYRO_I2C_ADR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(1);

}
