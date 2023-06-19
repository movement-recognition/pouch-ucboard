## Hardware


## Flashing the microcontroller



## General settings in the software

The system-clock of the ESP32-Microcontroller is also reduced to save some power. Reducing it's frequency from 80MHz down to 40MHz saves around 40mA. Because our application does not make use of possible wireless communications, the wifi and bluetooth modules are shut down as well. this also frees up the second Analog-to-Digital-Converter on the µC-Processor-Board.

## Data Processing flow
The sensor data should be aquired in frequency as high as possible. Because the pure aquisition-process takes about 2500-300µs, the aquisition should happen in a 5ms-raster (200Hz).

It wasn't possible to aquire all the sensor data within an interrupt call. Therefor the interrupt function only toggles the `fsm_measure_loop` variable to 1. In the main loop, a measurement is only initiated when this value is 1. After a measurement, this value is reset to 0.

Main cause for that is the longeivity of the I²C-Communication process. The analog registers can be read within a few hundred microseconds.
The rest of the implementation is written in a non-blocking style (whereever possible) for that reason.
In the main loop, the processing functions (named `check_xxxx()` are called alternatinly with the sensor-data-aquisition-function `check_interrupt()`. That quarantees a call of the interrupt-function every 5ms in most cases.
This raster can't be guaranteed: Some atomar functions like opening or closing Files on the SD-Card take about 8 milliseconds. The impact of that is reduced to a minimum by keeping files open if feasible.

The aquired data is written to a ring buffer. A buffer consists of an array with a fixed length (in our case 128 elements) and a pointer that can store an index-value pointing to one of the elements in the array. After each write-process, the pointer is incremented to point to the next empty element in the list. If the end of the list is reached, the pointer is reset to the first element in the list. This is realized with the use of the modulo-operator (`%`). To store multiple Data-Series in parallel, multipe arrays share the same pointer. In this implementation this is the case for every instance of the three Sensor-Classes: One "highG" for the Analog ADXL377, one "lowG" for the I²C-controlled ADXL 345 and a third one (called "other") reading the Vibration- and Temperature sensors.


## Data analysis
Every 100 milliseconds the data is read from the ring-buffers and processed. The buffer keeps around 640ms worth of values. Because most events are of around this length, all values are summed up and divided to get their mean values. Multiple tests have shown, that these simple values deliver the best results for further interpretation compared to their calculation cost.
If the values of the Accelerometer in the z-axis deviate from the default 10m²/s-Value by more than 1g (default value, can be reconfigured), the `fsm_sensorpack`-statemachine switches to the "movement upwards" or "movement downwards", depending on the sensor measuring 11g's or only 9g's. On the other hand, the statemachine switches to "free fall"-mode if all three axis summed up don't measure any relevant amount of acceleration (default value: <0.3g). Detecting, if the sensor board is moving or not is a little bit tricky. Therefor it combines the measurements of the X and Y accelerometer-axis with the value of the vibration sensor. This sensor is debounced heavily before using it's values - they are quite similar to a PWM-Signal so it's duty cycle is determined by averaging over the whole 640ms as well.

Caused by the difference of the buffer-length (640ms) and the call-rate (100ms), each raw-datapoint is processed at least six times during it's lifetime. this guarantees, that spikes or events happening at the front or end of the buffer can be detected by forward-looking-algorithms that may be used later.

## Serial communication

Possible commands are:

- `time` : returns the current microprocessor-time in milliseconds
- `trsh_plnr`: sets the planar-threshold
- `trsh_vibr`: sets the vibration-threshold
- `trsh_zaxs`: sets the treshold for detections in the z-layer
- `status`: 
- `log_mode`: sets the logging mode (first byte = write all measurements to sd, second byte = write all events to sd-card). defaults to 0x02.

- `clear`: clears the data-archive on the sd-card. should be called after all data has been dumped
- `dump`: dumps all contents on the sd-card to the serial console. during that process, analysis-precision cannot be guaranteed.

All lines to be parsed by a python-script on the "raspberry pi"-side start with the `>;` part and close with the newline-character `\\n`.