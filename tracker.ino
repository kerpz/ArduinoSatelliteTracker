#define SERVER_NAME "http://www.celestrak.com/satcat/tle.php?CATNR=25544"

Sgp4 sat;

void trackerSetup() {
  Serial.println("latitude = " + String(latitude, 7) + " longitude = " + String(longitude, 7) + " altitude = " + String(altitude, 2));
  sat.site(latitude, longitude, altitude); //set site latitude[°], longitude[°] and altitude[m]
  Serial.println("sat_name = " + String(sat_name));
  Serial.println("tle_line1 = " + String(tle_line1));
  Serial.println("tle_line2 = " + String(tle_line2));
  sat.init(sat_name, tle_line1, tle_line2); //initialize satellite parameters 
  //sat.findsat(timeNow);
  //sat.findsat(unixtime);
  //invjday(sat.satJd, timeZone, true, year, mon, day, hr, minute, sec);
  //Serial.println("\nLocal time: " + String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  //Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  //Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  //Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
}

void trackerLoop() {
  //int timezone = 8 ;  //utc + 8
  //int _year;
  //int _month;
  //int _day;
  //int _hour;
  //int _minute;
  //double _second;
  
  sat.findsat(epoch);
  //invjday(sat.satJd, timezone, true, _year, _month, _day, _hour, _minute, _second);
  //Serial.println(String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  //Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  //Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  //Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
  sat_latitude = sat.satLat;
  sat_longitude = sat.satLon;
  sat_altitude = sat.satAlt;
  sat_azimuth = sat.satAz;
  sat_elevation = sat.satEl;
  sat_distance = sat.satDist;

  //second = _second;
  //minute = _minute;
  //hour = _hour;
  //day = _day;
  //month = _month;
  //year = _year;
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
