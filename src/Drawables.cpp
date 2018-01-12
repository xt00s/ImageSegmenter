#include "Drawables.h"
#include "Helper.h"
#include <QPainter>

Polygon::Polygon(const QPolygon& polygon)
	: Drawable(polygon.boundingRect()), polygon_(polygon)
{}

void Polygon::draw(QPainter& painter, const QColor& color) const
{
	painter.setPen(Qt::NoPen);
	painter.setBrush(color);
	painter.drawPolygon(polygon_);
}

//-----------------------------------------------------------------------------------

Line::Line(const QLineF& line, qreal width)
	: Drawable(QRect(help::floor(line.p1()).toPoint(), help::floor(line.p2()).toPoint()).normalized())
	, line_(line), width_(width)
{
	auto m = qRound(width/2) + 1;
	rect_.adjust(-m,-m,m,m);
}

void Line::draw(QPainter& painter, const QColor& color) const
{
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(color, width_, Qt::SolidLine, Qt::RoundCap));
	painter.drawLine(line_);
}

//-----------------------------------------------------------------------------------

Circle::Circle(const QPointF& c, qreal r)
	: Drawable(QRect(help::floor(c - QPointF(r, r)).toPoint(), help::floor(c + QPointF(r, r)).toPoint()))
	, center_(c), radius_(r)
{
	if (r == .5) {
		qreal p;
		if (modf(center_.x(), &p) == .5 && modf(center_.y(), &p) == .5) {
			center_.setX(center_.x() + .1);
		}
	}
}

void Circle::draw(QPainter& painter, const QColor& color) const
{
	painter.setPen(Qt::NoPen);
	painter.setBrush(color);
	painter.drawEllipse(center_, radius_, radius_);
}

//-----------------------------------------------------------------------------------

Bitmap::Bitmap(const QPoint& topLeft, const QImage& bmp)
	: Drawable(QRect(topLeft, bmp.size()))
	, topLeft_(topLeft), bmp_(bmp)
{}

void Bitmap::draw(QPainter& painter, const QColor& color) const
{
	bmp_.setColor(0, QColor(Qt::transparent).rgba());
	if (color == Qt::transparent && painter.compositionMode() == QPainter::CompositionMode_Source) {
		bmp_.setColor(1, QColor(Qt::white).rgba());
		painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
		painter.drawImage(topLeft_, bmp_);
		painter.setCompositionMode(QPainter::CompositionMode_Source);
	} else {
		bmp_.setColor(1, color.rgba());
		painter.drawImage(topLeft_, bmp_);
	}
}
