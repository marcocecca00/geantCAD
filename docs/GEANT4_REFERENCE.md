# Geant4 Reference - Documentazione Ufficiale

## Documentazione Principale

**Book For Application Developers**: https://geant4.web.cern.ch/documentation/pipelines/master/bfad_html/ForApplicationDevelopers/

Questa è la "bibbia" per sviluppare applicazioni Geant4. Contiene:

### Sezioni Chiave per GeantCAD

#### 1. Detector Definition and Response
- **Geometry**: Come definire solidi, logical volumes, physical volumes
- **Material**: Come definire materiali (NIST, custom, mixtures)
- **Hits**: Come implementare Sensitive Detectors
- **GDML Import**: Importare geometrie da file GDML

#### 2. User Actions
- **G4VUserDetectorConstruction**: Costruzione geometria detector
- **G4VUserPhysicsList**: Configurazione physics processes
- **G4VUserPrimaryGeneratorAction**: Generazione particelle primarie
- **G4VUserActionInitialization**: Inizializzazione user actions

#### 3. Physics Processes
- **Electromagnetic Interactions**: Processi EM
- **Hadronic Interactions**: Processi hadronici
- **Optical Photon Processes**: Processi ottici (Cerenkov, scintillation)
- **Particle Decay**: Decadimenti

#### 4. Analysis
- **G4AnalysisManager**: Gestione output (ROOT, CSV, HDF5)
- **Histograms, Profiles, Ntuples**: Strutture dati per analisi

#### 5. Visualization
- **VTK Driver**: Visualizzazione con VTK
- **Qt Driver**: Visualizzazione con Qt
- **Controlling Visualization**: Comandi per controllo visualizzazione

### Esempi Geant4

Gli esempi Geant4 sono disponibili nel repository ufficiale e mostrano:
- Esempio B1-B5: Esempi base
- Esempi estesi: Esempi avanzati con diverse funzionalità

### Note per GeantCAD

1. **GDML Export**: GeantCAD esporta in GDML, che può essere importato direttamente in Geant4 usando `G4GDMLParser`

2. **Sensitive Detectors**: Implementare `G4VSensitiveDetector` per ogni tipo (CalorimeterSD, TrackerSD, OpticalSD)

3. **Physics Lists**: Usare `G4PhysListFactory` o costruire liste modulari

4. **Primary Generator**: Usare `G4ParticleGun` o `G4GeneralParticleSource` per generazione particelle

5. **Analysis**: Usare `G4AnalysisManager` per output ROOT/CSV con fallback automatico

## Link Utili

- **Documentazione principale**: https://geant4.web.cern.ch/documentation/
- **FAQ**: https://geant4.web.cern.ch/documentation/pipelines/master/faq_html/FAQ/
- **Esempi**: Disponibili nel repository Geant4

## Best Practices

1. Sempre verificare che i volumi non si sovrappongano
2. Usare materiali NIST quando possibile
3. Configurare correttamente i production cuts
4. Usare regioni per ottimizzare i cuts
5. Testare con esempi semplici prima di progetti complessi

