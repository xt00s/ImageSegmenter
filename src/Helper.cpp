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

	QPixmap colorPixmap(QSize size, QColor color)
	{
		QPixmap pixmap(size);
		pixmap.fill(Qt::transparent);
		QPainter p(&pixmap);
		p.setPen(QColor(color).darker(200));
		p.setBrush(color);
		p.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));
		return pixmap;
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

	QPixmap rgb2gray(const QPixmap& pixmap)
	{
		if (pixmap.isNull())
			return QPixmap();

		auto image = pixmap.toImage();
		auto w = image.width(), h = image.height();
		if (image.format() == QImage::Format_RGB32 ||
			image.format() == QImage::Format_ARGB32 ||
			image.format() == QImage::Format_ARGB32_Premultiplied) {
			for (int i = 0; i < h; i++) {
				auto r = (uint*)image.scanLine(i), end = r + w;
				while (r < end) {
					auto gray = qGray(*r);
					*r++ = qRgba(gray, gray, gray, qAlpha(*r));
				}
			}
		} else {
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					auto p = image.pixel(i, j);
					auto gray = qGray(p);
					image.setPixel(i, j, qRgba(gray, gray, gray, qAlpha(p)));
				}
			}
		}
		return QPixmap::fromImage(image);
	}

}
