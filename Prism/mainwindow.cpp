#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QDateTime>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSlider>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTabWidget>
#include <QtMath>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupWorkbenchUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updatePreview();
}

void MainWindow::setupWorkbenchUi()
{
    setWindowTitle("Prism - Soft ISP Workbench");
    resize(1280, 800);

    auto *previewFrame = new QFrame(this);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setMinimumSize(640, 420);

    auto *previewLayout = new QVBoxLayout(previewFrame);
    stageLabel = new QLabel("No image loaded", previewFrame);
    previewLabel = new QLabel("Image Preview", previewFrame);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumHeight(360);
    previewLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    previewLabel->setStyleSheet("QLabel { background: #202124; color: #d0d0d0; border: 1px solid #3a3a3a; }");
    previewScrollArea = new QScrollArea(previewFrame);
    previewScrollArea->setWidget(previewLabel);
    previewScrollArea->setWidgetResizable(true);
    previewScrollArea->setAlignment(Qt::AlignCenter);

    previewLayout->addWidget(stageLabel);
    previewLayout->addWidget(previewScrollArea, 1);
    setCentralWidget(previewFrame);

    setupMenus();
    setupStageList();
    setupParameterPanel();
    setupBottomPanel();

    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenus()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    auto *openAction = fileMenu->addAction("Open Image...", this, &MainWindow::openImage);
    openAction->setShortcut(QKeySequence::Open);

    auto *openProjectAction = fileMenu->addAction("Open Project...", this, &MainWindow::openProject);
    openProjectAction->setShortcut(QKeySequence("Ctrl+Shift+O"));

    auto *saveProjectAction = fileMenu->addAction("Save Project...", this, &MainWindow::saveProject);
    saveProjectAction->setShortcut(QKeySequence::Save);

    fileMenu->addSeparator();
    auto *exportAction = fileMenu->addAction("Export Preview...", this, &MainWindow::exportPreview);
    exportAction->setShortcut(QKeySequence("Ctrl+E"));

    fileMenu->addSeparator();
    auto *exitAction = fileMenu->addAction("Exit", this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    auto *viewMenu = menuBar()->addMenu("&View");
    auto *fitAction = viewMenu->addAction("Fit to Window", this, &MainWindow::fitPreviewToWindow);
    fitAction->setShortcut(QKeySequence("Ctrl+0"));

    auto *actualSizeAction = viewMenu->addAction("Actual Size", this, &MainWindow::showPreviewActualSize);
    actualSizeAction->setShortcut(QKeySequence("Ctrl+1"));

    auto *zoomInAction = viewMenu->addAction("Zoom In", this, &MainWindow::zoomPreviewIn);
    zoomInAction->setShortcuts(QKeySequence::ZoomIn);

    auto *zoomOutAction = viewMenu->addAction("Zoom Out", this, &MainWindow::zoomPreviewOut);
    zoomOutAction->setShortcuts(QKeySequence::ZoomOut);

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About Prism", this, [this]() {
        QMessageBox::about(
            this,
            "About Prism",
            "Prism\n\n"
            "A Qt Widgets workbench for learning and debugging image pipeline stages.\n\n"
            "Current MVP supports image preview, stage selection, white balance, exposure, histogram, metadata, and preview export.");
    });
}

