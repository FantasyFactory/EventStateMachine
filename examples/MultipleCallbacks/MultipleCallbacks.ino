/*
  MultipleCallbacks

  Example demonstrating multiple callbacks for state events.
  This example shows how to add multiple callbacks for
  enter, exit, and during state events.

  The circuit:
  - Built-in LED on pin LED_BUILTIN

  This example works on:
  - Arduino boards
  - ESP8266/ESP32
  - Raspberry Pi Pico (RP2040)

  created May 8, 2025
  by Corrado Casoni
*/

#include <EventStateMachine.h>

// Define states
enum States {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_ERROR,
  NUM_STATES
};

// Create state machine
EventStateMachine stateMachine(NUM_STATES);

// Global state transition handlers
void beforeStateChange(uint8_t fromState, uint8_t toState) {
  Serial.print("GLOBAL: About to change from state ");
  Serial.print(fromState);
  Serial.print(" to state ");
  Serial.println(toState);
}

void afterStateChange(uint8_t fromState, uint8_t toState) {
  Serial.print("GLOBAL: Changed from state ");
  Serial.print(fromState);
  Serial.print(" to state ");
  Serial.println(toState);
}

// Multiple callbacks for RUNNING state
void logEnterRunning(uint8_t current, uint8_t previous) {
  Serial.println("Callback 1: Entering RUNNING state");
}

void setupHardwareForRunning(uint8_t current, uint8_t previous) {
  Serial.println("Callback 2: Setting up hardware for RUNNING state");
  digitalWrite(LED_BUILTIN, HIGH);
}

void blinkLED(uint8_t state) {
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }
}

void updateSerial(uint8_t state) {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    Serial.println("Still running...");
    lastUpdate = millis();
  }
}

void logExitRunning(uint8_t current, uint8_t next) {
  Serial.println("Callback 1: Exiting RUNNING state");
}

void shutdownHardwareForRunning(uint8_t current, uint8_t next) {
  Serial.println("Callback 2: Shutting down hardware for RUNNING state");
  digitalWrite(LED_BUILTIN, LOW);
}

// Multiple timeout callbacks
void shortTimeout(uint8_t current, uint8_t previous) {
  Serial.println("Short timeout (5s) triggered");
}

void longTimeout(uint8_t current, uint8_t previous) {
  Serial.println("Long timeout (10s) triggered");
  stateMachine.setState(STATE_ERROR);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("EventStateMachine Multiple Callbacks Example");
  
  // Add global state transition handlers
  stateMachine.addBeforeStateChangeHandler(beforeStateChange);
  stateMachine.addAfterStateChangeHandler(afterStateChange);
  
  // Add multiple callbacks for RUNNING state
  stateMachine.addOnEnter(STATE_RUNNING, logEnterRunning);
  stateMachine.addOnEnter(STATE_RUNNING, setupHardwareForRunning);
  
  stateMachine.addOnState(STATE_RUNNING, blinkLED);
  stateMachine.addOnState(STATE_RUNNING, updateSerial);
  
  stateMachine.addOnExit(STATE_RUNNING, logExitRunning);
  stateMachine.addOnExit(STATE_RUNNING, shutdownHardwareForRunning);
  
  // Add multiple timeouts
  stateMachine.addTimeout(STATE_RUNNING, 5000, shortTimeout);
  stateMachine.addTimeout(STATE_RUNNING, 10000, longTimeout);
  
  // Set initial state
  stateMachine.setState(STATE_IDLE);
  
  Serial.println("Press 'r' to enter RUNNING state");
  Serial.println("Press 'i' to enter IDLE state");
  Serial.println("Press 'e' to enter ERROR state");
}

void loop() {
  // Update state machine
  stateMachine.update();
  
  // Process serial commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'r':
        Serial.println("Command: Enter RUNNING state");
        stateMachine.setState(STATE_RUNNING);
        break;
      case 'i':
        Serial.println("Command: Enter IDLE state");
        stateMachine.setState(STATE_IDLE);
        break;
      case 'e':
        Serial.println("Command: Enter ERROR state");
        stateMachine.setState(STATE_ERROR);
        break;
    }
  }
  
  // Small delay
  delay(10);
}