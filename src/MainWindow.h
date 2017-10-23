#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QUndoStack>
#include <QMap>
#include "SegmentationScene.h"
#include "ZoomSlider.h"
#include "ToolToolBar.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void imageSelected(const QString& imagePath);
	void zoomChanged(double value);
	void zoomShifted(int delta);
	void mousePosChanged(const QPoint& pos);
	void toolChanged();
	void openFolder();
	void progressChanged();

private:
	void setup();
	void setupStyle();
	void setupStatusBar();
	void setupTools();
	void setupUndoRedo();
	void setupAbout();
	void updateSizeLabel();
	void updateTitle();
	void updateZoomFromView();
	void saveSettings();
	void restoreSettings();
	void setToolsEnabled(bool enabled);
	void open(const QString& schemePath, const QString& folderPath, const QString& outputPath);
	void saveMask();

private:
	Ui::MainWindow *ui;
	ZoomSlider* zoomSlider_;
	QLabel* zoomLabel_;
	QLabel* sizeLabel_;
	QLabel* posLabel_;
	QLabel* progressLabel_;
	QProgressBar* progressBar_;
	QUndoStack undoStack_;
	QString imagePath_;
	QString outputPath_;
	SegmentationScene scene_;
	QList<QAction*> tools_;
	QMap<QAction*, QAction*> toolToolbarActions_;
	QAction* currentTool_;
	QAction* toolToolbarsSeparator_;
};

#endif // MAINWINDOW_H
