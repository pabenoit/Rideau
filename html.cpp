#include <string>
#include <vector>
#include <functional>

#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#else
#include <WiFiNINA.h>
#endif

#include "html.h"
#include "Motor.h"
#include "configData.h"


extern WiFiClient client;

extern void closeManualCurtain(int idx);
extern void openManualCurtain(int idx);
extern void closeAutoCurtain(int idx);
extern void openAutoCurtain(int idx);
extern void curtainPeriodic(void);


static const char MAIN_page_part1[] PROGMEM = R"""(
<!doctype html><html lang="en"><head> <!-- Required meta tags --> <meta charset="UTF-8"> <meta name="keywords" content="HTML, CSS, JavaScript"> <meta name="author" content="Fygadi"> <meta name="viewport" content="width=device-width, initial-scale=1.0"> <title>Rideau Automatique</title> <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg width='24' height='24' fill='none' viewBox='0 0 24 24' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M11.996 18.532a1 1 0 0 1 .993.883l.007.117v1.456a1 1 0 0 1-1.993.116l-.007-.116v-1.456a1 1 0 0 1 1-1Zm6.037-1.932 1.03 1.03a1 1 0 0 1-1.415 1.413l-1.03-1.029a1 1 0 0 1 1.415-1.414Zm-10.66 0a1 1 0 0 1 0 1.414l-1.029 1.03a1 1 0 0 1-1.414-1.415l1.03-1.03a1 1 0 0 1 1.413 0ZM12.01 6.472a5.525 5.525 0 1 1 0 11.05 5.525 5.525 0 0 1 0-11.05Zm8.968 4.546a1 1 0 0 1 .117 1.993l-.117.007h-1.456a1 1 0 0 1-.116-1.993l.116-.007h1.456ZM4.479 10.99a1 1 0 0 1 .116 1.993l-.116.007H3.023a1 1 0 0 1-.117-1.993l.117-.007h1.456Zm1.77-6.116.095.083 1.03 1.03a1 1 0 0 1-1.32 1.497L5.958 7.4 4.93 6.371a1 1 0 0 1 1.32-1.497Zm12.813.083a1 1 0 0 1 .083 1.32l-.083.094-1.03 1.03a1 1 0 0 1-1.497-1.32l.084-.095 1.029-1.03a1 1 0 0 1 1.414 0ZM12 2.013a1 1 0 0 1 .993.883l.007.117v1.455a1 1 0 0 1-1.993.117L11 4.468V3.013a1 1 0 0 1 1-1Z' fill='%23ffaa00'/%3E%3C/svg%3E" /> <!-- Fonts --> <!-- Roboto --> <link rel="preconnect" href="https://fonts.googleapis.com"> <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin> <link href="https://fonts.googleapis.com/css2?family=Roboto:ital,wght@0,100;0,300;0,400;0,500;0,700;0,900;1,100;1,300;1,400;1,500;1,700;1,900&display=swap" rel="stylesheet"> <!-- Embed code --> <link rel="preconnect" href="https://fonts.googleapis.com"> <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin> <link href="https://fonts.googleapis.com/css2?family=Playfair+Display:ital,wght@0,400..900;1,400..900&display=swap" rel="stylesheet"> <!-- Bootstrap CSS v5.2.1 --> <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous"/> <style> input[role="switch"] { rotate: 180deg; } body { font-family: 'Embed code'; } .accordion { --bs-accordion-active-color: #5b99f5; --bs-accordion-color: #ffffff; --bs-accordion-active-bg: #2c1d3b; --bs-accordion-btn-bg: #492f50; --bs-accordion-bg: #4b3b5b; } .accordion-button::after { background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-plus' viewBox='0 0 16 16'%3E%3Cpath d='M8 4a.5.5 0 0 1 .5.5v3h3a.5.5 0 0 1 0 1h-3v3a.5.5 0 0 1-1 0v-3h-3a.5.5 0 0 1 0-1h3v-3A.5.5 0 0 1 8 4z'/%3E%3C/svg%3E"); } .accordion { --bs-accordion-btn-focus-box-shadow: none; } .accordion-button:not(.collapsed)::after { background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-dash' viewBox='0 0 16 16'%3E%3Cpath d='M4 8a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 0 1h-7A.5.5 0 0 1 4 8z'/%3E%3C/svg%3E"); } .accordion-button::after { transition: all 0.3s; } .card { --bs-card-color: #ffffff; --bs-card-bg: #4b3b5b; } /* footer */ .footer { background-color: #542d7c; width: auto; } .bd-placeholder-img { font-size: 1.125rem; text-anchor: middle; -webkit-user-select: none; -moz-user-select: none; user-select: none; } @media (min-width: 768px) { .bd-placeholder-img-lg { font-size: 3.5rem; } } .b-example-divider { width: 100%; height: 3rem; background-color: rgba(0, 0, 0, .1); border: solid rgba(0, 0, 0, .15); border-width: 1px 0; box-shadow: inset 0 .5em 1.5em rgba(0, 0, 0, .1), inset 0 .125em .5em rgba(0, 0, 0, .15); } .b-example-vr { flex-shrink: 0; width: 1.5rem; height: 100vh; } .bi { vertical-align: -.125em; fill: currentColor; } .nav-scroller { position: relative; z-index: 2; height: 2.75rem; overflow-y: hidden; } .nav-scroller .nav { display: flex; flex-wrap: nowrap; padding-bottom: 1rem; margin-top: -1px; overflow-x: auto; text-align: center; white-space: nowrap; -webkit-overflow-scrolling: touch; } .btn-bd-primary { --bd-violet-bg: #712cf9; --bd-violet-rgb: 112.520718, 44.062154, 249.437846; --bs-btn-font-weight: 600; --bs-btn-color: var(--bs-white); --bs-btn-bg: var(--bd-violet-bg); --bs-btn-border-color: var(--bd-violet-bg); --bs-btn-hover-color: var(--bs-white); --bs-btn-hover-bg: #6528e0; --bs-btn-hover-border-color: #6528e0; --bs-btn-focus-shadow-rgb: var(--bd-violet-rgb); --bs-btn-active-color: var(--bs-btn-hover-color); --bs-btn-active-bg: #5a23c8; --bs-btn-active-border-color: #5a23c8; } .bd-mode-toggle { z-index: 1500; } .bd-mode-toggle .dropdown-menu .active .bi { display: block !important; } .form-control-dark { border-color: var(--bs-gray); } .form-control-dark:focus { border-color: #fff; box-shadow: 0 0 0 .25rem rgba(255, 255, 255, .25); } .text-small { font-size: 85%; } .dropdown-toggle:not(:focus) { outline: 0; } </style>
)""";

