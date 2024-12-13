#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <unordered_map>

extern int sunSetOfTheday;
extern int sunRiseOfTheday;

// Class definitions
class Location
{
 public:
  // Montreal
  Location() : longitude(44.510202), latitude(-73.564301) {}

  double longitude;
  double latitude;
};

class Wifi
{
 public:
  Wifi() : ssid("HomeWiFi"), password("Password"), mdns("frideau") {}

  std::string ssid;
  std::string password;
  std::string mdns;
};

class Automation
{
 public:
  Automation() : action(static_cast<int>(Action::Open)), 
                 type(static_cast<int>(Type::Sun)), 
                 time("6:30"), 
                 offset(0), 
                 status(static_cast<int>(Status::Enable)) {}

  Automation(int action, int type, std::string time, int offset, int status)
      : action(action), type(type), time(time), offset(offset), status(status)
  {
  }

  int getActionTime(void)
  {
    // Set to sun rise or sun set by default
    int reqTime = (action == static_cast<int>(Automation::Action::Open)) ? sunRiseOfTheday : sunSetOfTheday;
    if (type == static_cast<int>(Automation::Type::SpecificTime))
    {
      // Specific time
      reqTime = (std::stoi(time.substr(0, 2)) * 60) + std::stoi(time.substr(3, 5));
      reqTime += offset;
    }

    return reqTime;
  }

  enum class Action
  {
    Open = 0,
    Close = 1
  };

  enum class Type
  {
    SpecificTime = 0,
    Sun = 1
  };

  enum class Status
  {
    Enable = 0,
    Disable = 1
  };

  int action;                     // 0|1  -> open=0 | close=1
  int type;                       // 0|1 -> Specific_time=0 | sun=1
  std::string time;               // format: "HHMMSS"
  int offset;                     // -120|+120 hour
  int status;                     // 0|1 -> enable=0 | disable=1
  bool allReadyRunToday = false;  // true if the automation has already run today
};

class Device
{
 public:
  Device() : name("") {}  // Default constructor
  Device(std::string name) : name(name) {}

  std::string name;
  std::unordered_map<int, Automation> automations;
};

class Config
{
 public:
    Config() : thresholdMaxCurrent(850),timeZoneOffset(-5 /*Montreal*/) {}

  int thresholdMaxCurrent;
  int timeZoneOffset;
  Location location;
  Wifi wifi;
  std::unordered_map<int, Device> devices;
};

class Configuration
{
 public:
  Configuration() {}

  bool createDevice(const char *jsonString);
  bool modifyDevice(const char *jsonString);
  bool deleteDevice(const char *jsonString);

bool createAutomation(const char *jsonString, int &automationId);
  bool modifyAutomation(const char *jsonString);
  bool deleteAutomation(const char *jsonString);

  bool modifyWifiLocation(const char *jsonString);

  std::string GetAllInfo();

  void loadFromEEPROM();
  void saveToEEPROM();

  void resetListAction();
  void applyListAction(void (*applyAction)(int deviceId, int action));

  Config cfg;
};

#endif
