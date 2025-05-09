#include "EvognitoIoT.h"
#include <rom/rtc.h>
#include <DNSServer.h>

EvognitoIoT::EvognitoIoT()
    : _apSsid("Evognito_AP"), _apPassword(""), _server(80) {} // Default: SSID "Evognito_AP", password kosong

EvognitoIoT::EvognitoIoT(const char *apSsid)
    : _apSsid(apSsid), _apPassword(""), _server(80) {} // Custom SSID, password kosong

EvognitoIoT::EvognitoIoT(const char *apSsid, const char *apPassword)
    : _apSsid(apSsid), _apPassword(apPassword), _server(80) {} // Custom SSID dan password

DNSServer dnsServer;

void EvognitoIoT::begin()
{
  if (!connectToSavedWiFi())
  {
    Serial.println("[EvognitoIoT] Tidak ditemukan WiFi tersimpan. Memulai mode AP.");
    delay(200);
    setupAP();
    setupWebPortal();

    // Blok program utama sampai koneksi berhasil
    Serial.println("[EvognitoIoT] Menunggu pengguna mengatur WiFi...");
    while (!isConnected())
    {
      dnsServer.processNextRequest();
      _server.handleClient();
      delay(100);
    }

    Serial.println("[EvognitoIoT] WiFi berhasil diatur dan terkoneksi.");
  }
  else
  {
    Serial.println("[EvognitoIoT] Tersambung ke WiFi tersimpan.");
    setupWebPortal();
  }
}

void EvognitoIoT::loop()
{
  dnsServer.processNextRequest(); // Handle DNS requests (Captive Portal)
  _server.handleClient();         // Handle HTTP requests (Web server)
}

bool EvognitoIoT::isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

IPAddress EvognitoIoT::getLocalIP()
{
  return WiFi.localIP();
}

bool EvognitoIoT::connectToSavedWiFi()
{
  _preferences.begin("wifi", true);
  String ssid = _preferences.getString("ssid", "");
  String password = _preferences.getString("password", "");
  _preferences.end();

  if (ssid == "")
  {
    Serial.println("[EvognitoIoT] Tidak ada SSID tersimpan.");
    return false;
  }

  Serial.print("[EvognitoIoT] Mencoba terhubung ke SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("[EvognitoIoT] Berhasil terhubung ke WiFi!");
      return true;
    }
    delay(500);
  }
  Serial.println("[EvognitoIoT] Gagal terhubung ke WiFi.");
  return false;
}

void EvognitoIoT::setupAP()
{
  Serial.print("[EvognitoIoT] Mengaktifkan Access Point: ");
  Serial.println(_apSsid);

#ifdef ESP8266
  WiFi.softAP(_apSsid, _apPassword);
#else
  WiFi.softAP(_apSsid, _apPassword);
#endif
  // Set IP Address for the AP Mode (192.168.24.1)
  WiFi.softAPConfig(IPAddress(192, 168, 24, 1), IPAddress(192, 168, 24, 1), IPAddress(255, 255, 255, 0));

  // Set up the DNS server to redirect all requests to the ESP32
  dnsServer.start(53, "*", IPAddress(192, 168, 24, 1));

  Serial.print("[EvognitoIoT] AP IP Address: ");
  Serial.println("192.168.24.1");
}

