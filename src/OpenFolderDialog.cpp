#include "OpenFolderDialog.h"
#include "ui_OpenFolderDialog.h"
#include "NewSchemeDialog.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QDir>

OpenFolderDialog::OpenFolderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenFolderDialog)
{
    ui->setupUi(this);
    restoreSettings();
    connect(ui->browseSchemeButton, &QPushButton::clicked, this, &OpenFolderDialog::browseScheme);
    connect(ui->browseFolderButton, &QPushButton::clicked, this, &OpenFolderDialog::browseFolder);
    connect(ui->browseOutputButton, &QPushButton::clicked, this, &OpenFolderDialog::browseOutput);
    connect(ui->createSchemeButton, &QPushButton::clicked, this, &OpenFolderDialog::createScheme);
}

OpenFolderDialog::~OpenFolderDialog()
{
    delete ui;
}

void OpenFolderDialog::saveSettings()
{
    QSettings settings;
    saveComboSettings(settings, "Scheme", *ui->schemeCombo);
    saveComboSettings(settings, "Folder", *ui->folderCombo);
    saveComboSettings(settings, "Output", *ui->outputCombo);
}

void OpenFolderDialog::restoreSettings()
{
    QSettings settings;
    restoreComboSettings(settings, "Scheme", *ui->schemeCombo);
    restoreComboSettings(settings, "Folder", *ui->folderCombo);
    restoreComboSettings(settings, "Output", *ui->outputCombo);
}

void OpenFolderDialog::saveComboSettings(QSettings& settings, const QString& key, const QComboBox& combo, int maxItems)
{
    QStringList paths;
    for (int i = 0; i < combo.count(); i++) {
        paths << combo.itemText(i);
    }
    auto currentPath = combo.currentText();
    paths.removeAll(currentPath);
    if (!currentPath.isEmpty()) {
        paths.insert(0, currentPath);
    }
    if (paths.count() > maxItems) {
        paths.erase(paths.begin() + maxItems, paths.end());
    }
    for (int i = 0; i < paths.count(); i++) {
        settings.setValue(QString("Recent-%1Path%2").arg(key).arg(i), paths[i]);
    }
}

void OpenFolderDialog::restoreComboSettings(QSettings& settings, const QString& key, QComboBox& combo)
{
    int i = 0;
    QString path;
    do {
        path = settings.value(QString("Recent-%1Path%2").arg(key).arg(i++)).toString();
        if (!path.isEmpty()) {
            combo.addItem(path);
        }
    } while (!path.isEmpty());
    combo.setCurrentIndex(0);
}

QString OpenFolderDialog::schemePath() const
{
    return ui->schemeCombo->currentText();
}

QString OpenFolderDialog::folderPath() const
{
    return ui->folderCombo->currentText();
}

QString OpenFolderDialog::outputPath() const
{
    return ui->outputCombo->currentText();
}

void OpenFolderDialog::browseScheme()
{
    auto path = QFileDialog::getOpenFileName(this, "Choose Scheme", schemePath(), "Scheme Files (*.json)");
    if (!path.isNull()) {
        ui->schemeCombo->setCurrentText(path);
    }
}

void OpenFolderDialog::browseFolder()
{
    auto path = QFileDialog::getExistingDirectory(this, "Choose Folder", folderPath());
    if (!path.isNull()) {
        ui->folderCombo->setCurrentText(path);
        ui->outputCombo->setCurrentText(QDir(path).filePath("output"));
    }
}

void OpenFolderDialog::browseOutput()
{
    auto path = QFileDialog::getExistingDirectory(this, "Choose Output Folder", outputPath());
    if (!path.isNull()) {
        ui->outputCombo->setCurrentText(path);
    }
}

void OpenFolderDialog::createScheme()
{
    NewSchemeDialog dialog(this);
    if (dialog.exec()) {
        ui->schemeCombo->setCurrentText(dialog.savedPath());
    }
}

void OpenFolderDialog::accept()
{
    if (ui->schemeCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "", "No scheme path chosen", QMessageBox::Ok);
        return;
    }
    if (ui->folderCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "", "No folder chosen", QMessageBox::Ok);
        return;
    }
    if (ui->outputCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "", "No output folder chosen", QMessageBox::Ok);
        return;
    }
    if (QDir(ui->folderCombo->currentText()) == QDir(ui->outputCombo->currentText())) {
        QMessageBox::warning(this, "", "Images folder and output folder have to be different", QMessageBox::Ok);
        return;
    }
    saveSettings();
    QDialog::accept();
}
