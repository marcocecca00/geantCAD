# GeantCAD Modernization Status

## ✅ Completato

### 1. Migrazione Qt6
- ✅ CMakeLists.txt aggiornato per Qt6 (default)
- ✅ Supporto Qt5 mantenuto con `GEANTCAD_USE_QT5=ON`
- ✅ Tutti i riferimenti Qt5:: sostituiti con Qt6:: quando appropriato
- ✅ Compatibilità VTK verificata (QVTKOpenGLNativeWidget funziona con Qt6)
- ✅ Fix API Qt6: `QDropEvent::position()`, `QEnterEvent`, `QActionGroup`

### 2. View-Cube Interattivo
- ✅ Implementato view-cube usando `vtkOrientationMarkerWidget`
- ✅ Aggiunte funzioni per viste standard (Front, Back, Left, Right, Top, Bottom, Isometric)
- ✅ View-cube posizionato in bottom-right corner del viewport

### 3. Containerizzazione
- ✅ Dockerfile creato con Ubuntu 22.04
- ✅ Compilazione automatica VTK con Qt6 support
- ✅ docker-compose.yml per sviluppo semplificato
- ✅ .dockerignore per build efficiente

### 4. Documentazione
- ✅ README.md aggiornato con istruzioni Qt6 e Docker
- ✅ docs/QT6_MIGRATION.md aggiornato
- ✅ docs/COORDINATION.md aggiornato

### 5. Persistenza Impostazioni
- ✅ Build/Run dialog salva l'ultimo percorso progetto con QSettings
- ✅ Percorso build directory persistente

### 6. GDML Export Migliorato
- ✅ Esportazione rotazioni con Euler angles
- ✅ Riferimenti `<rotationref>` nei `<physvol>`

### 7. Outliner Migliorato
- ✅ Colonna visibilità con checkbox
- ✅ Icone per tipo di shape
- ✅ Toggle visibilità dal menu contestuale
- ✅ Supporto visibilità in VolumeNode (serializzato)
- ✅ Viewport rispetta stato visibilità

### 8. Export Mesh
- ✅ Export STL via VTK
- ✅ Export OBJ via VTK
- ✅ Menu File → Export con sottomenu
- ✅ MeshExporter class con supporto VTK condizionale

### 9. Database Materiali NIST
- ✅ NistMaterialDatabase class con 100+ materiali
- ✅ Categorie: Elements, Compounds, Gases, Metals, Plastics, Scintillators, Biological, Shielding, Optical
- ✅ Ricerca materiali per nome/formula
- ✅ Inspector aggiornato con combo box categorizzato
- ✅ Separatori visivi per categoria

### 10. Physics Panel Configurabile
- ✅ Opzioni EM: Standard, Option1-4, Penelope, Livermore
- ✅ Modelli adronici: FTFP_BERT, QGSP_BERT, QGSP_BIC, FTFP_INCLXX
- ✅ Toggle per: EM, Decay, Optical, Hadronic, Ions, Radioactive
- ✅ Step Limiter configurabile
- ✅ Production cuts per γ, e⁻, e⁺, protoni
- ✅ Preview della configurazione
- ✅ UI con scroll area per pannelli grandi

### 11. Sensitive Detectors e Scoring
- ✅ ScorerConfig struct per MultiFunctionalDetector
- ✅ Tipi scorer: energy_deposit, track_length, n_of_step, flux, dose
- ✅ Filtri particella e energia
- ✅ Scoring mesh opzionale con bins configurabili
- ✅ Serializzazione completa in JSON

### 12. UI/UX Improvements
- ✅ ThemeManager class con Dark/Light/System themes
- ✅ Palette professionale (VS Code/JetBrains inspired)
- ✅ Stylesheet completo per tutti i widget Qt
- ✅ PreferencesDialog con tabs per Appearance/Viewport/Grid/Geant4
- ✅ ShortcutsDialog con lista completa shortcut + ricerca
- ✅ Menu Edit → Preferences (Ctrl+,)
- ✅ Menu Help → Keyboard Shortcuts (Ctrl+/)
- ✅ Menu Help → About GeantCAD

## 🔄 In Corso / Prossimi Passi

### 1. Generazione Codice Geant4
- ⏳ DetectorConstruction con SD e scorers
- ⏳ PhysicsList con tutte le opzioni
- ⏳ ActionInitialization completo
- ⏳ Pannello preview codice generato

### 3. Import Formati CAD
- ⏳ Import STL
- ⏳ Import OBJ
- ⏳ STEP/IGES (richiede OpenCascade)

### 4. Strumenti Avanzati
- ⏳ Array/Pattern parametrici
- ⏳ Allineamento e distribuzione
- ⏳ Gruppo/Ungroup gerarchico

## Note Tecniche

- **Qt6**: Default, Qt5 supportato per retrocompatibilità
- **VTK**: Richiede compilazione con `VTK_QT_VERSION=6`
- **Docker**: Immagine include tutte le dipendenze pre-compilate
- **View-Cube**: Usa `vtkOrientationMarkerWidget` con `vtkCubeSource`
- **NIST Materials**: 100+ materiali catalogati da G4NistManager

## File Nuovi/Modificati in Questa Sessione

### Nuovi File
- `generator/include/MeshExporter.hh` - Export mesh header
- `generator/src/MeshExporter.cpp` - STL/OBJ export implementation
- `core/include/NistMaterialDatabase.hh` - NIST materials catalog
- `core/src/NistMaterialDatabase.cpp` - Database implementation

### File Modificati
- `CMakeLists.txt` - Nuovi source files, VTK link per generator
- `core/include/VolumeNode.hh` - Visibility flag, enhanced SD config
- `core/src/VolumeNode.cpp` - Serializzazione visibility e scorers
- `core/include/PhysicsConfig.hh` - Opzioni EM/Hadronic, cuts
- `core/src/PhysicsConfig.cpp` - Implementazione nuove opzioni
- `generator/src/GDMLExporter.cpp` - Export rotazioni
- `app/include/BuildRunDialog.hh` - Getter per directory
- `app/src/BuildRunDialog.cpp` - Getter implementation
- `app/src/MainWindow.cpp` - QSettings, menu export, visibility signal
- `app/include/Outliner.hh` - Colonna visibilità, getShapeIcon
- `app/src/Outliner.cpp` - UI visibilità, icone shape
- `app/src/Viewport3D.cpp` - Rispetta visibility node
- `app/src/Inspector.cpp` - NIST material combo categorizzato
- `app/include/PhysicsPanel.hh` - Nuovi widget physics
- `app/src/PhysicsPanel.cpp` - UI completa physics list

## Verifica Build

Dopo le modifiche, verificare:
1. ✅ Compilazione senza errori con Qt6
2. ✅ Viewport3D funziona correttamente
3. ✅ View-cube interattivo funzionante
4. ✅ Tutti i segnali/slot funzionano
5. ✅ Docker build funziona
6. ✅ Export GDML con rotazioni
7. ✅ Export STL/OBJ
8. ✅ Material selector funzionante
9. ✅ Physics panel completo
10. ✅ Visibility toggle in outliner
