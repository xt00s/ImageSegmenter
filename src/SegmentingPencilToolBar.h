#ifndef SEGMENTINGPENCILTOOLBAR_H
#define SEGMENTINGPENCILTOOLBAR_H

#include <QToolBar>

class QLabel;
class QDoubleSpinBox;
class QCheckBox;
class QSpinBox;
class QToolButton;

class SegmentingPencilToolBar : public QToolBar
{
public:
    SegmentingPencilToolBar();

    QLabel* bandwidthLabel;
    QDoubleSpinBox* bandwidthSpinBox;
    QAction* actionIncreaseBandwidth;
    QAction* actionDecreaseBandwidth;
    QCheckBox* smoothCheckBox;
    QAction* actionIncreaseSmoothRadius;
    QAction* actionDecreaseSmoothRadius;
    QSpinBox* smoothRadiusSpinBox;
    QToolButton* finishButton;
};

#endif // SEGMENTINGPENCILTOOLBAR_H
