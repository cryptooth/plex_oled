#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

#ifdef ESP32
#include <WebServer.h>
#include <WiFi.h>
#else
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#endif

#include <U8g2lib.h>

// ==========================================
// PIN DEFINITIONS (ESP32-C3 vs NodeMCU)
// ==========================================
#ifdef ESP32
// ESP32-C3 SuperMini
#define ENC_CLK 1 // Changed from 8 (Strap) to 1 (Safe)
#define ENC_DT 3  // Changed from 9 (Strap) to 3 (Safe)
#define ENC_SW 5  // Moved from 10 (Flash Pin) to 5 (Safe)
#define BAT_PIN 0 // GPIO0
#else
// NodeMCU (Legacy - Button Mapping for reference)
// Keeping buttons for fallback or defining encoder pins here if needed
#define ENC_CLK 12 // D6
#define ENC_DT 14  // D5
#define ENC_SW 2   // D4
#define BAT_PIN A0
#endif

// ==========================================
// OBJECTS
// ==========================================
// 3-Wire SPI (No DC Pin) - Matches User's Jumper Config
extern U8G2_SSD1322_NHD_256X64_F_3W_SW_SPI u8g2;

#ifdef ESP32
extern WebServer server;
#else
extern ESP8266WebServer server;
#endif

// ==========================================
// CONFIG VARIABLES
// ==========================================
extern char plex_ip[40];
extern char plex_port[6];
extern char plex_token[40];

extern char artist_size[10];
extern char artist_weight[10];
extern char song_size[10];
extern char song_weight[10];

// ==========================================
// STATE VARIABLES
// ==========================================
enum Mode { MODE_PLAYING, MODE_MENU, MODE_CLOCK, MODE_INFO };
extern Mode currentMode;

extern String currentArtist;
extern String currentAlbum;
extern String currentSong;
extern String currentType;
extern String currentCodec;
extern String playerMachineID;
extern String playerIP;
extern String playerPort;

extern long currentDuration;
extern long currentOffset;
extern bool isPlaying;
extern int batteryPercent;

// ==========================================
// UI STATE
// ==========================================
extern int menuIndex;
extern int menuLen;
extern const char *menuItems[];
extern int currentBrightness;

extern int scrollCursor_Artist;
extern int scrollCursor_Song;
extern unsigned long lastScrollTime;
extern unsigned long lastActiveDataTime;
extern int screensaverTimeout;

// ==========================================
// SYSTEM FLAGS
// ==========================================
extern bool shouldSaveConfig;

// ==========================================
// WEATHER GLOBALS
// ==========================================
extern bool weatherEnabled;
extern char weatherCity[30];
extern float weatherLat;
extern float weatherLon;
extern String weatherLocationName; // Confirmed Location Name (e.g. "Sile, TR")

extern float currentTemp;
extern int currentWeatherCode; // 0=Clear, 1-3=Cloud, etc.
extern unsigned long lastWeatherTime;
extern bool weatherValid; // New Flag

#endif
