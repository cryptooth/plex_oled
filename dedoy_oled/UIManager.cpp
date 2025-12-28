#include "UIManager.h"
#include "Bitmaps.h"
#include "Globals.h"
#include "InputManager.h" // For handleButtons call inside draw loop
#include <U8g2lib.h>
#include <time.h>

// Latch variable for swap state to prevent tearing
static bool latchShowAlbum = false;

String msToTime(long ms) {
  int totalSeconds = ms / 1000;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  String sMin = String(minutes);
  String sSec = String(seconds);
  if (seconds < 10)
    sSec = "0" + sSec;
  return sMin + ":" + sSec;
}

const uint8_t *getFont(const char *size, const char *weight) {
  String s = String(size);
  String w = String(weight);
  if (s == "small")
    return (w == "bold") ? u8g2_font_helvB10_tr : u8g2_font_helvR10_tr;
  if (s == "large")
    return (w == "bold") ? u8g2_font_helvB14_tr : u8g2_font_helvR14_tr;
  return (w == "bold") ? u8g2_font_helvB12_tr : u8g2_font_helvR12_tr;
}

void readBattery() {
  int raw = analogRead(BAT_PIN);

#ifdef ESP32
  // ESP32 (12-bit, 0-4095). Assuming x4 scale of NodeMCU values for now.
  // May need calibration.
  int minRaw = 1860;
  int maxRaw = 2600;
#else
  // ESP8266 (10-bit, 0-1024)
  int minRaw = 465;
  int maxRaw = 650;
#endif

  int p = map(raw, minRaw, maxRaw, 0, 100);
  if (p > 100)
    p = 100;
  if (p < 0)
    p = 0;
  batteryPercent = p;
}

void drawNowPlaying() {
  int screenW = 256;
  int badgeSpace = 0;

  // 1. Battery (Far Right)
  int batX = screenW - 24;
  u8g2.drawFrame(batX, 2, 18, 9);
  u8g2.drawBox(batX + 18, 4, 2, 5);
  int batLevel = (14 * batteryPercent) / 100;
  if (batLevel > 0)
    u8g2.drawBox(batX + 2, 4, batLevel, 5);
  badgeSpace += 26;

  // 2. Codec
  if (currentCodec != "") {
    String codecUpper = currentCodec;
    codecUpper.toUpperCase();
    u8g2.setFont(u8g2_font_helvB08_tr);
    int badgeW = u8g2.getStrWidth(codecUpper.c_str()) + 8;
    int badgeX = batX - badgeW - 5;
    u8g2.drawRFrame(badgeX, 0, badgeW, 13, 3);
    u8g2.drawStr(badgeX + 4, 10, codecUpper.c_str());
    badgeSpace += badgeW + 5;
  }

  // 3. Play/Pause Icon (Left of Codec/Battery)
  int iconX = screenW - badgeSpace - 12;
  if (isPlaying) {
    u8g2.drawXBMP(iconX, 2, 8, 10, icon_play);
  } else {
    u8g2.drawXBMP(iconX, 2, 8, 10, icon_pause);
  }
  badgeSpace += 15;

  // 4. Artist (Static Top)
  const uint8_t *fontArtist = getFont(artist_size, artist_weight);
  u8g2.setFont(fontArtist);

  // Show Artist always on top
  int wA = u8g2.getStrWidth(currentArtist.c_str());
  if (wA > (screenW - badgeSpace)) {
    u8g2.drawStr(-scrollCursor_Artist, 18, currentArtist.c_str());
  } else {
    u8g2.drawStr(0, 18, currentArtist.c_str());
  }

  // 5. Song / Album (Alternating Bottom)
  const uint8_t *fontSong = getFont(song_size, song_weight);
  u8g2.setFont(fontSong);

  String bottomText = currentSong;
  // Use latched state to ensure entire frame is consistent
  if (currentAlbum != "" && latchShowAlbum) {
    bottomText = currentAlbum;
  }

  int wS = u8g2.getStrWidth(bottomText.c_str());
  if (wS > 256) {
    u8g2.drawStr(-scrollCursor_Song, 42, bottomText.c_str());
  } else {
    int xCentered = (256 - wS) / 2;
    u8g2.drawStr(xCentered, 42, bottomText.c_str());
  }

  // 6. Total Time
  u8g2.setFont(u8g2_font_helvB10_tr);
  String sTotal = msToTime(currentDuration);
  int wTotal = u8g2.getStrWidth(sTotal.c_str());
  u8g2.drawStr(256 - wTotal, 62, sTotal.c_str());

  // Progress Bar
  int barW = 256 - wTotal - 6;
  if (barW > 0) {
    u8g2.drawFrame(0, 54, barW, 8);
    if (currentDuration > 0) {
      int progressPx = (currentOffset * barW) / currentDuration;
      u8g2.drawBox(2, 56, progressPx, 4);
    }
  }
}

void drawMenu() {
  // Title: "MENU" (Left Aligned, Hel vB12)
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.drawStr(0, 14, "MENU");
  u8g2.drawLine(0, 16, 45, 16);

  u8g2.setFont(u8g2_font_helvB10_tr);

  for (int i = 0; i < menuLen; i++) {
    if (abs(menuIndex - i) < 2) {
      int relativePos = i - menuIndex;
      int y = 35 + (relativePos * 16);

      String label = menuItems[i];
      String value = "";

      // Append Values for Config Items
      if (label == "Artist Font")
        value = ": " + String(artist_size);
      else if (label == "Song Font")
        value = ": " + String(song_size);
      else if (label == "Screensaver")
        value = ": " + String(screensaverTimeout / 1000) + "s";
      else if (label == "Brightness")
        value = ": " + String(map(currentBrightness, 0, 255, 0, 100)) + "%";

      if (i == menuIndex) {
        u8g2.drawStr(48, y, ">");
        u8g2.drawStr(60, y, (label + value).c_str());
      } else {
        u8g2.drawStr(60, y, (label + value).c_str());
      }
    }
  }
}

