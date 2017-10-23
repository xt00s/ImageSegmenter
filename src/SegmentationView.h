#ifndef SEGMENTATIONVIEW_H
#define SEGMENTATIONVIEW_H

#include <QGraphicsView>

class SegmentationView : public QGraphicsView
{
	Q_OBJECT
public:
	SegmentationView(QWidget *parent = 0);

	void setup();

signals:
	void zoomShifted(int delta);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	bool panning_;
	QPoint panningPos_;
	QCursor lastCursor_;
};

#endif // SEGMENTATIONVIEW_H
