#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <QRect>
#include <QPolygon>
#include <QLine>
#include <QImage>

class QPainter;
class QColor;
class QImage;

class Drawable
{
public:
    Drawable(const QRect& rect);
    virtual ~Drawable() {}
    QRect rect() const;
    virtual void draw(QPainter& painter, const QColor& color) const = 0;
protected:
    QRect rect_;
};

inline Drawable::Drawable(const QRect& rect) : rect_(rect) {}
inline QRect Drawable::rect() const { return rect_; }

class Polygon : public Drawable
{
public:
    Polygon(const QPolygonF& polygon);
    void draw(QPainter& painter, const QColor& color) const override;
private:
    QPolygonF polygon_;
};


class Line : public Drawable
{
public:
    Line(const QLineF& line, qreal width);
    void draw(QPainter &painter, const QColor &color) const override;
private:
    QLineF line_;
    qreal width_;
};


class Circle : public Drawable
{
public:
    Circle(const QPointF& center, qreal radius);
    void draw(QPainter& painter, const QColor& color) const override;
private:
    QPointF center_;
    qreal radius_;
};

class Bitmap : public Drawable
{
public:
    Bitmap(const QPoint& topLeft, const QImage& bmp);
    void draw(QPainter& painter, const QColor& color) const override;
private:
    QPoint topLeft_;
    mutable QImage bmp_;
};

#endif // DRAWABLE_H
