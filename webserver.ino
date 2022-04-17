// https://randomnerdtutorials.com/esp8266-nodemcu-websocket-server-arduino/

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

//ESP8266WebServer webServer(80);
// Create AsyncWebServer object on port 80
AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");

void webserverSetup() {
  webServer.on("/", handleRoot);
  webServer.on("/info", handleInfo);
  webServer.on("/config", handleConfig);
  webServer.on("/control", handleControl);
  webServer.on("/configsave", handleConfigSave);
  webServer.on("/reset", handleReset);
  //webServer.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  //webServer.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.on("/update", handleUpdate);
  webServer.on("/updatesave", HTTP_POST, []() {
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
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
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  webServer.onNotFound(handleNotFound);
  webServer.begin(); // Web server start
}

void webserverLoop() {
  // web server
  webServer.handleClient();
}


const char HTTP_STYLE[] PROGMEM = "<style>"
  "body {color: #333;font-family: Century Gothic, sans-serif;font-size: 16px;line-height: 24px;margin: 0;padding: 0}"
  "nav{background: #3861d1;color: #fff;display: block;font-size: 1.3em;padding: 1em}"
  "nav b{display: block;font-size: 1.5em;margin-bottom: 0.5em}"
  "textarea,input,select{outline: 0;font-size: 14px;border: 1px solid #ccc;padding: 8px;width: 100%;box-sizing: border-box}"
  "input[type='checkbox']{float: left;width: 20px}"
  "textarea:focus,input:focus,select:focus{border-color: #5ab}"
  ".container{margin: auto;width: 90%}"
  "@media(min-width:1200px){.container{margin: auto;width: 30%}}"
  "@media(min-width:768px) and (max-width:1200px){.container{margin: auto;width: 50%}}"
  //"h1{font-size: 3em}"
  //".btn{background: #0ae;border-radius: 4px;border: 0;color: #fff;cursor: pointer;font-size: 1.5em;display: inline-block;margin: 2px 0;padding: 10px 14px 11px;width: 100%}"
  ".btn{background: #0ae;border-radius: 4px;border: 0;color: #fff;cursor: pointer;font-size: 1.5em;display: inline-block;margin: 2px 0;padding: 10px 0px 11px;width: 100%;text-align: center;text-decoration: none}"
  ".btn:hover{background: #09d}"
  ".btn:active,.btn:focus{background: #08b}"
  "label>*{display: inline}"
  "form>*{display: block;margin-bottom: 10px}"
  ".msg{background: #def;border-left: 5px solid #59d;padding: 1.5em}"
  //".q{float: right;width: 64px;text-align: right}"
  //".l{background: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==') no-repeat left center;background-size: 1em}"
  ".table {table-layout : fixed;width: 100%}"
  ".table td{padding:.5em;text-align:left}"
  ".table tbody>:nth-child(2n-1){background:#ddd}"
  "</style>";
//const char HTTP_SCRIPT[] PROGMEM = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";

String createPage(String body) {
  String html = "";
  html += F( "<!DOCTYPE html>"
             "<html lang='en'>"
             "<head><title>" )+String(APPNAME)+F( "</title>"
             "<meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'>" );
  html += FPSTR(HTTP_STYLE);
  //html += FPSTR(HTTP_SCRIPT);
  html += F( "</head>"
             "<body>"
             "<div class='container'>" );
  html += body;
  html += F( "</div>"
             "</body>"
             "</html>" );
  return html;
}

void handleRoot() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String body = F( "<nav><b>" )+String(APPNAME)+F( "</b>Main Page</nav><br>" );
  
  body += F( "<a class=\"btn\" href=\"/info\">Information</a><br><br>"
             "<a class=\"btn\" href=\"/config\">Configuration</a><br><br>"
             "<a class=\"btn\" href=\"/control\">Control</a><br><br>"
             "<a class=\"btn\" href=\"/update\">Update</a><br><br>" );

  body += F( "<div class=\"msg\">" );

  if (WiFi.SSID() != "") {
    body += F( "Configured to connect to access point " );
    body += WiFi.SSID();
    if (WiFi.status() == WL_CONNECTED) {
      body += F( " and <strong>currently connected</strong> on IP <a href=\"http://" );
      body += WiFi.localIP().toString();
      body += F( "/\">" );
      body += WiFi.localIP().toString();
      body += F( "</a>" );
    }
    else {
      body += F(" but <strong>not currently connected</strong> to network.");
    }
  }
  else {
    body += F("No network currently configured.");
  }

  body += F( "</div><br><br>" );

  webServer.send(200, "text/html", createPage(body));
}

void handleInfo() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String body = F( "<nav><b>" )+String(APPNAME)+F( "</b>Information</nav><br>" );

  body += F("<h3>Device Data</h3>");
  body += F("<table class=\"table\"><tbody><tr><td>Chip ID</td><td>");
  body += ESP.getChipId();
  body += F("</td></tr>");
  body += F("<tr><td>Flash Chip ID</td><td>");
  body += ESP.getFlashChipId();
  body += F("</td></tr>");
  body += F("<tr><td>IDE Flash Size</td><td>");
  body += ESP.getFlashChipSize();
  body += F(" bytes</td></tr>");
  body += F("<tr><td>Real Flash Size</td><td>");
  body += ESP.getFlashChipRealSize();
  body += F(" bytes</td></tr>");
  body += F("<tr><td>Access Point IP</td><td>");
  body += WiFi.softAPIP().toString();
  body += F("</td></tr>");
  body += F("<tr><td>Access Point MAC</td><td>");
  body += WiFi.softAPmacAddress();
  body += F("</td></tr>");

  body += F("<tr><td>SSID</td><td>");
  body += WiFi.SSID();
  body += F("</td></tr>");

  body += F("<tr><td>Station IP</td><td>");
  body += WiFi.localIP().toString();
  body += F("</td></tr>");

  body += F("<tr><td>Station MAC</td><td>");
  body += WiFi.macAddress();
  body += F("</td></tr>");
  body += F("</tbody></table><br>");

  body += F("<h3>Motor Status</h3>");
  body += F("<table class=\"table\"><tbody><tr><td>Mode</td><td>");
  body += String(motor_mode);
  body += F("</td></tr></tbody></table><br>");

  body += F("<h3>MPU9250 Data</h3>");
  body += F("<table class=\"table\"><tbody><tr><td>Yaw</td><td>");
  body += String(yaw);
  body += F(" deg</td></tr><tr><td>Pitch</td><td>");
  body += String(pitch);
  body += F(" deg</td></tr><tr><td>Roll</td><td>");
  body += String(roll);
  body += F(" deg</td></tr><tr><td>Temperature</td><td>");
  body += String(temperature);
  body += F(" C</td></tr></tbody></table><br>");

  body += F("<h3>Error Counter</h3>");
  body += F("<table class=\"table\"><tbody><tr><td>Wifi Error</td><td>");
  body += String(wifi_error);
  body += F("</td></tr><tr><td>MPU9250 Found</td><td>");
  body += String(mpu9250_found);
  body += F("</td></tr><tr><td>AK8963 Found</td><td>");
  body += String(ak8963_found);
  body += F("</td></tr><tr><td>MPU Error</td><td>");
  body += String(mpu_error);
  body += F("</td></tr><tr><td>Run Time (minutes)</td><td>");
  body += String(run_time);
  body += F("</td></tr></tbody></table><br><br>");

  body += F( "<a class=\"btn\" href=\"/reset\">Reset</a><br>" );
  body += F( "<a class=\"btn\" href=\"/\">Back</a><br><br>" );
  
  webServer.send(200, "text/html", createPage(body));
}

