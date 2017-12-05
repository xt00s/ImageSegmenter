#include "Tool.h"
#include "SegmentationScene.h"
#include <QAction>

Tool::Tool(QAction* action, SegmentationScene* scene, QObject *parent)
	: QObject(parent)
	, action_(action)
	, scene_(scene)
{
	connect(action, &QAction::triggered, this, &Tool::actionTriggered);
}

void Tool::activate()
{
	if (scene()->tool_ != this) {
		if (scene()->tool_)
			scene()->tool_->deactivate();
		scene()->tool_ = this;
		action_->setChecked(true);
		onActivate();
		emit activated();
	}
}

void Tool::deactivate()
{
	if (scene()->tool_ == this) {
		scene()->tool_ = 0;
		action_->setChecked(false);
		onDeactivate();
		emit deactivated();
	}
}

void Tool::actionTriggered()
{
	if (scene()->tool_ == this) {
		action_->setChecked(true);
	} else {
		activate();
	}
}

void Tool::onDeactivate()
{
    clear();
}
