#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QString>

#include "histogramrenderer.h"
#include "pipelinemodel.h"
#include "previewprocessor.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QCheckBox;
class QGroupBox;
class QResizeEvent;
class QScrollArea;
class QSlider;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupWorkbenchUi();
    void setupMenus();
    void setupStageList();
    void setupParameterPanel();
    void setupBottomPanel();
    void openImage();
    void openProject();
    void saveProject();
    void loadPreset();
    void savePreset();
    void exportPreview();
    void resetPreviewParameters();
    void fitPreviewToWindow();
    void showPreviewActualSize();
    void zoomPreviewIn();
    void zoomPreviewOut();
    void selectStage(QListWidgetItem *item);
    void updateStageControls();
    void updatePreview();
    void updateHistogram(const QImage &previewImage);
    void updateMetadata();
    QImage buildPreviewImage(bool allowOriginalBypass = true) const;
    QImage buildSplitPreviewImage() const;
    bool loadImageFile(const QString &filePath, bool showError = true);
    void setPreviewZoom(double scale);
    void showPreviewZoomStatus();
    void appendLog(const QString &message);

    Ui::MainWindow *ui;
    QListWidget *stageList = nullptr;
    QLabel *stageLabel = nullptr;
    QLabel *previewLabel = nullptr;
    QLabel *histogramLabel = nullptr;
    QLabel *blackLevelValueLabel = nullptr;
    QLabel *normalizeValueLabel = nullptr;
    QLabel *exposureValueLabel = nullptr;
    QLabel *redGainValueLabel = nullptr;
    QLabel *blueGainValueLabel = nullptr;
    QLabel *colorSaturationValueLabel = nullptr;
    QLabel *stageDescriptionLabel = nullptr;
    QCheckBox *showOriginalCheckBox = nullptr;
    QCheckBox *splitCompareCheckBox = nullptr;
    QGroupBox *blackLevelGroup = nullptr;
    QGroupBox *normalizeGroup = nullptr;
    QGroupBox *whiteBalanceGroup = nullptr;
    QGroupBox *colorMatrixGroup = nullptr;
    QGroupBox *exposureGroup = nullptr;
    QPlainTextEdit *metadataView = nullptr;
    QPlainTextEdit *logView = nullptr;
    QScrollArea *previewScrollArea = nullptr;
    QSlider *blackLevelSlider = nullptr;
    QSlider *normalizeSlider = nullptr;
    QSlider *exposureSlider = nullptr;
    QSlider *redGainSlider = nullptr;
    QSlider *blueGainSlider = nullptr;
    QSlider *colorSaturationSlider = nullptr;
    QImage currentImage;
    QString currentImageName;
    QString currentImagePath;
    QString currentImageFormat;
    HistogramRenderer histogramRenderer;
    PipelineModel pipelineModel;
    PreviewParams previewParams;
    PreviewProcessor previewProcessor;
    bool fitPreviewToWindowEnabled = true;
    bool showOriginalPreview = false;
    bool splitComparePreview = false;
    double previewZoomScale = 1.0;
};
#endif // MAINWINDOW_H
