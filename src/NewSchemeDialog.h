#ifndef NEWSCHEMEDIALOG_H
#define NEWSCHEMEDIALOG_H

#include <QDialog>
#include <QTreeWidget>

namespace Ui {
    class NewSchemeDialog;
}

class CategoryList : public QTreeWidget
{
public:
    CategoryList(QWidget *parent = 0);
    void setup();
protected:
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override;
};

class NewSchemeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewSchemeDialog(QWidget *parent = 0);
    ~NewSchemeDialog();

    void accept() override;
    QString savedPath() const;

private:
    bool save();

private slots:
    void newCategory();
    void removeCategory();
    void categoryClicked(QTreeWidgetItem *item, int column);
    void browse();

private:
    Ui::NewSchemeDialog *ui;
};

#endif // NEWSCHEMEDIALOG_H
