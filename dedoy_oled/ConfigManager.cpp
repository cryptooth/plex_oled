#include "ConfigManager.h"
#include "Bitmaps.h"
#include "Globals.h"
#include <ArduinoJson.h>
#include <WiFiManager.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <LittleFS.h>
#else
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#endif

void loadConfig() {
  // Mount LittleFS, format if failed (important for new chips)
  if (LittleFS.begin(true)) {
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();
        DynamicJsonDocument json(1024);
        deserializeJson(json, buf.get());
        if (json.containsKey("plex_ip"))
          strcpy(plex_ip, json["plex_ip"]);
        if (json.containsKey("plex_port"))
          strcpy(plex_port, json["plex_port"]);
        if (json.containsKey("plex_token"))
          strcpy(plex_token, json["plex_token"]);
        if (json.containsKey("artist_size"))
          strcpy(artist_size, json["artist_size"]);
        if (json.containsKey("song_size"))
          strcpy(song_size, json["song_size"]);
        if (json.containsKey("screensaver_timeout"))
          screensaverTimeout = json["screensaver_timeout"];
        if (json.containsKey("brightness")) {
          currentBrightness = json["brightness"];
          u8g2.setContrast(currentBrightness); // Apply immediately
        }

        // Weather Config
        if (json.containsKey("weather_enabled"))
          weatherEnabled = json["weather_enabled"];
        if (json.containsKey("weather_city"))
          strcpy(weatherCity, json["weather_city"]);
        if (json.containsKey("weather_lat"))
          weatherLat = json["weather_lat"];
        if (json.containsKey("weather_lon"))
          weatherLon = json["weather_lon"];
        if (json.containsKey("weather_loc_name"))
          weatherLocationName = json["weather_loc_name"].as<String>();
      }
    }
  } else {
    // Failed to mount
  }
}

void saveConfig() {
  // Ensure mounted
  if (!LittleFS.begin(true))
    return;

  DynamicJsonDocument json(1024);
  json["plex_ip"] = plex_ip;
  json["plex_port"] = plex_port;
  json["plex_token"] = plex_token;
  json["artist_size"] = artist_size;
  json["song_size"] = song_size;
  json["screensaver_timeout"] = screensaverTimeout;
  json["brightness"] = currentBrightness;

  // Weather
  json["weather_enabled"] = weatherEnabled;
  json["weather_city"] = weatherCity;
  json["weather_lat"] = weatherLat;
  json["weather_lon"] = weatherLon;
  json["weather_loc_name"] = weatherLocationName;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    // Failed to open
    return;
  }
  serializeJson(json, configFile);
  configFile.close();
}

void saveConfigCallback() { shouldSaveConfig = true; }

void configModeCallback(WiFiManager *myWiFiManager) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvB10_tr);
    u8g2.drawStr(0, 20, "WiFi Setup Mode");
    u8g2.setFont(u8g2_font_helvR08_tr);
    char apInfo[50];
    sprintf(apInfo, "Net: %s", myWiFiManager->getConfigPortalSSID().c_str());
    u8g2.drawStr(0, 52, apInfo);
    u8g2.drawStr(0, 62, "IP: 192.168.4.1");
  } while (u8g2.nextPage());
}

