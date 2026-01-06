# GUI Redesign - Stile MRADSIM

## Obiettivo
Riorganizzare la GUI per assomigliare a MRADSIM con layout professionale e dark theme.

## Struttura Layout Target

```
┌─────────────────────────────────────────────────────────────────┐
│ Menu Bar: File | Edit | View | ...                            │
├─────────────────────────────────────────────────────────────────┤
│ Toolbar: [Geometry] [Create] [Materials] [Source] [Analysis]  │
│          [Physics] [Simulation] [Run]                          │
├──────┬──────────────────────────────────────────────┬──────────┤
│      │                                              │          │
│ Left │           Central Viewport 3D               │  Right   │
│Panel │           (Main 3D Scene)                  │  Panel   │
│      │                                              │          │
│[Proj │                                              │ [Config] │
│ Mgr] │                                              │          │
│      │                                              │ Tab      │
│[Info]│                                              │ Vertical │
│      │                                              │          │
└──────┴──────────────────────────────────────────────┴──────────┘
```

## Componenti da Creare/Modificare

### 1. Left Panel (Project Manager + Information)
**File**: `app/include/ProjectManagerPanel.hh/cpp` (NUOVO)

**Struttura**:
- **Top**: Project Manager (tree view migliorato)
  - Mostra gerarchia SceneGraph
  - Icon per tipo oggetto
  - Checkbox per enable/disable
  - Context menu (delete, duplicate, etc.)
- **Bottom**: Information Panel
  - Proprietà oggetto selezionato
  - Material, Transform, etc.
  - Collapsible sections

**Features**:
- Collapsible/expandable sections
- Drag & drop per riordinare
- Multi-select support

### 2. Right Panel (Configuration Panels con Tab Verticali)
**File**: `app/include/RightPanelContainer.hh/cpp` (NUOVO)

**Tab Verticali** (stile MRADSIM):
- **Materials** → MaterialPanel (già PhysicsPanel può essere riorganizzato)
- **Source** → ParticleGunPanel
- **Analysis** → OutputPanel
- **Physics** → PhysicsPanel
- **Simulation** → BuildRunPanel (nuovo, wrapper BuildRunDialog)

**Features**:
- Tab verticali a destra
- Switch tra pannelli con click
- Icon per ogni tab
- Badge per notifiche (opzionale)

### 3. Top Toolbar Organizzata per Categorie
**File**: `app/include/CategoryToolbar.hh/cpp` (NUOVO) o modificare `Toolbar.hh`

**Categorie**:
- **Geometry**: View tools (orbit, pan, zoom, frame)
- **Create**: Primitives (Box, Tube, Sphere, Cone, Trd)
- **Materials**: Material selection/creation
- **Source**: Particle gun configuration
- **Analysis**: Output configuration
- **Physics**: Physics list configuration
- **Simulation**: Run, Build & Run

**Layout**:
- Ogni categoria ha dropdown o gruppo di icone
- Icon chiare e moderne
- Tooltip descrittivi

### 4. Dark Theme Migliorato
**File**: `app/resources/stylesheets/modern-dark.qss` (già esiste, migliorare)

**Colori MRADSIM-style**:
- Background: #1e1e1e o #2b2b2b
- Panel background: #252525
- Text: #e0e0e0
- Accent: #0078d4 (blue) o #00a8ff (cyan)
- Border: #404040
- Hover: #3a3a3a

### 5. Central Viewport
**File**: `app/src/Viewport3D.cpp` (già esiste, migliorare)

**Miglioramenti**:
- Grid più visibile
- Axes più grandi
- Info overlay (Stepsize, Plane Direction come MRADSIM)
- Coordinate system indicator migliorato

## Piano Implementazione

### Fase 1: Layout Base (Priorità Alta)
1. Creare `ProjectManagerPanel` (left panel)
2. Creare `RightPanelContainer` con tab verticali
3. Riorganizzare `MainWindow` layout
4. Testare layout base

### Fase 2: Toolbar Categorie (Priorità Alta)
1. Creare `CategoryToolbar` o modificare `Toolbar`
2. Organizzare azioni per categorie
3. Aggiungere icon (Qt built-in o custom)
4. Testare toolbar

### Fase 3: Dark Theme (Priorità Media)
1. Migliorare `modern-dark.qss`
2. Applicare a tutti i widget
3. Testare su tutti i componenti
4. Aggiustare colori per leggibilità

### Fase 4: Miglioramenti Viewport (Priorità Media)
1. Grid più visibile
2. Axes migliorati
3. Info overlay
4. Coordinate system indicator

### Fase 5: Polish (Priorità Bassa)
1. Animazioni subtle
2. Icon custom
3. Tooltip migliorati
4. Keyboard shortcuts

## File da Creare

### Nuovi Widget
- `app/include/ProjectManagerPanel.hh/cpp` - Left panel con Project Manager + Info
- `app/include/RightPanelContainer.hh/cpp` - Container per right panel con tab verticali
- `app/include/CategoryToolbar.hh/cpp` - Toolbar organizzata per categorie (opzionale, può modificare Toolbar esistente)
- `app/include/VerticalTabBar.hh/cpp` - Tab bar verticale custom (opzionale, può usare QTabBar ruotato)

### File da Modificare
- `app/src/MainWindow.cpp` - Riorganizzare layout
- `app/resources/stylesheets/modern-dark.qss` - Migliorare theme
- `app/src/Viewport3D.cpp` - Migliorare visualizzazione

## Note

- Mantenere compatibilità con codice esistente
- Riutilizzare widget esistenti (PhysicsPanel, OutputPanel, etc.)
- Focus su layout e organizzazione, non riscrivere tutto
- Testare su diverse risoluzioni

## Riferimenti

- MRADSIM GUI come riferimento visivo
- Qt Designer per prototipi rapidi (opzionale)
- QSS per styling consistente

