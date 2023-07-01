#include <WiFi.h>
#include <math.h>
#include <cstdint>
#include <math.h>
// includes of scripts not necessary: all .ino-files are included automatically in alphabetical order.
#include "lowG.h"
#include "highG.h"
#include "other.h"

#include "sd_card.h"

#define ONBOARD_LED 2



#define MAIN_CPU_SPEED 40 // switch between 40 and 80 MHz

// #define TIME_LOGGER // switch timing related debugging info on/off


// initialization for serial-communication
uint_fast8_t fsm_serial = 0;
const uint8_t serial_buflen = 32;
const char serial_endl = '\n';
uint_fast8_t serial_bufidx = 0;
uint8_t serial_buffer[serial_buflen];

String command;
String parameter;

// initialize the measurement-objects.
// would be better to implement them as singletons.
HighG highG;
LowG lowG;
Other other;


// Settings for SD-Card
SD_Card sdCard;
File file;
uint_fast32_t last_flush_timestamp = 0;
const uint_fast16_t flush_interval = 1000; // milliseconds
// sd logging mode: 
// 0x01 = measurement-logging to SD-Card
// 0x02 = event-logging to SD-Card
// 0x04 = measurement-logging to Serial-console
// 0x08 = event-logging to Serial-console
// 0x10 = tris-logging to SD-Card
// 0x20 = tris-logging to Serial-console
uint_fast8_t sd_logging_mode = 0x3A; 

// Settings for analysis

uint_fast32_t last_analysis = 0;
uint_fast8_t fsm_sensorpack = 0;
uint_fast8_t fsm_sensorpack_last = 0;
const uint_fast32_t analysis_interval = 100; // milliseconds

uint_fast16_t treshold_magnitude = 3000;
uint_fast16_t treshold_magni_stdev_high = 100;
uint_fast16_t treshold_magni_stdev_low = 15;
double treshold_theta_stdev = 0.02;

int_fast32_t anal_Magni_X_outer = 0;
int_fast32_t anal_Magni_Y_outer = 0;
int_fast32_t anal_Magni_Z_outer = 0;
int_fast32_t anal_Vibration = 0;
int_fast32_t anal_Temperature = 0;

// Settings for third-loop
#define TRIS_BUFFER_SIZE 50
uint_fast32_t last_tris = 0;
uint_fast8_t tris_ptr = 0;

double tris_magni[TRIS_BUFFER_SIZE];
double tris_theta[TRIS_BUFFER_SIZE];
double tris_phi[TRIS_BUFFER_SIZE];

const uint_fast32_t tris_interval = 2500;
uint_fast8_t tris_update = 0;

double tris_magni_sum = 0;
double tris_phi_sum = 0;
double tris_theta_sum = 0;

double tris_magni_stdev = 0;
double tris_phi_stdev = 0;
double tris_theta_stdev = 0;

// initialize the interrupt-variables
hw_timer_t *interrupt = NULL;
uint_fast8_t fsm_measure_loop = 0;

uint_fast16_t measurement_count = 0;
const uint_fast16_t measurement_buffer = 5; // every n measurements they are written to SD-Card.

// fired with 200Hz. 
// see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html
void IRAM_ATTR interrupt_routine() {

    fsm_measure_loop = 1;
}

