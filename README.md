# EvognitoIoT

**EvognitoIoT** adalah library WiFi Manager sederhana untuk ESP32 yang menyediakan captive portal (halaman login otomatis) untuk menghubungkan perangkat ke WiFi dengan mudah.

---

## ✨ Fitur

- Captive portal otomatis saat WiFi belum dikonfigurasi.
- Menyimpan SSID dan password menggunakan Preferences (non-volatile).
- Menyediakan halaman HTML untuk:
  - Menampilkan daftar jaringan WiFi
  - Mengisi kredensial WiFi
  - Melihat status koneksi
  - Reset dan reboot perangkat
- Mudah digunakan, cocok untuk proyek IoT berbasis ESP32.

---

## 🚀 Cara Penggunaan

### 1. Tambahkan library ke project kamu

#### PlatformIO:

Tambahkan ke `platformio.ini`:

```ini
lib_deps =
  https://github.com/evognito/EvognitoIoT.git
```

## License

This project is licensed under the [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/).  
You are free to use and modify this library for personal and educational purposes, but **not for commercial use**.
