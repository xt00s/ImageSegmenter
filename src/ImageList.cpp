#include "ImageList.h"
#include "Helper.h"
#include <QFileInfo>
#include <QDir>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QAction>

#define SEGM_COLOR	QColor(128,128,128)
#define CURSOR_SIZE QSize(16,16)

enum ImageListRole {
    PathRole = Qt::UserRole,
    CurrentRole
};

class ImageListIconDelegate : public QStyledItemDelegate
{
public:
    ImageListIconDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , cursor_(QIcon(":/image/icons/image-cursor.svg").pixmap(CURSOR_SIZE))
    {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);
        if (index.data(CurrentRole).toBool()) {
            auto x = (option.rect.width() - cursor_.width()) / 2;
            auto y = (option.rect.height() - cursor_.height()) / 2;
            painter->drawPixmap(option.rect.topLeft() + QPoint(x,y), cursor_);
        }
    }
private:
    QPixmap cursor_;
};

ImageList::ImageList(QWidget *parent)
    : QTreeWidget(parent)
    , selected_(-1)
    , segmented_(0)
    , contextMenu_(nullptr)
    , createEmptyMaskAction_(nullptr)
    , removeMaskAction_(nullptr)
{
    createContextMenu();
    boldFont_.setBold(true);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &ImageList::itemDoubleClicked);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &ImageList::showContextMenu);
}

void ImageList::setup()
{
    setItemDelegateForColumn(0, new ImageListIconDelegate(this));
    setColumnWidth(0, 20);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void ImageList::createContextMenu()
{
    contextMenu_ = new QMenu(this);
    createEmptyMaskAction_ = contextMenu_->addAction("Create Empty Mask", this, &ImageList::createEmptyMask);
    removeMaskAction_ = contextMenu_->addAction("Remove Mask", this, &ImageList::removeMask);
}

void ImageList::open(const QString &path, const QString& outputPath, const QString &schemeName)
{
    QTreeWidget::clear();
    int lastSelected = selected_;

    outputPath_ = outputPath;
    schemeName_ = schemeName;
    selected_ = -1;
    segmented_ = 0;

    int first = -1;
    QStringList filters({"*.bmp", "*.png", "*.jpg", "*.jpeg"});
    for (auto& f : QDir(path).entryInfoList(filters, QDir::NoFilter, QDir::Name)) {
        auto p = new QTreeWidgetItem(invisibleRootItem());
        p->setText(1, f.baseName());
        p->setData(0, PathRole, f.absoluteFilePath());
        p->setToolTip(1, p->data(0, PathRole).toString());
        if (segmented(p)) {
            p->setTextColor(1, SEGM_COLOR);
            segmented_++;
        } else if (first < 0) {
            first = row(p);
        }
    }
    select(first < 0 ? 0 : first);
    if (selected_ < 0 && lastSelected >= 0) {
        emit selected(QString());
    }
    emit progressChanged();
}

void ImageList::clear()
{
    int old = selected_;
    selected_ = -1;
    QTreeWidget::clear();
    if (old >= 0) {
        emit selected(QString());
    }
}

void ImageList::next()
{
    int n;
    for (n = qMax(0, selected_ + 1); n < count(); n++) {
        if (!markedAsSegmented(item(n))) {
            break;
        }
    }
    select(n < count() ? n : selected_ + 1);
}

void ImageList::prev()
{
    int n;
    for (n = selected_ - 1; n >= 0; n--) {
        if (!markedAsSegmented(item(n))) {
            break;
        }
    }
    select(n >= 0 ? n : selected_ - 1);
}

bool ImageList::isPrevAvailable() const
{
    return selected_ > 0;
}

bool ImageList::isNextAvailable() const
{
    return selected_ < (count()-1);
}

void ImageList::itemDoubleClicked(QTreeWidgetItem* item, int column)
{
    select(row(item));
}

void ImageList::showContextMenu(const QPoint& pos)
{
    auto item = itemAt(pos);
    if (!item)
        return;
    createEmptyMaskAction_->setEnabled(!markedAsSegmented(item));
    createEmptyMaskAction_->setData(pos);
    removeMaskAction_->setEnabled(markedAsSegmented(item));
    removeMaskAction_->setData(pos);
    contextMenu_->popup(mapToGlobal(pos));
}

void ImageList::createEmptyMask()
{
    auto item = itemAt(createEmptyMaskAction_->data().toPoint());
    if (item && !markedAsSegmented(item)) {
        emit createEmptyMaskRequested(item->data(0, PathRole).toString());
        update(row(item));
        if (markedAsSegmented(item)) {
            segmented_++;
            emit progressChanged();
        }
    }
    createEmptyMaskAction_->setData(QVariant());
}

void ImageList::removeMask()
{
    auto item = itemAt(removeMaskAction_->data().toPoint());
    if (item && markedAsSegmented(item)) {
        emit removeMaskRequested(item->data(0, PathRole).toString());
        update(row(item));
        if (!markedAsSegmented(item)) {
            segmented_--;
            emit progressChanged();
        }
    }
    removeMaskAction_->setData(QVariant());
}

bool ImageList::segmented(QTreeWidgetItem *item)
{
    auto maskPath = help::segmentationMaskFilePath(item->data(0, PathRole).toString(), outputPath_, schemeName_);
    return !maskPath.isEmpty() && QFileInfo::exists(maskPath);
}

bool ImageList::markedAsSegmented(QTreeWidgetItem* item)
{
    return item->textColor(1) == SEGM_COLOR;
}

void ImageList::update(int index)
{
    auto p = item(index);
    p->setData(0, CurrentRole, index == selected_);
    p->setFont(1, index == selected_ ? boldFont_ : QFont());
    if (segmented(p)) {
        p->setTextColor(1, SEGM_COLOR);
    } else {
        p->setData(1, Qt::TextColorRole, QVariant());
    }
}

void ImageList::select(int index)
{
    if (index < 0 || index >= count() || selected_ == index) {
        return;
    }
    int prev = selected_;
    bool prevMarked = prev >= 0 ? markedAsSegmented(item(prev)) : false;
    bool marked = markedAsSegmented(item(index));
    selected_ = index;
    setCurrentItem(item(selected_));
    scrollToItem(item(selected_), QAbstractItemView::PositionAtCenter);
    emit selected(item(selected_)->data(0, PathRole).toString());
    bool segmChanged = false;
    if (prev >= 0) {
        update(prev);
        if (prevMarked != markedAsSegmented(item(prev))) {
            segmented_ += prevMarked ? -1 : 1;
            segmChanged = true;
        }
    }
    update(selected_);
    if (marked != markedAsSegmented(item(selected_))) {
        segmented_ += marked ? -1 : 1;
        segmChanged = true;
    }
    if (segmChanged) {
        emit progressChanged();
    }
}
