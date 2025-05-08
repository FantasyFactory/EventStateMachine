/*
  BasicStateMachine

  Basic example of using the EventStateMachine library.
  This example shows how to create a simple state machine
  with three states: IDLE, RUNNING, and ERROR.

  The circuit:
  - Built-in LED on pin LED_BUILTIN

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

// Callback functions
void onEnterRunning(uint8_t current, uint8_t previous) {
  Serial.println("Entering RUNNING state");
  digitalWrite(LED_BUILTIN, HIGH);
}

void onExitRunning(uint8_t current, uint8_t next) {
  Serial.println("Exiting RUNNING state");
  digitalWrite(LED_BUILTIN, LOW);
}

void duringRunning(uint8_t state) {
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }
}

void onRunningTimeout(uint8_t current, uint8_t previous) {
  Serial.println("RUNNING state timeout");
  stateMachine.setState(STATE_ERROR);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("EventStateMachine Basic Example");
  
  // Configure states
  stateMachine.configureState(
    STATE_IDLE,     // State
    0,              // No timeout
    nullptr,        // No onEnter callback
    nullptr,        // No onState callback
    nullptr,        // No onExit callback
    nullptr         // No onTimeout callback
  );
  
  stateMachine.configureState(
    STATE_RUNNING,           // State
    10000,                   // 10 second timeout
    onEnterRunning,          // onEnter callback
    duringRunning,           // onState callback
    onExitRunning,           // onExit callback
    onRunningTimeout         // onTimeout callback
  );
  
  stateMachine.configureState(
    STATE_ERROR,    // State
    0,              // No timeout
    nullptr,        // No onEnter callback
    nullptr,        // No onState callback
    nullptr,        // No onExit callback
    nullptr         // No onTimeout callback
  );
  
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