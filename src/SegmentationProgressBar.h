#ifndef SEGMENTATIONPROGRESSBAR_H
#define SEGMENTATIONPROGRESSBAR_H

#include <QProgressBar>

class SegmentationProgressBar : public QProgressBar
{
	Q_OBJECT
public:
	explicit SegmentationProgressBar(QWidget *parent = 0);
protected:
	void paintEvent(QPaintEvent* event) override;
};

#endif // SEGMENTATIONPROGRESSBAR_H
