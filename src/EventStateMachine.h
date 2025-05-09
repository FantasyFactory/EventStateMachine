/*
  EventStateMachine.h - Event-driven State Machine library for Arduino, ESP8266/ESP32, and RP2040
  Created by Corrado Casoni (corrado.casoni@gmail.com), May 8, 2025.
  Released under MIT License.
*/

#ifndef EVENT_STATE_MACHINE_H
#define EVENT_STATE_MACHINE_H

#include <Arduino.h>

// Definizioni per implementazioni specifiche per piattaforma
#if defined(ESP8266) || defined(ESP32)
  #include <Ticker.h>
  #include <vector>
  #include <functional>
  typedef Ticker TimerImplementation;
#elif defined(ARDUINO_ARCH_RP2040)
  // RP2040 ha un buon supporto per STL
  #include <vector>
  #include <functional>
  
  // Implementazione timer personalizzata per RP2040
  class TimerImplementation {
  private:
    unsigned long startTime;
    unsigned long duration;
    bool active;
    void (*callback)(void);
    
  public:
    TimerImplementation() : active(false), callback(nullptr) {}
    
    void once_ms(unsigned long ms, void (*func)(void)) {
      startTime = millis();
      duration = ms;
      callback = func;
      active = true;
    }
    
    void detach() {
      active = false;
    }
    
    bool update() {
      if (active && (millis() - startTime >= duration)) {
        active = false;
        if (callback) {
          callback();
          return true;
        }
      }
      return false;
    }
    
    bool isActive() const {
      return active;
    }
  };
#else
  // Per Arduino classici, implementazione semplificata
  // Implementazione timer personalizzata per Arduino
  class TimerImplementation {
  private:
    unsigned long startTime;
    unsigned long duration;
    bool active;
    void (*callback)(void);
    
  public:
    TimerImplementation() : active(false), callback(nullptr) {}
    
    void once_ms(unsigned long ms, void (*func)(void)) {
      startTime = millis();
      duration = ms;
      callback = func;
      active = true;
    }
    
    void detach() {
      active = false;
    }
    
    bool update() {
      if (active && (millis() - startTime >= duration)) {
        active = false;
        if (callback) {
          callback();
          return true;
        }
      }
      return false;
    }
    
    bool isActive() const {
      return active;
    }
  };

  // Implementazione semplice di array dinamico per Arduino
  template<typename T, size_t MAX_SIZE = 8>
  class SimpleArray {
  private:
    T items[MAX_SIZE];
    size_t count;
    
  public:
    SimpleArray() : count(0) {}
    
    bool push_back(const T& item) {
      if (count < MAX_SIZE) {
        items[count++] = item;
        return true;
      }
      return false;
    }
    
    bool erase(T* itemPtr) {
      size_t index = itemPtr - items;
      if (index < count) {
        // Sposta tutti gli elementi successivi
        for (size_t j = index; j < count - 1; j++) {
          items[j] = items[j + 1];
        }
        count--;
        return true;
      }
      return false;
    }
    
    T* begin() { return items; }
    T* end() { return items + count; }
    const T* begin() const { return items; }
    const T* end() const { return items + count; }
    
    size_t size() const { return count; }
    T& operator[](size_t index) { return items[index]; }
    const T& operator[](size_t index) const { return items[index]; }
  };
#endif

// Seleziona il contenitore appropriato in base alla piattaforma
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  template<typename T>
  using CallbackContainer = std::vector<T>;
#else
  template<typename T>
  using CallbackContainer = SimpleArray<T>;
#endif

// Definizione del tipo di funzione per i callback
typedef void (*StateCallback)(uint8_t currentState, uint8_t otherState);
typedef void (*StateFunction)(uint8_t state);
typedef void (*GlobalStateCallback)(uint8_t fromState, uint8_t toState);

// Definizione della struttura timeout 
struct TimeoutInfo {
  unsigned long duration;      // Durata in millisecondi
  StateCallback callback;      // Funzione callback
  TimerImplementation timer;   // Timer per la gestione del timeout
  bool active;                 // Flag per indicare se il timer è attivo
  
  #if !defined(ESP8266) && !defined(ESP32)
  uint8_t stateIndex;          // Indice dello stato per timer non-ESP
  uint8_t timeoutIndex;        // Indice del timeout per timer non-ESP
  #endif
};

// Struttura per la definizione di uno stato
struct StateDefinition {
  CallbackContainer<TimeoutInfo> timeouts;                      // Informazioni sui timeout
  CallbackContainer<StateCallback> onEnters;                    // Callback all'entrata dello stato
  CallbackContainer<StateFunction> onStates;                    // Callback durante lo stato
  CallbackContainer<StateCallback> onExits;                     // Callback all'uscita dello stato
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
  CallbackContainer<GlobalStateCallback> beforeStateChangeHandlers;
  CallbackContainer<GlobalStateCallback> afterStateChangeHandlers;
  
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


#endif // EVENT_STATE_MACHINE_H