void check_serial() {
    if(99 == fsm_serial) {
        for(uint_fast8_t i = 0; i < serial_buflen; i++) {
            serial_buffer[i] = '\0';
        }
        serial_bufidx = 0;
        fsm_serial = 0;
    }

    while (Serial.available() > 0 && 0 == fsm_serial) {
        uint8_t buffer = Serial.read();
        if(buffer != serial_endl) {
            if(serial_bufidx < serial_buflen) { // ignore all longer inputs than the buffer size
                serial_buffer[serial_bufidx] = buffer;
                serial_bufidx++;
            }
        } else {
            fsm_serial = 1;
            serial_buffer[serial_bufidx] = '\0';
        }
    }

    
    if(1 == fsm_serial) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif
        // split the input line into command and parameter
        String serialstr;
        serialstr = String((char*) serial_buffer);
        uint_fast8_t delim = serialstr.indexOf(' ');

        if (delim != -1) {
            command = serialstr.substring(0, delim);
            parameter = serialstr.substring(delim + 1);
        } else {
            command = serialstr;
        }

        if(command.equals("time")) {
            Serial.print(">;time;");
            Serial.print(millis());
            Serial.println(";");
        } else if(command.equals("marker")) {
            file.print(parameter);
            file.print("\n");
            Serial.print(">;marker;");
            Serial.println(parameter);
        } else if(command.equals("trsh_magni")) {
            treshold_magnitude = parameter.toInt();
        } else if(command.equals("trsh_mstdev_h")) {
            treshold_magni_stdev_high = parameter.toInt();
        } else if(command.equals("trsh_mstdev_l")) {
            treshold_magni_stdev_low = parameter.toInt();
        } else if(command.equals("trsh_t_stdev")) {
            treshold_theta_stdev = parameter.toDouble();
        } else if(command.equals("write_prm")) {
            sd_logging_mode = parameter.toInt();
        } else if(command.equals("read_prm")) {
            char message[60];
            snprintf(message, sizeof(message), ">;stat;%d;%d;%d;%d;%lf\n",
                sd_logging_mode,
                treshold_magnitude, treshold_magni_stdev_high, treshold_magni_stdev_low,
                treshold_theta_stdev
            );
            Serial.print(message);
        } else if(command.equals("status")) {
            // return the last measurement-dataset
            char message[60];
            snprintf(message, sizeof(message), ">;meas;%d;%d;%d;%d;%d;%d;%d;%d;%d;\n",
                    highG.meas_T[highG.meas_ptr],
                    highG.meas_X[highG.meas_ptr], highG.meas_Y[highG.meas_ptr], highG.meas_Z[highG.meas_ptr],
                    lowG.meas_X[lowG.meas_ptr], lowG.meas_Y[lowG.meas_ptr], lowG.meas_Z[lowG.meas_ptr],
                    other.meas_Temp[other.meas_ptr], other.meas_Vibration[other.meas_ptr]
                );
            Serial.print(message);
        } else if(command.equals("statex")) {
            char message[60];
            snprintf(message, sizeof(message), ">;anal;%d;%d;%d;%d;%d;%d;%d\n",
                last_analysis, fsm_sensorpack,
                anal_Magni_X_outer, anal_Magni_Y_outer, anal_Magni_Z_outer,
                anal_Vibration, anal_Temperature
            );
            Serial.print(message);
            char message_two[70];
            snprintf(message_two, sizeof(message_two), ">;tris;%d;%lf;%lf;%lf;%lf;%lf;%lf\n",
                last_tris, 
                tris_magni_sum, tris_phi_sum, tris_theta_sum,
                tris_magni_stdev, tris_phi_stdev, tris_theta_stdev
            );
            Serial.print(message_two);
        } else if(command.equals("clear")) {
            digitalWrite(ONBOARD_LED, 1);
            file.close();
            sdCard.writeFile(SD, "/rawData.csv", "#;meas;ticks;xH;yH;zH;xL;yL;zL;magni;temp;vibr;\n#;anal;ticks;fsm_sensorpack;xMean;yMean;zMean;vibrMean;tempMean;\n");
            file = SD.open("/rawData.csv", FILE_APPEND);
            digitalWrite(ONBOARD_LED, 0);
        } else if(command.equals("dump")) {
            digitalWrite(ONBOARD_LED, 1);
            file.close();
            sdCard.readFile(SD, "/rawData.csv");
            file = SD.open("/rawData.csv", FILE_APPEND);
            digitalWrite(ONBOARD_LED, 0);
        }
        
        fsm_serial = 99;
        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();
            uint_fast32_t delta = t_2-t_1;
            Serial.print("serialop\t");
            Serial.println(delta);
        #endif
    }
}