static const char MAIN_page_part2[] PROGMEM = R"""(
<script> //InitialiseSite function initialisePage() { initiliseInputOnLoad(); loadAccordionStateOnload(); toggleItemRideaux(); toggleItemVoilages(); } function initiliseInputOnLoad() { //Others document.getElementById('serverTimeLabel').innerText = settings.serverTime; document.getElementById('modeManuelMotorSpeedSlider').value = settings.motorSpeed; document.getElementById('motorSpeedLabel').innerText = settings.motorSpeed; //Rideaux document.getElementById('rideauxSwitch').checked = settings.rideaux.rideauxSwitchOn; //Open document.getElementById('rideauxSunriseRadio').checked = settings.rideaux.opening.isSunrise; document.getElementById('rideauxSunriseTime').value = settings.sunriseTime; document.getElementById('rideauxOpenAtTimeRadio').checked = settings.rideaux.opening.isAtTime; document.getElementById('rideauxOpenAtTime').value = settings.rideaux.opening.atTime; //Close document.getElementById('rideauxSunsetRadio').checked = settings.rideaux.closing.isSunset; document.getElementById('rideauxSunsetTime').value = settings.sunsetTime; document.getElementById('rideauxCloseAtTimeRadio').checked = settings.rideaux.closing.isAtTime; document.getElementById('rideauxCloseAtTime').value = settings.rideaux.closing.atTime; //Voilages document.getElementById('voilagesSwitch').checked = settings.voilages.voilagesSwitchOn; //Open document.getElementById('voilagesSunriseRadio').checked = settings.voilages.opening.isSunrise; document.getElementById('voilagesSunriseTime').value = settings.sunriseTime; document.getElementById('voilagesOpenAtTimeRadio').checked = settings.voilages.opening.isAtTime; document.getElementById('voilagesOpenAtTime').value = settings.voilages.opening.atTime; //Close document.getElementById('voilagesSunsetRadio').checked = settings.voilages.closing.isSunset; document.getElementById('voilagesSunsetTime').value = settings.sunsetTime; document.getElementById('voilagesCloseAtTimeRadio').checked = settings.voilages.closing.isAtTime; document.getElementById('voilagesCloseAtTime').value = settings.voilages.closing.atTime; } function loadAccordionStateOnload() { // Select all elements with the class .accordion-item const accordionItems = document.querySelectorAll('.accordion-item'); // Loop through each .accordion-item for (let i = 0; i < accordionItems.length; i++) { const item = accordionItems[i]; const accordionButton = item.querySelectorAll('.accordion-button')[0]; const accordionCollapse = item.querySelectorAll('.accordion-collapse')[0]; let accordionState = getCookie(accordionCollapse.id); if (accordionState === 'expanded') { accordionButton.classList.remove('collapsed'); accordionCollapse.classList.add('show'); } else { accordionButton.classList.add('collapsed'); accordionCollapse.classList.remove('show'); } } } //Disable sections function toggleItemRideaux() { enableDisableAllChildren('cardGroupRideaux', 'rideauxSwitch'); } function toggleItemVoilages() { enableDisableAllChildren('cardGroupVoilages', 'voilagesSwitch'); } function enableDisableAllChildren(parentID, boolEnableSwitchID) { let parent = document.getElementById(parentID); let boolEnable = !(document.getElementById(boolEnableSwitchID).checked); let childElements = parent.getElementsByTagName('*'); for (let i = 0; i < childElements.length; i++) { let element = childElements[i ]; if (element.tagName !== 'DIV') { // Avoid disabling child divs recursively element.disabled = boolEnable; if (element.tagName === 'INPUT' || element.tagName === 'TEXTAREA') { element.style.pointerEvents = boolEnable ? 'none' : 'auto'; element.style.opacity = boolEnable ? '0.5' : '1'; } } } // Disable or enable the parent parent.readonly = boolEnable; if (boolEnable) { parent.style.pointerEvents = 'none'; parent.style.opacity = '0.5'; } else { parent.style.pointerEvents = 'auto'; parent.style.opacity = '1'; } } //Slider update text value function updateSliderText() { let slider = document.getElementById("modeManuelMotorSpeedSlider"); let span = document.getElementById("motorSpeedLabel"); span.textContent = slider.value; span.style.fontWeight = "bold"; } //Form validation function validateFormRideaux() { //TODO: return true; } function validateFormVoilages() { //TODO: return true; } function validateFormModeManuel() { //TODO: return true; } //Form Reset function resetFormRideaux(event) { //TODO: find a better way to do it event.preventDefault(); //Rideaux document.getElementById('rideauxSwitch').checked = settings.rideaux.rideauxSwitchOn; //Open document.getElementById('rideauxSunriseRadio').checked = settings.rideaux.opening.isSunrise; document.getElementById('rideauxSunriseTime').value = settings.sunriseTime; document.getElementById('rideauxOpenAtTimeRadio').checked = settings.rideaux.opening.isAtTime; document.getElementById('rideauxOpenAtTime').value = settings.rideaux.opening.atTime; //Close document.getElementById('rideauxSunsetRadio').checked = settings.rideaux.closing.isSunset; document.getElementById('rideauxSunsetTime').value = settings.sunsetTime; document.getElementById('rideauxCloseAtTimeRadio').checked = settings.rideaux.closing.isAtTime; document.getElementById('rideauxCloseAtTime').value = settings.rideaux.closing.atTime; toggleItemRideaux(); } function resetFormVoilages(event) { //TODO: find a better way to do it event.preventDefault(); //Voilages document.getElementById('voilagesSwitch').checked = settings.voilages.voilagesSwitchOn; //Open document.getElementById('voilagesSunriseRadio').checked = settings.voilages.opening.isSunrise; document.getElementById('voilagesSunriseTime').value = settings.sunriseTime; document.getElementById('voilagesOpenAtTimeRadio').checked = settings.voilages.opening.isAtTime; document.getElementById('voilagesOpenAtTime').value = settings.voilages.opening.atTime; //Close document.getElementById('voilagesSunsetRadio').checked = settings.voilages.closing.isSunset; document.getElementById('voilagesSunsetTime').value = settings.sunsetTime; document.getElementById('voilagesCloseAtTimeRadio').checked = settings.voilages.closing.isAtTime; document.getElementById('voilagesCloseAtTime').value = settings.voilages.closing.atTime; toggleItemVoilages(); } // Attach event listener for accordion state change document.addEventListener('shown.bs.collapse', function (event) { setCookie(event.target.id, 'expanded', 0); }); document.addEventListener('hidden.bs.collapse', function (event) { setCookie(event.target.id, 'collapsed', 0); }); //Cookies declare en premiere car sinon pas exister function setCookie(cname, cvalue, exdays) { const d = new Date(); d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000)); let expires = (exdays !== 0) ? "expires=" + d.toUTCString() : ''; document.cookie = cname + "=" + cvalue.toString() + ";" + expires + ";path=/" + ";SameSite=Lax"; } function getCookie(cname) { let name = cname + "="; let ca = document.cookie.split(';'); for (let i = 0; i < ca.length; i++) { let c = ca[i]; while (c.charAt(0) == ' ') { c = c.substring(1); } if (c.indexOf(name) == 0) { return c.substring(name.length, c.length); } } return ""; } //https://getbootstrap.com/docs/4.0/components/collapse/#collapseoptions function collapseItem(id, option) { let element = document.querySelector(id); // try to get collapse instance let bsCollapse = bootstrap.Collapse.getInstance(element); // if the instance is not yet initialized then create new collapse if (bsCollapse === null) { bsCollapse = new bootstrap.Collapse(element, { toggle: false // this parameter is important! }) } console.log(option); if (option === 'show') bsCollapse.show(); else if (option === 'hide') bsCollapse.hide(); else if ('toggle') bsCollapse.toggle(); } </script></head><body data-bs-theme="dark" onload="initialisePage()"> <div class="container"> <header class="d-flex flex-wrap justify-content-center py-3 mb-4 border-bottom"> <h1 class="d-flex align-items-center mb-3 mb-md-0 me-md-auto"> <span class="fs-4">Rideaux automatique</span> </h1> <ul class="nav nav-pills"> <li class="nav-item"><a href="#accordionItemRideaux" class="nav-link" onclick="collapseItem('#accordionItemRideaux', 'show')">Rideaux</a></li> <li class="nav-item"><a href="#accordionItemVoilages" class="nav-link" onclick="collapseItem('#accordionItemVoilages', 'show')">Voilages</a></li> <li class="nav-item"><a href="#accordionItemModeManuel" class="nav-link" onclick="collapseItem('#accordionItemModeManuel', 'show')">Manuel</a></li> <li class="dropdown"> <a class="nav-link dropdown-toggle" href="#" role="button" data-bs-toggle="dropdown" aria-expanded="false"> Others </a> <ul class="dropdown-menu"> <li><a class="dropdown-item" href="https://github.com/Fygadi" target="_blank">Github</a></li> <li><a class="dropdown-item" href="#credits" onclick="collapseItem('#creditsCollapse', 'toggle')">Crédits</a></li> </ul> </li> </ul> </header> </div> <main class="container"> <!-- accordion --> <div class="accordion mb-5" id="accordionSettings"> <!-- item 1 --> <form method="post" action="/Auto-Rideaux" name="rideauxForm" onsubmit="return validateFormRideaux()" onreset="resetFormRideaux(event)"> <div class="accordion-item"> <h2 class="accordion-header"> <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#accordionItemRideaux" aria-expanded="false" aria-controls="accordionItemRideaux"> <label class="h4">Rideaux</span> </button> </h2> <div id="accordionItemRideaux" class="accordion-collapse collapse" data-bs-parent="#accordionSettings"> <div class="accordion-body"> <div class="form-check form-switch mx-3 mb-3"> <input class="form-check-input" type="checkbox" role="switch" onchange="toggleItemRideaux()" id="rideauxSwitch" name="rideauxSwitch" checked> <label class="form-check-label" for="rideauxSwitch"> On / Off </label> </div> <div class="card-group" id="cardGroupRideaux"> <!-- cardgroup --> <div class="card"> <!-- Rideaux Ouverture --> <div class="card-body"> <h4 class="card-title text-center">Ouverture</h4> <div class="form-check"> <input class="form-check-input" type="radio" name="rideauxOpeningRadio" id="rideauxSunriseRadio" required value="sun"> <label class="form-check-label" for="rideauxSunriseRadio"> Au lever du soleil à <input id="rideauxSunriseTime" type="time" value="06:15" readonly onclick="this.blur()"> </label> </div> <div class="form-check"> <input class="form-check-input" type="radio" name="rideauxOpeningRadio" id="rideauxOpenAtTimeRadio" value="time"> <label class="form-check-label" for="rideauxOpenAtTimeRadio"> À <input class="form-check-label" type="time" id="rideauxOpenAtTime" name="rideauxOpenAtTime" value="07:00"> </label> </div> </div> </div> <div class="card"> <!-- Rideaux Fermeture --> <div class="card-body"> <h4 class="card-title text-center">Fermeture</h4> <div class="form-check"> <input class="form-check-input" type="radio" name="rideauxClosingRadio" id="rideauxSunsetRadio" value="sun"> <label class="form-check-label" for="rideauxSunsetRadio"> Au couché du soleil à <input id="rideauxSunsetTime" type="time" value="18:45" readonly onclick="this.blur()"> </label> </div> <div class="form-check"> <input class="form-check-input" type="radio" name="rideauxClosingRadio" id="rideauxCloseAtTimeRadio" value="time"> <label class="form-check-label" for="rideauxClosingLeverTime"> À <input class="form-check-label" type="time" id="rideauxCloseAtTime" name="rideauxCloseAtTime" value="20:00"> </label> </div> </div> </div> </div> </div> <!-- Buttons --> <div class="hstack col-md-8 mx-auto btn-group"> <button type="submit" class="btn btn-primary">Sauvegarder</button> <button type="reset" class="btn btn-secondary">Réinitialiser</button> </div> </div> </div> </form> <!-- item 2 --> <form method="post" action="/Auto-Voilages" name="voilagesForm" onsubmit="return validateFormVoilages()" onreset="resetFormVoilages(event)"> <div class="accordion-item"> <h2 class="accordion-header"> <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#accordionItemVoilages" aria-expanded="false" aria-controls="accordionItemVoilages"> <label class="h4">Voilages</label> </button> </h2> <div id="accordionItemVoilages" class="accordion-collapse collapse" data-bs-parent="#accordionSettings"> <div class="accordion-body"> <div class="form-check form-switch mx-3 mb-3"> <input onclick="toggleItemVoilages()" class="form-check-input" type="checkbox" role="switch" name="voilagesSwitch" id="voilagesSwitch" checked> <input type="hidden" name="voilagesSwitch" value="off"> <label class="form-check-label" for="voilagesSwitch">On / Off</label> </div> <div class="card-group" id="cardGroupVoilages"> <!-- Cardgroup --> <div class="card"> <!-- Voilages Ouverture --> <div class="card-body"> <h4 class="card-title text-center">Ouverture</h4> <div class="form-check"> <input class="form-check-input" type="radio" name="voilagesOpeningRadio" id="voilagesSunriseRadio" value="sun"> <label class="form-check-label" for="voilagesSunriseRadio"> Au lever du soleil à <input id="voilagesSunriseTime" type="time" value="00:00" readonly onclick="this.blur()"> </label> </div> <div class="form-check"> <input class="form-check-input" type="radio" name="voilagesOpeningRadio" id="voilagesOpenAtTimeRadio" value="time"> <label class="form-check-label" for="voilagesOpenAtTimeRadio"> À <input class="form-check-label" type="time" id="voilagesOpenAtTime" name="voilagesOpenAtTime" value="00:00"> </label> </div> </div> </div> <div class="card"> <!-- voilage Fermeture --> <div class="card-body"> <h4 class="card-title text-center">Fermeture</h4> <div class="form-check"> <input class="form-check-input" type="radio" name="voilagesClosingRadio" id="voilagesSunsetRadio" value="sun"> <label class="form-check-label" for="voilagesSunsetRadio"> Au couché du soleil à <input id="voilagesSunsetTime" type="time" value="00:00" readonly> </label> </div> <div class="form-check"> <input class="form-check-input" type="radio" name="voilagesClosingRadio" id="voilagesCloseAtTimeRadio" value="sun"> <label class="form-check-label" for="voilagesCloseAtTimeRadio"> À <input class="form-check-label" type="time" id="voilagesCloseAtTime" name="voilagesCloseAtTime" value="00:00"> </label> </div> </div> </div> </div> </div> <!-- Buttons --> <div class="hstack col-md-8 mx-auto btn-group"> <button type="submit" class="btn btn-primary">Sauvegarder</button> <button type="reset" class="btn btn-secondary">Réinitialiser</button> </div> </div> </div> </form> <!-- item 3 --> <form method="post" action="/Manuel" name="modeManuel" onsubmit="return validateFormModeManuel()"> <div class="accordion-item"> <h2 class="accordion-header"> <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#accordionItemModeManuel" aria-expanded="false" aria-controls="accordionItemModeManuel"> <span class="h4">Mode manuel</span> </button> </h2> <div id="accordionItemModeManuel" class="accordion-collapse collapse" data-bs-parent="#accordionSettings"> <div class="accordion-body"> <div class="accordion" id="accordionExample"> <div class="accordion" id="accordionPanelsStayOpenExample"> Date et heure du serveur: <strong><label id="serverTimeLabel">Unknown</label></strong> <!-- Item 1 --> <div class="accordion-item"> <h4 class="accordion-header"> <button class="accordion-button" type="button" data-bs-toggle="collapse" data-bs-target="#accordionItemModeManuelRideaux" aria-expanded="true" aria-controls="accordionItemModeManuelRideaux"> Rideaux </button> </h4> <div id="accordionItemModeManuelRideaux" class="accordion-collapse collapse"> <div class="accordion-body"> <div class="btn-group"> <button class="btn btn-primary" name="modeManuelRideauxButton" value="open">Ouvrir</button> <button class="btn btn-primary" name="modeManuelRideauxButton" value="close">Fermer</button> </div> </div> </div> </div> <!-- Item 2 --> <div class="accordion-item"> <h2 class="accordion-header"> <button class="accordion-button" type="button" data-bs-toggle="collapse" data-bs-target="#accordionItemModeManuelVoilages" aria-expanded="true" aria-controls="accordionItemModeManuelVoilages"> Voilages </button> </h2> <div id="accordionItemModeManuelVoilages" class="accordion-collapse collapse"> <div class="accordion-body"> <div class="btn-group"> <button class="btn btn-primary" name="modeManuelVoilagesButton" value="open">Ouvrir</button> <button class="btn btn-primary" name="modeManuelVoilagesButton" value="close">Fermer</button> </div> </div> </div> </div> <div class="mt-2"> <label for="modeManuelMotorSpeedSlider" class="form-label"> Vitesse : <strong><span id="motorSpeedLabel">unknow</span></strong></label> <input name="modeManuelMotorSpeedSlider" type="range" class="form-range" id="modeManuelMotorSpeedSlider" min="1" max="255" value="100" oninput="updateSliderText()"> </div> </div> </div> </div> </div> </div> </form> </div> </main> <footer> <div class="container footer"> <div class="collapse mx-3 mt-3" id="creditsCollapse"> <h2 class="text-body-emphasis mb-2 h3"><strong class="h3">Crédits</strong></h2> <p class="text-body-secondary h6"> Développement serveur par <strong class="text-body-emphasis">Pascal Benoit</strong><br> Développement web par <strong class="text-body-emphasis">Victor Benoit</strong> <br> <strong class="text-body-emphasis">©Copyright 2024 Tous droits réservés</strong></p> </div> <div class="container text-center"> <div class="row align-items-center"> <div class="col"> <div class="container"> <nav class="navbar"> <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#creditsCollapse" aria-controls="creditsCollapse" aria-expanded="false" aria-label="Toggle navigation"> <span class="navbar-toggler-icon"></span> </button> </nav> </div> </div> <div class="col"> <a class="nav-link" href="https://github.com/Fygadi" target="_blank"> <!-- github svg --> <svg width="40px" height="40px" viewBox="0 0 20 20" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"> <title>github [#142]</title> <desc>Created with Sketch.</desc> <g id="Page-1" stroke="none" stroke-width="1" fill="none" fill-rule="evenodd"> <g id="Dribbble-Light-Preview" transform="translate(-140.000000, -7559.000000)" fill="#000000"> <g id="icons" transform="translate(56.000000, 160.000000)"> <path d="M94,7399 C99.523,7399 104,7403.59 104,7409.253 C104,7413.782 101.138,7417.624 97.167,7418.981 C96.66,7419.082 96.48,7418.762 96.48,7418.489 C96.48,7418.151 96.492,7417.047 96.492,7415.675 C96.492,7414.719 96.172,7414.095 95.813,7413.777 C98.04,7413.523 100.38,7412.656 100.38,7408.718 C100.38,7407.598 99.992,7406.684 99.35,7405.966 C99.454,7405.707 99.797,7404.664 99.252,7403.252 C99.252,7403.252 98.414,7402.977 96.505,7404.303 C95.706,7404.076 94.85,7403.962 94,7403.958 C93.15,7403.962 92.295,7404.076 91.497,7404.303 C89.586,7402.977 88.746,7403.252 88.746,7403.252 C88.203,7404.664 88.546,7405.707 88.649,7405.966 C88.01,7406.684 87.619,7407.598 87.619,7408.718 C87.619,7412.646 89.954,7413.526 92.175,7413.785 C91.889,7414.041 91.63,7414.493 91.54,7415.156 C90.97,7415.418 89.522,7415.871 88.63,7414.304 C88.63,7414.304 88.101,7413.319 87.097,7413.247 C87.097,7413.247 86.122,7413.234 87.029,7413.87 C87.029,7413.87 87.684,7414.185 88.139,7415.37 C88.139,7415.37 88.726,7417.2 91.508,7416.58 C91.513,7417.437 91.522,7418.245 91.522,7418.489 C91.522,7418.76 91.338,7419.077 90.839,7418.982 C86.865,7417.627 84,7413.783 84,7409.253 C84,7403.59 88.478,7399 94,7399" id="github-[#142]"> </path> </g> </g> </g> </svg> </a> </div> </div> </div> </div> </footer> <!-- Bootstrap JavaScript Libraries --> <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.11.8/dist/umd/popper.min.js" integrity="sha384-I7E8VVD/ismYTF4hNIPjVp/Zjvgyol6VFvRkX/vR+Vc4jQkC+hVqc2pM8ODewa9r" crossorigin="anonymous"></script> <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.min.js" integrity="sha384-BBtl+eGJRgqQAUMxJ7pMwbEyER4l1g+O15P+16Ep7Q9Q+zqX6gSbd85u4mG4QzX+" crossorigin="anonymous"></script> </body></html>
)""";

