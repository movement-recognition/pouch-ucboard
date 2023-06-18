#include <WiFi.h>
#include <cstdint>
#include <esp_bt.h>


void setLowPowerMode() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    setCpuFrequencyMhz(MAIN_CPU_SPEED);
}

void scanI2C() {
    Serial.println("scanning i2c-bus!");
    Serial.println("stauses:");
    Serial.println(" -------------------");
    Serial.println(" 0 = success (0=█, better visibility)");
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
}
