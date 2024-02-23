// network
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "network.h"
// DNSServer dnsServer;

/* Soft AP network parameters */
// IPAddress apIP(192, 168, 4, 1);
// IPAddress subnet(255, 255, 255, 0);
// IPAddress gateway(192, 168, 4, 1);

void networkSetup()
{
  static byte c = 0;
  static byte l = 30; // wifi connect timeout limit

  WiFi.mode(WIFI_AP_STA);

  // Serial.print("Setting soft-AP configuration ... ");
  // Serial.println(WiFi.softAPConfig(apIP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.println();

  Serial.print("Starting Soft-AP ... ");
  Serial.println(WiFi.softAP(APssid, APpassword) ? "Success" : "Failed!");

  Serial.print("Soft-AP IP address: ");
  Serial.println(WiFi.softAPIP());

  WiFi.hostname(APssid);

  // dnsServer.setTTL(300);
  // dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  // dnsServer.start(53, "*", apIP);
  // Serial.println("DNS started!");

  if (strlen(ssid) > 0)
  {
    Serial.print("Connecting to " + String(ssid) + " ");
    WiFi.begin(ssid, password);
    while (c < l)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println(" Success");

        Serial.print("Local IP address: ");
        Serial.println(WiFi.localIP());
        break;
      }
      delay(500);
      Serial.print(".");
      c++;
    }
    if (c >= l)
    {
      Serial.println(" Failed!");
    }
  }

  Serial.print("Starting MDNS (tracker.local) ... ");
  Serial.println(MDNS.begin("tracker") ? "Success." : "Failed!");
  MDNS.addService("http", "tcp", 80);
}

void networkLoop()
{
  // dns server
  // dnsServer.processNextRequest();
  MDNS.update();
}
