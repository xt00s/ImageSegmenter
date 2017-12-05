#include "MagicWandToolBar.h"
#include "HandlessSlider.h"
#include <QLabel>

MagicWandToolBar::MagicWandToolBar(QWidget* parent)
	: QToolBar(parent)
{
	toleranceLabel = new QLabel("Tolerance: ", this);
	toleranceLabel->setObjectName("toleranceLabel");
	toleranceLabel->setIndent(7);
	toleranceSlider = new HandlessSlider(this);
	toleranceSlider->setObjectName("toleranceSlider");
	toleranceSlider->setFixedSize(150, 16);

	addWidget(toleranceLabel);
	addWidget(toleranceSlider);
	setObjectName("magicWandToolbar");
}
