#include <Arduino.h>
#include <ArduinoMDNS.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <sunset.h>
#include <SolarCalculator.h>
#include <HX711.h>

#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#else
#include <WiFiNINA.h>
#endif

#include "ntp.h"

#include "AsyncTimer.h"
#include "configData.h"
#include "configurationFile.h"
#include "curtainController.h"
#include "html.h"
#include "motor.h"
#include "utility.h"

#ifdef ARDUINO_UNOR4_WIFI
//#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#endif

#ifdef ARDUINO_UNOR4_WIFI
ArduinoLEDMatrix matrix;
#endif

#ifdef LOAD_CELL
HX711 scale;
#endif


WiFiServer server(80);
WiFiClient client = server.available();

// Allow to reach the Arduino by http://rideau.local
WiFiUDP mdnsUDP;
MDNS mdns(mdnsUDP);

SunSet sun;

// Prevents the curtain from operating indefinitely in case of an issue.
class AsyncTimer timer;

class Config config;
class CurtainController curtainController(WIFI_SSID, WIFI_PASSWORD);

int ropeTensionCur = 0;

/**
 * @brief Sets up the initial configuration and connections.
 * 
 * This function initializes the serial communication, load cell (if defined), curtain controller,
 * WiFi connection, mDNS service, and time synchronization. It performs the following tasks:
 * - Initializes the serial communication at 57600 baud rate.
 * - Sets up the load cell if the LOAD_CELL macro is defined.
 * - Calls the setup method of the curtain controller.
 * - Enables WiFi and connects to the network.
 * - Starts the web server and prints the WiFi status.
 * - Initializes the mDNS library to allow access via "rideau.local".
 * - Sets the position and timezone for the sun object.
 * - Synchronizes the time with an NTP server and prints the current date and time.
 */
void setup() {
  Serial.begin(57600);

#ifdef ARDUINO_UNOR4_WIFI
  matrix.begin();
  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.begin();
  matrix.play(true);
#endif

#ifdef LOAD_CELL
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
#endif

  // Update configuration value with the EEPROM values.
  config.readCfg2Epprom();

  curtainController.setup();

  //  while (!Serial);
  Serial.println("Serial ready");

  wifiEnable();
  listNetworks();
  wifiConnect();
  server.begin();
  wifiPrintStatus();

  // Initialize the mDNS library.
  // This allows the device to be reached via the host name "rideau.local".
  // Always call this before any other mDNS method!
  mdns.begin(WiFi.localIP(), MDNS_NAME);
  Serial.println("mDNS server ready");
  Serial.print("Access the server at: http://");
  Serial.print(MDNS_NAME);
  Serial.println(".local");

  // Start time synchronization
  // Set the geographical position (latitude and longitude) and the default timezone offset
  sun.setPosition(config.m_latitude, config.m_longitude, config.m_daylightSavingOffset);
  sun.setTZOffset(config.m_daylightSavingOffset);

  // Retreive the time from internet
  // https://playground.arduino.cc/Code/Time/
  Serial.println("Request NTP");
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 60);
  {
    char timeString[255];
    sprintf(timeString, "Date: %d/%d/%d", day(), month(), year());
    Serial.println(timeString);

    sprintf(timeString, "Heure: %d:%02d:%02d", hour(), minute(), second());
    Serial.println(timeString);
  }
}

/**
 * @brief Main loop function.
 * 
 * This function is called repeatedly in the Arduino main loop. It performs the following tasks:
 * - Reads the rope tension using either a load cell or an analog pin.
 * - Checks for incoming client connections and serves HTML content.
 * - Runs the Bonjour (mDNS) module to handle network service discovery.
 * - Executes the curtain controller's run method to manage curtain operations.
 * - Handles timer events.
 */
void loop() {

#ifdef LOAD_CELL
  // Read the rope tention
  if (scale.is_ready()) {
    ropeTensionCur = int(scale.get_value() / 500);
//    Serial.println(ropeTensionCur);
  }
#else
  int value = analogRead(CURRENT_MEASUREMENT_PIN);
  static int average[20] = { 0 };
  static int index = 0;
  static int avg = 0;

  // Average on 20 samples
  avg -= average[index];
  avg += value;
  average[index++] = value;
  index = index % 20;

  ropeTensionCur = abs((avg / 20) - 510);

//  if (index == 0)
//    Serial.println(ropeTensionCur);

#endif

  client = server.available();
  if (client) {
    Serial.println("Client connected");
    htmlRun(config, getTimeStr, getSunrise, getSunset);
  }

  // This actually runs the Bonjour module.
  // YOU HAVE TO CALL THIS PERIODICALLY, OR NOTHING WILL WORK!
  // Preferably, call it once per loop().
  mdns.run();

  curtainController.run();

  timer.handle(now());
}


