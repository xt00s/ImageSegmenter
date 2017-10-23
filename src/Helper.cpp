#include "Helper.h"
#include <QApplication>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

namespace help
{
	WaitCursor::WaitCursor()
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
	}

	WaitCursor::~WaitCursor()
	{
		QApplication::restoreOverrideCursor();
	}

	QIcon emptyIcon(QSize size)
	{
		QPixmap empty(size);
		empty.fill(Qt::transparent);
		return QIcon(empty);
	}

	QIcon colorIcon(QSize size, QColor color)
	{
		QPixmap image(size);
		image.fill(Qt::transparent);
		QPainter p(&image);
		p.setPen(QColor(color).darker(200));
		p.setBrush(color);
		p.drawRect(image.rect().adjusted(0, 0, -1, -1));
		return QIcon(image);
	}

	QPixmap lightenPixmap(const QPixmap& pixmap, qreal factor)
	{
		QPixmap dark(pixmap.size());
		dark.fill(Qt::transparent);
		QPainter p(&dark);
		p.setBackgroundMode(Qt::TransparentMode);
		p.setBackground(QBrush(Qt::transparent));
		p.eraseRect(pixmap.rect());
		p.setOpacity(factor);
		p.drawPixmap(0, 0, pixmap);
		return dark;
	}

	QString correctFileName(const QString &name)
	{
		static QRegularExpression re(R"([<>:"/\|])");
		return QString(name).remove(re);
	}

	QString segmentationMaskFilePath(const QString &imagePath, const QString outputPath, const QString &scheme)
	{
		auto corrected = correctFileName(scheme);
		if (corrected.isEmpty()) {
			return QString();
		}
		QFileInfo info(imagePath);
		return QFileInfo(outputPath, info.baseName() + "_" + corrected + ".png").filePath();
	}
}
