#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QLabel>

namespace geantcad {

class BuildRunDialog : public QDialog {
    Q_OBJECT

public:
    BuildRunDialog(QWidget* parent = nullptr);
    ~BuildRunDialog();
    
    void setProjectDirectory(const QString& dir);
    void setBuildDirectory(const QString& dir);
    QString getProjectDirectory() const;
    QString getBuildDirectory() const;

private slots:
    void onBrowse();
    void onBrowseBuildDir();
    void onBuild();
    void onRun();
    void onStop();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    void setupUI();
    void appendOutput(const QString& text, bool isError = false);
    void setButtonsEnabled(bool enabled);
    
    QLineEdit* projectDirEdit_;
    QPushButton* browseButton_;
    QLineEdit* buildDirEdit_;
    QPushButton* browseBuildDirButton_;
    QPushButton* buildButton_;
    QPushButton* runButton_;
    QPushButton* stopButton_;
    QPushButton* closeButton_;
    QTextEdit* consoleOutput_;
    QProgressBar* progressBar_;
    
    QProcess* buildProcess_;
    QProcess* runProcess_;
    bool isBuilding_;
    bool isRunning_;
};

} // namespace geantcad

