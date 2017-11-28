#ifndef POLYGONTOOL_H
#define POLYGONTOOL_H

#include "Tool.h"
#include <QPoint>

class PolylineItem;
class StartMarkerItem;

class PolygonTool : public Tool
{
public:
	PolygonTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	void clear() override;

protected:
	void onActivate() override;
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
	PolylineItem* polyItem_;
	StartMarkerItem* markerItem_;
	QPoint polyPixmapPos_;
};

#endif // POLYGONTOOL_H
