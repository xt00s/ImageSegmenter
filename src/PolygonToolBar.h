#ifndef POLYGONTOOLBAR_H
#define POLYGONTOOLBAR_H

#include <QToolBar>

class QCheckBox;

class PolygonToolBar : public QToolBar
{
public:
    PolygonToolBar(QWidget* parent = nullptr);

    QCheckBox* alignToPixelsCheckBox;
};

#endif // POLYGONTOOLBAR_H
