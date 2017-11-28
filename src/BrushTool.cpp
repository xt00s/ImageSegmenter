#include "BrushTool.h"
#include "SegmentationScene.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "BrushToolBar.h"
#include <QGraphicsSceneMouseEvent>
#include <QComboBox>

BrushTool::BrushTool(QAction* action, SegmentationScene* scene, QObject* parent)
	: Tool(action, scene, parent)
	, pressed_(false)
	, crossCursor_(QPixmap(":/image/icons/cross_cursor.png"))
{
	brushCursorItem_ = new BrushCursorItem;
	brushCursorItem_->hide();
	scene->addItem(brushCursorItem_);

	auto toolbar = new BrushToolBar;
	connect(toolbar->widthCombo, &QComboBox::currentTextChanged, this, &BrushTool::widthChanged);
	brushCursorItem_->setWidth(toolbar->widthCombo->currentText().toInt());
	toolbar_ = toolbar;
}

void BrushTool::clear()
{
}

void BrushTool::onActivate()
{
	scene()->setViewCursor(crossCursor_);
}

void BrushTool::onDeactivate()
{
	scene()->unsetViewCursor();
	brushCursorItem_->hide();
}

void BrushTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_ || event->button() != Qt::LeftButton) {
		return;
	}
	if (brushCursorItem_->isVisible()) {
		auto pos = scene()->canvasItem()->mapFromScene(brushCursorItem_->pos());
		Circle circle(pos, brushCursorItem_->width() / 2);
		pressedRect_ = circle.rect();
		canvasCopy_.reset(scene()->canvasItem()->extractFragment(QRect()));
		scene()->canvasItem()->draw(circle);
		scene()->setViewCursor(Qt::BlankCursor);
	}
	pressed_ = true;
}

void BrushTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (!brushCursorItem_->isVisible()) {
		brushCursorItem_->show();
	}
	else if (pressed_) {
		auto start = scene()->canvasItem()->mapFromScene(brushCursorItem_->pos());
		auto end = scene()->canvasItem()->mapFromScene(event->scenePos());
		Line line(QLineF(start, end), brushCursorItem_->width());
		pressedRect_ |= line.rect();
		scene()->canvasItem()->draw(line);
	}
	brushCursorItem_->setPos(event->scenePos());
}

void BrushTool::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
	if (pressed_) {
		if (brushCursorItem_->isVisible()) {
			emit scene()->newCommand(new DrawFragmentCommand(scene()->canvasItem(), canvasCopy_->extract(pressedRect_)));
			canvasCopy_.reset();
			scene()->setViewCursor(crossCursor_);
		}
		pressed_ = false;
	}
}

void BrushTool::leaveEvent(QEvent*)
{
	brushCursorItem_->hide();
}

void BrushTool::widthChanged(const QString& width)
{
	brushCursorItem_->setWidth(width.toInt());
}
