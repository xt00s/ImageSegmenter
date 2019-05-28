#include "AboutDialog.h"
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
    ui->licenseLabel->setText(QString("The software is based on Qt %1 (%2, %3) and licensed under %4.")
                              .arg(QT_VERSION_STR)
                              .arg(QSysInfo::buildCpuArchitecture())
                              .arg(R"(<a href="http://doc.qt.io/qt-5/lgpl.html">license</a>)")
                              .arg(R"(<a href="https://opensource.org/licenses/MIT">MIT license</a>)"));
    ui->licenseLabel->setOpenExternalLinks(true);
    setFixedSize(400, 143);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
