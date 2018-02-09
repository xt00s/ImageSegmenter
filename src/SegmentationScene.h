#ifndef SEGMENTATIONSCENE_H
#define SEGMENTATIONSCENE_H

#include <QGraphicsScene>

class QUndoCommand;
class CanvasItem;
class OverlayItem;
class Tool;

class SegmentationScene : public QGraphicsScene
{
	Q_OBJECT
public:
	SegmentationScene(QObject *parent = 0);

	void setup();
	void updateSceneRect();

	CanvasItem* canvasItem() const;
	OverlayItem* overlayItem() const;
	Tool* tool() const;

	QPoint pixmapPosFromScene(const QPointF& scenePos) const;
	void setViewCursor(const QCursor& cursor);
	void unsetViewCursor();

signals:
	void mousePosChanged(const QPoint& pos);
	void newCommand(QUndoCommand* command);

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	friend class Tool;
	CanvasItem* canvasItem_;
	OverlayItem* overlayItem_;
	Tool* tool_;
};

inline CanvasItem* SegmentationScene::canvasItem() const { return canvasItem_; }
inline OverlayItem*SegmentationScene::overlayItem() const { return overlayItem_; }
inline Tool* SegmentationScene::tool() const { return tool_; }

#endif // SEGMENTATIONSCENE_H
