#ifndef MAGICWANDTOOL_H
#define MAGICWANDTOOL_H

#include "Tool.h"
#include <QScopedPointer>
#include <QPoint>

class GuideLineItem;
class Selection;
class MagicWandToolBar;

class MagicWandTool : public Tool
{
public:
    MagicWandTool(QAction* action, SegmentationScene* scene, QObject *parent = nullptr);

    QToolBar* toolbar() const override;
    void clear() override;

protected:
    void onActivate() override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private slots:
    void toleranceChanged(double tolerance);

private:
    void rebuildSelection(double tolerance);
    void apply();
    void updateTolerance();

private:
    MagicWandToolBar* toolbar_;
    double maxToleranceScreenDistance_;
    double toleranceFactor_;
    bool pressed_;
    QPoint pixmapStartPos_;
    QScopedPointer<Selection> selection_;
    GuideLineItem* guideLine_;
};

#endif // MAGICWANDTOOL_H
