#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QImageReader>
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
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <cmath>

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
    stageList->addItems({
        "01 Raw Decode",
        "02 Black Level",
        "03 Normalize",
        "04 Demosaic",
        "05 White Balance",
        "06 Color Matrix",
        "07 Exposure",
        "08 Tone Mapping",
        "09 Gamma",
        "10 Display Preview"
    });
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

    auto *panel = new QWidget(dock);
    auto *layout = new QVBoxLayout(panel);

    auto *whiteBalance = new QGroupBox("White Balance", panel);
    auto *whiteBalanceLayout = new QFormLayout(whiteBalance);
    redGainSlider = new QSlider(Qt::Horizontal, whiteBalance);
    redGainSlider->setRange(50, 250);
    redGainSlider->setValue(100);
    blueGainSlider = new QSlider(Qt::Horizontal, whiteBalance);
    blueGainSlider->setRange(50, 250);
    blueGainSlider->setValue(100);
    redGainValueLabel = new QLabel("1.00x", whiteBalance);
    blueGainValueLabel = new QLabel("1.00x", whiteBalance);
    whiteBalanceLayout->addRow("Red gain", redGainSlider);
    whiteBalanceLayout->addRow("Red value", redGainValueLabel);
    whiteBalanceLayout->addRow("Blue gain", blueGainSlider);
    whiteBalanceLayout->addRow("Blue value", blueGainValueLabel);

    auto *exposure = new QGroupBox("Exposure", panel);
    auto *exposureLayout = new QFormLayout(exposure);
    exposureSlider = new QSlider(Qt::Horizontal, exposure);
    exposureSlider->setRange(-200, 200);
    exposureSlider->setValue(0);
    exposureValueLabel = new QLabel("0.00 EV", exposure);
    exposureLayout->addRow("EV", exposureSlider);
    exposureLayout->addRow("Value", exposureValueLabel);

    auto *resetButton = new QPushButton("Reset Parameters", panel);
    auto *applyButton = new QPushButton("Apply Preview", panel);
    connect(redGainSlider, &QSlider::valueChanged, this, [this](int value) {
        currentRedGain = value / 100.0;
        redGainValueLabel->setText(QString::number(currentRedGain, 'f', 2) + "x");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Red gain preview: " + QString::number(currentRedGain, 'f', 2) + "x");
    });
    connect(blueGainSlider, &QSlider::valueChanged, this, [this](int value) {
        currentBlueGain = value / 100.0;
        blueGainValueLabel->setText(QString::number(currentBlueGain, 'f', 2) + "x");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Blue gain preview: " + QString::number(currentBlueGain, 'f', 2) + "x");
    });
    connect(exposureSlider, &QSlider::valueChanged, this, [this](int value) {
        currentExposureEv = value / 100.0;
        exposureValueLabel->setText(QString::number(currentExposureEv, 'f', 2) + " EV");
        updatePreview();
        updateMetadata();
        statusBar()->showMessage("Exposure preview: " + QString::number(currentExposureEv, 'f', 2) + " EV");
    });
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        appendLog(QString("Applied preview: red %1x, blue %2x, exposure %3 EV")
                      .arg(currentRedGain, 0, 'f', 2)
                      .arg(currentBlueGain, 0, 'f', 2)
                      .arg(currentExposureEv, 0, 'f', 2));
    });
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetPreviewParameters);

    layout->addWidget(whiteBalance);
    layout->addWidget(exposure);
    layout->addStretch();
    layout->addWidget(resetButton);
    layout->addWidget(applyButton);

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
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

    QImageReader reader(filePath);
    reader.setAutoTransform(true);

    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, "Open Image", "Could not open image:\n" + reader.errorString());
        appendLog("Failed to open image: " + filePath);
        return;
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
    statusBar()->showMessage("Image loaded");
}

void MainWindow::exportPreview()
{
    if (currentImage.isNull()) {
        QMessageBox::information(this, "Export Preview", "Open an image before exporting.");
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Preview",
        currentImageName.isEmpty() ? "preview.png" : QFileInfo(currentImageName).completeBaseName() + "_preview.png",
        "PNG image (*.png);;JPEG image (*.jpg *.jpeg)");

    if (filePath.isEmpty()) {
        return;
    }

    const QImage preview = buildPreviewImage();
    if (!preview.save(filePath)) {
        QMessageBox::warning(this, "Export Preview", "Could not save preview image.");
        appendLog("Failed to export preview: " + filePath);
        return;
    }

    appendLog("Exported preview: " + filePath);
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
    statusBar()->showMessage("Fit preview to window");
}

