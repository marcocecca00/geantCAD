# Task List e Divisione Lavoro - GeantCAD

Questo documento divide le attività di sviluppo tra **Agent 3** (Core Functionality) e **Agent 4** (UI Integration), seguendo la struttura del progetto.

## Agent 3: Core Functionality 🔧

Agent 3 si occupa di tutto ciò che riguarda il **modello dati**, la **logica di business**, la **serializzazione** e la **generazione** di progetti Geant4. Nessuna dipendenza GUI.

### 1. Completare il Core Model (`core/`)

#### 1.1 Classi fondamentali

- [x] **VolumeNode**: Completare metodi get/set, ID unico (contatore statico), gestione parent/children
- [x] **Shape**: Implementare tutte le forme (Box, Tube, Sphere, Cone, Trd, Polycone, Polyhedra)
- [x] **Transform**: Completare trasformazioni (translation, rotation, scale, world transform)
- [x] **Material**: Factory NIST completa (30+ materiali), materiali custom con densità/Z/A, proprietà visive
- [x] **PhysicsConfig**: Configurazione completa physics lists (EM, Decay, Optical, Hadronic)
- [x] **OutputConfig**: Schema completo (EventSummary, StepHits, Custom), campi configurabili
- [x] **ParticleGunConfig**: Configurazione completa (energia uniforme/gaussiana, posizione punto/sfera/superficie, fascio)

#### 1.2 Serializzazione

- [x] Implementare `toJson()`/`fromJson()` per tutte le classi usando `nlohmann::json`
- [x] Clonazione oggetti per copiare volumi (usando JSON serialization)
- [x] Versioning formato (`version.json`)

#### 1.3 Command Pattern e Undo/Redo

- [x] Completare gerarchia `Command`: `CreateVolumeCommand`, `DeleteVolumeCommand`, `TransformVolumeCommand`, `DuplicateVolumeCommand`
- [x] `CommandStack`: Gestione undo/redo completa
- [x] Integrazione comandi con tutte le operazioni

### 2. Gestione File Progetto (`core/`)

- [x] **Serializzazione progetto**: `saveSceneToFile()` e `loadSceneFromFile()`
  - File `.geantcad` come directory contenente:
    - `scene.json`: geometria e gerarchia
    - `materials.json`: materiali custom
    - `physics.json`: configurazione physics
    - `output.json`: configurazione output
    - `particleGun.json`: configurazione particle gun
    - `version.json`: versione formato
- [x] **Versioning**: Verifica versione al load, supporto migrazioni future

### 3. Libreria Materiali (`core/`)

- [x] Factory materiali NIST (30+ materiali: aria, vuoto, acqua, piombo, silicio, metalli, gas, scintillatori, etc.)
- [x] Materiali custom: densità, Z, A, proprietà visive (colore)
- [x] Salvataggio materiali custom in `materials.json`

### 4. Primitive e Forme (`core/`)

- [X] Nuove forme: `Polycone`, `Polyhedra` (Cons non implementato - non presente in Geant4 GDML standard)
- [ ] Operazioni booleane (unione, sottrazione) - preparazione struttura dati
- [X] Factory per tutte le forme
- [X] Duplicazione volumi con copy number e offset automatico (implementato in DuplicateVolumeCommand)

### 5. Generator (`generator/`)

#### 5.1 TemplateEngine

- [x] Motore templating con sostituzione variabili
- [x] Preservazione codice utente tra marker `// ==== USER CODE BEGIN` / `// ==== USER CODE END`

#### 5.2 GDMLExporter

- [x] Conversione `SceneGraph` → GDML completo (tutte le forme supportate)
- [x] Include materiali, solidi, struttura gerarchica
- [x] Superfici ottiche (G4LogicalSkinSurface) - implementato export opticalsurface e skinsurface
- [x] Definizione setup

#### 5.3 Geant4ProjectGenerator

- [x] Creazione directory progetto da template `templates/geant4_project`
- [x] Generazione file:
  - `DetectorConstruction.cc` (con GDML loader)
  - `PhysicsList.cc` (basato su `PhysicsConfig`)
  - `PrimaryGeneratorAction.cc/hh` (completo con tutte le funzionalità ParticleGunConfig)
  - `CalorimeterSD.cc/hh` e `CalorimeterHit.cc/hh` (generati automaticamente se SD abilitati)
  - `TrackerSD.cc/hh` e `TrackerHit.cc/hh` (generati automaticamente se SD abilitati)
  - `OpticalSD.cc/hh` e `OpticalHit.cc/hh` (generati automaticamente se SD abilitati)
  - `EventAction.cc` e `RunAction.cc` (con integrazione OutputConfig)
  - Macro Geant4 (basate su `ParticleGunConfig`)
  - `CMakeLists.txt`
