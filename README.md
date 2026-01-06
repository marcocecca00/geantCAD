# GeantCAD

**GeantCAD** è un editor CAD-like interattivo per costruire geometrie di detector e generare automaticamente progetti Geant4 completi.

## Caratteristiche principali

- **Editor 3D interattivo**: costruzione geometria con controlli CAD-like (orbit, pan, zoom, gizmo)
- **Scene graph gerarchico**: volumi organizzati in struttura madre-figlio
- **Primitive geometriche**: Box, Tube, Sphere, Cone, Trd
- **Sensitive Detectors**: assegnazione e generazione automatica classi SD
- **SiPM tool**: attach-to-face con supporto array NxM
- **Wrapping ottico**: superfici ottiche (G4LogicalSkinSurface) con preset
- **Physics configurabile**: toggle EM, Decay, Optical, Hadronic
- **Output ROOT/CSV**: configurabile da GUI con fallback automatico
- **Generazione progetto Geant4**: completo e compilabile con CMake

## Architettura

Il progetto è organizzato in 4 layer separati:

1. **Core Model** (`core/`): Scene graph, serializzazione, command stack (no GUI)
2. **Editor/GUI** (`app/`): Interfaccia Qt con viewport VTK, outliner, inspector
3. **Generator** (`generator/`): Export GDML e generazione progetto Geant4
4. **Templates** (`templates/`): Template progetto Geant4

## Scelte tecnologiche

- **Linguaggio**: C++17
- **GUI**: Qt 5
- **Viewport 3D**: VTK + Qt (scelta per robustezza picking/interaction CAD-grade)
- **Build**: CMake 3.20+
- **Target Geant4**: 11.x
- **Export**: GDML (MVP) + backend native (futuro)

## Dipendenze

- Qt 5 (Core, Widgets, OpenGL, Gui)
- VTK 9+ (per viewport 3D, opzionale)
- nlohmann_json (per serializzazione)
- Geant4 11.x (per progetti generati)
- ROOT (opzionale, per output)
- Python 3 + pybind11 (opzionale, per Python bindings)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

Con ROOT:
```bash
cmake .. -DGEANTCAD_USE_ROOT=ON
```

## Uso

```bash
./geantcad
```

1. Crea volumi dalla toolbar
2. Seleziona e modifica proprietà nell'Inspector
3. Assegna materiali e SensitiveDetectors
4. Configura physics e output
5. Genera progetto Geant4 con "Generate"

## Formato progetto

File progetto: `*.geantcad` (JSON) contenente:
- `scene.json`: geometria e gerarchia
- `materials.json`: materiali
- `physics.json`: configurazione physics
- `output.json`: configurazione output
- `version.json`: versione formato

## Safe regeneration

I file generati usano **marker regions** per preservare codice custom:

```cpp
// ==== USER CODE BEGIN <tag>
// ... codice utente ...
// ==== USER CODE END <tag>
```

Il generator preserva il contenuto tra i marker durante la rigenerazione.

## Esempi

La cartella `examples/` è disponibile per progetti esempio (attualmente vuota).

## Documentazione

- `docs/COORDINATION.md`: Coordinamento agenti di sviluppo

## Licenza

[Da definire]