#if 0
// Using const, data should be write with the code simply.
static const char MAIN_page_part1[] PROGMEM = R"""(
<!doctype html>
<html lang="en">

<head>
  <!-- Required meta tags -->
  <meta charset="UTF-8">
  <meta name="keywords" content="HTML, CSS, JavaScript">
  <meta name="author" content="Fygadi">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">

  <title>Rideau Automatique</title>
  <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg width='24' height='24' fill='none' viewBox='0 0 24 24' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M11.996
  18.532a1 1 0 0 1 .993.883l.007.117v1.456a1 1 0 0 1-1.993.116l-.007-.116v-1.456a1 1 0 0 1 1-1Zm6.037-1.932 1.03 1.03a1
  1 0 0 1-1.415 1.413l-1.03-1.029a1 1 0 0 1 1.415-1.414Zm-10.66 0a1 1 0 0 1 0 1.414l-1.029 1.03a1 1 0 0
  1-1.414-1.415l1.03-1.03a1 1 0 0 1 1.413 0ZM12.01 6.472a5.525 5.525 0 1 1 0 11.05 5.525 5.525 0 0 1 0-11.05Zm8.968
  4.546a1 1 0 0 1 .117 1.993l-.117.007h-1.456a1 1 0 0 1-.116-1.993l.116-.007h1.456ZM4.479 10.99a1 1 0 0 1 .116
  1.993l-.116.007H3.023a1 1 0 0 1-.117-1.993l.117-.007h1.456Zm1.77-6.116.095.083 1.03 1.03a1 1 0 0 1-1.32 1.497L5.958
  7.4 4.93 6.371a1 1 0 0 1 1.32-1.497Zm12.813.083a1 1 0 0 1 .083 1.32l-.083.094-1.03 1.03a1 1 0 0 1-1.497-1.32l.084-.095
  1.029-1.03a1 1 0 0 1 1.414 0ZM12 2.013a1 1 0 0 1 .993.883l.007.117v1.455a1 1 0 0 1-1.993.117L11 4.468V3.013a1 1 0 0 1
  1-1Z' fill='%23ffaa00'/%3E%3C/svg%3E" />

  <!-- Fonts -->
  <!-- Roboto -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link
    href="https://fonts.googleapis.com/css2?family=Roboto:ital,wght@0,100;0,300;0,400;0,500;0,700;0,900;1,100;1,300;1,400;1,500;1,700;1,900&display=swap"
    rel="stylesheet">

  <!-- Embed code -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Playfair+Display:ital,wght@0,400..900;1,400..900&display=swap"
    rel="stylesheet">

  <!-- Bootstrap CSS v5.2.1 -->
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet"
    integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous"/>
  
  <style>
    input[role="switch"] {
      rotate: 180deg;
    }

    body {
      font-family: 'Embed code';
    }

    .accordion {
      --bs-accordion-active-color: #5b99f5;
      --bs-accordion-color: #ffffff;
      --bs-accordion-active-bg: #2c1d3b;
      --bs-accordion-btn-bg: #492f50;
      --bs-accordion-bg: #4b3b5b;
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
      --bs-card-color: #ffffff;
      --bs-card-bg: #4b3b5b;
    }


    /* footer */
    .footer {
      background-color: #542d7c;
      width: auto;
    }

    .bd-placeholder-img {
      font-size: 1.125rem;
      text-anchor: middle;
      -webkit-user-select: none;
      -moz-user-select: none;
      user-select: none;
    }

    @media (min-width: 768px) {
      .bd-placeholder-img-lg {
        font-size: 3.5rem;
      }
    }

    .b-example-divider {
      width: 100%;
      height: 3rem;
      background-color: rgba(0, 0, 0, .1);
      border: solid rgba(0, 0, 0, .15);
      border-width: 1px 0;
      box-shadow: inset 0 .5em 1.5em rgba(0, 0, 0, .1), inset 0 .125em .5em rgba(0, 0, 0, .15);
    }

    .b-example-vr {
      flex-shrink: 0;
      width: 1.5rem;
      height: 100vh;
    }

    .bi {
      vertical-align: -.125em;
      fill: currentColor;
    }

    .nav-scroller {
      position: relative;
      z-index: 2;
      height: 2.75rem;
      overflow-y: hidden;
    }

    .nav-scroller .nav {
      display: flex;
      flex-wrap: nowrap;
      padding-bottom: 1rem;
      margin-top: -1px;
      overflow-x: auto;
      text-align: center;
      white-space: nowrap;
      -webkit-overflow-scrolling: touch;
    }

    .btn-bd-primary {
      --bd-violet-bg: #712cf9;
      --bd-violet-rgb: 112.520718, 44.062154, 249.437846;

      --bs-btn-font-weight: 600;
      --bs-btn-color: var(--bs-white);
      --bs-btn-bg: var(--bd-violet-bg);
      --bs-btn-border-color: var(--bd-violet-bg);
      --bs-btn-hover-color: var(--bs-white);
      --bs-btn-hover-bg: #6528e0;
      --bs-btn-hover-border-color: #6528e0;
      --bs-btn-focus-shadow-rgb: var(--bd-violet-rgb);
      --bs-btn-active-color: var(--bs-btn-hover-color);
      --bs-btn-active-bg: #5a23c8;
      --bs-btn-active-border-color: #5a23c8;
    }

    .bd-mode-toggle {
      z-index: 1500;
    }

    .bd-mode-toggle .dropdown-menu .active .bi {
      display: block !important;
    }


    .form-control-dark {
      border-color: var(--bs-gray);
    }

    .form-control-dark:focus {
      border-color: #fff;
      box-shadow: 0 0 0 .25rem rgba(255, 255, 255, .25);
    }

    .text-small {
      font-size: 85%;
    }

    .dropdown-toggle:not(:focus) {
      outline: 0;
    }
  </style>

)""";