void MainWindow::setupStageList()
{
    auto *dock = new QDockWidget("Pipeline", this);
    dock->setObjectName("PipelineDock");

    stageList = new QListWidget(dock);
    for (const PipelineStage &stage : pipelineModel.stages()) {
        auto *item = new QListWidgetItem(stage.displayName, stageList);
        item->setToolTip(stage.description);
    }
    stageList->setCurrentRow(0);
    connect(stageList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current) {
        selectStage(current);
    });

    dock->setWidget(stageList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::setupParameterPanel()
{
    auto *dock = new QDockWidget("Stage Parameters", this);
    dock->setObjectName("StageParametersDock");
    dock->setMinimumWidth(260);
    dock->setMaximumWidth(260);

    auto *panel = new QWidget(dock);
    auto *layout = new QVBoxLayout(panel);

    stageDescriptionLabel = new QLabel(panel);
    stageDescriptionLabel->setWordWrap(true);

    whiteBalanceGroup = new QGroupBox("White Balance", panel);
    auto *whiteBalanceLayout = new QFormLayout(whiteBalanceGroup);
    redGainSlider = new QSlider(Qt::Horizontal, whiteBalanceGroup);
    redGainSlider->setRange(50, 250);
    redGainSlider->setValue(100);
    blueGainSlider = new QSlider(Qt::Horizontal, whiteBalanceGroup);
    blueGainSlider->setRange(50, 250);
    blueGainSlider->setValue(100);
    redGainValueLabel = new QLabel("1.00x", whiteBalanceGroup);
    blueGainValueLabel = new QLabel("1.00x", whiteBalanceGroup);
    whiteBalanceLayout->addRow("Red gain", redGainSlider);
    whiteBalanceLayout->addRow("Red value", redGainValueLabel);
    whiteBalanceLayout->addRow("Blue gain", blueGainSlider);
    whiteBalanceLayout->addRow("Blue value", blueGainValueLabel);

    exposureGroup = new QGroupBox("Exposure", panel);
    auto *exposureLayout = new QFormLayout(exposureGroup);
    exposureSlider = new QSlider(Qt::Horizontal, exposureGroup);
    exposureSlider->setRange(-200, 200);
    exposureSlider->setValue(0);
    exposureValueLabel = new QLabel("0.00 EV", exposureGroup);
    exposureLayout->addRow("EV", exposureSlider);
    exposureLayout->addRow("Value", exposureValueLabel);

    showOriginalCheckBox = new QCheckBox("Show Original", panel);
    splitCompareCheckBox = new QCheckBox("Split Compare", panel);
    auto *loadPresetButton = new QPushButton("Load Preset", panel);
    auto *savePresetButton = new QPushButton("Save Preset", panel);
    auto *resetButton = new QPushButton("Reset Parameters", panel);
    auto *applyButton = new QPushButton("Apply Preview", panel);
    connect(redGainSlider, &QSlider::valueChanged, this, [this](int value) {
        previewParams.redGain = value / 100.0;
        redGainValueLabel->setText(QString::number(previewParams.redGain, 'f', 2) + "x");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Red gain preview: " + QString::number(previewParams.redGain, 'f', 2) + "x");
    });
    connect(blueGainSlider, &QSlider::valueChanged, this, [this](int value) {
        previewParams.blueGain = value / 100.0;
        blueGainValueLabel->setText(QString::number(previewParams.blueGain, 'f', 2) + "x");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Blue gain preview: " + QString::number(previewParams.blueGain, 'f', 2) + "x");
    });
    connect(exposureSlider, &QSlider::valueChanged, this, [this](int value) {
        previewParams.exposureEv = value / 100.0;
        exposureValueLabel->setText(QString::number(previewParams.exposureEv, 'f', 2) + " EV");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Exposure preview: " + QString::number(previewParams.exposureEv, 'f', 2) + " EV");
    });
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        appendLog(QString("Applied preview: red %1x, blue %2x, exposure %3 EV")
                      .arg(previewParams.redGain, 0, 'f', 2)
                      .arg(previewParams.blueGain, 0, 'f', 2)
                      .arg(previewParams.exposureEv, 0, 'f', 2));
    });
    connect(showOriginalCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        showOriginalPreview = checked;
        updatePreview();
        updateMetadata();
        statusBar()->showMessage(checked ? "Showing original image" : "Showing pipeline preview");
    });
    connect(splitCompareCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        splitComparePreview = checked;
        updatePreview();
        updateMetadata();
        statusBar()->showMessage(checked ? "Showing split compare" : "Showing single preview");
    });
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetPreviewParameters);
    connect(loadPresetButton, &QPushButton::clicked, this, &MainWindow::loadPreset);
    connect(savePresetButton, &QPushButton::clicked, this, &MainWindow::savePreset);

    layout->addWidget(stageDescriptionLabel);
    layout->addWidget(showOriginalCheckBox);
    layout->addWidget(splitCompareCheckBox);
    layout->addWidget(whiteBalanceGroup);
    layout->addWidget(exposureGroup);
    layout->addStretch();
    layout->addWidget(loadPresetButton);
    layout->addWidget(savePresetButton);
    layout->addWidget(resetButton);
    layout->addWidget(applyButton);

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    updateStageControls();
}