void check_interrupt() {
    if(fsm_measure_loop) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif
        highG.interrupt();
        lowG.interrupt();
        other.interrupt();
        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();
            uint_fast32_t delta = t_2-t_1;
            Serial.print("tim_meas\t");
            Serial.println(delta);
        #endif
        measurement_count++;
        fsm_measure_loop = 0;
    }
}

void check_write_sd() {
    if(measurement_count >= measurement_buffer) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif
        if(!file) {
            File file = SD.open("/rawData.csv", FILE_APPEND);
        }
        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();
        #endif

        if(sd_logging_mode & 0x01 || sd_logging_mode & 0x04) { // write measurements to SD-Card
            for(uint_fast16_t q = measurement_count; q >0; q--) {
                uint8_t highG_ptr = (highG.meas_ptr + HIGH_G_BUFFER_SIZE - q) % HIGH_G_BUFFER_SIZE;
                uint8_t lowG_ptr = (lowG.meas_ptr + LOW_G_BUFFER_SIZE - q) % LOW_G_BUFFER_SIZE;
                uint8_t other_ptr = (other.meas_ptr + OTHER_BUFFER_SIZE - q) % OTHER_BUFFER_SIZE;

                char message[60];
                snprintf(message, sizeof(message), ">;meas;%d;%d;%d;%d;%d;%d;%d;%d;%d;\n",
                    highG.meas_T[highG_ptr],
                    highG.meas_X[highG_ptr], highG.meas_Y[highG_ptr], highG.meas_Z[highG_ptr],
                    lowG.meas_X[lowG_ptr], lowG.meas_Y[lowG_ptr], lowG.meas_Z[lowG_ptr],
                    other.meas_Temp[other_ptr], other.meas_Vibration[other_ptr]
                );
                if(sd_logging_mode & 0x01) {
                    auto status = file.print(message);
                }
                if(sd_logging_mode & 0x04) {
                    Serial.print(message);
                }
            }
        }

        if((sd_logging_mode & 0x02 || sd_logging_mode & 0x08) && fsm_sensorpack != fsm_sensorpack_last) {
            char message[60];
            snprintf(message, sizeof(message), ">;anal;%d;%d;%d;%d;%d;%d;%d\n",
                last_analysis, fsm_sensorpack,
                anal_Magni_X_outer, anal_Magni_Y_outer, anal_Magni_Z_outer,
                anal_Vibration, anal_Temperature
            );
            if(sd_logging_mode & 0x02) {
                auto status = file.print(message);
            }
            if(sd_logging_mode & 0x08) {
                Serial.print(message);
            }
            // fsm_sensorpack_last = fsm_sensorpack;
        }

        if((sd_logging_mode & 0x10 || sd_logging_mode & 0x20) && (fsm_sensorpack != fsm_sensorpack_last || tris_update)) {
            char message[100];
            snprintf(message, sizeof(message), ">;tris;%d;%lf;%lf;%lf;%lf;%lf;%lf\n",
                last_tris, 
                tris_magni_sum, tris_phi_sum, tris_theta_sum,
                tris_magni_stdev, tris_phi_stdev, tris_theta_stdev
            );
            if(sd_logging_mode & 0x10) {
                auto status = file.print(message);
            }
            if(sd_logging_mode & 0x20) {
                Serial.print(message);
            }

            tris_update = 0;
            fsm_sensorpack_last = fsm_sensorpack;
        }
        
        #ifdef TIME_LOGGER
            uint_fast32_t t_3 = micros();

            Serial.print("sd_fopen\t");
            Serial.println(t_2-t_1);
            Serial.print("str+upld\t");
            Serial.println(t_3-t_2);
        #endif

        measurement_count = 0;
    }
}

void check_flush() {
    if(millis() - last_flush_timestamp > flush_interval || millis() - last_flush_timestamp < 0) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif
        digitalWrite(ONBOARD_LED, 1);
        file.flush();
        last_flush_timestamp = millis();
        digitalWrite(ONBOARD_LED, 0);
        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();

            Serial.print("timflush\t");
            Serial.println(t_2-t_1);
        #endif
    }
}