static const char MAIN_page_part2[] PROGMEM = R"""(
   <script>
   //InitialiseSite
    function initialisePage() {
      initiliseInputOnLoad();
      loadAccordionStateOnload();
      toggleItemRideaux();
      toggleItemVoilages();
    }

    function initiliseInputOnLoad() {
      //Others
      document.getElementById('serverTimeLabel').innerText = settings.serverTime;
      document.getElementById('modeManuelMotorSpeedSlider').value = settings.motorSpeed;
      document.getElementById('motorSpeedLabel').innerText = settings.motorSpeed;

      //Rideaux
      document.getElementById('rideauxSwitch').checked = settings.rideaux.rideauxSwitchOn;
      //Open
      document.getElementById('rideauxSunriseRadio').checked = settings.rideaux.opening.isSunrise;
      document.getElementById('rideauxSunriseTime').value = settings.sunriseTime;
      document.getElementById('rideauxOpenAtTimeRadio').checked = settings.rideaux.opening.isAtTime;
      document.getElementById('rideauxOpenAtTime').value = settings.rideaux.opening.atTime;
      //Close
      document.getElementById('rideauxSunsetRadio').checked = settings.rideaux.closing.isSunset;
      document.getElementById('rideauxSunsetTime').value = settings.sunsetTime;
      document.getElementById('rideauxCloseAtTimeRadio').checked = settings.rideaux.closing.isAtTime;
      document.getElementById('rideauxCloseAtTime').value = settings.rideaux.closing.atTime;

      //Voilages
      document.getElementById('voilagesSwitch').checked = settings.voilages.voilagesSwitchOn;
      //Open
      document.getElementById('voilagesSunriseRadio').checked = settings.voilages.opening.isSunrise;
      document.getElementById('voilagesSunriseTime').value = settings.sunriseTime;
      document.getElementById('voilagesOpenAtTimeRadio').checked = settings.voilages.opening.isAtTime;
      document.getElementById('voilagesOpenAtTime').value = settings.voilages.opening.atTime;
      //Close
      document.getElementById('voilagesSunsetRadio').checked = settings.voilages.closing.isSunset;
      document.getElementById('voilagesSunsetTime').value = settings.sunsetTime;
      document.getElementById('voilagesCloseAtTimeRadio').checked = settings.voilages.closing.isAtTime;
      document.getElementById('voilagesCloseAtTime').value = settings.voilages.closing.atTime;
    }

    function loadAccordionStateOnload() {
      // Select all elements with the class .accordion-item
      const accordionItems = document.querySelectorAll('.accordion-item');
      // Loop through each .accordion-item
      for (let i = 0; i < accordionItems.length; i++) {
        const item = accordionItems[i];
        const accordionButton = item.querySelectorAll('.accordion-button')[0];
        const accordionCollapse = item.querySelectorAll('.accordion-collapse')[0];

        let accordionState = getCookie(accordionCollapse.id);

        if (accordionState === 'expanded') {
          accordionButton.classList.remove('collapsed');
          accordionCollapse.classList.add('show');
        }
        else {
          accordionButton.classList.add('collapsed');
          accordionCollapse.classList.remove('show');
        }
      }
    }

    //Disable sections
    function toggleItemRideaux() {
      enableDisableAllChildren('cardGroupRideaux', 'rideauxSwitch');
    }
    function toggleItemVoilages() {
      enableDisableAllChildren('cardGroupVoilages', 'voilagesSwitch');
    }
    function enableDisableAllChildren(parentID, boolEnableSwitchID) {
      let parent = document.getElementById(parentID);
      let boolEnable = !(document.getElementById(boolEnableSwitchID).checked);

      let childElements = parent.getElementsByTagName('*');
      for (let i = 0; i < childElements.length; i++) {
        let element = childElements[i ];
        if (element.tagName !== 'DIV') { // Avoid disabling child divs recursively
          element.disabled = boolEnable;
          if (element.tagName === 'INPUT' || element.tagName === 'TEXTAREA') {
            element.style.pointerEvents = boolEnable ? 'none' : 'auto';
            element.style.opacity = boolEnable ? '0.5' : '1';
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
      let slider = document.getElementById("modeManuelMotorSpeedSlider");
      let span = document.getElementById("motorSpeedLabel");
      span.textContent = slider.value;
      span.style.fontWeight = "bold";
    }

    //Form validation
    function validateFormRideaux() {
      //TODO:
      return true;
    }
    function validateFormVoilages() {
      //TODO:
      return true;
    }
    function validateFormModeManuel() {
      //TODO:
      return true;
    }

    //Form Reset
    function resetFormRideaux(event) {
      //TODO: find a better way to do it
      event.preventDefault();
      //Rideaux
      document.getElementById('rideauxSwitch').checked = settings.rideaux.rideauxSwitchOn;
      //Open
      document.getElementById('rideauxSunriseRadio').checked = settings.rideaux.opening.isSunrise;
      document.getElementById('rideauxSunriseTime').value = settings.sunriseTime;
      document.getElementById('rideauxOpenAtTimeRadio').checked = settings.rideaux.opening.isAtTime;
      document.getElementById('rideauxOpenAtTime').value = settings.rideaux.opening.atTime;
      //Close
      document.getElementById('rideauxSunsetRadio').checked = settings.rideaux.closing.isSunset;
      document.getElementById('rideauxSunsetTime').value = settings.sunsetTime;
      document.getElementById('rideauxCloseAtTimeRadio').checked = settings.rideaux.closing.isAtTime;
      document.getElementById('rideauxCloseAtTime').value = settings.rideaux.closing.atTime;
      toggleItemRideaux();
    }
    function resetFormVoilages(event) {
      //TODO: find a better way to do it
      event.preventDefault();
      //Voilages
      document.getElementById('voilagesSwitch').checked = settings.voilages.voilagesSwitchOn;
      //Open
      document.getElementById('voilagesSunriseRadio').checked = settings.voilages.opening.isSunrise;
      document.getElementById('voilagesSunriseTime').value = settings.sunriseTime;
      document.getElementById('voilagesOpenAtTimeRadio').checked = settings.voilages.opening.isAtTime;
      document.getElementById('voilagesOpenAtTime').value = settings.voilages.opening.atTime;
      //Close
      document.getElementById('voilagesSunsetRadio').checked = settings.voilages.closing.isSunset;
      document.getElementById('voilagesSunsetTime').value = settings.sunsetTime;
      document.getElementById('voilagesCloseAtTimeRadio').checked = settings.voilages.closing.isAtTime;
      document.getElementById('voilagesCloseAtTime').value = settings.voilages.closing.atTime;
      toggleItemVoilages();
    }

    // Attach event listener for accordion state change
    document.addEventListener('shown.bs.collapse', function (event) {
      setCookie(event.target.id, 'expanded', 0);
    });
    document.addEventListener('hidden.bs.collapse', function (event) {
      setCookie(event.target.id, 'collapsed', 0);
    });

    //Cookies declare en premiere car sinon pas exister
    function setCookie(cname, cvalue, exdays) {
      const d = new Date();
      d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
      let expires = (exdays !== 0) ? "expires=" + d.toUTCString() : '';
      document.cookie = cname + "=" + cvalue.toString() + ";" + expires + ";path=/" + ";SameSite=Lax";
    }
    function getCookie(cname) {
      let name = cname + "=";
      let ca = document.cookie.split(';');
      for (let i = 0; i < ca.length; i++) {
        let c = ca[i];
        while (c.charAt(0) == ' ') {
          c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
          return c.substring(name.length, c.length);
        }
      }
      return "";
    }

    //https://getbootstrap.com/docs/4.0/components/collapse/#collapseoptions
    function collapseItem(id, option)
    {
      let element = document.querySelector(id);
    // try to get collapse instance
    let bsCollapse = bootstrap.Collapse.getInstance(element);
    // if the instance is not yet initialized then create new collapse
    if (bsCollapse === null) {
        bsCollapse = new bootstrap.Collapse(element, {
            toggle: false   // this parameter is important!
        })
    }

    console.log(option);
    if (option === 'show')
      bsCollapse.show();
    else if (option === 'hide')
      bsCollapse.hide();
    else if ('toggle')
      bsCollapse.toggle();
    }
  </script>
</head>

<body data-bs-theme="dark" onload="initialisePage()">

  <div class="container">
    <header class="d-flex flex-wrap justify-content-center py-3 mb-4 border-bottom">
      <h1 class="d-flex align-items-center mb-3 mb-md-0 me-md-auto">
        <span class="fs-4">Rideaux automatique</span>
      </h1>

      <ul class="nav nav-pills">
        <li class="nav-item"><a href="#accordionItemRideaux" class="nav-link" onclick="collapseItem('#accordionItemRideaux', 'show')">Rideaux</a></li>
        <li class="nav-item"><a href="#accordionItemVoilages" class="nav-link" onclick="collapseItem('#accordionItemVoilages', 'show')">Voilages</a></li>
        <li class="nav-item"><a href="#accordionItemModeManuel" class="nav-link" onclick="collapseItem('#accordionItemModeManuel', 'show')">Manuel</a></li>
        <li class="dropdown">
          <a class="nav-link dropdown-toggle" href="#" role="button" data-bs-toggle="dropdown" aria-expanded="false">
            Others
          </a>

          <ul class="dropdown-menu">
            <li><a class="dropdown-item" href="https://github.com/Fygadi" target="_blank">Github</a></li>
            <li><a class="dropdown-item" href="#credits" onclick="collapseItem('#creditsCollapse', 'toggle')">Crédits</a></li>
          </ul>
        </li>
      </ul>
    </header>
  </div>

  <main class="container">
    <!-- accordion -->
    <div class="accordion mb-5" id="accordionSettings">
      <!-- item 1 -->
      <form method="post" action="/Auto-Rideaux" name="rideauxForm" onsubmit="return validateFormRideaux()"
        onreset="resetFormRideaux(event)">
        <div class="accordion-item">
          <h2 class="accordion-header">
            <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse"
              data-bs-target="#accordionItemRideaux" aria-expanded="false" aria-controls="accordionItemRideaux">
              <label class="h4">Rideaux</span>
            </button>
          </h2>
          <div id="accordionItemRideaux" class="accordion-collapse collapse" data-bs-parent="#accordionSettings">
            <div class="accordion-body">
              <div class="form-check form-switch mx-3 mb-3">
                <input class="form-check-input" type="checkbox" role="switch" onchange="toggleItemRideaux()"
                  id="rideauxSwitch" name="rideauxSwitch" checked>
                <label class="form-check-label" for="rideauxSwitch">
                  On / Off
                </label>
              </div>
              <div class="card-group" id="cardGroupRideaux"> <!-- cardgroup -->
                <div class="card"> <!-- Rideaux Ouverture -->
                  <div class="card-body">
                    <h4 class="card-title text-center">Ouverture</h4>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="rideauxOpeningRadio" id="rideauxSunriseRadio"
                        required value="sun">
                      <label class="form-check-label" for="rideauxSunriseRadio">
                        Au lever du soleil à <input id="rideauxSunriseTime" type="time" value="06:15" readonly
                          onclick="this.blur()">
                      </label>
                    </div>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="rideauxOpeningRadio"
                        id="rideauxOpenAtTimeRadio" value="time">
                      <label class="form-check-label" for="rideauxOpenAtTimeRadio">
                        À
                        <input class="form-check-label" type="time" id="rideauxOpenAtTime" name="rideauxOpenAtTime"
                          value="07:00">
                      </label>
                    </div>
                  </div>
                </div>
                <div class="card"> <!-- Rideaux Fermeture -->
                  <div class="card-body">
                    <h4 class="card-title text-center">Fermeture</h4>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="rideauxClosingRadio" id="rideauxSunsetRadio"
                        value="sun">
                      <label class="form-check-label" for="rideauxSunsetRadio">
                        Au couché du soleil à <input id="rideauxSunsetTime" type="time" value="18:45" readonly
                          onclick="this.blur()">
                      </label>
                    </div>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="rideauxClosingRadio"
                        id="rideauxCloseAtTimeRadio" value="time">
                      <label class="form-check-label" for="rideauxClosingLeverTime">
                        À
                        <input class="form-check-label" type="time" id="rideauxCloseAtTime" name="rideauxCloseAtTime"
                          value="20:00">
                      </label>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <!-- Buttons -->
            <div class="hstack col-md-8 mx-auto btn-group">
              <button type="submit" class="btn btn-primary">Sauvegarder</button>
              <button type="reset" class="btn btn-secondary">Réinitialiser</button>
            </div>
          </div>
        </div>
      </form>
      <!-- item 2 -->
      <form method="post" action="/Auto-Voilages" name="voilagesForm" onsubmit="return validateFormVoilages()"
        onreset="resetFormVoilages(event)">
        <div class="accordion-item">
          <h2 class="accordion-header">
            <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse"
              data-bs-target="#accordionItemVoilages" aria-expanded="false" aria-controls="accordionItemVoilages">
              <label class="h4">Voilages</label>
            </button>
          </h2>
          <div id="accordionItemVoilages" class="accordion-collapse collapse" data-bs-parent="#accordionSettings">
            <div class="accordion-body">
              <div class="form-check form-switch mx-3 mb-3">
                <input onclick="toggleItemVoilages()" class="form-check-input" type="checkbox" role="switch"
                  name="voilagesSwitch" id="voilagesSwitch" checked>
                <input type="hidden" name="voilagesSwitch" value="off">
                <label class="form-check-label" for="voilagesSwitch">On / Off</label>
              </div>
              <div class="card-group" id="cardGroupVoilages"> <!-- Cardgroup -->
                <div class="card"> <!-- Voilages Ouverture -->
                  <div class="card-body">
                    <h4 class="card-title text-center">Ouverture</h4>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="voilagesOpeningRadio" id="voilagesSunriseRadio"
                        value="sun">
                      <label class="form-check-label" for="voilagesSunriseRadio">
                        Au lever du soleil à <input id="voilagesSunriseTime" type="time" value="00:00" readonly
                          onclick="this.blur()">
                      </label>
                    </div>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="voilagesOpeningRadio"
                        id="voilagesOpenAtTimeRadio" value="time">
                      <label class="form-check-label" for="voilagesOpenAtTimeRadio">
                        À
                        <input class="form-check-label" type="time" id="voilagesOpenAtTime" name="voilagesOpenAtTime"
                          value="00:00">
                      </label>
                    </div>
                  </div>
                </div>
                <div class="card"> <!-- voilage Fermeture -->
                  <div class="card-body">
                    <h4 class="card-title text-center">Fermeture</h4>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="voilagesClosingRadio" id="voilagesSunsetRadio"
                        value="sun">
                      <label class="form-check-label" for="voilagesSunsetRadio">
                        Au couché du soleil à <input id="voilagesSunsetTime" type="time" value="00:00" readonly>
                      </label>
                    </div>
                    <div class="form-check">
                      <input class="form-check-input" type="radio" name="voilagesClosingRadio"
                        id="voilagesCloseAtTimeRadio" value="sun">
                      <label class="form-check-label" for="voilagesCloseAtTimeRadio">
                        À
                        <input class="form-check-label" type="time" id="voilagesCloseAtTime" name="voilagesCloseAtTime"
                          value="00:00">
                      </label>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <!-- Buttons -->
            <div class="hstack col-md-8 mx-auto btn-group">
              <button type="submit" class="btn btn-primary">Sauvegarder</button>
              <button type="reset" class="btn btn-secondary">Réinitialiser</button>
            </div>
          </div>
        </div>
      </form>
      <!-- item 3 -->
      <form method="post" action="/Manuel" name="modeManuel" onsubmit="return validateFormModeManuel()">
        <div class="accordion-item">
          <h2 class="accordion-header">
            <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse"
              data-bs-target="#accordionItemModeManuel" aria-expanded="false" aria-controls="accordionItemModeManuel">
              <span class="h4">Mode manuel</span>
            </button>
          </h2>

          <div id="accordionItemModeManuel" class="accordion-collapse collapse" data-bs-parent="#accordionSettings">
            <div class="accordion-body">
              <div class="accordion" id="accordionExample">
                <div class="accordion" id="accordionPanelsStayOpenExample">
                  Date et heure du serveur: <strong><label id="serverTimeLabel">Unknown</label></strong>
                  <!-- Item 1 -->
                  <div class="accordion-item">
                    <h4 class="accordion-header">
                      <button class="accordion-button" type="button" data-bs-toggle="collapse"
                        data-bs-target="#accordionItemModeManuelRideaux" aria-expanded="true"
                        aria-controls="accordionItemModeManuelRideaux">
                        Rideaux
                      </button>
                    </h4>
                    <div id="accordionItemModeManuelRideaux" class="accordion-collapse collapse">
                      <div class="accordion-body">
                        <div class="btn-group">
                          <button class="btn btn-primary" name="modeManuelRideauxButton" value="open">Ouvrir</button>
                          <button class="btn btn-primary" name="modeManuelRideauxButton" value="close">Fermer</button>
                        </div>
                      </div>
                    </div>
                  </div>
                  <!-- Item 2 -->
                  <div class="accordion-item">
                    <h2 class="accordion-header">
                      <button class="accordion-button" type="button" data-bs-toggle="collapse"
                        data-bs-target="#accordionItemModeManuelVoilages" aria-expanded="true"
                        aria-controls="accordionItemModeManuelVoilages">
                        Voilages
                      </button>
                    </h2>
                    <div id="accordionItemModeManuelVoilages" class="accordion-collapse collapse">
                      <div class="accordion-body">
                        <div class="btn-group">
                          <button class="btn btn-primary" name="modeManuelVoilagesButton" value="open">Ouvrir</button>
                          <button class="btn btn-primary" name="modeManuelVoilagesButton" value="close">Fermer</button>
                        </div>
                      </div>
                    </div>
                  </div>
                  <div class="mt-2">
                    <label for="modeManuelMotorSpeedSlider" class="form-label"> Vitesse : <strong><span
                          id="motorSpeedLabel">unknow</span></strong></label>
                    <input name="modeManuelMotorSpeedSlider" type="range" class="form-range"
                      id="modeManuelMotorSpeedSlider" min="1" max="255" value="255" oninput="updateSliderText()">
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </form>
    </div>
  </main>

  <footer>
    <div class="container footer">
        <div class="collapse mx-3 mt-3" id="creditsCollapse">
          <h2 class="text-body-emphasis mb-2 h3"><strong class="h3">Crédits</strong></h2>
          <p class="text-body-secondary h6">
            Développement serveur par <strong class="text-body-emphasis">Pascal Benoit</strong><br>
            Développement web par <strong class="text-body-emphasis">Victor Benoit</strong> <br> <strong
              class="text-body-emphasis">©Copyright 2024 Tous droits réservés</strong></p>
        </div>
        <div class="container text-center">
          <div class="row align-items-center">
            <div class="col">
              <div class="container">
                <nav class="navbar">
                  <button class="navbar-toggler" type="button" data-bs-toggle="collapse"
                    data-bs-target="#creditsCollapse"
                    aria-controls="creditsCollapse" aria-expanded="false"
                    aria-label="Toggle navigation">
                    <span class="navbar-toggler-icon"></span>
                  </button>
                </nav>
              </div>
            </div>
            <div class="col">
              <a class="nav-link" href="https://github.com/Fygadi" target="_blank">
                <!-- github svg -->
                <svg width="40px" height="40px" viewBox="0 0 20 20" version="1.1" xmlns="http://www.w3.org/2000/svg"
                  xmlns:xlink="http://www.w3.org/1999/xlink">
                  <title>github [#142]</title>
                  <desc>Created with Sketch.</desc>
                  <g id="Page-1" stroke="none" stroke-width="1" fill="none" fill-rule="evenodd">
                    <g id="Dribbble-Light-Preview" transform="translate(-140.000000, -7559.000000)" fill="#000000">
                      <g id="icons" transform="translate(56.000000, 160.000000)">
                        <path
                          d="M94,7399 C99.523,7399 104,7403.59 104,7409.253 C104,7413.782 101.138,7417.624 97.167,7418.981 C96.66,7419.082 96.48,7418.762 96.48,7418.489 C96.48,7418.151 96.492,7417.047 96.492,7415.675 C96.492,7414.719 96.172,7414.095 95.813,7413.777 C98.04,7413.523 100.38,7412.656 100.38,7408.718 C100.38,7407.598 99.992,7406.684 99.35,7405.966 C99.454,7405.707 99.797,7404.664 99.252,7403.252 C99.252,7403.252 98.414,7402.977 96.505,7404.303 C95.706,7404.076 94.85,7403.962 94,7403.958 C93.15,7403.962 92.295,7404.076 91.497,7404.303 C89.586,7402.977 88.746,7403.252 88.746,7403.252 C88.203,7404.664 88.546,7405.707 88.649,7405.966 C88.01,7406.684 87.619,7407.598 87.619,7408.718 C87.619,7412.646 89.954,7413.526 92.175,7413.785 C91.889,7414.041 91.63,7414.493 91.54,7415.156 C90.97,7415.418 89.522,7415.871 88.63,7414.304 C88.63,7414.304 88.101,7413.319 87.097,7413.247 C87.097,7413.247 86.122,7413.234 87.029,7413.87 C87.029,7413.87 87.684,7414.185 88.139,7415.37 C88.139,7415.37 88.726,7417.2 91.508,7416.58 C91.513,7417.437 91.522,7418.245 91.522,7418.489 C91.522,7418.76 91.338,7419.077 90.839,7418.982 C86.865,7417.627 84,7413.783 84,7409.253 C84,7403.59 88.478,7399 94,7399"
                          id="github-[#142]">
                        </path>
                      </g>
                    </g>
                  </g>
                </svg>
              </a>
            </div>
          </div>
        </div>
    </div>
  </footer>

    <!-- Bootstrap JavaScript Libraries -->
    <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.11.8/dist/umd/popper.min.js"
      integrity="sha384-I7E8VVD/ismYTF4hNIPjVp/Zjvgyol6VFvRkX/vR+Vc4jQkC+hVqc2pM8ODewa9r"
      crossorigin="anonymous"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.min.js"
      integrity="sha384-BBtl+eGJRgqQAUMxJ7pMwbEyER4l1g+O15P+16Ep7Q9Q+zqX6gSbd85u4mG4QzX+"
      crossorigin="anonymous"></script>
  </body>
</html>
)""";
#endif


