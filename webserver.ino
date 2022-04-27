#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

// Create AsyncWebServer object on port 80
AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    //if (ledState){
    //  return "ON";
    //}
    //else{
      return "OFF";
    //}
  }
  return String();
}

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
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  webServer.on("/system", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json;
    
    json += "[";
    json += "{\"type\":\"title\",\"value\":\"System\"},";
    json += "{\"type\":\"table\",\"name\":\"system\",\"data\":[";
    json += "[\"Chip ID\",\""+String(ESP.getChipId())+"\"],";
    json += "[\"Free Heap\",\""+String(ESP.getFreeHeap())+"\"],";
    json += "[\"Flash ID\",\""+String(ESP.getFlashChipId())+"\"],";
    json += "[\"Flash Size\",\""+String(ESP.getFlashChipSize())+"\"],";
    json += "[\"Flash Real Size\",\""+String(ESP.getFlashChipRealSize())+"\"]";
    json += "]}";
    json += "]";
    
    //request->send(200, "application/json", json);
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
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

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "";

    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Wifi\"},";
    json += "{\"type\":\"text\",\"label\":\"Hardware AP\",\"name\":\"hardware1\",\"value\":\"" + WiFi.softAPmacAddress() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Address AP\",\"name\":\"address1\",\"value\":\"" + WiFi.softAPIP().toString() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"SSID AP\",\"name\":\"ssid1\",\"value\":\"" + WiFi.SSID() + "\"},";

    json += "{\"type\":\"text\",\"label\":\"Hardware\",\"name\":\"hardware\",\"value\":\"" + WiFi.macAddress() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Address\",\"name\":\"address\",\"value\":\"" + WiFi.localIP().toString() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"SSID\",\"name\":\"ssid\",\"value\":\"" + WiFi.SSID() + "\"},";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\"}";
    json += "]";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
      //nothing and dont remove it
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument doc(1024);
    String json = "";

    deserializeJson(doc, data);
    // if we have data then
    // set local data
    // save config
    //serializeJson(doc, json);

    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Wifi\"},";
    json += "{\"type\":\"text\",\"label\":\"Hardware AP\",\"name\":\"hardware1\",\"value\":\"" + WiFi.softAPmacAddress() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Address AP\",\"name\":\"address1\",\"value\":\"" + WiFi.softAPIP().toString() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"SSID AP\",\"name\":\"ssid1\",\"value\":\"" + WiFi.SSID() + "\"},";

    json += "{\"type\":\"text\",\"label\":\"Hardware\",\"name\":\"hardware\",\"value\":\"" + WiFi.macAddress() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"Address\",\"name\":\"address\",\"value\":\"" + WiFi.localIP().toString() + "\"},";
    json += "{\"type\":\"text\",\"label\":\"SSID\",\"name\":\"ssid\",\"value\":\"" + WiFi.SSID() + "\"},";
    json += "{\"type\":\"button\",\"label\":\"UPDATE\",\"name\":\"update\"}";
    json += "]";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.on("/tracker", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "";
    String opts = "";
    String sep="";

    for (byte i=0; i<61; i++) {
      opts += sep + "[\""+String(i)+"\",\""+String(i)+"\"]";
      sep = ",";
    }

    json += "[";
    json += "{\"type\":\"title\",\"value\":\"Satellite\"},";
    json += "{\"type\":\"table\",\"name\":\"satellite\",\"data\":[";
    json += "[\"Name\",\""+String(sat_name)+"\"],";
    json += "[\"Epoch\",\""+String(epoch)+"\"],";
    json += "[\"Latitude\",\""+String(sat_latitude, 7)+"\"],";
    json += "[\"Longitude\",\""+String(sat_longitude, 7)+"\"],";
    json += "[\"Altitude (km)\",\""+String(sat_altitude)+"\"],";
    json += "[\"Azimuth\",\""+String(sat_azimuth)+"\"],";
    json += "[\"Elevation\",\""+String(sat_elevation)+"\"],";
    json += "[\"Distance\",\""+String(sat_distance)+"\"]";
    json += "]},";
    json += "{\"type\":\"title\",\"value\":\"Location\"},";
    json += "{\"type\":\"table\",\"name\":\"location\",\"data\":[";
    json += "[\"Latitude\",\""+String(latitude, 7)+"\"],";
    json += "[\"Longitude\",\""+String(longitude, 7)+"\"],";
    json += "[\"Altitude (m)\",\""+String(altitude)+"\"],";
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
    json += "{\"type\":\"select\",\"label\":\"Refresh\",\"name\":\"refresh\",\"value\":\"2\",\"options\":["+opts+"]}";
    json += "]";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.on("/tracker", HTTP_POST, [](AsyncWebServerRequest *request) {
      //nothing and dont remove it
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);

    String json = "";
    String opts = "";
    String sep="";

    for (byte i=0; i<61; i++) {
      opts += sep + "[\""+String(i)+"\",\""+String(i)+"\"]";
      sep = ",";
    }

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
    json += "{\"type\":\"select\",\"label\":\"Refresh\",\"name\":\"refresh\",\"value\":\""+String(doc["refresh"])+"\",\"options\":["+opts+"]}";
    json += "]";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  // Send a POST request to <IP>/post with a form field message set to <message>
  webServer.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      //nothing and dont remove it
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    String json;

    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    String json;
    StaticJsonDocument<20> doc;
    doc["message"] = "Endpoint not found";
    serializeJson(doc, json);
    request->send(404, "application/json", json);
  });
  
  // websockets
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
      {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
          data[len] = 0;
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, data);

          latitude = doc["latitude"];
          longitude = doc["longitude"];
          altitude = doc["altitude"];

          //Serial.println(test);
        }
        break;
      }
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
      default:
        break;
    }
  });
  webServer.addHandler(&ws);
  
  webServer.begin(); // Web server start
}

void webserverLoop() {
  ws.cleanupClients();
}

void wsLoop() {
  String json;
  //char datetime[20] = {0};
  //sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second); 

  json += "{";
  json += "\"satellite\": {";
  json += "\"timestamp\":\"" + String(epoch) + "\"";
  json += ",\"latitude\":" + String(sat_latitude, 7);
  json += ",\"longitude\":" + String(sat_longitude, 7);
  json += ",\"altitude\":" + String(sat_altitude);
  json += ",\"azimuth\":" + String(sat_azimuth);
  json += ",\"elevation\":" + String(sat_elevation);
  json += ",\"distance\":" + String(sat_distance);
  json += "}";
  json += ",\"location\": {";
  json += "\"latitude\":" + String(latitude, 7);
  json += ",\"longitude\":" + String(longitude, 7);
  json += ",\"altitude\":" + String(altitude);
  json += ",\"declination\":" + String(declination);
  json += "}";
  json += "}";

  ws.textAll(json);
}
