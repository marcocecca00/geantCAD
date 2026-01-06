# GeantCAD

**GeantCAD** è un editor CAD-like interattivo per costruire geometrie di detector e generare automaticamente progetti Geant4 completi.

![GeantCAD Screenshot](docs/screenshot.png)

## ✨ Caratteristiche Principali

### Editing 3D
- **Editor 3D interattivo**: viewport VTK con controlli CAD-like (orbit, pan, zoom, gizmo)
- **ViewCube interattivo**: orientamento rapido della scena con clic sul cubo 3D
- **Scene graph gerarchico**: volumi organizzati in struttura madre-figlio
- **Manipolazione gizmo**: move, rotate, scale con feedback visivo

### Primitive Geometriche
- Box, Tube, Sphere, Cone, Trd (trapezoid)
- Supporto per future primitive aggiuntive

### Strumenti di Analisi (Nuovo!)
- **Piani di taglio**: sezione dinamica X/Y/Z per analisi interne
- **Misurazione**: distanze, angoli, coordinate punti
- **History Panel**: visualizzazione e navigazione undo/redo

### Configurazione Simulazione
- **Sensitive Detectors**: assegnazione e generazione automatica classi SD
- **SiPM tool**: attach-to-face con supporto array NxM
- **Wrapping ottico**: superfici ottiche (G4LogicalSkinSurface) con preset
- **Physics configurabile**: toggle EM, Decay, Optical, Hadronic
- **Output ROOT/CSV**: configurabile da GUI con fallback automatico

### Generazione Codice
- **Generazione progetto Geant4**: completo e compilabile con CMake
- **Export GDML**: per interoperabilità
- **Python bindings**: per automazione e scripting

## 🏗️ Architettura

```
geantcad/
├── core/           # Core model (SceneGraph, VolumeNode, Shape, Material)
├── app/            # Qt GUI (MainWindow, Viewport3D, Panels)
├── generator/      # Export GDML e generazione progetto Geant4
├── templates/      # Template progetto Geant4
└── docs/           # Documentazione
```

## 📦 Dipendenze

### Richieste
- **CMake** ≥ 3.20
- **Qt** 6 (consigliato) o Qt 5.15+ (Core, Widgets, OpenGL, Gui, OpenGLWidgets)
- **nlohmann_json** ≥ 3.2.0

### Opzionali
- **VTK** 9+ con GUISupportQt (per viewport 3D completo)
- **Python** 3 + pybind11 (per Python bindings)
- **ROOT** (per output ROOT)
- **Geant4** 11.x (per eseguire progetti generati)

## 🔧 Build

### Build Standard

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DGEANTCAD_PREFER_QT6=ON
cmake --build . -j$(nproc)
```

### Opzioni CMake

| Opzione | Default | Descrizione |
|---------|---------|-------------|
| `GEANTCAD_PREFER_QT6` | ON | Preferisce Qt6 su Qt5 |
| `GEANTCAD_USE_ROOT` | OFF | Abilita supporto ROOT |
| `GEANTCAD_BUILD_PYTHON_BINDINGS` | ON | Compila Python bindings |
| `GEANTCAD_BUILD_EXAMPLES` | ON | Compila esempi |

### Con Docker (Consigliato)

```bash
# Build container
docker build -t geantcad:latest .

# Esegui con X11 forwarding (Linux)
xhost +local:docker
docker run -it --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix geantcad:latest

# Oppure usa docker-compose
docker-compose up
```

## 🚀 Uso

### Avvio

```bash
./geantcad
```

### Workflow Base

1. **Crea volumi**: dalla toolbar "Shapes" → Add Shape
2. **Seleziona e trasforma**: usa Select/Move/Rotate/Scale dalla toolbar
3. **Modifica proprietà**: pannello Properties a destra
4. **Assegna materiali**: pannello Materials
5. **Configura simulazione**: pannelli Physics, Source, Output
6. **Genera progetto**: Menu Generate → Generate Geant4 Project

### Scorciatoie Tastiera

| Tasto | Azione |
|-------|--------|
| S | Tool Select |
| G | Tool Move |
| R | Tool Rotate |
| T | Tool Scale |
| F | Frame Selection |
| Home | Reset View |
| Ctrl+Z | Undo |
| Ctrl+Shift+Z | Redo |
| Delete | Elimina selezione |
| Ctrl+G | Mostra/nascondi griglia |

### ViewCube

Il ViewCube nell'angolo della viewport permette di:
- **Clic su faccia**: vista standard (Front, Top, Right, ecc.)
- **Drag**: rotazione libera della vista
- **Clic su spigolo**: vista isometrica

## 📁 Formato Progetto

I file `.geantcad` sono archivi JSON contenenti:
- `scene.json`: geometria e gerarchia
- `materials.json`: materiali definiti
- `physics.json`: configurazione physics
- `output.json`: configurazione output
- `version.json`: versione formato

## 🔄 Safe Regeneration

I file generati supportano **marker regions** per preservare codice custom:

```cpp
// ==== USER CODE BEGIN <tag>
// ... codice personalizzato ...
// ==== USER CODE END <tag>
```

Il generator preserva il contenuto tra i marker durante la rigenerazione.

## 🐍 Python API

```python
import geantcad_python as gcad

# Crea scene
scene = gcad.SceneGraph()

# Aggiungi box
box = gcad.makeBox(100.0, 50.0, 50.0)
node = scene.createVolume("MyBox", box, gcad.Material.makeAir())

# Esporta GDML
exporter = gcad.GDMLExporter()
exporter.export(scene, "output.gdml")
```

## 📚 Documentazione

- [COORDINATION.md](docs/COORDINATION.md) - Linee guida sviluppo
- [AGENT_TASKS.md](docs/AGENT_TASKS.md) - Task divisione lavoro
- [GUI_REDESIGN.md](docs/GUI_REDESIGN.md) - Piano redesign UI
- [QT6_MIGRATION.md](docs/QT6_MIGRATION.md) - Note migrazione Qt6
- [GEANT4_REFERENCE.md](docs/GEANT4_REFERENCE.md) - Riferimento Geant4

## 🤝 Contribuire

1. Fork del repository
2. Crea branch per feature (`git checkout -b feature/nuova-feature`)
3. Segui le convenzioni in `COORDINATION.md`
4. Testa le modifiche (`cd build && cmake --build .`)
5. Crea Pull Request

## 📝 Changelog

### v0.2.0 (In sviluppo)
- ✨ Migrazione a Qt6 (con fallback Qt5)
- ✨ ViewCube interattivo per orientamento vista
- ✨ Piani di taglio X/Y/Z
- ✨ Strumenti di misurazione (distanza, angolo, punto)
- ✨ History Panel con visualizzazione undo/redo
- ✨ Toolbar riorganizzata con categorie
- ✨ Docker support completo
- 🔧 CMake modernizzato

### v0.1.0
- Versione iniziale
- Editor 3D con VTK
- Generazione progetti Geant4
- Export GDML

## 📄 Licenza

[Da definire]

## 🙏 Ringraziamenti

- [Qt Project](https://www.qt.io/)
- [VTK](https://vtk.org/)
- [Geant4 Collaboration](https://geant4.web.cern.ch/)
- Ispirato da FreeCAD, Mayo, e altri CAD open source
