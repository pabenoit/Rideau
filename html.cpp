#include <WiFiNINA.h>
#include "html.h"
#include "Motor.h"
#include "configData.h"
#include <string>
#include <vector>
#include <functional>

extern WiFiClient client;

extern void closeRideauManuel(int idx);
extern void openRideauManuel(int idx);
extern void closeRideauAuto(int idx);
extern void openRideauAuto(int idx);

void generateHtmlPage(class Config &config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)())
{

  std::string preformattedPage = R"(
<!doctype html>
<html lang="en">
    <head>
        <title>Rideau Automatique</title>
        <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg width='24' height='24' fill='none' viewBox='0 0 24 24' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M11.996
  18.532a1 1 0 0 1 .993.883l.007.117v1.456a1 1 0 0 1-1.993.116l-.007-.116v-1.456a1 1 0 0 1 1-1Zm6.037-1.932 1.03 1.03a1
  1 0 0 1-1.415 1.413l-1.03-1.029a1 1 0 0 1 1.415-1.414Zm-10.66 0a1 1 0 0 1 0 1.414l-1.029 1.03a1 1 0 0
  1-1.414-1.415l1.03-1.03a1 1 0 0 1 1.413 0ZM12.01 6.472a5.525 5.525 0 1 1 0 11.05 5.525 5.525 0 0 1 0-11.05Zm8.968
  4.546a1 1 0 0 1 .117 1.993l-.117.007h-1.456a1 1 0 0 1-.116-1.993l.116-.007h1.456ZM4.479 10.99a1 1 0 0 1 .116
  1.993l-.116.007H3.023a1 1 0 0 1-.117-1.993l.117-.007h1.456Zm1.77-6.116.095.083 1.03 1.03a1 1 0 0 1-1.32 1.497L5.958
  7.4 4.93 6.371a1 1 0 0 1 1.32-1.497Zm12.813.083a1 1 0 0 1 .083 1.32l-.083.094-1.03 1.03a1 1 0 0 1-1.497-1.32l.084-.095
  1.029-1.03a1 1 0 0 1 1.414 0ZM12 2.013a1 1 0 0 1 .993.883l.007.117v1.455a1 1 0 0 1-1.993.117L11 4.468V3.013a1 1 0 0 1
  1-1Z' fill='%23ffaa00'/%3E%3C/svg%3E"/>
        <!-- Required meta tags -->
        <meta charset="utf-8"/>
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no"/>
        <!-- Bootstrap CSS v5.2.1 -->
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous"/>
        <style>
            .accordion {
                --bs-accordion-active-bg: #5c636a;
                --bs-accordion-active-color: white;
                --bs-accordion-btn-bg: #6c757d;
                --bs-accordion-color: black;
                --bs-accordion-bg: #4c5152;
            }

            .accordion-button::after {
                background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-plus' viewBox='0 0 16 16'%3E%3Cpath d='M8 4a.5.5 0 0 1 .5.5v3h3a.5.5 0 0 1 0 1h-3v3a.5.5 0 0 1-1 0v-3h-3a.5.5 0 0 1 0-1h3v-3A.5.5 0 0 1 8 4z'/%3E%3C/svg%3E");
            }

            .accordion {
                --bs-accordion-btn-focus-box-shadow: none;
            }

            .accordion-button:not(.collapsed)::after {
                background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-dash' viewBox='0 0 16 16'%3E%3Cpath d='M4 8a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 0 1h-7A.5.5 0 0 1 4 8z'/%3E%3C/svg%3E");
            }

            .accordion-button::after {
                transition: all 0.3s;
            }

            .card {
                background-color: #4c5152;
            }
        </style>
        <script>
            //InitialiseSite
            function initialisePage() {
                initialiseVariable();
                toggleItemRideaux();
                toggleItemVoilages();
            }

            //Variable iniiliasation
            function initialiseVariable() {
                let settings = {
)";

  client.println(preformattedPage.data());

  client.print("motorSpeed: '");
  client.print(config.manualSpeed);
  client.print("',\n");

  client.print("serverTime: '");
  client.print(getTimeStr());
  client.print("',\n");

  client.print("sunriseTime: '");
  int timeSoleil = getSunrise();
  preformattedPage = (((timeSoleil / 60 < 10) ? "0" : "") + std::to_string(timeSoleil / 60) + ":" + ((timeSoleil % 60 < 10) ? "0" : "") + std::to_string(timeSoleil % 60));
  client.print(preformattedPage.data());
  client.print("',\n");

  client.print("sunsetTime: '");
  timeSoleil = getSunset();
  preformattedPage = (((timeSoleil / 60 < 10) ? "0" : "") + std::to_string(timeSoleil / 60) + ":" + ((timeSoleil % 60 < 10) ? "0" : "") + std::to_string(timeSoleil % 60));
  client.print(preformattedPage.data());
  client.print("',\n");

  client.print("\n");

  for (int idx = 0; idx < 2; idx++)
  {
    if (idx == 0)
    {
      client.print("rideaux: {\n");

      if (config.rideau[idx].isEnabled)
        client.print("rideauxSwitchOn: true,\n");
      else
        client.print("rideauxSwitchOn: false,\n");
    }
    else
    {
      client.print("voilages: {\n");
      if (config.rideau[idx].isEnabled)
        client.print("voilagesSwitchOn: true,\n");
      else
        client.print("voilagesSwitchOn: false,\n");
    }

    client.print("opening: {\n");

    if (config.rideau[idx].isOpenAtSunrise == true)
      client.print("isSunrise: true,\n");
    else
      client.print("isSunrise: false,\n");

    client.print("// sunriseTime,\n");
    client.print("\n");

    if (config.rideau[idx].isOpenAtTime == true)
      client.print("isAtTime: true,\n");
    else
      client.print("isAtTime: false,\n");

    int actionAtTime = config.rideau[idx].openAtTime;
    client.print("atTime: '");
    preformattedPage = (((actionAtTime / 60 < 10) ? "0" : "") + std::to_string(actionAtTime / 60) + ":" + ((actionAtTime % 60 < 10) ? "0" : "") + std::to_string(actionAtTime % 60));
    client.print(preformattedPage.data());
    client.print("'\n");

    client.print("},\n");
    client.print("closing: {\n");
    if (config.rideau[idx].isCloseAtSunset == true)
      client.print("isSunset: true,\n");
    else
      client.print("isSunset: false,\n");

    client.print("// sunsetTime,\n");
    client.print("\n");
    if (config.rideau[idx].isCloseAtTime == true)
      client.print("isAtTime: true,\n");
    else
      client.print("isAtTime: false,\n");

    actionAtTime = config.rideau[idx].closeAtTime;
    client.print("atTime: '");
    preformattedPage = (((actionAtTime / 60 < 10) ? "0" : "") + std::to_string(actionAtTime / 60) + ":" + ((actionAtTime % 60 < 10) ? "0" : "") + std::to_string(actionAtTime % 60));
    client.print(preformattedPage.data());
    client.print("'\n");

    client.print("}\n");

    if (idx == 0)
      client.print("},\n");
    else
      client.print("}\n");
  }

  client.print("}\n");

  preformattedPage = R"(
                {
                    // Rideaux
                    document.getElementById('rideaux-switch').checked = settings.rideaux.rideauxSwitchOn;
                    {
                        //Ouverture
                        document.getElementById('rideaux-sunrise-radio').checked = settings.rideaux.opening.isSunrise;
                        document.getElementById('rideaux-sunrise-time').value = settings.sunriseTime;
                        document.getElementById('rideaux-open-at-time-radio').checked = settings.rideaux.opening.isAtTime;
                        document.getElementById('rideaux-open-at-time').value = settings.rideaux.opening.atTime;
                    }
                    {
                        //Fermeture
                        document.getElementById('rideaux-sunset-radio').checked = settings.rideaux.closing.isSunset;
                        document.getElementById('rideaux-sunset-time').value = settings.sunsetTime;
                        document.getElementById('rideaux-close-at-time-radio').checked = settings.rideaux.closing.isAtTime;
                        document.getElementById('rideaux-close-at-time').value = settings.rideaux.closing.atTime;
                    }
                }
                {
                    //Voilage
                    document.getElementById('voilages-switch').checked = settings.voilages.voilagesSwitchOn;
                    {
                        //Ouverture
                        document.getElementById('voilages-sunrise-radio').checked = settings.voilages.opening.isSunrise;
                        document.getElementById('voilages-sunrise-time').value = settings.sunriseTime;
                        document.getElementById('voilages-open-at-time-radio').checked = settings.voilages.opening.isAtTime;
                        document.getElementById('voilages-open-at-time').value = settings.voilages.opening.atTime;
                    }
                    {
                        //Fermeture
                        document.getElementById('voilages-sunset-radio').checked = settings.voilages.closing.isSunset;
                        document.getElementById('voilages-sunset-time').value = settings.sunsetTime;
                        document.getElementById('voilages-close-at-time-radio').checked = settings.voilages.closing.isAtTime;
                        document.getElementById('voilages-close-at-time').value = settings.voilages.closing.atTime;
                    }
                }

                {
                    //Speed Slider
                    document.getElementById('mode-manuel-motor-speed-slider').value = settings.motorSpeed;

                    let motorSpeedLabel = document.getElementById('motor-speed-label').textContent = settings.motorSpeed;
                }

                document.getElementById('server-time-label').textContent = settings.serverTime;
            }

            //Disable sections
            function toggleItemRideaux() {
                enableDisableAllChildren('card-group-rideaux', 'rideaux-switch');
            }
            function toggleItemVoilages() {
                enableDisableAllChildren('card-group-voilages', 'voilages-switch');
            }
)";
  client.println(preformattedPage.data());

  preformattedPage = R"(
            function enableDisableAllChildren(parentID, boolEnableSwitchID) {
                let parent = document.getElementById(parentID);
                let boolEnable = !(document.getElementById(boolEnableSwitchID).checked);

                let childElements = parent.getElementsByTagName('*');
                for (let i = 0; i < childElements.length; i++) {
                    let element = childElements[i];
                    if ((element.id !== "rideaux-sunrise-time") && (element.id !== "rideaux-sunset-time") && (element.id !== "voilages-sunrise-time") && (element.id !== "voilages-sunset-time"))
                    {
                      if (element.tagName !== 'DIV') {
                          // Avoid disabling child divs recursively
                          element.readonly = boolEnable;
                          if (element.tagName === 'INPUT' || element.tagName === 'TEXTAREA') {
                              element.style.pointerEvents = boolEnable ? 'none' : 'auto';
                              element.style.opacity = boolEnable ? '0.5' : '1';
                          }
                      }
                    }
                }
                // Disable or enable the parent
                parent.readonly = boolEnable;
                if (boolEnable) {
                    parent.style.pointerEvents = 'none';
                    parent.style.opacity = '0.5';
                } else {
                    parent.style.pointerEvents = 'auto';
                    parent.style.opacity = '1';
                }
            }

            //Slider update text value
            function updateSliderText() {
                let slider = document.getElementById("mode-manuel-motor-speed-slider");
                let span = document.getElementById("motor-speed-label");
                span.textContent = slider.value;
                span.style.fontWeight = "bold";
            }

            //Form validation
            function validateFormRideaux() {
                return true;
            }

            function validateFormVoilages() {
                return true;
            }

            function validateFormModeManuel() {
                return true;
            }
        </script>
    </head>
)";
  client.println(preformattedPage.data());

  preformattedPage = R"(<body class='bg-dark' onload='initialisePage()'> <header class='mb-4'> <!-- navbar here --> <div class='container'> <div class='collapse' id='navbarToggleExternalContent' data-bs-theme='dark'> <div class='bg-dark p-4'> <h5 class='text-body-emphasis h4'>Additional information</h5> <span class='text-body-secondary'>Credit: Pascal et Victor Benoit <BR> Copyright 2024</span> </div> </div> <nav class='navbar navbar-dark bg-dark'> <div class='container-fluid'> <button class='navbar-toggler' type='button' data-bs-toggle='collapse' data-bs-target='#navbarToggleExternalContent' aria-controls='navbarToggleExternalContent' aria-expanded='false' aria-label='Toggle navigation'>)";
  client.println(preformattedPage.data());
  preformattedPage = R"( <span class='navbar-toggler-icon'></span> </button> </div> </nav> </div> </header> <main class='container text-light'> <!-- accordion --> <div class='accordion mb-5' id='accordion-settings'> <!-- item 1 --> <form method='get' action='/Auto-Rideaux' name='rideaux-form' id='rideaux-form' onsubmit='return validateFormRideaux()'> <div class='accordion-item'> <h2 class='accordion-header'> <button class='accordion-button' type='button' data-bs-toggle='collapse' data-bs-target='#collapse-rideaux' aria-expanded='true' aria-controls='collapse-rideaux'>Rideaux </button> </h2> <div id='collapse-rideaux' class='accordion-collapse collapse show' data-bs-parent='#accordion-settings'> <!--data-bs-parent='#accordion-settings'--> <div class='accordion-body'> <div class='form-check form-switch mx-3'> <input class='form-check-input' type='checkbox' role='switch' onchange='toggleItemRideaux()' id='rideaux-switch' name='rideaux-switch' checked> )";
  client.println(preformattedPage.data());
  preformattedPage = R"(<label class='form-check-label' for='rideaux-switch'>On / Off </label> </div> <div class='card-group' id='card-group-rideaux'> <!-- cardgroup --> <div class='card'> <!-- Rideaux Ouverture --> <div class='card-body'> <h4 class='card-title'>Ouverture</h4> <div class='form-check'> <input class='form-check-input' type='radio' name='rideaux-opening-radio' id='rideaux-sunrise-radio' required value='sun'> <label class='form-check-label' for='rideaux-sunrise-radio'> Au lever du soleil à <input id='rideaux-sunrise-time' type='time' value='06:15' readonly> </label> </div> <div class='form-check'> <input class='form-check-input' type='radio' name='rideaux-opening-radio' id='rideaux-open-at-time-radio' value='time'> <label class='form-check-label' for='rideaux-open-at-time-radio'> À <input class='form-check-label' type='time' id='rideaux-open-at-time' name='rideaux-open-at-time' value='07:00'> </label> </div> </div> </div> <div class='card'> <!-- Rideaux Fermeture -->)";
  client.println(preformattedPage.data());
  preformattedPage = R"(<div class='card-body'> <h4 class='card-title'>Fermeture</h4> <div class='form-check'> <input class='form-check-input' type='radio' name='rideaux-closing-radio' id='rideaux-sunset-radio' value='sun'> <label class='form-check-label' for='rideaux-sunset-radio'> Au couché du soleil à <input id='rideaux-sunset-time' type='time' value='18:45' readonly> </label> </div> <div class='form-check'> <input class='form-check-input' type='radio' name='rideaux-closing-radio' id='rideaux-close-at-time-radio' value='time'> <label class='form-check-label' for='rideaux-closing-lever-time'> À <input class='form-check-label' type='time' id='rideaux-close-at-time' name='rideaux-close-at-time' value='20:00'> </label> </div> </div> </div> </div> </div> <!-- Buttons --> <div class='hstack col-md-8 mx-auto btn-group'> <button type='submit' class='btn btn-secondary'>Sauvegarder</button> <button type='reset' class='btn btn-outline-secondary'>Réinitialiser</button> </div> </div> </div> )";
  client.println(preformattedPage.data());
  preformattedPage = R"(</form> <form method='get' action='/Auto-Voilage' name='voilages-form' id='voilages-form' onsubmit='return validateFormVoilages()'> <div class='accordion-item'> <!-- Item 2 --> <h2 class='accordion-header'> <button class='accordion-button collapsed button' type='button' data-bs-toggle='collapse' data-bs-target='#collapse-voilages' aria-expanded='false' aria-controls='collapse-voilages'>Voilages </button> </h2> <div id='collapse-voilages' class='accordion-collapse collapse' data-bs-parent='#accordion-settings'> <!--data-bs-parent='#accordion-settings'--> <div class='accordion-body'> <div class='form-check form-switch mx-3'> <input onclick='toggleItemVoilages()' class='form-check-input' type='checkbox' role='switch' name='voilages-switch' id='voilages-switch' checked> <label class='form-check-label' for='voilages-switch'>On / Off</label> </div> <div class='card-group' id='card-group-voilages'> <!-- Cardgroup -->)";
  client.println(preformattedPage.data());
  preformattedPage = R"(<div class='card'> <!-- Voilages Ouverture --> <div class='card-body bdcalou'> <h4 class='card-title'>Ouverture</h4> <span> <div class='form-check'> <input class='form-check-input' type='radio' name='voilages-opening-radio' id='voilages-sunrise-radio' value='sun'> <label class='form-check-label' for='voilages-sunrise-radio'> Au lever du soleil à <input id='voilages-sunrise-time' type='time' value='00:00' readonly> </label> </div> <div class='form-check'> <input class='form-check-input' type='radio' name='voilages-opening-radio' id='voilages-open-at-time-radio' value='time'> <label class='form-check-label' for='voilages-open-at-time-radio'> À <input class='form-check-label' type='time' id='voilages-open-at-time' name='voilages-open-at-time' value='00:00'> </label> </div> </span> </div> </div> <div class='card'> <!-- voilage Fermeture --> <div class='card-body'> <h4 class='card-title'>Fermeture</h4> <span> <div class='form-check'> )";
  client.println(preformattedPage.data());
  preformattedPage = R"(<input class='form-check-input' type='radio' name='voilages-closing-radio' id='voilages-sunset-radio' value='sun'> <label class='form-check-label' for='voilages-sunset-radio'> Au couché du soleil à <input id='voilages-sunset-time' type='time' value='00:00' readonly> </label> </div> <div class='form-check'> <input class='form-check-input' type='radio' name='voilages-closing-radio' id='voilages-close-at-time-radio' value='sun'> <label class='form-check-label' for='voilages-close-at-time-radio'> À <input class='form-check-label' type='time' id='voilages-close-at-time' name='voilages-close-at-time' value='00:00'> </label> </div> </span> </div> </div> </div> </div> <!-- Buttons --> <div class='hstack col-md-8 mx-auto btn-group'> <button type='submit' class='btn btn-secondary'>Sauvegarder</button> <button type='reset' class='btn btn-outline-secondary'>Réinitialiser</button> </div> </div> </div> </form> <form name='mode-manuel' action='Manuel' method='get' onsubmit='return validateFormModeManuel()'>)";
  client.println(preformattedPage.data());
  preformattedPage = R"(<div class='accordion-item'> <h2 class='accordion-header'> <button class='accordion-button collapsed' type='button' data-bs-toggle='collapse' data-bs-target='#collapse-manuel' aria-expanded='false' aria-controls='collapse-manuel'>Mode manuel </button> </h2> <div id='collapse-manuel' class='accordion-collapse collapse' data-bs-parent='#accordion-settings'> <!--data-bs-parent='#accordion-settings'--> <div class='accordion-body'> <div class='accordion' id='accordionExample'> <div class='accordion' id='accordionPanelsStayOpenExample'> <!-- Rideaux --> <strong> <label id='server-time-label'>Unknown</label> </strong> <div class='accordion-item'> <h2 class='accordion-header'> <button class='accordion-button collapsed' type='button' data-bs-toggle='collapse' data-bs-target='#panelsStayOpen-collapseOne' aria-expanded='false' aria-controls='panelsStayOpen-collapseOne'>Rideaux </button> </h2> <div id='panelsStayOpen-collapseOne' class='accordion-collapse collapse show'> )";
  client.println(preformattedPage.data());
  preformattedPage = R"(<div class='accordion-body'> <div class='btn-group'> <button class='btn btn-secondary' name='mode-manuel-rideaux-button' value='open'>Ouvrir</button> <button class='btn btn-secondary' name='mode-manuel-rideaux-button' value='close'>Fermer</button> </div> </div> </div> </div> <!-- Voilages --> <div class='accordion-item'> <h2 class='accordion-header'> <button class='accordion-button collapsed' type='button' data-bs-toggle='collapse' data-bs-target='#panelsStayOpen-collapseTwo' aria-expanded='false' aria-controls='panelsStayOpen-collapseTwo'>Voilages </button> </h2> <div id='panelsStayOpen-collapseTwo' class='accordion-collapse collapse show'> <div class='accordion-body'> <div class='btn-group'> <button class='btn btn-secondary' name='mode-manuel-voilages-button' value='open'>Ouvrir</button> <button class='btn btn-secondary' name='mode-manuel-voilages-button' value='close'>Fermer</button> </div> </div> </div> </div> <div class='mt-2'> )";
  client.println(preformattedPage.data());
  preformattedPage = R"(<label for='mode-manuel-motor-speed-slider' class='form-label'> vitesse : <strong> <span id='motor-speed-label'>unknow</span> </strong> </label> <input name='mode-manuel-motor-speed-slider' type='range' class='form-range' id='mode-manuel-motor-speed-slider' min='1' max='255' value='100' oninput='updateSliderText()'> </div> </div> </div> </div> </div> </div> </form> </div> </main> <footer> <!-- place footer here --> </footer> <!-- Bootstrap JavaScript Libraries --> <script src='https://cdn.jsdelivr.net/npm/@popperjs/core@2.11.8/dist/umd/popper.min.js' integrity='sha384-I7E8VVD/ismYTF4hNIPjVp/Zjvgyol6VFvRkX/vR+Vc4jQkC+hVqc2pM8ODewa9r' crossorigin='anonymous'></script> <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.min.js' integrity='sha384-BBtl+eGJRgqQAUMxJ7pMwbEyER4l1g+O15P+16Ep7Q9Q+zqX6gSbd85u4mG4QzX+' crossorigin='anonymous'></script> </body></html> )";
  client.println(preformattedPage.data());

  // The HTTP response ends with another blank line:
  client.println();
}

