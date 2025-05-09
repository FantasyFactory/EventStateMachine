# EventStateMachine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)

Una libreria flessibile e potente per implementare macchine a stati event-driven su Arduino, ESP8266/ESP32 e RP2040 (Raspberry Pi Pico).

## Caratteristiche

- **Compatibilità cross-platform**: funziona su schede Arduino, ESP8266/ESP32 e RP2040 (Raspberry Pi Pico)
- **Gestione avanzata degli eventi**: supporta callback multipli per ogni evento di stato (entrata, uscita, durante)
- **Timeout configurabili**: possibilità di impostare più timeout per stato con callback specifici
- **Gestori globali di transizione**: hook personalizzabili prima e dopo il cambio di stato
- **Facile integrazione con storage persistente**: esempi di salvataggio e recupero dello stato (utilizzando LittleFS su ESP/RP2040 o SD su Arduino)
- **Gestione robusta degli errori**: verifica della validità degli stati e dei callback
- **Implementazione ottimizzata per piattaforma**: utilizza funzionalità native dove disponibili, con alternative per piattaforme più semplici

## Requisiti

- Scheda Arduino, ESP8266/ESP32 o RP2040 (Raspberry Pi Pico)
- Arduino IDE 1.8.0 o superiore
- Per l'esempio StateRecovery:
  - ESP8266/ESP32 o RP2040: LittleFS
  - Schede Arduino: libreria SD e scheda SD

## Installazione

### Tramite Arduino IDE Library Manager (Consigliato)

1. Apri Arduino IDE
2. Vai su Sketch > Include Library > Manage Libraries...
3. Cerca "EventStateMachine"
4. Seleziona la libreria e clicca "Install"

### Installazione Manuale

1. Scarica la libreria come file ZIP da GitHub
2. Apri Arduino IDE
3. Vai su Sketch > Include Library > Add .ZIP Library...
4. Seleziona il file ZIP scaricato

## Considerazioni Specifiche per Piattaforma

### ESP8266/ESP32

Sulle piattaforme ESP, la libreria utilizza:
- Libreria **Ticker** per le funzioni di temporizzazione (timer hardware)
- **Container STL** (std::vector) per la memorizzazione dei callback
- **LittleFS** per la persistenza dello stato negli esempi

### RP2040 (Raspberry Pi Pico)

Su RP2040, la libreria utilizza:
- **Implementazione personalizzata del timer** basata su millis()
- **Container STL** (std::vector) per la memorizzazione dei callback (ben supportati su RP2040)
- **LittleFS** per la persistenza dello stato negli esempi

### Schede Arduino (AVR, SAMD, ecc.)

Sulle schede Arduino standard, la libreria utilizza:
- **Implementazione personalizzata del timer** basata su millis()
- **SimpleArray** come alternativa a std::vector per la memorizzazione dei callback (supporto STL limitato)
- **Scheda SD** per la persistenza dello stato negli esempi

### Utilizzo della Memoria

L'utilizzo della memoria varia in base alla piattaforma:
- ESP8266/ESP32 e RP2040: l'utilizzo di container STL richiede più memoria ma offre maggiore flessibilità
- Schede Arduino: l'utilizzo di SimpleArray con capacità fissa è più efficiente in termini di memoria ma ha capacità limitata

## Concetti Principali

### Stati e Callback

EventStateMachine utilizza un sistema di callback per gestire gli eventi di stato:

- **onEnter**: eseguito quando si entra in uno stato
- **onState**: eseguito continuamente mentre si è in uno stato (nel metodo `update()`)
- **onExit**: eseguito quando si esce da uno stato
- **onTimeout**: eseguito quando scade un timeout configurato

### Timeouts

Ogni stato può avere più timeout con diverse durate e callback:

```cpp
// Aggiunge un timeout di 5 secondi allo stato RUNNING
stateMachine.addTimeout(STATE_RUNNING, 5000, onRunningTimeout);
```

### Gestori Globali di Transizione

Puoi registrare funzioni che verranno chiamate prima e dopo ogni transizione di stato:

```cpp
// Aggiungi un gestore per salvare ogni transizione
stateMachine.addAfterStateChangeHandler(saveStateTransition);
```

## Utilizzo Base

```cpp
#include <EventStateMachine.h>

// Definisci gli stati
enum States {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_ERROR,
  NUM_STATES
};

// Crea la macchina a stati
EventStateMachine stateMachine(NUM_STATES);

// Callback all'entrata dello stato RUNNING
void onEnterRunning(uint8_t current, uint8_t previous) {
  Serial.println("Entrando nello stato RUNNING");
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Aggiungi i callback
  stateMachine.addOnEnter(STATE_RUNNING, onEnterRunning);
  
  // Imposta lo stato iniziale
  stateMachine.setState(STATE_IDLE);
}

void loop() {
  // Aggiorna la macchina a stati
  stateMachine.update();
  
  // Puoi cambiare stato in base a condizioni esterne
  if (/* qualche condizione */) {
    stateMachine.setState(STATE_RUNNING);
  }
}
```

## API Completa

### Costruttore e Distruttore

```cpp
// Crea una macchina a stati con il numero specificato di stati
EventStateMachine(uint8_t numberOfStates);

// Distruttore - ferma tutti i ticker e libera la memoria
~EventStateMachine();
```

### Configurazione degli Stati

