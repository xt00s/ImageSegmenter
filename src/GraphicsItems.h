#ifndef POLYLINEITEM_H
#define POLYLINEITEM_H

#include <QGraphicsItem>
#include <QPen>
#include <QPropertyAnimation>

class Scheme;
class Category;
class Drawable;
class OutlineItem;

class CanvasItem : public QGraphicsObject
{
	Q_OBJECT
	Q_PROPERTY(int outlineDashOffset READ outlineDashOffset  WRITE setOutlineDashOffset)
public:
	CanvasItem(QGraphicsItem* parent = 0);

	QRect pixmapRect() const;
	QSize pixmapSize() const;
	QPixmap mask(const QColor& background = Qt::black) const;

	void setPixmap(const QPixmap& pixmap, const QPixmap& mask = QPixmap());
	void setScheme(const Scheme* scheme);

	const Category* category() const;
	void setCategory(const Category* cat);

	void setClipRegion(const Category* cat);

	bool isPixmapVisible() const;
	void setPixmapVisible(bool visible);

	bool isMaskVisible() const;
	void setMaskVisible(bool visible);

	bool isPixmapGray() const;
	void setPixmapGray(bool gray);

	double pixmapOpacity() const;
	void setPixmapOpacity(double opacity);

	int outlineDashOffset() const;
	void setOutlineDashOffset(int outlineDashOffset);

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
	void addOutline(QImage& bmp, const QVector<QPoint>& boundary);

private:
	QPixmap pixmap_;
	QPixmap pixmapG_;
	QPainterPath shape_;
	QVector<QPixmap> layers_;
	QRegion clipRegion_;
	const Scheme* scheme_;
	const Category* category_;
	bool maskVisible_;
	bool pixmapVisible_;
	bool pixmapGray_;
	double pixmapOpacity_;
	int outlineDashOffset_;
	QList<OutlineItem*> outlines_;
	QPropertyAnimation* outlineAnimation_;
};

inline QRect CanvasItem::pixmapRect() const { return pixmap_.rect(); }
inline QSize CanvasItem::pixmapSize() const { return pixmap_.size(); }
inline const Category* CanvasItem::category() const { return category_; }
inline bool CanvasItem::isPixmapGray() const { return pixmapGray_; }
inline bool CanvasItem::isPixmapVisible() const { return pixmapVisible_; }
inline bool CanvasItem::isMaskVisible() const { return maskVisible_; }
inline double CanvasItem::pixmapOpacity() const { return pixmapOpacity_; }
inline int CanvasItem::outlineDashOffset() const { return outlineDashOffset_; }

class PolylineItem : public QGraphicsItem
{
public:
	PolylineItem(QGraphicsItem *parent = 0);

	void begin();
	void setLastPoint(const QPointF& p);
	void removeLastCommittedPoint();
	void commitPoint(const QPointF& p);
	void commit();
	void clear();
	bool canClose() const;
	bool empty() const;
	QPolygonF polygon() const;

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
	QPen darkPen_;
	QPen lightPen_;
	QPolygonF polygon_;
};

inline QPolygonF PolylineItem::polygon() const { return polygon_; }

class OutlineItem : public QGraphicsItem
{
public:
	OutlineItem(const QVector<QPointF>& outline, QGraphicsItem *parent = 0);
	OutlineItem(const QPolygonF& outline, QGraphicsItem *parent = 0);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	int dashOffset() const;
	void setDashOffset(int offset);

private:
	QPen darkPen_;
	QPen lightPen_;
	QPolygonF outline_;
	int dashOffset_;
};

inline int OutlineItem::dashOffset() const { return dashOffset_; }

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
