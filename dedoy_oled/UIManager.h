#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <Arduino.h>

void drawNowPlaying();
void drawNowPlaying();
void drawMenu();
void drawClock();
void drawInfo();
String msToTime(long ms);
const uint8_t *getFont(const char *size, const char *weight);
void readBattery();
void uiLoop();

#endif
