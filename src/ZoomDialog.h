#ifndef ZOOMDIALOG_H
#define ZOOMDIALOG_H

#include <QDialog>

namespace Ui {
    class ZoomDialog;
}

class ZoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZoomDialog(QWidget *parent = 0);
    ~ZoomDialog();

    double zoom() const;
    void setZoom(double zoom);

    bool keepZoomLevel() const;
    void setKeepZoomLevel(bool keep);

    double keepingZoom() const;
    void setKeepingZoom(double zoom);

private:
    Ui::ZoomDialog *ui;
    double zoom_;
    int izoom_;
};

#endif // ZOOMDIALOG_H
