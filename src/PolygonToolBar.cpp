#include "PolygonToolBar.h"
#include <QCheckBox>

PolygonToolBar::PolygonToolBar(QWidget* parent)
	: QToolBar(parent)
{
	auto spacer = new QWidget(this);
	spacer->setFixedWidth(5);

	alignToPixelsCheckBox = new QCheckBox("&Align to pixels", this);
	alignToPixelsCheckBox->setObjectName("alignToPixelsCheckBox");
	alignToPixelsCheckBox->setToolTip("Align polygon vertices to image pixels");
	alignToPixelsCheckBox->setChecked(false);

	addWidget(spacer);
	addWidget(alignToPixelsCheckBox);
	setObjectName("polygonToolBar");
}