void processHttpGetRequest(
    Config &config,
    const String &requestPath)
{

  if (requestPath.startsWith("/Manuel"))
  {
    bool voilagesOpen = false;
    bool voilagesClose = false;
    bool rideauxOpen = false;
    bool rideauxClose = false;

    int queryStartIndex = requestPath.indexOf('?') + 1;

    while (queryStartIndex < requestPath.length())
    {
      int labelEndIndex = requestPath.indexOf('=', queryStartIndex);
      if (labelEndIndex == -1)
        break;

      String label = requestPath.substring(queryStartIndex, labelEndIndex);
      int paramStartIndex = labelEndIndex + 1;
      int paramEndIndex = requestPath.indexOf('&', paramStartIndex);
      if (paramEndIndex == -1)
        paramEndIndex = requestPath.length();

      String param = requestPath.substring(paramStartIndex, paramEndIndex);
      queryStartIndex = paramEndIndex + 1;

      if (label.equals("mode-manuel-voilages-button"))
      {
        voilagesOpen = param.equals("open");
        voilagesClose = param.equals("close");
      }
      else if (label.equals("mode-manuel-rideaux-button"))
      {
        rideauxOpen = param.equals("open");
        rideauxClose = param.equals("close");
      }
      else if (label.equals("mode-manuel-motor-speed-slider"))
      {
        config.manualSpeed = param.toInt();
        Serial.print("== Receieved Spped: ");
        Serial.println(config.manualSpeed);
      }
    }

    Serial.println("== Motor Action");
    // Perform actions for voilages
    if (voilagesOpen)
      openRideauManuel(1);
    if (voilagesClose)
      closeRideauManuel(1);

    // Perform actions for rideaux
    if (rideauxOpen)
      openRideauManuel(0);
    if (rideauxClose)
      closeRideauManuel(0);
  }

  // Process auto configuration for rideaux or voilages
  bool isAutoRideaux = requestPath.startsWith("/Auto-Rideaux");
  bool isAutoVoilage = requestPath.startsWith("/Auto-Voilage");

  if (isAutoRideaux || isAutoVoilage)
  {
    int curtainIndex = isAutoVoilage ? 1 : 0;
    String prefix = isAutoVoilage ? "voilages" : "rideaux";

    // Default value if not present
    config.rideau[curtainIndex].isEnabled = false;

    int queryStartIndex = requestPath.indexOf('?') + 1;

    while (queryStartIndex < requestPath.length())
    {
      int labelEndIndex = requestPath.indexOf('=', queryStartIndex);
      if (labelEndIndex == -1)
        break;

      String label = requestPath.substring(queryStartIndex, labelEndIndex);
      int paramStartIndex = labelEndIndex + 1;
      int paramEndIndex = requestPath.indexOf('&', paramStartIndex);
      if (paramEndIndex == -1)
        paramEndIndex = requestPath.length();

      String param = requestPath.substring(paramStartIndex, paramEndIndex);
      queryStartIndex = paramEndIndex + 1;

      if (label.equals(prefix + "-switch"))
      {
        config.rideau[curtainIndex].isEnabled = param.equals("on");
      }
      else if (label.equals(prefix + "-opening-radio"))
      {
        bool isOpenAtSunrise = param.equals("sun");
        config.rideau[curtainIndex].isOpenAtSunrise = isOpenAtSunrise;
        config.rideau[curtainIndex].isOpenAtTime = !isOpenAtSunrise;
      }
      else if (label.equals(prefix + "-open-at-time"))
      {
        config.rideau[curtainIndex].openAtTime = param.substring(0, 2).toInt() * 60 + param.substring(5).toInt();
      }
      else if (label.equals("openSpeed"))
      {
        config.rideau[curtainIndex].openingSpeed = param.toInt();
      }
      else if (label.equals(prefix + "-closing-radio"))
      {
        bool isCloseAtSunset = param.equals("sun");
        config.rideau[curtainIndex].isCloseAtSunset = isCloseAtSunset;
        config.rideau[curtainIndex].isCloseAtTime = !isCloseAtSunset;
      }
      else if (label.equals(prefix + "-close-at-time"))
      {
        config.rideau[curtainIndex].closeAtTime = param.substring(0, 2).toInt() * 60 + param.substring(5).toInt();
      }
      else if (label.equals("closeSpeed"))
      {
        config.rideau[curtainIndex].closingSpeed = param.toInt();
      }
    }

    openRideauAuto(curtainIndex);
    closeRideauAuto(curtainIndex);
  }
}

