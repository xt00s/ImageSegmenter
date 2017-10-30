#ifndef SCHEMETREE_H
#define SCHEMETREE_H

#include <QTreeWidget>
#include <QScopedPointer>
#include "Scheme.h"

class SchemeTree : public QTreeWidget
{
	Q_OBJECT
public:
	SchemeTree(QWidget *parent = 0);

	void setup();
	bool open(const QString& path);
	void clear();
	void resetCategoryStates();

	Category* currentCategory() const;
	Scheme* scheme() const;
	QString errorString() const;

signals:
	void schemeChanged(Scheme* scheme);
	void selected(Category* cat);
	void visibilityChanged(Category* cat);
	void lockedChanged(Category* cat);
	void clipChanged(Category* cat);

private slots:
	void itemClicked(QTreeWidgetItem *item, int column);

protected:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	void addCategory(Category* cat);
	Category* itemCategory(QTreeWidgetItem* item) const;

private:
	QScopedPointer<Scheme> scheme_;
	QScopedPointer<Category> eraserCategory_;
	QString errorString_;
	QPixmap statePixmaps_[4];
};

inline Scheme *SchemeTree::scheme() const { return scheme_.data(); }
inline QString SchemeTree::errorString() const { return errorString_; }

#endif // SCHEMETREE_H
