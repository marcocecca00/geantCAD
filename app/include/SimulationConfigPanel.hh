#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include "PhysicsPanel.hh"
#include "OutputPanel.hh"
#include "ParticleGunPanel.hh"
#include "BuildRunDialog.hh"

namespace geantcad {

/**
 * Simulation Config Panel: Contiene tutte le configurazioni di simulazione
 * Physics, Particle Gun (Source), Output/Analysis, Build & Run
 */
class SimulationConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SimulationConfigPanel(QWidget* parent = nullptr);
    ~SimulationConfigPanel();
    
    PhysicsPanel* getPhysicsPanel() const { return physicsPanel_; }
    OutputPanel* getOutputPanel() const { return outputPanel_; }
    ParticleGunPanel* getParticleGunPanel() const { return particleGunPanel_; }

signals:
    void physicsConfigChanged();
    void outputConfigChanged();
    void particleGunConfigChanged();

private slots:
    void onBuildRun();

private:
    void setupUI();
    
    QTabWidget* tabWidget_;
    
    PhysicsPanel* physicsPanel_;
    OutputPanel* outputPanel_;
    ParticleGunPanel* particleGunPanel_;
    QWidget* buildRunWidget_;
};

} // namespace geantcad

