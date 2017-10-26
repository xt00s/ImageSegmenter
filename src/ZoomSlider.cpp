#include "ZoomSlider.h"

ZoomSlider::ZoomSlider(QWidget *parent)
	: Slider(parent)
	, zoom_(1)
	, min_(1)
	, max_(1)
{
	connect(this, &Slider::valueChanged, this, &ZoomSlider::valueChanged);
}

void ZoomSlider::setZoomRange(double min, double max)
{
	min_ = min;
	max_ = max;
	setRange(int(min*100), int(max*100));
	setZoom(zoom_);
}

void ZoomSlider::setZoomTicks(const QVector<double>& ticks)
{
	ticks_ = ticks;
}

void ZoomSlider::setZoom(double zoom)
{
	auto z = boundZoom(zoom);
	if (z != zoom_) {
		auto v = int(z*100);
		if (v != value()) {
			setValue(v);
		} else {
			zoom_ = z;
			emit zoomChanged(zoom_);
		}
	}
}

void ZoomSlider::zoomIn()
{
	int i = 0;
	for (auto& z : ticks_) {
		if (z > zoom_)
			break;
		i++;
	}
	setZoom(i < ticks_.count() ? ticks_[i] : zoom_ * 1.2);
}

void ZoomSlider::zoomOut()
{
	int i = 0;
	for (auto& z : ticks_) {
		if (z >= zoom_)
			break;
		i++;
	}
	setZoom((i-1) >= 0 ? ticks_[i-1] : zoom_ / 1.2);
}

int ZoomSlider::valueFromPos(int pos, int minPos, int maxPos)
{
	int center = (maxPos + minPos) / 2;
	if (pos >= center) {
		return (pos - center) * (maximum() - 100) / (maxPos - center) + 100;
	} else {
		return pos * (100 - minimum()) / center + minimum();
	}
}

int ZoomSlider::posFromValue(int value, int minPos, int maxPos)
{
	int center = (maxPos + minPos) / 2;
	if (value >= 100) {
		return center + (value - 100) * (maxPos - center) / (maximum() - 100);
	} else {
		return (value - minimum()) * center / (100 - minimum());
	}
}

void ZoomSlider::valueChanged(int value)
{
	auto z = boundZoom(value == 100 ? 1 : double(value) / 100);
	if (zoom_ != z) {
		zoom_ = z;
		emit zoomChanged(zoom_);
	}
}

double ZoomSlider::boundZoom(double zoom) const
{
	return qBound(min_, zoom, max_);
}