void handleConfig() {
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");

  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String body = F( "<nav><b>" )+String(APPNAME)+F( "</b>Configuration</nav><br>" );
  
  body += F( "<form method=\"post\" action=\"/configsave\">"
             "<h3>Network Config</h3>"
             "<label>SSID</label>"
             "<select name=\"n\">" );
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      body += F( "<option value=\"" );
      body += WiFi.SSID(i);
      if (String(ssid) == WiFi.SSID(i)) {
        body += F( "\" selected>" );
      }
      else {
        body += F( "\">" );
      }
      
      body += WiFi.SSID(i);
      body += F( " " );
      body += getRSSIasQuality(WiFi.RSSI(i));
      body += F( "% " );
      if (WiFi.encryptionType(i) != ENC_TYPE_NONE) {
        body += F( " &#9906" );
      }
      body += F( "</option>" );
    }
  } else {
    body += F( "<option value=\"\"></option>" );
  }
  body += F( "</select><label>Password</label><input type=\"password\" name=\"p\" length=32 placeholder=\"password\" value=\"" );
  body += password;
  body += F( "\"><br>" );

  body += F( "<h3>MPU9250 Config</h3>"
             "<label>MPU9250</label>"
             "<select name=\"mpu9250_e\">"
             "<option value=\"0\"" );
  if (mpu9250_enable == 0) body += F( " selected" );
  body += F( ">Disable</option><option value=\"1\"" );
  if (mpu9250_enable == 1) body += F( " selected" );
  body += F( ">Enable</option>"
             "</select><br>" );

  body += F( "<label>Mag bias X</label><input type=\"text\" name=\"m_bias_x\" placeholder=\"0.0\" value=\"" );
  body += String(m_bias_x);
  body += F( "\"><br>" );
  body += F( "<label>Mag bias Y</label><input type=\"text\" name=\"m_bias_y\" placeholder=\"0.0\" value=\"" );
  body += String(m_bias_y);
  body += F( "\"><br>" );
  body += F( "<label>Mag bias Z</label><input type=\"text\" name=\"m_bias_z\" placeholder=\"0.0\" value=\"" );
  body += String(m_bias_z);
  body += F( "\"><br>" );
  body += F( "<label>Declination</label><input type=\"text\" name=\"declination\" placeholder=\"0.0\" value=\"" );
  body += String(declination);
  body += F( "\"><br>" );

  body += F( "<h3>Display Config</h3>"
             "<label>Display</label>"
             "<select name=\"display_e\">"
             "<option value=\"0\"" );
  if (display_enable == 0) body += F( " selected" );
  body += F( ">Disable</option><option value=\"1\"" );
  if (display_enable == 1) body += F( " selected" );
  body += F( ">Enable</option>"
             "</select><br>" );

  body += F( "<h3>Beep Config</h3>"
             "<label>Beep</label>"
             "<select name=\"beep_e\">"
             "<option value=\"0\"" );
  if (beep_enable == 0) body += F( " selected" );
  body += F( ">Disable</option><option value=\"1\"" );
  if (beep_enable == 1) body += F( " selected" );
  body += F( ">Enable</option>"
             "</select><br>"
             "<button class=\"btn\" type=\"submit\">Save</button>"
             "</form>"
             "<a class=\"btn\" href=\"/\">Back</a><br><br>" );

  webServer.send(200, "text/html", createPage(body));
}


