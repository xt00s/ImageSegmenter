#ifndef BRUSHTOOLBAR_H
#define BRUSHTOOLBAR_H

#include <QToolBar>

class QLabel;
class QComboBox;

class BrushToolBar : public QToolBar
{
	Q_OBJECT
public:
	BrushToolBar(QWidget *parent = 0);

	QLabel* widthLabel;
	QComboBox* widthCombo;
	QAction* actionIncreaseWidth;
	QAction* actionDecreaseWidth;

private:
	void incWidth();
	void decWidth();
};

#endif // BRUSHTOOLBAR_H
