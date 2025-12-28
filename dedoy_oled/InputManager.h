#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Arduino.h>

void handleInputs(); // Unified handler
void IRAM_ATTR isrEncoder();
void IRAM_ATTR isrEncSwitch();

extern volatile bool btnShortPressTriggered;
extern volatile bool btnLongPressTriggered;

#endif
