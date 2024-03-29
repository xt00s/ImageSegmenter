#ifndef POLYLINEITEM_H
#define POLYLINEITEM_H

#include <QGraphicsItem>
#include <QPen>

class Scheme;
class Category;
class Drawable;
class Selection;

class CanvasItem : public QGraphicsItem
{
public:
    CanvasItem(QGraphicsItem* parent = nullptr);

    QRect pixmapRect() const;
    QSize pixmapSize() const;
    QPixmap mask(const QColor& background = Qt::black) const;

    QPixmap pixmap() const;
    void setPixmap(const QPixmap& pixmap, const QPixmap& mask = QPixmap());

    const Category* category() const;
    void setCategory(const Category* cat);

    void setScheme(const Scheme* scheme);
    void setClipRegion(const Category* cat);

    bool clipRegionVisible() const;
    void setClipRegionVisible(bool visible);

    bool isPixmapVisible() const;
    void setPixmapVisible(bool visible);

    bool isMaskVisible() const;
    void setMaskVisible(bool visible);

    bool isPixmapGray() const;
    void setPixmapGray(bool gray);

    double pixmapOpacity() const;
    void setPixmapOpacity(double opacity);

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
        Fragment(const QRect& rect, const Category* category = nullptr) : rect_(rect), category_(category) {}
        friend class CanvasItem;
        QRect rect_;
        const Category* category_;
        QVector<QPixmap> layerFragements_;
    };

    Fragment* extractFragment(const QRect& rect, const Category* category = nullptr) const;
    void restoreFragment(const Fragment& fragment);

private:
    void initLayers(const QPixmap& mask);
    void drawColored(const Drawable& shape);
    void erase(const Drawable& shape);

private:
    QPixmap pixmap_;
    QPixmap pixmapG_;
    QPainterPath shape_;
    QVector<QPixmap> layers_;
    QRegion clipRegion_;
    QScopedPointer<Selection> clipSelection_;
    const Scheme* scheme_;
    const Category* category_;
    bool maskVisible_;
    bool pixmapVisible_;
    bool pixmapGray_;
    bool clipRegionVisible_;
    double pixmapOpacity_;
};

inline QRect CanvasItem::pixmapRect() const { return pixmap_.rect(); }
inline QSize CanvasItem::pixmapSize() const { return pixmap_.size(); }
inline QPixmap CanvasItem::pixmap() const { return pixmap_; }
inline const Category* CanvasItem::category() const { return category_; }
inline bool CanvasItem::clipRegionVisible() const { return clipRegionVisible_; }
inline bool CanvasItem::isPixmapGray() const { return pixmapGray_; }
inline bool CanvasItem::isPixmapVisible() const { return pixmapVisible_; }
inline bool CanvasItem::isMaskVisible() const { return maskVisible_; }
inline double CanvasItem::pixmapOpacity() const { return pixmapOpacity_; }

class OverlayItem : public QGraphicsItem
{
public:
    OverlayItem(QGraphicsItem* parent = nullptr);

    void setSize(const QSize& size);

    QPixmap pixmap() const;

    void beginDrawing();
    void endDrawing();
    void draw(const Drawable& shape, const QColor& color);
    void undo();
    void clear();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QPainterPath opaqueArea() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QPixmap pixmap_;
    QPixmap pixmapCopy_;
    QPainterPath shape_;
    QList<QPair<QPoint, QPixmap>> undoStack_;
    QRect drawingRect_;
};

inline QPixmap OverlayItem::pixmap() const { return pixmap_; }

class PolylineItem : public QGraphicsItem
{
public:
    PolylineItem(QGraphicsItem *parent = nullptr);

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
    OutlineItem(const QVector<QPointF>& outline, QGraphicsItem *parent = nullptr);
    OutlineItem(const QPolygonF& outline, QGraphicsItem *parent = nullptr);

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
    StartMarkerItem(QGraphicsItem *parent = nullptr);

    bool isHovering() const;
    void clear();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool hovering_;
};

inline bool StartMarkerItem::isHovering() const { return hovering_; }

class BrushCursorItem : public QGraphicsItem
{
public:
    BrushCursorItem(QGraphicsItem *parent = nullptr);

    qreal width() const;
    void setWidth(qreal width);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    qreal width_;
};

inline qreal BrushCursorItem::width() const { return width_; }

class GuideLineItem : public QGraphicsItem
{
public:
    GuideLineItem(QGraphicsItem *parent = nullptr);
    GuideLineItem(const QLineF& line, QGraphicsItem *parent = nullptr);

    QLineF line() const;
    void setLine(const QLineF& line);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QLineF line_;
    QPen pen_;
};

inline QLineF GuideLineItem::line() const { return line_; }

#endif // POLYLINEITEM_H
