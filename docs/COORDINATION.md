# Coordinamento Sviluppo GeantCAD

## Overview

Linee guida per lo sviluppo di GeantCAD.

## Convenzioni

1. **Nuovi file**: OK senza coordinamento
2. **Modifiche file esistenti**: 
   - Se aggiunta pura → OK
   - Se modifica esistente → coordinare
3. **Build**: Testare sempre dopo modifiche (`cd build && cmake --build .`)
4. **Git**: Commit frequenti per sicurezza
5. **Codice**: Seguire stile esistente, commentare codice complesso

## Checklist Pre-Commit

- [ ] Progetto compila (`cd build && cmake --build .`)
- [ ] Nessun warning nuovo rilevante
- [ ] Funzionalità esistenti ancora funzionano
- [ ] Nuove funzionalità testate
- [ ] Codice formattato correttamente

## Struttura Progetto

- `core/`: Core model (SceneGraph, VolumeNode, Shape, Material, etc.)
- `app/`: Qt application (GUI, Viewport3D, Inspector, etc.)
- `generator/`: Geant4 project generator (GDMLExporter, TemplateEngine)
- `templates/`: Geant4 project templates
- `docs/`: Documentazione

## Note

- Il progetto usa Qt5 (non Qt6)
- VTK è opzionale (fallback se non disponibile)
- Python bindings sono opzionali (compilabili con `GEANTCAD_BUILD_PYTHON_BINDINGS=ON`)

