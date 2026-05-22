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
    fileMenu->addAction("Export Preview...");
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
    exposureLayout->addRow("EV", new QSlider(Qt::Horizontal, exposure));

    auto *applyButton = new QPushButton("Apply Preview", panel);

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

    const QPixmap pixmap = QPixmap::fromImage(currentImage).scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    previewLabel->setPixmap(pixmap);
}

void MainWindow::appendLog(const QString &message)
{
    if (logView) {
        logView->appendPlainText(message);
    }
}
