#include <Filters.h>
#include <AH/Timing/MillisMicrosTimer.hpp>
#include <Filters/Butterworth.hpp>

#define ONBOARD_LED  2

void setup() {
  Serial.begin(115200);
}

// Sampling frequency
const double f_s = 100; // Hz
// Cut-off frequency (-3 dB)
const double f_c = 25; // Hz
// Normalized cut-off frequency
const double f_n = 2 * f_c / f_s;

// Sample timer
Timer<micros> timer = std::round(1e6 / f_s);

// Sixth-order Butterworth filter
auto filter = butter<6>(f_n);

void loop() {
//   if (timer)
//     Serial.println(filter(analogRead(A0)));

delay(100);
  digitalWrite(ONBOARD_LED,HIGH);
  delay(100);
  digitalWrite(ONBOARD_LED,LOW);


}