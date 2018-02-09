#include "SegmentingPencilToolBar.h"
#include <QLabel>
#include <QDoubleSpinBox>
#include <QToolButton>

SegmentingPencilToolBar::SegmentingPencilToolBar()
{
	bandwidthLabel = new QLabel("Bandwidth: ", this);
	bandwidthLabel->setObjectName("bandwidthLabel");
	bandwidthLabel->setIndent(5);

	bandwidthSpinBox = new QDoubleSpinBox(this);
	bandwidthSpinBox->setObjectName("bandwidthSpinBox");
	bandwidthSpinBox->setWrapping(false);
	bandwidthSpinBox->setSuffix(" %");
	bandwidthSpinBox->setRange(0, 100);
	bandwidthSpinBox->setSingleStep(1);

	finishButton = new QToolButton(this);
	finishButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	finishButton->setText("Finish");
	finishButton->setIcon(QIcon(":/image/icons/check.svg"));
	finishButton->setFixedHeight(20);

	addWidget(bandwidthLabel);
	addWidget(bandwidthSpinBox);
	addSeparator();
	addWidget(finishButton);
	setObjectName("segmentingPencilToolbar");
}
