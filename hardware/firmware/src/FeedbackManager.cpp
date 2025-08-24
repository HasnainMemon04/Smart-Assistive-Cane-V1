#include "FeedbackManager.h"
#include "ToF.h"

// Static member variables
uint32_t FeedbackManager::buttonPressStart = 0;
bool FeedbackManager::buttonPressed = false;
bool FeedbackManager::modeChanged = false;

// Radar Mode Button Variables (BTN1)
uint32_t FeedbackManager::radarButtonPressStart = 0;
bool FeedbackManager::radarButtonPressed = false;
bool FeedbackManager::radarModeActivated = false;
bool FeedbackManager::wasInRadarMode = false;

// ToF Mode Button Variables (BTN3)
uint32_t FeedbackManager::tofButtonPressStart = 0;
bool FeedbackManager::tofButtonPressed = false;
bool FeedbackManager::tofModeChanged = false;

void FeedbackManager::init() {
  pinMode(BTN2_PIN, INPUT_PULLUP); // User mode button
  pinMode(BTN1_PIN, INPUT_PULLUP); // Radar mode button
  pinMode(BTN3_PIN, INPUT_PULLUP); // ToF mode button
  Serial.println("🎛️  Feedback Manager initialized");
  Serial.println("   Press BTN2 for 4 seconds to cycle feedback modes");
  Serial.println("   Hold BTN1 for 2+ seconds to toggle radar mode");
  Serial.println("   Press BTN3 for 2 seconds to switch ToF modes");
}

void FeedbackManager::update(SensorData* sensorData) {
  static uint32_t lastDebounceTime = 0;
  static uint32_t lastRadarDebounceTime = 0;
  static uint32_t lastTofDebounceTime = 0;
  const uint32_t debounceDelay = 50;
  const uint32_t longPressTime = 4000; // 4 seconds for feedback mode
  const uint32_t radarActivationTime = 2000; // 2 seconds for radar activation
  const uint32_t tofLongPressTime = 2000; // 2 seconds for ToF mode
  
  // Handle BTN2 (Feedback Mode)
  bool buttonState = !digitalRead(BTN2_PIN); // Active LOW with pullup
  
  // Debounce
  if (buttonState != buttonPressed) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState && !buttonPressed) {
      // Button just pressed
      buttonPressed = true;
      buttonPressStart = millis();
      modeChanged = false;
      Serial.println("🔘 BTN2 pressed - hold for 4 seconds to cycle feedback mode");
    } else if (!buttonState && buttonPressed) {
      // Button released
      buttonPressed = false;
      if (!modeChanged) {
        Serial.println("🔘 BTN2 released - no mode change");
      }
    } else if (buttonState && buttonPressed && !modeChanged) {
      // Check for long press
      if ((millis() - buttonPressStart) >= longPressTime) {
        cycleMode(sensorData);
        modeChanged = true;
      }
    }
  }
  
  // Handle BTN1 (Radar Mode Toggle)
  bool radarButtonState = !digitalRead(BTN1_PIN); // Active LOW with pullup
  
  // Debounce
  if (radarButtonState != radarButtonPressed) {
    lastRadarDebounceTime = millis();
  }
  
  if ((millis() - lastRadarDebounceTime) > debounceDelay) {
    if (radarButtonState && !radarButtonPressed) {
      // Button just pressed
      radarButtonPressed = true;
      radarButtonPressStart = millis();
      radarModeActivated = false;
      Serial.println("🔘 BTN1 pressed - hold for 2+ seconds to toggle radar mode");
    } else if (!radarButtonState && radarButtonPressed) {
      // Button released
      radarButtonPressed = false;
      if (!radarModeActivated) {
        Serial.println("🔘 BTN1 released - no mode change");
      }
    } else if (radarButtonState && radarButtonPressed && !radarModeActivated) {
      // Check for long press to toggle radar mode
      if ((millis() - radarButtonPressStart) >= radarActivationTime) {
        if (ToF_isRadarMode()) {
          ToF_switchToSimpleMode();
          Serial.println("📡 Radar mode deactivated - switched to simple mode");
        } else {
          ToF_switchToRadarMode();
          Serial.println("📡 Radar mode activated - scanning enabled");
        }
        radarModeActivated = true;
      }
    }
  }
  
  // Handle BTN3 (ToF Mode Switching)
  bool tofButtonState = !digitalRead(BTN3_PIN); // Active LOW with pullup
  
  // Debounce
  if (tofButtonState != tofButtonPressed) {
    lastTofDebounceTime = millis();
  }
  
  if ((millis() - lastTofDebounceTime) > debounceDelay) {
    if (tofButtonState && !tofButtonPressed) {
      // Button just pressed
      tofButtonPressed = true;
      tofButtonPressStart = millis();
      tofModeChanged = false;
      Serial.println("🔘 BTN3 pressed - hold for 2 seconds to switch ToF mode");
    } else if (!tofButtonState && tofButtonPressed) {
      // Button released
      tofButtonPressed = false;
      if (!tofModeChanged) {
        Serial.println("🔘 BTN3 released - no mode change");
      }
    } else if (tofButtonState && tofButtonPressed && !tofModeChanged) {
      // Check for long press
      if ((millis() - tofButtonPressStart) >= tofLongPressTime) {
        cycleToFMode();
        tofModeChanged = true;
      }
    }
  }
}

void FeedbackManager::cycleMode(SensorData* sensorData) {
  uint8_t oldMode = sensorData->feedbackMode;
  
  // Cycle through modes: BOTH -> BUZZER -> VIBRATION -> BOTH
  sensorData->feedbackMode = (sensorData->feedbackMode + 1) % 3;
  
  Serial.printf("🔄 Feedback mode changed: %s → %s\n", 
                getModeName(oldMode), getModeName(sensorData->feedbackMode));
  
  // LED visual feedback removed - unnecessary for blind users
}

const char* FeedbackManager::getModeName(uint8_t mode) {
  switch (mode) {
    case FEEDBACK_MODE_BOTH:
      return "BOTH";
    case FEEDBACK_MODE_BUZZER:
      return "BUZZER";
    case FEEDBACK_MODE_VIBRATION:
      return "VIBRATION";
    default:
      return "UNKNOWN";
  }
}

bool FeedbackManager::shouldUseBuzzer(uint8_t mode) {
  return (mode == FEEDBACK_MODE_BOTH || mode == FEEDBACK_MODE_BUZZER);
}

bool FeedbackManager::shouldUseVibration(uint8_t mode) {
  return (mode == FEEDBACK_MODE_BOTH || mode == FEEDBACK_MODE_VIBRATION);
}

void FeedbackManager::cycleToFMode() {
  // Switch between SIMPLE and RADAR modes
  if (ToF_isRadarMode()) {
    ToF_switchToSimpleMode();
  } else {
    ToF_switchToRadarMode();
  }
  
  // LED visual feedback removed - unnecessary for blind users
}