/**
 * @brief Generates the HTTP header for an HTML page.
 * 
 * This function sends the HTTP header to the client, indicating a successful response (200 OK).
 * It includes various headers such as cache control, content type, and connection settings.
 * 
 * @param client The WiFiClient object representing the client connection.
 */
void generateHtmlHeaderPage(WiFiClient client) {
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Vary: Origin\r\n");
  client.print("Access-Control-Allow-Credentials: true\r\n");
  client.print("Accept-Ranges: bytes\r\n");
  client.print("Cache-Control: public, max-age=0\r\n");
  client.print("Last-Modified: Tue, 30 Jul 2024 21:05:15 GMT\r\n");
  client.print("ETag: W\"2363-19105762aa3\"\r\n");
  client.print("Content-Type: text/html; charset=UTF-8\r\n");
  client.print("Date: Tue, 30 Jul 2024 21:46:23 GMT\r\n");
  client.print("Connection: keep-alive\r\n");
  client.print("Keep-Alive: timeout=5\r\n");
  client.print("\r\n");
}

/**
 * @brief Generates the HTML page displaying user data.
 * 
 * This function generates and sends an HTML page to the client, displaying various user data
 * such as motor speed, server time, sunrise and sunset times, and the status of the rideaux and voilages.
 * 
 * @param client The WiFiClient object representing the client connection.
 * @param config The configuration object containing user settings.
 * @param getTimeStr A function pointer that returns the current server time as a string.
 * @param getSunrise A function pointer that returns the sunrise time in minutes since midnight.
 * @param getSunset A function pointer that returns the sunset time in minutes since midnight.
 */
