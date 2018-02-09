#ifndef SEGMENTINGPENCILTOOLBAR_H
#define SEGMENTINGPENCILTOOLBAR_H

#include <QToolBar>

class QLabel;
class QDoubleSpinBox;
class QToolButton;

class SegmentingPencilToolBar : public QToolBar
{
	Q_OBJECT
public:
	SegmentingPencilToolBar();

	QLabel* bandwidthLabel;
	QDoubleSpinBox* bandwidthSpinBox;
	QToolButton* finishButton;
};

#endif // SEGMENTINGPENCILTOOLBAR_H
