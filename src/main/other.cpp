#include "other.h"

#include <Arduino.h>
#include <cstdint>
#include <esp_attr.h>

void Other::interrupt() {
    
    meas_Temp[meas_ptr] = analogRead(HW_TEMPERATURE_PIN);
    meas_Vibration[meas_ptr] = analogRead(HW_VIBRATION_PIN);
    meas_T[meas_ptr] = millis();

    meas_ptr = (meas_ptr+1) % OTHER_BUFFER_SIZE;
}

void Other::process() {

}


void Other::setup() {


}
