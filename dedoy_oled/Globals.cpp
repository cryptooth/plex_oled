#include "Globals.h"

// Initialize Objects
#ifdef ESP32
// ESP32-C3 SuperMini Pinout: CLK=4, DATA=6, CS=7, RST=2 (No DC)
// 3-Wire SPI: (rotation, clock, data, cs, reset)
U8G2_SSD1322_NHD_256X64_F_3W_SW_SPI u8g2(U8G2_R0, 4, 6, 7, 2);
WebServer server(80);
#else
// NodeMCU: CLK=14, DATA=13, CS=15, RST=5 (Assuming 5 was Reset)
U8G2_SSD1322_NHD_256X64_F_3W_SW_SPI u8g2(U8G2_R0, 14, 13, 15, 5);
ESP8266WebServer server(80);
#endif

// Configuration Defaults
char plex_ip[40] = "192.168.1.100";
char plex_port[6] = "32400";
char plex_token[40] = "ENTER_TOKEN";

char artist_size[10] = "medium";
char artist_weight[10] = "bold";
char song_size[10] = "large";
char song_weight[10] = "bold";

// State
Mode currentMode = MODE_PLAYING;
String currentArtist = "Plex Waiting...";
String currentAlbum = "";
String currentSong = "No Connection";
String currentType = "track";
String currentCodec = "";
String playerMachineID = "";
String playerIP = "";
String playerPort = "";

long currentDuration = 0;
long currentOffset = 0;
bool isPlaying = false;
int batteryPercent = 100;

// UI State
const char *menuItems[] = {"Artist Font", "Song Font", "Brightness",
                           "Screensaver", "Info",      "Reboot",
                           "Exit"};
int menuIndex = 0;
int menuLen = 7;
int currentBrightness = 255;

int scrollCursor_Artist = 0;
int scrollCursor_Song = 0;
unsigned long lastScrollTime = 0;
unsigned long lastActiveDataTime = 0;
int screensaverTimeout = 5000;

// System
bool shouldSaveConfig = false;
