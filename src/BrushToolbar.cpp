#include "BrushToolBar.h"
#include "SegmentationScene.h"
#include <QApplication>

BrushToolBar::BrushToolBar(QWidget *parent)
    : ToolToolBar(parent)
{
	widthLabel = new QLabel("Brush width:", this);
	widthLabel->setObjectName("brushWidthLabel");
	widthLabel->setIndent(7);
	widthCombo = new QComboBox(this);
	widthCombo->setObjectName("brushWidthCombo");
	widthCombo->setEditable(true);
	widthCombo->setValidator(new QIntValidator(1, 1000, this));
	int ranges[][3] = {
		{1,1,20},
		{20,5,100},
		{100,25,500},
		{500,50,1000}
	};
	QStringList widths;
	for (int r = 0; r < _countof(ranges); r++) {
		for (int i = ranges[r][0]; i < ranges[r][2]; i += ranges[r][1]) {
			widths << QString::number(i);
		}
	}
	widthCombo->addItems(widths << "1000");
    widthCombo->setEditText("20");
    actionIncreaseWidth = new QAction("Increase brush width", this);
	actionIncreaseWidth->setIcon(QIcon(":/image/icons/rounded_plus.svg"));
	actionIncreaseWidth->setObjectName("actionIncreaseWidth");
    actionIncreaseWidth->setShortcuts({QKeySequence("]"), QKeySequence("Ctrl+]"), QKeySequence("Shift+]")});
	actionDecreaseWidth = new QAction("Decrease brush width", this);
	actionDecreaseWidth->setIcon(QIcon(":/image/icons/rounded_minus.svg"));
	actionDecreaseWidth->setObjectName("actionDecreaseWidth");
    actionDecreaseWidth->setShortcuts({QKeySequence("["), QKeySequence("Ctrl+["), QKeySequence("Shift+[")});

	addWidget(widthLabel);
	addAction(actionDecreaseWidth);
	addWidget(widthCombo);
	addAction(actionIncreaseWidth);
	setIconSize(QSize(16, 16));
	setObjectName("brushToolbar");

	connect(actionIncreaseWidth, &QAction::triggered, this, &BrushToolBar::incWidth);
	connect(actionDecreaseWidth, &QAction::triggered, this, &BrushToolBar::decWidth);
}

void BrushToolBar::activate(SegmentationScene& scene)
{
	scene.setBrushWidth(widthCombo->currentText().toInt());
}

void BrushToolBar::incWidth()
{
	int v = widthCombo->currentText().toInt();
	if (v < 1000) {
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            for (int i = 0; i < widthCombo->count(); i++) {
                if (widthCombo->itemText(i).toInt() > v) {
                    widthCombo->setCurrentIndex(i);
                    return;
                }
            }
        } else {
            auto inc = QApplication::keyboardModifiers() & Qt::ControlModifier ? 5 : 1;
            widthCombo->setCurrentText(QString::number(qMin(v + inc, 1000)));
        }
	}
}

void BrushToolBar::decWidth()
{
	int v = widthCombo->currentText().toInt();
	if (v > 1) {
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            for (int i = 0; i < widthCombo->count(); i++) {
                if (widthCombo->itemText(i).toInt() >= v) {
                    widthCombo->setCurrentIndex(i-1);
                    return;
                }
            }
        } else {
            auto inc = QApplication::keyboardModifiers() & Qt::ControlModifier ? 5 : 1;
            widthCombo->setCurrentText(QString::number(qMax(v - inc, 1)));
        }
	}
}
