#include "SegmentingPencilTool.h"
#include "SegmentingPencilToolBar.h"
#include "Selection.h"
#include "SegmentationScene.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "Helper.h"
#include <QDoubleSpinBox>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QToolButton>

SegmentingPencilTool::SegmentingPencilTool(QAction* action, SegmentationScene* scene, QObject *parent)
	: Tool(action, scene, parent)
	, pressed_(false)
	, pressedButton_(0)
	, cursor_(QPixmap(":/image/icons/cross_cursor.png"))
{
	toolbar_ = new SegmentingPencilToolBar;
	toolbar_->bandwidthSpinBox->setValue(20);
	toolbar_->finishButton->setEnabled(false);
	connect(toolbar_->bandwidthSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double){ rebuildSelection(); });
	connect(toolbar_->finishButton, &QToolButton::clicked, [this](bool){ apply(); });
}

QToolBar* SegmentingPencilTool::toolbar() const
{
	return toolbar_;
}

void SegmentingPencilTool::clear()
{
	scene()->overlayItem()->clear();
	drawButtons_.clear();
	selection_.reset();
	toolbar_->finishButton->setEnabled(false);
}

void SegmentingPencilTool::onActivate()
{
	scene()->setViewCursor(cursor_);
	scene()->overlayItem()->show();
}

void SegmentingPencilTool::onDeactivate()
{
	scene()->unsetViewCursor();
	scene()->overlayItem()->hide();
	clear();
}

void SegmentingPencilTool::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Enter:
	case Qt::Key_Return:
		apply();
		break;
	case Qt::Key_Backspace:
		scene()->overlayItem()->undo();
		if (!drawButtons_.empty()) {
			drawButtons_.removeLast();
			rebuildSelection();
		}
		break;
	case Qt::Key_Escape:
		clear();
		break;
	}
}

void SegmentingPencilTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_ || (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)) {
		return;
	}
	auto pixmapPos = scene()->pixmapPosFromScene(event->scenePos());
	if (scene()->canvasItem()->pixmapRect().contains(pixmapPos)) {
		pressedButton_ = event->button();
		drawingColor_ = QColor(event->button() == Qt::LeftButton ? Qt::red : Qt::blue);
		lastPos_ = scene()->overlayItem()->mapFromScene(event->scenePos());
		drawButtons_ << (event->button() == Qt::LeftButton);
		scene()->overlayItem()->beginDrawing();
		scene()->overlayItem()->draw(Circle(lastPos_, .5), drawingColor_);
		pressed_ = true;
	}
}

void SegmentingPencilTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_) {
		auto pos = scene()->overlayItem()->mapFromScene(event->scenePos());
		scene()->overlayItem()->draw(Line(QLineF(lastPos_, pos), 1), drawingColor_);
		lastPos_ = pos;
	}
}

void SegmentingPencilTool::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_ && event->button() == pressedButton_) {
		scene()->overlayItem()->endDrawing();
		rebuildSelection();
		pressed_ = false;
	}
}

void SegmentingPencilTool::rebuildSelection()
{
	if (!drawButtons_.contains(true) || !drawButtons_.contains(false)) {
		selection_.reset();
		toolbar_->finishButton->setEnabled(false);
		return;
	}
	auto image = scene()->canvasItem()->pixmap().toImage();
	auto seeds = scene()->overlayItem()->pixmap().toImage();
	auto sigma = 0.5 + 15 * toolbar_->bandwidthSpinBox->value() / 100;
	auto bmp = help::segmentIGC(image, seeds, Qt::red, Qt::blue, sigma);
	selection_.reset(new Selection(bmp, true, scene(), this));
	toolbar_->finishButton->setEnabled(true);
}

void SegmentingPencilTool::apply()
{
	if (!selection_.isNull()) {
		auto category = scene()->canvasItem()->category();
		if (category) {
			auto filledRect = selection_->filledAreaRect();
			auto bmp = new Bitmap(filledRect.topLeft(), selection_->filledArea());
			emit scene()->newCommand(new DrawCommand(scene()->canvasItem(), bmp));
		}
	}
	clear();
}
