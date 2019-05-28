#include "Slider.h"
#include <QPainter>
#include <QMouseEvent>

Slider::Slider(QWidget *parent)
    : QWidget(parent)
    , pressed_(false)
    , pos_(0)
    , value_(0)
{
    setMouseTracking(true);
}

void Slider::setValue(double value)
{
    auto v = boundValue(value);
    if (v != value_) {
        value_ = v;
        updatePos();
        update();
        emit valueChanged(value_);
    }
}

void Slider::setGrooveColor(const QColor& color)
{
    if (color != grooveColor_) {
        grooveColor_ = color;
        update();
    }
}

void Slider::setValueGrooveColor(const QColor& color)
{
    if (valueGrooveColor_ != color) {
        valueGrooveColor_ = color;
        update();
    }
}

void Slider::setHandleColor(const QColor& color)
{
    if (color != handleColor_) {
        handleColor_ = color;
        update();
    }
}

void Slider::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    paint(p);
}

void Slider::paint(QPainter& p)
{
    auto r = handleSpaceRect();
    auto w = handleSize().width();
    p.setPen(Qt::NoPen);
    p.setBrush(valueGrooveColor_);
    p.drawRect(r.left(), r.center().y(), pos_ - r.left(), 2);
    p.setBrush(grooveColor_);
    p.drawRect(pos_, r.center().y(), r.right() - pos_ + 1, 2);
    p.setBrush(handleColor_.darker(underMouse() ? 200 : 100));
    p.drawRoundedRect(pos_, r.top(), w, r.height(), w/2, w/2);
}

void Slider::mousePressEvent(QMouseEvent* event)
{
    if (pressed_ || event->button() != Qt::LeftButton) {
        return;
    }
    if (!overHandle(event->pos())) {
        pos_ = boundPos(event->pos().x() - handleSize().width()/2);
        updateValue();
        update();
    }
    pressed_ = true;
    pressedPos_ = pos_;
    pressedMousePos_ = event->pos();
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        pressed_ = false;
    }
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

double Slider::boundValue(double value) const
{
    return qBound(0., value, 1.);
}

int Slider::boundPos(int pos) const
{
    auto r = handleSpaceRect().adjusted(0,0,-handleSize().width()+1,0);
    return qBound(r.left(), pos, r.right());
}

void Slider::updateValue()
{
    auto r = handleSpaceRect().adjusted(0,0,-handleSize().width()+1,0);
    setValue(valueFromPos(pos_, r.left(), r.right()));
}

void Slider::updatePos()
{
    auto r = handleSpaceRect().adjusted(0,0,-handleSize().width()+1,0);
    pos_ = boundPos(posFromValue(value_, r.left(), r.right()));
}

double Slider::valueFromPos(int pos, int minPos, int maxPos) const
{
    if (pos == minPos) {
        return 0;
    } else if (pos == maxPos) {
        return 1;
    } else {
        return minPos >= maxPos ? 0 : double(pos - minPos) / (maxPos - minPos);
    }
}

int Slider::posFromValue(double value, int minPos, int maxPos) const
{
    return value * (maxPos - minPos) + minPos;
}

QSize Slider::handleSize() const
{
    return QSize(8, contentsRect().height());
}

QRect Slider::handleSpaceRect() const
{
    return contentsRect();
}

bool Slider::overHandle(const QPoint& pos) const
{
    return QRect(QPoint(pos_, handleSpaceRect().top()), handleSize()).contains(pos);
}

QSize Slider::sizeHint() const
{
    return QSize(16,100);
}
