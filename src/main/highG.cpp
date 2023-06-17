#include "highG.h"

#include <Arduino.h>
#include <cstdint>
#include <esp_attr.h>




void HighG::interrupt() {
    meas_X[meas_ptr] = analogRead(HW_HIGHACCEL_X_PIN);
    meas_Y[meas_ptr] = analogRead(HW_HIGHACCEL_Y_PIN);
    meas_Z[meas_ptr] = analogRead(HW_HIGHACCEL_Z_PIN);
    meas_T[meas_ptr] = millis();

    meas_ptr = (meas_ptr+1) % HIGH_G_BUFFER_SIZE;
}

void HighG::process() {

}


void HighG::setup() {
    // analogReadResolution(12);
    // analogSetCycles(4);
    // analogSetAttenuation(ADC_0db);    

}