void MainWindow::showPreviewActualSize()
{
    fitPreviewToWindowEnabled = false;
    if (previewScrollArea) {
        previewScrollArea->setWidgetResizable(false);
    }
    updatePreview();
    statusBar()->showMessage("Preview at actual size");
}

void MainWindow::selectStage(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const QString stageName = item->text();
    const QString imageName = currentImageName.isEmpty() ? QString("No image loaded") : currentImageName;

    stageLabel->setText(imageName + " - " + stageName);
    statusBar()->showMessage("Selected " + stageName);
    updateMetadata();
    appendLog("Selected stage: " + stageName);
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
    }

    previewLabel->setPixmap(pixmap);
    previewLabel->resize(pixmap.size());
    updateHistogram(previewImage);
}

void MainWindow::updateHistogram(const QImage &previewImage)
{
    if (!histogramLabel || previewImage.isNull()) {
        return;
    }

    std::array<int, 256> redBins {};
    std::array<int, 256> greenBins {};
    std::array<int, 256> blueBins {};

    const QImage image = previewImage.convertToFormat(QImage::Format_ARGB32);
    const int sampleStep = std::max(1, image.width() * image.height() / 250000);

    int sampleIndex = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if ((sampleIndex++ % sampleStep) != 0) {
                continue;
            }

            const QRgb pixel = line[x];
            ++redBins[qRed(pixel)];
            ++greenBins[qGreen(pixel)];
            ++blueBins[qBlue(pixel)];
        }
    }

    const int histogramWidth = 512;
    const int histogramHeight = 180;
    const int bottomPadding = 18;
    const int graphHeight = histogramHeight - bottomPadding;
    const int maxBin = std::max({
        *std::max_element(redBins.begin(), redBins.end()),
        *std::max_element(greenBins.begin(), greenBins.end()),
        *std::max_element(blueBins.begin(), blueBins.end())
    });

    QPixmap histogram(histogramWidth, histogramHeight);
    histogram.fill(QColor("#202124"));

    QPainter painter(&histogram);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QColor("#3a3a3a"));
    painter.drawLine(0, graphHeight, histogramWidth, graphHeight);

    if (maxBin > 0) {
        auto drawChannel = [&](const std::array<int, 256> &bins, const QColor &color) {
            painter.setPen(color);
            for (int i = 0; i < 256; ++i) {
                const int x = i * 2;
                const int height = static_cast<int>((static_cast<double>(bins[i]) / maxBin) * (graphHeight - 8));
                painter.drawLine(x, graphHeight, x, graphHeight - height);
                painter.drawLine(x + 1, graphHeight, x + 1, graphHeight - height);
            }
        };

        drawChannel(redBins, QColor(255, 80, 80, 180));
        drawChannel(greenBins, QColor(80, 220, 120, 180));
        drawChannel(blueBins, QColor(90, 150, 255, 180));
    }

    painter.setPen(QColor("#d0d0d0"));
    painter.drawText(8, histogramHeight - 5, "RGB histogram");

    histogramLabel->setPixmap(histogram);
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

    const QListWidgetItem *stageItem = stageList ? stageList->currentItem() : nullptr;
    const QString stageName = stageItem ? stageItem->text() : "None";
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
        "  Exposure EV: %13\n")
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
                             .arg(currentRedGain, 0, 'f', 2)
                             .arg(currentBlueGain, 0, 'f', 2)
                             .arg(currentExposureEv, 0, 'f', 2);

    metadataView->setPlainText(text);
}

QImage MainWindow::buildPreviewImage() const
{
    if (currentImage.isNull()
        || (currentExposureEv == 0.0 && currentRedGain == 1.0 && currentBlueGain == 1.0)) {
        return currentImage;
    }

    QImage preview = currentImage.convertToFormat(QImage::Format_ARGB32);
    const double exposureScale = std::pow(2.0, currentExposureEv);

    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QRgb pixel = line[x];
            const int red = std::clamp(static_cast<int>(qRed(pixel) * currentRedGain * exposureScale), 0, 255);
            const int green = std::clamp(static_cast<int>(qGreen(pixel) * exposureScale), 0, 255);
            const int blue = std::clamp(static_cast<int>(qBlue(pixel) * currentBlueGain * exposureScale), 0, 255);
            line[x] = qRgba(red, green, blue, qAlpha(pixel));
        }
    }

    return preview;
}

void MainWindow::appendLog(const QString &message)
{
    if (logView) {
        logView->appendPlainText(message);
    }
}
