#ifndef POLYLINEITEM_H
#define POLYLINEITEM_H

#include <QGraphicsItem>
#include <QPen>

class Scheme;
class Category;
class Drawable;

class CanvasItem : public QGraphicsItem
{
public:
	CanvasItem(QGraphicsItem* parent = 0);

	QRect pixmapRect() const;
	QSize pixmapSize() const;
	QPixmap mask(const QColor& background = Qt::black) const;

	void setPixmap(const QPixmap& pixmap, const QPixmap& mask = QPixmap());
	void setScheme(const Scheme* scheme);

	const Category* category() const;
	void setCategory(const Category* cat);

	bool isPixmapVisible() const;
	void setPixmapVisible(bool visible);

	bool isPixmapBright() const;
	void setPixmapBright(bool bright);

	void drawPolygon(const QPolygon& polygon);
	void drawLine(const QLineF& line, qreal width);
	void drawCircle(const QPointF& center, qreal radius);
	void draw(const Drawable& shape);

	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	QPainterPath opaqueArea() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

public:
	class Fragment
	{
	public:
		QRect rect() const { return rect_; }
		Fragment* extract(const QRect& rect) const;
	private:
		Fragment(const QRect& rect, const Category* category = 0) : rect_(rect), category_(category) {}
		friend class CanvasItem;
		QRect rect_;
		const Category* category_;
		QVector<QPixmap> layerFragements_;
	};

	Fragment* extractFragment(const QRect& rect, const Category* category = 0) const;
	void restoreFragment(const Fragment& fragment);

private:
	void initLayers(const QPixmap& mask);
	void drawColored(const Drawable& shape);
	void erase(const Drawable& shape);

private:
	QPixmap pixmap_;
	QPainterPath shape_;
	QVector<QPixmap> layers_;
	const Scheme* scheme_;
	const Category* category_;
	bool pixmapVisible_;
	bool pixmapBright_;
};

inline QRect CanvasItem::pixmapRect() const { return pixmap_.rect(); }
inline QSize CanvasItem::pixmapSize() const { return pixmap_.size(); }
inline const Category* CanvasItem::category() const { return category_; }
inline bool CanvasItem::isPixmapBright() const { return pixmapBright_; }
inline bool CanvasItem::isPixmapVisible() const { return pixmapVisible_; }

class PolylineItem : public QGraphicsItem
{
public:
	PolylineItem(QGraphicsItem *parent = 0);

	void begin();
	void setLastPoint(const QPointF& p);
	void commitPoint(const QPointF& p);
	void commit();
	void clear();
	bool canClose() const;
	QPolygonF polygon() const;

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
	QPen darkPen_;
	QPen lightPen_;
	QPolygonF polygon_;
};

inline QPolygonF PolylineItem::polygon() const { return polygon_; }

class StartMarkerItem : public QGraphicsItem
{
public:
	StartMarkerItem(QGraphicsItem *parent = 0);

	bool isHovering() const;
	void clear();

	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
	bool hovering_;
};

inline bool StartMarkerItem::isHovering() const { return hovering_; }

class BrushCursorItem : public QGraphicsItem
{
public:
	BrushCursorItem(QGraphicsItem *parent = 0);

	qreal width() const;
	void setWidth(qreal width);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
	qreal width_;
};

inline qreal BrushCursorItem::width() const { return width_; }

#endif // POLYLINEITEM_H