void check_analyze() {
    if(millis() - last_analysis > analysis_interval || millis() - last_analysis < 0) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif

        // calculate average of measurement time-window
        anal_Magni_X_outer = 0;
        anal_Magni_Y_outer = 0;
        anal_Magni_Z_outer = 0;
        uint_fast8_t buffer_ptr = lowG.meas_ptr;
        for(uint_fast8_t i = 0; i < LOW_G_BUFFER_SIZE; i++) {
            uint_fast8_t buf_ptr = (buffer_ptr + i) % LOW_G_BUFFER_SIZE;

            anal_Magni_X_outer += lowG.meas_X[buf_ptr];
            anal_Magni_Y_outer += lowG.meas_Y[buf_ptr];
            anal_Magni_Z_outer += lowG.meas_Z[buf_ptr];
        }
        anal_Magni_X_outer = anal_Magni_X_outer / (float)(LOW_G_BUFFER_SIZE);
        anal_Magni_Y_outer = anal_Magni_Y_outer / (float)(LOW_G_BUFFER_SIZE);
        anal_Magni_Z_outer = anal_Magni_Z_outer / (float)(LOW_G_BUFFER_SIZE);
        
        anal_Vibration = 0;
        anal_Temperature = 0;
        buffer_ptr = other.meas_ptr;
        for(uint_fast8_t i = 0; i< OTHER_BUFFER_SIZE; i++) {
            uint_fast8_t buf_ptr = (buffer_ptr + i) % OTHER_BUFFER_SIZE;
            anal_Vibration += other.meas_Vibration[i];
            anal_Temperature += other.meas_Temp[i];
        }
        anal_Vibration = anal_Vibration / (float)(OTHER_BUFFER_SIZE);
        anal_Temperature = anal_Temperature / (float)(OTHER_BUFFER_SIZE);

        // if(anal_Magni_Z_outer < 300 && anal_Magni_Z_outer > -300) {
        //     fsm_sensorpack = 5; 
        // } else if(anal_Magni_Z_outer > 10000+treshold_z_axis) {
        //     fsm_sensorpack = 4;
        // } else if(anal_Magni_Z_outer < 10000-treshold_z_axis) {
        //     fsm_sensorpack = 3; // moving downwards
        // } else if(abs(anal_Magni_X_outer) > treshold_planar || abs(anal_Magni_Y_outer) > treshold_planar) {
        //     fsm_sensorpack = 2; // accelerated movement
        // } else if(anal_Vibration >= treshold_vibration) {
        //     fsm_sensorpack = 1; // movement planar
        // } else if(anal_Vibration < treshold_vibration) {
        //     fsm_sensorpack = 0; // standstill
        // }

        // Add data to the tris-buffer
        double magnitude = sqrt(anal_Magni_X_outer * anal_Magni_X_outer + anal_Magni_Y_outer * anal_Magni_Y_outer + anal_Magni_Z_outer * anal_Magni_Z_outer);
        double phi = acos(anal_Magni_Z_outer / magnitude);
        double theta = atan2(anal_Magni_Y_outer, anal_Magni_X_outer);

        tris_ptr = (tris_ptr + 1) % TRIS_BUFFER_SIZE;
        tris_magni[tris_ptr] = magnitude;
        tris_theta[tris_ptr] = theta;
        tris_phi[tris_ptr] = phi;


        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();

            Serial.print("analysis\t");
            Serial.println(t_2-t_1);
        #endif

        last_analysis = millis();
    }
}