- [x] Uso `TemplateEngine` per generazione
- [x] Generazione automatica setup Sensitive Detectors basato su SceneGraph (tutti i tipi)
- [x] Generazione automatica configurazione PrimaryGeneratorAction da ParticleGunConfig
- [x] Generazione automatica codice output da OutputConfig

### 6. Sensitive Detectors (`generator/`)

- [x] Generazione classi SD separate:
  - `CalorimeterSD.cc/hh` (deriva da `G4VSensitiveDetector`) - template e generazione implementati
  - `CalorimeterHit.cc/hh` - template implementato
  - `TrackerSD.cc/hh` - template completo implementato
  - `TrackerHit.cc/hh` - template completo implementato
  - `OpticalSD.cc/hh` - template completo implementato
  - `OpticalHit.cc/hh` - template completo implementato
- [x] Registrazione SD in `DetectorConstruction` - generazione automatica codice setup per tutti i tipi SD
- [x] Integrazione con `OutputManager` per salvataggio dati - generazione codice EventAction/RunAction basato su OutputConfig

### 7. Testing (`core/`, `generator/`)

- [ ] Test unitari con Catch2:
  - Serializzazione (toJson/fromJson)
  - Comandi undo/redo
  - Conversione GDML
- [ ] CI/CD: GitHub Actions per build e test

---

## Agent 4: UI Integration 🎨

Agent 4 si occupa di tutto ciò che riguarda l'**interfaccia utente**, l'**interazione** con il viewport, i **pannelli** e l'**integrazione** con il core.

### 1. Viewport e Manipolazione (`app/`)

#### 1.1 Gizmo di Trasformazione

- [ ] Migliorare manipolatore viewport (traslazioni, rotazioni, scaling)
- [ ] Snap alla griglia
- [ ] Integrazione con `vtkBoxWidget` o gizmo custom
- [ ] Collegamento movimenti → `Transform` volumi
- [ ] Integrazione con sistema comandi (undo/redo)

#### 1.2 Controlli Camera e Griglia

- [ ] Migliorare orbit, pan, zoom
- [ ] Comandi: "frame selection", "reset view"
- [ ] Impostazione spaziatura griglia da menu
- [ ] Attivazione/disattivazione griglia

#### 1.3 Picking e Selezione

- [ ] ✅ **COMPLETATO**: Picking sicuro con controlli null
- [ ] Migliorare highlighting selezione
- [ ] Selezione multipla (opzionale)

### 2. Interfaccia Utente (`app/`)

#### 2.1 Inspector

- [ ] ✅ **BASE COMPLETATO**: Visualizzazione proprietà volume
- [ ] Completare modifica dimensioni forme (tutti i tipi)
- [ ] Modifica materiale con preview colore
- [ ] Modifica trasformazioni (translation, rotation, scale)
- [ ] Configurazione SensitiveDetector completa
- [ ] Configurazione superfici ottiche completa
- [ ] Tutte le modifiche devono essere undo/redo

#### 2.2 Outliner

- [ ] ✅ **BASE COMPLETATO**: Tree view con icon
- [ ] ✅ **COMPLETATO**: Context menu (Delete, Duplicate, Rename)
- [ ] Drag & drop per cambiare parent
- [ ] Rinominare nodi con doppio click
- [ ] Evidenziare volumi sensibili con icona/colore
- [ ] Selezione sincronizzata con viewport

#### 2.3 Toolbar

- [ ] ✅ **BASE COMPLETATO**: Tool per creare primitive
- [ ] Organizzare toolbar per categorie (Geometry, Create, Materials, Source, Analysis, Physics, Simulation)
- [ ] Tool attivo visibile (cursor, move, rotate…)
- [ ] Associazione ogni bottone → comando corrispondente
- [ ] Tool per duplicare e cancellare

#### 2.4 Dialogo Build & Run

- [ ] ✅ **BASE COMPLETATO**: Dialogo BuildRunDialog
- [ ] Migliorare: scelta directory build
- [ ] Esecuzione `cmake` e `make` con output in log
- [ ] Esecuzione app Geant4 generata
- [ ] Cattura output standard in pannello log

