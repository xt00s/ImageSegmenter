#include "HandlessSlider.h"
#include <QPainter>

HandlessSlider::HandlessSlider(QWidget* parent)
    : Slider(parent)
    , textVisible_(true)
    , defaultTextFormatter_([](double v){ return QString("%1%").arg(v*100,0,'f',0); })
{
}

void HandlessSlider::paint(QPainter& p)
{
    auto r = contentsRect();
    QRect vr(r.left() + 1, r.top()+1, position() - r.left(), r.height()-2);
    QRect gr(position() + 1, r.top()+1, r.right() - position(), r.height()-2);

    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(valueGrooveColor());
    p.drawRect(vr);
    p.setBrush(grooveColor());
    p.drawRect(gr);
    p.setPen(Qt::lightGray);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(0,0,-1,-1));

    if (textVisible_) {
        auto text = textFormatter_ ? textFormatter_(value()) : defaultTextFormatter_(value());
        auto tr = handleSpaceRect().adjusted(4,0,0,0);
        p.setFont(textFont_);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(qGray(valueGrooveColor().rgb()) > 127 ? Qt::black : Qt::white));
        p.setClipRect(vr);
        p.drawText(tr, Qt::AlignLeft | Qt::AlignBaseline | Qt::TextSingleLine, text);
        p.setPen(QPen(qGray(grooveColor().rgb()) > 127 ? Qt::black : Qt::white));
        p.setClipRect(gr);
        p.drawText(tr, Qt::AlignLeft | Qt::AlignBaseline | Qt::TextSingleLine, text);
    }
}

QSize HandlessSlider::handleSize() const
{
    return {1, contentsRect().height()};
}

QRect HandlessSlider::handleSpaceRect() const
{
    return contentsRect().adjusted(0,1,-1,-1);
}

void HandlessSlider::setTextFont(const QFont& font)
{
    if (textFont_ != font) {
        textFont_ = font;
        update();
    }
}

void HandlessSlider::setTextVisible(bool visible)
{
    if (textVisible_ != visible) {
        textVisible_ = visible;
        update();
    }
}

void HandlessSlider::setTextFormatter(const TextFormatter& formatter)
{
    textFormatter_ = formatter;
    update();
}
