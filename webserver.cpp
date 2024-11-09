//  webserver
#include <ESP8266WebServer.h>
#include "ArduinoJson.h"

#include "webserver.h"

ESP8266WebServer webServer(80);

int getRSSIasQuality(int RSSI)
{
  int quality = 0;

  if (RSSI <= -100)
  {
    quality = 0;
  }
  else if (RSSI >= -50)
  {
    quality = 100;
  }
  else
  {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

void webserverSetup()
{
  webServer.on("/", HTTP_GET, []()
               { webServer.send(200, "text/html", index_html); });
  webServer.on("/system", HTTP_POST, []()
               {
    byte expand_system = 0;
    byte expand_wifiap = 1;
    byte expand_wifista = 1;
    byte expand_command = 1;

    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["expand_system"]) expand_system = doc["expand_system"];
      if (doc["expand_wifiap"]) expand_wifiap = doc["expand_wifiap"];
      if (doc["expand_wifista"]) expand_wifista = doc["expand_wifista"];
      if (doc["expand_command"]) expand_command = doc["expand_command"];

      if (doc["reboot"]) ESP.restart();
    }

    String json;
    
    json += "[";
    json += "{\"label\":\"System\",\"name\":\"expand_system\",\"value\":"+String(expand_system)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Chip ID\",\"name\":\"chip_id\",\"value\":\""+String(ESP.getChipId())+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Free Heap\",\"name\":\"free_heap\",\"value\":\""+String(ESP.getFreeHeap())+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Flash ID\",\"name\":\"flash_id\",\"value\":\""+String(ESP.getFlashChipId())+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Flash Size\",\"name\":\"flash_size\",\"value\":\""+String(ESP.getFlashChipSize())+"\",\"attrib\":\"disabled\"}";
    //json += "{\"type\":\"text\",\"label\":\"Flash Real Size\",\"name\":\"flash_real_size\",\"value\":\""+String(ESP.getFlashChipRealSize())+"\",\"attrib\":\"disabled\"}";
    json += "]},";
    json += "{\"label\":\"Wifi AP\",\"name\":\"expand_wifiap\",\"value\":"+String(expand_wifiap)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"AP MAC\",\"name\":\"ap_mac\",\"value\":\""+WiFi.softAPmacAddress()+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"AP Address\",\"name\":\"ap_address\",\"value\":\""+WiFi.softAPIP().toString()+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"AP SSID\",\"name\":\"ap_ssid\",\"value\":\""+String(APssid)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Connected Devices\",\"name\":\"connected_devices\",\"value\":\""+String(WiFi.softAPgetStationNum())+"\",\"attrib\":\"disabled\"}";
    json += "]},";
    json += "{\"label\":\"Wifi Station\",\"name\":\"expand_wifista\",\"value\":"+String(expand_wifista)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"MAC\",\"name\":\"mac\",\"value\":\""+WiFi.macAddress()+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Address\",\"name\":\"address\",\"value\":\""+WiFi.localIP().toString()+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"SSID\",\"name\":\"ssid\",\"value\":\""+WiFi.SSID()+"\",\"attrib\":\"disabled\"}";
    json += "]},";
    json += "{\"label\":\"Command\",\"name\":\"expand_command\",\"value\":"+String(expand_command)+",\"elements\":[";
    json += "{\"type\":\"button\",\"label\":\"REBOOT\",\"name\":\"reboot\",\"value\":\"reboot\",\"confirm\":\"Are you sure you want to reboot?\"}";
    json += "]}";
    json += "]";
    
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json); });
  webServer.on("/scan", HTTP_GET, []()
               {
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
    webServer.send(200, "application/json", json); });
  webServer.on("/config", HTTP_POST, []()
               {
    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["update"]) {
        if (doc["ssid"]) strcpy(ssid, (const char*)doc["ssid"]);
        if (doc["password"]) strcpy(password, (const char*)doc["password"]);

        if (doc["beep_enable"]) beep_enable = doc["beep_enable"];
        if (doc["analog_enable"]) analog_enable = doc["analog_enable"];
        if (doc["display_enable"]) display_enable = doc["display_enable"];
        if (doc["ads1115_enable"]) ads1115_enable = doc["ads1115_enable"];
        if (doc["mpu9250_enable"]) mpu9250_enable = doc["mpu9250_enable"];
    
        if (doc["latitude"]) latitude = doc["latitude"];
        if (doc["longitude"]) longitude = doc["longitude"];
        if (doc["altitude"]) altitude = doc["altitude"];
        if (doc["declination"]) declination = doc["declination"];

        if (doc["tracker_enable"]) tracker_enable = doc["tracker_enable"];
        if (doc["tle_name"]) strcpy(tle_name, (const char*)doc["tle_name"]);
        if (doc["tle_line1"]) strcpy(tle_line1, (const char*)doc["tle_line1"]);
        if (doc["tle_line2"]) strcpy(tle_line2, (const char*)doc["tle_line2"]);
        if (doc["auto_control"]) auto_control = doc["auto_control"];

        if (doc["motor_enable"]) motor_enable = doc["motor_enable"];
        if (doc["ms_per_deg"]) ms_per_deg = doc["ms_per_deg"];

        saveConfig();
        Serial.println("Save config");
      }
    }

    String json;
    String sep;
    int n = WiFi.scanNetworks();

    json += "[";
    json += "{\"label\":\"Wifi Connect\",\"name\":\"expand_wifi\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"select\",\"label\":\"SSID\",\"name\":\"ssid\",\"value\":\"" + WiFi.SSID() + "\",\"options\":[";
    if (n > 0) {
      for (int i = 0; i < n; i++) {
        json += sep+"[\""+WiFi.SSID(i)+"\",\""+WiFi.SSID(i)+" "+String(getRSSIasQuality(WiFi.RSSI(i)))+"%";
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
    json += "{\"type\":\"text\",\"label\":\"Password\",\"name\":\"password\",\"value\":\"" + String(password) + "\"}";
    json += "]},";

    json += "{\"label\":\"Components\",\"name\":\"expand_component\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"select\",\"label\":\"Beep\",\"name\":\"beep_enable\",\"value\":\"" + String(beep_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"Analog\",\"name\":\"analog_enable\",\"value\":\"" + String(analog_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"Display\",\"name\":\"display_enable\",\"value\":\"" + String(display_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"ADS1115\",\"name\":\"ads1115_enable\",\"value\":\"" + String(ads1115_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"select\",\"label\":\"MPU9250\",\"name\":\"mpu9250_enable\",\"value\":\"" + String(mpu9250_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]}";
    json += "]},";

    json += "{\"label\":\"My Location\",\"name\":\"expand_location\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Latitude\",\"name\":\"latitude\",\"value\":\"" + String(latitude, 7) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Longitude\",\"name\":\"longitude\",\"value\":\"" + String(longitude, 7) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Altitude\",\"name\":\"altitude\",\"value\":\"" + String(altitude) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Declination\",\"name\":\"declination\",\"value\":\"" + String(declination) + "\"}";
    json += "]},";

    json += "{\"label\":\"Tracker\",\"name\":\"expand_tracker\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"select\",\"label\":\"Tracker\",\"name\":\"tracker_enable\",\"value\":\"" + String(tracker_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"text\",\"label\":\"TLE Name\",\"name\":\"tle_name\",\"value\":\"" + String(tle_name) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"TLE Line1\",\"name\":\"tle_line1\",\"value\":\"" + String(tle_line1) + "\"},";
    json += "{\"type\":\"text\",\"label\":\"TLE Line2\",\"name\":\"tle_line2\",\"value\":\"" + String(tle_line2) + "\"},";
    json += "{\"type\":\"select\",\"label\":\"Auto Control\",\"name\":\"auto_control\",\"value\":\"" + String(auto_control) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]}";
    json += "]},";

    json += "{\"label\":\"Motor\",\"name\":\"expand_motor\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"select\",\"label\":\"Motor\",\"name\":\"motor_enable\",\"value\":\"" + String(motor_enable) + "\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"text\",\"label\":\"msPerDeg\",\"name\":\"ms_per_deg\",\"value\":\"" + String(ms_per_deg, 3) + "\"}";
    json += "]},";

    json += "{\"label\":\"Page\",\"name\":\"expand_page\",\"value\":1,\"elements\":[";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
    json += "]}";
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json); });
  webServer.on("/app", HTTP_POST, []()
               {
    byte refresh = 2;
    int degreeAz = 0;
    int degreeEl = 0;

    byte expand_location = 0;
    byte expand_satellite = 1;
    byte expand_mpu9250 = 1;
    byte change_satellite = 0;
    byte expand_motor = 1;
    byte expand_motor1 = 1;
    byte expand_motor2 = 1;
    byte expand_errors = 0;
    byte expand_page = 0;

    if (webServer.arg("plain") != "{}") {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, webServer.arg("plain").c_str());

      if (doc["actionMotor"]) {
        int action = doc["actionMotor"];
        if (action == 5) { // enter
          stopAz();
          stopEl();
          motor_az = 0.0;
          motor_el = 0.0;
        }
        else if (action == 1) forwardEl(); // up
        else if (action == 2) reverseEl(); // down
        else if (action == 3) reverseAz(); // left
        else if (action == 4) forwardAz(); // right
        //else if (action == 5) {
        //  forwardAz();
        //  forwardEl();
        //}
        //else if (action == 6) {
        //  reverseAz();
        //  reverseEl();
        //}
      }
      else if (doc["actionAz"]) {
        if (doc["degreeAz"]) {
          degreeAz = doc["degreeAz"];
          Serial.print("rotateAz ");
          Serial.println(degreeAz);
          setAz(degreeAz);
        }
      }
      else if (doc["actionEl"]) {
        if (doc["degreeEl"]) {
          degreeEl = doc["degreeEl"];
          Serial.print("rotateEl ");
          Serial.println(degreeEl);
          setEl(degreeEl);
        }
      }

      if (doc["expand_location"]) expand_location = doc["expand_location"];
      if (doc["expand_satellite"]) expand_satellite = doc["expand_satellite"];
      if (doc["expand_mpu9250"]) expand_mpu9250 = doc["expand_mpu9250"];
      if (doc["expand_motor"]) expand_motor = doc["expand_motor"];
      if (doc["expand_motor1"]) expand_motor1 = doc["expand_motor1"];
      if (doc["expand_motor2"]) expand_motor2 = doc["expand_motor2"];
      if (doc["change_satellite"]) change_satellite = doc["change_satellite"];
      if (doc["expand_errors"]) expand_errors = doc["expand_errors"];
      if (doc["expand_page"]) expand_page = doc["expand_page"];

      if (doc["update"]) {
        if (doc["catalog_number"]) catalog_number = String(doc["catalog_number"]);
        getTLE(catalog_number);
        if (doc["auto_control"]) auto_control = doc["auto_control"];
      }

      if (doc["refresh"]) refresh = doc["refresh"];
    }

    String json;
    String disabled = "false";
    
    json += "[";
    if (analog_enable) {
      json += "{\"label\":\"Analog Sensor\",\"name\":\"expand_analog\",\"value\":1,\"elements\":[";
      json += "{\"type\":\"text\",\"label\":\"Volt\",\"name\":\"device_voltage\",\"value\":\""+String(a_voltage)+"\",\"attrib\":\"disabled\"}";
      json += "]},";
    }

    if (mpu9250_enable) {
      json += "{\"label\":\"MPU9250\",\"name\":\"expand_mpu9250\",\"value\":"+String(expand_mpu9250)+",\"elements\":[";
      json += "{\"type\":\"text\",\"label\":\"Yaw\",\"name\":\"yaw\",\"value\":\""+String(yaw)+"\",\"attrib\":\"disabled\"},";
      json += "{\"type\":\"text\",\"label\":\"Pitch\",\"name\":\"pitch\",\"value\":\""+String(pitch)+"\",\"attrib\":\"disabled\"},";
      json += "{\"type\":\"text\",\"label\":\"Roll\",\"name\":\"roll\",\"value\":\""+String(roll)+"\",\"attrib\":\"disabled\"},";
      json += "{\"type\":\"text\",\"label\":\"Temperature\",\"name\":\"temperature\",\"value\":\""+String(temperature)+"\",\"attrib\":\"disabled\"}";
      json += "]},";
    }

    json += "{\"label\":\"My Location\",\"name\":\"expand_location\",\"value\":"+String(expand_location)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Latitude\",\"name\":\"latitude\",\"value\":\""+String(latitude,7)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Longitude\",\"name\":\"longitude\",\"value\":\""+String(longitude,7)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Altitude\",\"name\":\"altitude\",\"value\":\""+String(altitude)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Declination\",\"name\":\"declination\",\"value\":\""+String(declination)+"\",\"attrib\":\"disabled\"}";
    json += "]},";

    json += "{\"label\":\"Satellite\",\"name\":\"expand_satellite\",\"value\":"+String(expand_satellite)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Name\",\"name\":\"tle_name\",\"value\":\""+String(tle_name)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Epoch\",\"name\":\"epoch\",\"value\":\""+String(epoch)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Latitude\",\"name\":\"sat_latitude\",\"value\":\""+String(sat_latitude,7)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Longitude\",\"name\":\"sat_longitude\",\"value\":\""+String(sat_longitude,7)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Altitude\",\"name\":\"sat_altitude\",\"value\":\""+String(sat_altitude)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Azimuth\",\"name\":\"sat_azimuth\",\"value\":\""+String(sat_azimuth)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Elevation\",\"name\":\"sat_elevation\",\"value\":\""+String(sat_elevation)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Distance\",\"name\":\"sat_distance\",\"value\":\""+String(sat_distance)+"\",\"attrib\":\"disabled\"}";
    json += "]},";

    json += "{\"label\":\"Change Satellite\",\"name\":\"change_satellite\",\"value\":"+String(change_satellite)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Catalog Number\",\"name\":\"catalog_number\",\"value\":\""+catalog_number+"\"},";
    json += "{\"type\":\"select\",\"label\":\"Auto Control\",\"name\":\"auto_control\",\"value\":\""+String(auto_control)+"\",\"options\":[[\"0\",\"Disabled\"],[\"1\",\"Enabled\"]]},";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
    json += "]},";

    json += "{\"label\":\"Motor\",\"name\":\"expand_motor\",\"value\":"+String(expand_motor)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Motor Mode Az\",\"name\":\"motor_mode_az\",\"value\":\""+String(motor_mode_az)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Motor Az\",\"name\":\"motor_az\",\"value\":\""+String(motor_az)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Motor Mode El\",\"name\":\"motor_mode_el\",\"value\":\""+String(motor_mode_el)+"\",\"attrib\":\"disabled\"},";
    json += "{\"type\":\"text\",\"label\":\"Motor El\",\"name\":\"motor_el\",\"value\":\""+String(motor_el)+"\",\"attrib\":\"disabled\"}";
    json += "]},";

    json += "{\"label\":\"Motor Control 1\",\"name\":\"expand_motor1\",\"value\":"+String(expand_motor1)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Az Degree\",\"name\":\"degreeAz\",\"value\":\""+String(degreeAz)+"\"},";
    disabled = motor_mode_az == 0 ? "false" : "true";
    json += "{\"type\":\"button\",\"label\":\"Set Az\",\"name\":\"actionAz\",\"value\":\"1\",\"disabled\":\""+disabled+"\"},";
    json += "{\"type\":\"text\",\"label\":\"El Degree\",\"name\":\"degreeEl\",\"value\":\""+String(degreeEl)+"\"},";
    disabled = motor_mode_el == 0 ? "false" : "true";
    json += "{\"type\":\"button\",\"label\":\"Set El\",\"name\":\"actionEl\",\"value\":\"1\",\"disabled\":\""+disabled+"\"}";
    json += "]},";

    json += "{\"label\":\"Motor Control 2\",\"name\":\"expand_motor2\",\"value\":"+String(expand_motor2)+",\"elements\":[";
    json += "{\"type\":\"arrows\",\"name\":\"actionMotor\"}";
    json += "]},";

    json += "{\"label\":\"Error\",\"name\":\"expand_errors\",\"value\":"+String(expand_errors)+",\"elements\":[";
    json += "{\"type\":\"text\",\"label\":\"Get\",\"name\":\"get_error\",\"value\":\""+String(get_error)+"\",\"attrib\":\"disabled\"}";
    json += "{\"type\":\"text\",\"label\":\"Http Code\",\"name\":\"http_code\",\"value\":\""+String(http_code)+"\",\"attrib\":\"disabled\"}";
    json += "]},";

    json += "{\"label\":\"Page\",\"name\":\"expand_page\",\"value\":"+String(expand_page)+",\"elements\":[";
    json += "{\"type\":\"refresh\",\"label\":\"Refresh\",\"name\":\"refresh\",\"value\":"+String(refresh)+"}";
    json += "]}";

    json += "]";
    
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json); });
  webServer.on("/firmware", HTTP_POST, []()
               {
    String json;
    json += "[";
    json += "{\"label\":\"Firmware Upgrade\",\"name\":\"firmware_upgrade\",\"value\":\"1\",\"elements\":[";
    json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\",\"accept\":\".bin\"},";
    json += "{\"type\":\"button\",\"label\":\"UPLOAD\",\"name\":\"upload\",\"value\":\"upload\"}";
    json += "]}";
    json += "]";

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json); });
  webServer.on(
      "/upload", HTTP_POST, []()
      {
    String json;
    json += "[";
    json += "{\"label\":\"Firmware Upgrade\",\"name\":\"firmware_upgrade\",\"value\":\"1\",\"elements\":[";
    if (Update.hasError())
      json += "{\"type\":\"alert\",\"value\":\"Upload failed!\"},";
    else
      json += "{\"type\":\"alert\",\"value\":\"Upload success!\"},";

    json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\",\"accept\":\".bin\"},";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
    json += "]}";
    json += "]";
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", json);
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = webServer.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.setDebugOutput(true);
          // WiFiUDP::stopAll();
          Serial.printf("Update: %s\n", upload.filename.c_str());
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);

            String json;
            json += "[";
            json += "{\"label\":\"Firmware Upgrade\",\"name\":\"firmware_upgrade\",\"value\":\"1\",\"elements\":[";
            if (Update.hasError())
              json += "{\"type\":\"alert\",\"value\":\"Upload failed!\"},";
            else
              json += "{\"type\":\"alert\",\"value\":\"Upload success!\"},";

            json += "{\"type\":\"file\",\"label\":\"File\",\"name\":\"file\",\"value\":\"\",\"accept\":\".bin\"},";
            json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\",\"value\":\"update\"}";
            json += "]}";
            json += "]";
            webServer.sendHeader("Access-Control-Allow-Origin", "*");
            webServer.send(200, "application/json", json);
          }
          else
          {
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
  webServer.onNotFound([]()
                       {
    String json;
    //StaticJsonDocument<20> doc;
    //doc["message"] = "Endpoint not found";
    //serializeJson(doc, json);
    json += "[";
    json += "{\"label\":\"404 Error\",\"name\":\"not_found\",\"value\":\"1\",\"elements\":[";
    json += "{\"type\":\"alert\",\"value\":\"Page not found!\"}";
    json += "]}";
    json += "]";
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(404, "application/json", json); });

  Serial.print("Starting Webserver ... ");
  webServer.begin();
  Serial.println("Done");
}

void webserverLoop()
{
  webServer.handleClient();
}
