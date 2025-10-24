#include <Arduino.h>

#define LED_PIN       2          // on-board led (common for esp32 dev boards)
#define BUTTON_PIN    4          // button wired from gpio4 to gnd
#define DEBOUNCE_MS   50         // debounce time
#define DEBOUNCE_US   (DEBOUNCE_MS * 1000UL)

hw_timer_t* debounceTimer = nullptr;
volatile bool debounceActive = false;   // blocks re-entrant presses while debouncing

// --- one-shot debounce timer isr ---
// fires once after debounce window; confirms press and toggles led
void IRAM_ATTR onDebounceTimer() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // invert led state
  }
  debounceActive = false; // allow next press
}

// --- gpio button isr ---
// starts the one-shot debounce timer on a falling edge
void IRAM_ATTR onButtonISR() {
  if (!debounceActive) {
    debounceActive = true;
    // arm one-shot alarm: autoreload=false
    timerAlarmWrite(debounceTimer, DEBOUNCE_US, false);
    timerAlarmEnable(debounceTimer);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // button to gnd with internal pull-up; idle high, pressed low
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, onButtonISR, FALLING);

  // create a 1 mhz base timer (1 tick = 1 µs)
  // correct syntax: timerBegin(timer_number, divider, countUp)
  debounceTimer = timerBegin(0, 80, true);   // 80 -> 1 µs per tick (80mhz/80 = 1mhz)

  // attach the interrupt function to the timer
  // correct syntax: timerAttachInterrupt(timer, function, edge)
  timerAttachInterrupt(debounceTimer, &onDebounceTimer, true);
}

void loop() {
  // nothing to do. interrupts handle everything.
}
