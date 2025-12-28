#include "Bitmaps.h"
#include "ConfigManager.h"
#include "Globals.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include "UIManager.h"
#include <Arduino.h>

unsigned long lastPlexUpdate = 0;
unsigned long lastBatteryUpdate = 0;

// Weather Variables
bool weatherEnabled = false;
char weatherCity[30] = "";
float weatherLat = 0.0;
float weatherLon = 0.0;
String weatherLocationName = "";
float currentTemp = 0.0;
int currentWeatherCode = 0;
unsigned long lastWeatherTime = 0; // ensure this is defined
bool weatherValid = false;         // Initialize

void setup() {
  // THERMAL OPTIMIZATION
  setCpuFrequencyMhz(80); // Drop to 80MHz to reduce heat (Default is 160)

  Serial.begin(115200);
  delay(100);
  Serial.println("Starting NodeMCU Plex Display...");

  // INPUTS (Rotary Encoder)
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  // Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_SW), isrEncSwitch,
                  CHANGE); // New ISR
  // SW interrupt not strictly needed if we poll fast, but better to use it or
  // poll. Let's us poll SW in loop() or use ISR. ISR ensures we catch clicks.
  // Actually, for button debouncing, polling in handleInputs is often cleaner
  // if loop is fast. But let's use ISR for consistency with previous
  // architecture. attachInterrupt(digitalPinToInterrupt(ENC_SW), isrBtn,
  // FALLING); Wait, let's keep it simple. Polling SW is easier for Long Press
  // logic. But the user system uses interrupts. Let's use Polling for SW to
  // handle Long Pres easier? No, let's use the existing ISR pattern for SW too.

  // OLED & LOGO
  u8g2.begin();
  u8g2.setContrast(128); // Reduce contrast to save display/power
  u8g2.clearBuffer();
  u8g2.drawXBMP(76, 7, 103, 50, epd_bitmap_plex_logo_boot);
  u8g2.sendBuffer();

  // CONFIG & WIFI
  WiFiManager wm;
  // Enable WiFi Power Save
  WiFi.setSleep(true);
  setupConfig(wm);

  // NTP TIME (UTC+3 for Turkey)
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("System Ready!");
  delay(2000);
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();

  // PLEX UPDATE
  // Always check Plex status (Adaptive polling)
  long interval = (currentMode == MODE_CLOCK) ? 10000 : 3000;
  if (currentMillis - lastPlexUpdate > interval) {
    getPlexData();
    lastPlexUpdate = currentMillis;
  }

  // BATTERY UPDATE
  if (currentMillis - lastBatteryUpdate > 10000) {
    readBattery();
    lastBatteryUpdate = currentMillis;
  }

  // WEATHER UPDATE (Every 30 Minutes)
  if (weatherEnabled &&
      (currentMillis - lastWeatherTime > 1800000 || lastWeatherTime == 0)) {
    getWeather();
    lastWeatherTime = currentMillis;
    if (lastWeatherTime == 0)
      lastWeatherTime = 1; // Prevent stuck at 0
  }

  // BUTTON INPUT (High Priority)
  // Poll buttons every loop iteration to prevent missed presses during display
  // refresh
  handleInputs();

  // UI LOOP (Handles Drawing Only)
  // Throttled to ~20FPS to reduce ESP32-C3 Heating (Software SPI is CPU
  // intensive)
  static unsigned long lastUiUpdate = 0;
  if (currentMillis - lastUiUpdate > 50) {
    uiLoop();
    lastUiUpdate = currentMillis;
  }

  // Power Saving Yield
  delay(10);
}
