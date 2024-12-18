#include <stdio.h>
#include <string.h>
#include <vector>

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <TimeLib.h>
#include <WiFi.h>

#include "Configuration.h"
#include "utility.h"

//#include "index-DNSdRnKq.js.h"
//#include "index_html.h"

#define DEBUG 0  // Set to 0 to disable debug prints

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
void handleGet_index_DNSdRnKq_js(WiFiClient &client);
void handleGet_slash(WiFiClient &client);
void processPut_createDevice(WiFiClient &client, String data);
std::tuple<bool, int, int> processPost_actionDevice(WiFiClient &client, String data);
void processPut_createAutomation(WiFiClient &client, String data);
void processPatch_modifyDevice(WiFiClient &client, String data);
void processPatch_modifyAutomation(WiFiClient &client, String data);
void processPatch_modifyConfig(WiFiClient &client, String data);
void processDelete_deleteDevice(WiFiClient &client, String data);
void processDelete_deleteAutomation(WiFiClient &client, String data);
void sendNotFound(WiFiClient &client);
void generateGraphHtml(WiFiClient &client);

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
    else if (requestHeader.indexOf("GET /index_DNSdRnKq_js") >= 0)
    {
      handleGet_index_DNSdRnKq_js(client);
    }
    else if (requestHeader.indexOf("GET /graph") >= 0)
    {
      generateGraphHtml(client);
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
      if (requestHeader.indexOf("GET /?threshold=") >= 0)
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
        systemCfg.cfg.thresholdMaxCurrent = threshold;
        systemCfg.saveToEEPROM();
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
    else if (requestHeader.indexOf("PATCH /configuration HTTP/1.1") >= 0)
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
  client.println(systemCfg.cfg.thresholdMaxCurrent);

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

void generateGraphHtml(WiFiClient &client)

{
  String startLogGraph = R"(
<html>
    <head>
        <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
            

      <script type="text/javascript">
google.charts.load('current', {packages: ['corechart', 'line']});
google.charts.setOnLoadCallback(drawCurveTypes);


function drawCurveTypes() {
      var data = new google.visualization.DataTable();
      data.addColumn('number', 'X');
      data.addColumn('number', 'Threshold');
      data.addColumn('number', 'Courrant');

      data.addRows([
)";

  String endLogGraph = R"(
      ]);

      var options = {
        hAxis: {
          title: 'Time'
        },
        vAxis: {
          title: 'uAmp'
        },
        series: {
          1: {curveType: 'function'}
        }
      };

      var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
      chart.draw(data, options);
    }
    
    </script>
    </head>
    <body>
        <div id="chart_div" style="width: 90%; height: 90%"></div>
    </body>
  </html>
)";

#if 0
// Dummy data for testing only
  tensionLog.clear();
  for (int i = 0; i < 100; i++)
    tensionLog.push_back(rand() % 100);
#endif

  client.print(startLogGraph.c_str());
  for (int i = 0; i < tensionLog.size(); i++)
  {
    client.print("[" + String(tensionLog[i].first) + ", " + String(systemCfg.cfg.thresholdMaxCurrent) + ", " +
                 String(tensionLog[i].second) + "], ");
  }
  client.print(endLogGraph.c_str());
}

#if 1
void handleGet_index_html(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Content-Encoding: gzip");
  client.print("Content-Length: ");
//  client.println(index_html_gz_len );
  client.println("Connection: close");
  client.println();
//  for (int i = 0; i < index_html_gz_len; i++)
//    client.write(index_html_gz[i]);
}
#endif

void handleGet_index_DNSdRnKq_js(WiFiClient &client)
{
  (void)client;
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Content-Encoding: gzip");
  client.print("Content-Length: ");
//  client.println(index_DNSdRnKq_js_gz_len );
  client.println("Connection: close");
  client.println();
//  for (int i = 0; i < index_DNSdRnKq_js_gz_len; i++)
//    client.write(index_DNSdRnKq_js_gz[i]);
}