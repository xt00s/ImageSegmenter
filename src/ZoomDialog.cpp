#include "ZoomDialog.h"
#include "ui_ZoomDialog.h"

ZoomDialog::ZoomDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ZoomDialog)
    , zoom_(1)
    , izoom_(100)
{
    ui->setupUi(this);
    ui->keepZoomSpinBox->setEnabled(ui->keepCheckBox->isChecked());

    connect(ui->keepCheckBox, &QCheckBox::toggled, [this](bool checked){ ui->keepZoomSpinBox->setEnabled(checked); });
}

ZoomDialog::~ZoomDialog()
{
    delete ui;
}

double ZoomDialog::zoom() const
{
    return izoom_ == ui->zoomSpinBox->value() ? zoom_ : double(ui->zoomSpinBox->value()) / 100;
}

void ZoomDialog::setZoom(double zoom)
{
    if (zoom_ != zoom) {
        zoom_ = zoom;
        izoom_ = int(zoom * 100);
        ui->zoomSpinBox->setValue(izoom_);
    }
}

bool ZoomDialog::keepZoomLevel() const
{
    return ui->keepCheckBox->isChecked();
}

void ZoomDialog::setKeepZoomLevel(bool keep)
{
    ui->keepCheckBox->setChecked(keep);
}

double ZoomDialog::keepingZoom() const
{
    return double(ui->keepZoomSpinBox->value()) / 100;
}

void ZoomDialog::setKeepingZoom(double zoom)
{
    ui->keepZoomSpinBox->setValue(int(zoom * 100));
}
