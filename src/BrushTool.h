#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include "Tool.h"
#include "GraphicsItems.h"
#include <QCursor>

class BrushTool : public Tool
{
	Q_OBJECT
public:
	BrushTool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	void clear() override;

protected:
	void onActivate() override;
	void onDeactivate() override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

private slots:
	void widthChanged(const QString& width);

private:
	bool pressed_;
	QRect pressedRect_;
	BrushCursorItem* brushCursorItem_;
	QScopedPointer<CanvasItem::Fragment> canvasCopy_;
	QCursor crossCursor_;
};

#endif // BRUSHTOOL_H
