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
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include <algorithm>
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

    previewLayout->addWidget(stageLabel);
    previewLayout->addWidget(previewLabel, 1);
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
    fileMenu->addAction("Open Image...", this, &MainWindow::openImage);
    fileMenu->addAction("Export Preview...", this, &MainWindow::exportPreview);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

    auto *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Fit to Window");
    viewMenu->addAction("Actual Size");

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About Prism");
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
    whiteBalanceLayout->addRow("Red gain", new QSlider(Qt::Horizontal, whiteBalance));
    whiteBalanceLayout->addRow("Blue gain", new QSlider(Qt::Horizontal, whiteBalance));

    auto *exposure = new QGroupBox("Exposure", panel);
    auto *exposureLayout = new QFormLayout(exposure);
    exposureSlider = new QSlider(Qt::Horizontal, exposure);
    exposureSlider->setRange(-200, 200);
    exposureSlider->setValue(0);
    exposureValueLabel = new QLabel("0.00 EV", exposure);
    exposureLayout->addRow("EV", exposureSlider);
    exposureLayout->addRow("Value", exposureValueLabel);

    auto *applyButton = new QPushButton("Apply Preview", panel);
    connect(exposureSlider, &QSlider::valueChanged, this, [this](int value) {
        currentExposureEv = value / 100.0;
        exposureValueLabel->setText(QString::number(currentExposureEv, 'f', 2) + " EV");
        updatePreview();
        statusBar()->showMessage("Exposure preview: " + QString::number(currentExposureEv, 'f', 2) + " EV");
    });
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        appendLog("Applied exposure preview: " + QString::number(currentExposureEv, 'f', 2) + " EV");
    });

    layout->addWidget(whiteBalance);
    layout->addWidget(exposure);
    layout->addStretch();
    layout->addWidget(applyButton);

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::setupBottomPanel()
{
    auto *dock = new QDockWidget("Inspector", this);
    dock->setObjectName("InspectorDock");

    auto *tabs = new QTabWidget(dock);
    tabs->addTab(new QLabel("Histogram placeholder", tabs), "Histogram");
    tabs->addTab(new QLabel("Metadata placeholder", tabs), "Metadata");

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
    stageLabel->setText(currentImageName);
    updatePreview();

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

void MainWindow::selectStage(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const QString stageName = item->text();
    const QString imageName = currentImageName.isEmpty() ? QString("No image loaded") : currentImageName;

    stageLabel->setText(imageName + " - " + stageName);
    statusBar()->showMessage("Selected " + stageName);
    appendLog("Selected stage: " + stageName);
}

void MainWindow::updatePreview()
{
    if (!previewLabel || currentImage.isNull()) {
        return;
    }

    const QSize targetSize = previewLabel->contentsRect().size();
    if (targetSize.isEmpty()) {
        return;
    }

    const QPixmap pixmap = QPixmap::fromImage(buildPreviewImage()).scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    previewLabel->setPixmap(pixmap);
}

QImage MainWindow::buildPreviewImage() const
{
    if (currentImage.isNull() || currentExposureEv == 0.0) {
        return currentImage;
    }

    QImage preview = currentImage.convertToFormat(QImage::Format_ARGB32);
    const double scale = std::pow(2.0, currentExposureEv);

    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QRgb pixel = line[x];
            const int red = std::clamp(static_cast<int>(qRed(pixel) * scale), 0, 255);
            const int green = std::clamp(static_cast<int>(qGreen(pixel) * scale), 0, 255);
            const int blue = std::clamp(static_cast<int>(qBlue(pixel) * scale), 0, 255);
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