### 3. Strumenti Specializzati (`app/`)

#### 3.1 Strumento SiPM

- [ ] Tool dedicato per attaccare array SiPM alle facce volumi
- [ ] Picking facce con `vtkCellPicker`
- [ ] Calcolo orientamento SiPM (allineamento normale faccia)
- [ ] Impostazione dimensione e numero moduli (NxM)
- [ ] Visualizzazione array SiPM nel viewport

#### 3.2 Wrapping Ottico

- [X] ✅ **BASE COMPLETATO**: Configurazione superfici ottiche in Inspector
- [X] ✅ **COMPLETATO**: Preset completi: Tyvek, ESR, Black absorber
- [X] ✅ **COMPLETATO**: Parametri personalizzati: riflettività, sigma_alpha, model, finish
- [ ] Preview visiva superfici ottiche

### 4. Pannelli Configurazione (`app/`)

#### 4.1 PhysicsPanel

- [ ] ✅ **BASE COMPLETATO**: Toggle EM, Decay, Optical, Hadronic
- [ ] Selezione physics list completa
- [ ] Preview configurazione

#### 4.2 OutputPanel

- [ ] ✅ **BASE COMPLETATO**: Scelta file output (ROOT/CSV), schema, campi
- [ ] Migliorare UI per selezione campi
- [ ] Preview schema output

#### 4.3 ParticleGunPanel

- [ ] ✅ **BASE COMPLETATO**: Configurazione base
- [ ] Estendere: energia uniforme/gaussiana
- [ ] Posizione: punto/sfera/superficie
- [ ] Fascio con apertura
- [ ] Preview configurazione

### 5. Import e Export (`app/`)

#### 5.1 Import STEP/STL

- [ ] Dialogo import file STEP/STL
- [ ] Usare `vtkSTLReader` o OpenCascade per STEP
- [ ] Trattare volumi tessellati come non sensibili
- [ ] Rendering volumi importati nel viewport

### 6. Preferenze e Localizzazione (`app/`)

- [ ] Salvataggio preferenze con `QSettings`:
  - Layout GUI
  - Impostazioni griglia
  - Ultimi file aperti
- [ ] Supporto traduzioni:
  - File `.qm` con `QTranslator`
  - Preparazione struttura per i18n

### 7. Esempi e Documentazione (`examples/`, `docs/`)

- [ ] Creare progetti esempio:
  - Calorimetro semplice
  - Rivelatore a strati con SiPM
- [ ] Documentazione uso applicazione
- [ ] Guida processo generazione
- [ ] Istruzioni build
- [ ] Guida contributori

### 8. Licenza e Distribuzione

- [ ] Scegliere licenza open source (GPL-3.0, LGPL, MIT)
- [ ] Aggiungere file `LICENSE`
- [ ] Preparare packaging (AppImage, pacchetto Debian)

---

## Task Condivise (Coordinamento Richiesto) 🤝

Queste task richiedono coordinamento tra Agent 3 e Agent 4:

### A. Integrazione Comandi

- Agent 3: ✅ Implementa comandi nel core (COMPLETATO)
- Agent 4: Integra comandi nella UI (toolbar, inspector, outliner)

### B. Serializzazione Progetto

- Agent 3: ✅ Implementa `saveSceneToFile()`/`loadSceneFromFile()` (COMPLETATO)
- Agent 4: Integra in MainWindow (menu File → Save/Load)

### C. Generazione Progetto Geant4

- Agent 3: Implementa `Geant4ProjectGenerator`
- Agent 4: Integra in MainWindow (menu Generate → Build & Run)

### D. Sensitive Detectors

- Agent 3: Genera classi SD
- Agent 4: ✅ UI per assegnazione SD (Inspector), ✅ evidenziazione in Outliner con colore ciano e tooltip

---

## Priorità Sviluppo

### Fase 1: MVP Funzionante (Alta Priorità)

1. ✅ Core model base (VolumeNode, Shape, Transform, Material)
2. ✅ Serializzazione base (toJson/fromJson)
3. ✅ Command pattern base
4. ✅ GUI base (Viewport, Inspector, Outliner)
5. ✅ Generazione progetto Geant4 base

### Fase 2: Funzionalità Essenziali

