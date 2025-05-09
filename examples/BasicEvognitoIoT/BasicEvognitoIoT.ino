#include <EvognitoIoT.h>

// Pilih salah satu dari baris ini sesuai kebutuhan:
// EvognitoIoT wifiManager; // Default SSID "Evognito_AP", password kosong
EvognitoIoT wifiManager("EvognitoSetup"); // SSID custom tanpa password
// EvognitoIoT wifiManager("EvognitoSetup", "12345678"); // SSID dan password custom

void setup()
{
  Serial.begin(115200);
  wifiManager.begin();
}

void loop()
{
  if (!wifiManager.isConnected())
  {
    // Jalankan logika jika tidak terkoneksi WiFi
    Serial.println("Gagal terkoneksi...");
    delay(1000);
    return;
  }

  Serial.println(wifiManager.getLocalIP());
  Serial.println("Yeayyy...");
  delay(500);
  wifiManager.loop(); // WAJIB DI AKHIR
}
