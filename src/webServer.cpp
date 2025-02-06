#include <stdio.h>
#include <string.h>
#include <vector>

// #include <Arduino.h>
#include <Arduino_JSON.h>
#include <TimeLib.h>
#include <WiFi.h>

#include "Configuration.h"
#include "utility.h"

// Webserver page
//  #include "webpage/index_html.h"
#include "webpage/index_di15Ebn2_js.h"

#define DEBUG 1  // Set to 0 to disable debug prints

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// Declare the server variable as external if it is defined in another file
extern WiFiServer server;
extern Configuration systemCfg;

std::tuple<bool, int, int> processHttpRequest(WiFiClient &client, const String &requestHeader, const String &data);
std::tuple<bool, int, int> handleHttpRequest();
void handleGet_GetAllInfo(WiFiClient &client);
void handleGet_index_html(WiFiClient &client);
void handleGet_index_GBR06sQK_js(WiFiClient &client);
void handleGet_slash(WiFiClient &client);
void processPut_createDevice(WiFiClient &client, String data);
std::tuple<bool, int, int> processPost_actionDevice(WiFiClient &client, String data);
std::tuple<bool, int, int> processPost_motorConfig(WiFiClient &client, String data);
void processPut_createAutomation(WiFiClient &client, String data);
void processPatch_modifyDevice(WiFiClient &client, String data);
void processPatch_modifyAutomation(WiFiClient &client, String data);
void processPatch_modifyConfig(WiFiClient &client, String data);
void processDelete_deleteDevice(WiFiClient &client, String data);
void processDelete_deleteAutomation(WiFiClient &client, String data);
void sendNotFound(WiFiClient &client);
std::string generateGraphHtml(WiFiClient &client);

