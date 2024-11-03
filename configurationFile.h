// List of all default configuration variables
// You only need to modify them here

// Use by EEPROM to verify data in flash match teh version
#define VERSION 1

// WIFI Information to Connect
#define WIFI_SSID "RFBP"
#define WIFI_PASSWORD "CE1736A5"

// Allows access to the web server using this domain name instead of an IP address
// Example: http://rideau.local
#define MDNS_NAME "frideau"

// Location used to calculate the sunrise and sunset for Montreal
#define LATITUDE 45.508888
#define LONGITUDE -73.561668
#define DAYLIGHT_SAVING_OFFSET -4

// Opening speed 1-255
#define OPENING_SPEED 255
#define CLOSING_SPEED 255
#define MANUAL_SPEED 255

// Default to 7:00 AM (7*60 minutes)
#define DEFAULT_OPEN_TIME (7 * 60)

// Default to 10:00 PM (22*60 minutes)
#define DEFAULT_CLOSE_TIME (22 * 60)

#define IS_CLOSE_AT_SUNSET true
#define IS_CLOSE_AT_TIME false

// Hardware Connections
// -------------------
// Motor 1 connection
// Pin 0 - wire orange - Used to control the speed
// Pin 1 - wire yellow - motor control
// Pin 2 - wire green  - motor control
#define MOTOR1_PIN_ENA 2
#define MOTOR1_PIN1 3
#define MOTOR1_PIN2 4

// Motor 2 connection
// Pin 3 - wire blue    - Used to control the speed
// Pin 4 - wire purple  - motor control
// Pin 5 - wire white   - motor control
#define MOTOR2_PIN_ENA 5
#define MOTOR2_PIN1 6
#define MOTOR2_PIN2 7

#define MAX_TENSION_THRESHOLD 25

// Use motor current to measure the tension
// ACS720 
#define CURRENT_MEASUREMENT_PIN A5
