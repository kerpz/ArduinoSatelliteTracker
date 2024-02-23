// ntp
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "ntp.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void ntpSetup()
{
  Serial.print("Starting NTP Client ... ");
  timeClient.begin();
  // timeClient.setTimeOffset(28800); // GMT +8 = 8 x 3600
  Serial.println("Done");
}

void ntpLoop()
{
  timeClient.update();
  epoch = timeClient.getEpochTime();
}
