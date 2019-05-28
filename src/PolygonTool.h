#ifndef POLYGONTOOL_H
#define POLYGONTOOL_H

#include "Tool.h"
#include <QPoint>

class PolygonToolBar;
class PolylineItem;
class StartMarkerItem;

class PolygonTool : public Tool
{
public:
    PolygonTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

    QToolBar* toolbar() const override;
    void clear() override;

protected:
    void onActivate() override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    PolygonToolBar* toolbar_;
    PolylineItem* polyItem_;
    StartMarkerItem* markerItem_;
};

#endif // POLYGONTOOL_H
