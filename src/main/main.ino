#include <WiFi.h>
#include <esp_bt.h>


// includes of scripts not necessary: all .ino-files are included automatically in alphabetical order.
//#include "util.cpp"
#include "lowG.h"
#include "highG.h"
#include "other.h"


// VIBR-Sensor: D32
// TEMP-Sensor: 


#define ONBOARD_LED 2

char buffer = 0;

bool toggle2 = true;

hw_timer_t *interrupt = NULL;

HighG highG;
LowG lowG;
Other other;



// IRAM_ATTR: see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html
void IRAM_ATTR interrupt_routine() {
    // digitalWrite(ONBOARD_LED, toggle2);
    // if(toggle2) {
    //     toggle2 = false;
    // } else {
    //     toggle2 = true;
    // }
}

void serial() {
    if (Serial.available() > 0) {
        buffer = Serial.read();
        if(buffer != '\n') {
            Serial.print("\n>>> recieved: ");
            Serial.print(buffer);
            Serial.print(" >> ");
            switch (buffer) {
                case 'a':
                    highG.process();
                    for(int i=0; i < highG.bufsize; i++) {
                        Serial.print(highG.meas_X[i]);
                        Serial.print(";\t");
                        Serial.print(highG.meas_Y[i]);
                        Serial.print(";\t");
                        Serial.print(highG.meas_Z[i]);
                        Serial.print(";\t");

                        Serial.print(lowG.meas_X[i]);
                        Serial.print(";\t");
                        Serial.print(lowG.meas_Y[i]);
                        Serial.print(";\t");
                        Serial.print(lowG.meas_Z[i]);
                        Serial.print(";\n");
                    }
                    Serial.println("############");
                    break;
                case 'b':
                    Serial.println("scan i2c!");
                    Serial.println("stauses:");
                    Serial.println(" -------------------");
                    Serial.println(" 0 = success");
                    Serial.println(" 1 = data too long");
                    Serial.println(" 2 = nack on addr-tx");
                    Serial.println(" 3 = nack on data-tx");
                    Serial.println(" 4 = other error");
                    Serial.println(" -------------------\n");
                    Serial.print("     ");
                    for(byte address = 1; address < 127; address++ ) {
                        Serial.print(" ");
                        if(address < 16) {
                            Serial.print("0");
                        }
                        Serial.print(address, HEX);
                        Wire.beginTransmission(address);
                        Serial.print("=");
                        int error = Wire.endTransmission(1);
                        if(error == 0) {
                          Serial.print("█");
                        } else {
                          Serial.print(error);
                        }
                        if(address %32 == 31) {
                            Serial.print("\n");
                        }
                    }
                    Serial.print("\n");
                default:
                    break;
            }
        }
    }
}


void setup() {
    // SETTINGS FOR LOW 
    setLowPowerMode();

    Wire.setClock(1000000);
    Wire.begin(21, 22); // SDA, SCL

    highG.setup();
    lowG.setup();
    other.setup();

    // pinMode(ONBOARD_LED, OUTPUT);

    Serial.begin(115200);


    // auto timer_cfg = timerBegin(0, 40, true);
    // timerAttachInterrupt(timer_cfg, &interrupt_routine, true);
    // timerAlarmWrite(timer_cfg, 20000, true); // 20ms => 50Hz
    // timerAlarmEnable(timer_cfg);

}

void loop() {
    serial();
    uint32_t t_1 = micros();
    highG.interrupt();
    lowG.interrupt();
    other.interrupt();
    uint32_t t_2 = micros();
    uint32_t delta = t_2-t_1;
    Serial.print("t=");
    Serial.println(delta);
}

