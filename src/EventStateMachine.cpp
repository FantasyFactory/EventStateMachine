/*
  EventStateMachine.cpp - Event-driven State Machine library for Arduino ESP8266/ESP32
  Created by Corrado Casoni (corrado.casoni@gmail.com), May 8, 2025.
  Released under MIT License.
*/
#include "EventStateMachine.h"

#if defined(ESP8266) || defined(ESP32)

// Inizializzazione della variabile statica
EventStateMachine* EventStateMachine::instance = nullptr;

// Funzione statica per il callback del Ticker
void EventStateMachine::onTimeoutStatic(uint8_t state, uint8_t timeoutIndex) {
  if (instance) {
    instance->onTimeout(state, timeoutIndex);
  }
}

void EventStateMachine::onTimeout(uint8_t state, uint8_t timeoutIndex) {
  // Verifica che lo stato corrente sia quello per cui il timeout è stato impostato
  if (currentState == state && timeoutIndex < states[state].timeouts.size()) {
    if (debugEnabled) {
      Serial.print("DEBUG: Timeout triggered for state ");
      Serial.print(state);
      Serial.print(", index ");
      Serial.println(timeoutIndex);
    }
    
    states[state].timeouts[timeoutIndex].callback(currentState, previousState);
  }
}

bool EventStateMachine::isValidState(uint8_t state) const {
  return state < numStates;
}

EventStateMachine::EventStateMachine(uint8_t numberOfStates) {
  numStates = numberOfStates;
  states = new StateDefinition[numStates];
  currentState = 0;
  previousState = 0;
  stateChanged = true;
  stateEnteredTime = millis();
  debugEnabled = false;
  
  // Imposta questa istanza come l'istanza globale
  setInstance();
}

EventStateMachine::~EventStateMachine() {
  // Ferma tutti i Ticker
  for (uint8_t s = 0; s < numStates; s++) {
    for (auto& timeoutInfo : states[s].timeouts) {
      timeoutInfo.ticker.detach();
    }
  }
  
  delete[] states;
}

void EventStateMachine::configureState(uint8_t state, unsigned long timeout,
                  StateCallback onEnter,
                  StateFunction onState,
                  StateCallback onExit,
                  StateCallback onTimeout) {
  if (!isValidState(state)) return;
  
  // Aggiungi i callback se non sono nullptr
  if (onEnter != nullptr) {
    addOnEnter(state, onEnter);
  }
  
  if (onState != nullptr) {
    addOnState(state, onState);
  }
  
  if (onExit != nullptr) {
    addOnExit(state, onExit);
  }
  
  if (timeout > 0 && onTimeout != nullptr) {
    addTimeout(state, timeout, onTimeout);
  }
}

bool EventStateMachine::addTimeout(uint8_t state, unsigned long timeout, StateCallback onTimeout) {
  if (!isValidState(state) || onTimeout == nullptr) return false;
  
  // Crea e aggiungi la struttura TimeoutInfo
  TimeoutInfo timeoutInfo;
  timeoutInfo.duration = timeout;
  timeoutInfo.callback = onTimeout;
  
  states[state].timeouts.push_back(timeoutInfo);
  return true;
}

bool EventStateMachine::addOnEnter(uint8_t state, StateCallback onEnter) {
  if (!isValidState(state) || onEnter == nullptr) return false;
  
  states[state].onEnters.push_back(onEnter);
  return true;
}

bool EventStateMachine::addOnState(uint8_t state, StateFunction onState) {
  if (!isValidState(state) || onState == nullptr) return false;
  
  states[state].onStates.push_back(onState);
  return true;
}

bool EventStateMachine::addOnExit(uint8_t state, StateCallback onExit) {
  if (!isValidState(state) || onExit == nullptr) return false;
  
  states[state].onExits.push_back(onExit);
  return true;
}

