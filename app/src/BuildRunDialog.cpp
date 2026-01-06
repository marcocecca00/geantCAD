#include "BuildRunDialog.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QColor>
#include <QThread>

namespace geantcad {

BuildRunDialog::BuildRunDialog(QWidget* parent)
    : QDialog(parent)
    , buildProcess_(nullptr)
    , runProcess_(nullptr)
    , isBuilding_(false)
    , isRunning_(false)
{
    setupUI();
}

BuildRunDialog::~BuildRunDialog() {
    if (buildProcess_) {
        buildProcess_->kill();
        buildProcess_->waitForFinished();
        delete buildProcess_;
    }
    if (runProcess_) {
        runProcess_->kill();
        runProcess_->waitForFinished();
        delete runProcess_;
    }
}

void BuildRunDialog::setupUI() {
    setWindowTitle("Build & Run Geant4 Project");
    setMinimumSize(700, 500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Project directory
    QHBoxLayout* dirLayout = new QHBoxLayout();
    dirLayout->addWidget(new QLabel("Project Directory:", this));
    projectDirEdit_ = new QLineEdit(this);
    dirLayout->addWidget(projectDirEdit_);
    browseButton_ = new QPushButton("Browse...", this);
    connect(browseButton_, &QPushButton::clicked, this, &BuildRunDialog::onBrowse);
    dirLayout->addWidget(browseButton_);
    mainLayout->addLayout(dirLayout);
    
    // Build directory (optional, defaults to projectDir/build)
    QHBoxLayout* buildDirLayout = new QHBoxLayout();
    buildDirLayout->addWidget(new QLabel("Build Directory (optional):", this));
    buildDirEdit_ = new QLineEdit(this);
    buildDirEdit_->setPlaceholderText("Defaults to <project>/build");
    buildDirLayout->addWidget(buildDirEdit_);
    browseBuildDirButton_ = new QPushButton("Browse...", this);
    connect(browseBuildDirButton_, &QPushButton::clicked, this, &BuildRunDialog::onBrowseBuildDir);
    buildDirLayout->addWidget(browseBuildDirButton_);
    mainLayout->addLayout(buildDirLayout);
    
    // Console output
    consoleOutput_ = new QTextEdit(this);
    consoleOutput_->setReadOnly(true);
    consoleOutput_->setFont(QFont("Courier", 9));
    mainLayout->addWidget(consoleOutput_);
    
    // Progress bar
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 0); // Indeterminate
    progressBar_->setVisible(false);
    mainLayout->addWidget(progressBar_);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buildButton_ = new QPushButton("Build", this);
    runButton_ = new QPushButton("Run", this);
    stopButton_ = new QPushButton("Stop", this);
    closeButton_ = new QPushButton("Close", this);
    
    connect(buildButton_, &QPushButton::clicked, this, &BuildRunDialog::onBuild);
    connect(runButton_, &QPushButton::clicked, this, &BuildRunDialog::onRun);
    connect(stopButton_, &QPushButton::clicked, this, &BuildRunDialog::onStop);
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
    
    stopButton_->setEnabled(false);
    
    buttonLayout->addWidget(buildButton_);
    buttonLayout->addWidget(runButton_);
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton_);
    
    mainLayout->addLayout(buttonLayout);
}

void BuildRunDialog::setProjectDirectory(const QString& dir) {
    projectDirEdit_->setText(dir);
}

void BuildRunDialog::setBuildDirectory(const QString& dir) {
    buildDirEdit_->setText(dir);
}

QString BuildRunDialog::getProjectDirectory() const {
    return projectDirEdit_->text();
}

QString BuildRunDialog::getBuildDirectory() const {
    return buildDirEdit_->text();
}

void BuildRunDialog::onBrowse() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Geant4 Project Directory", 
                                                      projectDirEdit_->text());
    if (!dir.isEmpty()) {
        projectDirEdit_->setText(dir);
        // Auto-update build directory if empty
        if (buildDirEdit_->text().isEmpty()) {
            buildDirEdit_->setPlaceholderText(dir + "/build");
        }
    }
}

void BuildRunDialog::onBrowseBuildDir() {
    QString defaultDir = buildDirEdit_->text();
    if (defaultDir.isEmpty() && !projectDirEdit_->text().isEmpty()) {
        defaultDir = projectDirEdit_->text() + "/build";
    }
    
    QString dir = QFileDialog::getExistingDirectory(this, "Select Build Directory", defaultDir);
    if (!dir.isEmpty()) {
        buildDirEdit_->setText(dir);
    }
}

void BuildRunDialog::onBuild() {
    QString projectDir = projectDirEdit_->text();
    if (projectDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a project directory.");
        return;
    }
    
    QDir dir(projectDir);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Error", "Project directory does not exist.");
        return;
    }
    
    if (isBuilding_ || isRunning_) {
        QMessageBox::warning(this, "Error", "A process is already running.");
        return;
    }
    
    isBuilding_ = true;
    setButtonsEnabled(false);
    stopButton_->setEnabled(true);
    progressBar_->setVisible(true);
    consoleOutput_->clear();
    appendOutput("Starting build process...\n");
    
    // Determine build directory
    QString buildDir = buildDirEdit_->text().trimmed();
    if (buildDir.isEmpty()) {
        // Default to projectDir/build
        buildDir = projectDir + "/build";
    }
    QDir().mkpath(buildDir);
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BuildRunDialog::onProcessFinished);
    connect(buildProcess_, &QProcess::errorOccurred, this, &BuildRunDialog::onProcessError);
    connect(buildProcess_, &QProcess::readyReadStandardOutput,
            this, &BuildRunDialog::onReadyReadStandardOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError,
            this, &BuildRunDialog::onReadyReadStandardError);
    
    // Run cmake configuration first, then build
    appendOutput("Running cmake configuration...\n");
    buildProcess_->setWorkingDirectory(projectDir);
    buildProcess_->setProperty("buildStep", "configure");
    buildProcess_->start("cmake", QStringList() << "-S" << projectDir << "-B" << buildDir);
}

