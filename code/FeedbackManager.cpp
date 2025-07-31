#include "FeedbackManager.h"
#include "ToF.h"

// Static member variables
uint32_t FeedbackManager::buttonPressStart = 0;
bool FeedbackManager::buttonPressed = false;
bool FeedbackManager::modeChanged = false;

// ToF Mode Button Variables
uint32_t FeedbackManager::tofButtonPressStart = 0;
bool FeedbackManager::tofButtonPressed = false;
bool FeedbackManager::tofModeChanged = false;

void FeedbackManager::init() {
  pinMode(BTN2_PIN, INPUT_PULLUP); // User mode button
  pinMode(BTN1_PIN, INPUT_PULLUP); // ToF mode button
  Serial.println("🎛️  Feedback Manager initialized");
  Serial.println("   Press BTN2 for 4 seconds to cycle feedback modes");
  Serial.println("   Press BTN1 for 2 seconds to switch ToF modes");
}

void FeedbackManager::update(SensorData* sensorData) {
  static uint32_t lastDebounceTime = 0;
  static uint32_t lastTofDebounceTime = 0;
  const uint32_t debounceDelay = 50;
  const uint32_t longPressTime = 4000; // 4 seconds for feedback mode
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
  
  // Handle BTN1 (ToF Mode)
  bool tofButtonState = !digitalRead(BTN1_PIN); // Active LOW with pullup
  
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
      Serial.println("🔘 BTN1 pressed - hold for 2 seconds to switch ToF mode");
    } else if (!tofButtonState && tofButtonPressed) {
      // Button released
      tofButtonPressed = false;
      if (!tofModeChanged) {
        Serial.println("🔘 BTN1 released - no mode change");
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
  
  // Visual feedback with LEDs
  for (int i = 0; i < sensorData->feedbackMode + 1; i++) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    delay(200);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    delay(200);
  }
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
  
  // Visual feedback with LEDs
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  delay(500);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
} 