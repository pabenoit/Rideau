#include <stdio.h>

#include <Arduino_JSON.h>
#include <TimeLib.h>
#include <Wifi.h>

#include "utility.h"

time_t int2time_t(int in)
{
  struct tm tmTime;
  tmTime.tm_sec = 0;
  tmTime.tm_min = in % 60;
  tmTime.tm_hour = in / 60;
  tmTime.tm_mday = day();
  tmTime.tm_mon = month() - 1;
  tmTime.tm_year = year() - 1900;
  tmTime.tm_isdst = false;

  time_t out = mktime(&tmTime);

  // If the computed time is in the past, add one day
  if (out < now())
  {
    tmTime.tm_mday += 1;
    out = mktime(&tmTime);
  }

  return out;
}

int parseTimeToMinutes(String timeStr)
{
  int hour = timeStr.substring(11, 13).toInt();
  int minute = timeStr.substring(14, 16).toInt();
  return hour * 60 + minute;
}

#if 0
bool getSunriseSunset(double longitude,
                      double latitude,
                      int year,
                      int month,
                      int day,
                      int timeZoneOffset,
                      int &sunriseTime,
                      int &sunsetTime)
{
  sun.setPosition(latitude, longitude, timeZoneOffset);
  sun.setTZOffset(timeZoneOffset);
  sun.setCurrentDate(year, month, day);
  sunsetTime = static_cast<int>(sun.calcSunset());
  sunriseTime = static_cast<int>(sun.calcSunrise());
return true;
}

/*
 * Calculates sunrise and sunset times based on current time. All times are in Unix Timestamp format.
 */
bool getSunriseSunset(double longitude,
                      double latitude,
                      int year,
                      int month,
                      int day,
                      int timeZoneOffset,
                      int &sunriseTime,
                      int &sunsetTime)
{
  // Constants
//  const double PI = 3.14159265358979323846;
  const double ZENITH = -0.83; // Official zenith for sunrise/sunset

  // Convert the date to Julian Day
  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;
  double JDN = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
  double JD = JDN + (0.0 - 12.0) / 24.0; // Julian Day with time

  // Calculate the number of days since J2000.0
  double n = JD - 2451545.0 + 0.0008;

  // Mean solar noon
  double Jstar = n - (longitude / 360.0);

  // Solar mean anomaly
  double M = fmod((357.5291 + 0.98560028 * Jstar), 360.0);

  // Equation of the center
  double C = 1.9148 * sin(M * PI / 180.0) + 0.02 * sin(2 * M * PI / 180.0) + 0.0003 * sin(3 * M * PI / 180.0);

  // Ecliptic longitude
  double lambda = fmod((M + C + 180 + 102.9372), 360.0);

  // Solar transit
  double Jtransit = Jstar + (0.0053 * sin(M * PI / 180.0)) - (0.0069 * sin(2 * lambda * PI / 180.0));

  // Declination of the Sun
  double delta = asin(sin(lambda * PI / 180.0) * sin(23.44 * PI / 180.0)) * 180.0 / PI;

  // Hour angle
  double omega = acos((sin(ZENITH * PI / 180.0) - sin(latitude * PI / 180.0) * sin(delta * PI / 180.0)) / (cos(latitude * PI / 180.0) * cos(delta * PI / 180.0))) * 180.0 / PI;

  // Julian day for sunrise and sunset
  double Jrise = Jtransit - (omega / 360.0);
  double Jset = Jtransit + (omega / 360.0);

  // Convert Julian day to Unix Timestamp
  unsigned long unixRise = (Jrise - 2440587.5) * 86400.0;
  unsigned long unixSet = (Jset - 2440587.5) * 86400.0;

  // Convert Unix Timestamp to hours, minutes, and seconds
  sunriseTime = (unixRise % 86400) + timeZoneOffset * 3600;
  sunsetTime = (unixSet % 86400) + timeZoneOffset * 3600;

  // Adjust for negative times
  if (sunriseTime < 0) {
    sunriseTime += 86400;
  }
  if (sunsetTime < 0) {
    sunsetTime += 86400;
  }

  return true;
}
#else

