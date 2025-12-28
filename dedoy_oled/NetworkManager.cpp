#include "NetworkManager.h"
#include "Globals.h"
#include "NetworkManager.h"
#include <ArduinoJson.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#else
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

void sendPlexCommand(String command) {
  if (playerMachineID == "") {
    Serial.println("Error: No Player Machine ID. Cannot Send Command.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  // Determine Media Type for Command
  String targetType = "video"; // Default
  if (currentType == "track") {
    targetType = "music";
  }

  // ---------------------------------------------------------
  // METHOD 1: Server Relay (Preferred for Web/Remote)
  // ---------------------------------------------------------
  // Fix: Removed "&type=" as it causes 404s on some players
  String url = "http://" + String(plex_ip) + ":" + String(plex_port) +
               "/player/playback/" + command +
               "?machineIdentifier=" + playerMachineID;

  Serial.println("Try 1 (Relay): " + url);

  if (http.begin(client, url)) {
    http.addHeader("X-Plex-Token", plex_token);
    http.addHeader("X-Plex-Target-Client-Identifier", playerMachineID);

    // Mimic real client to avoid "Unknown Client" rejection
    http.addHeader("X-Plex-Client-Identifier",
                   "ESP32-Controller-V1"); // Unique ID
    http.addHeader("X-Plex-Product",
                   "Plex for Android"); // Spoofing standard client
    http.addHeader("X-Plex-Version", "9.0.0");
    http.addHeader("X-Plex-Platform", "Android");
    http.addHeader("X-Plex-Device", "ESP32");

    int httpCode = http.GET();
    if (httpCode == 200) {
      Serial.println("[HTTP] Relay Success (200)");
      http.end();
      return; // Success!
    } else {
      Serial.printf("[HTTP] Relay Failed (%d). Trying Direct...\n", httpCode);
      Serial.println("[HTTP] Response: " + http.getString());
    }
    http.end();
  }

  // ---------------------------------------------------------
  // METHOD 2: Direct IP (Fallback for Native Apps/Open Ports)
  // ---------------------------------------------------------
  // Web Browsers usually BLOCK port 32400 or don't listen.
  // But apps like Plexamp or PMP might accept this.
  if (playerIP != "" && playerPort != "") {
    // Fix: Removed type param here too
    String directUrl =
        "http://" + playerIP + ":" + playerPort + "/player/playback/" + command;

    Serial.println("Try 2 (Direct): " + directUrl);

    if (http.begin(client, directUrl)) {
      http.addHeader("X-Plex-Token", plex_token);
      http.addHeader("X-Plex-Client-Identifier", "ESP32-Controller-V1");
      http.addHeader("X-Plex-Product", "Plex for Android");
      http.addHeader("X-Plex-Version", "9.0.0");

      // Timeout short for direct because bad IPs hang
      http.setTimeout(1500);

      int directCode = http.GET();
      if (directCode == 200) {
        Serial.println("[HTTP] Direct IP Success (200)");
      } else {
        Serial.printf("[HTTP] Direct IP Failed (%d)\n", directCode);
      }
      http.end();
    }
  } else {
    Serial.print("Direct IP info missing. IP: '");
    Serial.print(playerIP);
    Serial.print("', Port: '");
    Serial.print(playerPort);
    Serial.println("'");
  }
}

void getPlexData() {
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + String(plex_ip) + ":" + String(plex_port) +
               "/status/sessions";

  if (http.begin(client, url)) {
    http.addHeader("X-Plex-Token", plex_token);
    http.addHeader("Accept", "application/json");
    http.setTimeout(2000);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getString());
      JsonArray metadata = doc["MediaContainer"]["Metadata"];

      if (metadata.size() > 0) {
        // PRIORITY LOGIC:
        // Search through ALL sessions.
        // If we find a "playing" session, we take it.
        // If multiple are playing, we take the LAST one in the list (newest).
        int activeIndex = 0;
        bool foundPlaying = false;

        for (int i = 0; i < metadata.size(); i++) {
          String state = metadata[i]["Player"]["state"].as<String>();
          if (state == "playing") {
            activeIndex = i;
            foundPlaying = true;
            // No break -> keep looking for later (newer) playing sessions
          }
        }
        // If none playing, we stick with 0 (first paused item)

        JsonObject media = metadata[activeIndex];
        currentType = media["type"].as<String>();

        // Player Info Parsing
        JsonObject player = media["Player"];
        String state = player["state"].as<String>();

        if (player.containsKey("machineIdentifier")) {
          String newID = player["machineIdentifier"].as<String>();
          if (newID != playerMachineID && newID != "") {
            Serial.print("New Player Detected: ");
            Serial.print(player["title"].as<String>());
            Serial.print(" [");
            Serial.print(newID);
            Serial.println("]");
            playerMachineID = newID;
          }

          playerIP = player["address"].as<String>();
          if (player.containsKey("port")) {
            playerPort = player["port"].as<String>();
          } else {
            playerPort = "";
          }
          if (playerPort == "null")
            playerPort = "";
        }

        isPlaying = (state == "playing");
        currentCodec = "";
        if (media.containsKey("Media")) {
          JsonArray mediaParams = media["Media"];
          if (mediaParams.size() > 0) {
            currentCodec = mediaParams[0]["audioCodec"].as<String>();
            if (currentCodec == "null")
              currentCodec = "";
            if (currentCodec == "")
              currentCodec = mediaParams[0]["container"].as<String>();
          }
        }

        if (currentType == "track") {
          currentArtist = media["grandparentTitle"].as<String>();
          currentAlbum = media["parentTitle"].as<String>();
          currentSong = media["title"].as<String>();
        } else {
          currentArtist = "Film / Dizi";
          currentAlbum = "";
          currentSong = media["title"].as<String>();
        }
        currentOffset = media["viewOffset"];
        currentDuration = media["duration"];

        // Update timestamp ONLY if playing.
        if (state == "playing") {
          lastActiveDataTime = millis();
        }

        // Auto-Wake from Clock if Playing
        if (currentMode == MODE_CLOCK && state == "playing")
          currentMode = MODE_PLAYING;

        // Timeout Logic: Only go to Clock if NOT playing AND NOT in Menu
        // Timeout Logic: Only go to Clock if NOT playing AND NOT in Menu/Info
        if (state != "playing" &&
            (millis() - lastActiveDataTime > screensaverTimeout)) {
          if (currentMode != MODE_MENU && currentMode != MODE_INFO) {
            currentMode = MODE_CLOCK;
          }
        }

      } else {
        // No Data / Idle
        if (millis() - lastActiveDataTime > screensaverTimeout) {
          if (currentMode != MODE_MENU &&
              currentMode != MODE_INFO) { // Protect Menu & Info
            currentArtist = "Plex Ready";
            currentAlbum = "";
            currentSong = "Waiting...";
            currentDuration = 0;
            currentCodec = "";
            isPlaying = false;
            // Do NOT clear machineID so we can still play/pause the last device

            currentMode = MODE_CLOCK;
          }
        }
      }
    }
  }
  http.end();
}

void getWeather() {
  if (!weatherEnabled)
    return;
  if (weatherLat == 0.0 && weatherLon == 0.0)
    return;

  WiFiClient client;
  HTTPClient http;

  // OpenMeteo URL (Free, No Key)
  String url =
      "http://api.open-meteo.com/v1/forecast?latitude=" + String(weatherLat) +
      "&longitude=" + String(weatherLon) +
      "&current_weather=true&timezone=auto";

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == 200) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getString());

      JsonObject current = doc["current_weather"];
      if (current.containsKey("temperature")) {
        currentTemp = current["temperature"];
        currentWeatherCode = current["weathercode"];
        lastWeatherTime = millis();
        weatherValid = true; // Data is fresh and confirmed
        // Serial.printf("Weather: Temp=%.1f Code=%d\n", currentTemp,
        // currentWeatherCode);
      }
    }
    http.end();
  }
}
