#include "SchemeTree.h"
#include "Helper.h"
#include <QJsonDocument>
#include <QFile>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QItemSelectionModel>
#include <QPainter>
#include <QMouseEvent>
#include <QShortcut>

#define COLOR_ICON_SIZE QSize(20,20)
#define STATE_ICON_SIZE QSize(13,13)

enum SchemeTreeRole {
	PixmapRole = Qt::UserRole,
	ColorRole,
	ClipperRole
};

class SchemeTreeIconDelegate : public QStyledItemDelegate
{
public:
	SchemeTreeIconDelegate(QObject *parent = 0)
		: QStyledItemDelegate(parent)
	{}
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		QStyledItemDelegate::paint(painter, option, index);

		auto pixmap = index.data(PixmapRole).value<QPixmap>();
		auto x = (option.rect.width() - pixmap.width()) / 2;
		auto y = (option.rect.height() - pixmap.height()) / 2;
		QRect r(option.rect.topLeft() + QPoint(x,y), pixmap.size());
		painter->drawPixmap(r, pixmap);

		if (index.data(ClipperRole).toBool()) {
			int m = 3;
			auto color = index.data(ColorRole).value<QColor>();
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setBrush(Qt::NoBrush);
			painter->setPen(QPen(qGray(color.rgb()) > 127 ? Qt::black : Qt::white, 1.5, Qt::DotLine));
			painter->drawEllipse(r.adjusted(m,m,-m,-m));
			painter->restore();
		}
	}
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		return COLOR_ICON_SIZE + QSize(0,2);
	}
};

class SchemeTreeSelectionModel : public QItemSelectionModel
{
public:
	SchemeTreeSelectionModel(QAbstractItemModel *model, QObject* parent = 0)
		: QItemSelectionModel(model, parent)
	{}
	void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override
	{
		if (command & Select) {
			for (auto& r : selection) {
				if (r.top() == 0)
					break;
				if (r.right() == 0 || (r.right() >= 2 && r.left() <= 3))
					return;
			}
		}
		QItemSelectionModel::select(selection, command);
	}
};

SchemeTree::SchemeTree(QWidget *parent)
	: QTreeWidget(parent)
{
	eraserCategory_.reset(new Category);
	eraserCategory_->setName("Eraser");

	statePixmaps_[0] = QIcon(":/image/icons/lock.svg").pixmap(STATE_ICON_SIZE);
	statePixmaps_[1] = help::lightenPixmap(statePixmaps_[0], 0.2);
	statePixmaps_[2] = QIcon(":/image/icons/eye.svg").pixmap(STATE_ICON_SIZE);
	statePixmaps_[3] = help::lightenPixmap(statePixmaps_[2], 0.2);

	setMouseTracking(true);
	setupShortcuts();
	connect(this, &QTreeWidget::itemClicked, this, &SchemeTree::itemClicked);
}

void SchemeTree::setupShortcuts()
{
	for (int i = 0; i < 10; i++) {
		auto categoryShortcut = new QShortcut(i ? QString::number(i) : "`", this);
		connect(categoryShortcut, &QShortcut::activated, [this,i]{
			if (i >= 0 && i < invisibleRootItem()->childCount()) {
				setCurrentItem(invisibleRootItem()->child(i), 1);
			}
		});
	}
	auto clipShortcut = new QShortcut(QString("C"), this);
	connect(clipShortcut, &QShortcut::activated, [this]{ itemClicked(currentItem(), 0); });
}

bool SchemeTree::open(const QString &path)
{
	bool had = !scheme_.isNull();
	scheme_.reset();
	QTreeWidget::clear();

	bool failed = false;
	for (;;) {
		QFile file(path);
		if (!file.open(QFile::ReadOnly)) {
			errorString_ = QString("Can't open the file '%1': %2").arg(path).arg(file.errorString());
			failed = true;
			break;
		}
		QJsonParseError error;
		auto json = QJsonDocument::fromJson(file.readAll(), &error);
		if (error.error != QJsonParseError::NoError) {
			errorString_ = QString("Invalid json file '%1': %2").arg(path).arg(error.errorString());
			failed = true;
			break;
		}
		scheme_.reset(Scheme::fromJson(json.object()));
		if (scheme_->name().isEmpty()) {
			errorString_ = "The scheme has no name";
			failed = true;
			break;
		}
		if (!scheme_->categoryCount()) {
			errorString_ = "The scheme is empty";
			failed = true;
			break;
		}
		break;
	}
	if (failed) {
		if (had) {
			emit schemeChanged(0);
		}
		scheme_.reset();
		return false;
	}
	addCategory(eraserCategory_.data());
	for (auto& c : scheme_->categories()) {
		addCategory(c);
	}
	setCurrentItem(invisibleRootItem()->child(1), 1);
	emit schemeChanged(scheme_.data());
	return true;
}

