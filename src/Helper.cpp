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

	DefaultCursor::DefaultCursor()
	{
		QApplication::setOverrideCursor(QCursor());
	}

	DefaultCursor::~DefaultCursor()
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

	QImage flood(const QImage& src, const QPoint& start, const std::function<bool(QRgb)>& test)
	{
		if (!src.valid(start)) {
			return QImage();
		}
		QSize sz = src.size();
		QImage bmp(sz, QImage::Format_MonoLSB);
		bmp.fill(0);

		std::function<QRgb(const uchar*,int,int)> getPixel;
		switch (src.format()) {
		case QImage::Format_RGB32:
			getPixel = [](const uchar* sl, int x, int) -> QRgb { return 0xff000000 | reinterpret_cast<const QRgb*>(sl)[x]; };
			break;
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
			getPixel = [](const uchar* sl, int x, int) -> QRgb { return reinterpret_cast<const QRgb*>(sl)[x]; };
			break;
		default:
			getPixel = [&src](const uchar*, int x, int y) -> QRgb { return src.pixel(x, y); };
		}

		struct Range {
			int x;
			int y;
			int maxX;
			int dir;
		};
		QVector<Range> ranges(1, {start.x(), start.y(), sz.width(), 0});

		while (!ranges.empty()) {
			auto r = ranges.takeFirst();
			auto br = bmp.scanLine(r.y);
			auto sr = src.constScanLine(r.y);
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
					} else {
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
				}
				s = e+1;
			}
		}
		return bmp;
	}

	QRect filledRect(const QImage& bmp)
	{
		if (bmp.format() != QImage::Format_MonoLSB) {
			return QRect();
		}
		auto sz = bmp.size();
		auto bitsInBlock = sizeof(quint64) * 8;
		int blocks = sz.width() / bitsInBlock;
		int l = sz.width(), r = -1, t = -1, b = -1, lb = blocks-1, rb = 0;
		quint64 lastBlockMask = (1 << (sz.width() % bitsInBlock)) - 1;

		for (int y = 0, i,j; y < sz.height(); y++) {
			auto sl = reinterpret_cast<const quint64*>(bmp.scanLine(y));
			for (i = 0; i <= lb && !sl[i]; i++);

			j = -1;
			if (i <= lb) {
				j = scanBitForward(sl[i]);
				lb = i;
			} else if (i == blocks) {
				j = scanBitForward(sl[blocks] & lastBlockMask);
			}
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
				for (i = blocks-1; i >= rb && !sl[i]; i--);
				if (i >= rb) {
					j = scanBitReverse(sl[i]);
					rb = i;
				}
			}
			if (j >= 0) {
				int newR = i * bitsInBlock + j;
				if (newR > r) {
					r = newR;
				}
			}
			if (y > b) {
				b = y;
			}
		}
		return l < sz.width() ? QRect(QPoint(l,t), QPoint(r,b)) : QRect();
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

}
