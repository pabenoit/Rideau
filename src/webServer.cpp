#include <stdio.h>
#include <string.h>
#include <vector>

#include <Arduino_JSON.h>
#include <ESPAsyncWebServer.h>
#include <TimeLib.h>
#include <WiFi.h>


#include "Configuration.h"
#include "utility.h"


#define DEBUG 1  // Set to 0 to disable debug prints

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT_FTC Serial.println(__FUNCTION__);
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_FTC
#endif

// Declare the server variable as external if it is defined in another file
extern AsyncWebServer server;
extern Configuration systemCfg;
extern std::vector<std::pair<uint32_t, int>> tensionLog;
extern AsyncWebSocket ws;


void dummyRequestHandelFunction(AsyncWebServerRequest* request) 
{
  DEBUG_PRINT_FTC
}

void dummyUploadHandelFunction(AsyncWebServerRequest* request,
                               const String& filename,
                               size_t index,
                               uint8_t* data,
                               size_t len,
                               bool final)
{
  DEBUG_PRINT_FTC
}

void dummyBodyHandelFunction(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {}

// ==== START Config Section ===
void handleGetConfig(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC
  request->send(200, "application/json", systemCfg.GetAllInfo().c_str());
}

void handlePutConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC
   ws.textAll( systemCfg.GetAllInfo().c_str());
}

void handlePatchConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC

  String requestBody = String((char*)data).substring(0, len);
  if (systemCfg.modifyWifiLocation(requestBody.c_str()))
    request->send(200);
  else
    request->send(400);

  ws.textAll( systemCfg.GetAllInfo().c_str());
}

void handleDeletConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC

  ws.textAll( systemCfg.GetAllInfo().c_str());
}

// ==== START Automation Section ===
void handlePutAutomation(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC
  String requestBody = String((char*)data).substring(0, len);

  int automationId = 0;

  systemCfg.createAutomation(requestBody.c_str(), automationId);
  JSONVar root;
  root["automationId"] = automationId;
  std::string responce = JSON.stringify(root).c_str();

  // Send response
  request->send(200, "application/json", responce.c_str());

  ws.textAll( systemCfg.GetAllInfo().c_str());
}

void handlePatchAutomation(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC
  // where data is give by the request via the body
  String requestBody = String((char*)data).substring(0, len);

  if (systemCfg.modifyAutomation(requestBody.c_str()))
    request->send(200);
  else
    request->send(400);

   ws.textAll( systemCfg.GetAllInfo().c_str());
}

void handleDeleteAutomation(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC
  // where data is give by the request via the body
  String requestBody = String((char*)data).substring(0, len);
  if (systemCfg.deleteAutomation(requestBody.c_str()))
    request->send(200);
  else
    request->send(400);

  ws.textAll( systemCfg.GetAllInfo().c_str());
}
// ==== END Automation Section ===

void handlePostOperation(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC

  if (request->hasParam("action") && request->hasParam("id"))
  {
    int id = request->getParam("id")->value().toInt();
    int action = request->getParam("action")->value().toInt();
  
    // TODO: Call MOTOR Action

    request->send(200);
  }
  else
    request->send(400, "text/plain", "Wrong | Missing id parameter");
}

