#include "SegmentationView.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <QApplication>

SegmentationView::SegmentationView(QWidget *parent)
    : QGraphicsView(parent)
    , panning_(false)
{}

void SegmentationView::setup()
{
    viewport()->installEventFilter(this);
}

bool SegmentationView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == viewport()) {
        switch (event->type()) {
        case QEvent::Wheel: {
                if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                    auto wheelEvent = static_cast<QWheelEvent*>(event);
                    zoomShifted(wheelEvent->angleDelta().y() / 120);
                    return true;
                }
                break;
            }
        case QEvent::MouseButtonPress: {
                auto mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::RightButton && QApplication::keyboardModifiers() & Qt::ControlModifier) {
                    panningPos_ = mouseEvent->globalPos();
                    panning_ = true;
                    lastCursor_ = viewport()->cursor();
                    viewport()->setCursor(Qt::ClosedHandCursor);
                    return true;
                }
                break;
            }
        case QEvent::MouseButtonRelease: {
                auto mouseEvent = static_cast<QMouseEvent*>(event);
                if (panning_ && mouseEvent->button() == Qt::RightButton) {
                    panning_ = false;
                    viewport()->setCursor(lastCursor_);
                    return true;
                }
                break;
            }
        case QEvent::MouseMove: {
                auto mouseEvent = static_cast<QMouseEvent*>(event);
                if (panning_) {
                    auto d = panningPos_ - mouseEvent->globalPos();
                    panningPos_ = mouseEvent->globalPos();
                    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + d.x());
                    verticalScrollBar()->setValue(verticalScrollBar()->value() + d.y());
                }
                break;
            }
        }
    }
    return QGraphicsView::eventFilter(watched, event);
}
