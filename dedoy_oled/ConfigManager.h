#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <WiFiManager.h>

void loadConfig();
void saveConfig();
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
void handleRoot();
void handleSave();
void setupConfig(WiFiManager &wm);

#endif