bool getSunriseSunset(double longitude,
                      double latitude,
                      int year,
                      int month,
                      int day,
                      int timeZoneOffset,
                      int &sunriseTime,
                      int &sunsetTime)
{
  WiFiClient client;
  unsigned long startTime = millis();

  if (client.connect("api.sunrise-sunset.org", 80))
  {
    Serial.println("Connected to server");

    String request = "GET /json?lat=" + String(latitude, 6) + "&lng=" + String(longitude, 6) + "&date=" + String(year) +
                     "-" + String(month) + "-" + String(day) + "&formatted=0" +  " HTTP/1.1";

    Serial.println(request.c_str());
    // Make an HTTP request:
    client.println(request.c_str());
    client.println("Host: api.sunrise-sunset.org");
    client.println("Connection: close");
    client.println();

    // Wait for response with timeout protection
    while (!client.available())
    {
      if (millis() - startTime > 5000)
      {
        Serial.println("Error: Timeout waiting for response");
        client.stop();
        return false;
      }
      delay(100);
    }

    // Read response into a String
    String response = client.readString();

    // Find the start of the JSON payload
    int headerEndIndex = response.indexOf("\r\n\r\n") + 4;
    int chunkSizeEndIndex = response.indexOf("\r\n", headerEndIndex);
    int jsonStartIndex = chunkSizeEndIndex + 2;
    int jsonEndIndex = response.indexOf("\r\n", jsonStartIndex);
    int checksumStartIndex = jsonEndIndex + 2;
    int checksumEndIndex = response.indexOf("\r\n\r\n", checksumStartIndex);

    String chunkSize = response.substring(headerEndIndex, chunkSizeEndIndex);
    String jsonResponse = response.substring(jsonStartIndex, jsonEndIndex);
    String checksum = response.substring(checksumStartIndex, checksumEndIndex);

    Serial.println("+++++++++++++++++++");
    Serial.println("Chunk Size: " + chunkSize);
    Serial.println("JSON Response: " + jsonResponse);
    Serial.println("Checksum: " + checksum);

    String lSunrise = "Vide";
    String lSunset = "Vide";

    JSONVar json = JSON.parse(jsonResponse);
    if (JSON.typeof(json) == "undefined")
    {
      Serial.println("Error: Parsing JSON");
      return false;
    }

    if (json.hasOwnProperty("results"))
    {
      JSONVar results = json["results"];

      if (results.hasOwnProperty("sunrise") && results.hasOwnProperty("sunset"))
      {
        String sunriseStr = (const char *)results["sunrise"];
        String sunsetStr = (const char *)results["sunset"];

        // Convert sunrise and sunset times to seconds since midnight
        sunriseTime = parseTimeToMinutes(sunriseStr) + timeZoneOffset * 60;
        sunsetTime = parseTimeToMinutes(sunsetStr) + timeZoneOffset * 60;

        if (sunriseTime < 0)
        {
          sunriseTime += (60*24);
        }

        if (sunsetTime < 0)
        {
          sunsetTime += (60*24);
        } 

        // Print results
        Serial.print("Sunrise: ");
        Serial.print(sunriseTime);
        Serial.println(" minutes");
        Serial.print("Sunset : ");
        Serial.print(sunsetTime);
        Serial.println(" minutes");

        return true;
      }
      else
      {
        Serial.println("Error: JSON does not have 'sunrise || sunset' property");
        return false;
      }
    }
    else
    {
      Serial.println("Error: JSON does not have 'results' property");
      return false;
    }
  }
  else
  {
    Serial.println("Connection to server failed");
    return false;
  }

  // Disconnect from server
  if (client.connected())
  {
    client.stop();
    Serial.println("Disconnected from server");
  }

  return false;
}
#endif