void handleConfigSave() {
  Serial.println("config save");
  webServer.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  webServer.arg("p").toCharArray(password, sizeof(password) - 1);

  mpu9250_enable = webServer.arg("mpu9250_e").toInt();
  display_enable = webServer.arg("display_e").toInt();
  beep_enable = webServer.arg("beep_e").toInt();

  m_bias_x = webServer.arg("m_bias_x").toFloat();
  m_bias_y = webServer.arg("m_bias_y").toFloat();
  m_bias_z = webServer.arg("m_bias_z").toFloat();
  declination = webServer.arg("declination").toFloat();

  webServer.sendHeader("Location", "config", true);
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
  saveConfig();
}

void handleControl() {
  if (webServer.hasArg("motor_m")) motor_mode = webServer.arg("motor_m").toInt();

  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String body = F( "<nav><b>" )+String(APPNAME)+F( "</b>Control</nav><br>" );

/*
  body += F( "<form method=\"post\" action=\"/control\">"
             "<h3>Motor Control</h3>"
             "<label>Mode</label>"
             "<select name=\"motor_m\">"
             "<option value=\"0\"" );
  if (motor_mode == 0) body += F( " selected" );
  body += F( ">Stop</option><option value=\"1\"" );
  if (motor_mode == 1) body += F( " selected" );
  body += F( ">Fwd Az</option><option value=\"2\"" );
  if (motor_mode == 2) body += F( " selected" );
  body += F( ">Rev Az</option><option value=\"3\"" );
  if (motor_mode == 3) body += F( " selected" );
  body += F( ">Fwd El</option><option value=\"4\"" );
  if (motor_mode == 4) body += F( " selected" );
  body += F( ">Rev El</option><option value=\"5\"" );
  if (motor_mode == 5) body += F( " selected" );
  body += F( ">Fwd Both</option><option value=\"6\"" );
  if (motor_mode == 6) body += F( " selected" );
  body += F( ">Rev Both</option>"
             "</select><br>"
             "<button class=\"btn\" type=\"submit\">Submit</button>"
             "</form>"
*/
  body += F("<h3>MPU9250 Data</h3>");
  body += F("<table class=\"table\"><tbody><tr><td>Yaw</td><td>");
  body += String(yaw);
  body += F(" deg</td></tr><tr><td>Pitch</td><td>");
  body += String(pitch);
  body += F(" deg</td></tr><tr><td>Roll</td><td>");
  body += String(roll);
  body += F(" deg</td></tr><tr><td>Temperature</td><td>");
  body += String(temperature);
  body += F(" C</td></tr></tbody></table><br>");

  body += F( "<table class=\"table\"><tbody><tr><td>"
             "<form method=\"post\" action=\"/control\">"
             "<input type=\"hidden\" name=\"motor_m\" value=\"2\">"
             "<button class=\"btn\" type=\"submit\">Left</button>"
             "</form></td><td>"
             "<form method=\"post\" action=\"/control\">"
             "<input type=\"hidden\" name=\"motor_m\" value=\"1\">"
             "<button class=\"btn\" type=\"submit\">Right</button>"
             "</form></td></tr><tr><td>"
             "<form method=\"post\" action=\"/control\">"
             "<input type=\"hidden\" name=\"motor_m\" value=\"3\">"
             "<button class=\"btn\" type=\"submit\">Up</button>"
             "</form></td><td>"
             "<form method=\"post\" action=\"/control\">"
             "<input type=\"hidden\" name=\"motor_m\" value=\"4\">"
             "<button class=\"btn\" type=\"submit\">Down</button>"
             "</form></td></tr></tbody></table>"
             "<form method=\"post\" action=\"/control\">"
             "<input type=\"hidden\" name=\"motor_m\" value=\"0\">"
             "<button class=\"btn\" type=\"submit\">STOP</button>"
             "</form><br>"

             "<a class=\"btn\" href=\"/\">Back</a><br><br>" );

  webServer.send(200, "text/html", createPage(body));
}