/**
 * @brief Prints the current WiFi connection status.
 * 
 * This function prints the SSID of the connected network, the board's IP address,
 * the received signal strength (RSSI), and a message indicating the URL to access the board.
 */
void wifiPrintStatus() {
  // Print the SSID of the network you're connected to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Display received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

/**
 * @brief Enables the WiFi module and checks its status.
 * 
 * This function checks if the WiFi module is present and functioning correctly.
 * If the module is not found, it prints an error message and enters an infinite loop.
 * It also checks the firmware version of the WiFi module and prompts for an upgrade if the version is below 1.0.0.
 */
void wifiEnable() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Error: Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

/**
 * @brief Scans and lists nearby WiFi networks.
 * 
 * This function scans for nearby WiFi networks and prints the number of available networks,
 * along with their SSID and signal strength in dBm.
 * If it fails to get a WiFi connection, it will enter an infinite loop.
 */
void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a WiFi connection");
    while (true)
      ;
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.println(" dBm");
  }
}


/**
 * @brief Connects to the WiFi network.
 * 
 * This function attempts to connect to the WiFi network using the SSID and password
 * provided in the curtainController object. It will keep trying until a connection is established.
 * 
 * If the ARDUINO_UNOR4_WIFI macro is defined, it will also display a "Connecting ..." message on the matrix.
 */
void wifiConnect() {

//#ifdef ARDUINO_UNOR4_WIFI
//  matrix.stroke(255, 0, 0);
//  matrix.loadFrame(LEDMATRIX_DANGER);
//#endif

  // attempt to connect to Wifi network:
  while (curtainController.getWebStatus() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(curtainController.getWifiSsid());

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    curtainController.setWebStatus(WiFi.begin(curtainController.getWifiSsid(), curtainController.getWifiPassword()));

    // wait 10 seconds for connection:
    delay(10000);
  }

#ifdef ARDUINO_UNOR4_WIFI
  matrix.stroke(0, 255, 0);
  matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
#endif
}

/**
 * @brief Closes the curtain manually.
 * 
 * This function triggers the motor to move the curtain backward at a specified manual speed.
 * 
 * @param idx The index of the curtain to be closed.
 */
void closeManualCurtain(int idx) {
  curtainController.m_motorCurtain[idx].action(Motor::O_NORMAL, Motor::REVERSE, config.m_manualSpeed);
}

/**
 * @brief Opens the curtain manually.
 * 
 * This function triggers the motor to move the curtain forward at a specified manual speed.
 * 
 * @param idx The index of the curtain to be opened.
 */
void openManualCurtain(int idx) {
  curtainController.m_motorCurtain[idx].action(Motor::O_NORMAL, Motor::FORWARD, config.m_manualSpeed);
}

/**
 * @brief Closes the curtain automatically.
 * 
 * This function triggers the motor to move the curtain backward at a specified closing speed.
 * 
 * @param idx The index of the curtain to be closed.
 */
void closeAutoCurtain(int idx) {
  timer.cancel(curtainController.m_closeTimerId[idx]);

  if (config.curtain[idx].isCloseAtSunset)
    curtainController.m_closeTimerId[idx] = timer.setTimeout([idx]() {
      closeManualCurtain(idx);
      closeAutoCurtain(idx);
    },
                                                             int2time_t(static_cast<int>(sun.calcSunset())));

  if (config.curtain[idx].isCloseAtTime)
    curtainController.m_closeTimerId[idx] = timer.setTimeout([idx]() {
      closeManualCurtain(idx);
      closeAutoCurtain(idx);
    },
                                                             int2time_t(config.curtain[idx].closeAtTime));
}

/**
 * @brief Opens the curtain automatically.
 * 
 * This function triggers the motor to move the curtain forward at a specified opening speed.
 * 
 * @param idx The index of the curtain to be opened.
 */
void openAutoCurtain(int idx) {
  timer.cancel(curtainController.m_openTimerId[idx]);

  if (config.curtain[idx].isOpenAtSunrise)
    curtainController.m_openTimerId[idx] = timer.setTimeout([idx]() {
      openManualCurtain(idx);
      openAutoCurtain(idx);
    },
                                                            int2time_t(static_cast<int>(sun.calcSunrise())));

  if (config.curtain[idx].isOpenAtTime)
    curtainController.m_openTimerId[idx] = timer.setTimeout([idx]() {
      openManualCurtain(idx);
      openAutoCurtain(idx);
    },
                                                            int2time_t(config.curtain[idx].openAtTime));
}