void generateUserDataHtmlPage(WiFiClient client, class Config &config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)()) {

  client.println("<script>");
  client.println("      let settings = {");

  // Display the user data in web page
  client.println((String) "motorSpeed: '" + config.m_manualSpeed + "',");
  client.println((String) "serverTime: '" + getTimeStr() + "',");

  client.print("sunriseTime: '");
  int timeSoleil = getSunrise();
  std::string preformattedPage = (((timeSoleil / 60 < 10) ? "0" : "") + std::to_string(timeSoleil / 60) + ":" + ((timeSoleil % 60 < 10) ? "0" : "") + std::to_string(timeSoleil % 60));
  client.print(preformattedPage.data());
  client.print("',\n");

  client.print("sunsetTime: '");
  timeSoleil = getSunset();
  preformattedPage = (((timeSoleil / 60 < 10) ? "0" : "") + std::to_string(timeSoleil / 60) + ":" + ((timeSoleil % 60 < 10) ? "0" : "") + std::to_string(timeSoleil % 60));
  client.print(preformattedPage.data());
  client.print("',\n");

  client.print("\n");

  for (int idx = 0; idx < 2; idx++) {
    if (idx == 0) {
      client.print("rideaux: {\n");

      if (config.curtain[idx].isEnabled)
        client.print("rideauxSwitchOn: true,\n");
      else
        client.print("rideauxSwitchOn: false,\n");
    } else {
      client.print("voilages: {\n");
      if (config.curtain[idx].isEnabled)
        client.print("voilagesSwitchOn: true,\n");
      else
        client.print("voilagesSwitchOn: false,\n");
    }

    client.print("opening: {\n");

    if (config.curtain[idx].isOpenAtSunrise == true)
      client.print("isSunrise: true,\n");
    else
      client.print("isSunrise: false,\n");

    client.print("// sunriseTime,\n");
    client.print("\n");

    if (config.curtain[idx].isOpenAtTime == true)
      client.print("isAtTime: true,\n");
    else
      client.print("isAtTime: false,\n");

    int actionAtTime = config.curtain[idx].openAtTime;
    client.print("atTime: '");
    preformattedPage = (((actionAtTime / 60 < 10) ? "0" : "") + std::to_string(actionAtTime / 60) + ":" + ((actionAtTime % 60 < 10) ? "0" : "") + std::to_string(actionAtTime % 60));
    client.print(preformattedPage.data());
    client.print("'\n");

    client.print("},\n");
    client.print("closing: {\n");
    if (config.curtain[idx].isCloseAtSunset == true)
      client.print("isSunset: true,\n");
    else
      client.print("isSunset: false,\n");

    client.print("// sunsetTime,\n");
    client.print("\n");
    if (config.curtain[idx].isCloseAtTime == true)
      client.print("isAtTime: true,\n");
    else
      client.print("isAtTime: false,\n");

    actionAtTime = config.curtain[idx].closeAtTime;
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
  client.print("</script>\n");
}

/**
 * @brief Handles the root URL request and generates the main HTML page.
 * 
 * This function is called when the root URL ("/") is requested. It generates the main HTML page
 * by sending the header, user data, and footer sections to the client. The user data includes
 * various settings and statuses retrieved from the provided configuration and callback functions.
 * 
 * @param config The configuration object containing user settings.
 * @param getTimeStr A function pointer that returns the current server time as a string.
 * @param getSunrise A function pointer that returns the sunrise time in minutes since midnight.
 * @param getSunset A function pointer that returns the sunset time in minutes since midnight.
 */
void handleRoot(class Config &config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)()) {

  generateHtmlHeaderPage(client);

  // Display first section of the web page
  const size_t sectionSize1 = sizeof(MAIN_page_part1);
  for (unsigned int i = 0; i < (sectionSize1 - 1); i += 255) {
    client.write(&(MAIN_page_part1[i]), (sectionSize1 - i - 1) < 255 ? (sectionSize1 - i - 1) : 255);
    // it take to long to send the web page and the motor do not check to rope tention during this time, avoid this by call it periodicly.
    curtainPeriodic();
  }

  // Display the user data in the web page
  generateUserDataHtmlPage(client, config, getTimeStr, getSunrise, getSunset);

  // Display second section of the web page
  const size_t sectionSize2 = sizeof(MAIN_page_part2);
  for (unsigned int i = 0; i < (sectionSize2 - 1); i += 255) {
    client.write(&(MAIN_page_part2[i]), (sectionSize2 - i - 1) < 255 ? (sectionSize2 - i - 1) : 255);
    // it take to long to send the web page and the motor do not check to rope tention during this time, avoid this by call it periodicly.
    curtainPeriodic();
  }

  client.println("");
  client.println("");

  // The HTTP response ends with another blank line:
  client.println();
}


