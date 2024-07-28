// List of all default configuration variables
// You only need to modify them here

// WIFI Information to Connect
#define WIFI_SSID     "RFBP"
#define WIFI_PASSWORD "CE1736A5"

// Allows access to the web server using this domain name instead of an IP address
// Example: http://rideau.local
#define MDNS_NAME "rideau"

// Location used to calculate the sunrise and sunset for Montreal
#define LATITUDE 45.508888
#define LONGITUDE -73.561668
#define DAYLIGHT_SAVING_OFFSET -4

// Opening speed 1-255
#define OPENING_SPEED 120
#define CLOSING_SPEED 120
#define MANUAL_SPEED 255

// Default to 7:00 AM (7*60 minutes)
#define DEFAULT_OPEN_AT_TIME (7*60)

// Default to 10:00 PM (22*60 minutes)
#define DEFAULT_CLOSE_AT_TIME (22*60)

#define IS_CLOSE_AT_SUNSET true
#define IS_CLOSE_AT_TIME   false

// Hardware Connections
// -------------------
// Motor 1 connection
// Pin 0 - wire orange - Used to control the speed 
// Pin 1 - wire yellow - motor control
// Pin 2 - wire green  - motor control
#define MOTOR_1_PIN_ENA 0
#define MOTOR_1_PIN1    1
#define MOTOR_1_PIN2    2

// Motor 2 connection
// Pin 3 - wire blue    - Used to control the speed 
// Pin 4 - wire purple  - motor control
// Pin 5 - wire white   - motor control
#define MOTOR_2_PIN_ENA 3
#define MOTOR_2_PIN1    4
#define MOTOR_2_PIN2    5

#define MAX_TENTION_TRESHOLD 100

// HX711 circuit wiring
// Used by HX711 to measure the tention
#define LOADCELL_DOUT_PIN 6 
#define LOADCELL_SCK_PIN  7