void EvognitoIoT::setupWebPortal()
{
  String style = "<style>"
                 "body{font-family:Arial,sans-serif;background:#f4f4f4;padding:20px;margin:0;}"
                 "h2{color:#333;}"
                 "form{margin-top:20px;}"
                 "input[type='text'],input[type='password']{font-size:1em;margin:5px 0;padding:8px;width:100%;max-width:300px;box-sizing:border-box;}"
                 "input[type='submit']{background:#007BFF;color:white;border:none;border-radius:5px;padding:10px 20px;cursor:pointer;margin-top:10px;}"
                 "input[type='submit']:hover{background:#0056b3;}"
                 "input[type='radio']{margin-right:10px;}"
                 "ul{list-style:none;padding:0;}"
                 "li{margin:10px 0;}"
                 "a{display:inline-block;margin-top:15px;color:#007BFF;text-decoration:none;}"
                 "a:hover{text-decoration:underline;}"
                 "label{display:block;margin-top:10px;font-weight:bold;}"
                 "p{margin:10px 0;}"
                 "</style>";

  _server.on("/", [this, style]()
             {
                   String html = "<!DOCTYPE html><html><head><title>Evognito Portal</title>" + style + "</head><body>";
                   html += "<h2>Evognito WiFi Portal</h2><ul>";
                   html += "<li><a href='/connect'>Connect to WiFi</a></li>";
                   html += "<li><a href='/info'>Device Info</a></li>";
                   html += "<li><a href='/reset'>Reset WiFi</a></li>";
                   html += "<li><a href='/reboot'>Reboot Device</a></li>";
                   html += "</ul></body></html>";
                   _server.send(200, "text/html", html); });

  // ⛓️ Connect Page with Available Networks
  _server.on("/connect", [this, style]()
             {

  Serial.println("[EvognitoIoT] Memindai jaringan WiFi...");
  int n = WiFi.scanNetworks();
  Serial.printf("[EvognitoIoT] Ditemukan %d jaringan WiFi.\n", n);

  String html = "<!DOCTYPE html><html><head><title>Connect WiFi</title>" + style + "</head><body>";
  html += "<h2>Available WiFi Networks</h2><form method='POST' action='/save'>";
  if (n == 0) {
    html += "<p>No WiFi networks found.</p>";
  } else {
    for (int i = 0; i < n; ++i) {
      int quality = map(WiFi.RSSI(i), -100, -50, 0, 100);
      html += "<label><input type='radio' name='ssid' value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(quality) + "%)</label>";
    }
  }
  html += "<label>Password:</label><input type='password' name='password'>";
  html += "<input type='submit' value='Connect'></form><a href='/'>Back</a></body></html>";
  _server.send(200, "text/html", html); });

  // ⚙️ Settings Page (visible after connected too)
  _server.on("/settings", [this, style]()
             {
    _preferences.begin("wifi", true);
    String savedSsid = _preferences.getString("ssid", "");
    _preferences.end();
  
    String html = "<!DOCTYPE html><html><head><title>WiFi Settings</title>" + style + "</head><body>";
    html += "<h2>WiFi Settings</h2><form action='/save' method='post'>";
    html += "<label>SSID:</label><input name='ssid' value='" + savedSsid + "'>";
    html += "<label>Password:</label><input name='password' type='password'>";
    html += "<input type='submit' value='Save & Connect'></form><a href='/'>Back</a></body></html>";
    _server.send(200, "text/html", html); });

  _server.on("/save", HTTP_POST, [this]()
             {
  String ssid = _server.arg("ssid");
  String password = _server.arg("password");

  Serial.println("[EvognitoIoT] Menyimpan kredensial WiFi...");
  Serial.println("[EvognitoIoT] SSID: " + ssid);
  Serial.println("[EvognitoIoT] Password: " + password);
  
  _preferences.begin("wifi", false);
  _preferences.putString("ssid", ssid);
  _preferences.putString("password", password);
  _preferences.end();
  
  Serial.println("[EvognitoIoT] Restart dalam 2 detik...");

  _server.send(200, "text/html", "<h2>Connecting to WiFi...</h2><p>SSID: " + ssid + "</p><p>Restarting device...</p>");
  delay(2000);
  ESP.restart(); });

  _server.on("/info", [this, style]()
             {
      uint32_t chipId = (uint32_t)(ESP.getEfuseMac() >> 32);
    
      float temperature = temperatureRead(); // Baca suhu internal chip
    
      String html = style;
      html += "<h2>Device Info</h2>";
      html += "<ul>";
      html += "<li><b>Hostname:</b> " + String(WiFi.getHostname()) + "</li>";
      html += "<li><b>IP Address:</b> " + WiFi.localIP().toString() + "</li>";
      html += "<li><b>SSID:</b> " + WiFi.SSID() + "</li>";
      html += "<li><b>Status:</b> " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "</li>";
      html += "<li><b>MAC Address:</b> " + WiFi.macAddress() + "</li>";
      html += "<li><b>Chip ID:</b> " + String(chipId, HEX) + "</li>";
      html += "<li><b>Free Heap:</b> " + String(ESP.getFreeHeap()) + " bytes</li>";
      html += "<li><b>Total Heap:</b> " + String(ESP.getHeapSize()) + " bytes</li>";
      html += "<li><b>Flash Size:</b> " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
    
      if (SPIFFS.begin(true)) {
        html += "<li><b>SPIFFS Used:</b> " + String(SPIFFS.usedBytes() / 1024) + " KB / " + String(SPIFFS.totalBytes() / 1024) + " KB</li>";
      } else {
        html += "<li><b>SPIFFS:</b> Not Mounted</li>";
      }
    
      html += "<li><b>SDK Version:</b> " + String(ESP.getSdkVersion()) + "</li>";
      html += "<li><b>Temperature:</b> " + String(temperature, 2) + " °C</li>";
      html += "</ul>";
      html += "<a href='/'>Back</a>";
    
      _server.send(200, "text/html", html); });

  _server.on("/reset", [this, style]()
             {
        _preferences.begin("wifi", false);
        _preferences.clear();
        _preferences.end();
      
        String html = "<!DOCTYPE html><html><head><title>Reset WiFi</title>" + style + "</head><body>";
        html += "<h2>WiFi reset successful.</h2><p>Device will restart...</p></body></html>";
        _server.send(200, "text/html", html);
        delay(2000);
        ESP.restart(); });

  _server.on("/reboot", [this, style]()
             {
          String html = "<!DOCTYPE html><html><head><title>Rebooting</title>" + style + "</head><body>";
          html += "<h2>Rebooting...</h2></body></html>";
          _server.send(200, "text/html", html);
          delay(2000);
          ESP.restart(); });

  _server.begin();
  return;
}

void EvognitoIoT::handleRoot()
{
  String html = "<h2>Evognito WiFi Setup</h2><form action='/save' method='post'>";
  html += "SSID: <input name='ssid' value='" + _preferences.getString("ssid", "Evognito_AP") + "'><br>";
  html += "Password: <input name='password' type='password'><br>";
  html += "<input type='submit' value='Save & Connect'></form>";
  _server.send(200, "text/html", html);
}

void EvognitoIoT::handleSave()
{
  String ssid = _server.arg("ssid");
  String password = _server.arg("password");

  if (ssid == "")
  {
    ssid = "Evognito_AP"; // Default SSID if not set
  }

  _preferences.begin("wifi", false);
  _preferences.putString("ssid", ssid);
  _preferences.putString("password", password);
  _preferences.end();

  _server.send(200, "text/html", "<h2>Settings Saved. Rebooting...</h2>");
  delay(2000);
  ESP.restart();
}
