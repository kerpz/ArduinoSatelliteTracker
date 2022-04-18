#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

// Create AsyncWebServer object on port 80
AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
  <title>Webserver v1.0</title>
  <meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'>
  <style>
    body {color: #333;font-family: Century Gothic, sans-serif;font-size: 16px;line-height: 24px;margin: 0;padding: 0}
    nav{background: #3861d1;color: #fff;display: block;font-size: 1.3em;padding: 1em}
    nav b{display: block;font-size: 1.5em;margin-bottom: 0.5em}
    textarea,input,select{outline: 0;font-size: 14px;border: 1px solid #ccc;padding: 8px;width: 100%;box-sizing: border-box}
    input[type='checkbox']{float: left;width: 20px}
    textarea:focus,input:focus,select:focus{border-color: #5ab}
    .container{margin: auto;width: 90%}
    @media(min-width:1200px){.container{margin: auto;width: 30%}}
    @media(min-width:768px) and (max-width:1200px){.container{margin: auto;width: 50%}}
    /* h1{font-size: 3em} */
    /* .btn{background: #0ae;border-radius: 4px;border: 0;color: #fff;cursor: pointer;font-size: 1.5em;display: inline-block;margin: 2px 0;padding: 10px 14px 11px;width: 100%} */
    .btn{background: #0ae;border-radius: 4px;border: 0;color: #fff;cursor: pointer;font-size: 1.5em;display: inline-block;margin: 2px 0;padding: 10px 0px 11px;width: 100%;text-align: center;text-decoration: none}
    .btn:hover{background: #09d}
    .btn:active,.btn:focus{background: #08b}
    label>*{display: inline}
    form>*{display: block;margin-bottom: 10px}
    .msg{background: #def;border-left: 5px solid #59d;padding: 1.5em}
    /* .q{float: right;width: 64px;text-align: right} */
    /* .l{background: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==') no-repeat left center;background-size: 1em} */
    .table {table-layout : fixed;width: 100%}
    .table td{padding:.5em;text-align:left}
    .table tbody>:nth-child(2n-1){background:#ddd}
   </style>
</head>
<body>
  <div class="topnav">
    <h1>Webserver v1.0</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    window.addEventListener('load', onLoad);
    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage; // <-- add this line
    }
    function onOpen(event) {
      console.log('Connection opened');
    }
    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }
    function onMessage(event) {
      var state;
      if (event.data == "1"){
        state = "ON";
      }
      else{
        state = "OFF";
      }
      document.getElementById('state').innerHTML = state;
    }
    function onLoad(event) {
      initWebSocket();
      initButton();
    }
    function initButton() {
      document.getElementById('button').addEventListener('click', toggle);
    }
    function toggle(){
      websocket.send('toggle');
    }
  </script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void webserverSetup() {
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  webServer.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json;
    StaticJsonDocument<300> doc;
    doc["id"] = ESP.getChipId();
    doc["freeheap"] = ESP.getFreeHeap();
    doc["flashid"] = ESP.getFlashChipId();
    doc["flashsize"] = ESP.getFlashChipSize();
    doc["flashrealsize"] = ESP.getFlashChipRealSize();
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });
  // Send a POST request to <IP>/post with a form field message set to <message>
  webServer.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      //nothing and dont remove it
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
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
          if (strcmp((char*)data, "toggle") == 0) {
            //ledState = !ledState;
            //ws.textAll(String(ledState));
          }
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
