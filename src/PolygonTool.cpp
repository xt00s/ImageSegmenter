#include "PolygonTool.h"
#include "PolygonToolBar.h"
#include "SegmentationScene.h"
#include "GraphicsItems.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "Helper.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCheckBox>

PolygonTool::PolygonTool(QAction* action, SegmentationScene* scene, QObject* parent)
	: Tool(action, scene, parent)
{
	markerItem_ = new StartMarkerItem(scene->canvasItem());
	markerItem_->hide();
	polyItem_ = new PolylineItem(scene->canvasItem());
	polyItem_->hide();

	toolbar_ = new PolygonToolBar;
}

QToolBar* PolygonTool::toolbar() const
{
	return toolbar_;
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
	auto pixmapPos = scene()->pixmapPosFromScene(event->scenePos());
	if (scene()->canvasItem()->pixmapRect().contains(pixmapPos)) {
		auto pos = scene()->canvasItem()->mapFromScene(event->scenePos());
		auto align = toolbar_->alignToPixelsCheckBox->isChecked();
		if (!polyItem_->isVisible()) {
			polyItem_->setPos(align ? pixmapPos + QPointF(.5, .5) : pos);
			polyItem_->begin();
			polyItem_->show();
			markerItem_->setPos(align ? pixmapPos + QPointF(.5, .5) : pos);
			markerItem_->setAcceptHoverEvents(false);
			markerItem_->show();
		}
		else {
			if (markerItem_->isHovering()) {
				if (polyItem_->canClose()) {
					polyItem_->commit();
					auto poly = polyItem_->polygon().translated(polyItem_->pos());
					emit scene()->newCommand(new DrawCommand(scene()->canvasItem(), new Polygon(poly)));
					clear();
				}
			}
			else {
				polyItem_->commitPoint(align ? pixmapPos - help::floor(polyItem_->pos()) : pos - polyItem_->pos());
				if (!markerItem_->acceptHoverEvents() && polyItem_->canClose())
					markerItem_->setAcceptHoverEvents(true);
			}
		}
	}
}

void PolygonTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (polyItem_->isVisible()) {
		auto pos = scene()->canvasItem()->mapFromScene(event->scenePos());
		polyItem_->setLastPoint(toolbar_->alignToPixelsCheckBox->isChecked() ?
									help::floor(pos) - help::floor(polyItem_->pos()) :
									pos - polyItem_->pos());
	}
}
