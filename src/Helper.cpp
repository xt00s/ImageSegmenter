#include <memory>

#include "Helper.h"
#include <QApplication>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include "graph.h"
#include <memory>

using namespace std;

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

    DefaultCursor::DefaultCursor()
    {
        QApplication::setOverrideCursor(QCursor());
    }

    DefaultCursor::~DefaultCursor()
    {
        QApplication::restoreOverrideCursor();
    }

    QPixmap colorPixmap(QSize size, const QColor& color)
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
        p.setOpacity(factor);
        p.drawPixmap(0, 0, pixmap);
        return dark;
    }

    QString correctFileName(const QString &name)
    {
        static QRegularExpression re(R"([<>:"/\|])");
        return QString(name).remove(re);
    }

    QString segmentationMaskFilePath(const QString &imagePath, const QString& outputPath, const QString &scheme)
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

    QVector<QPoint> traceBoundary(const QImage& bmp, const QPoint& start, const QPoint& backtrace, bool mooreNeighborhood, bool invertColors, bool close)
    {
        uchar color = invertColors ? 0 : 1;
        if (bmp.format() != QImage::Format_MonoLSB
                || !bmp.valid(start)
                || qLsbBit(bmp.scanLine(start.y()), start.x()) != color
                || (bmp.valid(backtrace) && qLsbBit(bmp.scanLine(backtrace.y()), backtrace.x()) == color)) {
            return QVector<QPoint>();
        }
        static QVector<QPoint> nhood8 = { {-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0} };
        static QVector<QPoint> nhood4 = { {-1,0},{0,-1},{1,0},{0,1} };
        auto& nhood = mooreNeighborhood ? nhood8 : nhood4;
        int dir = nhood.indexOf(backtrace - start);
        if (dir < 0) {
            return QVector<QPoint>();
        }
        QVector<QPoint> boundary(1, start);
        QPoint p = start;
        auto sz = bmp.size();
        auto bpl = bmp.bytesPerLine(), H = nhood.size();
        auto sl = bmp.scanLine(p.y());
        const uchar* r[3] = { sl - bpl, sl, sl + bpl };

        int n1 = -1;
        for (int i = 0; i < H; i++) {
            auto k = (dir - i + H - 1) % H;
            auto c = p + nhood[k];
            if (bmp.valid(c) && qLsbBit(r[nhood[k].y()+1], c.x()) == color) {
                n1 = k;
                break;
            }
        }
        if (n1 >= 0) {
            n1 = (n1 + H / 2) % H;
            for (int n = (dir + 1) % H;;) {
                bool inside = (p.x()-1) >= 0 && (p.y()-1) >= 0 && (p.x()+1) < sz.width() && (p.y()+1) < sz.height();
                if (inside) {
                    for (int i = 0; i < H; i++, n = ++n % H) {
                        auto x = p.x() + nhood[n].x();
                        if (qLsbBit(r[nhood[n].y()+1], x) == color) {
                            break;
                        }
                    }
                } else {
                    for (int i = 0; i < H; i++, n = ++n % H) {
                        auto c = p + nhood[n];
                        if (c.x() < 0 || c.y() < 0 || c.x() >= sz.width() || c.y() >= sz.height()) {
                            continue;
                        }
                        if (qLsbBit(r[nhood[n].y()+1], c.x()) == color) {
                            break;
                        }
                    }
                }
                p += nhood[n];
                if (start == p && n == n1) {
                    break;
                }
                boundary << p;
                auto roff = nhood[n].y() * bpl;
                r[0] += roff;
                r[1] += roff;
                r[2] += roff;
                n = mooreNeighborhood ? (n + (n & 1) + 6) % H : (n + 3) % H;
            }
        }
        if (close) {
            boundary << start;
        }
        return qMove(boundary);
    }

    QVector<QPoint> outlineFromBoundary(const QVector<QPoint>& B)
    {
        if (B.count() < 2 || B.first() != B.last()) {
            return QVector<QPoint>();
        }
        if (B.count() == 2) {
            auto p = B.first();
            return QVector<QPoint>() << p << p + QPoint(1,0) << p + QPoint(1,1) << p + QPoint(0,1) << p;
        }

        static int n2i[3][3] = {{0,7,6},{1,0,5},{2,3,4}};
        static int jumps[8] = {3,0,0,1,1,2,2,3};
        static QPoint corners[4] = {{0,0},{1,0},{1,1},{0,1}};
        static QPoint cSteps[4] = {{1,0},{0,1},{-1,0},{0,-1}};
        static QPoint steps[8][4] {
            {{-1,0},{0, 1},{-1,0},{0,-1}},
            {{0,-1},{0, 1},{-1,0},{0,-1}},
            {{1, 0},{0,-1},{-1,0},{0,-1}},
            {{1, 0},{1, 0},{-1,0},{0,-1}},
            {{1, 0},{0, 1},{1, 0},{0,-1}},
            {{1, 0},{0, 1},{0, 1},{0,-1}},
            {{1, 0},{0, 1},{-1,0},{0, 1}},
            {{1, 0},{0, 1},{-1,0},{-1,0}},
        };

        const QPoint p11(1,1);
        const int diag[] = {136,34};

        auto off = B[1] - B[0] + p11;
        auto c = n2i[off.x()][off.y()]/2, start = c;
        QVector<QPoint> L(1, B.first() + corners[c]);

        for (int i = 1; i < B.count(); i++) {
            off = B[i] - B[i-1] + p11;
            int n = n2i[off.x()][off.y()];
            if (n & 1 && i+1 < B.count() && B[i+1] != B[i-1]) { // go to diagonal pixel in case of 4-way boundary
                auto off2 = B[i+1] - B[i] + p11;
                int n2 = n2i[off2.x()][off2.y()];
                if (diag[(n/2) & 1] & (1<<n2) && n2+1 == n) {
                    n = n2;
                    i++;
                }
            }
            int stepCount = (n/2 - c + 4) % 4 + 1;
            for (int j = 0; j < stepCount; j++, c = ++c % 4) {
                L << L.last() + steps[n][c];
            }
            c = jumps[n];
        }

        int stepCount = (start - c + 4) % 4;
        for (int i = 0; i < stepCount; i++, c = ++c % 4) {
            L << L.last() + cSteps[c];
        }

        return qMove(L);
    }

    QImage flood(const QImage& src, const QPoint& start, const function<bool(QRgb)>& test)
    {
        if (!src.valid(start)) {
            return QImage();
        }
        QSize sz = src.size();
        QImage bmp(sz, QImage::Format_MonoLSB);
        bmp.fill(0);

        struct Range {
            int x;
            int y;
            int maxX;
            int dir;
        };
        QVector<Range> ranges(1, {start.x(), start.y(), sz.width(), 0});

        auto getPixel = pixelReader(src);

        while (!ranges.empty()) {
            auto r = ranges.takeFirst();
            auto br = bmp.scanLine(r.y);
            auto sr = src.scanLine(r.y);
            int s = r.x, e;
            for (;;) {
                while (s < r.maxX && !test(getPixel(sr, s, r.y))) {
                    s++;
                }
                if (s >= r.maxX) {
                    break;
                }
                e = s+1;
                if (qLsbBit(br, s)) {
                    while (e < sz.width() && qLsbBit(br, e)) {
                        e++;
                    }
                } else {
                    if (s == r.x) {
                        while (s >= 0 && test(getPixel(sr, s, r.y))) {
                            qLsbSet(br, s);
                            s--;
                        }
                        s++;
                    } else {
                        qLsbSet(br, s);
                    }
                    while (e < sz.width() && test(getPixel(sr, e, r.y))) {
                        qLsbSet(br, e);
                        e++;
                    }
                    if (!r.dir) {
                        if (r.y > 0) {
                            ranges.append({s, r.y-1, e, 1});
                        }
                        if (r.y < sz.height()-1) {
                            ranges.append({s, r.y+1, e, -1});
                        }
                        break;
                    }
                    if (r.x-s > 1) {
                        ranges.append({s, r.y + r.dir, r.x-1, r.dir * -1});
                    }
                    if (e-r.maxX > 1) {
                        ranges.append({r.maxX+1, r.y + r.dir, e, r.dir * -1});
                    }
                    int newY = r.y - r.dir;
                    if (newY >= 0 && newY < sz.height()) {
                        ranges.append({s, r.y - r.dir, e, r.dir});
                    }
                }
                s = e+1;
            }
        }
        return bmp;
    }

    QRect filledRect(const QImage& bmp)
    {
        if (bmp.format() != QImage::Format_MonoLSB) {
            return {};
        }
        const auto bitsInBlock = sizeof(quint64) * 8;
        auto sz = bmp.size();
        int blocks = sz.width() / bitsInBlock;
        int l = sz.width(), r = -1, t = -1, b = -1;
        quint64 lastBlockMask = (quint64(1) << (sz.width() % bitsInBlock)) - 1;

        for (int y = 0, i; y < sz.height(); y++) {
            auto sl = reinterpret_cast<const quint64*>(bmp.scanLine(y));
            for (i = 0; i < blocks && !sl[i]; i++);

            auto j = scanBitForward(sl[i] & (i < blocks ? quint64(-1) : lastBlockMask));
            if (j < 0) {
                continue;
            }
            int newL = i * bitsInBlock + j;
            if (newL < l) {
                l = newL;
                if (t < 0) {
                    t = y;
                }
            }
            j = scanBitReverse(sl[i = blocks] & lastBlockMask);
                               if (j < 0) {
                for (i = blocks-1; !sl[i]; i--);
                j = scanBitReverse(sl[i]);
            }
            int newR = i * bitsInBlock + j;
                    if (newR > r) {
                    r = newR;
        }
                    if (y > b) {
                    b = y;
        }
        }
        return l < sz.width() ? QRect(QPoint(l,t), QPoint(r,b)) : QRect();
    }

    using GraphD = Graph<double,double,double>;

    QImage segmentIGC(const QImage& src, const QImage& seedMask, const QColor& fColor, const QColor& bColor, double sigma)
    {
        auto srcPixel = help::pixelReader(src), seedPixel = help::pixelReader(seedMask);
        auto frgb = fColor.rgb(), brgb = bColor.rgb();
        auto s2 = -2 * sigma * sigma;
        auto weight = [s2](QRgb c1, QRgb c2) {
            auto dr = qRed(c1)-qRed(c2);
            auto dg = qGreen(c1)-qGreen(c2);
            auto db = qBlue(c1)-qBlue(c2);
            return exp(double(dr * dr + dg * dg + db * db) / s2);
        };
        auto S = src.size();
        auto nodes = S.width() * S.height();
        GraphD graph(nodes, nodes * 2);

        graph.add_node(nodes);
        for (int y = 0, i = 0; y < S.height(); y++) {
            auto seedSL = seedMask.scanLine(y);
            auto srcSL = src.scanLine(y), srcSL2 = y+1 < S.height() ? src.scanLine(y+1) : nullptr;
            for (int x = 0; x < S.width(); x++, i++) {
                auto seedRgb = seedPixel(seedSL, x, y);
                auto srcRgb = srcPixel(srcSL,x, y);

                if (seedRgb == frgb) {
                    graph.add_tweights(i, 5, 0);
                } else if (seedRgb == brgb) {
                    graph.add_tweights(i, 0, 5);
                } else {
                    graph.add_tweights(i, 0, 0);
                }
                if (x+1 < S.width()) {
                    auto w  = weight(srcRgb, srcPixel(srcSL,x+1,y));
                    graph.add_edge(i, i+1, w, w);
                }
                if (y+1 < S.height()) {
                    auto w  = weight(srcRgb, srcPixel(srcSL2,x,y+1));
                    graph.add_edge(i, i+S.width(), w, w);
                }
            }
        }
        graph.maxflow();

        QImage bmp(S, QImage::Format_MonoLSB);
        bmp.fill(0);
        for (int y = 0, i = 0; y < S.height(); y++) {
            auto sl = bmp.scanLine(y);
            for (int x = 0; x < S.width(); x++, i++) {
                if (graph.what_segment(i) == GraphD::SOURCE) {
                    qLsbSet(sl, x);
                }
            }
        }
        return bmp;
    }

    QImage diskBitmap(int radius)
    {
        QImage bmp(radius * 2 + 1, radius * 2 + 1, QImage::Format_MonoLSB);
        bmp.fill(0);
        auto r2 = radius * radius;
        for (int y = -radius; y <= radius; y++) {
            auto sl = bmp.scanLine(y + radius);
            auto y2 = y * y;
            for (int x = -radius; x <= radius; x++) {
                if ((x * x + y2) <= r2) {
                    qLsbSet(sl, x + radius);
                }
            }
        }
        return bmp;
    }

    QImage boundKernel(const QImage& src, const QImage& kernel)
    {
        auto r = kernel.rect();
        auto cx = r.width() / 2, cy = r.height() / 2;
        r.setLeft(r.left() + (cx - src.width() + 1));
        r.setRight(r.right() - (cx - src.width() + kernel.width() % 2));
        r.setTop(r.top() + (cy - src.height() + 1));
        r.setBottom(r.bottom() - (cy - src.height() + kernel.height() % 2));
        auto K = r.intersected(kernel.rect());
        if (K == kernel.rect()) {
            return kernel;
        }
        return kernel.copy(K);
    }

    struct KernelOffsets
    {
        unique_ptr<quint64[]> kernelData;
        quint64* kernel[64];
        int start[64];
        int count[64];
        int bpl;
    };

    KernelOffsets generateKernelOffsets(const QImage& K)
    {
        KernelOffsets KO{};
        KO.bpl = (62 + K.width()) / 64 + 1;
        KO.kernelData = make_unique<quint64[]>(KO.bpl * K.height() * 64);
        auto off = (64 * (K.width() / 64 + 1) - K.width() / 2) % 64, coff = 64 - off;

        for (int i = 0; i < 64; i++) {
            auto d = K.width() / 2 - i;
            KO.start[i] = -(d > 0 ? (d - 1) / 64 + 1 : d / 64);
            KO.count[i] = (off + K.width() - 1) / 64 + 1;
            KO.kernel[i] = KO.kernelData.get() + KO.bpl * K.height() * i;
            auto M = (quint64(1) << ((K.width() + off) % 64)) - 1;
            if (!M) {
                M = -1;
            }
            for (int y = 0; y < K.height(); y++) {
                auto sl = reinterpret_cast<const quint64*>(K.scanLine(y));
                auto p = KO.kernel[i] + KO.bpl * y;

                *p++ = *sl++ << off;
                for (int x = 1; x < KO.count[i]; x++, sl++) {
                    *p++ = (*sl << off) | (*(sl-1) >> coff);
                }
                *(p-1) &= M;
            }
            off = (off + 1) % 64;
            coff = 64 - off;
        }
        return KO;
    }

    QImage imerode(const QImage& src, const QImage& K, const KernelOffsets& KO)
    {
        QImage bmp(src.size(), QImage::Format_MonoLSB);
        bmp.fill(0);

        const auto cx = K.width() / 2, cy = K.height() / 2, bpl = src.bytesPerLine();
        const auto C = K.width() - cx - 1, L = (src.width() - 1) / 64 + 1, E = cx - 1, F = L - 1;
        const auto A = qMin(cx, src.width() - C), B = qMax(cx, src.width() - C);
        const auto M = src.width() % 64 ? ~((quint64(1) << (src.width() % 64)) - 1) : 0;
        const auto both = (A != cx);

        auto overlap = [bpl, &KO](const quint64* src, const quint64* kernel, int h, int w, quint64 m) -> quint64 {
            for (int y = 0; y < h; y++) {
                auto p = src, k = kernel;
                for (int x = 0; x < (w-1); x++, p++, k++) {
                    if ((*p & *k) != *k) {
                        return 0;
                    }
                }
                if (((*p | m) & *k) != *k) {
                    return 0;
                }
                src = reinterpret_cast<const quint64*>(reinterpret_cast<const uchar*>(src) + bpl);
                kernel += KO.bpl;
            }
            return 1;
        };

        for (int y = 0, x; y < src.height(); y++) {
            auto kTop = qMax(cy - y, 0), kHeight = qMin(cy - y + src.height(), K.height()) - kTop;
            auto kTopOff = kTop * KO.bpl;
            auto bmpSL = reinterpret_cast<quint64*>(bmp.scanLine(y));
            auto srcSL = reinterpret_cast<const quint64*>(src.scanLine(y) - (cy - kTop) * bpl);

            for (x = 0; x < A; x++) {
                auto block = x / 64, bit = x % 64, loff = (E - x) / 64 + 1;
                auto sp = srcSL + block + KO.start[bit] + loff;
                auto kp = KO.kernel[bit] + kTopOff + loff;
                auto m = block + KO.start[bit] + KO.count[bit] == L ? M : 0;
                auto b = overlap(sp, kp, kHeight, KO.count[bit] - loff, m);
                *(bmpSL + block) |= (b << bit);
            }
            if (both) {
                for (; x < B; x++) {
                    auto block = x / 64, bit = x % 64, loff = (E - x) / 64 + 1, roff = (x + C) / 64 - F;
                    auto sp = srcSL + block + KO.start[bit] + loff;
                    auto kp = KO.kernel[bit] + kTopOff + loff;
                    auto m = block + KO.start[bit] + KO.count[bit] == L ? M : 0;
                    auto b = overlap(sp, kp, kHeight, KO.count[bit] - loff - roff, m);
                    *(bmpSL + block) |= (b << bit);
                }
            } else {
                for (; x < B; x++) {
                    auto block = x / 64, bit = x % 64;
                    auto m = block + KO.start[bit] + KO.count[bit] == L ? M : 0;
                    auto b = overlap(srcSL + block + KO.start[bit], KO.kernel[bit] + kTopOff, kHeight, KO.count[bit], m);
                    *(bmpSL + block) |= (b << bit);
                }
            }
            for (; x < src.width(); x++) {
                auto block = x / 64, bit = x % 64, roff = (x + C) / 64 - F;
                auto b = overlap(srcSL + block + KO.start[bit], KO.kernel[bit] + kTopOff, kHeight, KO.count[bit] - roff, M);
                *(bmpSL + block) |= (b << bit);
            }
        }
        return bmp;
    }

    QImage imdilate(const QImage& src, const QImage& K, const KernelOffsets& KO)
    {
        QImage bmp(src.size(), QImage::Format_MonoLSB);
        bmp.fill(0);

        const auto cx = K.width() / 2, cy = K.height() / 2, bpl = src.bytesPerLine();
        const auto C = K.width() - cx - 1, L = (src.width() - 1) / 64 + 1, E = cx - 1, F = L - 1;
        const auto A = qMin(cx, src.width() - C), B = qMax(cx, src.width() - C);
        const auto M = src.width() % 64 ? ((quint64(1) << (src.width() % 64)) - 1) : -1;
        const auto both = (A != cx);

        auto unite = [bpl, &KO](quint64* bmp, const quint64* kernel, int h, int w, quint64 m) {
            for (int y = 0; y < h; y++) {
                auto p = bmp;
                auto k = kernel;
                for (int x = 0; x < (w-1); x++, p++, k++) {
                    *p |= *k;
                }
                *p |= (*k & m);
                bmp = reinterpret_cast<quint64*>(reinterpret_cast<uchar*>(bmp) + bpl);
                kernel += KO.bpl;
            }
        };

        for (int y = 0, x; y < src.height(); y++) {
            auto kTop = qMax(cy - y, 0), kHeight = qMin(cy - y + src.height(), K.height()) - kTop;
            auto kTopOff = kTop * KO.bpl;
            auto srcSL = src.scanLine(y);
            auto bmpSL = reinterpret_cast<quint64*>(bmp.scanLine(y) - (cy - kTop) * bpl);

            for (x = 0; x < A; x++) {
                if (qLsbBit(srcSL, x)) {
                    auto block = x / 64, bit = x % 64, loff = (E - x) / 64 + 1;
                    auto bp = bmpSL + block + KO.start[bit] + loff;
                    auto kp = KO.kernel[bit] + kTopOff + loff;
                    auto m = block + KO.start[bit] + KO.count[bit] == L ? M : -1;
                    unite(bp, kp, kHeight, KO.count[bit] - loff, m);
                }
            }
            if (both) {
                for (; x < B; x++) {
                    if (qLsbBit(srcSL, x)) {
                        auto block = x / 64, bit = x % 64, loff = (E - x) / 64 + 1, roff = (x + C) / 64 - F;
                        auto bp = bmpSL + block + KO.start[bit] + loff;
                        auto kp = KO.kernel[bit] + kTopOff + loff;
                        auto m = block + KO.start[bit] + KO.count[bit] == L ? M : -1;
                        unite(bp, kp, kHeight, KO.count[bit] - loff - roff, m);
                    }
                }
            } else {
                for (; x < B; x++) {
                    if (qLsbBit(srcSL, x)) {
                        auto block = x / 64, bit = x % 64;
                        auto m = block + KO.start[bit] + KO.count[bit] == L ? M : -1;
                        unite(bmpSL + block + KO.start[bit], KO.kernel[bit] + kTopOff, kHeight, KO.count[bit], m);
                    }
                }
            }
            for (; x < src.width(); x++) {
                if (qLsbBit(srcSL, x)) {
                    auto block = x / 64, bit = x % 64, roff = (x + C) / 64 - F;
                    unite(bmpSL + block + KO.start[bit], KO.kernel[bit] + kTopOff, kHeight, KO.count[bit] - roff, M);
                }
            }
        }
        return bmp;
    }

    template <class F>
    QImage immorth(const QImage& src, const QImage& kernel, const F& f)
    {
        if (src.format() != QImage::Format_MonoLSB || kernel.format() != QImage::Format_MonoLSB ||
                src.isNull() || kernel.isNull()) {
            return QImage();
        }
        auto K = boundKernel(src, kernel);
        return f(src, K, generateKernelOffsets(K));
    }

    QImage imerode(const QImage& src, const QImage& kernel)
    {
        return immorth(src, kernel, [](const QImage& src, const QImage& K, const KernelOffsets& KO) {
            return imerode(src, K, KO);
        });
    }

    QImage imdilate(const QImage& src, const QImage& kernel)
    {
        return immorth(src, kernel, [](const QImage& src, const QImage& K, const KernelOffsets& KO) {
            return imdilate(src, K, KO);
        });
    }

    QImage imopen(const QImage& src, const QImage& kernel)
    {
        return immorth(src, kernel, [](const QImage& src, const QImage& K, const KernelOffsets& KO) {
            return imdilate(imerode(src, K, KO), K, KO);
        });
    }

    QImage imclose(const QImage& src, const QImage& kernel)
    {
        return immorth(src, kernel, [](const QImage& src, const QImage& K, const KernelOffsets& KO) {
            return imerode(imdilate(src, K, KO), K, KO);
        });
    }

    int scanBitForward(quint64 v)
    {
        if (!v) {
            return -1;
        }
#if _MSC_VER
        ulong i;
        _BitScanForward64(&i, v);
        return i;
#elif __GNUC__
        return __builtin_ctzll(v);
#else
        int i;
        for (i = 0; !(v & 1); v>>=1, i++);
        return i;
#endif
    }

    int scanBitReverse(quint64 v)
    {
        if (!v) {
            return -1;
        }
#if _MSC_VER
        ulong i;
        _BitScanReverse64(&i, v);
        return i;
#elif __GNUC__
        return __builtin_clzll(v);
#else
        int i;
        for (i = 0; v; v>>=1, i++);
        return i-1;
#endif
    }

    function<QRgb(const uchar*,int,int)> pixelReader(const QImage& image)
    {
        switch (image.format()) {
        case QImage::Format_RGB32:
            return [](const uchar* sl, int x, int) -> QRgb { return 0xff000000 | reinterpret_cast<const QRgb*>(sl)[x]; };
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            return [](const uchar* sl, int x, int) -> QRgb { return reinterpret_cast<const QRgb*>(sl)[x]; };
            break;
        default:
            return [&image](const uchar*, int x, int y) -> QRgb { return image.pixel(x, y); };
        }
    }

}