void SchemeTree::clear()
{
	bool had = !scheme_.isNull();
	scheme_.reset();
	QTreeWidget::clear();
	if (had) {
		emit schemeChanged(scheme_.data());
	}
}

void SchemeTree::resetCategoryStates()
{
	for (int i = 1; i < invisibleRootItem()->childCount(); i++) {
		auto item = invisibleRootItem()->child(i);
		auto c = itemCategory(item);
		c->setVisible(true);
		c->setLocked(false);
		item->setIcon(2, statePixmaps_[c->isLocked() ? 0 : 1]);
		item->setIcon(3, statePixmaps_[c->isVisible() ? 2 : 3]);
		item->setData(0, ClipperRole, false);
	}
}

Category *SchemeTree::currentCategory() const
{
	auto c = currentItem();
	return c ? itemCategory(c) : 0;
}

void SchemeTree::itemClicked(QTreeWidgetItem *item, int column)
{
	auto c = itemCategory(item);
	if (c->index() >= 0) {
		switch (column) {
		case 0: {
				auto clipper = item->data(0, ClipperRole).toBool();
				if (!clipper) {
					for (int i = 1; i < invisibleRootItem()->childCount(); i++) {
						invisibleRootItem()->child(i)->setData(0, ClipperRole, false);
					}
				}
				item->setData(0, ClipperRole, !clipper);
				emit clipChanged(clipper ? 0 : c);
				break;
			}
		case 2:
			c->setLocked(!c->isLocked());
			item->setIcon(2, statePixmaps_[c->isLocked() ? 0 : 1]);
			emit lockedChanged(c);
			break;
		case 3:
			c->setVisible(!c->isVisible());
			item->setIcon(3, statePixmaps_[c->isVisible() ? 2 : 3]);
			emit visibilityChanged(c);
			break;
		}
	}
}

void SchemeTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	auto indexes = deselected.indexes();
	if (!indexes.empty()) {
		auto prev = itemFromIndex(indexes[0]);
		if (prev) {
			prev->setFont(1, QFont());
		}
	}
	indexes = selected.indexes();
	if (!indexes.empty()) {
		auto cur = itemFromIndex(indexes[0]);
		if (cur) {
			QFont f;
			f.setBold(true);
			cur->setFont(1, f);
			emit this->selected(itemCategory(cur));
		} else {
			emit this->selected(0);
		}
	} else {
		emit this->selected(0);
	}
	QTreeWidget::selectionChanged(selected, deselected);
}

void SchemeTree::addCategory(Category* cat)
{
	auto item = new QTreeWidgetItem(invisibleRootItem());
	item->setText(1, cat->name());
	item->setData(1, Qt::UserRole, reinterpret_cast<qlonglong>(cat));
	if (cat->index() < 0) {
		item->setData(0, PixmapRole, QIcon(":/image/icons/eraser.svg").pixmap(COLOR_ICON_SIZE));
	} else {
		item->setData(0, ColorRole, cat->color());
		item->setData(0, PixmapRole, help::colorPixmap(COLOR_ICON_SIZE, cat->color()));
		item->setData(0, ClipperRole, false);
		item->setIcon(2, statePixmaps_[cat->isLocked() ? 0 : 1]);
		item->setIcon(3, statePixmaps_[cat->isVisible() ? 2 : 3]);
	}
}

Category *SchemeTree::itemCategory(QTreeWidgetItem *item) const
{
	return reinterpret_cast<Category*>(item->data(1, Qt::UserRole).toULongLong());
}

void SchemeTree::setup()
{
	auto oldSelectionModel = selectionModel();
	setSelectionModel(new SchemeTreeSelectionModel(model(), this));
	delete oldSelectionModel;

	setItemDelegateForColumn(0, new SchemeTreeIconDelegate(this));
	int widths[] = { 24, 100, 24, 24 };
	for (int i = 0; i < columnCount(); i++) {
		setColumnWidth(i, abs(widths[i]));
	}
	header()->setSectionResizeMode(0, QHeaderView::Fixed);
	header()->setSectionResizeMode(1, QHeaderView::Stretch);
	header()->setSectionResizeMode(2, QHeaderView::Fixed);
	header()->setSectionResizeMode(3, QHeaderView::Fixed);
}

void SchemeTree::mouseMoveEvent(QMouseEvent* event)
{
	QTreeWidget::mouseMoveEvent(event);
	setCursor(QCursor(itemAt(event->pos()) ? Qt::PointingHandCursor : Qt::ArrowCursor));
}

void SchemeTree::leaveEvent(QEvent* event)
{
	QTreeWidget::leaveEvent(event);
	setCursor(QCursor());
}
