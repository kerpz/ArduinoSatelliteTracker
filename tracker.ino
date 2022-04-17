#define SERVER_NAME "http://www.celestrak.com/satcat/tle.php?CATNR=32382"

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
// openssl s_client -connect api.thingspeak.com:443 | openssl x509 -fingerprint -noout
//const uint8_t fingerprint[20] = {0x27, 0x18, 0x92, 0xDD, 0xA4, 0x26, 0xC3, 0x07, 0x09, 0xB9, 0x7A, 0xE6, 0xC5, 0x21, 0xB9, 0x5B, 0x48, 0xF7, 0x16, 0xE1};
#include <Sgp4.h>

Sgp4 sat;

char satname[] = "ISS (ZARYA)";
char tle_line1[] = "1 25544U 98067A   22105.52091101  .00008356  00000-0  15383-3 0  9994";  //Line one from the TLE data
char tle_line2[] = "2 25544  51.6432 286.9653 0004720  29.1347  36.8252 15.50072235335468";  //Line two from the TLE data

void trackerSetup() {
  sat.site(latitude, longitude, altitude); //set site latitude[°], longitude[°] and altitude[m]
  sat.init(satname, tle_line1, tle_line2); //initialize satellite parameters 
  //sat.findsat(timeNow);
  //sat.findsat(unixtime);
  //invjday(sat.satJd, timeZone, true, year, mon, day, hr, minute, sec);
  //Serial.println("\nLocal time: " + String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  //Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  //Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  //Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
}

void trackerLoop() {
  int timezone = 8 ;  //utc + 8
  int  year; int mon; int day; int hr; int minute; double sec;
  
  sat.findsat(epochTime); // epochTime
  invjday(sat.satJd, timezone, true, year, mon, day, hr, minute, sec);
  Serial.println(String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  //Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
}

void getTLE () {
  
  WiFiClient client; // http
  HTTPClient http;

  if (WiFi.status() == WL_CONNECTED) {
    // https insecure
    //client.setFingerprint(fingerprint);
    //client.setInsecure();
    //if (http_timeout > 0) http.setTimeout(http_timeout); // ms
    if (http.begin(client, SERVER_NAME)) {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
      http.end();
    }
  }
}
