#define SERVER_NAME http://www.celestrak.com/satcat/tle.php?CATNR=32382

void getTLE () {
  HTTPClient http;
  http.begin(SERVER_NAME);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
  }
  http.end();
}
