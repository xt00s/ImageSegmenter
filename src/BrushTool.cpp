#include "BrushTool.h"
#include "SegmentationScene.h"
#include "UndoCommands.h"
#include "Drawables.h"
#include "BrushToolBar.h"
#include <QGraphicsSceneMouseEvent>
#include <QComboBox>
#include <QGraphicsView>
#include <QToolButton>
#include <QKeyEvent>

BrushTool::BrushTool(QAction* action, SegmentationScene* scene, QObject* parent)
    : Tool(action, scene, parent)
    , pressed_(false)
    , crossCursor_(QPixmap(":/image/icons/cross_cursor.png"))
{
    brushCursorItem_ = new BrushCursorItem;
    brushCursorItem_->hide();
    scene->addItem(brushCursorItem_);

    widthGuideLine_ = new GuideLineItem();
    widthGuideLine_->hide();
    widthGuideLine_->setZValue(1);
    scene->addItem(widthGuideLine_);

    toolbar_ = new BrushToolBar;
    connect(toolbar_->widthCombo, &QComboBox::currentTextChanged, this, &BrushTool::widthChanged);
    connect(toolbar_->defineWidthButton, &QToolButton::toggled, this, &BrushTool::defineWidthToggled);
    brushCursorItem_->setWidth(toolbar_->widthCombo->currentText().toInt());
}

QToolBar* BrushTool::toolbar() const
{
    return toolbar_;
}

void BrushTool::clear()
{
}

void BrushTool::onActivate()
{
    scene()->setViewCursor(crossCursor_);
    auto view = scene()->views()[0];
    if (view->underMouse()) {
        auto viewPos = view->mapFromGlobal(QCursor::pos());
        brushCursorItem_->setPos(view->mapToScene(viewPos));
        brushCursorItem_->show();
    }
}

void BrushTool::onDeactivate()
{
    scene()->unsetViewCursor();
    brushCursorItem_->hide();
    toolbar_->defineWidthButton->setChecked(false);
}

void BrushTool::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        if (!pressed_) {
            toolbar_->defineWidthButton->setChecked(false);
        }
        break;
    }
}

void BrushTool::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (pressed_ || event->button() != Qt::LeftButton) {
        return;
    }
    if (toolbar_->defineWidthButton->isChecked()) {
        widthGuideLine_->setLine(QLineF(event->scenePos(), event->scenePos()));
        widthGuideLine_->show();
    }
    else if (brushCursorItem_->isVisible()) {
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
    if (toolbar_->defineWidthButton->isChecked()) {
        if (pressed_) {
            widthGuideLine_->setLine(QLineF(widthGuideLine_->line().p1(), event->scenePos()));
        }
    }
    else {
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
}

void BrushTool::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (pressed_ && event->button() == Qt::LeftButton) {
        if (toolbar_->defineWidthButton->isChecked()) {
            auto width = qRound(widthGuideLine_->line().length());
            if (width > 0) {
                toolbar_->widthCombo->setCurrentText(QString::number(width));
            }
            widthGuideLine_->hide();
            toolbar_->defineWidthButton->setChecked(false);
        }
        else if (brushCursorItem_->isVisible()) {
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

void BrushTool::defineWidthToggled(bool checked)
{
    if (checked) {
        scene()->unsetViewCursor();
        brushCursorItem_->hide();
    } else {
        onActivate();
    }
}
