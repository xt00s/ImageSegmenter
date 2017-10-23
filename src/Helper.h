#ifndef QTHELP_H
#define QTHELP_H

#include <QIcon>

namespace help
{
	struct WaitCursor
	{
		WaitCursor();
		~WaitCursor();
	};

	inline QPointF floor(const QPointF& p) { return QPointF(::floor(p.x()), ::floor(p.y())); }

	QIcon emptyIcon(QSize size);
	QIcon colorIcon(QSize size, QColor color);
	QPixmap lightenPixmap(const QPixmap& pixmap, qreal factor);
	QString correctFileName(const QString& name);
	QString segmentationMaskFilePath(const QString& imagePath, const QString outputPath, const QString& scheme);
}

#endif // QTHELP_H
