# GeantCAD Modernization Status

## ✅ Completato

### 1. Migrazione Qt6
- ✅ CMakeLists.txt aggiornato per Qt6 (default)
- ✅ Supporto Qt5 mantenuto con `GEANTCAD_USE_QT5=ON`
- ✅ Tutti i riferimenti Qt5:: sostituiti con Qt6:: quando appropriato
- ✅ Compatibilità VTK verificata (QVTKOpenGLNativeWidget funziona con Qt6)

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

## 🔄 In Corso / Prossimi Passi

### 1. Refactoring UI Principale
- ⏳ Massimizzare spazio viewport centrale
- ⏳ Riorganizzare layout per pattern CAD moderni
- ⏳ Migliorare indicatore assi XYZ

### 2. Toolbar Riorganizzazione
- ⏳ Spostare toolbar vicino al viewport
- ⏳ Categorie strumenti (Geometry, Create, Materials, etc.)
- ⏳ Icone intuitive con tooltip descrittivi

### 3. Outliner Miglioramenti
- ⏳ Filtri e ricerca
- ⏳ Gestione livelli/gruppi
- ⏳ Checkbox hide/show per elementi

### 4. Pannelli Proprietà
- ⏳ Consolidare in interfaccia a tab
- ⏳ Sezioni collassabili per ridurre clutter
- ⏳ Pattern "Model/Tasks" stile FreeCAD

### 5. Strumenti Avanzati
- ⏳ Piani di taglio
- ⏳ Strumenti di misurazione (distanze, angoli, aree)
- ⏳ Esplosione scena per analisi strutture complesse

### 6. Funzionalità CAD
- ⏳ Array/pattern parametrici
- ⏳ Duplicazione avanzata
- ⏳ Gruppo/ungroup
- ⏳ Allineamento e distribuzione

### 7. Import/Export
- ⏳ Supporto formati CAD standard (STEP, IGES, STL, OBJ)
- ⏳ Integrazione con OpenCascade o plugin

## Note Tecniche

- **Qt6**: Default, Qt5 supportato per retrocompatibilità
- **VTK**: Richiede compilazione con `VTK_QT_VERSION=6`
- **Docker**: Immagine include tutte le dipendenze pre-compilate
- **View-Cube**: Usa `vtkOrientationMarkerWidget` con `vtkCubeSource`

## Verifica Build

Dopo le modifiche, verificare:
1. ✅ Compilazione senza errori con Qt6
2. ✅ Viewport3D funziona correttamente
3. ✅ View-cube interattivo funzionante
4. ✅ Tutti i segnali/slot funzionano
5. ✅ Docker build funziona

