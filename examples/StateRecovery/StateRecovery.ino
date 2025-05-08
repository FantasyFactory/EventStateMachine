/*
  StateRecovery

  Example demonstrating state persistence and recovery.
  This example shows how to save state transitions to flash
  and recover the last state after a restart.

  The circuit:
  - Built-in LED on pin LED_BUILTIN

  created May 8, 2025
  by Corrado Casoni
*/

#include <EventStateMachine.h>
#include <LittleFS.h>

// Define states
enum States {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_ERROR,
  NUM_STATES
};

// State names for logging
const char* STATE_NAMES[] = {
  "IDLE",
  "RUNNING",
  "PAUSED",
  "ERROR"
};

// Create state machine
EventStateMachine stateMachine(NUM_STATES);

// Global state transition handlers
void saveStateTransition(uint8_t fromState, uint8_t toState) {
  Serial.print("Saving state transition: ");
  Serial.print(STATE_NAMES[fromState]);
  Serial.print(" -> ");
  Serial.println(STATE_NAMES[toState]);
  
  // Open log file in append mode
  File stateLog = LittleFS.open("/state_log.txt", "a");
  if (stateLog) {
    // Write timestamp and state info
    stateLog.print(millis());
    stateLog.print(",");
    stateLog.print(fromState);
    stateLog.print(",");
    stateLog.println(toState);
    stateLog.close();
    
    Serial.println("State transition saved successfully");
  } else {
    Serial.println("Error opening state log file!");
  }
}

// State callbacks
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

// Function to recover last state from flash
bool recoverLastStateFromFlash(uint8_t& lastState) {
  if (LittleFS.exists("/state_log.txt")) {
    File stateLog = LittleFS.open("/state_log.txt", "r");
    if (stateLog) {
      // Read the last saved state
      String lastLine;
      while (stateLog.available()) {
        String line = stateLog.readStringUntil('\n');
        if (line.length() > 0) {
          lastLine = line;
        }
      }
      
      if (lastLine.length() > 0) {
        // Parse the last line (format: timestamp,fromState,toState)
        int lastComma = lastLine.lastIndexOf(',');
        if (lastComma > 0) {
          lastState = (uint8_t)lastLine.substring(lastComma + 1).toInt();
          stateLog.close();
          return true;
        }
      }
      stateLog.close();
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize filesystem
  if (!LittleFS.begin()) {
    Serial.println("Error mounting LittleFS!");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
  
  Serial.println("EventStateMachine State Recovery Example");
  
  // Add global state transition handler
  stateMachine.addAfterStateChangeHandler(saveStateTransition);
  
  // Configure state callbacks
  stateMachine.addOnEnter(STATE_RUNNING, onEnterRunning);
  stateMachine.addOnState(STATE_RUNNING, duringRunning);
  stateMachine.addOnExit(STATE_RUNNING, onExitRunning);
  
  // Try to recover last state
  uint8_t recoveredState = 0;
  if (recoverLastStateFromFlash(recoveredState) && recoveredState < NUM_STATES) {
    Serial.print("Last state recovered from flash: ");
    Serial.println(STATE_NAMES[recoveredState]);
    stateMachine.setState(recoveredState);
    } else {
      Serial.println("No state recovered, initializing to IDLE state");
      stateMachine.setState(STATE_IDLE);
    }
    Serial.println("\nCommands:");
    Serial.println("'r' - Enter RUNNING state");
    Serial.println("'i' - Enter IDLE state");
    Serial.println("'p' - Enter PAUSED state");
    Serial.println("'e' - Enter ERROR state");
    Serial.println("'c' - Clear state log");
    Serial.println("'l' - List stored state transitions");
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
            case 'p':
                Serial.println("Command: Enter PAUSED state");
                stateMachine.setState(STATE_PAUSED);
                break;
            case 'e':
                Serial.println("Command: Enter ERROR state");
                stateMachine.setState(STATE_ERROR);
                break;
            case 'c':
                Serial.println("Command: Clear state log");
                if (LittleFS.remove("/state_log.txt")) {
                    Serial.println("State log cleared");
                } else {
                    Serial.println("Error clearing state log");
                }
                break;
            case 'l':
                Serial.println("Command: List state transitions");
                if (LittleFS.exists("/state_log.txt")) {
                    File stateLog = LittleFS.open("/state_log.txt", "r");
                    if (stateLog) {
                    Serial.println("\n--- State Transition Log ---");
                    Serial.println("Timestamp,FromState,ToState");
                    
                    // Read and print each line
                    while (stateLog.available()) {
                        String line = stateLog.readStringUntil('\n');
                        if (line.length() > 0) {
                        // Parse the line (format: timestamp,fromState,toState)
                        int firstComma = line.indexOf(',');
                        int lastComma = line.lastIndexOf(',');
                        
                        if (firstComma > 0 && lastComma > firstComma) {
                            unsigned long timestamp = line.substring(0, firstComma).toInt();
                            uint8_t fromState = line.substring(firstComma + 1, lastComma).toInt();
                            uint8_t toState = line.substring(lastComma + 1).toInt();
                            
                            // Format and print the line
                            Serial.print(timestamp);
                            Serial.print("ms: ");
                            
                            if (fromState < NUM_STATES) {
                            Serial.print(STATE_NAMES[fromState]);
                            } else {
                            Serial.print("Unknown(");
                            Serial.print(fromState);
                            Serial.print(")");
                            }
                            
                            Serial.print(" -> ");
                            
                            if (toState < NUM_STATES) {
                            Serial.println(STATE_NAMES[toState]);
                            } else {
                            Serial.print("Unknown(");
                            Serial.print(toState);
                            Serial.println(")");
                            }
                        } else {
                            Serial.println(line); // Print raw line if parsing fails
                        }
                        }
                    }
                    Serial.println("--- End of Log ---\n");
                    stateLog.close();
                    } else {
                    Serial.println("Error opening state log file");
                    }
                } else {
                    Serial.println("State log file does not exist");
                }
                break;
            default:
                Serial.println("Unknown command");
                break;
        }
    }

    
    // Small delay
    delay(10);
}