WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void ntpSetup() {
  timeClient.begin();
}

void ntpLoop() {
  timeClient.update();
  epochTime = timeClient.getEpochTime();
}
