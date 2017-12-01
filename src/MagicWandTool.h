#ifndef MAGICWANDTOOL_H
#define MAGICWANDTOOL_H

#include "Tool.h"
#include <QScopedPointer>
#include <QPoint>

class GuideLineItem;
class Selection;

class MagicWandTool : public Tool
{
public:
	MagicWandTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	void clear() override;

protected:
	void onActivate() override;
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
	void rebuildSelection(double tolerance);
	void apply();

private:
	double maxToleranceScreenDistance_;
	bool pressed_;
	QPointF start_;
	QPoint pixmapStartPos_;
	QScopedPointer<Selection> selection_;
	GuideLineItem* guideLine_;
};

#endif // MAGICWANDTOOL_H
