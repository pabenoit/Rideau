#include <stdio.h>
#include <string.h>
#include <map>
#include <string>

#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <ESPmDNS.h>

#include <TimeLib.h>

#include "Configuration.h"

#define MAX_DEVICES 2

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

      MDNS.setInstanceName(systemCfg.cfg.wifi.mdns.c_str());      
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

  String sunsetText = String(sunSetOfTheday / 60) + ":" + (sunSetOfTheday % 60  ? "0" : "") + String(sunSetOfTheday % 60);
  root["Sunset"] = sunsetText.c_str();

  String sunriseText = String(sunRiseOfTheday / 60) + ":" + (sunRiseOfTheday % 60 < 10 ? "0" : "") + String(sunRiseOfTheday % 60);
  root["Sunrise"] = sunriseText.c_str();
  
  root["motorThresholdMax"] = systemCfg.cfg.motorThresholdMax;
  root["motorThreshold"] = systemCfg.cfg.motorThreshold;
  root["motorBlindTime"] = systemCfg.cfg.motorBlindTime;
  root["motorRunTimeLimit"] = systemCfg.cfg.motorRunTimeLimit;
  root["motorSpeed"] = systemCfg.cfg.motorSpeed;
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


// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
// Function to save the Configuration class to FLASH using Preferences
void Configuration::saveToFLASH() {
  Serial.println("Saving to FLASH");

  preferences.begin("Global");
  // Increment flash write count
  unsigned int fwCount = preferences.getUInt("fwCount");
  preferences.putUInt("fwCount", ++fwCount);
  preferences.end();

  preferences.begin("Config");
  preferences.putInt("mThrMax", cfg.motorThresholdMax);
  preferences.putInt("mThr", cfg.motorThreshold);
  preferences.putInt("mBlindT", cfg.motorBlindTime);
  preferences.putInt("mRunTLimit", cfg.motorRunTimeLimit);
  preferences.putInt("mSpeed", cfg.motorSpeed);
  preferences.end();

  preferences.begin("Location");
  preferences.putDouble("longitude", cfg.location.longitude); 
  preferences.putDouble("latitude", cfg.location.latitude); 
  preferences.putInt("tzOffset", cfg.timeZoneOffset);
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
  cfg.motorThresholdMax = preferences.getInt("mThrMax", 1600);
  cfg.motorThreshold = preferences.getInt("mThr", 850);
  cfg.motorBlindTime = preferences.getInt("mBlindT", 150);
  cfg.motorRunTimeLimit = preferences.getInt("mRunTLimit", 6000);
  cfg.motorSpeed = preferences.getInt("mSpeed", 50);
  preferences.end(); // End preferentces for Config

  preferences.begin("Location");  // Namespace for Location
  cfg.location.longitude = preferences.getDouble("longitude", 44.510202);
  cfg.location.latitude = preferences.getDouble("latitude", -73.564301);
  cfg.timeZoneOffset = preferences.getInt("tzOffset", -5);
  preferences.end();  // End preferences for Location

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

