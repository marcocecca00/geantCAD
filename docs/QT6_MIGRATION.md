# Migrazione Qt5 -> Qt6

## Stato Attuale

**Qt5 è ancora utilizzato** perché VTK è tipicamente compilato con Qt5.

Per utilizzare Qt6 completamente, è necessario:
1. Ricompilare VTK con Qt6 abilitato
2. Aggiornare CMakeLists.txt per usare Qt6

## Cambiamenti Preparati (non ancora applicati)

### CMakeLists.txt
- `find_package(Qt5 ...)` → `find_package(Qt6 ...)`
- `Qt5::Core` → `Qt6::Core`
- `Qt5::Widgets` → `Qt6::Widgets`
- `Qt5::OpenGL` → `Qt6::OpenGL`
- `Qt5::Gui` → `Qt6::Gui`

### QVTKOpenGLNativeWidget
- In Qt6, `QVTKOpenGLNativeWidget` è disponibile tramite VTK::GUISupportQt
- Nessun cambiamento necessario nel codice, VTK gestisce la compatibilità

### QOverload
- In Qt6, molti overload sono risolti automaticamente
- `QOverload<int>::of(&QComboBox::currentIndexChanged)` può essere semplificato
- Tuttavia, per compatibilità manteniamo QOverload dove necessario

### Q_DECLARE_METATYPE e qRegisterMetaType
- Funzionano identicamente in Qt6
- Nessun cambiamento necessario

## Note

- VTK 9.1+ supporta sia Qt5 che Qt6
- QVTKOpenGLNativeWidget funziona con entrambe le versioni
- La migrazione è principalmente un cambio di namespace Qt5:: → Qt6::

## Verifica

Dopo la migrazione, verificare:
1. Compilazione senza errori
2. Viewport3D funziona correttamente
3. Tutti i segnali/slot funzionano
4. Nessun crash all'avvio

