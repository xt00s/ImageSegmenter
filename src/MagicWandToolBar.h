#ifndef MAGICWANDTOOLBAR_H
#define MAGICWANDTOOLBAR_H

#include <QToolBar>

class QLabel;
class HandlessSlider;
class QToolButton;

class MagicWandToolBar : public QToolBar
{
	Q_OBJECT
public:
	MagicWandToolBar(QWidget *parent = 0);

	QLabel* toleranceLabel;
	HandlessSlider* toleranceSlider;
	QToolButton* finishButton;
};

#endif // MAGICWANDTOOLBAR_H
