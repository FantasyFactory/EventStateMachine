/*
  EventStateMachine.cpp - Event-driven State Machine library for Arduino, ESP8266/ESP32, and RP2040
  Created by Corrado Casoni (corrado.casoni@gmail.com), May 8, 2025.
  Released under MIT License.
*/
#include "EventStateMachine.h"

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
    
    auto& timeoutInfo = states[state].timeouts[timeoutIndex];
    timeoutInfo.active = false;
    timeoutInfo.callback(currentState, previousState);
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
  // Ferma tutti i timer
  for (uint8_t s = 0; s < numStates; s++) {
    for (auto& timeoutInfo : states[s].timeouts) {
      timeoutInfo.timer.detach();
      timeoutInfo.active = false;
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
  timeoutInfo.active = false;
  
#if !defined(ESP8266) && !defined(ESP32)
  timeoutInfo.stateIndex = state;
  timeoutInfo.timeoutIndex = states[state].timeouts.size();
#endif
  
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
  for (size_t i = 0; i < timeouts.size(); i++) {
    if (timeouts[i].duration == timeout) {
      timeouts[i].timer.detach(); // Assicurati di fermare il timer
      timeouts[i].active = false;
      
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      timeouts.erase(timeouts.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      timeouts.erase(&timeouts[i]);
#endif
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnEnter(uint8_t state, StateCallback onEnter) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onEnters;
  for (size_t i = 0; i < callbacks.size(); i++) {
    if (callbacks[i] == onEnter) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      callbacks.erase(callbacks.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      callbacks.erase(&callbacks[i]);
#endif
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnState(uint8_t state, StateFunction onState) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onStates;
  for (size_t i = 0; i < callbacks.size(); i++) {
    if (callbacks[i] == onState) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      callbacks.erase(callbacks.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      callbacks.erase(&callbacks[i]);
#endif
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeOnExit(uint8_t state, StateCallback onExit) {
  if (!isValidState(state)) return false;
  
  auto& callbacks = states[state].onExits;
  for (size_t i = 0; i < callbacks.size(); i++) {
    if (callbacks[i] == onExit) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      callbacks.erase(callbacks.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      callbacks.erase(&callbacks[i]);
#endif
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
  for (size_t i = 0; i < beforeStateChangeHandlers.size(); i++) {
    if (beforeStateChangeHandlers[i] == handler) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      beforeStateChangeHandlers.erase(beforeStateChangeHandlers.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      beforeStateChangeHandlers.erase(&beforeStateChangeHandlers[i]);
#endif
      return true;
    }
  }
  return false;
}

bool EventStateMachine::removeAfterStateChangeHandler(GlobalStateCallback handler) {
  for (size_t i = 0; i < afterStateChangeHandlers.size(); i++) {
    if (afterStateChangeHandlers[i] == handler) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      afterStateChangeHandlers.erase(afterStateChangeHandlers.begin() + i);
#else
      // Per Arduino, usa SimpleArray::erase
      afterStateChangeHandlers.erase(&afterStateChangeHandlers[i]);
#endif
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
  for (size_t i = 0; i < beforeStateChangeHandlers.size(); i++) {
    beforeStateChangeHandlers[i](currentState, newState);
  }
  
  // Ferma tutti i timer attivi dello stato corrente
  for (auto& timeoutInfo : states[currentState].timeouts) {
    timeoutInfo.timer.detach();
    timeoutInfo.active = false;
  }
  
  // Esegui tutti i callback di uscita
  for (size_t i = 0; i < states[currentState].onExits.size(); i++) {
    states[currentState].onExits[i](currentState, newState);
  }
  
  previousState = currentState;
  currentState = newState;
  stateEnteredTime = millis();
  stateChanged = true;
  
  // Esegui tutti i callback di entrata
  for (size_t i = 0; i < states[currentState].onEnters.size(); i++) {
    states[currentState].onEnters[i](currentState, previousState);
  }
  
  // Imposta i nuovi timeout
#if defined(ESP8266) || defined(ESP32)
  // Implementazione per ESP con Ticker
  for (size_t i = 0; i < states[currentState].timeouts.size(); i++) {
    auto& timeoutInfo = states[currentState].timeouts[i];
    timeoutInfo.active = true;
    
    // Usa la funzione statica come callback, passando stato e indice
    timeoutInfo.timer.once_ms(timeoutInfo.duration, 
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
#else
  // Implementazione per Arduino/RP2040 basata su millis()
  for (size_t i = 0; i < states[currentState].timeouts.size(); i++) {
    auto& timeoutInfo = states[currentState].timeouts[i];
    timeoutInfo.stateIndex = currentState;
    timeoutInfo.timeoutIndex = i;
    timeoutInfo.active = true;
    timeoutInfo.timer.once_ms(timeoutInfo.duration, nullptr);
    
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
#endif
  
  // Esegui tutti gli handler globali dopo il cambio di stato
  for (size_t i = 0; i < afterStateChangeHandlers.size(); i++) {
    afterStateChangeHandlers[i](previousState, currentState);
  }
}

void EventStateMachine::update() {
#if !defined(ESP8266) && !defined(ESP32)
  // Per piattaforme non-ESP, controlla e aggiorna i timer attivi
  for (size_t i = 0; i < states[currentState].timeouts.size(); i++) {
    TimeoutInfo& timeoutInfo = states[currentState].timeouts[i];
    if (timeoutInfo.active && timeoutInfo.timer.update()) {
      // Timer scaduto, chiama il callback
      onTimeout(currentState, i);
    }
  }
#endif
  
  // Esegui tutte le funzioni di stato
  for (size_t i = 0; i < states[currentState].onStates.size(); i++) {
    states[currentState].onStates[i](currentState);
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