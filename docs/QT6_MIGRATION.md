# Migrazione Qt5 -> Qt6

## Stato Attuale

**Qt6 è ora supportato** tramite il flag CMake `GEANTCAD_PREFER_QT6=ON` (default).

Il sistema rileva automaticamente Qt6 o Qt5 e configura le librerie appropriate.

## Configurazione CMake

### Compilazione con Qt6 (Consigliato)

```bash
cmake .. -DGEANTCAD_PREFER_QT6=ON
```

### Compilazione con Qt5 (Fallback)

```bash
cmake .. -DGEANTCAD_PREFER_QT6=OFF
```

## Dipendenze Qt6

Ubuntu 22.04+:
```bash
sudo apt install qt6-base-dev qt6-base-dev-tools libqt6opengl6-dev libqt6openglwidgets6 qt6-tools-dev
```

Fedora:
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
```

## Cambiamenti API Rilevanti

### Componenti Qt

| Qt5 | Qt6 | Note |
|-----|-----|------|
| `Qt5::Core` | `Qt6::Core` | - |
| `Qt5::Widgets` | `Qt6::Widgets` | - |
| `Qt5::OpenGL` | `Qt6::OpenGL` | - |
| `Qt5::Gui` | `Qt6::Gui` | - |
| - | `Qt6::OpenGLWidgets` | **Nuovo in Qt6** |

### API Deprecate/Rimosse

| Qt5 | Qt6 | Migrazione |
|-----|-----|-----------|
| `QFontMetrics::width()` | `QFontMetrics::horizontalAdvance()` | Sostituire chiamate |
| `QString::null` | `QString()` | Usare costruttore default |
| `QDesktopWidget` | `QScreen` | Usare `QGuiApplication::screens()` |
| `QTextStream::operator<<(const char*)` | Richiede `QString` | Cast esplicito |

### QOverload per Signals/Slots

```cpp
// Funziona in Qt5 e Qt6
connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, &MyClass::onIndexChanged);
```

### QVTKOpenGLNativeWidget

- In Qt6, richiede `Qt6::OpenGLWidgets`
- VTK deve essere compilato con `VTK_QT_VERSION=6`
- Nessun cambiamento nel codice C++ richiesto

## VTK e Qt6

### Requisiti

Per usare VTK con Qt6, VTK deve essere compilato con:
```
-DVTK_QT_VERSION=6
-DVTK_GROUP_ENABLE_Qt=YES
```

### Verifica Compatibilità

CMake verifica automaticamente la compatibilità VTK-Qt e mostra warning se rileva mismatch.

### Build VTK Manuale per Qt6

```bash
git clone https://github.com/Kitware/VTK.git
cd VTK && mkdir build && cd build

cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DVTK_QT_VERSION=6 \
    -DVTK_GROUP_ENABLE_Qt=YES \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES \
    -DVTK_BUILD_TESTING=OFF

ninja && sudo ninja install
```

## Docker

Il Dockerfile include opzione per build VTK con Qt6 (commentata di default poiché Ubuntu 22.04 ha VTK9 nei repo).

## Checklist Migrazione

### Completato
- [x] CMakeLists.txt supporta Qt6 e Qt5
- [x] Auto-detection versione Qt
- [x] Componente OpenGLWidgets per Qt6
- [x] Compatibility layer per API comuni

### Verificare
- [ ] `QFontMetrics::width()` → `horizontalAdvance()`
- [ ] VTK compilato con Qt6 (se applicabile)
- [ ] Test funzionale completo

## Troubleshooting

### Errore: Qt6 non trovato

```
Could not find Qt6
```

Soluzione: Installare Qt6 development packages o usare `-DGEANTCAD_PREFER_QT6=OFF`

### Errore: VTK Qt version mismatch

```
VTK may be compiled with a different Qt version
```

Soluzione: Ricompilare VTK con `-DVTK_QT_VERSION=6` o usare Qt5

### Errore: horizontalAdvance not found

```
'horizontalAdvance' is not a member of 'QFontMetrics'
```

Soluzione: Qt5 < 5.11 non ha questo metodo. Usare Qt5.15+ o aggiungere compat wrapper.

## Riferimenti

- [Qt5 to Qt6 Porting Guide](https://doc.qt.io/qt-6/portingguide.html)
- [API Changes in Qt6](https://doc.qt.io/qt-6/qtcore-changes-qt6.html)
- [VTK Qt Integration](https://vtk.org/doc/nightly/html/md__builds_gitlab_kitware_sciviz_ci_Documentation_Doxygen_QVTKOpenGLNativeWidget.html)
