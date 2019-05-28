#ifndef QTHELP_H
#define QTHELP_H

#include <QIcon>
#include <functional>

namespace help
{
    struct WaitCursor
    {
        WaitCursor();
        ~WaitCursor();
    };

    struct DefaultCursor
    {
        DefaultCursor();
        ~DefaultCursor();
    };

    inline QPointF floor(const QPointF& p) { return QPointF(::floor(p.x()), ::floor(p.y())); }

    inline uchar qLsbBit(const uchar* line, int x) { return (*(line + x/8) >> (x & 7)) & 1; }
    inline void qLsbSet(uchar* line, int x) { *(line + x/8) |= (uchar(1) << (x & 7)); }

    QPixmap colorPixmap(QSize size, QColor color);
    QPixmap lightenPixmap(const QPixmap& pixmap, qreal factor);
    QString correctFileName(const QString& name);
    QString segmentationMaskFilePath(const QString& imagePath, const QString outputPath, const QString& scheme);

    QPixmap rgb2gray(const QPixmap& pixmap);

    QVector<QPoint> traceBoundary(const QImage& bmp, const QPoint& start, const QPoint& backtrace, bool mooreNeighborhood = true, bool invertColors = false, bool close = true);
    QVector<QPoint> outlineFromBoundary(const QVector<QPoint>& boundary);

    QImage flood(const QImage& src, const QPoint& start, const std::function<bool(QRgb)>& test);
    QRect filledRect(const QImage& bmp);

    QImage segmentIGC(const QImage& src, const QImage& seedMask, const QColor& fColor, const QColor& bColor, double sigma);

    QImage diskBitmap(int radius);
    QImage imerode(const QImage& src, const QImage& kernel);
    QImage imdilate(const QImage& src, const QImage& kernel);
    QImage imopen(const QImage& src, const QImage& kernel);
    QImage imclose(const QImage& src, const QImage& kernel);

    int scanBitForward(quint64 v);
    int scanBitReverse(quint64 v);

    std::function<QRgb(const uchar*,int,int)> pixelReader(const QImage& image);
}

#endif // QTHELP_H
