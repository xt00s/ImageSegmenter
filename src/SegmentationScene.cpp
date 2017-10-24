#include "SegmentationScene.h"
#include "Helper.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QKeyEvent>

SegmentationScene::SegmentationScene(QObject *parent)
	: QGraphicsScene(parent), tool_(Tool::Polygon), pressed_(false)
{
	canvasItem_ = new CanvasItem;
	markerItem_ = new StartMarkerItem(canvasItem_);
	polyItem_ = new PolylineItem(canvasItem_);
	brushCursorItem_ = new BrushCursorItem;
	canvasItem_->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
	polyItem_->hide();
	markerItem_->hide();
	brushCursorItem_->hide();
	addItem(canvasItem_);
	addItem(brushCursorItem_);
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

void SegmentationScene::setTool(SegmentationScene::Tool tool)
{
	if (tool == tool_)
		return;

	clearToolState();
	if (tool_ == Tool::Brush) {
		for (auto& v : views()) {
			v->viewport()->unsetCursor();
		}
		brushCursorItem_->hide();
	}
	if (tool == Tool::Brush) {
		for (auto& v : views()) {
			v->viewport()->setCursor(Qt::BlankCursor);
		}
	}
	tool_ = tool;
}

void SegmentationScene::setBrushWidth(qreal width)
{
	brushCursorItem_->setWidth(width);
}

void SegmentationScene::clearToolState()
{
	if (tool_ == Tool::Polygon) {
		markerItem_->hide();
		markerItem_->clear();
		polyItem_->hide();
		polyItem_->clear();
	}
}

QPoint SegmentationScene::pixmapPosFromScene(const QPointF& scenePos) const
{
	return help::floor(canvasItem_->mapFromScene(scenePos)).toPoint();
}

void SegmentationScene::keyPressEvent(QKeyEvent *event)
{
	QGraphicsScene::keyPressEvent(event);
	if (!event->isAccepted()) {
		switch (event->key()) {
		case Qt::Key_Escape:
			clearToolState();
			break;
		case Qt::Key_Backspace: {
				if (tool_ == Tool::Polygon) {
					if (polyItem_->isVisible()) {
						polyItem_->removeLastCommittedPoint();
						if (polyItem_->empty()) {
							clearToolState();
						}
					}
				}
				break;
			}
		}
	}
}

void SegmentationScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mousePressEvent(event);
	if (pressed_ || event->button() != Qt::LeftButton)
		return;

	switch (tool_) {
	case Tool::Polygon: {
			auto pos = pixmapPosFromScene(event->scenePos());
			if (canvasItem_->pixmapRect().contains(pos)) {
				if (!polyItem_->isVisible()) {
					polyPixmapPos_ = pos;
					polyItem_->setPos(pos + QPointF(.5, .5));
					polyItem_->begin();
					polyItem_->show();
					markerItem_->setPos(pos + QPointF(.5, .5));
					markerItem_->setAcceptHoverEvents(false);
					markerItem_->show();
				}
				else {
					if (markerItem_->isHovering()) {
						if (polyItem_->canClose()) {
							polyItem_->commit();
							auto poly = polyItem_->polygon().toPolygon().translated(polyPixmapPos_);
							emit newCommand(new DrawCommand(canvasItem_, new Polygon(poly)));
							clearToolState();
						}
					}
					else {
						polyItem_->commitPoint(pos - polyItem_->pos() + QPointF(.5, .5));
						if (!markerItem_->acceptHoverEvents() && polyItem_->canClose())
							markerItem_->setAcceptHoverEvents(true);
					}
				}
			}
			break;
		}
	case Tool::Brush:
		if (brushCursorItem_->isVisible()) {
			auto pos = canvasItem_->mapFromScene(brushCursorItem_->pos());
			Circle circle(pos, brushCursorItem_->width() / 2);
			pressedRect_ = circle.rect();
			canvasCopy_.reset(canvasItem_->extractFragment(QRect()));
			canvasItem_->draw(circle);
		}
		break;
	}
	pressed_ = true;
}

void SegmentationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseMoveEvent(event);

	switch (tool_) {
	case Tool::Polygon: {
			if (polyItem_->isVisible()) {
				polyItem_->setLastPoint(pixmapPosFromScene(event->scenePos()) - polyItem_->pos() + QPointF(.5, .5));
			}
			break;
		}
	case Tool::Brush:
		if (!brushCursorItem_->isVisible()) {
			brushCursorItem_->show();
		} else if (pressed_) {
			auto start = canvasItem_->mapFromScene(brushCursorItem_->pos());
			auto end = canvasItem_->mapFromScene(event->scenePos());
			Line line(QLineF(start, end), brushCursorItem_->width());
			pressedRect_ |= line.rect();
			canvasItem_->draw(line);
		}
		brushCursorItem_->setPos(event->scenePos());
		break;
	}
	emit mousePosChanged(pixmapPosFromScene(event->scenePos()));
}

void SegmentationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);

	if (pressed_) {
		if (tool_ == Tool::Brush) {
			if (brushCursorItem_->isVisible()) {
				emit newCommand(new DrawFragmentCommand(canvasItem_, canvasCopy_->extract(pressedRect_)));
				canvasCopy_.reset();
			}
		}
		pressed_ = false;
	}
}

bool SegmentationScene::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Leave) {
		if (tool_ == Tool::Brush) {
			brushCursorItem_->hide();
		}
	}
	return QGraphicsScene::eventFilter(watched, event);
}