bool EventStateMachine::removeTimeout(uint8_t state, unsigned long timeout) {
  if (!isValidState(state)) return false;
  
  auto& timeouts = states[state].timeouts;
  for (auto it = timeouts.begin(); it != timeouts.end(); ++it) {
    if (it->duration == timeout) {
      it->ticker.detach(); // Assicurati di fermare il ticker
      timeouts.erase(it);
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnEnter(uint8_t state, StateCallback onEnter) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onEnters;
  for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
    if (*it == onEnter) {
      callbacks.erase(it);
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnState(uint8_t state, StateFunction onState) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onStates;
  for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
    if (*it == onState) {
      callbacks.erase(it);
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnExit(uint8_t state, StateCallback onExit) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onExits;
  for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
    if (*it == onExit) {
      callbacks.erase(it);
      return true;
    }
  }
  return false;
}

void EventStateMachine::addBeforeStateChangeHandler(GlobalStateCallback handler) {
  if (handler != nullptr) {
    beforeStateChangeHandlers.push_back(handler);
  }
}

void EventStateMachine::addAfterStateChangeHandler(GlobalStateCallback handler) {
  if (handler != nullptr) {
    afterStateChangeHandlers.push_back(handler);
  }
}

bool EventStateMachine::removeBeforeStateChangeHandler(GlobalStateCallback handler) {
  for (auto it = beforeStateChangeHandlers.begin(); it != beforeStateChangeHandlers.end(); ++it) {
    if (*it == handler) {
      beforeStateChangeHandlers.erase(it);
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeAfterStateChangeHandler(GlobalStateCallback handler) {
  for (auto it = afterStateChangeHandlers.begin(); it != afterStateChangeHandlers.end(); ++it) {
    if (*it == handler) {
      afterStateChangeHandlers.erase(it);
      return true;
    }
  }
  return false;
}

void EventStateMachine::setState(uint8_t newState) {
  if (!isValidState(newState)) return;
  
  // Non fare nulla se lo stato non cambia
  if (newState == currentState) return;
  
  // Esegui tutti gli handler globali prima del cambio di stato
  for (const auto& handler : beforeStateChangeHandlers) {
    handler(currentState, newState);
  }
  
  // Ferma tutti i ticker attivi dello stato corrente
  for (auto& timeoutInfo : states[currentState].timeouts) {
    timeoutInfo.ticker.detach();
  }
  
  // Esegui tutti i callback di uscita
  for (const auto& onExit : states[currentState].onExits) {
    onExit(currentState, newState);
  }
  
  previousState = currentState;
  currentState = newState;
  stateEnteredTime = millis();
  stateChanged = true;
  
  // Esegui tutti i callback di entrata
  for (const auto& onEnter : states[currentState].onEnters) {
    onEnter(currentState, previousState);
  }
  
  // Imposta i nuovi timeout
  for (size_t i = 0; i < states[currentState].timeouts.size(); i++) {
    auto& timeoutInfo = states[currentState].timeouts[i];
    
    // Usa la funzione statica come callback, passando stato e indice
    timeoutInfo.ticker.once_ms(timeoutInfo.duration, 
      [state = currentState, index = i]() {
        EventStateMachine::onTimeoutStatic(state, index);
      }
    );
    
    if (debugEnabled) {
      Serial.print("DEBUG: Timeout set for state ");
      Serial.print(currentState);
      Serial.print(", index ");
      Serial.print(i);
      Serial.print(", duration ");
      Serial.print(timeoutInfo.duration);
      Serial.println(" ms");
    }
  }
  
  // Esegui tutti gli handler globali dopo il cambio di stato
  for (const auto& handler : afterStateChangeHandlers) {
    handler(previousState, currentState);
  }
}

void EventStateMachine::update() {
  // Esegui tutte le funzioni di stato
  for (const auto& onState : states[currentState].onStates) {
    onState(currentState);
  }
  
  stateChanged = false;
}

uint8_t EventStateMachine::getCurrentState() const {
  return currentState;
}

uint8_t EventStateMachine::getPreviousState() const {
  return previousState;
}

bool EventStateMachine::isStateChanged() const {
  return stateChanged;
}

unsigned long EventStateMachine::timeInCurrentState() const {
  return millis() - stateEnteredTime;
}

#endif // defined(ESP8266) || defined(ESP32)