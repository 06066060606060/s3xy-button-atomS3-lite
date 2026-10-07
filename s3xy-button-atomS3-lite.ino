#include <Arduino.h>
#include "S3XYButton.h"

// ============================================================================
// M5Stack AtomS3 Lite hardware
// ============================================================================

static constexpr uint8_t BUTTON_PIN = 41;  // Physical button -> GND
static constexpr uint8_t RGB_LED_PIN = 35; // AtomS3 Lite built-in RGB LED

// AtomS3 Lite uses a WS2812/SK6812-style addressable RGB LED.
// If your Arduino-ESP32 version provides neopixelWrite(), no extra RGB
// library is required.

// ============================================================================
// Button timing
// ============================================================================

static constexpr uint32_t DEBOUNCE_MS = 30;
static constexpr uint32_t LONG_PRESS_MS = 800;
static constexpr uint32_t DOUBLE_CLICK_MS = 350;

static bool buttonStableState = HIGH;
static bool buttonLastReading = HIGH;
static uint32_t lastDebounceTime = 0;
static uint32_t buttonPressTime = 0;
static bool longPressTriggered = false;
static bool waitingForSecondClick = false;
static uint32_t firstClickTime = 0;

// ============================================================================
// RGB LED helpers
// ============================================================================

static void rgb(uint8_t r, uint8_t g, uint8_t b) {
  // AtomS3 Lite onboard RGB LED.
  // neopixelWrite() handles the WS2812/SK6812 protocol on ESP32.
  neopixelWrite(RGB_LED_PIN, r, g, b);
}

static void rgbOff() {
  rgb(0, 0, 0);
}

static void rgbDisconnected() {
  rgb(32, 0, 0);       // Red: waiting for S3XY Commander
}

static void rgbConnected() {
  rgb(0, 32, 0);       // Green: connected and subscribed
}

static void rgbAction() {
  rgb(0, 0, 40);       // Blue: command sent
}

// ============================================================================
// S3XY callbacks
// ============================================================================

void onConnected() {
  Serial.println("[S3XY] connected");
  rgbConnected();
}

void onDisconnected() {
  Serial.println("[S3XY] disconnected");
  rgbDisconnected();
}

// ============================================================================
// Button actions
// ============================================================================

static void sendSingleIfReady() {
  if (s3xy_ready()) {
    Serial.println("[BUTTON] SINGLE");
    rgbAction();
    s3xy_send_single();
    delay(40);
    rgbConnected();
  } else {
    Serial.println("[BUTTON] SINGLE ignored - S3XY not ready");
    rgb(40, 20, 0);     // Amber: not ready
    delay(80);
    rgbDisconnected();
  }
}

static void sendDoubleIfReady() {
  if (s3xy_ready()) {
    Serial.println("[BUTTON] DOUBLE");
    rgbAction();
    s3xy_send_double();
    delay(40);
    rgbConnected();
  } else {
    Serial.println("[BUTTON] DOUBLE ignored - S3XY not ready");
    rgb(40, 20, 0);
    delay(80);
    rgbDisconnected();
  }
}

static void sendLongIfReady() {
  if (s3xy_ready()) {
    Serial.println("[BUTTON] LONG");
    rgbAction();
    s3xy_send_long();
    delay(40);
    rgbConnected();
  } else {
    Serial.println("[BUTTON] LONG ignored - S3XY not ready");
    rgb(40, 20, 0);
    delay(80);
    rgbDisconnected();
  }
}

// ============================================================================
// Physical button state machine
// ============================================================================

static void handleButton() {
  const uint32_t now = millis();
  const bool reading = digitalRead(BUTTON_PIN);

  // Debounce input.
  if (reading != buttonLastReading) {
    lastDebounceTime = now;
    buttonLastReading = reading;
  }

  if ((now - lastDebounceTime) < DEBOUNCE_MS) {
    return;
  }

  // Stable state transition.
  if (reading != buttonStableState) {
    buttonStableState = reading;

    if (buttonStableState == LOW) {
      // Pressed.
      buttonPressTime = now;
      longPressTriggered = false;
      Serial.println("[BUTTON] pressed");
    } else {
      // Released.
      const uint32_t pressDuration = now - buttonPressTime;
      Serial.printf("[BUTTON] released after %lu ms\n",
                    static_cast<unsigned long>(pressDuration));

      // Long command was already sent while held.
      if (longPressTriggered) {
        return;
      }

      // Short click: start or complete double-click sequence.
      if (pressDuration < LONG_PRESS_MS) {
        if (!waitingForSecondClick) {
          waitingForSecondClick = true;
          firstClickTime = now;
          Serial.println("[BUTTON] waiting for second click");
        } else if ((now - firstClickTime) <= DOUBLE_CLICK_MS) {
          waitingForSecondClick = false;
          sendDoubleIfReady();
        } else {
          firstClickTime = now;
        }
      }
    }
  }

  // Long press is sent once while the button remains held.
  if (buttonStableState == LOW &&
      !longPressTriggered &&
      (now - buttonPressTime >= LONG_PRESS_MS)) {
    longPressTriggered = true;
    waitingForSecondClick = false;
    sendLongIfReady();
  }

  // Confirm single click after the double-click window expires.
  if (waitingForSecondClick &&
      (now - firstClickTime > DOUBLE_CLICK_MS)) {
    waitingForSecondClick = false;
    sendSingleIfReady();
  }
}

// ============================================================================
// Arduino setup / loop
// ============================================================================

void setup() {
  Serial.begin(115200);

  // Physical button: GPIO 41 -> button -> GND.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // RGB LED output.
  pinMode(RGB_LED_PIN, OUTPUT);
  rgbOff();

  s3xy_on_connect(onConnected);
  s3xy_on_disconnect(onDisconnected);
  s3xy_begin("ENH_BTN");

  // Initial state: not connected.
  rgbDisconnected();

  Serial.println();
  Serial.println("========================================");
  Serial.println("S3XY Virtual Button - AtomS3 Lite");
  Serial.println("========================================");
  Serial.println("Button : GPIO 41 -> button -> GND");
  Serial.println("RGB LED: GPIO 35");
  Serial.println("Single : 1 short press");
  Serial.println("Double : 2 short presses");
  Serial.println("Long   : hold >= 800 ms");
  Serial.println("========================================");
}

void loop() {
  // Keep the S3XY BLE state machine serviced.
  s3xy_loop();

  // Poll the physical button without blocking delays.
  handleButton();

  // If the BLE connection disappears without a callback reaching the LED,
  // keep the indicator consistent with the actual state.
  static bool lastReady = false;
  const bool ready = s3xy_ready();
  if (ready != lastReady) {
    lastReady = ready;
    if (ready) {
      rgbConnected();
    } else {
      rgbDisconnected();
    }
  }
}
