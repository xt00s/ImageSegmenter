#ifndef SEGMENTINGPENCILTOOL_H
#define SEGMENTINGPENCILTOOL_H

#include "Tool.h"
#include <QCursor>
#include <QVector>
#include <QColor>

class Selection;
class SegmentingPencilToolBar;

class SegmentingPencilTool : public Tool
{
public:
	SegmentingPencilTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	QToolBar* toolbar() const override;
	void clear() override;

protected:
	void onActivate() override;
	void onDeactivate() override;
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
	void rebuildSelection();
	void apply();

	double getWidth() const;

private:
	SegmentingPencilToolBar* toolbar_;
	QScopedPointer<Selection> selection_;
	QVector<bool> drawButtons_;
	bool pressed_;
	int pressedButton_;
	double width_;
	QPointF lastPos_;
	QColor drawingColor_;
	QCursor cursor_;
};

#endif // SEGMENTINGPENCILTOOL_H
