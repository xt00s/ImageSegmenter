#ifndef TOOLTOOLBAR_H
#define TOOLTOOLBAR_H

#include <QToolBar>

class SegmentationScene;

class ToolToolBar : public QToolBar
{
public:
	ToolToolBar(QWidget *parent = 0);
	virtual void activate(SegmentationScene& scene) = 0;
};

#endif // TOOLTOOLBAR_H