std::tuple<bool, int, int> processHttpRequest(WiFiClient &client, const String &requestHeader, const String &data)
{
  std::tuple<bool, int, int> rc = std::make_tuple(false, 0, 0);

  String method = "";
  // Determine the HTTP method
  if (requestHeader.startsWith("OPTIONS "))
  {
    method = "OPTIONS";
  }
  else if (requestHeader.startsWith("GET "))
  {
    method = "GET";
  }
  else if (requestHeader.startsWith("PUT "))
  {
    method = "PUT";
  }
  else if (requestHeader.startsWith("PATCH "))
  {
    method = "PATCH";
  }
  else if (requestHeader.startsWith("DELETE "))
  {
    method = "DELETE";
  }
  else if (requestHeader.startsWith("POST "))
  {
    method = "POST";
  }

  // Process requests based on method and URL
  if (method == "GET")
  {
    if (requestHeader.indexOf("GET /config") >= 0)
    {
      handleGet_GetAllInfo(client);
    }
    else if (requestHeader.indexOf("GET /index.html") >= 0)
    {
      handleGet_index_html(client);
    }
    else if (requestHeader.indexOf("GET /index-di15Ebn2.js") >= 0)
    {
      handleGet_index_GBR06sQK_js(client);
    }
    else if (requestHeader.indexOf("GET /graph") >= 0)
    {
      client.println("HTTP/1.1 200 OK");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      std::string response = generateGraphHtml(client);
      client.print("Content-Length: ");
      client.println(response.length() + 2);
      client.println();
      client.println(response.c_str());
    }
    else if (requestHeader.indexOf("GET /open1") >= 0)
    {
      handleGet_slash(client);
      rc = std::make_tuple(true, 0, 0);
    }
    else if (requestHeader.indexOf("GET /close1") >= 0)
    {
      rc = std::make_tuple(true, 0, 1);
      handleGet_slash(client);
    }
    else if (requestHeader.indexOf("GET /log") >= 0)
    {
      if (requestHeader.indexOf("GET /config?threshold=") >= 0)
      {
        String key = "?threshold=";
        int pos = requestHeader.indexOf(key);
        pos += key.length();
        String numberStr = requestHeader.substring(pos);
        int spacePos = numberStr.indexOf(' ');  // Find the space after the number
        numberStr = numberStr.substring(0, spacePos);
        int threshold = numberStr.toInt();
        DEBUG_PRINT("Extracted number: ");
        DEBUG_PRINTLN("");
        DEBUG_PRINTLN(threshold);

        // Extract the threshold value
        // int threshold = requestHeader.substring(requestHeader.indexOf("GET /?threshold=") + 15).toInt();
        systemCfg.cfg.motorThresholdMax = threshold;
        systemCfg.saveToFLASH();
      }
      handleGet_slash(client);
    }
    else if (requestHeader.indexOf("GET /") >= 0)
    {
      handleGet_index_html(client);
    }
    else
    {
      DEBUG_PRINTLN("Error: GET  request not found");
      DEBUG_PRINTLN(">> Request: ");
      DEBUG_PRINTLN(requestHeader.c_str());
      DEBUG_PRINTLN(">> End Request: ");
      sendNotFound(client);
    }
  }
  else if (method == "OPTIONS")
  {
    // To allowed cross Authorisation
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("====== OPTIONS ======");

    client.println("HTTP/1.1 204 No Content");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE, PATCH");
    client.println("Access-Control-Allow-Headers: Authorization, Content-Type");
    client.println();
  }
  else if (method == "PUT")
  {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("====== PUT ======");

    if (requestHeader.indexOf("PUT /device HTTP/1.1") >= 0)
      processPut_createDevice(client, data);
    else if (requestHeader.indexOf("PUT /automation HTTP/1.1") >= 0)
      processPut_createAutomation(client, data);
    else
    {
      DEBUG_PRINTLN("Error: PUT  request not found");
      DEBUG_PRINTLN(">> Request: ");
      DEBUG_PRINTLN(requestHeader.c_str());
      DEBUG_PRINTLN(">> End Request: ");
      sendNotFound(client);
    }
  }
  else if (method == "PATCH")
  {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("====== PATCH ======");

    if (requestHeader.indexOf("PATCH /device HTTP/1.1") >= 0)
      processPatch_modifyDevice(client, data);
    else if (requestHeader.indexOf("PATCH /automation HTTP/1.1") >= 0)
      processPatch_modifyAutomation(client, data);
    else if (requestHeader.indexOf("PATCH /config HTTP/1.1") >= 0)
      processPatch_modifyConfig(client, data);
    else
    {
      DEBUG_PRINTLN("Error: PATCH  request not found");
      DEBUG_PRINTLN(">> Request: ");
      DEBUG_PRINTLN(requestHeader.c_str());
      DEBUG_PRINTLN(">> End Request: ");
      sendNotFound(client);
    }
  }
  else if (method == "DELETE")
  {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("====== DELETE ======");

    if (requestHeader.indexOf("DELETE /device HTTP/1.1") >= 0)
      processDelete_deleteDevice(client, data);
    else if (requestHeader.indexOf("DELETE /automation HTTP/1.1") >= 0)
      processDelete_deleteAutomation(client, data);
    else
    {
      DEBUG_PRINTLN("Error: DELETE  request not found");
      DEBUG_PRINTLN(">> Request: ");
      DEBUG_PRINTLN(requestHeader.c_str());
      DEBUG_PRINTLN(">> End Request: ");
      sendNotFound(client);
    }
  }
  else if (method == "POST")
  {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("====== POST ======");

    if (requestHeader.indexOf("POST /action HTTP/1.1") >= 0)
      rc = processPost_actionDevice(client, data);
    else if (requestHeader.indexOf("POST /updateField HTTP/1.1") >= 0)
    {
      rc = processPost_motorConfig(client, data);
    }
    else
    {
      DEBUG_PRINTLN("Error: POST  request not found");
      DEBUG_PRINTLN(">> Request: ");
      DEBUG_PRINTLN(requestHeader.c_str());
      DEBUG_PRINTLN(">> End Request: ");
      sendNotFound(client);
    }
  }
  else
  {
    DEBUG_PRINTLN("Error: Method not supported");
    DEBUG_PRINTLN(">> Request: ");
    DEBUG_PRINTLN(requestHeader.c_str());
    DEBUG_PRINTLN(">> End Request: ");
    sendNotFound(client);
  }

  return rc;
}

