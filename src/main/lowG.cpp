#include "lowG.h"

#include <Arduino.h>
#include <cstdint>
#include <esp_attr.h>

// #include <Adafruit_ADXL345_U.h>
// #include <Adafruit_Sensor.h>



void LowG::interrupt() {
    Wire.beginTransmission(HW_LOCACCEL_I2C_ADR);
    Wire.write(0x32);
    Wire.endTransmission(1);
    // delay(1);
    Wire.requestFrom(HW_LOCACCEL_I2C_ADR, 6);
    while (Wire.available() == 0){};

    byte dataXL = Wire.read();
    byte dataXM = Wire.read();
    byte dataYL = Wire.read();
    byte dataYM = Wire.read();
    byte dataZL = Wire.read();
    byte dataZM = Wire.read();
    
    meas_X[meas_ptr] = convSignedInt(dataXM, dataXL) * 39;
    meas_Y[meas_ptr] = convSignedInt(dataYM, dataYL) * 39;
    meas_Z[meas_ptr] = convSignedInt(dataZM, dataZL) * 39;
    meas_T[meas_ptr] = millis();

    meas_ptr = (meas_ptr+1) % LOW_G_BUFFER_SIZE;
}

void LowG::process() {

}


void LowG::setup() {

    Wire.setClock(100000);
    Wire.begin(21, 22); // SDA, SCL

    // Set the POWER_CTL-Register
    Wire.beginTransmission(HW_LOCACCEL_I2C_ADR);
    Wire.write(0x2D);
    Wire.write(0x0B); // 00001011 = 11 = B
    Wire.endTransmission(1);

    // Set the RATE-Register
    Wire.beginTransmission(HW_LOCACCEL_I2C_ADR);
    Wire.write(0x2C);
    // Wire.write(0x0C); // 0x0000 1100 => normal operation, rate-value of 1100 (=400Hz)
    Wire.write(0x0F); // rate-value of 3.2kHz
    Wire.endTransmission(1);

    // Set the DATA_FORMAT-Register
    Wire.beginTransmission(HW_LOCACCEL_I2C_ADR);
    Wire.write(0x31);
    Wire.write(0x0A);
    Wire.endTransmission(1);

}

int16_t LowG::convSignedInt(uint8_t msb, uint8_t lsb) {
    int16_t value = (msb << 8) | lsb;
    if (value & 0x8000) {
        value = -((~value & 0xFFFF) + 1);
    }
    return value;
}