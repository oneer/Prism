#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QString>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
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
    void exportPreview();
    void resetPreviewParameters();
    void fitPreviewToWindow();
    void showPreviewActualSize();
    void selectStage(QListWidgetItem *item);
    void updatePreview();
    void updateHistogram(const QImage &previewImage);
    void updateMetadata();
    QImage buildPreviewImage() const;
    void appendLog(const QString &message);

    Ui::MainWindow *ui;
    QListWidget *stageList = nullptr;
    QLabel *stageLabel = nullptr;
    QLabel *previewLabel = nullptr;
    QLabel *histogramLabel = nullptr;
    QLabel *exposureValueLabel = nullptr;
    QLabel *redGainValueLabel = nullptr;
    QLabel *blueGainValueLabel = nullptr;
    QPlainTextEdit *metadataView = nullptr;
    QPlainTextEdit *logView = nullptr;
    QScrollArea *previewScrollArea = nullptr;
    QSlider *exposureSlider = nullptr;
    QSlider *redGainSlider = nullptr;
    QSlider *blueGainSlider = nullptr;
    QImage currentImage;
    QString currentImageName;
    QString currentImagePath;
    QString currentImageFormat;
    double currentExposureEv = 0.0;
    double currentRedGain = 1.0;
    double currentBlueGain = 1.0;
    bool fitPreviewToWindowEnabled = true;
};
#endif // MAINWINDOW_H