/**
 * @brief Extracts the value associated with a given key from a query string.
 * 
 * This function searches for a specified key within an input query string and extracts the corresponding value.
 * If the key is found, the value is stored in the provided reference and the function returns true.
 * If the key is not found, the function returns false.
 * 
 * @param input The input query string to search within.
 * @param key The key whose associated value is to be extracted.
 * @param value A reference to a string where the extracted value will be stored.
 * @return true if the key is found and the value is extracted successfully, false otherwise.
 */
bool getKeyValue(const std::string &input, const std::string key, std::string &value) {
  size_t startPos = input.find(key);
  if (startPos == std::string::npos) {
    // Key not found
    return false;
  }

  startPos += key.length();
  size_t endPos = input.find('&', startPos);

  if (endPos == std::string::npos) {
    // If no ampersand is found, take the rest of the string
    endPos = input.length();
  }

  value = input.substr(startPos, endPos - startPos);
  return true;
}


/**
 * @brief Handles manual control post requests.
 * 
 * This function processes the post data received from a manual control form. It extracts the values
 * for voilages and rideaux control buttons, as well as the motor speed slider. Based on the extracted
 * values, it performs the corresponding actions to open or close the voilages and rideaux.
 * 
 * @param postData The post data received from the manual control form.
 * @param config The configuration object containing user settings, including the manual motor speed.
 */
