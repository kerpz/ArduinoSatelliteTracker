#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Sgp4.h>

#include "tracker.h"

float sat_latitude;
float sat_longitude;
float sat_altitude;
float sat_azimuth;
float sat_elevation;
float sat_distance;

String catalog_number = "25544";

uint16_t get_error = 0;
int http_code = 0;

Sgp4 sat;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  data.replace("\r", "");
  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void getTLE(String catalog_number)
{
  // WiFiClient client; // http
  WiFiClientSecure client; // https
  HTTPClient http;

  if (WiFi.status() == WL_CONNECTED)
  {
    // https insecure
    client.setInsecure();
    // if (http_timeout > 0) http.setTimeout(http_timeout); // ms
    Serial.print("Getting TLE ... ");
    if (http.begin(client, "https://celestrak.org/NORAD/elements/gp.php?CATNR=" + catalog_number))
    {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        Serial.println("success");

        String line1 = getValue(payload, '\n', 0);
        String line2 = getValue(payload, '\n', 1);
        String line3 = getValue(payload, '\n', 2);

        line1.toCharArray(tle_name, 32);
        line2.toCharArray(tle_line1, 70);
        line3.toCharArray(tle_line2, 70);

        sat.init(tle_name, tle_line1, tle_line2); // initialize satellite parameters
      }
      else
      {
        Serial.println("failed");
        // Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        if (get_error < 65500)
          get_error++;
      }
      http.end();
      http_code = httpCode;
    }
  }
}

void trackerSetup()
{
  Serial.print("Loading TLE ... ");
  sat.site(latitude, longitude, altitude);  // set site latitude[°], longitude[°] and altitude[m]
  sat.init(tle_name, tle_line1, tle_line2); // initialize satellite parameters
  Serial.println("done");
  // Serial.println("latitude = " + String(latitude, 7) + " longitude = " + String(longitude, 7) + " altitude = " + String(altitude, 2));
  // Serial.println("sat_name = " + String(sat_name));
  // Serial.println("tle_line1 = " + String(tle_line1));
  // Serial.println("tle_line2 = " + String(tle_line2));
  // sat.findsat(timeNow);
  // sat.findsat(unixtime);
  // invjday(sat.satJd, timeZone, true, year, mon, day, hr, minute, sec);
  // Serial.println("\nLocal time: " + String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  // Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  // Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  // Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
}

void trackerLoop()
{
  // int timezone = 8 ;  //utc + 8
  // int _year;
  // int _month;
  // int _day;
  // int _hour;
  // int _minute;
  // double _second;

  sat.findsat((unsigned long)epoch);
  // invjday(sat.satJd, timezone, true, _year, _month, _day, _hour, _minute, _second);
  // Serial.println(String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  // Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  // Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  // Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
  sat_latitude = sat.satLat;
  sat_longitude = sat.satLon;
  sat_altitude = sat.satAlt;
  sat_azimuth = sat.satAz;
  sat_elevation = sat.satEl;
  sat_distance = sat.satDist;

  // second = _second;
  // minute = _minute;
  // hour = _hour;
  // day = _day;
  // month = _month;
  // year = _year;

  if (auto_control)
  {
    setAz(round(sat_azimuth));   // 0 - 360
    setEl(round(sat_elevation)); // 0 - 90
  }
}