void MainWindow::setupBottomPanel()
{
    auto *dock = new QDockWidget("Inspector", this);
    dock->setObjectName("InspectorDock");

    auto *tabs = new QTabWidget(dock);
    histogramLabel = new QLabel("Open an image to view histogram", tabs);
    histogramLabel->setAlignment(Qt::AlignCenter);
    histogramLabel->setMinimumHeight(180);
    histogramLabel->setStyleSheet("QLabel { background: #202124; color: #d0d0d0; border: 1px solid #3a3a3a; }");
    tabs->addTab(histogramLabel, "Histogram");

    metadataView = new QPlainTextEdit(tabs);
    metadataView->setReadOnly(true);
    metadataView->setPlainText("Open an image to view metadata.");
    tabs->addTab(metadataView, "Metadata");

    logView = new QPlainTextEdit(tabs);
    logView->setReadOnly(true);
    logView->setPlainText("Prism workbench initialized.");
    tabs->addTab(logView, "Log");

    dock->setWidget(tabs);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::openImage()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;All files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    if (loadImageFile(filePath)) {
        statusBar()->showMessage("Image loaded");
    }
}

void MainWindow::openProject()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Project",
        QString(),
        "Prism project (*.json);;All files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Open Project", "Could not open project file.");
        appendLog("Failed to open project: " + filePath);
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::warning(this, "Open Project", "Project file is not valid JSON.");
        appendLog("Invalid project file: " + filePath);
        return;
    }

    const QJsonObject project = document.object();
    const QString imagePath = project.value("imagePath").toString();
    if (imagePath.isEmpty() || !loadImageFile(imagePath, false)) {
        QMessageBox::warning(this, "Open Project", "Project image could not be loaded.");
        appendLog("Project image could not be loaded: " + imagePath);
        return;
    }

    const int stageIndex = pipelineModel.indexOfStage(project.value("activeStage").toString());
    if (stageIndex >= 0 && stageList) {
        stageList->setCurrentRow(stageIndex);
    }

    if (redGainSlider) {
        redGainSlider->setValue(qRound(project.value("redGain").toDouble(1.0) * 100.0));
    }
    if (blueGainSlider) {
        blueGainSlider->setValue(qRound(project.value("blueGain").toDouble(1.0) * 100.0));
    }
    if (exposureSlider) {
        exposureSlider->setValue(qRound(project.value("exposureEv").toDouble(0.0) * 100.0));
    }
    if (showOriginalCheckBox) {
        showOriginalCheckBox->setChecked(project.value("showOriginal").toBool(false));
    }
    if (splitCompareCheckBox) {
        splitCompareCheckBox->setChecked(project.value("splitCompare").toBool(false));
    }

    if (project.value("fitPreviewToWindow").toBool(true)) {
        fitPreviewToWindow();
    } else {
        setPreviewZoom(project.value("zoomScale").toDouble(1.0));
    }

    updateStageControls();
    updatePreview();
    updateMetadata();
    appendLog("Opened project: " + filePath);
    statusBar()->showMessage("Project opened");
}

void MainWindow::saveProject()
{
    if (currentImagePath.isEmpty()) {
        QMessageBox::information(this, "Save Project", "Open an image before saving a project.");
        return;
    }

    const PipelineStage *stage = stageList ? pipelineModel.stageAt(stageList->currentRow()) : nullptr;
    const QString defaultName = QFileInfo(currentImageName).completeBaseName() + ".prism.json";
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Project",
        defaultName,
        "Prism project (*.json);;All files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    QJsonObject project;
    project["version"] = 1;
    project["imagePath"] = currentImagePath;
    project["activeStage"] = stage ? stage->id : QString();
    project["redGain"] = previewParams.redGain;
    project["blueGain"] = previewParams.blueGain;
    project["exposureEv"] = previewParams.exposureEv;
    project["fitPreviewToWindow"] = fitPreviewToWindowEnabled;
    project["showOriginal"] = showOriginalPreview;
    project["splitCompare"] = splitComparePreview;
    project["zoomScale"] = previewZoomScale;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Save Project", "Could not save project file.");
        appendLog("Failed to save project: " + filePath);
        return;
    }

    file.write(QJsonDocument(project).toJson(QJsonDocument::Indented));
    appendLog("Saved project: " + filePath);
    statusBar()->showMessage("Project saved");
}

