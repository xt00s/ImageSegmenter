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
	bandwidthSpinBox->setKeyboardTracking(false);
	bandwidthSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
	bandwidthSpinBox->setMaximumWidth(50);
	bandwidthSpinBox->setDecimals(1);
	bandwidthSpinBox->setSuffix(" %");
	bandwidthSpinBox->setRange(0, 100);
	bandwidthSpinBox->setSingleStep(1);

	actionIncreaseBandwidth = new QAction("Increase bandwidth", this);
	actionIncreaseBandwidth->setIcon(QIcon(":/image/icons/plus-in-circle.svg"));
	actionIncreaseBandwidth->setObjectName("actionIncreaseBandwidth");

	actionDecreaseBandwidth = new QAction("Decrease bandwidth", this);
	actionDecreaseBandwidth->setIcon(QIcon(":/image/icons/minus-in-circle.svg"));
	actionDecreaseBandwidth->setObjectName("actionDecreaseBandwidth");

	finishButton = new QToolButton(this);
	finishButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	finishButton->setText("Finish");
	finishButton->setIcon(QIcon(":/image/icons/check.svg"));
	finishButton->setFixedHeight(20);

	addWidget(bandwidthLabel);
	addAction(actionDecreaseBandwidth);
	addWidget(bandwidthSpinBox);
	addAction(actionIncreaseBandwidth);
	addSeparator();
	addWidget(finishButton);
	setIconSize(QSize(16, 16));
	setObjectName("segmentingPencilToolbar");

	connect(actionIncreaseBandwidth, &QAction::triggered, [this]{
		bandwidthSpinBox->setValue(bandwidthSpinBox->value() + bandwidthSpinBox->singleStep());
	});
	connect(actionDecreaseBandwidth, &QAction::triggered, [this]{
		bandwidthSpinBox->setValue(bandwidthSpinBox->value() - bandwidthSpinBox->singleStep());
	});
}
