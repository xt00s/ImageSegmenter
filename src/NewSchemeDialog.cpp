#include "src\NewSchemeDialog.h"
#include "ui_NewSchemeDialog.h"
#include "Scheme.h"
#include <QEvent>
#include <QMouseEvent>
#include <QColorDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>

class ColorDelegate : public QStyledItemDelegate
{
public:
	ColorDelegate(QObject *parent = 0)
		: QStyledItemDelegate(parent)
	{}
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyledItemDelegate::paint(painter, option, index);
		auto color = index.data(Qt::BackgroundColorRole).value<QColor>();
		painter->fillRect(option.rect.adjusted(0,1,-1,-1), color);
	}
};

CategoryList::CategoryList(QWidget* parent)
	: QTreeWidget(parent)
{
	setMouseTracking(true);
}

void CategoryList::setup()
{
	setItemDelegateForColumn(1, new ColorDelegate(this));
	setHeaderLabels({ "Name", "Color"});
	setColumnWidth(1, 80);
	header()->setSectionResizeMode(0, QHeaderView::Stretch);
	header()->setSectionResizeMode(1, QHeaderView::Fixed);
}

void CategoryList::mouseMoveEvent(QMouseEvent* event)
{
	QTreeWidget::mouseMoveEvent(event);
	auto index = indexAt(event->pos());
	setCursor(QCursor(index.isValid() && index.column() == 1 ? Qt::PointingHandCursor : Qt::ArrowCursor));
}

void CategoryList::leaveEvent(QEvent* event)
{
	QTreeWidget::leaveEvent(event);
	setCursor(QCursor());
}

bool CategoryList::edit(const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* event)
{
	if (index.column() == 1)
		return false;
	return QTreeWidget::edit(index, trigger, event);
}


NewSchemeDialog::NewSchemeDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::NewSchemeDialog)
{
	ui->setupUi(this);
	ui->categoryList->setup();
	ui->removeCategoryButton->setEnabled(false);

	connect(ui->newCategoryButton, &QPushButton::clicked, this, &NewSchemeDialog::newCategory);
	connect(ui->removeCategoryButton, &QPushButton::clicked, this, &NewSchemeDialog::removeCategory);
	connect(ui->categoryList, &CategoryList::itemClicked, this, &NewSchemeDialog::categoryClicked);
	connect(ui->browseButton, &QPushButton::clicked, this, &NewSchemeDialog::browse);
}

NewSchemeDialog::~NewSchemeDialog()
{
	delete ui;
}

void NewSchemeDialog::accept()
{
	if (!ui->categoryList->invisibleRootItem()->childCount()) {
		QMessageBox::warning(this, "", "No categories added", QMessageBox::Ok);
		return;
	}
	if (ui->nameEdit->text().isEmpty()) {
		QMessageBox::warning(this, "", "No name specified", QMessageBox::Ok);
		return;
	}
	if (ui->pathEdit->text().isEmpty()) {
		QMessageBox::warning(this, "", "No path chosen", QMessageBox::Ok);
		return;
	}
	if (!save()) {
		return;
	}
	QSettings().setValue("LastNewSchemePath", ui->pathEdit->text());
	QDialog::accept();
}

QString NewSchemeDialog::savedPath() const
{
	return ui->pathEdit->text();
}

bool NewSchemeDialog::save()
{
	QFile file(ui->pathEdit->text());
	if (!file.open(QFile::WriteOnly)) {
		auto message = QString("Can't open file '%1': %2")
				.arg(ui->pathEdit->text())
				.arg(file.errorString());
		QMessageBox::warning(this, "", message, QMessageBox::Ok);
		return false;
	}
	Scheme scheme;
	scheme.setName(ui->nameEdit->text());
	for (int i = 0; i < ui->categoryList->invisibleRootItem()->childCount(); i++) {
		auto item = ui->categoryList->invisibleRootItem()->child(i);
		auto c = new Category;
		c->setName(item->text(0));
		c->setColor(item->backgroundColor(1));
		scheme.addCategory(c);
	}
	file.write(QJsonDocument(scheme.toJson()).toJson(QJsonDocument::Indented));
	return true;
}

void NewSchemeDialog::newCategory()
{
	auto c = new QTreeWidgetItem(ui->categoryList);
	c->setText(0, "New Category");
	c->setBackgroundColor(1, QColor(Qt::white));
	c->setFlags(c->flags() | Qt::ItemIsEditable);
	ui->categoryList->setCurrentItem(c);
	ui->categoryList->editItem(c, 0);
	ui->removeCategoryButton->setEnabled(true);
}

void NewSchemeDialog::removeCategory()
{
	qDeleteAll(ui->categoryList->selectedItems());
	ui->removeCategoryButton->setEnabled(ui->categoryList->invisibleRootItem()->childCount() != 0);
}

void NewSchemeDialog::categoryClicked(QTreeWidgetItem* item, int column)
{
	if (column != 1)
		return;
	QColorDialog dialog(this);
	dialog.setCurrentColor(item->backgroundColor(1));
	if (dialog.exec()) {
		item->setBackgroundColor(1, dialog.selectedColor());
	}
}

void NewSchemeDialog::browse()
{
	auto lastPath =  QSettings().value("LastNewSchemePath").toString();
	auto path = QFileDialog::getSaveFileName(this, "Choose Path", lastPath, "Scheme Files (*.json)");
	if (!path.isNull()) {
		ui->pathEdit->setText(path);
	}
}