void handleReset() {
  wifi_error = 0;

  run_time = 0;

  webServer.sendHeader("Location", "/", true);
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
  ESP.restart();
}

void handleUpdate() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String body = F( "<nav><b>" )+String(APPNAME)+F( "</b>Update</nav><br>" );
  
  body += F( "<h3>Upload Firmware</h3>"
             "<form method=\"post\" action=\"updatesave\" enctype=\"multipart/form-data\">"
             "<input type=\"file\" name=\"update\"><br>"
             "<button class=\"btn\" type=\"submit\">Upload</button>"
             "</form>"
             "<a class=\"btn\" href=\"/\">Back</a><br><br>" );

  webServer.send(200, "text/html", createPage(body));
}

void handleNotFound() {
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += webServer.uri();
  message += F("\nMethod: ");
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += webServer.args();
  message += F("\n");

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n");
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/plain", message);
}

/*
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

uint16_t crc16(uint8_t *buffer, uint8_t length)
{
    uint8_t i, j;
    uint16_t crc = 0xFFFF;
    uint16_t tmp;

    // Calculate the CRC.
    for (i = 0; i < length; i++) {
        crc = crc ^ buffer[i];
        for (j = 0; j < 8; j++) {
            tmp = crc & 0x0001;
            crc = crc >> 1;
            if (tmp) {
                crc = crc ^ 0xA001;
            }
        }
    }
    return crc;
}
*/
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