void handleRoot() {
  String html = "<html><head><meta name='viewport' "
                "content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:sans-serif;padding:20px;background:#222;"
          "color:#fff}";
  html += "input{width:100%;padding:10px;margin:5px 0;box-sizing:border-box;}";
  html += "input[type=checkbox]{width:auto;margin-right:10px;}";
  html += "input[type=submit]{background:#e5a00d;color:black;border:none;font-"
          "weight:bold;cursor:pointer;}";
  html += ".card{background:#333;padding:20px;border-radius:8px;max-width:"
          "400px;margin:0 auto;}";
  html += "</style></head><body>";

  html += "<div class='card'><h2>Plex Control Settings</h2>";
  html += "<p>Device IP: " + WiFi.localIP().toString() + "</p>";

  html += "<form action='/save' method='POST'>";

  html += "<label>Plex Server IP:</label>";
  html += "<input type='text' name='plex_ip' value='" + String(plex_ip) + "'>";

  html += "<label>Plex Port:</label>";
  html +=
      "<input type='text' name='plex_port' value='" + String(plex_port) + "'>";

  html += "<label>Plex Token:</label>";
  html += "<input type='text' name='plex_token' value='" + String(plex_token) +
          "'>";

  html += "<hr>";

  // Weather Section
  html += "<h3>Weather (Screensaver)</h3>";

  String checked = weatherEnabled ? "checked" : "";
  html += "<label><input type='checkbox' name='weather_enabled' " + checked +
          "> Enable</label><br><br>";

  html += "<label>City Name (e.g., London, New York):</label>";
  html += "<input type='text' name='weather_city' value='" +
          String(weatherCity) + "'>";

  if (weatherLocationName != "") {
    html += "<p style='color:#aaa;font-size:12px;'>Last Found: " +
            weatherLocationName + "</p>";
  }

  html += "<hr>";
  html += "<label>Artist Font (small/medium/large):</label>";
  html += "<input type='text' name='artist_size' value='" +
          String(artist_size) + "'>";

  html += "<label>Song Font:</label>";
  html +=
      "<input type='text' name='song_size' value='" + String(song_size) + "'>";

  html += "<label>Screensaver Time (ms):</label>";
  html += "<input type='number' name='screensaver_timeout' value='" +
          String(screensaverTimeout) + "'>";

  html += "<input type='submit' value='SAVE & REBOOT'>";
  html += "</form></div></body></html>";

  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("plex_ip"))
    strcpy(plex_ip, server.arg("plex_ip").c_str());
  if (server.hasArg("plex_port"))
    strcpy(plex_port, server.arg("plex_port").c_str());
  if (server.hasArg("plex_token"))
    strcpy(plex_token, server.arg("plex_token").c_str());
  if (server.hasArg("artist_size"))
    strcpy(artist_size, server.arg("artist_size").c_str());
  if (server.hasArg("song_size"))
    strcpy(song_size, server.arg("song_size").c_str());
  if (server.hasArg("screensaver_timeout"))
    screensaverTimeout = server.arg("screensaver_timeout").toInt();

  // Weather Logic
  if (server.hasArg("weather_enabled")) {
    weatherEnabled = true;
  } else {
    weatherEnabled = false;
  }

  if (server.hasArg("weather_city")) {
    String newCity = server.arg("weather_city");
    strcpy(weatherCity, newCity.c_str());

    // Perform Geocoding if enabled and city present
    if (weatherEnabled && newCity.length() > 2) {
      HTTPClient http;
      WiFiClient client;

      // Simple URL Encoder for critical chars
      String encodedCity = "";
      for (int i = 0; i < newCity.length(); i++) {
        char c = newCity.charAt(i);
        if (c == ' ')
          encodedCity += "%20";
        else if (c == 0xC5)
          encodedCity += "S"; // Ş -> S (approximation for API)
        else if (c == 0x9E)
          encodedCity += "s"; // ş -> s
        else if (c == 0xC4) {
          encodedCity += String(c);
        } else
          encodedCity += String(c);
      }

      // Better Strategy: Strict ASCII mapping for reliability
      // OpenMeteo is fuzzy search, so "Sile" finds "Şile".
      encodedCity = "";
      for (int i = 0; i < newCity.length(); i++) {
        unsigned char c = (unsigned char)newCity[i];
        if (c == ' ')
          encodedCity += "%20";
        else
          encodedCity += (char)c;
      }

      // ACTUALLY: The easiest fix for "Şile" failing is if we blindly send
      // UTF-8 bytes in the URL. OpenMeteo likely needs percent encoding for
      // those bytes. Let's use a proper simple url encoder.
      encodedCity = "";
      char hex[4];
      for (size_t i = 0; i < newCity.length(); i++) {
        unsigned char c = (unsigned char)newCity[i];
        if (isalnum(c)) {
          encodedCity += (char)c;
        } else {
          sprintf(hex, "%%%02X", c);
          encodedCity += hex;
        }
      }

      String geoUrl =
          "http://geocoding-api.open-meteo.com/v1/search?name=" + encodedCity +
          "&count=1&language=en&format=json"; // Changing lang to EN for API too
                                              // if possible?
      // Keeping 'en' is safer for general english usage

      if (http.begin(client, geoUrl)) {
        int code = http.GET();
        if (code == 200) {
          DynamicJsonDocument doc(2048);
          deserializeJson(doc, http.getString());
          if (doc.containsKey("results") && doc["results"].size() > 0) {
            weatherLat = doc["results"][0]["latitude"];
            weatherLon = doc["results"][0]["longitude"];
            String name = doc["results"][0]["name"].as<String>();
            String country = doc["results"][0]["country"].as<String>();
            weatherLocationName = name + ", " + country;
          } else {
            weatherLocationName = "Not Found!";
          }
        }
        http.end();
      }
    }
  }

  saveConfig();

  String html = "<html><body "
                "style='background:#222;color:#fff;text-align:center;padding:"
                "50px;font-family:sans-serif;'>";
  html += "<h1>Saved!</h1>";

  if (weatherEnabled) {
    html += "<p>Weather Location:</p>";
    html += "<h2>" + weatherLocationName + "</h2>";
  }

  html += "<p>Device is rebooting...</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);

  delay(1000);
  ESP.restart();
}

void setupConfig(WiFiManager &wm) {
  loadConfig();
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setAPCallback(configModeCallback);

  // Custom Parameters
  WiFiManagerParameter custom_plex_ip("ip", "Plex IP", plex_ip, 40);
  WiFiManagerParameter custom_plex_port("port", "Plex Port", plex_port, 6);
  WiFiManagerParameter custom_plex_token("token", "Plex Token", plex_token, 40);

  wm.addParameter(&custom_plex_ip);
  wm.addParameter(&custom_plex_port);
  wm.addParameter(&custom_plex_token);

  // Show "Connecting" status on OLED (Full Buffer)
  u8g2.clearBuffer();

  // Draw WiFi Icon (Left) - 59x50px
  u8g2.drawXBMP(5, 7, 59, 50, epd_bitmap_wifi_symbol);

  // Draw Text (Right of Icon, Left Aligned)
  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.drawStr(70, 28, "Connecting");

  u8g2.setFont(u8g2_font_helvR08_tr);
  u8g2.drawStr(70, 45, "Please wait...");

  u8g2.sendBuffer();

  if (!wm.autoConnect("PlexDisplay-Setup")) {
    ESP.restart();
  }

  strcpy(plex_ip, custom_plex_ip.getValue());
  strcpy(plex_port, custom_plex_port.getValue());
  strcpy(plex_token, custom_plex_token.getValue());

  if (shouldSaveConfig)
    saveConfig();

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}
