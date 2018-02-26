#ifndef EXAMPLELIST_H
#define EXAMPLELIST_H

#include <QTreeWidget>

class ImageList : public QTreeWidget
{
	Q_OBJECT
public:
	ImageList(QWidget *parent = 0);

	void setup();
	void open(const QString& path, const QString& outputPath, const QString &schemeName);
	void clear();

	bool isPrevAvailable() const;
	bool isNextAvailable() const;

	int count() const;
	int segmentedCount() const;

signals:
	void selected(const QString& examplePath);
	void progressChanged();

public slots:
	void next();
	void prev();

private slots:
	void itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
	bool segmented(QTreeWidgetItem* item);
	bool markedAsSegmented(QTreeWidgetItem* item);
	void update(int index);
	void select(int index);

	QTreeWidgetItem* item(int index) const;
	int row(QTreeWidgetItem* item) const;

private:
	QString outputPath_;
	QString schemeName_;
	int selected_;
	int segmented_;
	QFont boldFont_;
};

inline int ImageList::segmentedCount() const { return segmented_; }
inline int ImageList::count() const { return invisibleRootItem()->childCount(); }
inline QTreeWidgetItem*ImageList::item(int index) const { return invisibleRootItem()->child(index); }
inline int ImageList::row(QTreeWidgetItem* item) const { return invisibleRootItem()->indexOfChild(item); }

#endif // EXAMPLELIST_H