void MainWindow::loadPreset()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Load Preset",
        QString(),
        "Prism preset (*.json);;All files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Load Preset", "Could not open preset file.");
        appendLog("Failed to load preset: " + filePath);
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::warning(this, "Load Preset", "Preset file is not valid JSON.");
        appendLog("Invalid preset file: " + filePath);
        return;
    }

    const QJsonObject preset = document.object();
    if (redGainSlider) {
        redGainSlider->setValue(qRound(preset.value("redGain").toDouble(1.0) * 100.0));
    }
    if (blueGainSlider) {
        blueGainSlider->setValue(qRound(preset.value("blueGain").toDouble(1.0) * 100.0));
    }
    if (exposureSlider) {
        exposureSlider->setValue(qRound(preset.value("exposureEv").toDouble(0.0) * 100.0));
    }
    if (showOriginalCheckBox) {
        showOriginalCheckBox->setChecked(preset.value("showOriginal").toBool(false));
    }
    if (splitCompareCheckBox) {
        splitCompareCheckBox->setChecked(preset.value("splitCompare").toBool(false));
    }

    updatePreview();
    updateMetadata();
    appendLog("Loaded preset: " + filePath);
    statusBar()->showMessage("Preset loaded");
}

void MainWindow::savePreset()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Preset",
        "preview-preset.json",
        "Prism preset (*.json);;All files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    QJsonObject preset;
    preset["version"] = 1;
    preset["redGain"] = previewParams.redGain;
    preset["blueGain"] = previewParams.blueGain;
    preset["exposureEv"] = previewParams.exposureEv;
    preset["showOriginal"] = showOriginalPreview;
    preset["splitCompare"] = splitComparePreview;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Save Preset", "Could not save preset file.");
        appendLog("Failed to save preset: " + filePath);
        return;
    }

    file.write(QJsonDocument(preset).toJson(QJsonDocument::Indented));
    appendLog("Saved preset: " + filePath);
    statusBar()->showMessage("Preset saved");
}

bool MainWindow::loadImageFile(const QString &filePath, bool showError)
{
    QImageReader reader(filePath);
    reader.setAutoTransform(true);

    QImage image = reader.read();
    if (image.isNull()) {
        if (showError) {
            QMessageBox::warning(this, "Open Image", "Could not open image:\n" + reader.errorString());
        }
        appendLog("Failed to open image: " + filePath);
        return false;
    }

    currentImage = image;
    currentImageName = QFileInfo(filePath).fileName();
    currentImagePath = filePath;
    currentImageFormat = QString::fromLatin1(reader.format()).toUpper();
    stageLabel->setText(currentImageName);
    updatePreview();
    updateMetadata();

    const QString message = QString("Loaded %1 (%2 x %3)")
                                .arg(filePath)
                                .arg(currentImage.width())
                                .arg(currentImage.height());
    appendLog(message);
    return true;
}

void MainWindow::exportPreview()
{
    if (currentImage.isNull()) {
        QMessageBox::information(this, "Export Preview", "Open an image before exporting.");
        return;
    }

    const PipelineStage *stage = stageList ? pipelineModel.stageAt(stageList->currentRow()) : nullptr;
    const QString stageId = stage ? stage->id : QString("preview");
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Preview",
        currentImageName.isEmpty() ? stageId + ".png" : QFileInfo(currentImageName).completeBaseName() + "_" + stageId + ".png",
        "PNG image (*.png);;JPEG image (*.jpg *.jpeg)");

    if (filePath.isEmpty()) {
        return;
    }

    const QImage preview = buildPreviewImage(false);
    if (!preview.save(filePath)) {
        QMessageBox::warning(this, "Export Preview", "Could not save preview image.");
        appendLog("Failed to export preview: " + filePath);
        return;
    }

    const QString stageName = stage ? stage->displayName : QString("Preview");
    const QString reportPath = QFileInfo(filePath).absolutePath() + "/" + QFileInfo(filePath).completeBaseName() + ".json";
    QJsonObject report;
    report["version"] = 1;
    report["exportedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["sourceImagePath"] = currentImagePath;
    report["sourceImageName"] = currentImageName;
    report["exportedImagePath"] = filePath;
    report["stageId"] = stage ? stage->id : QString();
    report["stageName"] = stageName;
    report["redGain"] = previewParams.redGain;
    report["blueGain"] = previewParams.blueGain;
    report["exposureEv"] = previewParams.exposureEv;
    report["showOriginal"] = showOriginalPreview;
    report["splitCompare"] = splitComparePreview;
    report["fitPreviewToWindow"] = fitPreviewToWindowEnabled;
    report["zoomScale"] = previewZoomScale;

    QFile reportFile(reportPath);
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        reportFile.write(QJsonDocument(report).toJson(QJsonDocument::Indented));
        appendLog("Exported parameter report: " + reportPath);
    } else {
        appendLog("Failed to export parameter report: " + reportPath);
    }

    appendLog("Exported " + stageName + ": " + filePath);
    statusBar()->showMessage("Preview exported");
}

