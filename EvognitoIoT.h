#ifndef EVOGNITO_IOT_H
#define EVOGNITO_IOT_H

#include <Preferences.h>
#include <SPIFFS.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServerBase = ESP8266WebServer;
#else
#include <WiFi.h>
#include <WebServer.h>
using WebServerBase = WebServer;
#endif

class EvognitoIoT
{
public:
  // Constructors
  EvognitoIoT();                                           // Default: SSID "Evognito_AP", password kosong
  EvognitoIoT(const char *apSsid);                         // Custom SSID, password kosong
  EvognitoIoT(const char *apSsid, const char *apPassword); // Custom SSID dan password

  void begin();           // Memulai koneksi WiFi atau masuk mode AP jika gagal
  void loop();            // Wajib dipanggil di loop utama
  bool isConnected();     // Mengecek koneksi ke WiFi
  IPAddress getLocalIP(); // Mendapatkan IP lokal

private:
  const char *_apSsid;
  const char *_apPassword;
  WebServerBase _server;
  Preferences _preferences;

  void setupAP();
  void setupWebPortal();
  bool connectToSavedWiFi();
  void handleRoot();
  void handleSave();
};

#endif
