# EventStateMachine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A flexible and powerful event-driven State Machine library for ESP8266 and ESP32.

*Read this in [Italian](README_IT.md)*

## Features

- **Advanced event handling**: multiple callbacks for each state event (enter, exit, during)
- **Configurable timeouts**: set multiple timeouts per state with specific callbacks
- **Global transition handlers**: customizable hooks before and after state changes
- **Easy integration with persistent storage**: example of saving and recovering state from flash memory
- **Robust error handling**: verification of state validity and callbacks
- **Fully C++ based**: uses modern features like vectors and functionals

## Requirements

- ESP8266 or ESP32 board
- Arduino IDE 1.8.0 or higher
- (Optional) LittleFS for the state persistence example

## Installation

### Via Arduino IDE Library Manager (Recommended)

1. Open Arduino IDE
2. Go to Sketch > Include Library > Manage Libraries...
3. Search for "EventStateMachine"
4. Select the library and click "Install"

### Manual Installation

1. Download the library as a ZIP file from GitHub
2. Open Arduino IDE
3. Go to Sketch > Include Library > Add .ZIP Library...
4. Select the downloaded ZIP file

## Core Concepts

### States and Callbacks

EventStateMachine uses a callback system to handle state events:

- **onEnter**: executed when entering a state
- **onState**: executed continuously while in a state (in the `update()` method)
- **onExit**: executed when exiting a state
- **onTimeout**: executed when a configured timeout expires

### Timeouts

Each state can have multiple timeouts with different durations and callbacks:

```cpp
// Add a 5-second timeout to the RUNNING state
stateMachine.addTimeout(STATE_RUNNING, 5000, onRunningTimeout);
```

### Global Transition Handlers

You can register functions that will be called before and after every state transition:

```cpp
// Add a handler to save every transition
stateMachine.addAfterStateChangeHandler(saveStateTransition);
```

## Basic Usage

```cpp
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

// Callback when entering RUNNING state
void onEnterRunning(uint8_t current, uint8_t previous) {
  Serial.println("Entering RUNNING state");
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Add callbacks
  stateMachine.addOnEnter(STATE_RUNNING, onEnterRunning);
  
  // Set initial state
  stateMachine.setState(STATE_IDLE);
}

void loop() {
  // Update state machine
  stateMachine.update();
  
  // You can change the state based on external conditions
  if (/* some condition */) {
    stateMachine.setState(STATE_RUNNING);
  }
}
```

## Complete API

### Constructor and Destructor

```cpp
// Create a state machine with the specified number of states
EventStateMachine(uint8_t numberOfStates);

// Destructor - stops all tickers and frees memory
~EventStateMachine();
```

### State Configuration

```cpp
// Configure a state in a single call
void configureState(
  uint8_t state,            // State to configure
  unsigned long timeout = 0, // Optional timeout
  StateCallback onEnter = nullptr,  // Entry callback
  StateFunction onState = nullptr,  // During callback
  StateCallback onExit = nullptr,   // Exit callback
  StateCallback onTimeout = nullptr // Timeout callback
);

// Methods to add individual callbacks
bool addTimeout(uint8_t state, unsigned long timeout, StateCallback onTimeout);
bool addOnEnter(uint8_t state, StateCallback onEnter);
bool addOnState(uint8_t state, StateFunction onState);
bool addOnExit(uint8_t state, StateCallback onExit);

// Methods to remove callbacks
bool removeTimeout(uint8_t state, unsigned long timeout);
bool removeOnEnter(uint8_t state, StateCallback onEnter);
bool removeOnState(uint8_t state, StateFunction onState);
bool removeOnExit(uint8_t state, StateCallback onExit);
```

### Global Transition Handlers

```cpp
void addBeforeStateChangeHandler(GlobalStateCallback handler);
void addAfterStateChangeHandler(GlobalStateCallback handler);
bool removeBeforeStateChangeHandler(GlobalStateCallback handler);
bool removeAfterStateChangeHandler(GlobalStateCallback handler);
```

### Control and Execution

```cpp
// Change the current state
void setState(uint8_t newState);

// Perform an update cycle (call this in loop())
void update();

// Enable/disable debug messages on the serial port
void setDebug(bool enable);
```

### Informational Methods

```cpp
// Get the current state
uint8_t getCurrentState() const;

// Get the previous state
uint8_t getPreviousState() const;

// Check if the state has just changed
bool isStateChanged() const;

// Get the time spent in the current state (in ms)
unsigned long timeInCurrentState() const;
```

## Examples

The library includes three complete examples:

### BasicStateMachine

Demonstrates basic state machine usage with three states and essential callbacks. Controls the built-in LED based on state and responds to serial commands.

### MultipleCallbacks

Illustrates how to use multiple callbacks for each type of state event. Includes examples of:
- Multiple entry callbacks for a state
- Multiple during callbacks for a state
- Multiple exit callbacks for a state
- Multiple timeouts with different durations

### StateRecovery

Shows how to implement state persistence using LittleFS. Allows you to:
- Save each state transition to a log file
- Recover the last saved state after a restart
- View the history of transitions

## Design Considerations

### Performance

The library uses C++ vectors to manage multiple callbacks, which offers flexibility but requires a certain amount of memory. On devices with very limited memory, consider using only the necessary callbacks.

### Synchronization

Timeouts are managed using the Ticker library, which works asynchronously. This means that timeout callbacks can be executed at any time, even during other operations.

## Troubleshooting

### Timeouts not firing

- Verify that `update()` is called regularly in the loop
- Check that there are no long delay() calls blocking execution
- Make sure the timeout callback is not null

### Callbacks not executed

- Ensure the state is valid (< numStates)
- Verify that callbacks have been properly registered
- Check that the state machine instance is not being recreated

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests on GitHub.

## License

This library is released under the MIT License. See the LICENSE file for details.

## Credits

Created by Corrado Casoni (corrado.casoni@gmail.com), May 2025.
