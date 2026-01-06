# Riorganizzazione Pannelli GUI

## Problema Attuale

La divisione attuale non ha molto senso:
- **Left**: ProjectManagerPanel (Outliner + Inspector)
- **Right**: RightPanelContainer (Materials, Source, Analysis, Physics, Simulation)

Problemi:
- Materials è separato da Inspector, ma dovrebbe essere parte delle proprietà oggetto
- Source/Physics/Output sono configurazioni di simulazione, non proprietà oggetto
- Inspector e Materials sono concettualmente collegati (proprietà dell'oggetto selezionato)

## Nuova Struttura Proposta

### Layout a 3 Colonne

**Left (20%)**: Scene Hierarchy
- Solo Outliner (albero scene)
- Focus sulla struttura gerarchica

**Center (50%)**: Viewport 3D
- Visualizzazione e interazione 3D
- Gizmo per trasformazioni

**Right (30%)**: Split Verticale
- **Top (60%)**: Properties Panel
  - Inspector completo (Name, Transform, Shape, Material, SD, Optical)
  - Material selector integrato
  - Tutte le proprietà dell'oggetto selezionato
  
- **Bottom (40%)**: Simulation Config Panel
  - Physics configuration
  - Particle Gun (Source)
  - Output/Analysis
  - Build & Run

## Vantaggi

1. **Separazione logica**: Proprietà oggetto vs Configurazione simulazione
2. **Material integrato**: Material è parte delle proprietà oggetto, non configurazione
3. **Più intuitivo**: Sinistra = struttura, Centro = vista, Destra = proprietà + simulazione
4. **Più spazio**: Outliner ha più spazio, Properties e Simulation sono separati

## Implementazione

1. Creare `PropertiesPanel` che unifica Inspector + Material selector
2. Creare `SimulationConfigPanel` che contiene Physics, ParticleGun, Output
3. Modificare `MainWindow::setupUI()` per nuovo layout
4. Rimuovere `ProjectManagerPanel` (sostituito da Outliner standalone)
5. Rimuovere `RightPanelContainer` (sostituito da PropertiesPanel + SimulationConfigPanel)

