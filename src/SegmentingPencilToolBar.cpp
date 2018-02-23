#include "SegmentingPencilToolBar.h"
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QToolButton>

SegmentingPencilToolBar::SegmentingPencilToolBar()
{
	bandwidthLabel = new QLabel("Bandwidth:", this);
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

	auto spacer = new QWidget(this);
	spacer->setFixedWidth(5);

	smoothCheckBox = new QCheckBox("Smooth:", this);
	smoothCheckBox->setObjectName("smoothCheckBox");
	smoothCheckBox->setChecked(false);

	actionIncreaseSmoothRadius = new QAction("Increase smooth radius", this);
	actionIncreaseSmoothRadius->setIcon(QIcon(":/image/icons/plus-in-circle.svg"));
	actionIncreaseSmoothRadius->setObjectName("actionIncreaseSmoothRadius");
	actionIncreaseSmoothRadius->setEnabled(false);

	actionDecreaseSmoothRadius = new QAction("Decrease smooth radius", this);
	actionDecreaseSmoothRadius->setIcon(QIcon(":/image/icons/minus-in-circle.svg"));
	actionDecreaseSmoothRadius->setObjectName("actionDecreaseSmoothRadius");
	actionDecreaseSmoothRadius->setEnabled(false);

	smoothRadiusSpinBox = new QSpinBox(this);
	smoothRadiusSpinBox->setObjectName("smoothRadiusSpinBox");
	smoothRadiusSpinBox->setWrapping(false);
	smoothRadiusSpinBox->setKeyboardTracking(false);
	smoothRadiusSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
	smoothRadiusSpinBox->setMaximumWidth(20);
	smoothRadiusSpinBox->setRange(1, 10);
	smoothRadiusSpinBox->setSingleStep(1);
	smoothRadiusSpinBox->setToolTip("Smooth radius");
	smoothRadiusSpinBox->setEnabled(false);

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
	addWidget(spacer);
	addWidget(smoothCheckBox);
	addAction(actionDecreaseSmoothRadius);
	addWidget(smoothRadiusSpinBox);
	addAction(actionIncreaseSmoothRadius);
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
	connect(actionIncreaseSmoothRadius, &QAction::triggered, [this]{
		smoothRadiusSpinBox->setValue(smoothRadiusSpinBox->value() + smoothRadiusSpinBox->singleStep());
	});
	connect(actionDecreaseSmoothRadius, &QAction::triggered, [this]{
		smoothRadiusSpinBox->setValue(smoothRadiusSpinBox->value() - smoothRadiusSpinBox->singleStep());
	});
	connect(smoothCheckBox, &QCheckBox::toggled, [this](bool checked){
		actionIncreaseSmoothRadius->setEnabled(checked);
		actionDecreaseSmoothRadius->setEnabled(checked);
		smoothRadiusSpinBox->setEnabled(checked);
	});
}
