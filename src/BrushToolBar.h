#ifndef BRUSHTOOLBAR_H
#define BRUSHTOOLBAR_H

#include "ToolToolBar.h"
#include <QLabel>
#include <QComboBox>
#include <QAction>

class SegmentationScene;

class BrushToolBar : public ToolToolBar
{
	Q_OBJECT
public:
	BrushToolBar(QWidget *parent = 0);

	QLabel* widthLabel;
	QComboBox* widthCombo;
	QAction* actionIncreaseWidth;
	QAction* actionDecreaseWidth;

	void activate(SegmentationScene& scene) override;

private:
	void incWidth();
	void decWidth();
};

#endif // BRUSHTOOLBAR_H
