// network
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

#include "network.h"

DNSServer dnsServer;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
// IPAddress subnet(255, 255, 255, 0);
// IPAddress gateway(192, 168, 4, 1);

uint16_t wifi_error;

void networkSetup()
{
  Serial.println();

  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(APPCODE);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event)
                                 {
    Serial.println("[WiFi] Disconnected (event)");
    Serial.print("[WiFi] Reason: ");
    Serial.println(event.reason); 
    if (wifi_error < 65500) wifi_error++; });

  WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event)
                          {
    Serial.println("[WiFi] Connected / Reconnected (event)");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP()); });

  if (sta_enable)
  {
    Serial.print("Connecting to " + String(sta_ssid) + " ");
    WiFi.begin(sta_ssid, sta_key);
  }
  else
  {
    Serial.println("AP mode only!");
  }

  Serial.print("Starting Soft-AP (" + String(ap_ssid) + ") ... ");
  Serial.println(WiFi.softAP(ap_ssid, ap_key) ? "Success" : "Failed!");

  Serial.print("Soft-AP IP address: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("Starting MDNS (" + String(APPCODE) + ".local) ... ");
  Serial.println(MDNS.begin(APPCODE) ? "Success." : "Failed!");
  MDNS.addService("http", "tcp", 80);

  Serial.print("Starting DNS (" + String(APPCODE) + ") ... ");
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "*", apIP);
  Serial.println("DNS started!");
}

void networkLoop()
{
  dnsServer.processNextRequest();
  MDNS.update();
}
