#include "ImageList.h"
#include "Helper.h"
#include <QFileInfo>
#include <QDir>

ImageList::ImageList(QWidget *parent)
	: QListWidget(parent)
	, emptyIcon_(help::emptyIcon(QSize(16, 16)))
	, selectedIcon_(":/image/icons/image-cursor.svg")
	, selected_(-1)
	, segmented_(0)
{
	boldFont_.setBold(true);
	connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
}

void ImageList::open(const QString &path, const QString& outputPath, const QString &schemeName)
{
	QListWidget::clear();
	int lastSelected = selected_;

	outputPath_ = outputPath;
	schemeName_ = schemeName;
	selected_ = -1;
	segmented_ = 0;

	int first = -1;
	QStringList filters({"*.bmp", "*.png", "*.jpg", "*.jpeg"});
	for (auto& f : QDir(path).entryInfoList(filters, QDir::NoFilter, QDir::Name)) {
		auto p = new QListWidgetItem(emptyIcon_, f.baseName(), this);
		p->setData(Qt::UserRole, f.absoluteFilePath());
		p->setToolTip(p->data(Qt::UserRole).toString());
		if (segmented(p)) {
			p->setBackgroundColor("lightgreen");
			segmented_++;
		} else if (first < 0) {
			first = row(p);
		}
		addItem(p);
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
	QListWidget::clear();
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

void ImageList::itemDoubleClicked(QListWidgetItem *item)
{
	select(row(item));
}

bool ImageList::segmented(QListWidgetItem *item)
{
	auto maskPath = help::segmentationMaskFilePath(item->data(Qt::UserRole).toString(), outputPath_, schemeName_);
	return !maskPath.isEmpty() && QFileInfo::exists(maskPath);
}

bool ImageList::markedAsSegmented(QListWidgetItem* item)
{
	return item->backgroundColor() == QColor("lightgreen");
}

void ImageList::update(int index)
{
	item(index)->setIcon(index == selected_ ? selectedIcon_ : emptyIcon_);
	item(index)->setFont(index == selected_ ? boldFont_ : QFont());
	if (segmented(item(index))) {
		item(index)->setBackgroundColor("lightgreen");
	} else {
		item(index)->setData(Qt::BackgroundColorRole, QVariant());
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
	emit selected(item(selected_)->data(Qt::UserRole).toString());
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

