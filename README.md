# ESP32 Plex OLED Display & Controller

**A custom-built Wi-Fi remote and Now Playing display for Plex, powered by ESP32-C3.**

![Boot Screen](https://raw.githubusercontent.com/cryptooth/plex_oled/main/demo.jpg) <!-- You can add a photo later -->

## 🌟 Features
*   **Now Playing Display:** Shows Artist, Song, and Playback Status in real-time.
*   **Rotary Control:** 
    *   **Turn:** Adjust Volume / Scroll Menus.
    *   **Click:** Play / Pause.
    *   **Double Click:** Next Track.
*   **Plex Integration:** Connects directly to PMS server via API (No intermediary server needed).
*   **Weather Screensaver:** Displays live temperature and weather conditions (via OpenMeteo) when idle.
*   **Web Config Portal:** Easily configure WiFi, Plex IP/Token, and Weather settings via browser.
*   **Battery Power:** Optimized for LiPo battery usage with Deep Sleep and CPU throttling (80MHz).

## 🛠 Hardware Required
*   **Microcontroller:** ESP32-C3 SuperMini (or generic ESP32)
*   **Display:** 3.12" OLED (SSD1322 Driver, 256x64 pixels) via SPI
*   **Input:** Rotary Encoder (KY-040 type) with Push Button
*   **Battery:** 3.7V LiPo (Optional)

### Wiring Diagram (ESP32-C3 SuperMini)

| Component | Pin Name | ESP32 GPIO |
| :--- | :--- | :--- |
| **OLED** | SCLK (Clock) | **GPIO 4** |
| | SDIN (Data) | **GPIO 6** |
| | CS (Chip Select) | **GPIO 7** |
| | DC (Data/Cmd) | **GND** (3-Wire SPI) |
| | RST (Reset) | **GPIO 2** |
| **Encoder** | CLK | **GPIO 1** |
| | DT | **GPIO 3** |
| | SW (Button) | **GPIO 5** |
| **Battery** | V-Sense | **GPIO 0** (Analog) |

> **Note:** The OLED is configured for **3-Wire SPI** mode (BS0=1, BS1=0 on the back jumpers).

## 🚀 Setup Guide

### 1. Installation
1.  Clone this repository.
2.  Open `dedoy_oled/dedoy_oled.ino` in Arduino IDE.
3.  Install Required Libraries:
    *   **U8g2** by oliver
    *   **ArduinoJson** by Benoit Blanchon
    *   **WiFiManager** by tzapu
4.  Select Board: **ESP32C3 Dev Module**.
5.  **Upload** the sketch.

### 2. Configuration
1.  On first boot, the device will create a **WiFi Hotspot** named `PlexDisplay-Setup`.
2.  Connect to it with your phone/laptop.
3.  A portal should open automatically (or go to `192.168.4.1`).
4.  **Enter Credentials:**
    *   **WiFi SSID/Password**
    *   **Plex Server IP & Port** (e.g., 192.168.1.5 : 32400)
    *   **Plex Token** (Find this in your Plex XML/XML)
    *   **(Optional) Enable Weather:** Enter your City Name.
5.  Click **Save & Reboot**.

## 🎮 Controls
*   **Main Screen:**
    *   **Rotate CW:** Next Track
    *   **Rotate CCW:** Previous Track
    *   **Click:** Play / Pause
    *   **Long Press:** Open/Close Menu
*   **Menu:**
    *   **Rotate:** Scroll Items
    *   **Click:** Select Item

## 📜 License
MIT License. Open source and free to use.
