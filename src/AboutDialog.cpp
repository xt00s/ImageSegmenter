#include "src/AboutDialog.h"
#include "ui_AboutDialog.h"
#include <QApplication>
#include <QSysInfo>

#ifndef NO_VERSION_HEADER
#include "version.h"
#endif

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	ui->appNameLabel->setText(QApplication::applicationName() + " " + QApplication::applicationVersion());
	ui->buildDateLabel->setText(QString("Built on ") + BUILD_DATE_TIME);
	ui->qtVersionLabel->setText(QString("Based on Qt %1 (%2, %3)")
								.arg(QT_VERSION_STR)
								.arg(QSysInfo::buildCpuArchitecture())
								.arg(R"(<a href="http://doc.qt.io/qt-5/lgpl.html">license</a>)"));
	ui->qtVersionLabel->setOpenExternalLinks(true);
	setFixedSize(400, 143);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
