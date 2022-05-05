ESP8266WebServer webServer(80);

int getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

void webserverSetup() {
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", index_html);
  });
  webServer.on("/system", HTTP_POST, []() {
    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["reboot"]) ESP.restart();
    }

    String json;
    
    json += "[";
    json += "{\"type\":\"title\",\"value\":\"System\"},";
    json += "{\"type\":\"table\",\"name\":\"system\",\"data\":[";
    json += "[\"Chip ID\",\""+String(ESP.getChipId())+"\"],";
    json += "[\"Free Heap\",\""+String(ESP.getFreeHeap())+"\"],";
    json += "[\"Flash ID\",\""+String(ESP.getFlashChipId())+"\"],";
    json += "[\"Flash Size\",\""+String(ESP.getFlashChipSize())+"\"],";
    json += "[\"Flash Real Size\",\""+String(ESP.getFlashChipRealSize())+"\"]";
    json += "]},";
    json += "{\"type\":\"title\",\"value\":\"Wifi AP\"},";
    json += "{\"type\":\"table\",\"name\":\"wifiap\",\"data\":[";
    json += "[\"Hardware AP\",\""+WiFi.softAPmacAddress()+"\"],";
    json += "[\"Address AP\",\""+WiFi.softAPIP().toString()+"\"],";
    json += "[\"SSID AP\",\""+String(APssid)+"\"],";
    json += "[\"Connected AP\",\""+String(WiFi.softAPgetStationNum())+"\"]";
    json += "]},";
    json += "{\"type\":\"title\",\"value\":\"Wifi Station\"},";
    json += "{\"type\":\"table\",\"name\":\"wifista\",\"data\":[";
    json += "[\"Hardware\",\""+WiFi.macAddress()+"\"],";
    json += "[\"Address\",\""+WiFi.localIP().toString()+"\"],";
    json += "[\"SSID\",\""+WiFi.SSID()+"\"]";
    json += "]},";
    json += "{\"type\":\"button\",\"label\":\"REBOOT\",\"name\":\"reboot\",\"value\":\"reboot\",\"confirm\":\"Are you sure you want to reboot?\"}";
    json += "]";
    
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/scan", HTTP_GET, []() {
    String json = "";

    json += "[";
    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
    } else if(n) {
      for (int i = 0; i < n; ++i) {
        if (i) json += ",";
        json += "{";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += "\"signal\":" + String(getRSSIasQuality(WiFi.RSSI(i)));
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
        json += ",\"channel\":" + String(WiFi.channel(i));
        json += ",\"secure\":" + String(WiFi.encryptionType(i));
        json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
        json += "}";
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) {
        WiFi.scanNetworks(true);
      }
    }
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/config", HTTP_POST, []() {
    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["ssid"]) strcpy(ssid, (const char*)doc["ssid"]);
      if (doc["password"]) strcpy(password, (const char*)doc["password"]);

      if (doc["mpu9250_enable"]) mpu9250_enable = doc["mpu9250_enable"];
      if (doc["display_enable"]) display_enable = doc["display_enable"];
      if (doc["beep_enable"]) beep_enable = doc["beep_enable"];
  
      if (doc["latitude"]) latitude = doc["latitude"];
      if (doc["longitude"]) longitude = doc["longitude"];
      if (doc["altitude"]) altitude = doc["altitude"];
      if (doc["declination"]) declination = doc["declination"];

      saveConfig();
      Serial.println("Save config");
    }

    String json;
    String sep;
    int n = WiFi.scanNetworks();

    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Wifi\"},";
    json += "{\"type\":\"select\",\"label\":\"SSID\",\"name\":\"ssid\",\"value\":\"" + WiFi.SSID() + "\",\"options\":[";
    if (n > 0) {
      for (int i = 0; i < n; i++) {
        json += sep+"[\""+WiFi.SSID(i)+"\",\""+WiFi.SSID(i)+" "+getRSSIasQuality(WiFi.RSSI(i))+"%";
        if (WiFi.encryptionType(i) != ENC_TYPE_NONE) {
          json += " &#9906";
        }
        json += "\"]";
        sep = ",";
      }
    } else {
      json += "[\"\",\"\"]";
    }
    json += "]},";
    json += "{\"type\":\"text\",\"label\":\"Password\",\"name\":\"password\",\"value\":\"" + String(password) + "\"},";

    json += "{\"type\":\"title\",\"value\":\"Components\"},";
    json += "{\"type\":\"select\",\"label\":\"MPU9250\",\"name\":\"mpu9250_enable\",\"value\":\"" + String(mpu9250_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"Display\",\"name\":\"display_enable\",\"value\":\"" + String(display_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"Beep\",\"name\":\"beep_enable\",\"value\":\"" + String(beep_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";

    json += "{\"type\":\"title\",\"value\":\"Tracker\"},";
    json += "{\"type\":\"text\",\"label\":\"Latitude\",\"name\":\"latitude\",\"value\":\"" + String(latitude, 7) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Latitude\",\"name\":\"longitude\",\"value\":\"" + String(longitude, 7) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Altitude\",\"name\":\"altitude\",\"value\":\"" + String(altitude) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Declination\",\"name\":\"declination\",\"value\":\"" + String(declination) + "\"},";

    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/tracker", HTTP_POST, []() {
    byte refresh = 2;

    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["refresh"]) refresh = atoi(doc["refresh"]);
    }

    String json = "";

    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Satellite\"},";
    json += "{\"type\":\"table\",\"name\":\"satellite\",\"data\":[";
    json += "[\"Name\",\""+String(sat_name)+"\"],";
    json += "[\"Epoch\",\""+String(epoch)+"\"],";
    json += "[\"Latitude\",\""+String(sat_latitude, 7)+"\"],";
    json += "[\"Longitude\",\""+String(sat_longitude, 7)+"\"],";
    json += "[\"Altitude\",\""+String(sat_altitude)+"\"],";
    json += "[\"Azimuth\",\""+String(sat_azimuth)+"\"],";
    json += "[\"Elevation\",\""+String(sat_elevation)+"\"],";
    json += "[\"Distance\",\""+String(sat_distance)+"\"]";
    json += "]},";
    json += "{\"type\":\"title\",\"value\":\"Location\"},";
    json += "{\"type\":\"table\",\"name\":\"location\",\"data\":[";
    json += "[\"Latitude\",\""+String(latitude, 7)+"\"],";
    json += "[\"Longitude\",\""+String(longitude, 7)+"\"],";
    json += "[\"Altitude\",\""+String(altitude)+"\"],";
    json += "[\"Declination\",\""+String(declination)+"\"]";
    json += "]},";
    json += "{\"type\":\"title\",\"value\":\"Sensor\"},";
    json += "{\"type\":\"table\",\"name\":\"sensor\",\"data\":[";
    //json += "[\"Azimuth\",\""+String(azimuth)+"\"],";
    //json += "[\"Elevation\",\""+String(elevation)+"\"],";
    json += "[\"Yaw\",\""+String(yaw)+"\"],";
    json += "[\"Pitch\",\""+String(pitch)+"\"],";
    json += "[\"Roll\",\""+String(roll)+"\"],";
    json += "[\"Temperature\",\""+String(temperature)+"\"]";
    json += "]},";
    json += "{\"type\":\"refresh\",\"label\":\"Refresh\",\"name\":\"refresh\",\"value\":\""+String(refresh)+"\"}";
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/control", HTTP_POST, []() {
    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());
      if (doc["action"]) motor_mode = doc["action"];
    }

    String json;
    
    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Control\"},";
    json += "{\"type\":\"table\",\"name\":\"status\",\"data\":[";
    json += "[\"Yaw\",\""+String(yaw)+"\"],";
    json += "[\"Pitch\",\""+String(pitch)+"\"],";
    json += "[\"Roll\",\""+String(roll)+"\"],";
    json += "[\"Temperature\",\""+String(temperature)+"\"],";
    json += "[\"Motor\",\""+String(motor_mode)+"\"]";
    json += "]},";
    json += "{\"type\":\"arrows\"}";
    json += "]";
    
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/firmware", HTTP_POST, []() {
    String json;
    json += "[";
    json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\"},";
    json += "{\"type\":\"button\",\"label\":\"UPLOAD\",\"name\":\"upload\",\"value\":\"upload\"}";
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
  });
  webServer.on("/upload", HTTP_POST, []() {
    String json;
    json += "[";
    if (Update.hasError())
      json += "{\"type\":\"title\",\"value\":\"Upload Failed\"},";
   else
      json += "{\"type\":\"title\",\"value\":\"Upload Success\"},";

    json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\"},";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
    json += "]";
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = webServer.upload();
   if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      //WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);

        String json;
        json += "[";
        if (Update.hasError())
          json += "{\"type\":\"title\",\"value\":\"Upload Failed\"},";
       else
          json += "{\"type\":\"title\",\"value\":\"Upload Success\"},";
    
        json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\"},";
        json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
        json += "]";
        webServer.sendHeader("Access-Control-Allow-Origin", "*");
        webServer.send(200, "application/json", json);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  // Send a POST request to <IP>/post with a form field message set to <message>
  /*
  webServer.on("/post", HTTP_POST, []() {
      //nothing and dont remove it
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    String json;

    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });
  */
  webServer.onNotFound([]() {
    String json;
    //StaticJsonDocument<20> doc;
    //doc["message"] = "Endpoint not found";
    //serializeJson(doc, json);
    json += "[";
    json += "{\"type\":\"title\",\"value\":\"404 Not Found\"}";
    json += "]";
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(404, "application/json", json);
  });
  
  webServer.begin(); // Web server start
}

void webserverLoop() {
  webServer.handleClient();
}
