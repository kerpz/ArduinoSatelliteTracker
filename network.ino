#define APssid "SatelliteTracker-AP2"
#define APpassword  "12345678"

DNSServer dnsServer;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

void networkSetup() {
  static byte c = 0;
  static byte l = 30; // wifi connect timeout limit

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);
  WiFi.hostname(APssid);

  dnsServer.start(53, "*", apIP);

  if (strlen(ssid) > 0) {
    Serial.println();
    Serial.println("Connecting to " + String(ssid));
    WiFi.begin(ssid, password);
    while (c < l) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected");
        break;
      }
      delay(500);
      Serial.print(".");
      c++;
    }
    if (c >= l) {
      Serial.println();
      Serial.println("WiFi failed to connect!");
      //Serial.println(WiFi.SSID());
      //Serial.println(WiFi.softAPIP().toString());
    }
  }
}

void networkLoop() {
  // dns server
  dnsServer.processNextRequest();
}
