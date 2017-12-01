#include "MagicWandTool.h"
#include "Selection.h"
#include "SegmentationScene.h"
#include "GraphicsItems.h"
#include "Helper.h"
#include "Category.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include <QApplication>
#include <QScreen>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QVector2D>

MagicWandTool::MagicWandTool(QAction* action, SegmentationScene* scene, QObject* parent)
	: Tool(action, scene, parent)
	, pressed_(false)
{
	auto screenSize = QApplication::primaryScreen()->size();
	maxToleranceScreenDistance_ = qMin(screenSize.width(), screenSize.height()) * 0.9;
	guideLine_ = new GuideLineItem();
	guideLine_->hide();
	guideLine_->setZValue(1);
	scene->addItem(guideLine_);
}

void MagicWandTool::clear()
{
	selection_.reset();
	scene()->canvasItem()->setClipRegionVisible(true);
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
	}
}

void MagicWandTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_ || event->button() != Qt::LeftButton) {
		return;
	}
	pixmapStartPos_ = scene()->pixmapPosFromScene(event->scenePos());
	if (scene()->canvasItem()->pixmapRect().contains(pixmapStartPos_)) {
		start_ = event->scenePos();
		scene()->canvasItem()->setClipRegionVisible(false);
		rebuildSelection(0);
		guideLine_->show();
		guideLine_->setLine(QLineF(event->scenePos(), event->scenePos()));
		pressed_ = true;
	}
}

void MagicWandTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (pressed_) {
		guideLine_->setLine(QLineF(guideLine_->line().p1(), event->scenePos()));
		rebuildSelection(QVector2D(event->scenePos() - start_).length() / maxToleranceScreenDistance_);
	}
}

void MagicWandTool::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	guideLine_->hide();
	pressed_ = false;
}

void MagicWandTool::rebuildSelection(double tolerance)
{
	auto image = scene()->canvasItem()->pixmap().toImage();
	auto maxD2 = qRound(3*255*255 * tolerance);
	auto rgb = image.pixel(pixmapStartPos_);
	int sr = qRed(rgb), sg = qGreen(rgb), sb = qBlue(rgb);

	auto bmp = help::flood(image, pixmapStartPos_, [sr,sg,sb,maxD2](QRgb rgb){
			int dr = sr-qRed(rgb), dg = sg-qGreen(rgb), db = sb-qBlue(rgb);
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
