#include <WiFi.h>
#include <esp_bt.h>


void setLowPowerMode() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    setCpuFrequencyMhz(40);
}