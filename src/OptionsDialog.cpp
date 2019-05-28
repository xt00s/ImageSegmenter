#include "src\OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "Options.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    ui->resetToolSelectionCheckBox->setChecked(Options::get().resetToolSelection);
    ui->resetCategorySelectionCheckBox->setChecked(Options::get().resetCategorySelection);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}


void OptionsDialog::accept()
{
    Options::get().resetToolSelection = ui->resetToolSelectionCheckBox->isChecked();
    Options::get().resetCategorySelection = ui->resetCategorySelectionCheckBox->isChecked();
    QDialog::accept();
}
