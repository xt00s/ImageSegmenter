#include "SegmentationProgressBar.h"
#include <QPainter>

SegmentationProgressBar::SegmentationProgressBar(QWidget *parent)
	: QProgressBar(parent)
{
}

void SegmentationProgressBar::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	int chunk = qRound(size().width() * qreal(value() - minimum()) / (maximum() - minimum()));
	p.fillRect(QRect(0, 0, chunk, height()), "lightgreen");
	p.fillRect(QRect(chunk, 0, width()-chunk, height()), palette().midlight());
}
