void wifiSetup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);
  WiFi.hostname(APssid);

  if (strlen(ssid) > 0) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
  }
}
