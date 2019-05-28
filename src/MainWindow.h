#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QToolButton>
#include <QUndoStack>
#include <QMap>
#include "SegmentationScene.h"
#include "ZoomSlider.h"
#include "Tool.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void imageSelected(const QString& imagePath);
    void zoomChanged(double value);
    void zoomShifted(int delta);
    void mousePosChanged(const QPoint& pos);
    void progressChanged();
    void openFolder();
    void openZoomOptions();

private:
    void setup();
    void setupStyle();
    void setupStatusBar();
    void setupTools();
    void setupUndoRedo();
    void setupOtherActions();
    void setupTooltips();
    void updateSizeLabel();
    void updateTitle();
    void updateZoomFromView();
    void saveSettings();
    void restoreSettings();
    void setToolsEnabled(bool enabled);
    void open(const QString& schemePath, const QString& folderPath, const QString& outputPath);
    void saveMask();
    void createEmptyMask(const QString& imagePath);
    void removeMask(const QString& imagePath);

private:
    Ui::MainWindow *ui;
    ZoomSlider* zoomSlider_;
    QToolButton* zoomLevelButton_;
    QLabel* sizeLabel_;
    QLabel* posLabel_;
    QLabel* progressLabel_;
    QUndoStack undoStack_;
    QString imagePath_;
    QString outputPath_;
    SegmentationScene scene_;
    QList<Tool*> tools_;
    QMap<Tool*, QAction*> toolToolbarActions_;
    QAction* toolToolbarsSeparator_;
};

#endif // MAINWINDOW_H
