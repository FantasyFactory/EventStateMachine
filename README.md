# EventStateMachine

A flexible and powerful event-driven State Machine library for ESP8266 and ESP32.

## Features

- Multiple callbacks for each state event (enter, exit, during)
- Multiple configurable timeouts per state
- Global state transition handlers (before and after)
- Easy to integrate with persistent storage
- Robust error handling

## Installation

1. Download the ZIP file from GitHub
2. Open Arduino IDE
3. Go to Sketch > Include Library > Add .ZIP Library
4. Select the downloaded ZIP file

## Requirements

- ESP8266 or ESP32 board
- Arduino IDE 1.8.0 or higher

## Usage

### Basic Example

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

void setup() {
  // Add callbacks
  stateMachine.addOnEnter(STATE_RUNNING, onEnterRunning);
  stateMachine.addTimeout(STATE_RUNNING, 5000, onTimeout);
  
  // Set initial state
  stateMachine.setState(STATE_IDLE);
}

void loop() {
  // Update state machine
  stateMachine.update();
}
```

## Examples

See the examples folder for complete examples:

- BasicStateMachine: Simple state machine usage
- MultipleCallbacks: Using multiple callbacks for each state
- StateRecovery: Saving and recovering state from flash memory

## License

This library is released under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.