void MainWindow::resetPreviewParameters()
{
    if (redGainSlider) {
        redGainSlider->setValue(100);
    }
    if (blueGainSlider) {
        blueGainSlider->setValue(100);
    }
    if (exposureSlider) {
        exposureSlider->setValue(0);
    }

    appendLog("Reset preview parameters");
    statusBar()->showMessage("Preview parameters reset");
}

void MainWindow::fitPreviewToWindow()
{
    fitPreviewToWindowEnabled = true;
    if (previewScrollArea) {
        previewScrollArea->setWidgetResizable(true);
    }
    updatePreview();
    showPreviewZoomStatus();
}

void MainWindow::showPreviewActualSize()
{
    setPreviewZoom(1.0);
}

void MainWindow::zoomPreviewIn()
{
    setPreviewZoom(previewZoomScale * 1.25);
}

void MainWindow::zoomPreviewOut()
{
    setPreviewZoom(previewZoomScale / 1.25);
}

void MainWindow::selectStage(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const PipelineStage *stage = pipelineModel.stageAt(stageList->row(item));
    const QString stageName = stage ? stage->displayName : item->text();
    const QString imageName = currentImageName.isEmpty() ? QString("No image loaded") : currentImageName;

    stageLabel->setText(imageName + " - " + stageName);
    statusBar()->showMessage("Selected " + stageName);
    updateStageControls();
    updatePreview();
    updateMetadata();
    appendLog("Selected stage: " + stageName);
}

void MainWindow::updateStageControls()
{
    const int stageIndex = stageList ? stageList->currentRow() : 0;
    const PipelineStage *stage = pipelineModel.stageAt(stageIndex);

    if (stageDescriptionLabel) {
        if (stage) {
            stageDescriptionLabel->setText(QString(
                                               "%1\n\n"
                                               "Input: %2\n"
                                               "Output: %3\n"
                                               "Status: %4")
                                               .arg(stage->description)
                                               .arg(stage->inputDescription)
                                               .arg(stage->outputDescription)
                                               .arg(stage->implementationStatus));
        } else {
            stageDescriptionLabel->setText(QString());
        }
    }
    if (whiteBalanceGroup) {
        whiteBalanceGroup->setEnabled(stageIndex >= 4);
    }
    if (exposureGroup) {
        exposureGroup->setEnabled(stageIndex >= 6);
    }
}

