#ifndef SELECTION_H
#define SELECTION_H

#include <QPropertyAnimation>
#include <QScopedPointer>
#include <QImage>

class QGraphicsScene;
class QGraphicsPixmapItem;
class OutlineItem;

class Selection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int outlineDashOffset READ outlineDashOffset  WRITE setOutlineDashOffset)
public:
    Selection(QGraphicsScene* scene, QObject *parent = nullptr);
    Selection(const QImage& bmp, bool highlightVisible, QGraphicsScene* scene, QObject *parent = nullptr);
    ~Selection() override;

    void setArea(const QImage& bmp);

    QImage filledArea() const;
    QRect filledAreaRect() const;

    bool visible() const;
    void setVisible(bool visible);

    int outlineDashOffset() const;
    void setOutlineDashOffset(int outlineDashOffset);

    bool highlightVisible() const;
    void setHighlightVisible(bool visible);

    bool empty() const;

private:
    void initHighlight();
    void initOutlines();
    OutlineItem* outlineFromBoundary(const QVector<QPoint>& boundary, QImage& map);

private:
    bool visible_;
    bool highlightVisible_;
    int outlineDashOffset_;
    QGraphicsScene* scene_;
    QImage bitmap_;
    mutable QRect filledAreaRect_;
    QScopedPointer<QGraphicsPixmapItem> highlight_;
    QList<OutlineItem*> outlines_;
    QPropertyAnimation outlineAnimation_;
};

inline bool Selection::visible() const { return visible_; }
inline int Selection::outlineDashOffset() const { return outlineDashOffset_; }
inline bool Selection::highlightVisible() const { return highlightVisible_; }
inline bool Selection::empty() const { return outlines_.empty(); }

#endif // SELECTION_H