void handleManualPost(std::string postData, Config &config) {
  bool voilagesOpen = false;
  bool voilagesClose = false;
  bool rideauxOpen = false;
  bool rideauxClose = false;

  std::string value;
  if (getKeyValue(postData, "modeManuelVoilagesButton=", value)) {
    voilagesOpen = (value == "open");
    voilagesClose = (value == "close");
    Serial.print("== Received: modeManuelVoilagesButton");
    Serial.println(voilagesOpen);
    Serial.println(voilagesClose);
  }

  if (getKeyValue(postData, "modeManuelRideauxButton=", value)) {
    rideauxOpen = (value == "open");
    rideauxClose = (value == "close");
    Serial.print("== Received: modeManuelRideauxButton");
    Serial.println(rideauxOpen);
    Serial.println(rideauxClose);
  }

  if (getKeyValue(postData, "modeManuelMotorSpeedSlider=", value)) {
    config.m_manualSpeed = std::stoi(value);
    Serial.print("== Received Speed: ");
    Serial.println(config.m_manualSpeed);
  }

  Serial.println("== Motor Action");
  // Perform actions for voilages
  if (voilagesOpen) {
    Serial.println("voilagesOpen");
    openManualCurtain(1);
  } else if (voilagesClose) {
    Serial.println("voilagesClose");
    closeManualCurtain(1);
  }

  // Perform actions for curtain
  if (rideauxOpen) {
    Serial.println("rideauxOpen");
    openManualCurtain(0);
  } else if (rideauxClose) {
    Serial.println("rideauxClose");
    closeManualCurtain(0);
  }

  config.writeCfg2Epprom();
}



/**
 * @brief Handles the POST request for automatic rideaux control settings.
 * 
 * This function processes the POST data received from the automatic rideaux control form.
 * It updates the configuration settings for the specified rideaux or voilage based on the form data.
 * The settings include enabling/disabling the rideaux, setting the opening and closing times,
 * and determining whether to open/close at sunrise/sunset or at a specific time.
 * 
 * @param index The index of the rideaux (0 for rideaux, 1 for voilage).
 * @param postData The POST data received from the form.
 * @param config The configuration object containing user settings.
 */
void handleAutoCurtainsPost(int index, std::string postData, Config &config) {
  std::string prefix = (index == 0) ? "rideaux" : "voilage";

  // Default value if not present
  config.curtain[index].isEnabled = false;

  std::string value;
  if (getKeyValue(postData, prefix + "Switch=", value))
    config.curtain[index].isEnabled = (value == "on");

  if (getKeyValue(postData, prefix + "OpeningRadio=", value)) {
    bool isOpenAtSunrise = (value == "sun");
    config.curtain[index].isOpenAtSunrise = isOpenAtSunrise;
    config.curtain[index].isOpenAtTime = !isOpenAtSunrise;
  }

  if (getKeyValue(postData, prefix + "OpenAtTime=", value)) {
    config.curtain[index].openAtTime = std::stoi(value.substr(0, 2)) * 60 + std::stoi(value.substr(5, 2));
  }

  if (getKeyValue(postData, prefix + "ClosingRadio=", value)) {
    bool isCloseAtSunset = (value == "sun");
    config.curtain[index].isCloseAtSunset = isCloseAtSunset;
    config.curtain[index].isCloseAtTime = !isCloseAtSunset;
  }

  if (getKeyValue(postData, prefix + "CloseAtTime=", value))
    config.curtain[index].closeAtTime = std::stoi(value.substr(0, 2)) * 60 + std::stoi(value.substr(5, 2));

  // Update configuration value with the EEPROM values.
  config.writeCfg2Epprom();


  // Apply the action
  openAutoCurtain(index);
  closeAutoCurtain(index);
}


///////////////////////
///                 ///
///  Server Section ///
///                 ///
///////////////////////

/**
 * @brief Extracts the content length from an HTTP header.
 * 
 * This function searches for the "Content-Length" key in the provided HTTP header string
 * and extracts the corresponding value. If the key is found, it returns the content length as an integer.
 * If the key is not found, it returns -1.
 * 
 * @param input The HTTP header string to search within.
 * @return The content length as an integer if the key is found, -1 otherwise.
 */
int getContentLength(const std::string &input) {
  const std::string key = "Content-Length: ";
  size_t startPos = input.find(key);

  if (startPos == std::string::npos) {
    // Key not found
    return -1;
  }

  startPos += key.length();
  size_t endPos = input.find('\n', startPos);

  if (endPos == std::string::npos) {
    // If no newline is found, take the rest of the string
    endPos = input.length();
  }

  std::string valueStr = input.substr(startPos, endPos - startPos);
  return std::stoi(valueStr);
}



/**
 * @brief Handles incoming HTTP requests and routes them to the appropriate handlers.
 * 
 * This function processes incoming HTTP requests from a client, determines the type of request (GET or POST),
 * and routes it to the appropriate handler function. It reads the request headers and body, if present,
 * and handles different routes such as the root page, page1, page2, Auto-Rideaux, Auto-Voilages, and Manuel.
 * 
 * @param config The configuration object containing user settings.
 * @param getTimeStr A function pointer that returns the current server time as a string.
 * @param getSunrise A function pointer that returns the sunrise time in minutes since midnight.
 * @param getSunset A function pointer that returns the sunset time in minutes since midnight.
 */
void htmlRun(class Config &config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)()) {
  int contentLength = 0;
  int contentRcv = 0;
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    String postData = "";
    bool isBody = false;
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (isBody) {
          // End of headers, start reading body
          contentRcv++;
          postData += c;

          if (contentRcv >= contentLength) {
            // Check if the body has been completely received
            break;
          }
        } else {
          // is header
          request += c;

          if (request.endsWith("\r\n\r\n")) {

            if (request.startsWith("GET")) {
              // GET request has no body
              break;
            } else if (request.startsWith("POST")) {
              contentLength = getContentLength(request.c_str());
              isBody = true;
            }
          }
        }
      }
    }

    // Process all WEB pages
    if (request.startsWith("GET / ")) {
      Serial.println("Get ROOT page");
      handleRoot(config, getTimeStr, getSunrise, getSunset);

    } else if (request.startsWith("GET /page1 ")) {
      Serial.println("GET /page1");
      handleRoot(config, getTimeStr, getSunrise, getSunset);

    } else if (request.startsWith("POST /Auto-Rideaux ")) {
      Serial.println("POST /Auto-Rideaux");
      handleAutoCurtainsPost(0, postData.c_str(), config);
      handleRoot(config, getTimeStr, getSunrise, getSunset);

    } else if (request.startsWith("POST /Auto-Voilages ")) {
      Serial.println("POST /Auto-Voilages");
      handleAutoCurtainsPost(1, postData.c_str(), config);
      handleRoot(config, getTimeStr, getSunrise, getSunset);

    } else if (request.startsWith("POST /Manuel ")) {
      Serial.println("POST /Manuel");
      handleManualPost(postData.c_str(), config);
      handleRoot(config, getTimeStr, getSunrise, getSunset);

    } else {
      Serial.println("Get else");
      Serial.println(request);
      handleRoot(config, getTimeStr, getSunrise, getSunset);
    }

    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}