1. ✅ Completare tutte le forme geometriche (COMPLETATO: Box, Tube, Sphere, Cone, Trd, Polycone, Polyhedra)
2. Gizmo manipolazione completo
3. Sensitive Detectors completo
4. ✅ Physics e Output configurazione completa (COMPLETATO)

### Fase 3: Funzionalità Avanzate

1. SiPM tool
2. Operazioni booleane
3. Import STEP/STL
4. Testing completo
5. Documentazione completa

---

## Convenzioni di Coordinamento

1. **File condivisi**:

   - `core/include/*.hh`, `core/src/*.cpp` → Agent 3
   - `app/include/*.hh`, `app/src/*.cpp` → Agent 4
   - `generator/include/*.hh`, `generator/src/*.cpp` → Agent 3
   - `CMakeLists.txt` → Coordinare modifiche
2. **Commit**:

   - Commit frequenti e piccoli
   - Messaggi chiari: `[Agent3]` o `[Agent4]` nel messaggio
   - Testare build prima di commit
3. **Coordinamento modifiche**:

   - Se modifichi file dell'altro agente → comunicare
   - Se aggiungi nuovo file → OK senza coordinamento
   - Se modifichi interfaccia esistente → coordinare
4. **Testing**:

   - Agent 3: Test unitari core/generator
   - Agent 4: Test manuale UI
   - Entrambi: Verificare che progetto compili

---

## Note Importanti

- ✅ = Completato
- ⏳ = In corso
- ⬜ = Da fare

---

## Riepilogo Task Completate - Agent 4

### Inspector (Completato)

- ✅ Preview colore materiale: Aggiunto `QLabel` con preview colore RGB accanto al combo box materiale
- ✅ Undo/redo completo: Tutte le modifiche (nome, materiale, trasformazioni, shape, SD, Optical) usano il sistema di comandi
- ✅ Modifica dimensioni forme: Supporto completo per Box, Tube, Sphere, Cone, Trd
- ✅ Modifica trasformazioni: Translation e rotation con undo/redo
- ✅ Configurazione SensitiveDetector: UI completa con type, collection name, copy number
- ✅ Configurazione superfici ottiche: UI completa con preset (Tyvek, ESR, Black), model, finish, reflectivity, sigma_alpha

### Outliner (Completato)

- ✅ Drag & drop: Implementato cambio parent tramite drag & drop con validazione (previene cicli)
- ✅ Rename inline: Editing inline con doppio click o F2, validazione nomi vuoti
- ✅ Evidenziazione SD: Volumi sensibili evidenziati in ciano con tooltip informativo
- ✅ Context menu: Delete, Duplicate, Rename funzionanti
- ✅ Selezione sincronizzata: Selezione sincronizzata con viewport

### Toolbar (Completato)

- ✅ Organizzazione categorie:
  - Categoria **Tools** (Manipulation): Select, Move, Rotate, Scale
  - Categoria **Create** (Primitives): Box, Tube, Sphere, Cone, Trd
  - Categoria **Edit** (Modify): Duplicate, Delete
- ✅ Tool attivo visibile: `QActionGroup` esclusivo con tool buttons checkable, Select tool attivo di default
- ✅ Tooltips informativi: Ogni tool ha tooltip descrittivo

### Styling e UX (Completato)

- ✅ Dark theme moderno: Stylesheet QSS completo con transizioni CSS
- ✅ Icone appropriate: Icone Qt standard appropriate per ogni azione
- ✅ Font tuning: Font system moderni (Inter, Segoe UI, Roboto) con fallback

### File Modificati

- `app/include/Inspector.hh` - Aggiunto preview colore, supporto command stack
- `app/src/Inspector.cpp` - Implementato preview colore, undo/redo completo
- `app/include/Outliner.hh` - Aggiunti metodi drag & drop
- `app/src/Outliner.cpp` - Implementato drag & drop, rename, evidenziazione SD
- `app/src/Toolbar.cpp` - Organizzato per categorie, tool attivo visibile
- `app/resources/stylesheets/modern-dark.qss` - Stylesheet completo con transizioni
- `app/src/CollapsibleGroupBox.cpp` - Rimosso stylesheet inline, integrato nel QSS globale
- `app/src/MainWindow.cpp` - Aggiunte icone ai menu

**Repository**: https://github.com/marcocecca00/geantCAD

**Documentazione coordinamento**: `docs/COORDINATION.md`
