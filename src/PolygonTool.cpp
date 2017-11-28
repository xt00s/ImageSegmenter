#include "PolygonTool.h"
#include "SegmentationScene.h"
#include "GraphicsItems.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

PolygonTool::PolygonTool(QAction* action, SegmentationScene* scene, QObject* parent)
	: Tool(action, scene, parent)
{
	markerItem_ = new StartMarkerItem(scene->canvasItem());
	markerItem_->hide();
	polyItem_ = new PolylineItem(scene->canvasItem());
	polyItem_->hide();
}

void PolygonTool::clear()
{
	markerItem_->hide();
	markerItem_->clear();
	polyItem_->hide();
	polyItem_->clear();
}

void PolygonTool::onActivate()
{
}

void PolygonTool::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Escape:
		clear();
		break;
	case Qt::Key_Backspace:
		if (polyItem_->isVisible()) {
			polyItem_->removeLastCommittedPoint();
			if (polyItem_->empty()) {
				clear();
			}
		}
		break;
	}
}

void PolygonTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() != Qt::LeftButton) {
		return;
	}
	auto pos = scene()->pixmapPosFromScene(event->scenePos());
	if (scene()->canvasItem()->pixmapRect().contains(pos)) {
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
					emit scene()->newCommand(new DrawCommand(scene()->canvasItem(), new Polygon(poly)));
					clear();
				}
			}
			else {
				polyItem_->commitPoint(pos - polyItem_->pos() + QPointF(.5, .5));
				if (!markerItem_->acceptHoverEvents() && polyItem_->canClose())
					markerItem_->setAcceptHoverEvents(true);
			}
		}
	}
}

void PolygonTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (polyItem_->isVisible()) {
		polyItem_->setLastPoint(scene()->pixmapPosFromScene(event->scenePos()) - polyItem_->pos() + QPointF(.5, .5));
	}
}
