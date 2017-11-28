#include "SegmentationScene.h"
#include "Tool.h"
#include "GraphicsItems.h"
#include "Helper.h"
#include <QGraphicsView>
#include <QEvent>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

SegmentationScene::SegmentationScene(QObject *parent)
	: QGraphicsScene(parent)
	, tool_(0)
{
	canvasItem_ = new CanvasItem;
	canvasItem_->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
	addItem(canvasItem_);
}

void SegmentationScene::setup()
{
	for (auto& v : views()) {
		v->viewport()->installEventFilter(this);
	}
}

void SegmentationScene::updateSceneRect()
{
	if (canvasItem_->pixmapSize().isNull()) {
		setSceneRect(QRectF());
		return;
	}
	auto xgap = canvasItem_->pixmapSize().width() / 2;
	auto ygap = canvasItem_->pixmapSize().height() / 2;
	setSceneRect(canvasItem_->pixmapRect().adjusted(-xgap, -ygap, xgap, ygap));
}

QPoint SegmentationScene::pixmapPosFromScene(const QPointF& scenePos) const
{
	return help::floor(canvasItem_->mapFromScene(scenePos)).toPoint();
}

void SegmentationScene::setViewCursor(const QCursor& cursor)
{
	for (auto& v : views()) {
		v->viewport()->setCursor(cursor);
	}
}

void SegmentationScene::unsetViewCursor()
{
	for (auto& v : views()) {
		v->viewport()->unsetCursor();
	}
}

void SegmentationScene::keyPressEvent(QKeyEvent *event)
{
	QGraphicsScene::keyPressEvent(event);
	if (!event->isAccepted() && tool_) {
		tool_->keyPressEvent(event);
	}
}

void SegmentationScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mousePressEvent(event);
	if (tool_) {
		tool_->mousePressEvent(event);
	}
}

void SegmentationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseMoveEvent(event);
	if (tool_) {
		tool_->mouseMoveEvent(event);
	}
	emit mousePosChanged(pixmapPosFromScene(event->scenePos()));
}

void SegmentationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);
	if (tool_) {
		tool_->mouseReleaseEvent(event);
	}
}

bool SegmentationScene::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Leave && tool_) {
		tool_->leaveEvent(event);
	}
	return QGraphicsScene::eventFilter(watched, event);
}
