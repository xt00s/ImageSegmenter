#include "MagicWandToolBar.h"
#include "HandlessSlider.h"
#include <QLabel>
#include <QToolButton>

MagicWandToolBar::MagicWandToolBar(QWidget* parent)
	: QToolBar(parent)
{
	toleranceLabel = new QLabel("Tolerance: ", this);
	toleranceLabel->setObjectName("toleranceLabel");
	toleranceLabel->setIndent(5);

	toleranceSlider = new HandlessSlider(this);
	toleranceSlider->setObjectName("toleranceSlider");
	toleranceSlider->setFixedSize(150, 16);
	toleranceSlider->setTextFormatter([](double v){ return QString("%1%").arg(v*100,0,'f',1); });

	finishButton = new QToolButton(this);
	finishButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	finishButton->setText("Finish");
	finishButton->setIcon(QIcon(":/image/icons/check.svg"));
	finishButton->setFixedHeight(20);

	addWidget(toleranceLabel);
	addWidget(toleranceSlider);
	addSeparator();
	addWidget(finishButton);
	setObjectName("magicWandToolbar");
}
