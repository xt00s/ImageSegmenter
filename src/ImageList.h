#ifndef EXAMPLELIST_H
#define EXAMPLELIST_H

#include <QListWidget>

class ImageList : public QListWidget
{
	Q_OBJECT
public:
	ImageList(QWidget *parent = 0);

	void open(const QString& path, const QString& outputPath, const QString &schemeName);
	void clear();

	bool isPrevAvailable() const;
	bool isNextAvailable() const;

	int segmentedCount() const;

signals:
	void selected(const QString& examplePath);
	void progressChanged();

public slots:
	void next();
	void prev();

private slots:
	void itemDoubleClicked(QListWidgetItem *item);

private:
	bool segmented(QListWidgetItem* item);
	bool markedAsSegmented(QListWidgetItem* item);
	void update(int index);
	void select(int index);

private:
	QString outputPath_;
	QString schemeName_;
	int selected_;
	int segmented_;
	QIcon emptyIcon_;
	QIcon selectedIcon_;
	QFont boldFont_;
};

inline int ImageList::segmentedCount() const { return segmented_; }

#endif // EXAMPLELIST_H
