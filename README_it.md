# EventStateMachine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Una libreria flessibile e potente per implementare macchine a stati event-driven su ESP8266 e ESP32.

## Caratteristiche

- **Gestione avanzata degli eventi**: supporta callback multipli per ogni evento di stato (entrata, uscita, durante)
- **Timeout configurabili**: possibilità di impostare più timeout per stato con callback specifici
- **Gestori globali di transizione**: hook personalizzabili prima e dopo il cambio di stato
- **Facile integrazione con storage persistente**: esempio di salvataggio e recupero dello stato da memoria flash
- **Gestione robusta degli errori**: verifica della validità degli stati e dei callback
- **Completamente basata su C++**: utilizza caratteristiche moderne come vector e functional

## Requisiti

- Scheda ESP8266 o ESP32
- Arduino IDE 1.8.0 o superiore
- (Opzionale) LittleFS per l'esempio di persistenza dello stato

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

Dimostra l'utilizzo base della macchina a stati con tre stati e callback essenziali. Controlla il LED integrato in base allo stato e risponde ai comandi seriali.

### MultipleCallbacks

Illustra come utilizzare callback multipli per ogni tipo di evento di stato. Include esempi di:
- Più callback di entrata per uno stato
- Più callback durante uno stato
- Più callback di uscita da uno stato
- Più timeout con diverse durate

### StateRecovery

Mostra come implementare la persistenza dello stato utilizzando LittleFS. Permette di:
- Salvare ogni transizione di stato in un file di log
- Recuperare l'ultimo stato salvato dopo un riavvio
- Visualizzare la cronologia delle transizioni

## Considerazioni di Design

### Prestazioni

La libreria utilizza vector di C++ per gestire i callback multipli, il che offre flessibilità ma richiede una certa quantità di memoria. Su dispositivi con memoria molto limitata, considera di utilizzare solo i callback necessari.

### Sincronizzazione

I timeout sono gestiti utilizzando la libreria Ticker, che funziona in modo asincrono. Questo significa che i callback di timeout possono essere eseguiti in qualsiasi momento, anche durante altre operazioni.

## Risoluzione dei Problemi

### Timeout non scattano

- Verifica che `update()` venga chiamato regolarmente nel loop
- Controlla che non ci siano delay() lunghi che bloccano l'esecuzione
- Assicurati che il callback del timeout non sia nullo

### Callback non eseguiti

- Assicurati che lo stato sia valido (< numStates)
- Verifica che i callback siano stati registrati correttamente
- Controlla che l'istanza della macchina a stati non venga ricreata

## Contribuire

I contributi sono benvenuti! Sentiti libero di aprire issue o inviare pull request su GitHub.

## License

Questa libreria è rilasciata sotto licenza MIT. Vedi il file LICENSE per i dettagli.

## Credits

Creata da Corrado Casoni (corrado.casoni@gmail.com), Maggio 2025.
