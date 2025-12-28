#include "InputManager.h"
#include "ConfigManager.h"
#include "Globals.h"
#include "NetworkManager.h"

// Encoder State
volatile long encoderPos = 0;
volatile int lastEncoded = 0;
long lastEncoderPos = 0;

// Globals for ISR
volatile bool btnShortPressTriggered = false;
volatile bool btnLongPressTriggered = false;
volatile unsigned long isrBtnDownTime = 0;
volatile bool isrLongPressHandled = false;

// ISR for Encoder Rotation
void IRAM_ATTR isrEncoder() {
  int MSB = digitalRead(ENC_CLK); // MSB = most significant bit
  int LSB = digitalRead(ENC_DT);  // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // Converting the 2 pin value to single number
  int sum =
      (lastEncoded << 2) | encoded; // Adding it to the previous encoded value

  // Quadrature Decoding Table (Full resolution)
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderPos--;

  lastEncoded = encoded; // Store this value for next time
}

// ISR for Button (Runs on Change)
void IRAM_ATTR isrEncSwitch() {
  int state = digitalRead(ENC_SW);
  // Simple Debounce inside ISR
  static unsigned long lastIsrTime = 0;
  if (millis() - lastIsrTime < 30)
    return;
  lastIsrTime = millis();

  if (state == LOW) {
    // PRESSED
    isrBtnDownTime = millis();
    isrLongPressHandled = false;
  } else {
    // RELEASED
    if (isrBtnDownTime != 0 && !isrLongPressHandled) {
      // Check duration
      if (millis() - isrBtnDownTime < 800) {
        btnShortPressTriggered = true;
      }
    }
    isrBtnDownTime = 0;
  }
}

// Handler called from loop()
void handleInputs() {
  // 1. ROTATION HANDLING
  long newPos = encoderPos / 2;
  if (newPos != lastEncoderPos) {
    int direction = (newPos > lastEncoderPos) ? 1 : -1;
    lastEncoderPos = newPos;

    Serial.print("[INP] Rotation: ");
    Serial.print(direction);
    Serial.print(" | Mode: ");
    Serial.println(currentMode);

    // ROTATION ACTIONS
    if (currentMode == MODE_MENU) {
      if (direction > 0) { // CW: Down
        menuIndex++;
        if (menuIndex >= menuLen)
          menuIndex = 0;
      } else { // CCW: Up
        menuIndex--;
        if (menuIndex < 0)
          menuIndex = menuLen - 1;
      }
    } else if (currentMode == MODE_PLAYING) {
      if (playerMachineID == "")
        Serial.println("[INP] Error: No Machine ID!");

      if (direction > 0) {
        Serial.println("[INP] Skip Next");
        sendPlexCommand("skipNext");
      } else {
        Serial.println("[INP] Skip Prev");
        sendPlexCommand("skipPrevious");
      }
    } else if (currentMode == MODE_CLOCK || currentMode == MODE_INFO) {
      Serial.println("[INP] Waking Up");
      currentMode = MODE_PLAYING;
    }
  }

  // 2. LONG PRESS CHECK (Polled, but uses ISR vars)
  if (isrBtnDownTime != 0 && !isrLongPressHandled) {
    if (millis() - isrBtnDownTime > 800) {
      btnLongPressTriggered = true;
      isrLongPressHandled = true; // Prevent multiple triggers
    }
  }

  // B. EXECUTE ACTIONS (Main Loop Context)
  if (btnShortPressTriggered) {
    btnShortPressTriggered = false;
    Serial.print("[BTN] Short Press | Mode: ");
    Serial.println(currentMode);

    if (currentMode == MODE_MENU) {
      String selected = String(menuItems[menuIndex]);
      Serial.println("[BTN] Menu Selected: " + selected);

      if (selected == "Exit")
        currentMode = MODE_PLAYING;
      else if (selected == "Reboot")
        ESP.restart();
      else if (selected == "Info")
        currentMode = MODE_INFO;
      else if (selected == "Artist Font") {
        if (String(artist_size) == "small")
          strcpy(artist_size, "medium");
        else if (String(artist_size) == "medium")
          strcpy(artist_size, "large");
        else
          strcpy(artist_size, "small");
        saveConfig();
      } else if (selected == "Song Font") {
        if (String(song_size) == "small")
          strcpy(song_size, "medium");
        else if (String(song_size) == "medium")
          strcpy(song_size, "large");
        else
          strcpy(song_size, "small");
        saveConfig();
      } else if (selected == "Screensaver") {
        if (screensaverTimeout == 5000)
          screensaverTimeout = 15000;
        else if (screensaverTimeout == 15000)
          screensaverTimeout = 30000;
        else if (screensaverTimeout == 30000)
          screensaverTimeout = 60000;
        else
          screensaverTimeout = 5000;
        saveConfig();
      } else if (selected == "Brightness") {
        currentBrightness += 51;
        if (currentBrightness > 255)
          currentBrightness = 0;
        u8g2.setContrast(currentBrightness);
        saveConfig();
      }
    } else if (currentMode == MODE_PLAYING) {
      if (playerMachineID == "")
        Serial.println("[INP] Error: No Machine ID!");

      if (isPlaying) {
        Serial.println("[INP] PAUSE");
        sendPlexCommand("pause");
        isPlaying = false;
      } else {
        Serial.println("[INP] PLAY");
        sendPlexCommand("play");
        isPlaying = true;
      }
    } else {
      currentMode = MODE_PLAYING;
    }
  }

  if (btnLongPressTriggered) {
    btnLongPressTriggered = false;
    Serial.println("[BTN] LONG PRESS ACTION");

    if (currentMode == MODE_MENU || currentMode == MODE_INFO) {
      currentMode = MODE_PLAYING;
    } else {
      currentMode = MODE_MENU;
      menuIndex = 0;
    }
  }
}
