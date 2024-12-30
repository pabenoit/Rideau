#include <stdio.h>
#include <string.h>
#include <map>
#include <string>

#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <Preferences.h>

#include <TimeLib.h>

#include "Configuration.h"

#define MAX_DEVICES 2
#define nameof(var) #var  // Convert variable name to a string

extern Configuration systemCfg;

Preferences preferences;

bool Configuration::createDevice(const char *jsonString)
{
  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (systemCfg.cfg.devices.size() >= MAX_DEVICES)
  {
    Serial.println("Error: Maximum number of devices reached");
    return false;
  }

  int deviceId = 0;
  while (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
  {
    ++deviceId;
  }

  if (json.hasOwnProperty("deviceName"))
  {
    Device newDevice((const char *)json["deviceName"]);
    systemCfg.cfg.devices.insert(std::make_pair(deviceId, newDevice));
    rc = true;
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::modifyDevice(const char *jsonString)
{
  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("deviceId") && json.hasOwnProperty("properties"))
  {
    int deviceId = (int)json["deviceId"];

    JSONVar properties = json["properties"];

    if (properties.hasOwnProperty("deviceName"))
    {
      if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
      {
        systemCfg.cfg.devices[deviceId].name = (const char *)properties["deviceName"];
        rc = true;
      }
      else
      {
        Serial.print("Error: Device does not exist: ");
        Serial.println(deviceId);
      }
    }
    else
    {
      Serial.println("Error: Reading deviceName");
    }
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::deleteDevice(const char *jsonString)
{
  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("deviceId"))
  {
    int deviceId = (int)json["deviceId"];

    if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
    {
      systemCfg.cfg.devices.erase(deviceId);
      rc = true;
    }
    else
    {
      Serial.println("Error: Device does not exist");
    }
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::createAutomation(const char *jsonString, int &automationId)
{
  // {"deviceId":4,"automation":{"automationId":0,"action":0,"type":1,"time":"12:00","offset":0,"status":0}}

  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("deviceId") && json.hasOwnProperty("automation"))
  {
    int deviceId = (int)json["deviceId"];
    JSONVar automation = json["automation"];
    if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
    {
      if (automation.hasOwnProperty("action") && automation.hasOwnProperty("type") &&
          automation.hasOwnProperty("time") && automation.hasOwnProperty("offset") &&
          automation.hasOwnProperty("status"))
      {
        automationId = 0;
        while (systemCfg.cfg.devices[deviceId].automations.find(automationId) !=
               systemCfg.cfg.devices[deviceId].automations.end())
        {
          ++automationId;
        }

        Automation newAutomation((int)automation["action"],
                                 (int)automation["type"],
                                 (const char *)automation["time"],
                                 (int)automation["offset"],
                                 (int)automation["status"]);

        newAutomation.allReadyRunToday = ((hour() * 60) + minute() > newAutomation.getActionTime());
        systemCfg.cfg.devices[deviceId].automations.insert(std::make_pair(automationId, newAutomation));

        rc = true;
      }
      else
      {
        Serial.println("Error: JSON item does not exist");
      }
    }
    else
    {
      Serial.println("Error: DeviceId does not exist");
    }
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::modifyAutomation(const char *jsonString)
{
  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("deviceId") && json.hasOwnProperty("automation"))
  {
    int deviceId = (int)json["deviceId"];
    JSONVar automation = json["automation"];
    if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
    {
      if (automation.hasOwnProperty("automationId") && automation.hasOwnProperty("action") &&
          automation.hasOwnProperty("type") && automation.hasOwnProperty("time") &&
          automation.hasOwnProperty("offset") && automation.hasOwnProperty("status"))
      {
        int automationId = (int)automation["automationId"];
        auto &automations = systemCfg.cfg.devices[deviceId].automations;
        if (automations.find(automationId) != automations.end())
        {
          automations[automationId].action = (int)automation["action"];
          automations[automationId].type = (int)automation["type"];
          automations[automationId].time = (const char *)automation["time"];
          automations[automationId].offset = (int)automation["offset"];
          automations[automationId].status = (int)automation["status"];

          automations[automationId].allReadyRunToday =
              ((hour() * 60) + minute() > automations[automationId].getActionTime());
          rc = true;
        }
        else
        {
          Serial.println("Error: automationId does not exist");
        }
      }
      else
      {
        Serial.println("Error: JSON item does not exist");
      }
    }
    else
    {
      Serial.println("Error: DeviceId does not exist");
    }
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::deleteAutomation(const char *jsonString)
{
  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("deviceId") && json.hasOwnProperty("automationId"))
  {
    int deviceId = (int)json["deviceId"];
    int automationId = (int)json["automationId"];

    if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
    {
      Device &device = systemCfg.cfg.devices[deviceId];
      if (device.automations.find(automationId) != device.automations.end())
      {
        device.automations.erase(automationId);
        rc = true;
        Serial.println("Automation deleted successfully");
      }
      else
      {
        Serial.println("Error: Automation does not exist");
      }
    }
    else
    {
      Serial.println("Error: Device does not exist");
    }
  }
  else
  {
    Serial.println("Error: Missing JSON entry");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

bool Configuration::modifyWifiLocation(const char *jsonString)
{
  // {"location":{"longitude":22.2,"latitude":-73.561668},"wifi":{"ssid":"","password":"","mdns":"curtains"}}

  bool rc = false;
  JSONVar json = JSON.parse(jsonString);
  if (JSON.typeof(json) == "undefined")
  {
    Serial.println("Error: Parsing JSON");
    return false;
  }

  if (json.hasOwnProperty("location"))
  {
    JSONVar location = json["location"];
    if (location.hasOwnProperty("longitude") && location.hasOwnProperty("latitude"))
    {
      systemCfg.cfg.location.longitude = location["longitude"];
      systemCfg.cfg.location.latitude = location["latitude"];
      rc = true;
    }
    else
    {
      Serial.println("Error: longitude or latitude does not exist");
    }
  }
  else
  {
    Serial.println("Error: location does not exist");
  }

  if (json.hasOwnProperty("wifi"))
  {
    JSONVar wifi = json["wifi"];
    if (wifi.hasOwnProperty("ssid") && wifi.hasOwnProperty("password") && wifi.hasOwnProperty("mdns"))
    {
      systemCfg.cfg.wifi.ssid = (const char *)wifi["ssid"];
      systemCfg.cfg.wifi.password = (const char *)wifi["password"];
      systemCfg.cfg.wifi.mdns = (const char *)wifi["mdns"];
      rc = true;
    }
    else
    {
      Serial.println("Error: ssid, password or mdns does not exist");
    }
  }
  else
  {
    Serial.println("Error: wifi does not exist");
  }

  if (rc == true)
    saveToFLASH();

  return rc;
}

std::string Configuration::GetAllInfo()
{
  // Create the root JSON object
  JSONVar root;

  String dateString = String(day()) + "/" + String(month()) + "/" + String(year());
  root["date"] = dateString.c_str();

  String timeString = String(hour()) + ":" + String(minute()) + ":" + String(second());
  root["time"] = timeString.c_str();

  String sunsetText = String(sunSetOfTheday / 60) + ":" + String(sunSetOfTheday % 60);
  root["Sunset"] = sunsetText.c_str();

  String sunriseText = String(sunRiseOfTheday / 60) + ":" + String(sunRiseOfTheday % 60);
  root["Sunrise"] = sunriseText.c_str();

  root["thresholdMaxCurrent"] = systemCfg.cfg.thresholdMaxCurrent;
  root["timeZoneOffset"] = systemCfg.cfg.timeZoneOffset;

  // Add "location" object
  JSONVar location;
  location["longitude"] = systemCfg.cfg.location.longitude;
  location["latitude"] = systemCfg.cfg.location.latitude;
  root["location"] = location;

  // Add "wifi" object
  JSONVar wifi;
  wifi["ssid"] = systemCfg.cfg.wifi.ssid.c_str();
  wifi["password"] = systemCfg.cfg.wifi.password.c_str();
  wifi["mdns"] = systemCfg.cfg.wifi.mdns.c_str();
  //   wifi["ip"] = WiFi.localIP();
  root["wifi"] = wifi;

  // Add "devices" array
  JSONVar devicesArray;

  // Loop through each device and add to JSON
  int deviceIdx = 0;
  for (auto deviceIt = systemCfg.cfg.devices.begin(); deviceIt != systemCfg.cfg.devices.end(); ++deviceIt)
  {
    int deviceId = deviceIt->first;
    const Device &device = deviceIt->second;

    JSONVar deviceObject;
    deviceObject["deviceId"] = deviceId;
    deviceObject["deviceName"] = device.name.c_str();

    JSONVar automationsArray;

    // Loop through each automation and add to JSON
    int automationIdx = 0;
    for (auto automationIt = device.automations.begin(); automationIt != device.automations.end(); ++automationIt)
    {
      int automationId = automationIt->first;
      const Automation &automation = automationIt->second;

      JSONVar automationObject;
      automationObject["automationId"] = automationId;
      automationObject["action"] = automation.action;
      automationObject["type"] = automation.type;
      automationObject["time"] = automation.time.c_str();
      automationObject["offset"] = automation.offset;
      automationObject["status"] = automation.status;

      automationsArray[automationIdx++] = automationObject;
    }

    deviceObject["automations"] = automationsArray;
    devicesArray[deviceIdx++] = deviceObject;
  }

  root["devices"] = devicesArray;

  // Convert the JSON object to a string
  std::string result = JSON.stringify(root).c_str();

  return result;
}

#if 0 
// Function to save the Configuration class to EEPROM
void Configuration::saveToEEPROM()
{
  Serial.println("Saving to EEPROM");
  int addr = 0;

  EEPROM.put(addr, cfg.thresholdMaxCurrent);
  addr += sizeof(cfg.thresholdMaxCurrent);

  EEPROM.put(addr, cfg.timeZoneOffset);
  addr += sizeof(cfg.timeZoneOffset);

  // Save location
  EEPROM.put(addr, cfg.location.latitude);
  addr += sizeof(cfg.location.latitude);

  EEPROM.put(addr, cfg.location.longitude);
  addr += sizeof(cfg.location.longitude);

  // Save wifi
  int nameLength = cfg.wifi.ssid.length();
  EEPROM.put(addr, nameLength);
  Serial.print("Write - ssid.length : ");
  Serial.println(nameLength);

  addr += sizeof(nameLength);

  for (char c : cfg.wifi.ssid)
  {
    EEPROM.put(addr, c);
    addr += sizeof(char);
  }
  
  nameLength = cfg.wifi.password.length();
  EEPROM.put(addr, nameLength);
  addr += sizeof(nameLength);

  for (char c : cfg.wifi.password)
  {
    EEPROM.put(addr, c);
    addr += sizeof(char);
  }
    
  nameLength = cfg.wifi.mdns.length();
  EEPROM.put(addr, nameLength);
  addr += sizeof(nameLength);

  for (char c : cfg.wifi.mdns)
  {
    EEPROM.put(addr, c);
    addr += sizeof(char);
  }

  // Save number of devices
  int numDevices = cfg.devices.size();
  EEPROM.put(addr, numDevices);
  addr += sizeof(numDevices);

  // Save each device
  for (const auto &devicePair : cfg.devices)
  {
    int deviceId = devicePair.first;
    const Device &device = devicePair.second;

    // Save device ID
    EEPROM.put(addr, deviceId);
    addr += sizeof(deviceId);

    // Save device name length and name
    nameLength = device.name.length();
    EEPROM.put(addr, nameLength);
    addr += sizeof(nameLength);

    for (char c : device.name)
    {
      EEPROM.put(addr, c);
      addr += sizeof(char);
    }

    // Save number of automations
    int numAutomations = device.automations.size();
    EEPROM.put(addr, numAutomations);
    addr += sizeof(numAutomations);

    // Save each automation
    for (const auto &automationPair : device.automations)
    {
      int automationId = automationPair.first;
      const Automation &automation = automationPair.second;

      // Save automation ID
      EEPROM.put(addr, automationId);
      addr += sizeof(automationId);

      // Save automation data
      EEPROM.put(addr, automation.action);
      addr += sizeof(automation.action);

      EEPROM.put(addr, automation.type);
      addr += sizeof(automation.type);

      int timeLength = automation.time.length();
      EEPROM.put(addr, timeLength);

      addr += sizeof(timeLength);
      for (char c : automation.time)
      {
        EEPROM.put(addr, c);
        addr += sizeof(char);
      }

      EEPROM.put(addr, automation.offset);
      addr += sizeof(automation.offset);

      EEPROM.put(addr, automation.status);
      addr += sizeof(automation.status);
    }
  }

  EEPROM.commit();
}

// Function to load the Configuration class from EEPROM
void Configuration::loadFromEEPROM()
{
  Serial.println("Load to EEPROM");

  int addr = 0;

  EEPROM.get(addr, cfg.thresholdMaxCurrent);
  addr += sizeof(cfg.thresholdMaxCurrent);

  EEPROM.get(addr, cfg.timeZoneOffset);
  addr += sizeof(cfg.timeZoneOffset);

  // Load location
  EEPROM.get(addr, cfg.location.latitude);
  addr += sizeof(cfg.location.latitude);

  EEPROM.get(addr, cfg.location.longitude);
  addr += sizeof(cfg.location.longitude);

  // Load wifi
  int nameLength;
  EEPROM.get(addr, nameLength);
  addr += sizeof(nameLength);
  cfg.wifi.ssid = "";
  Serial.print("namelength : ");
  Serial.println(nameLength);
  Serial.print("char : ");
  for (int j = 0; j < nameLength; ++j)
  {
    char c;
    EEPROM.get(addr, c);
    addr += sizeof(char);
    cfg.wifi.ssid.push_back(c);
    Serial.print(c);
  }
  Serial.println();
  
  EEPROM.get(addr, nameLength);
  addr += sizeof(nameLength);
  cfg.wifi.password = "";
  Serial.print("namelength : ");
  Serial.println(nameLength);
  Serial.print("char : ");
  for (int j = 0; j < nameLength; ++j)
  {
    char c;
    EEPROM.get(addr, c);
    addr += sizeof(char);
    cfg.wifi.password.push_back(c);
    Serial.print(c);
  }
   Serial.println();

  EEPROM.get(addr, nameLength);
  addr += sizeof(nameLength);
  cfg.wifi.mdns = "";
  Serial.print("namelength : ");
  Serial.println(nameLength);
  Serial.print("char : ");
  for (int j = 0; j < nameLength; ++j)
  {
    char c;
    EEPROM.get(addr, c);
    addr += sizeof(char);
    cfg.wifi.mdns.push_back(c);
    Serial.print(c);
  }
  Serial.println();


  // Load number of devices
  int numDevices;
  EEPROM.get(addr, numDevices);
  addr += sizeof(numDevices);

  // Load each device
  for (int i = 0; i < numDevices; ++i)
  {
    int deviceId;
    Device device;

    // Load device ID
    EEPROM.get(addr, deviceId);
    addr += sizeof(deviceId);

    // Load device name length and name
    int nameLength;
    EEPROM.get(addr, nameLength);

    addr += sizeof(nameLength);
    device.name = "";
    for (int j = 0; j < nameLength; ++j)
    {
      char c;
      EEPROM.get(addr, c);
      addr += sizeof(char);
      device.name.push_back(c);
    }

    // Load number of automations
    int numAutomations;
    EEPROM.get(addr, numAutomations);
    addr += sizeof(numAutomations);

    // Load each automation
    for (int j = 0; j < numAutomations; ++j)
    {
      int automationId;
      Automation automation;

      // Save automation ID
      EEPROM.get(addr, automationId);
      addr += sizeof(automationId);

      // Save automation data
      EEPROM.get(addr, automation.action);
      addr += sizeof(automation.action);

      EEPROM.get(addr, automation.type);
      addr += sizeof(automation.type);

      int timeLength;
      EEPROM.get(addr, timeLength);

      addr += sizeof(timeLength);
      automation.time = "";
      for (int j = 0; j < timeLength; ++j)
      {
        char c;
        EEPROM.get(addr, c);
        addr += sizeof(char);
        automation.time.push_back(c);
      }

      EEPROM.get(addr, automation.offset);
      addr += sizeof(automation.offset);

      EEPROM.get(addr, automation.status);
      addr += sizeof(automation.status);

      device.automations.insert(std::make_pair(automationId, automation));
    }

    cfg.devices.insert(std::make_pair(deviceId, device));
  }
}
#endif

// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
// Function to save the Configuration class to FLASH using Preferences
void Configuration::saveToFLASH() {
  Serial.println("Saving to FLASH");

  preferences.begin("Global");
  // Increment flash write count
  unsigned int fwCount = preferences.getUInt("fwCount");
  preferences.putUInt("fwCount", ++fwCount);

  preferences.begin("Config");
  preferences.putInt("thrsCurMax", cfg.thresholdMaxCurrent);
  preferences.putInt("tzOffset", cfg.timeZoneOffset);
  preferences.end();

  preferences.begin("Location");
  preferences.putDouble("longitude", cfg.location.longitude); 
  preferences.putDouble("latitude", cfg.location.latitude); 
  preferences.end();

  preferences.begin("Wifi");
  preferences.putString("ssid", cfg.wifi.ssid.c_str());
  preferences.putString("password", cfg.wifi.password.c_str());
  preferences.putString("mdns", cfg.wifi.mdns.c_str());
  preferences.end();

  preferences.begin("Devices");
  preferences.putInt("count", cfg.devices.size());
  preferences.end();

  // Save each device in its own namespace
  int deviceIndex = 0;
  for (const auto &devicePair : cfg.devices) {
    const Device &device = devicePair.second;
    String namespaceName = "Device_" + String(devicePair.first);  // Unique namespace for each device
    preferences.begin(namespaceName.c_str());  // Start device namespace

    preferences.putInt("id", devicePair.first);  // Save device id
    preferences.putString("name", device.name.c_str());  // Save device name
    preferences.putInt("autoCnt", device.automations.size());  // Save number of automations

    int automationIndex = 0;
    for (const auto &automationPair : device.automations) {
      const Automation &automation = automationPair.second;

      String autoKey = "auto_" + String(automationIndex);  // Dynamic key for each automation
      preferences.putInt((autoKey + "_id").c_str(), automationPair.first);
      preferences.putInt((autoKey + "_act").c_str(), automation.action);
      preferences.putInt((autoKey + "_typ").c_str(), automation.type);
      preferences.putString((autoKey + "_time").c_str(), automation.time.c_str());
      preferences.putInt((autoKey + "_off").c_str(), automation.offset);
      preferences.putInt((autoKey + "_sts").c_str(), automation.status);

      automationIndex++;
    }

    preferences.end();  // End device namespace
    deviceIndex++;
  }
}

// Function to load the Configuration class from FLASH using Preferences
void Configuration::loadFromFLASH() {
  Serial.println("Loading from FLASH");

  preferences.begin("Config"); // Namespace for Config
  cfg.thresholdMaxCurrent = preferences.getInt("thrsCurMax", 850);
  cfg.timeZoneOffset = preferences.getInt("tzOffset", -5);
  preferences.end();

  preferences.begin("Location");  // Namespace for location
  cfg.location.longitude = preferences.getDouble("longitude", 44.510202);
  cfg.location.latitude = preferences.getDouble("latitude", -73.564301);
  preferences.end();  // End preferences for location

  preferences.begin("Wifi");  // Namespace for WiFi
  cfg.wifi.ssid = preferences.getString("ssid", "RFBP").c_str();
  cfg.wifi.password = preferences.getString("password", "CE1736A5").c_str();
  cfg.wifi.mdns = preferences.getString("mdns", "esp32").c_str();
  preferences.end();  // End preferences for WiFi

  preferences.begin("Devices");  // Namespace for devices metadata
  int numDevices = preferences.getInt("count", 0);  // Load number of devices
  preferences.end();

  // Load each device from its own namespace
  for (int deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
    String namespaceName = "Device_" + String(deviceIndex);  // Unique namespace for each device
    preferences.begin(namespaceName.c_str());  // Start device namespace

    int deviceId = preferences.getInt("id", -1);
    String deviceName = preferences.getString("name", "");
    int numAutomations = preferences.getInt("autoCnt", 0);

    Device device;
    device.name = deviceName.c_str();

    for (int automationIndex = 0; automationIndex < numAutomations; automationIndex++) {
      String autoKey = "auto_" + String(automationIndex);  // Dynamic key for each automation

      int automationId = preferences.getInt((autoKey + "_id").c_str(), -1);
      int action = preferences.getInt((autoKey + "_act").c_str(), 0);
      int type = preferences.getInt((autoKey + "_typ").c_str(), 0);
      String time = preferences.getString((autoKey + "_time").c_str(), "");
      int offset = preferences.getInt((autoKey + "_off").c_str(), 0);
      int status = preferences.getInt((autoKey + "_sts").c_str(), 0);

      Automation automation(action, type, time.c_str(), offset, status);
      device.automations.insert(std::make_pair(automationId, automation));
    }

    cfg.devices.insert(std::make_pair(deviceId, device));
    preferences.end();  // End device namespace
  }

  //Create the devices if they dont already exist
  if (numDevices = 0)
  {
    systemCfg.cfg.devices.insert(std::make_pair(0, Device("Drapes")));
    systemCfg.cfg.devices.insert(std::make_pair(1, Device("Curtains")));
  }
}


#if 0
void saveJSONToFile() {
  // Create a JSON document (this could be dynamic depending on your needs)
  StaticJsonDocument<200> doc;
  doc["sensor"] = "temperature";
  doc["value"] = 23.4;
  doc["unit"] = "C";

  // Open the file for writing (overwrite if it exists)
  File file = SPIFFS.open(filePath, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Serialize the JSON to the file
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write JSON to file");
  } else {
    Serial.println("JSON written to file successfully");
  }

  file.close();
}

void readJSONFromFile() {
  // Open the file for reading
  File file = SPIFFS.open(filePath, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Create a JSON document to store the data
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read JSON from file");
  } else {
    // Read values from the JSON object
    const char* sensor = doc["sensor"];
    float value = doc["value"];
    const char* unit = doc["unit"];

    // Print values to Serial Monitor
    Serial.print("Sensor: ");
    Serial.println(sensor);
    Serial.print("Value: ");
    Serial.println(value);
    Serial.print("Unit: ");
    Serial.println(unit);
  }

  file.close();
}
#endif

void Configuration::IncrementBootCount()
{
  unsigned int bootCnt = preferences.getUInt("bootCnt");
  preferences.putUInt("bootCnt", ++bootCnt);
  preferences.end();
}


void Configuration::resetListAction()
{
  for (auto deviceIt = systemCfg.cfg.devices.begin(); deviceIt != systemCfg.cfg.devices.end(); ++deviceIt)
  {
    Device &device = deviceIt->second;
    for (auto automationIt = device.automations.begin(); automationIt != device.automations.end(); ++automationIt)
    {
      Automation &automation = automationIt->second;
      automation.allReadyRunToday = false;
    }
  }
}

void Configuration::applyListAction(void (*applyAction)(int deviceId, int action))
{
  for (auto &devicePair : systemCfg.cfg.devices)
  {
    Device &device = devicePair.second;
    for (auto &automationPair : device.automations)
    {
      Automation &automation = automationPair.second;
      if (automation.status == static_cast<int>(Automation::Status::Enable) && !automation.allReadyRunToday)
      {
        int actionTime = automation.getActionTime();
        if ((hour() * 60) + minute() >= actionTime)
        {
          automation.allReadyRunToday = true;
          Serial.print("Process ACTION :");

          Serial.print(devicePair.first);
          Serial.print(" - ");
          Serial.print(device.name.c_str());
          Serial.print(" - ");
          Serial.print(automationPair.first);
          Serial.print(" - ");
          Serial.print(automation.action == 0 ? " \tOpen   " : " \tClose  ");
          Serial.println((String((actionTime) / 60) + ":" + String((actionTime) % 60)).c_str());

          applyAction(devicePair.first, automation.action);
        }
      }
    }
  }
}