void BuildRunDialog::onRun() {
    QString projectDir = projectDirEdit_->text();
    if (projectDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a project directory.");
        return;
    }
    
    QString executable = projectDir + "/build/geant4_project";
    if (!QFile::exists(executable)) {
        QMessageBox::warning(this, "Error", 
                            "Executable not found. Please build the project first.\n" + executable);
        return;
    }
    
    if (isBuilding_ || isRunning_) {
        QMessageBox::warning(this, "Error", "A process is already running.");
        return;
    }
    
    isRunning_ = true;
    setButtonsEnabled(false);
    stopButton_->setEnabled(true);
    progressBar_->setVisible(true);
    appendOutput("Starting Geant4 simulation...\n");
    
    runProcess_ = new QProcess(this);
    connect(runProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BuildRunDialog::onProcessFinished);
    connect(runProcess_, &QProcess::errorOccurred, this, &BuildRunDialog::onProcessError);
    connect(runProcess_, &QProcess::readyReadStandardOutput,
            this, &BuildRunDialog::onReadyReadStandardOutput);
    connect(runProcess_, &QProcess::readyReadStandardError,
            this, &BuildRunDialog::onReadyReadStandardError);
    
    QString macroFile = projectDir + "/macros/vis.mac";
    if (!QFile::exists(macroFile)) {
        macroFile = projectDir + "/macros/run.mac";
    }
    
    QStringList args;
    if (QFile::exists(macroFile)) {
        args << macroFile;
    }
    
    runProcess_->setWorkingDirectory(projectDir);
    runProcess_->start(executable, args);
}

void BuildRunDialog::onStop() {
    if (isBuilding_ && buildProcess_) {
        buildProcess_->kill();
        appendOutput("\nBuild process stopped by user.\n", true);
    }
    if (isRunning_ && runProcess_) {
        runProcess_->kill();
        appendOutput("\nRun process stopped by user.\n", true);
    }
}

void BuildRunDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QProcess* process = qobject_cast<QProcess*>(sender());
    
    if (process == buildProcess_) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString buildStep = buildProcess_->property("buildStep").toString();
            
            if (buildStep == "configure") {
                // Configuration successful, now build
                QString projectDir = projectDirEdit_->text();
                QString buildDir = projectDir + "/build";
                
                appendOutput("✓ Configuration successful!\n");
                appendOutput("Running cmake --build...\n");
                buildProcess_->setProperty("buildStep", "build");
                buildProcess_->start("cmake", QStringList() << "--build" << buildDir 
                                     << "-j" << QString::number(QThread::idealThreadCount()));
            } else {
                // Build finished
                appendOutput("\n✓ Build completed successfully!\n");
                isBuilding_ = false;
                setButtonsEnabled(true);
                stopButton_->setEnabled(false);
                progressBar_->setVisible(false);
            }
        } else {
            appendOutput(QString("\n✗ Build failed with exit code %1\n").arg(exitCode), true);
            isBuilding_ = false;
            setButtonsEnabled(true);
            stopButton_->setEnabled(false);
            progressBar_->setVisible(false);
        }
    } else if (process == runProcess_) {
        isRunning_ = false;
        if (exitStatus == QProcess::NormalExit) {
            appendOutput("\n✓ Simulation completed.\n");
        } else {
            appendOutput(QString("\n✗ Simulation exited with code %1\n").arg(exitCode), true);
        }
        setButtonsEnabled(true);
        stopButton_->setEnabled(false);
        progressBar_->setVisible(false);
    }
}

void BuildRunDialog::onProcessError(QProcess::ProcessError error) {
    QProcess* process = qobject_cast<QProcess*>(sender());
    QString errorMsg;
    
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Process failed to start. Check if cmake and make are installed.";
            break;
        case QProcess::Crashed:
            errorMsg = "Process crashed.";
            break;
        case QProcess::Timedout:
            errorMsg = "Process timed out.";
            break;
        default:
            errorMsg = "Unknown process error.";
    }
    
    appendOutput(QString("\n✗ Error: %1\n").arg(errorMsg), true);
    
    if (process == buildProcess_) {
        isBuilding_ = false;
    } else if (process == runProcess_) {
        isRunning_ = false;
    }
    
    setButtonsEnabled(true);
    stopButton_->setEnabled(false);
    progressBar_->setVisible(false);
}

void BuildRunDialog::onReadyReadStandardOutput() {
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (process) {
        QByteArray data = process->readAllStandardOutput();
        appendOutput(QString::fromLocal8Bit(data));
    }
}

void BuildRunDialog::onReadyReadStandardError() {
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (process) {
        QByteArray data = process->readAllStandardError();
        appendOutput(QString::fromLocal8Bit(data), true);
    }
}

void BuildRunDialog::appendOutput(const QString& text, bool isError) {
    QTextCharFormat format;
    if (isError) {
        format.setForeground(QColor(Qt::red));
    } else {
        format.setForeground(QColor(Qt::black));
    }
    
    QTextCursor cursor = consoleOutput_->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.setCharFormat(format);
    cursor.insertText(text);
    consoleOutput_->setTextCursor(cursor);
    consoleOutput_->ensureCursorVisible();
}

void BuildRunDialog::setButtonsEnabled(bool enabled) {
    buildButton_->setEnabled(enabled);
    runButton_->setEnabled(enabled);
    browseButton_->setEnabled(enabled);
    projectDirEdit_->setEnabled(enabled);
}

} // namespace geantcad

