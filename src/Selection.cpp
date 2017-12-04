#include "Selection.h"
#include "Helper.h"
#include "GraphicsItems.h"
#include <QGraphicsScene>

Selection::Selection(QGraphicsScene* scene, QObject *parent)
	: QObject(parent)
	, visible_(true)
	, highlightVisible_(true)
	, outlineDashOffset_(0)
	, scene_(scene)
	, outlineAnimation_(this, "outlineDashOffset")
{
	outlineAnimation_.setDuration(1000);
	outlineAnimation_.setStartValue(0);
	outlineAnimation_.setEndValue(-10);
	outlineAnimation_.setLoopCount(-1);
}

Selection::Selection(const QImage& bmp, bool highlightVisible, QGraphicsScene* scene, QObject* parent)
	: Selection(scene, parent)
{
	highlightVisible_ = highlightVisible;
	setArea(bmp);
}

Selection::~Selection()
{
	qDeleteAll(outlines_);
}

void Selection::setArea(const QImage& bmp)
{
	bitmap_ = bmp;
	if (bmp.format()  != QImage::Format_MonoLSB) {
		bitmap_ = QImage();
	}
	highlight_.reset();
	qDeleteAll(outlines_);
	outlines_.clear();
	outlineAnimation_.stop();
	initOulines();
	if (highlightVisible_) {
		initHighlight();
	}
	if (visible_ && !outlines_.empty()) {
		outlineAnimation_.start();
	}
}

QImage Selection::filledArea() const
{
	return bitmap_.copy(filledAreaRect());
}

QRect Selection::filledAreaRect() const
{
	if (filledAreaRect_.isEmpty()) {
		filledAreaRect_ = help::filledRect(bitmap_);
	}
	return filledAreaRect_;
}

void Selection::initHighlight()
{
	bitmap_.setColor(0, QColor(Qt::transparent).rgba());
	bitmap_.setColor(1, QColor(0,191,255,30).rgba());
	highlight_.reset(new QGraphicsPixmapItem(QPixmap::fromImage(bitmap_)));
	highlight_->setVisible(visible_);
	scene_->addItem(highlight_.data());
}

void Selection::initOulines()
{
	auto sz = bitmap_.size();
	QImage map(sz, QImage::Format_MonoLSB);		// left outlines map
	map.fill(0);
	for (int y = 0; y < sz.height(); y++) {
		auto sl = bitmap_.scanLine(y);
		auto osl = map.scanLine(y);
		uchar last = 0;
		for (int x = 0; x < sz.width(); x++) {
			uchar bit = qLsbBit(sl, x);
			if (bit && !last && !qLsbBit(osl, x)) {
				auto boundary = help::traceBoundary(bitmap_, QPoint(x,y), QPoint(x-1,y));
				auto outline = outlineFromBoundary(boundary, map);
				outlines_ << outline;
				outline->setVisible(visible_);
				scene_->addItem(outline);
			}
			last = bit;
		}
	}
}

OutlineItem* Selection::outlineFromBoundary(const QVector<QPoint>& boundary, QImage& map)
{
	auto outline = help::outlineFromBoundary(boundary);
	QVector<QPointF> outlineF;
	outlineF.reserve(outline.size());
	int c = outline.count() - 1;
	const QPoint dir(0,1);
	for (int i = 0; i < c; i++) {
		outlineF << outline[i];
		if ((outline[i] - outline[i+1]) == dir) {
			auto& p = outline[i+1];
			qLsbSet(map.scanLine(p.y()), p.x());
		}
	}
	outlineF << outline.last();
	return new OutlineItem(outlineF);
}

void Selection::setVisible(bool visible)
{
	if (visible_ != visible) {
		visible_ = visible;
		if (highlight_) {
			highlight_->setVisible(visible);
		}
		for (auto& o : outlines_) {
			o->setVisible(visible);
		}
		if (!visible && outlineAnimation_.state() == QPropertyAnimation::Running) {
			outlineAnimation_.pause();
		} else if (visible) {
			if (outlineAnimation_.state() == QPropertyAnimation::Paused) {
				outlineAnimation_.resume();
			} else if (outlineAnimation_.state() == QPropertyAnimation::Stopped) {
				outlineAnimation_.start();
			}
		}
	}
}

void Selection::setOutlineDashOffset(int offset)
{
	if (outlineDashOffset_ != offset) {
		outlineDashOffset_ = offset;
		for (auto& o : outlines_) {
			o->setDashOffset(offset);
		}
	}
}

void Selection::setHighlightVisible(bool visible)
{
	if (highlightVisible_ != visible) {
		highlightVisible_ = visible;
		if (visible && highlight_.isNull()) {
			initHighlight();
		}
		if (!highlight_.isNull()) {
			highlight_->setVisible(visible_);
		}
	}
}