std::tuple<bool, int, int> handleHttpRequest()
{
  std::tuple<bool, int, int> rc = std::make_tuple(false, 0, 0);

  // Listen for incoming clients
  WiFiClient client = server.available();

  if (client)
  {
    DEBUG_PRINTLN("New client connected");
    String requestHeader = "";
    String data = "";
    int contentLength = 0;

    bool isDataSection = false;

    while (client.connected())
    {
      if (client.available())
      {
        if (isDataSection == false)
        {
          // Read headers
          char c = client.read();
          Serial.print(c);
          requestHeader += c;

          // Header end with a white line
          if (requestHeader.endsWith("\r\n\r\n"))
          {
            isDataSection = true;

            // Extract the content length
            int contentLengthIndex = requestHeader.indexOf("Content-Length:");

            if (contentLengthIndex != -1)
            {
              int valueStartIndex = contentLengthIndex + 15;  // Length of "Content-Length:"
              int valueEndIndex = requestHeader.indexOf("\r\n", valueStartIndex);
              String contentLengthValue = requestHeader.substring(valueStartIndex, valueEndIndex);
              contentLength = contentLengthValue.toInt();
            }
          }
        }

        if (isDataSection == true && contentLength > 0)
        {
          // Read data
          char c = client.read();
          data += c;
          contentLength--;
        }

        if (isDataSection == true && contentLength == 0)
        {
          break;
        }
      }
    }

    if (requestHeader.length() > 0)
      rc = processHttpRequest(client, requestHeader, data);

    client.stop();
    DEBUG_PRINTLN("Client disconnected");
  }

  return rc;
}

void handleGet_GetAllInfo(WiFiClient &client)
{
  std::string responce = systemCfg.GetAllInfo();

  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  // +2 for CR and LF
  client.println(responce.length() + 2);
  client.println("Connection: close");
  client.println();
  client.println(responce.c_str());
}

