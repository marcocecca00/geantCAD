# Coordinamento Sviluppo GeantCAD

## Overview

Linee guida per lo sviluppo di GeantCAD. Per la divisione dettagliata del lavoro, vedere `docs/AGENT_TASKS.md`.

## Stack Tecnologico

| Componente | Tecnologia | Versione |
|------------|-----------|----------|
| Linguaggio | C++ | 17 |
| Build | CMake | ≥ 3.20 |
| GUI Framework | Qt | 6 (preferito), 5.15+ (fallback) |
| 3D Viewport | VTK | 9+ con GUISupportQt |
| Serializzazione | nlohmann_json | ≥ 3.2.0 |
| Python Bindings | pybind11 | 2.11+ |

## Struttura Progetto

```
geantcad/
├── core/                    # Core model (no GUI)
│   ├── include/            # Headers pubblici
│   └── src/                # Implementazioni
├── app/                     # Qt application (GUI)
│   ├── include/            # Headers Qt widgets
│   ├── src/                # Implementazioni widgets
│   └── resources/          # QRC, stylesheets, icons
├── generator/               # Code generation
│   ├── include/
│   ├── src/
│   └── bindings/           # Python bindings
├── templates/               # Geant4 project templates
└── docs/                    # Documentazione
```

## Convenzioni Codice

### Naming

| Tipo | Convenzione | Esempio |
|------|-------------|---------|
| Classe | PascalCase | `ViewCube`, `MeasurementTool` |
| Metodo/Funzione | camelCase | `updateScene()`, `getCamera()` |
| Variabile | camelCase | `gridSpacing`, `currentNode` |
| Membro privato | camelCase + underscore | `sceneGraph_`, `renderer_` |
| Costante | UPPER_SNAKE | `MAX_HISTORY_SIZE` |
| File header | PascalCase.hh | `ViewCube.hh` |
| File source | PascalCase.cpp | `ViewCube.cpp` |

### Qt Specifiche

- Signals: verbo al passato o evento (`clicked`, `selectionChanged`, `viewOrientationRequested`)
- Slots: prefisso `on` per handler (`onItemClicked`, `onPlaneToggled`)
- Qt Containers preferiti per API Qt, STL per core logic

### Documentazione Codice

```cpp
/**
 * @brief Breve descrizione della classe/funzione
 * 
 * Descrizione estesa se necessaria.
 * 
 * @param nome Descrizione parametro
 * @return Descrizione valore di ritorno
 */
```

## Componenti UI

### Widget Principali

| Widget | File | Descrizione |
|--------|------|-------------|
| `MainWindow` | MainWindow.* | Finestra principale, layout, menu |
| `Viewport3D` | Viewport3D.* | Viewport VTK con interazione 3D |
| `ViewCube` | ViewCube.* | Cubo orientamento interattivo |
| `Outliner` | Outliner.* | Tree view scene graph |
| `Inspector` | Inspector.* | Proprietà oggetto selezionato |
| `Toolbar` | Toolbar.* | Barra strumenti categorizzata |

### Pannelli Destri

| Pannello | Tab | Descrizione |
|----------|-----|-------------|
| `PhysicsPanel` | Materials/Physics | Configurazione fisica |
| `ParticleGunPanel` | Source | Sorgente particelle |
| `OutputPanel` | Analysis | Output dati |
| `HistoryPanel` | - | Undo/redo history |

### Widget Analisi

| Widget | Descrizione |
|--------|-------------|
| `ClippingPlaneWidget` | Controllo piani di taglio X/Y/Z |
| `MeasurementTool` | Misurazione distanze, angoli, punti |

## Build e Test

### Build Base

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

### Build con Qt6

```bash
cmake .. -DGEANTCAD_PREFER_QT6=ON
```

### Docker Build

```bash
docker build -t geantcad:latest .
docker run -it --rm geantcad:latest
```

## Checklist Pre-Commit

- [ ] Progetto compila senza errori (`cmake --build .`)
- [ ] Nessun warning nuovo rilevante
- [ ] Funzionalità esistenti verificate
- [ ] Nuove funzionalità testate manualmente
- [ ] Codice formattato correttamente
- [ ] Headers inclusi correttamente (no dipendenze cicliche)
- [ ] Documentazione aggiornata se necessario

## Workflow Git

### Branch Naming

| Tipo | Pattern | Esempio |
|------|---------|---------|
| Feature | `feature/descrizione` | `feature/viewcube-widget` |
| Bugfix | `fix/descrizione` | `fix/vtk-crash` |
| Refactor | `refactor/descrizione` | `refactor/qt6-migration` |
| Docs | `docs/descrizione` | `docs/update-readme` |

### Commit Messages

```
type(scope): breve descrizione

Corpo del commit se necessario.
Spiega il *cosa* e il *perché*, non il *come*.

Refs: #issue_number
```

Tipi: `feat`, `fix`, `refactor`, `docs`, `style`, `test`, `chore`

## VTK Integration Notes

### Conditional Compilation

```cpp
#ifndef GEANTCAD_NO_VTK
    // Codice VTK
    vtkSmartPointer<vtkActor> actor = ...;
#else
    // Fallback senza VTK
#endif
```

### Smart Pointers

Usare sempre `vtkSmartPointer<T>` per oggetti VTK:

```cpp
vtkSmartPointer<vtkRenderer> renderer_ = vtkSmartPointer<vtkRenderer>::New();
```

### Qt-VTK Widget

- `QVTKOpenGLNativeWidget` per Qt6
- Impostare `QSurfaceFormat::setDefaultFormat()` prima di creare QApplication

## Risorse e Riferimenti

- [Qt Documentation](https://doc.qt.io/)
- [VTK Documentation](https://vtk.org/doc/)
- [Geant4 Documentation](https://geant4.web.cern.ch/documentation)
- [FreeCAD Source](https://github.com/FreeCAD/FreeCAD) - Riferimento UI
- [Mayo Source](https://github.com/fougue/mayo) - Riferimento CAD Qt

## Contatti

Per domande sullo sviluppo, aprire issue su GitHub o contattare il maintainer.
