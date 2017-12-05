#include "ZoomSlider.h"

ZoomSlider::ZoomSlider(QWidget *parent)
	: Slider(parent)
	, zoom_(1)
	, min_(1)
	, max_(1)
	, updateZoom_(true)
{
	connect(this, &Slider::valueChanged, this, &ZoomSlider::valueChanged);
}

void ZoomSlider::setZoomRange(double min, double max)
{
	auto old = zoom_;
	min_ = min;
	max_ = max;
	zoom_ = min-1;
	setZoom(old);
}

void ZoomSlider::setZoomTicks(const QVector<double>& ticks)
{
	ticks_ = ticks;
}

void ZoomSlider::setZoom(double zoom)
{
	auto z = boundZoom(zoom);
	if (z != zoom_) {
		double v = 0;
		if (z < 1) {
			v = min_ != 1 ? (z-min_)/(1-min_)/2 : 0;
		} else {
			v = max_ != 1 ? (z-1)/(max_-1)/2 + 0.5 : 0;
		}
		if (v != value()) {
			updateZoom_ = false;
			setValue(v);
			updateZoom_ = true;
		}
		zoom_ = z;
		emit zoomChanged(zoom_);
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

double ZoomSlider::valueFromPos(int pos, int minPos, int maxPos) const
{
	int center = (maxPos + minPos) / 2;
	return pos == center ? 0.5 : Slider::valueFromPos(pos, minPos, maxPos);
}

int ZoomSlider::posFromValue(double value, int minPos, int maxPos) const
{
	int center = (maxPos + minPos) / 2;
	return value == 0.5 ? center : Slider::posFromValue(value, minPos, maxPos);
}

void ZoomSlider::valueChanged(double value)
{
	if (!updateZoom_) {
		return;
	}
	double zoom = 0;
	if (value < 0.5) {
		zoom = min_ != 1 ? min_ + value*2*(1-min_) : 1;
	} else {
		zoom = max_ != 1 ? 1 + (value-0.5)*2*(max_-1) : 0;
	}
	auto z = boundZoom(zoom);
	if (zoom_ != z) {
		zoom_ = z;
		emit zoomChanged(zoom_);
	}
}

double ZoomSlider::boundZoom(double zoom) const
{
	return qBound(min_, zoom, max_);
}