void check_tris() {
    if(millis() - last_tris > tris_interval || millis() - last_tris < 0) {
        #ifdef TIME_LOGGER
            uint_fast32_t t_1 = micros();
        #endif

        tris_magni_sum = 0;
        tris_phi_sum = 0;
        tris_theta_sum = 0;

        for(int i = 0; i < TRIS_BUFFER_SIZE; i++) {
            uint_fast8_t buf_ptr = (tris_ptr + i) % TRIS_BUFFER_SIZE;
            tris_magni_sum += tris_magni[buf_ptr];
            tris_phi_sum += tris_phi[buf_ptr];
            tris_theta_sum += tris_theta[buf_ptr];
        }

        tris_magni_sum = tris_magni_sum / (double)TRIS_BUFFER_SIZE;
        tris_phi_sum = tris_phi_sum / (double)TRIS_BUFFER_SIZE;
        tris_theta_sum = tris_theta_sum / (double)TRIS_BUFFER_SIZE;

        tris_magni_stdev = 0;
        tris_phi_stdev = 0;
        tris_theta_stdev = 0;
        for(int i = 0; i < TRIS_BUFFER_SIZE; i++) {
            uint_fast8_t buf_ptr = (tris_ptr + i) % TRIS_BUFFER_SIZE;
            
            double magni_d = tris_magni[buf_ptr] - tris_magni_sum;
            tris_magni_stdev += magni_d * magni_d;
            double phi_d = tris_phi[buf_ptr] - tris_phi_sum;
            tris_phi_stdev += phi_d * phi_d;
            double theta_d = tris_theta[buf_ptr] - tris_theta_sum;
            tris_theta_stdev += theta_d * theta_d;
        }

        tris_magni_stdev = sqrt(tris_magni_stdev / TRIS_BUFFER_SIZE);
        tris_phi_stdev = sqrt(tris_phi_stdev / TRIS_BUFFER_SIZE);
        tris_theta_stdev = sqrt(tris_theta_stdev / TRIS_BUFFER_SIZE);

        if(tris_phi_sum > 0.5) {
            fsm_sensorpack = 6; // being tilted
        } else if(tris_magni_sum < treshold_magnitude) {
            fsm_sensorpack = 5; // free fall
        } else if(0 == 1) {
            fsm_sensorpack = 4; // moving upwards
        } else if(0 == 1) {
            fsm_sensorpack = 3; // moving downwards
        } else if(tris_magni_stdev > treshold_magni_stdev_high || tris_theta_stdev > treshold_theta_stdev) {
            fsm_sensorpack = 2; // accelerated movement
        } else if(tris_magni_stdev > treshold_magni_stdev_low) {
            fsm_sensorpack = 1; // slight movement
        } else if(tris_magni_stdev <= treshold_magni_stdev_low) {
            fsm_sensorpack = 0; // standstill
        }

        #ifdef TIME_LOGGER
            uint_fast32_t t_2 = micros();

            Serial.print("tris  \t");
            Serial.println(t_2-t_1);
        #endif

        tris_update = 1;
        last_tris = millis();
    }
}

void setup() {
    pinMode(ONBOARD_LED, OUTPUT);

    setLowPowerMode();
    Serial.begin(500000);

    Wire.setClock(1000000);
    Wire.begin(21, 22); // SDA, SCL

    highG.setup();
    lowG.setup();
    other.setup();

    sdCard.setup();
    sdCard.writeFile(SD, "/rawData.csv", "#;meas;ticks;xH;yH;zH;xL;yL;zL;magni;temp;vibr;\n#;anal;ticks;fsm_sensorpack;xMean;yMean;zMean;vibrMean;tempMean;\n");
    file = SD.open("/rawData.csv", FILE_APPEND);
    

    // create the 200Hz-Interrupt
    auto timer_cfg = timerBegin(0, 40, true);
    timerAttachInterrupt(timer_cfg, &interrupt_routine, true);
    timerAlarmWrite(timer_cfg, 5000, true);
    timerAlarmEnable(timer_cfg);
}



void loop() {
    check_interrupt(); // takes ~2500-3000µs
    check_serial(); // takes ~300µs per Dataset
    check_interrupt();
    check_flush(); // takes between 15 and 8000µs
    check_interrupt();
    check_write_sd(); // takes around 2000µs for 10 measurements every 10 measurements
    check_interrupt();
    check_analyze(); // 100µs
    check_interrupt();
    check_tris(); // run every few seconds
        
    
}

