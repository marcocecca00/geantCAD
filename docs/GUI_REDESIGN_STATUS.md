# GUI Redesign Status - GeantCAD v0.2.0

## ✅ Completato

### 1. Migrazione Qt6
- ✅ CMakeLists.txt aggiornato per supporto Qt6/Qt5 automatico
- ✅ Flag `GEANTCAD_PREFER_QT6` per selezione versione
- ✅ Compatibility layer per API Qt5/Qt6
- ✅ Build configuration summary nel cmake output

### 2. Layout Base MRADSIM-Style
- ✅ **ProjectManagerPanel**: Left panel con Project Manager (tree view) + Information panel
- ✅ **RightPanelContainer**: Right panel con tab verticali (Materials, Source, Analysis, Physics, Simulation)
- ✅ **MainWindow**: Layout riorganizzato
  - Left: Scene Hierarchy (250-350px)
  - Center: Viewport 3D (espandibile)
  - Right: Configuration Panels (300-450px)

### 3. Toolbar Categorizzata
- ✅ **History**: Undo/Redo con shortcuts
- ✅ **View**: Views dropdown, Frame, Reset
- ✅ **Tools**: Select, Move, Rotate, Scale (mutualmente esclusivi)
- ✅ **Shapes**: Add Shape dropdown con primitive
- ✅ **Edit**: Duplicate, Delete, Group/Ungroup
- ✅ **Analysis**: Measure, Clip tools

### 4. ViewCube Interattivo
- ✅ Widget 3D renderizzato con Qt
- ✅ Facce cliccabili per viste standard
- ✅ Rotazione drag per orientamento libero
- ✅ Integrazione con camera VTK
- ✅ Colori per assi (RGB -> XYZ)

### 5. Piani di Taglio
- ✅ **ClippingPlaneWidget**: Controllo piani X/Y/Z
- ✅ Slider per posizione
- ✅ Flip direction per ogni piano
- ✅ Enable/disable individuale
- ✅ Reset all planes

### 6. Strumenti di Misurazione
- ✅ **MeasurementTool**: Widget completo
- ✅ Modalità distanza (2 punti)
- ✅ Modalità angolo (3 punti)
- ✅ Modalità coordinate punto
- ✅ Lista misurazioni salvate
- ✅ Visualizzazione 3D delle misure (VTK actors)
- ✅ Delete/Clear measurements

### 7. History Panel
- ✅ **HistoryPanel**: Visualizzazione command stack
- ✅ Lista comandi con icone per tipo
- ✅ Highlight stato corrente
- ✅ Double-click per jump to state
- ✅ Undo/Redo buttons con shortcuts
- ✅ Clear history

### 8. Containerizzazione Docker
- ✅ **Dockerfile**: Ubuntu 22.04, Qt6, VTK, dependencies
- ✅ **docker-compose.yml**: X11 forwarding, volumes
- ✅ **.dockerignore**: Ottimizzazione build context
- ✅ Opzioni per Geant4 e ROOT (commentate, attivabili)

### 9. Documentazione
- ✅ README.md aggiornato con nuove features
- ✅ COORDINATION.md con convenzioni aggiornate
- ✅ GUI_REDESIGN_STATUS.md (questo file)

## 🔄 Da Fare (Priorità Alta)

### 1. Integrazione Nuovi Widget in MainWindow
- ⏳ Aggiungere ViewCube come overlay nel viewport
- ⏳ Collegare ClippingPlaneWidget al viewport VTK
- ⏳ Collegare MeasurementTool al picking VTK
- ⏳ Aggiungere HistoryPanel come dock widget o tab

### 2. Collegare Segnali Toolbar
- ⏳ View actions → camera VTK
- ⏳ Shape creation → scene graph
- ⏳ Analysis tools → widgets corrispondenti
- ⏳ Undo/Redo → command stack

### 3. Test Build Qt6
- ⏳ Verificare compilazione con Qt6
- ⏳ Test funzionalità VTK con Qt6
- ⏳ Fix eventuali API deprecated

## 🔄 Da Fare (Priorità Media)

### 1. Dark Theme Migliorato
- ⏳ Refinement colori per ViewCube
- ⏳ Styling consistente per nuovi widget
- ⏳ Theme switching (dark/light)

### 2. Viewport Miglioramenti
- ⏳ Grid più visibile con colori assi
- ⏳ Info overlay (stepsize, coordinates)
- ⏳ Antialiasing migliorato

### 3. Outliner Miglioramenti
- ⏳ Icone per tipo oggetto
- ⏳ Checkbox visibility
- ⏳ Context menu completo
- ⏳ Drag & drop per riordinare

## 📋 Da Fare (Priorità Bassa)

### 1. Import/Export CAD
- ⏳ STL import/export
- ⏳ OBJ import/export
- ⏳ STEP/IGES (richiede OpenCascade)

### 2. Array/Pattern Tools
- ⏳ Linear array
- ⏳ Polar array
- ⏳ Alignment tools

### 3. Polish UI
- ⏳ Animazioni transizione
- ⏳ Icon custom (non system)
- ⏳ Keyboard shortcuts cheatsheet
- ⏳ Preferences dialog

## File Creati/Modificati

### Nuovi File
- `app/include/ViewCube.hh` - ViewCube widget header
- `app/src/ViewCube.cpp` - ViewCube implementation
- `app/include/ClippingPlaneWidget.hh` - Clipping planes header
- `app/src/ClippingPlaneWidget.cpp` - Clipping planes implementation
- `app/include/MeasurementTool.hh` - Measurement tool header
- `app/src/MeasurementTool.cpp` - Measurement tool implementation
- `app/include/HistoryPanel.hh` - History panel header
- `app/src/HistoryPanel.cpp` - History panel implementation
- `Dockerfile` - Docker build configuration
- `docker-compose.yml` - Docker compose setup
- `.dockerignore` - Docker ignore patterns

### File Modificati
- `CMakeLists.txt` - Qt6 support, nuovi source files
- `core/include/CommandStack.hh` - Added history access methods
- `app/include/Toolbar.hh` - Enhanced categorized toolbar
- `app/src/Toolbar.cpp` - Toolbar implementation with categories
- `README.md` - Updated documentation
- `docs/COORDINATION.md` - Updated conventions
- `docs/GUI_REDESIGN_STATUS.md` - This file

## Note Tecniche

### ViewCube Rendering
- Usa QPainter per rendering 2D del cubo 3D
- Proiezione prospettica semplificata
- Painter's algorithm per z-sorting facce

### VTK Integration
- Conditional compilation con `GEANTCAD_NO_VTK`
- Smart pointers per memory management
- Callback pattern per manipulator updates

### Qt6 Compatibility
- `QOverload` per signal/slot
- `Qt6::OpenGLWidgets` component
- No `QFontMetrics::width()` (usa `horizontalAdvance()`)
