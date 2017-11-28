#ifndef TOOLS_H
#define TOOLS_H

#include <QObject>

class QUndoCommand;
class QKeyEvent;
class QGraphicsSceneMouseEvent;
class QAction;
class QToolBar;
class SegmentationScene;

class Tool : public QObject
{
	Q_OBJECT
public:
	Tool(QAction* action, SegmentationScene* scene, QObject *parent = 0);

	void activate();
	void deactivate();
	virtual void clear() = 0;

	QAction* action() const;
	QToolBar* toolbar() const;
	SegmentationScene* scene() const;

signals:
	void activated();
	void deactivated();

protected:
	virtual void onActivate() = 0;
	virtual void onDeactivate();
	virtual void keyPressEvent(QKeyEvent *event) {}
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) {}
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) {}
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {}
	virtual void leaveEvent(QEvent* event) {}

private slots:
	void actionTriggered();

private:
	friend class SegmentationScene;
	QAction* action_;
	SegmentationScene* scene_;
protected:
	QToolBar* toolbar_;
};

inline QAction* Tool::action() const { return action_; }
inline QToolBar* Tool::toolbar() const { return toolbar_; }
inline SegmentationScene* Tool::scene() const { return scene_; }

#endif // TOOLS_H