void processPatch_modifyConfig(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for PATCH /configuration: " + data);

  if (systemCfg.modifyWifiLocation(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
}

#if 0
void handleGet_index_html(WiFiClient &client)
{
  std::string responce = systemCfg.GetAllInfo();

  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  // +2 for CR and LF
  client.println(responce.length() + 2);
  client.println("Connection: close");
  client.println();
  client.println(responce.c_str());
}
#endif

void handleGet_slash(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<html><body><h1>Welcome to RIDEAU product!</h1><pre>");

  String dateString = String(day()) + "/" + String(month()) + "/" + String(year());
  client.print("Date : ");
  client.println(dateString);

  String timeString = String(hour()) + ":" + String(minute()) + ":" + String(second());
  client.print("Time : ");
  client.println(timeString);

  client.print("time Zone Offset : ");
  client.println(systemCfg.cfg.timeZoneOffset);

  client.println("");
  client.println("\nSunset  : " + String(sunSetOfTheday / 60) + ":" + String(sunSetOfTheday % 60));

  String sunriseText = String(sunRiseOfTheday / 60) + ":" + String(sunRiseOfTheday % 60);
  client.print("Sunrise : ");
  client.println(sunriseText.c_str());

  client.println("\nMAX CURRENT THRESHOLD");
  client.print("  thresholdMaxCurrent : ");
  client.println(systemCfg.cfg.motorThresholdMax);

  client.println("\nLOCATION");
  client.print("  latitude : ");
  client.println(systemCfg.cfg.location.latitude);
  client.print("  longitude : ");
  client.println(systemCfg.cfg.location.longitude);

  client.println("\nWIFI");
  client.print("  ssid : ");
  client.println(systemCfg.cfg.wifi.ssid.c_str());
  client.print("  password : ");
  client.println(systemCfg.cfg.wifi.password.c_str());
  client.print("  mdns : ");
  client.println(systemCfg.cfg.wifi.mdns.c_str());

  client.println("\nLIST ACTION");
  client.println("State    DeviceId    AutoId    Action    Status    Time");

  for (const auto &devicePair : systemCfg.cfg.devices)
  {
    for (const auto &automationPair : devicePair.second.automations)
    {
      const Automation &automation = automationPair.second;
      client.print(automation.status == static_cast<int>(Automation::Status::Enable) ? "Enable   " : "Disable  ");
      client.print("    ");
      client.print(devicePair.first);
      client.print("          ");
      client.print(automationPair.first);
      client.print("        ");
      client.print(automation.action == 0 ? "Open     " : "Close    ");
      client.print(automation.allReadyRunToday ? "done      " : "todo      ");

      const int timeAction = automation.getActionTime();
      String timeText = String((timeAction) / 60) + ":" + String((timeAction) % 60);
      client.println(timeText.c_str());
    }
  }

  client.println("");
  for (const auto &devicePair : systemCfg.cfg.devices)
  {
    int deviceId = devicePair.first;
    const Device &device = devicePair.second;

    client.print(deviceId);
    client.print(" - ");
    client.println(device.name.c_str());

    client.println("      id action type time offset status allReadyRunToday");

    for (const auto &automationPair : device.automations)
    {
      int automationId = automationPair.first;
      const Automation &automation = automationPair.second;

      client.print("      ");
      client.print(automationId);
      client.print(automation.action == 0 ? " - Open  " : " - Close ");
      client.print(automation.type == 0 ? "- Time " : "- Sun  ");
      client.print(automation.time.c_str());
      client.print(" - ");
      client.print(automation.offset);
      client.print(" - ");
      client.print(automation.status == 0 ? "Enable " : "Disable");
      client.println(automation.allReadyRunToday ? " - true" : " - false");
    }
  }
  client.println("</pre></html></body>");
}

void processPut_createDevice(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for PUT /createDevice: " + data);

  if (systemCfg.createDevice(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

void processPatch_modifyDevice(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for PATCH /modifyDevice: " + data);
  if (systemCfg.modifyDevice(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

void processDelete_deleteDevice(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for DELETE /deleteDevice: " + data);

  if (systemCfg.deleteDevice(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

std::tuple<bool, int, int> processPost_actionDevice(WiFiClient &client, String data)
{
  std::tuple<bool, int, int> rc = std::make_tuple(false, 0, 0);

  DEBUG_PRINTLN("\n\nData received for POST /action: " + data);

  JSONVar json = JSON.parse(data.c_str());
  if ((JSON.typeof(json) != "undefined") && (json.hasOwnProperty("deviceId")) && (json.hasOwnProperty("action")))
  {
    int deviceId = (int)json["deviceId"];
    int action = (int)json["action"];

    if (systemCfg.cfg.devices.find(deviceId) != systemCfg.cfg.devices.end())
    {
      rc = std::make_tuple(true, deviceId, action);

      client.println("HTTP/1.1 200 OK");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Content-Type: application/json");
      client.println();
      return rc;
    }
  }

  client.println("HTTP/1.1 400 Bad Request");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/html");
  client.println();
  return rc;
}

std::tuple<bool, int, int> processPost_motorConfig(WiFiClient &client, String data)
{
  std::tuple<bool, int, int> rc = std::make_tuple(false, 0, 0);

  DEBUG_PRINTLN("\n\nData received for POST /motorConfig: " + data);

  JSONVar json = JSON.parse(data.c_str());
  if ((JSON.typeof(json) != "undefined") && (json.hasOwnProperty("key")) && (json.hasOwnProperty("value")))
  {
    String key = String((const char *)json["key"]);
    int value = (int)json["value"];

    Serial.print("Key: ");
    Serial.println(key);
    Serial.print("Value: ");
    Serial.println(value);

    if (key == "motorThresholdMax")
      systemCfg.cfg.motorThresholdMax = value;
    if (key == "motorThreshold")
      systemCfg.cfg.motorThreshold = value;
    if (key == "motorBlindTime")
      systemCfg.cfg.motorBlindTime = value;
    if (key == "motorRunTimeLimit")
      systemCfg.cfg.motorRunTimeLimit = value;
    if (key == "motorSpeed")
      systemCfg.cfg.motorSpeed = value;

    systemCfg.saveToFLASH();

    rc = std::make_tuple(false, 0, 0);

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
    return rc;
  }

  client.println("HTTP/1.1 400 Bad Request");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/html");
  client.println();
  return rc;
}

void processPut_createAutomation(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for PUT /createAutomation: " + data);

  int automationId = 0;
  if (systemCfg.createAutomation(data.c_str(), automationId))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    // +2 for CR and LF
    JSONVar root;
    root["automationId"] = automationId;
    std::string responce = JSON.stringify(root).c_str();

    client.println(responce.length() + 2);
    client.println("Connection: close");
    client.println();
    client.println(responce.c_str());
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

void processPatch_modifyAutomation(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for PATCH /automation: " + data);

  if (systemCfg.modifyAutomation(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

void processDelete_deleteAutomation(WiFiClient &client, String data)
{
  DEBUG_PRINTLN("\n\nData received for DELETE /automation: " + data);

  if (systemCfg.deleteAutomation(data.c_str()))
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: application/json");
    client.println();
  }
  else
  {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/html");
    client.println();
  }
}

void sendNotFound(WiFiClient &client)
{
  DEBUG_PRINTLN("\n\nReceived Page not found ");

  client.println("HTTP/1.1 404 Not Found");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<html><body><h1>404 - Page Not Found</h1></body></html>");
}

// extern std::vector<int> tensionLog;
extern std::vector<std::pair<uint32_t, int>> tensionLog;

std::string generateGraphHtml(WiFiClient &client)
{
  String startLogGraph = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Motor Config & Graph</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
        // Load Google Charts
        google.charts.load('current', { packages: ['corechart'] });
        google.charts.setOnLoadCallback(drawChart);

        function drawChart() {
            var data = new google.visualization.DataTable();
            data.addColumn('number', 'X');
            data.addColumn('number', 'Current');
            data.addColumn('number', 'Threshold Current');
            data.addColumn('number', 'Max Threshold');

            data.addRows([
            )";

  String endLogGraph = R"(
              ]);

            // Chart options
            var options = {
                title: 'Motor Current',
                hAxis: { title: 'Time (ms)' },
                vAxis: { title: 'Current (mA)' },
                series: {
                    0: { color: 'blue', pointSize: 5, lineWidth: 2 },
                    1: { color: 'red', pointSize: 0, lineWidth: 2 },
                    2: { color: 'purple', pointSize: 0, lineWidth: 2 },
                },
            };

            var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
            chart.draw(data, options);
        }
    </script>
</head>

<body>
    <div class="container my-4">
        <!-- Graph Section -->
        <div class="row">
            <div class="col-12">
                <h3 class="text-center mb-4">Motor Current Graph</h3>
                <div id="chart_div" style="width: 100%; height: 600px;"></div>
            </div>
        </div>

        <!-- Control buttons -->
        <div class="d-flex justify-content-center gap-3 mt-3">
            <button class="btn btn-primary btn-lg shadow" onclick="goToOpenPage() ">
                <i class="bi bi-arrow-left-right"></i>←→</button>
            <button class="btn btn-primary btn-lg shadow" onclick="goToClosePage() ">
                <i class="bi bi-arrow-right-left"></i>→←</button>
        </div>

        <!-- Configuration Section -->
        <div class="row mt-4 mb-3">
            <div class="col-12">
                <h3 class="text-center mb-4">Motor Configuration</h3>
                <div id="config-container">
                    <!-- Config Fields -->
                </div>
            </div>
        </div>
)";

  endLogGraph += "\n";

  endLogGraph += "<h2 class='mb-3 text-center fw-b'>Device Automations</h2>";
  endLogGraph += "<table class='table table-striped table-bordered'>";

  endLogGraph += "<thead class='table-dark'>";
  endLogGraph += "<tr>";
  endLogGraph += "<th>ID</th>";
  endLogGraph += "<th>Device Name</th>";
  endLogGraph += "<th>Action</th>";
  endLogGraph += "<th>Type</th>";
  endLogGraph += "<th>Time</th>";
  endLogGraph += "<th>Status</th>";
  endLogGraph += "<th>Run Today</th>";
  endLogGraph += "</tr>";
  endLogGraph += "</thead>";

  endLogGraph += "<tbody>";

  for (const auto &devicePair : systemCfg.cfg.devices)
  {
    int deviceId = devicePair.first;
    const Device &device = devicePair.second;

    for (const auto &automationPair : device.automations)
    {
      int automationId = automationPair.first;
      const Automation &automation = automationPair.second;

      endLogGraph += "<tr>\n";
      endLogGraph += "<td>\n";
      endLogGraph += automationId;
      endLogGraph += "</td>\n<td>";
      endLogGraph += device.name.c_str();
      endLogGraph += "</td>\n<td>";
      endLogGraph += automation.action == 0 ? "Open" : "Close";
      endLogGraph += "</td>\n<td>";
      endLogGraph += automation.type == 0 ? "Time" : "Sun";
      endLogGraph += "</td>\n<td>";
      const int timeAction = automation.getActionTime();
      String timeText = String(timeAction / 60) + ":" + (timeAction % 60 < 10 ? "0" : "") + String(timeAction % 60);
      endLogGraph += timeText.c_str();
      endLogGraph += "</td>\n<td>";
      endLogGraph += automation.status == 0 ? "Enable" : "Disable";
      endLogGraph += "</td>\n<td>";
      endLogGraph += automation.allReadyRunToday ? "True" : "False";
      endLogGraph += "</td>\n";
      endLogGraph += "</tr>\n";
    }
  }
  endLogGraph += "</tbody>";
  endLogGraph += "</table>";

  endLogGraph += R"(
    </div>

    <script>
        const configContainer = document.getElementById('config-container');

        const slidersConfig = [)";
  endLogGraph += "{name : 'Treshold maximum (mA)', key : 'motorThresholdMax', initialValue : ";
  endLogGraph += systemCfg.cfg.motorThresholdMax;
  endLogGraph += ", min : 0, max : 3000},";

  endLogGraph += "{name : 'Ending treshold (mA)', key : 'motorThreshold', initialValue : ";
  endLogGraph += systemCfg.cfg.motorThreshold;
  endLogGraph += ", min : 0, max : 3000},";

  endLogGraph += "{name : 'Motor blind time (ms)', key : 'motorBlindTime', initialValue : ";
  endLogGraph += systemCfg.cfg.motorBlindTime;
  endLogGraph += ", min : 0, max : 500},";

  endLogGraph += "{name : 'Maximum run time (ms)', key : 'motorRunTimeLimit', initialValue : ";
  endLogGraph += systemCfg.cfg.motorRunTimeLimit;
  endLogGraph += ", min : 0, max : 30000},";

  endLogGraph += "{name : 'Speed', key : 'motorSpeed', initialValue : ";
  endLogGraph += systemCfg.cfg.motorSpeed;
  endLogGraph += ", min : 1, max : 255},";
  endLogGraph += R"(];

        slidersConfig.forEach(({ name, key, initialValue, min, max }) => {
            const row = document.createElement('div');
            row.className = 'mb-4';

            row.innerHTML = `
            <div class="d-flex align-items-center justify-content-center">
                <div class="flex-grow-0.5 me-3">
                    <label for="${key}" class="form-label">${name}:</label>
                    <div class="d-flex align-items-center">
                        <input type="range" class="form-range me-3" id="${key}_slider" min="${min}" max="${max}" value="${initialValue}" step="1">
                        <input type="number" class="form-control form-control-sm mw w-50 mw-10" id="${key}" value="${initialValue}" min="${min}" max="${max}">
                        <button class="btn ms-2 btn-primary btn-sm align-self-start" id="${key}_button">Update</button>
                    </div>
                </div>
            </div>
        `;

            // Sync slider and input field
            const slider = row.querySelector(`#${key}_slider`);
            const input = row.querySelector(`#${key}`);
            const button = row.querySelector(`#${key}_button`);

            slider.addEventListener('input', (e) => {
                input.value = e.target.value;
            });
            input.addEventListener('input', (e) => {
                slider.value = e.target.value;
            });

            // Add fetch call to the button
            button.addEventListener('click', async () => {
                const value = input.value;

                try {
                    const response = await fetch('/updateField', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                        },
                        body: JSON.stringify({
                            key,
                            value: parseInt(value, 10),
                        }),
                    });

                    if (!response.ok) {
                        throw new Error(`Server error: ${response.statusText}`);
                    }

                    const result = await response.json();
                    // alert(`Updated ${name} successfully: ${JSON.stringify(result)}`);
                } catch (error) {
                    console.error('Error updating motor config:', error);
                    // alert(`Failed to update ${name}: ${error.message}`);
                }
            });

            configContainer.appendChild(row);
        });




async function sendCommand(command) {
    try {
        const response = await fetch(`/${command}`, {
            method: 'POST', // or 'GET' depending on your server
        });

        if (!response.ok) {
            throw new Error(`Server error: ${response.statusText}`);
        }

        console.log(`${command} command sent successfully`);
    } catch (error) {
        console.error(`Error sending ${command} command:`, error);
    }
}

function goToOpenPage() {
    sendCommand('open1');
}

function goToClosePage() {
    sendCommand('close1');
}

    </script>
)";

  String automationLogGraph = R"(
</body>
</html>
)";

  std::string result = startLogGraph.c_str();
  for (int i = 0; i < tensionLog.size(); i++)
  {
    result += ("[" + String(tensionLog[i].first) + ", " + String(tensionLog[i].second) + ", " +
               String(systemCfg.cfg.motorThresholdMax) + ", " + String(1600) + "], ")
                  .c_str();
  }
  result += endLogGraph.c_str();
  result += automationLogGraph.c_str();
  return result;
}

void handleGet_index_html(WiFiClient &client)
{
  Serial.println(" === START handleGet_index_html ====");

  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/html");
  //   client.println("Content-Encoding: gzip");
  //   client.print("Content-Length: ");
  //   client.println(index_html_gz_len +2);
  //   // +2 for CR and LF
  //   client.println("Connection: close");
  //   client.println();
  //  for (int i = 0; i < index_html_gz_len; i++)
  //    client.write(index_html_gz[i]);

  std::string responce = R"(
<!doctype html>
<html lang="en">

<head>
  <meta charset="UTF-8" />
  <link rel="icon" type="image/svg+xml" href="http://fygadipc.local:3000/favicon.ico" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Frideau</title>
  <script type="module" crossorigin src="http://fygadipc.local:3000/AppJs"></script>
</head>

<body>
  <style>
    body {
      margin-bottom: 56px !important
    }
  </style>

  <div id="root"></div>
</body>

</html>)";
  client.println(responce.length() + 2);
  client.println("Connection: close");
  client.println();
  client.println(responce.c_str());

  Serial.println(" === END handleGet_index_html ====");
}

void handleGet_index_GBR06sQK_js(WiFiClient &client)
{
// file isnt request anymore
#if 0
  Serial.println(" === START handleGet_index_GBR06sQK_js ====");
  Serial.println("HTTP/1.1 200 OK");
  Serial.println("Access-Control-Allow-Origin: *");
  Serial.println("Content-Type: text/javascript");
  Serial.println("Cache-Control: max-age=31536000,immutable");
  Serial.println("Content-Encoding: gzip");
  Serial.print("Content-Length: ");
  Serial.println(index_GBR06sQK_js_gz_len + 2);
  Serial.println("Connection: close");
  Serial.println();




  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Type: text/javascript");
  client.println("Cache-Control: max-age=31536000,immutable");
  client.println("Content-Encoding: gzip");
  client.print("Content-Length: ");
  client.println(index_GBR06sQK_js_gz_len + 2);
  // +2 for CR and LF
  client.println("Connection: close");
  client.println();
  for (int i = 0; i < index_GBR06sQK_js_gz_len; i++)
   client.write(index_GBR06sQK_js_gz[i]);
 
  Serial.println(" === END handleGet_index_GBR06sQK_js ====");

#endif
}