void MainWindow::updatePreview()
{
    if (!previewLabel || currentImage.isNull()) {
        return;
    }

    const QSize targetSize = previewScrollArea
                                 ? previewScrollArea->viewport()->contentsRect().size()
                                 : previewLabel->contentsRect().size();
    if (fitPreviewToWindowEnabled && targetSize.isEmpty()) {
        return;
    }

    const QImage previewImage = buildPreviewImage();
    QPixmap pixmap = QPixmap::fromImage(previewImage);
    if (fitPreviewToWindowEnabled) {
        pixmap = pixmap.scaled(
            targetSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    } else if (!qFuzzyCompare(previewZoomScale, 1.0)) {
        const QSize zoomedSize(
            qRound(previewImage.width() * previewZoomScale),
            qRound(previewImage.height() * previewZoomScale));
        pixmap = pixmap.scaled(
            zoomedSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    }

    previewLabel->setPixmap(pixmap);
    if (fitPreviewToWindowEnabled) {
        previewLabel->resize(targetSize);
    } else {
        previewLabel->resize(pixmap.size());
    }
    updateHistogram(previewImage);
}

void MainWindow::updateHistogram(const QImage &previewImage)
{
    if (!histogramLabel || previewImage.isNull()) {
        return;
    }

    histogramLabel->setPixmap(histogramRenderer.renderRgbHistogram(previewImage));
}

void MainWindow::updateMetadata()
{
    if (!metadataView) {
        return;
    }

    if (currentImage.isNull()) {
        metadataView->setPlainText("Open an image to view metadata.");
        return;
    }

    const PipelineStage *stage = stageList ? pipelineModel.stageAt(stageList->currentRow()) : nullptr;
    const QString stageName = stage ? stage->displayName : "None";
    const int stageIndex = stageList ? stageList->currentRow() : 0;
    const QString whiteBalanceState = stageIndex >= 4 ? "Applied" : "Pending";
    const QString exposureState = stageIndex >= 6 ? "Applied" : "Pending";
    const QString toneMappingState = stageIndex >= 7 ? "Applied" : "Pending";
    const QString gammaState = stageIndex >= 8 ? "Applied" : "Pending";
    const double megapixels = currentImage.width() * currentImage.height() / 1000000.0;
    const double memoryMiB = currentImage.sizeInBytes() / 1024.0 / 1024.0;

    const QString text = QString(
        "File\n"
        "  Name: %1\n"
        "  Path: %2\n"
        "  Format: %3\n\n"
        "Image\n"
        "  Size: %4 x %5\n"
        "  Megapixels: %6 MP\n"
        "  Depth: %7 bits per pixel\n"
        "  Has alpha: %8\n"
        "  Memory: %9 MiB\n\n"
        "Preview\n"
        "  Active stage: %10\n"
        "  Red gain: %11x\n"
        "  Blue gain: %12x\n"
        "  Exposure EV: %13\n"
        "  Showing original: %14\n"
        "  Split compare: %15\n"
        "  White balance: %16\n"
        "  Exposure: %17\n"
        "  Tone mapping: %18\n"
        "  Gamma: %19\n")
                             .arg(currentImageName)
                             .arg(currentImagePath)
                             .arg(currentImageFormat.isEmpty() ? "Unknown" : currentImageFormat)
                             .arg(currentImage.width())
                             .arg(currentImage.height())
                             .arg(megapixels, 0, 'f', 2)
                             .arg(currentImage.depth())
                             .arg(currentImage.hasAlphaChannel() ? "Yes" : "No")
                             .arg(memoryMiB, 0, 'f', 2)
                             .arg(stageName)
                             .arg(previewParams.redGain, 0, 'f', 2)
                             .arg(previewParams.blueGain, 0, 'f', 2)
                             .arg(previewParams.exposureEv, 0, 'f', 2)
                             .arg(showOriginalPreview ? "Yes" : "No")
                             .arg(splitComparePreview ? "Yes" : "No")
                             .arg(whiteBalanceState)
                             .arg(exposureState)
                             .arg(toneMappingState)
                             .arg(gammaState);

    metadataView->setPlainText(text);
}

QImage MainWindow::buildPreviewImage(bool allowOriginalBypass) const
{
    if (allowOriginalBypass && splitComparePreview) {
        return buildSplitPreviewImage();
    }

    if (allowOriginalBypass && showOriginalPreview) {
        return currentImage;
    }

    const int stageIndex = stageList ? stageList->currentRow() : 0;
    return previewProcessor.process(currentImage, previewParams, stageIndex);
}

QImage MainWindow::buildSplitPreviewImage() const
{
    const int stageIndex = stageList ? stageList->currentRow() : 0;
    const QImage original = currentImage.convertToFormat(QImage::Format_ARGB32);
    const QImage processed = previewProcessor.process(currentImage, previewParams, stageIndex).convertToFormat(QImage::Format_ARGB32);
    if (original.isNull() || processed.isNull()) {
        return currentImage;
    }

    const int labelHeight = 28;
    QImage splitImage(original.width() * 2, original.height() + labelHeight, QImage::Format_ARGB32);
    splitImage.fill(QColor(32, 33, 36));

    QPainter painter(&splitImage);
    painter.drawImage(0, labelHeight, original);
    painter.drawImage(original.width(), labelHeight, processed);
    painter.fillRect(original.width() - 1, 0, 2, splitImage.height(), QColor(220, 220, 220));
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 0, original.width(), labelHeight), Qt::AlignCenter, "Original");
    painter.drawText(QRect(original.width(), 0, processed.width(), labelHeight), Qt::AlignCenter, "Pipeline");
    painter.end();

    return splitImage;
}

void MainWindow::setPreviewZoom(double scale)
{
    fitPreviewToWindowEnabled = false;
    previewZoomScale = qBound(0.25, scale, 4.0);
    if (previewScrollArea) {
        previewScrollArea->setWidgetResizable(false);
    }
    updatePreview();
    showPreviewZoomStatus();
}

void MainWindow::showPreviewZoomStatus()
{
    if (fitPreviewToWindowEnabled) {
        statusBar()->showMessage("Preview zoom: Fit to Window");
        return;
    }

    statusBar()->showMessage("Preview zoom: " + QString::number(previewZoomScale * 100.0, 'f', 0) + "%");
}

void MainWindow::appendLog(const QString &message)
{
    if (logView) {
        logView->appendPlainText(message);
    }
}