void drawInfo() {
  u8g2.setFont(u8g2_font_helvB10_tr); // Slightly smaller title
  u8g2.drawStr(0, 10, "INFO");        // Lifted up
  u8g2.drawLine(0, 12, 30, 12);

  u8g2.setFont(u8g2_font_helvR08_tr); // 8px font needs ~10px line height
  int y = 24;
  int h = 10;

  // Condense WiFi to fit nicely
  u8g2.drawStr(
      0, y,
      ("WiFi: " + WiFi.SSID() + " (" + String(WiFi.RSSI()) + "dB)").c_str());
  y += h;
  u8g2.drawStr(0, y, ("IP: " + WiFi.localIP().toString()).c_str());
  y += h;
  u8g2.drawStr(0, y,
               ("Plex: " + String(plex_ip) + ":" + String(plex_port)).c_str());
  y += h;

  String status = "Bekleniyor";
  if (playerMachineID != "")
    status = "ID: " + playerMachineID.substring(0, 8);
  u8g2.drawStr(0, y, ("Durum: " + status).c_str());

  y += h;
  // y should be around 64 here, might be tight at bottom edge
  u8g2.drawStr(0, y, ("RAM: " + String(ESP.getFreeHeap()) + "b").c_str());
}

void drawClock() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  if (now < 100000) {
    // Time not synced yet
    u8g2.setFont(u8g2_font_helvB10_tr);
    u8g2.drawStr(50, 40, "Saat Ayarlaniyor...");
    return;
  }

  // HH:MM
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", t->tm_hour, t->tm_min);

  // Date DD/MM
  char dateStr[12];
  sprintf(dateStr, "%02d/%02d", t->tm_mday, t->tm_mon + 1);

  // Draw Big Clock
  // User requested BOLDEST font
  u8g2.setFont(u8g2_font_fub42_tn); // Furta Bold 42 (Very Thick)
  int wTime = u8g2.getStrWidth(timeStr);
  int xTime = (256 - wTime) / 2;
  u8g2.drawStr(xTime, 45, timeStr);

  // Draw Date (Bottom Right)
  u8g2.setFont(u8g2_font_helvB10_tr);
  int wDate = u8g2.getStrWidth(dateStr);
  u8g2.drawStr(256 - wDate - 4, 62, dateStr);

  // Draw Weather (Bottom Left)
  if (weatherEnabled) {
    if (weatherValid) {
      String wStr = String((int)currentTemp) + "C ";

      // Simple WMO Code Mapping (English)
      if (currentWeatherCode == 0)
        wStr += "Clear";
      else if (currentWeatherCode <= 3)
        wStr += "Cloudy";
      else if (currentWeatherCode <= 48)
        wStr += "Foggy";
      else if (currentWeatherCode <= 67)
        wStr += "Rain";
      else if (currentWeatherCode <= 77)
        wStr += "Snow";
      else if (currentWeatherCode <= 82)
        wStr += "Showers";
      else if (currentWeatherCode <= 99)
        wStr += "Storm";

      u8g2.drawStr(4, 62, wStr.c_str());
    } else {
      // Show placeholder if enabled but not fetched yet
      u8g2.setFont(u8g2_font_helvR08_tr);
      u8g2.drawStr(4, 62, "...");
    }
  }
}

void uiLoop() {
  unsigned long currentMillis = millis();

  // Only scroll in Playing Mode
  if (currentMode == MODE_PLAYING && currentMillis - lastScrollTime > 50) {
    const uint8_t *fontArtist = getFont(artist_size, artist_weight);
    u8g2.setFont(fontArtist);

    // Scroll Mantigi (Artist/Album degisimini takip et)
    String topTextScroll = currentArtist;
    static bool lastWasAlbum = false;
    bool isAlbum = (currentAlbum != "" && (millis() % 8000 > 4000));

    if (isAlbum != lastWasAlbum) {
      scrollCursor_Artist = 0;
      lastWasAlbum = isAlbum;
    }
    if (isAlbum)
      topTextScroll = currentAlbum;

    int wA = u8g2.getStrWidth(topTextScroll.c_str());
    // ... (Battery icon width calc is inside draw, simplistic approach here)
    int maxScrollArtist = wA - 200; // approx
    if (maxScrollArtist > 0) {
      scrollCursor_Artist += 2;
      if (scrollCursor_Artist > wA)
        scrollCursor_Artist = -200;
    } else
      scrollCursor_Artist = 0;

    const uint8_t *fontSong = getFont(song_size, song_weight);
    u8g2.setFont(fontSong);
    int wS = u8g2.getStrWidth(currentSong.c_str());
    if (wS > 256) {
      scrollCursor_Song += 2;
      if (scrollCursor_Song > wS)
        scrollCursor_Song = -256;
    } else
      scrollCursor_Song = 0;
  } // End Scroll Logic

  if (currentMillis - lastScrollTime > 50) { // Screen Refresh Rate
    lastScrollTime = currentMillis;

    // Determine swap state once per frame
    latchShowAlbum = (millis() % 8000 > 4000);

    u8g2.clearBuffer();

    if (currentMode == MODE_PLAYING) {
      drawNowPlaying();
    } else if (currentMode == MODE_CLOCK) {
      drawClock();
    } else if (currentMode == MODE_INFO) {
      drawInfo();
    } else {
      drawMenu();
    }

    u8g2.sendBuffer();
  }
}
