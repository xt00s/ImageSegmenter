#ifndef MAGICWANDTOOL_H
#define MAGICWANDTOOL_H

#include "Tool.h"

class MagicWandTool : public Tool
{
public:
	MagicWandTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	void clear() override;

protected:
	void onActivate() override;
	void onDeactivate() override;
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
};

#endif // MAGICWANDTOOL_H
