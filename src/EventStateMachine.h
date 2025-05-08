/*
  EventStateMachine.h - Event-driven State Machine library for Arduino ESP8266/ESP32
  Created by Corrado Casoni (corrado.casoni@gmail.com), May 8, 2025.
  Released under MIT License.
*/

#ifndef EVENT_STATE_MACHINE_H
#define EVENT_STATE_MACHINE_H

#include <Arduino.h>
#if defined(ESP8266) || defined(ESP32)
#include <Ticker.h>
#include <vector>
#include <functional>

// Definizione del tipo di funzione per i callback
typedef void (*StateCallback)(uint8_t currentState, uint8_t otherState);
typedef void (*StateFunction)(uint8_t state);
typedef void (*GlobalStateCallback)(uint8_t fromState, uint8_t toState);

// Definizione della struttura timeout 
struct TimeoutInfo {
  unsigned long duration;      // Durata in millisecondi
  StateCallback callback;      // Funzione callback
  Ticker ticker;               // Ticker per la gestione del timeout
};

// Struttura per la definizione di uno stato
struct StateDefinition {
  std::vector<TimeoutInfo> timeouts;                            // Informazioni sui timeout
  std::vector<StateCallback> onEnters;                          // Callback all'entrata dello stato
  std::vector<StateFunction> onStates;                          // Callback durante lo stato
  std::vector<StateCallback> onExits;                           // Callback all'uscita dello stato
};

class EventStateMachine {
private:
  uint8_t currentState;
  uint8_t previousState;
  bool stateChanged;
  StateDefinition* states;
  uint8_t numStates;
  unsigned long stateEnteredTime;
  bool debugEnabled;
  
  // Handler globali per transizioni di stato
  std::vector<GlobalStateCallback> beforeStateChangeHandlers;
  std::vector<GlobalStateCallback> afterStateChangeHandlers;
  
  // Verifica se uno stato è valido
  bool isValidState(uint8_t state) const;
  
  // Funzioni statiche per i callback dei Ticker
  static EventStateMachine* instance; // Istanza globale per i callback statici
  static void onTimeoutStatic(uint8_t state, uint8_t timeoutIndex);

public:
  EventStateMachine(uint8_t numberOfStates);
  ~EventStateMachine();
  
  // Metodo per impostare l'istanza globale
  void setInstance() { instance = this; }
  
  // Abilita/disabilita i messaggi di debug
  void setDebug(bool enable) { debugEnabled = enable; }
  
  // Metodo di configurazione completo
  void configureState(uint8_t state, unsigned long timeout = 0,
                      StateCallback onEnter = nullptr,
                      StateFunction onState = nullptr,
                      StateCallback onExit = nullptr,
                      StateCallback onTimeout = nullptr);
  
  // Metodi per aggiungere callback specifici
  bool addTimeout(uint8_t state, unsigned long timeout, StateCallback onTimeout);
  bool addOnEnter(uint8_t state, StateCallback onEnter);
  bool addOnState(uint8_t state, StateFunction onState);
  bool addOnExit(uint8_t state, StateCallback onExit);
  
  // Metodi per rimuovere callback specifici
  bool removeTimeout(uint8_t state, unsigned long timeout);
  bool removeOnEnter(uint8_t state, StateCallback onEnter);
  bool removeOnState(uint8_t state, StateFunction onState);
  bool removeOnExit(uint8_t state, StateCallback onExit);
  
  // Metodi per gli handler globali di cambio stato
  void addBeforeStateChangeHandler(GlobalStateCallback handler);
  void addAfterStateChangeHandler(GlobalStateCallback handler);
  bool removeBeforeStateChangeHandler(GlobalStateCallback handler);
  bool removeAfterStateChangeHandler(GlobalStateCallback handler);
  
  // Callback interno per i timeout
  void onTimeout(uint8_t state, uint8_t timeoutIndex);
  
  // Cambia lo stato
  void setState(uint8_t newState);
  
  // Esecuzione di un ciclo della macchina a stati
  void update();
  
  // Getter per lo stato corrente
  uint8_t getCurrentState() const;
  
  // Getter per lo stato precedente
  uint8_t getPreviousState() const;
  
  // Controlla se lo stato è appena cambiato
  bool isStateChanged() const;
  
  // Tempo trascorso nello stato corrente
  unsigned long timeInCurrentState() const;
};

#else
#error "This library requires ESP8266 or ESP32 boards"
#endif

#endif // EVENT_STATE_MACHINE_H