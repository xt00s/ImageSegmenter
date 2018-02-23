#include "SegmentingPencilTool.h"
#include "SegmentingPencilToolBar.h"
#include "Selection.h"
#include "SegmentationScene.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "Helper.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QToolButton>
#include <QGraphicsView>

SegmentingPencilTool::SegmentingPencilTool(QAction* action, SegmentationScene* scene, QObject *parent)
	: Tool(action, scene, parent)
	, pressed_(false)
	, pressedButton_(0)
	, smoothKernel_(help::diskBitmap(1))
	, cursor_(QPixmap(":/image/icons/cross_cursor.png"))
{
	toolbar_ = new SegmentingPencilToolBar;
	toolbar_->bandwidthSpinBox->setValue(20);
	toolbar_->smoothRadiusSpinBox->setValue(1);
	toolbar_->finishButton->setEnabled(false);
	connect(toolbar_->bandwidthSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this]{ rebuildSelection(); });
	connect(toolbar_->smoothCheckBox, &QCheckBox::toggled, [this]{ resmoothSelection(); });
	connect(toolbar_->finishButton, &QToolButton::clicked, [this]{ apply(); });
	connect(toolbar_->smoothRadiusSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int radius){
		smoothKernel_ = help::diskBitmap(radius);
		resmoothSelection();
	});
}

QToolBar* SegmentingPencilTool::toolbar() const
{
	return toolbar_;
}

void SegmentingPencilTool::clear()
{
	scene()->overlayItem()->clear();
	scene()->canvasItem()->setClipRegionVisible(true);
	drawButtons_.clear();
	selection_.reset();
	segmResult_ = QImage();
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
		width_ = getWidth();
		pressed_ = true;
		scene()->overlayItem()->beginDrawing();
		scene()->overlayItem()->draw(Circle(lastPos_, width_ / 2), drawingColor_);
		scene()->canvasItem()->setClipRegionVisible(false);
	}
}

void SegmentingPencilTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_) {
		auto pos = scene()->overlayItem()->mapFromScene(event->scenePos());
		scene()->overlayItem()->draw(Line(QLineF(lastPos_, pos), width_), drawingColor_);
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
		segmResult_ = QImage();
		if (drawButtons_.empty()) {
			scene()->canvasItem()->setClipRegionVisible(true);
		}
		toolbar_->finishButton->setEnabled(false);
		return;
	}
	help::WaitCursor wait;
	auto image = scene()->canvasItem()->pixmap().toImage();
	auto seeds = scene()->overlayItem()->pixmap().toImage();
	auto sigma = 0.5 + 15 * toolbar_->bandwidthSpinBox->value() / 100;
	segmResult_ = help::segmentIGC(image, seeds, Qt::red, Qt::blue, sigma);
	selection_.reset(new Selection(smoothed(), true, scene(), this));
	scene()->canvasItem()->setClipRegionVisible(false);
	toolbar_->finishButton->setEnabled(true);
}

void SegmentingPencilTool::resmoothSelection()
{
	if (!selection_.isNull()) {
		selection_.reset(new Selection(smoothed(), true, scene(), this));
	}
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

double SegmentingPencilTool::getWidth() const
{
	auto mapedUnit = scene()->views()[0]->transform().map(QLineF(0,0,1,0));
	return qMax(1., ceil(1 / mapedUnit.length()));
}

QImage SegmentingPencilTool::smoothed() const
{
	if (toolbar_->smoothCheckBox->isChecked()) {
		auto bmp = help::imclose(segmResult_, smoothKernel_);
		return help::imopen(bmp, smoothKernel_);
	}
	return segmResult_;
}
