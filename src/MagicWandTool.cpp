#include "MagicWandTool.h"
#include "MagicWandToolBar.h"
#include "Selection.h"
#include "SegmentationScene.h"
#include "GraphicsItems.h"
#include "Helper.h"
#include "Category.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "HandlessSlider.h"
#include <QToolButton>
#include <QApplication>
#include <QScreen>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QVector2D>

MagicWandTool::MagicWandTool(QAction* action, SegmentationScene* scene, QObject* parent)
	: Tool(action, scene, parent)
	, pressed_(false)
	, toleranceFactor_(1)
{
	auto screenSize = QApplication::primaryScreen()->size();
	maxToleranceScreenDistance_ = qMin(screenSize.width(), screenSize.height()) * 0.9;
	guideLine_ = new GuideLineItem();
	guideLine_->hide();
	guideLine_->setZValue(1);
	scene->addItem(guideLine_);

	toolbar_ = new MagicWandToolBar;
	connect(toolbar_->toleranceSlider, &Slider::valueChanged, this, &MagicWandTool::toleranceChanged);
	connect(toolbar_->finishButton, &QToolButton::clicked, [this]{ apply(); });
	toolbar_->finishButton->setEnabled(false);
}

QToolBar* MagicWandTool::toolbar() const
{
	return toolbar_;
}

void MagicWandTool::clear()
{
	selection_.reset();
	scene()->canvasItem()->setClipRegionVisible(true);
	toolbar_->finishButton->setEnabled(false);
}

void MagicWandTool::onActivate()
{
}

void MagicWandTool::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Enter:
	case Qt::Key_Return:
		apply();
		break;
	case Qt::Key_Escape:
		clear();
		break;
	case Qt::Key_Shift:
		toleranceFactor_ = 2;
		if (pressed_) {
			updateTolerance();
		}
		break;
	}
}

void MagicWandTool::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Shift:
		toleranceFactor_ = 1;
		if (pressed_) {
			updateTolerance();
		}
		break;
	}
}

void MagicWandTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_ || event->button() != Qt::LeftButton) {
		return;
	}
	pixmapStartPos_ = scene()->pixmapPosFromScene(event->scenePos());
	if (scene()->canvasItem()->pixmapRect().contains(pixmapStartPos_)) {
		scene()->canvasItem()->setClipRegionVisible(false);
		guideLine_->show();
		guideLine_->setLine(QLineF(event->scenePos(), event->scenePos()));
		pressed_ = true;
		if (!toolbar_->toleranceSlider->value()) {
			rebuildSelection(0);
		}
		toolbar_->toleranceSlider->setValue(0);
		toolbar_->finishButton->setEnabled(true);
	}
}

void MagicWandTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_) {
		guideLine_->setLine(QLineF(guideLine_->line().p1(), event->scenePos()));
		updateTolerance();
	}
}

void MagicWandTool::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		guideLine_->hide();
		pressed_ = false;
	}
}

void MagicWandTool::toleranceChanged(double tolerance)
{
	if (pressed_ || !selection_.isNull()) {
		rebuildSelection(tolerance);
	}
}

void MagicWandTool::rebuildSelection(double tolerance)
{
	auto image = scene()->canvasItem()->pixmap().toImage();
	auto rgb = image.pixel(pixmapStartPos_);
	auto r = qRed(rgb), g = qGreen(rgb), b = qBlue(rgb);

	auto rd = r > 127 ? r : 255-r;
	auto gd = g > 127 ? g : 255-g;
	auto bd = b > 127 ? b : 255-b;
	auto maxD2 = qRound((rd*rd + gd*gd + bd*bd) * tolerance);

	auto bmp = help::flood(image, pixmapStartPos_, [r,g,b,maxD2](QRgb rgb) {
			int dr = r-qRed(rgb), dg = g-qGreen(rgb), db = b-qBlue(rgb);
			return dr*dr + dg*dg + db*db <= maxD2;
	});
	selection_.reset(new Selection(bmp, true, scene(), this));
}

void MagicWandTool::apply()
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

void MagicWandTool::updateTolerance()
{
	auto tolerance = guideLine_->line().length() / maxToleranceScreenDistance_ * toleranceFactor_;
	toolbar_->toleranceSlider->setValue(tolerance);
}
