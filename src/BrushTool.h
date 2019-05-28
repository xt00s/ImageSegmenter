#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include "Tool.h"
#include "GraphicsItems.h"
#include <QCursor>

class BrushToolBar;
class GuideLineItem;

class BrushTool : public Tool
{
    Q_OBJECT
public:
    BrushTool(QAction* action, SegmentationScene* scene, QObject *parent = nullptr);

    QToolBar* toolbar() const override;
    void clear() override;

protected:
    void onActivate() override;
    void onDeactivate() override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void widthChanged(const QString& width);
    void defineWidthToggled(bool checked);

private:
    BrushToolBar* toolbar_;
    bool pressed_;
    QRect pressedRect_;
    BrushCursorItem* brushCursorItem_;
    QScopedPointer<CanvasItem::Fragment> canvasCopy_;
    QCursor crossCursor_;
    GuideLineItem* widthGuideLine_;
};

#endif // BRUSHTOOL_H
