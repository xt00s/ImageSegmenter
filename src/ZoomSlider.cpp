#include "ZoomSlider.h"
#include <QPainter>
#include <QMouseEvent>

#define W 8

ZoomSlider::ZoomSlider(QWidget *parent)
	: QWidget(parent)
	, mouseOverHandle_(false)
	, pressed_(false)
	, zoom_(1)
	, min_(1)
	, max_(1)
{
	setMouseTracking(true);
}

void ZoomSlider::setRange(double min, double max)
{
	min_ = min;
	max_ = max;
	setZoom(zoom_);
}

void ZoomSlider::setTicks(const QVector<double>& ticks)
{
	ticks_ = ticks;
}

void ZoomSlider::setZoom(double zoom)
{
	auto v = boundZoom(zoom);
	if (v != zoom_) {
		zoom_ = v;
		updatePos();
		update();
		emit zoomChanged(zoom_);
	}
}

void ZoomSlider::setGrooveColor(const QColor& grooveColor)
{
	if (grooveColor != grooveColor_) {
		grooveColor_ = grooveColor;
		update();
	}
}

void ZoomSlider::setHandleColor(const QColor& handleColor)
{
	if (handleColor != handleColor_) {
		handleColor_ = handleColor;
		update();
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

void ZoomSlider::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	auto r = contentsRect();
	p.setPen(Qt::NoPen);
	p.setBrush(grooveColor_);
	p.drawRect(r.left(), r.center().y(), r.width(), 2);
	p.setBrush(handleColor_.darker(mouseOverHandle_ ? 200 : 100));
	p.drawRoundedRect(pos_, r.top(), W, r.height(), W/2, W/2);
}

void ZoomSlider::mousePressEvent(QMouseEvent* event)
{
	if (mouseOverHandle_) {
		pressed_ = true;
	} else {
		pos_ = boundPos(event->pos().x() - W/2);
		updateZoom();
		update();
	}
}

void ZoomSlider::mouseReleaseEvent(QMouseEvent* event)
{
	pressed_ = false;
}

void ZoomSlider::mouseMoveEvent(QMouseEvent* event)
{
	if (pressed_) {
		pos_ = boundPos(event->pos().x() - W/2);
		updateZoom();
		update();
	}
	else {
		bool over = overHandle(event->pos());
		if (over != mouseOverHandle_) {
			mouseOverHandle_ = over;
			update();
		}
	}
}

void ZoomSlider::leaveEvent(QEvent* event)
{
	if (mouseOverHandle_) {
		mouseOverHandle_ = false;
		update();
	}
}

void ZoomSlider::resizeEvent(QResizeEvent* event)
{
	updatePos();
}

double ZoomSlider::boundZoom(double zoom) const
{
	return qBound(min_, zoom, max_);
}

int ZoomSlider::boundPos(int pos) const
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	return qBound(r.left(), pos, r.right());
}

void ZoomSlider::updateZoom()
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	double zoom;
	if (pos_ == r.center().x()) {
		zoom = 1;
	} else if (pos_ == r.right()) {
		zoom = max_;
	} else if (pos_ == 0) {
		zoom = min_;
	} else if (pos_ > r.center().x()) {
		zoom = double(pos_ - r.center().x()) / (r.right() - r.center().x()) * (max_ - 1) + 1;
	} else {
		zoom = double(pos_) / r.center().x() * (1-min_) + min_;
	}
	setZoom(zoom);
}

void ZoomSlider::updatePos()
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	if (zoom_ >= 1) {
		pos_ = r.center().x() + int((zoom_-1) / (max_-1) * (r.right() - r.center().x()));
	} else {
		pos_ = int((zoom_-min_) / (1-min_) * r.center().x());
	}
	pos_ = boundPos(pos_);
}

bool ZoomSlider::overHandle(const QPoint& pos) const
{
	auto r = contentsRect();
	return QRect(pos_, r.top(), W, r.height()).contains(pos);
}
