#ifndef MAGICWANDTOOLBAR_H
#define MAGICWANDTOOLBAR_H

#include <QToolBar>

class QLabel;
class HandlessSlider;
class QToolButton;

class MagicWandToolBar : public QToolBar
{
public:
    MagicWandToolBar(QWidget *parent = nullptr);

    QLabel* toleranceLabel;
    HandlessSlider* toleranceSlider;
    QToolButton* finishButton;
};

#endif // MAGICWANDTOOLBAR_H
