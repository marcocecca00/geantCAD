# Documentazione GeantCAD

## Documenti Disponibili

### 🔄 Coordinamento
- **`COORDINATION.md`** - Linee guida per lo sviluppo
  - Convenzioni di codice
  - Checklist pre-commit
  - Struttura progetto

## Struttura Progetto

```
geantCAD/
├── core/          # Core model (SceneGraph, VolumeNode, Shape, Material, etc.)
├── app/           # Qt application (GUI, Viewport3D, Inspector, etc.)
├── generator/     # Geant4 project generator (GDMLExporter, TemplateEngine)
├── templates/     # Geant4 project templates
└── docs/          # Documentazione
```

## Tecnologie

- **Qt5**: GUI framework
- **VTK**: Viewport 3D (opzionale)
- **C++17**: Linguaggio principale
- **Python**: Bindings opzionali (pybind11)
- **CMake**: Build system

## Build

Vedi `README.md` nella root del progetto per istruzioni di build.

