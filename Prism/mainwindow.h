#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

class QLabel;
class QPlainTextEdit;
class QResizeEvent;

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
    void updatePreview();
    void appendLog(const QString &message);

    Ui::MainWindow *ui;
    QLabel *stageLabel = nullptr;
    QLabel *previewLabel = nullptr;
    QPlainTextEdit *logView = nullptr;
    QImage currentImage;
};
#endif // MAINWINDOW_H