```cpp
// Configura uno stato in un'unica chiamata
void configureState(
  uint8_t state,            // Stato da configurare
  unsigned long timeout = 0, // Timeout opzionale
  StateCallback onEnter = nullptr,  // Callback all'entrata
  StateFunction onState = nullptr,  // Callback durante
  StateCallback onExit = nullptr,   // Callback all'uscita
  StateCallback onTimeout = nullptr // Callback al timeout
);

// Metodi per aggiungere singoli callback
bool addTimeout(uint8_t state, unsigned long timeout, StateCallback onTimeout);
bool addOnEnter(uint8_t state, StateCallback onEnter);
bool addOnState(uint8_t state, StateFunction onState);
bool addOnExit(uint8_t state, StateCallback onExit);

// Metodi per rimuovere callback
bool removeTimeout(uint8_t state, unsigned long timeout);
bool removeOnEnter(uint8_t state, StateCallback onEnter);
bool removeOnState(uint8_t state, StateFunction onState);
bool removeOnExit(uint8_t state, StateCallback onExit);
```

### Gestori di Transizione Globale

```cpp
void addBeforeStateChangeHandler(GlobalStateCallback handler);
void addAfterStateChangeHandler(GlobalStateCallback handler);
bool removeBeforeStateChangeHandler(GlobalStateCallback handler);
bool removeAfterStateChangeHandler(GlobalStateCallback handler);
```

### Controllo e Esecuzione

```cpp
// Cambia lo stato corrente
void setState(uint8_t newState);

// Esegue un ciclo di aggiornamento (da chiamare nel loop())
void update();

// Abilita/disabilita messaggi di debug sulla porta seriale
void setDebug(bool enable);
```

### Metodi Informativi

```cpp
// Ottiene lo stato corrente
uint8_t getCurrentState() const;

// Ottiene lo stato precedente
uint8_t getPreviousState() const;

// Verifica se lo stato è appena cambiato
bool isStateChanged() const;

// Ottiene il tempo trascorso nello stato corrente (in ms)
unsigned long timeInCurrentState() const;
```

## Esempi

La libreria include tre esempi completi:

### BasicStateMachine

Dimostra l'utilizzo base della macchina a stati con tre stati e callback essenziali. Controlla il LED integrato in base allo stato e risponde ai comandi seriali. Funziona su tutte le piattaforme supportate con codice identico.

### MultipleCallbacks

Illustra come utilizzare callback multipli per ogni tipo di evento di stato. Include esempi di:
- Più callback di entrata per uno stato
- Più callback durante uno stato
- Più callback di uscita da uno stato
- Più timeout con diverse durate

### StateRecovery

Mostra come implementare la persistenza dello stato. Permette di:
- Salvare ogni transizione di stato in un file di log
- Recuperare l'ultimo stato salvato dopo un riavvio
- Visualizzare la cronologia delle transizioni

Questo esempio dimostra un'implementazione di storage specifica per piattaforma:
- Su ESP8266/ESP32 e RP2040: utilizza LittleFS
- Su schede Arduino: utilizza la libreria SD (richiede una scheda SD collegata alla scheda)

## Considerazioni di Design

### Prestazioni

Su ESP8266/ESP32 e RP2040, la libreria utilizza vector di C++ per gestire i callback multipli, il che offre flessibilità ma richiede più memoria. Su schede Arduino con memoria limitata, viene utilizzato un array semplice a capacità fissa.

Per prestazioni ottimali su dispositivi con memoria limitata, considera di:
- Utilizzare solo i callback necessari
- Limitare il numero di stati e callback per stato
- Evitare operazioni complesse nei callback

### Sincronizzazione

Su ESP8266/ESP32, i timeout sono gestiti utilizzando la libreria Ticker, che funziona in modo asincrono. Su altre piattaforme, i timeout vengono controllati durante la chiamata a `update()` utilizzando un timing basato su millis().

Questo significa:
- Su piattaforme ESP: i callback di timeout possono essere eseguiti in qualsiasi momento (in modo asincrono)
- Su altre piattaforme: i callback di timeout vengono eseguiti solo durante la chiamata a `update()` (in modo sincrono)

## Risoluzione dei Problemi

### Timeout non scattano

- Verifica che `update()` venga chiamato regolarmente nel loop
- Controlla che non ci siano delay() lunghi che bloccano l'esecuzione (particolarmente importante su piattaforme non-ESP)
- Assicurati che il callback del timeout non sia nullo

### Callback non eseguiti

- Assicurati che lo stato sia valido (< numStates)
- Verifica che i callback siano stati registrati correttamente
- Controlla che l'istanza della macchina a stati non venga ricreata

### Problemi specifici per piattaforma

- **Arduino (AVR/SAMD)**: Se superi il numero massimo di callback (predefinito: 8 per tipo), verranno ignorati silenziosamente. Considera di aumentare il parametro del template SimpleArray se necessario.
- **ESP8266/ESP32**: Assicurati di non esaurire la memoria heap quando utilizzi molti vector di callback.
- **RP2040**: La precisione di temporizzazione può essere leggermente diversa dalle piattaforme ESP a causa dell'implementazione basata su millis().

## Contribuire

I contributi sono benvenuti! Sentiti libero di aprire issue o inviare pull request su GitHub.

## License

Questa libreria è rilasciata sotto licenza MIT. Vedi il file LICENSE per i dettagli.

## Credits

Creata da Corrado Casoni (corrado.casoni@gmail.com), Maggio 2025.