#include "Slider.h"
#include <QPainter>
#include <QMouseEvent>

#define W 8

Slider::Slider(QWidget *parent)
	: QWidget(parent)
	, pressed_(false)
	, pos_(0)
	, min_(0)
	, max_(0)
	, value_(0)
{
	setMouseTracking(true);
}

void Slider::setRange(int min, int max)
{
	min_ = min;
	max_ = max;
	setValue(value_);
}

void Slider::setValue(int value)
{
	auto v = boundValue(value);
	if (v != value_) {
		value_ = v;
		updatePos();
		update();
		emit valueChanged(value_);
	}
}

void Slider::setGrooveColor(const QColor& grooveColor)
{
	if (grooveColor != grooveColor_) {
		grooveColor_ = grooveColor;
		update();
	}
}

void Slider::setHandleColor(const QColor& handleColor)
{
	if (handleColor != handleColor_) {
		handleColor_ = handleColor;
		update();
	}
}

void Slider::paintEvent(QPaintEvent* event)
{
	auto r = contentsRect();
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(grooveColor_);
	p.drawRect(r.left(), r.center().y(), r.width(), 2);
	p.setBrush(handleColor_.darker(underMouse() ? 200 : 100));
	p.drawRoundedRect(pos_, r.top(), W, r.height(), W/2, W/2);
}

void Slider::mousePressEvent(QMouseEvent* event)
{
	if (!overHandle(event->pos())) {
		pos_ = boundPos(event->pos().x() - W/2);
		updateValue();
		update();
	}
	pressed_ = true;
	pressedPos_ = pos_;
	pressedMousePos_ = event->pos();
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
	pressed_ = false;
}

void Slider::mouseMoveEvent(QMouseEvent* event)
{
	if (pressed_) {
		pos_ = boundPos(pressedPos_ + event->pos().x() - pressedMousePos_.x());
		updateValue();
		update();
	}
}

void Slider::enterEvent(QEvent* event)
{
	update();
}

void Slider::leaveEvent(QEvent* event)
{
	update();
}

void Slider::resizeEvent(QResizeEvent* event)
{
	updatePos();
}

int Slider::boundValue(int value) const
{
	return qBound(min_, value, max_);
}

int Slider::boundPos(int pos) const
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	return qBound(r.left(), pos, r.right());
}

void Slider::updateValue()
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	setValue(valueFromPos(pos_, r.left(), r.right()));
}

int Slider::valueFromPos(int pos, int minPos, int maxPos)
{
	if (pos == minPos) {
		return min_;
	} else if (pos == maxPos) {
		return max_;
	} else {
		return minPos >= maxPos ? min_ :
			(pos - minPos) * (max_ - min_) / (maxPos - minPos) + min_;
	}
}

void Slider::updatePos()
{
	auto r = contentsRect().adjusted(0,0,-W+1,0);
	pos_ = boundPos(posFromValue(value_, r.left(), r.right()));
}

int Slider::posFromValue(int value, int minPos, int maxPos)
{
	return min_ >= max_ ? minPos :
		(value - min_) * (maxPos - minPos) / (max_ - min_) + minPos;
}

bool Slider::overHandle(const QPoint& pos) const
{
	auto r = contentsRect();
	return QRect(pos_, r.top(), W, r.height()).contains(pos);
}

QSize Slider::sizeHint() const
{
	return QSize(16,100);
}
