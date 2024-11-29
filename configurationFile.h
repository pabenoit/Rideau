#ifndef CONFIGURATION_FILE_H
#define CONFIGURATION_FILE_H

#define MESURE_ROPE_TENSION_BALANCE

// List of all default configuration variables
// You only need to modify them here

// Used by EEPROM to verify data in flash match the version
#define VERSION 2

// WIFI Information to Connect
#define WIFI_SSID "RFBP"
#define WIFI_PASSWORD "CE1736A5"

// Allows access to the web server using this domain name instead of an IP address
// Example: http://curtain.local
#define MDNS_NAME "srideau"

// Location used to calculate the sunrise and sunset for Montreal
#define LATITUDE 45.508888
#define LONGITUDE -73.561668
#define DAYLIGHT_SAVING_OFFSET -5

// Default to 7:00 AM (7*60 minutes)
#define DEFAULT_OPEN_TIME (7 * 60)

// Default to 10:00 PM (22*60 minutes)
#define DEFAULT_CLOSE_TIME (22 * 60)

#define IS_CLOSE_AT_SUNSET true
#define IS_CLOSE_AT_TIME false

// Hardware Connections
// -------------------
// Motor 1 (Left) connection
// Pin 2 - wire yellow - motor control
// Pin 3 - wire orange  - motor control
#define MOTOR1_PIN1 3
#define MOTOR1_PIN2 2

// Motor 2 (Right) connection
// Pin 4 - wire green  - motor control
// Pin 5 - wire blue   - motor control
#define MOTOR2_PIN1 5
#define MOTOR2_PIN2 4

// Use measure rope tension
// HX711 Measure the current
#ifdef MESURE_COURANT
#define CURRENT_MEASUREMENT_PIN A5
#define MAX_TENSION_THRESHOLD 25
#endif

#ifdef MESURE_ROPE_TENSION_BALANCE
// Use HX711 Balance
// Purple wire DT connected to pin D6
// White wire SCK connected to pin D7
#define LOADCELL_DOUT_PIN 8
#define LOADCELL_SCK_PIN 9
#define MAX_TENSION_THRESHOLD 50
#endif

constexpr uint32_t LEDMATRIX_ARROW_RIGHT[] = {0x800, 0xc3fe3ff3, 0xfe00c008, 66};
constexpr uint32_t LEDMATRIX_ARROW_LEFT[] = {0x10030, 0x7fcffc7, 0xfc300100, 66};
constexpr uint32_t LEDMATRIX_ARROW_STOP[] = {0xe01b, 0x3182083, 0x181b00e0, 66};

#endif // CONFIGURATION_FILE_H
