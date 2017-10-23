#ifndef OPENFOLDERDIALOG_H
#define OPENFOLDERDIALOG_H

#include <QDialog>

class QSettings;
class QComboBox;
namespace Ui {
class OpenFolderDialog;
}

class OpenFolderDialog : public QDialog
{
	Q_OBJECT

public:
	explicit OpenFolderDialog(QWidget *parent = 0);
	~OpenFolderDialog();

	QString schemePath() const;
	QString folderPath() const;
	QString outputPath() const;

	void accept() override;

private slots:
	void browseScheme();
	void browseFolder();
	void browseOutput();
	void createScheme();

private:
	void saveSettings();
	void restoreSettings();
	void saveComboSettings(QSettings& settings, const QString& key, const QComboBox& combo, int maxItems = 10);
	void restoreComboSettings(QSettings& settings, const QString& key, QComboBox& combo);

private:
	Ui::OpenFolderDialog *ui;
};

#endif // OPENFOLDERDIALOG_H
