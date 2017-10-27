#ifndef SEGMENTATIONSCENE_H
#define SEGMENTATIONSCENE_H

#include <QGraphicsScene>
#include <QScopedPointer>
#include <QCursor>
#include "GraphicsItems.h"

class QUndoCommand;

class SegmentationScene : public QGraphicsScene
{
	Q_OBJECT
public:
	enum class Tool { Polygon, Brush };

public:
	SegmentationScene(QObject *parent = 0);

	void setup();
	void updateSceneRect();
	void setBrushWidth(qreal width);
	void clearToolState();

	CanvasItem* canvasItem() const;

	Tool tool() const;
	void setTool(Tool tool);

signals:
	void mousePosChanged(const QPoint& pos);
	void newCommand(QUndoCommand* command);

private:
	QPoint pixmapPosFromScene(const QPointF& scenePos) const;
	void setViewCursor(const QCursor& cursor);
	void unsetViewCursor();

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	CanvasItem* canvasItem_;
	PolylineItem* polyItem_;
	StartMarkerItem* markerItem_;
	BrushCursorItem* brushCursorItem_;
	QCursor crossCursor_;
	QScopedPointer<CanvasItem::Fragment> canvasCopy_;
	QPoint polyPixmapPos_;
	Tool tool_;
	bool pressed_;
	QRect pressedRect_;
};

inline CanvasItem* SegmentationScene::canvasItem() const { return canvasItem_; }
inline SegmentationScene::Tool SegmentationScene::tool() const { return tool_; }

#endif // SEGMENTATIONSCENE_H