void handlePatchDevice(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
{
  DEBUG_PRINT_FTC

  String requestBody = String((char*)data).substring(0, len);
  if (systemCfg.modifyDevice(requestBody.c_str()))
    request->send(200);
  else
    request->send(400);

  ws.textAll( systemCfg.GetAllInfo().c_str());
}

void handleGet_index_html(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC
  String page =  R"rawliteral(<!doctype html><html lang="en"><head><meta charset="UTF-8" /><link rel="icon" type="image/svg+xml" href="http://fygadi-pc.local:3000/favicon.ico" /><meta name="viewport" content="width=device-width, initial-scale=1.0" /><title>Frideau</title><script type="module" src="http://fygadi-pc.local:3000/AppJs"></script></head><body><style>    body { margin-bottom: 56px !important }  </style><div id="root"></div></body></html>)rawliteral";

  request->send(200, "text/html", page);
}

void handleGetGraph(AsyncWebServerRequest* request)
{
    DEBUG_PRINT_FTC

    const char logGraph[] PROGMEM = R"(
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
        google.charts.setOnLoadCallback(fetchDataAndDrawChart);

        function fetchDataAndDrawChart() {
            fetch('/api/chart-data').then(response => response.json()).then(data => { drawChart(data); }).catch(error => {
                console.error('Error fetching chart data:', error);
            });
        }

        function drawChart(chartData) {
            const data = new google.visualization.DataTable();
            data.addColumn('number', 'X');
            data.addColumn('number', 'Current');
            data.addColumn('number', 'Threshold Current');
            data.addColumn('number', 'Max Threshold');
            chartData.forEach(item => {
                data.addRow([item.x, item.y, item.z, item.a]);
            });

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
        <div class="row">
            <div class="col-12">
                <h3 class="text-center mb-4">Motor Current Graph</h3>
                <div id="chart_div" style="width: 100%; height: 600px;"></div>
            </div>
        </div>
        <div class="d-flex justify-content-center gap-3 mt-3">
            <button class="btn btn-primary btn-lg shadow" onclick="goToOpenPage() ">
                <i class="bi bi-arrow-left-right"></i> ←→ </button>
            <button class="btn btn-primary btn-lg shadow" onclick="goToClosePage() ">
                <i class="bi bi-arrow-right-left"></i> →← </button>
        </div>
        <div class="row mt-4 mb-3">
            <div class="col-12">
                <h3 class="text-center mb-4">Motor Configuration</h3>
                <div id="config-container">
                </div>
            </div>
        </div>
        <h2 class='mb-3 text-center fw-b'>Device Automations</h2>
        <div id="table-automation-container"></div>
    </div>
    <script>
        async function createTableFromJSON() {
            try {
                const response = await fetch('/api/automationList')
                const data = await response.json();
                const tableContainer = document.getElementById('table-automation-container');
                const table = document.createElement('table');
                table.className = 'table table-striped table-bordered';
                const thead = document.createElement('thead');
                thead.className = 'table-dark';
                const tbody = document.createElement('tbody');
                const headers = ["id", "name", "action", "type", "time", "status", "run"];
                const headerRow = document.createElement('tr');
                headers.forEach(header => {
                    const th = document.createElement('th');
                    th.textContent = header;
                    headerRow.appendChild(th);
                });
                thead.appendChild(headerRow);
                data.forEach(item => {
                    const row = document.createElement('tr');
                    headers.forEach(header => {
                        const td = document.createElement('td');
                        td.textContent = item[header];
                        row.appendChild(td);
                    });
                    tbody.appendChild(row);
                });
                table.appendChild(thead);
                table.appendChild(tbody);
                tableContainer.appendChild(table);
            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }
        createTableFromJSON();
        const configContainer = document.getElementById('config-container');
        let slidersConfig;

        fetch('/api/motorCfg').then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        }).then(data => {
            slidersConfig = data;
            console.log(slidersConfig);
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
                const slider = row.querySelector(`#${key}_slider`);
                const input = row.querySelector(`#${key}`);
                const button = row.querySelector(`#${key}_button`);
                slider.addEventListener('input', (e) => { input.value = e.target.value; });
                input.addEventListener('input', (e) => { slider.value = e.target.value; });
                button.addEventListener('click', async () => {
                    const value = input.value;
                    try {
                        const response = await fetch('/updateField', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json', },
                            body: JSON.stringify({ key, value: parseInt(value, 10), }),
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
        }).catch(error => {
            console.error('There was a problem with the fetch operation:', error);
        });
        async function sendCommand(command) {
            try {
                const response = await fetch(`/${command}`, { method: 'POST', });
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
</body>

</html>
)";

  request->send(  200, "text/html", logGraph);
}

void handleGetGraphData(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC

 JSONVar dataArray;
 
#if 0
  for (int i = 0; i < tensionLog.size(); i++)
  {
        JSONVar point;
        point["x"] = tensionLog[i].first;
        point["y"] = tensionLog[i].second;
        point["z"] =systemCfg.cfg.motorThresholdMax
        point["a"] = 1600;
        dataArray[i] = point;
  }
#else
  for (int i = 0; i < 100; i++)
  {
        JSONVar point;
        point["x"] = i+2;
        point["y"] = i*12;
        point["z"] = 1200;
        point["a"] = 1600;
        dataArray[i] = point;
  }
#endif
  request->send(200, "application/json",  JSON.stringify(dataArray).c_str());
}

void handleGetMotorCfg(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC

  JSONVar motorCfg;

  JSONVar motorThresholdMax;
  motorThresholdMax["name"] = "Treshold maximum (mA)";
  motorThresholdMax["key"] = "motorThresholdMax";
  motorThresholdMax["initialValue"] = 1600;
  motorThresholdMax["min"] = 0;
  motorThresholdMax["max"] = 3000;
  motorCfg[0] = motorThresholdMax;

  JSONVar motorThreshold;
  motorThreshold["name"] = "Ending treshold (mA)";
  motorThreshold["key"] = "motorThreshold";
  motorThreshold["initialValue"] = 850;
  motorThreshold["min"] = 0;
  motorThreshold["max"] = 3000;
  motorCfg[1] = motorThreshold;

  JSONVar motorBlindTime;
  motorBlindTime["name"] = "Motor blind time (ms)";
  motorBlindTime["key"] = "motorBlindTime";
  motorBlindTime["initialValue"] = 150;
  motorBlindTime["min"] = 0;
  motorBlindTime["max"] = 500;
  motorCfg[2] = motorBlindTime;

  JSONVar motorRunTimeLimit;
  motorRunTimeLimit["name"] = "Maximum run time (ms)";
  motorRunTimeLimit["key"] = "motorRunTimeLimit";
  motorRunTimeLimit["initialValue"] = 9000;
  motorRunTimeLimit["min"] = 0;
  motorRunTimeLimit["max"] = 30000;
  motorCfg[3] = motorRunTimeLimit;

  JSONVar motorSpeed;
  motorSpeed["name"] = "Speed";
  motorSpeed["key"] = "motorSpeed";
  motorSpeed["initialValue"] = 240;
  motorSpeed["min"] = 1;
  motorSpeed["max"] = 255;
  motorCfg[4] = motorSpeed;

  request->send(200, "application/json",  JSON.stringify(motorCfg).c_str());
}

void handleGetAutomations(AsyncWebServerRequest* request)
{
  DEBUG_PRINT_FTC

  JSONVar automationLists;
  int idx =0;

  for (const auto &devicePair : systemCfg.cfg.devices)
  {
    int deviceId = devicePair.first;
    const Device &device = devicePair.second;

    for (const auto &automationPair : device.automations)
    {
      int automationId = automationPair.first;
      const Automation &automation = automationPair.second;

      JSONVar element;

      element["id"] = automationId;
      element["name"] = device.name.c_str();
      element["action"] = automation.action == 0 ? "Open" : "Close";
      element["type"] = automation.type == 0 ? "Time" : "Sun";

      const int timeAction = automation.getActionTime();
      String timeText = String(timeAction / 60) + ":" + (timeAction % 60 < 10 ? "0" : "") + String(timeAction % 60);
      element["time"] = timeText.c_str();
      element["status"] = automation.status == 0 ? "Enable" : "Disable";
      element["run"] = automation.allReadyRunToday ? "True" : "False";
      automationLists[idx++] = element;
    }
  }

  request->send(200, "application/json",  JSON.stringify(automationLists).c_str());
}

extern void setupWebServer()
{
  server.onNotFound(handleGet_index_html);

  server.on("/api/operation", HTTP_POST, handlePostOperation);

  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_PUT, dummyRequestHandelFunction, dummyUploadHandelFunction, handlePutConfig);
  server.on("/api/config", HTTP_PATCH, dummyRequestHandelFunction, dummyUploadHandelFunction, handlePatchConfig);
  server.on("/api/config", HTTP_DELETE, dummyRequestHandelFunction, dummyUploadHandelFunction, handleDeletConfig);

  server.on("/api/automation", HTTP_PUT, dummyRequestHandelFunction, dummyUploadHandelFunction, handlePutAutomation);
  server.on("/api/automation", HTTP_PATCH, dummyRequestHandelFunction, dummyUploadHandelFunction, handlePatchAutomation);
  server.on("/api/automation", HTTP_DELETE, dummyRequestHandelFunction, dummyUploadHandelFunction, handleDeleteAutomation);

  server.on("/api/device", HTTP_PATCH, dummyRequestHandelFunction, dummyUploadHandelFunction, handlePatchDevice);

  server.on("/graph", HTTP_GET, handleGetGraph);
  server.on("/api/chart-data", HTTP_GET, handleGetGraphData);
  server.on("/api/motorCfg", HTTP_GET, handleGetMotorCfg);
  server.on("/api/automationList", HTTP_GET, handleGetAutomations);
}