/*
void processHttpGetRequest(
  class Config &config,
  class Data &data,
  String getLine) {

  if (getLine.startsWith("/Manuel")) {
    bool openAction[2] = { false, false };
    bool closeAction[2] = { false, false };

    unsigned int length = getLine.length();
    int idx = 0;
    while ((idx < length) && (getLine.charAt(idx) != '?')) idx++;
    idx++;

    while (idx < length) {
      unsigned int labelStart = idx;
      unsigned int labelEnd;
      unsigned int paramStart = idx;
      unsigned int paramEnd;
      while ((idx < length) && (getLine.charAt(idx) != '=')) idx++;
      labelEnd = idx;

      idx++;
      if (idx >= length)
        break;

      paramStart = idx;
      while ((idx < length) && (getLine.charAt(idx) != '&')) idx++;
      paramEnd = idx;
      idx++;

      // Label or parameter empty, skip
      if ((labelStart == labelEnd) && (paramStart >= paramEnd))
        continue;

      String label = getLine.substring(labelStart, labelEnd);
      String param = getLine.substring(paramStart, paramEnd);

      if (label == "mode-manuel-voilages-button") {
        if (param == "open") openAction[1] = true;
        if (param == "close") closeAction[1] = true;
      }

      if (label == "mode-manuel-rideaux-button") {
        if (param == "open") openAction[0] = true;
        if (param == "close") closeAction[0] = true;
      }

      if (label == "mode-manuel-motor-speed-slider")
        config.manualSpeed = param.toInt();
    }

    // Do action after processed all parameters
    for (idx =0; idx < 2; idx++)
    {
    if (openAction[idx] == true)
    openRideau(idx);

    if (closeAction[idx] == true)
    closeRideau(idx);

    }
  }



  if ((getLine.startsWith("/Auto-Rideaux") == true) || (getLine.startsWith("/Auto-Voilage") == true)) {
    int rideau = 0;
    String prefix = "rideaux";

    if (getLine.startsWith("/Auto-Voilage") == true) {
      rideau = 1;
      prefix = "voilages";
    }

    // Default value if not present
    config.rideau[rideau].isEnabled = false;

    unsigned int length = getLine.length();
    int idx = 0;
    while ((idx < length) && (getLine.charAt(idx) != '?')) idx++;
    idx++;

    while (idx < length) {
      unsigned int labelStart = idx;
      unsigned int labelEnd;
      unsigned int paramStart = idx;
      unsigned int paramEnd;
      while ((idx < length) && (getLine.charAt(idx) != '=')) idx++;
      labelEnd = idx;

      idx++;
      if (idx >= length)
        break;

      paramStart = idx;
      while ((idx < length) && (getLine.charAt(idx) != '&')) idx++;
      paramEnd = idx;
      idx++;

      // Label or parameter empty, skip
      if ((labelStart == labelEnd) && (paramStart >= paramEnd))
        continue;

      String label = getLine.substring(labelStart, labelEnd);
      String param = getLine.substring(paramStart, paramEnd);



      if (label == prefix + "-switch") {
        if (param == "on") {
          config.rideau[rideau].isEnabled = true;

          Serial.print("=== IN: rideaux-switch on :");
          Serial.print(rideau);
          Serial.print("    Value :");
          Serial.print(config.rideau[rideau].isEnabled);
          Serial.println(" ===");
        }

      } else if (label == prefix + "-opening-radio") {
        if (param == "sun") {
          config.rideau[rideau].isOpenAtSunrise = true;
          config.rideau[rideau].isOpenAtTime = false;
        } else if (param == "time") {
          config.rideau[rideau].isOpenAtSunrise = false;
          config.rideau[rideau].isOpenAtTime = true;
        }

      } else if (label == prefix + "-open-at-time") {
        config.rideau[rideau].openAtTime = param.substring(0, 2).toInt() * 60 + param.substring(5).toInt();
        // =00%3A00
      } else if (label == "openSpeed") {
        config.rideau[rideau].openingSpeed  = param.toInt();

      } else if (label == prefix + "-closing-radio") {
        if (param == "sun") {
          config.rideau[rideau].isCloseAtSunset = true;
          config.rideau[rideau].isCloseAtTime = false;
        } else if (param == "time") {
          config.rideau[rideau].isCloseAtSunset = false;
          config.rideau[rideau].isCloseAtTime = true;
        }
        // =None

      } else if (label == prefix + "-close-at-time") {
        config.rideau[rideau].closeAtTime = param.substring(0, 2).toInt() * 60 + param.substring(5).toInt();
        // =00%3A00
      } else if (label == "closeSpeed") {
        config.rideau[rideau].closingSpeed  = param.toInt();
      }
    }
  }
}
*/

void htmlRun(class Config &config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)())
{

  if (client)
  {                          // if you get a client,
    String currentLine = ""; // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
                                //  Serial.write(c);          // print it out the serial monitor
        if (c == '\n')
        { // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {

            generateHtmlPage(config, getTimeStr, getSunrise, getSunset);

            // break out of the while loop:
            break;
          }
          else
          { // if you got a newline, then clear currentLine:
            // GET /S HTTP/1.1
            if (currentLine.startsWith("GET "))
              processHttpGetRequest(config, currentLine.substring(4));
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    //    Serial.println("client disconnected");
  }
}
