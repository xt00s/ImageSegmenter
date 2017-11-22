#include "GraphicsItems.h"
#include "Helper.h"
#include "Scheme.h"
#include "Drawables.h"
#include <QPainter>
#include <QBitmap>
#include <QGraphicsScene>

CanvasItem::CanvasItem(QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, scheme_(0)
	, category_(0)
	, maskVisible_(true)
	, pixmapVisible_(true)
	, pixmapGray_(false)
	, pixmapOpacity_(1)
{
	outlineAnimation_ = new QPropertyAnimation(this, "outlineDashOffset", this);
	outlineAnimation_->setDuration(1000);
	outlineAnimation_->setStartValue(0);
	outlineAnimation_->setEndValue(-10);
	outlineAnimation_->setLoopCount(-1);
}

QRectF CanvasItem::boundingRect() const
{
	return QRectF(QPointF(), pixmap_.size());
}

QPainterPath CanvasItem::shape() const
{
	return shape_;
}

QPainterPath CanvasItem::opaqueArea() const
{
	return shape();
}

void CanvasItem::setPixmap(const QPixmap& pixmap, const QPixmap& mask)
{
	prepareGeometryChange();
	pixmap_ = pixmap;
	pixmapG_ = pixmapGray_ ? help::rgb2gray(pixmap) : QPixmap();
	shape_ = QPainterPath();
	shape_.addRect(QRectF(QPointF(), pixmap.size()));
	clipRegion_ = QRegion();
	initLayers(mask);
}

void CanvasItem::setScheme(const Scheme* scheme)
{
	scheme_ = scheme;
}

void CanvasItem::setCategory(const Category* category)
{
	category_ = category;
}

void CanvasItem::setClipRegion(const Category* cat)
{
	qDeleteAll(outlines_);
	outlines_.clear();
	outlineAnimation_->stop();
	clipRegion_ = QRegion();

	if (cat && cat->index() >= 0 && !layers_.empty()) {
		auto mask = layers_[cat->index()].createMaskFromColor(cat->color(), Qt::MaskOutColor);
		auto bmp = mask.toImage().convertToFormat(QImage::Format_MonoLSB);
		auto sz = bmp.size();
		QImage obmp(sz, QImage::Format_MonoLSB);
		obmp.fill(0);
		for (int y = 0; y < sz.height(); y++) {
			auto sl = bmp.scanLine(y), osl = obmp.scanLine(y);
			uchar last = 0;
			for (int x = 0; x < sz.width(); x++) {
				uchar bit = qLsbBit(sl, x);
				if (bit && !last && !qLsbBit(osl, x)) {
					addOutline(obmp, help::traceBoundary(bmp, QPoint(x,y), QPoint(x-1,y), false));
				}
				last = bit;
			}
		}
		if (!outlines_.empty()) {
			for (auto& l : outlines_) {
				scene()->addItem(l);
			}
			clipRegion_ = QRegion(mask);
			outlineAnimation_->start();
		}
	}
}

void CanvasItem::addOutline(QImage& bmp, const QVector<QPoint>& boundary)
{
	for (auto& p : boundary) {
		qLsbSet(bmp.scanLine(p.y()), p.x());
	}
	auto outline = help::outlineFromBoundary(boundary);
	QVector<QPointF> outlineF;
	outlineF.reserve(outline.size());
	for (auto& p : outline) {
		outlineF << p;
	}
	outlines_ << new OutlineItem(outlineF);
}

void CanvasItem::drawPolygon(const QPolygon& polygon)
{
	draw(Polygon(polygon));
}

void CanvasItem::drawLine(const QLineF& line, qreal width)
{
	draw(Line(line, width));
}

void CanvasItem::drawCircle(const QPointF& center, qreal radius)
{
	draw(Circle(center, radius));
}

void CanvasItem::draw(const Drawable& shape)
{
	if (category_ && category_->index() < 0) {
		erase(shape);
	} else {
		drawColored(shape);
	}
}

CanvasItem::Fragment* CanvasItem::extractFragment(const QRect& rect, const Category* category) const
{
	auto r = rect == QRect() ? pixmap_.rect() : rect.intersected(pixmap_.rect());
	if (r.isEmpty()) {
		return 0;
	}
	if (category && category->index() < 0)
		category = 0;
	auto f = new Fragment(r, category);
	if (category) {
		f->layerFragements_ << layers_[category->index()].copy(r);
	}
	else {
		f->layerFragements_.reserve(layers_.count());
		for (auto& l : layers_) {
			f->layerFragements_ << l.copy(r);
		}
	}
	return f;
}

CanvasItem::Fragment* CanvasItem::Fragment::extract(const QRect& rect) const
{
	QRect vr, rr;
	if (rect == QRect()) {
		vr = rect_;
		rr = QRect(QPoint(), rect_.size());
	} else {
		vr = rect.intersected(rect_);
		rr = QRect(vr.topLeft() - rect_.topLeft(), vr.size());
	}
	if (rr.isEmpty()) {
		return 0;
	}
	auto f = new Fragment(vr, category_);
	f->layerFragements_.reserve(layerFragements_.count());
	for (auto& l : layerFragements_) {
		f->layerFragements_ << l.copy(rr);
	}
	return f;
}

void CanvasItem::restoreFragment(const Fragment& fragment)
{
	if (pixmap_.rect().contains(fragment.rect_)) {
		if (fragment.category_) {
			for (int i = 0; i < scheme_->categoryCount(); i++) {
				auto c = scheme_->categoryAt(i);
				if (c != fragment.category_) {
					QPainter p(&layers_[c->index()]);
					p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
					p.drawPixmap(fragment.rect_, fragment.layerFragements_[0]);
				}
			}
			QPainter p(&layers_[fragment.category_->index()]);
			p.setCompositionMode(QPainter::CompositionMode_Source);
			p.drawPixmap(fragment.rect_, fragment.layerFragements_[0]);
		}
		else {
			for (int i = 0; i < layers_.count(); i++) {
				QPainter p(&layers_[i]);
				p.setCompositionMode(QPainter::CompositionMode_Source);
				p.drawPixmap(fragment.rect_, fragment.layerFragements_[i]);
			}
		}
		update(fragment.rect_);
	}
}

void CanvasItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
	if (pixmapVisible_) {
		painter->setOpacity(pixmapOpacity_);
		painter->drawPixmap(QPointF(), pixmapGray_ ? pixmapG_ : pixmap_);
		painter->setOpacity(0.4 + 0.6 * (1 - pixmapOpacity_));
	}
	if (maskVisible_ && scheme_ && !layers_.empty()) {
		for (int i = 0; i < scheme_->categoryCount(); i++) {
			if (scheme_->categoryAt(i)->isVisible()) {
				painter->drawPixmap(QPointF(), layers_[i]);
			}
		}
	}
}

void CanvasItem::initLayers(const QPixmap& mask)
{
	layers_.clear();
	auto size = pixmap_.size();
	if (!scheme_ || !scheme_->categoryCount() || size.isNull())
		return;
	layers_.reserve(scheme_->categoryCount());
	for (int i = 0; i < scheme_->categoryCount(); i++) {
		if (mask.isNull() || mask.size() != pixmap_.size()) {
			auto layer = QPixmap(size);
			layer.fill(Qt::transparent);
			layers_ << layer;
		}
		else {
			auto layer = mask.copy();
			layer.setMask(mask.createMaskFromColor(scheme_->categoryAt(i)->color(), Qt::MaskOutColor));
			layers_ << layer;
		}
	}
}

void CanvasItem::drawColored(const Drawable& shape)
{
	if (!scheme_ || !category_ || category_->isLocked())
		return;

	auto r = shape.rect();
	QPainter p(&layers_[category_->index()]);
	p.setRenderHint(QPainter::Antialiasing, false);
	if (!clipRegion_.isEmpty()) {
		p.setClipRegion(clipRegion_);
	}
	shape.draw(p, category_->color());
	p.setCompositionMode(QPainter::CompositionMode_DestinationOut);

	for (int i = 0; i < scheme_->categoryCount(); i++) {
		auto c = scheme_->categoryAt(i);
		if (c != category_) {
			if (!c->isLocked()) {
				QPainter p2(&layers_[c->index()]);
				p2.setRenderHint(QPainter::Antialiasing, false);
				if (!clipRegion_.isEmpty()) {
					p2.setClipRegion(clipRegion_);
				}
				p2.setCompositionMode(QPainter::CompositionMode_Source);
				shape.draw(p2, Qt::transparent);
			} else {
				p.drawPixmap(r, layers_[c->index()], r);
			}
		}
	}
	update(r);
}

void CanvasItem::erase(const Drawable& shape)
{
	if (!scheme_)
		return;

	for (int i = 0; i < scheme_->categoryCount(); i++) {
		auto c = scheme_->categoryAt(i);
		if (!c->isLocked()) {
			QPainter p(&layers_[c->index()]);
			p.setRenderHint(QPainter::Antialiasing, false);
			if (!clipRegion_.isEmpty()) {
				p.setClipRegion(clipRegion_);
			}
			p.setCompositionMode(QPainter::CompositionMode_Source);
			shape.draw(p, Qt::transparent);
		}
	}
	update(shape.rect());
}

void CanvasItem::setOutlineDashOffset(int offset)
{
	if (outlineDashOffset_ != offset) {
		outlineDashOffset_ = offset;
		for (auto& l : outlines_) {
			l->setDashOffset(offset);
		}
	}
}

QPixmap CanvasItem::mask(const QColor& background) const
{
	QPixmap m(pixmap_.size());
	m.fill(background);
	QPainter p(&m);
	for (auto& l : layers_) {
		p.drawPixmap(QPoint(), l);
	}
	return m;
}

void CanvasItem::setPixmapVisible(bool visible)
{
	if (pixmapVisible_ != visible) {
		pixmapVisible_ = visible;
		update();
	}
}

void CanvasItem::setMaskVisible(bool visible)
{
	if (maskVisible_ != visible) {
		maskVisible_ = visible;
		update();
	}
}

void CanvasItem::setPixmapGray(bool gray)
{
	if (pixmapGray_ != gray) {
		pixmapGray_ = gray;
		if (pixmapGray_ && pixmapG_.isNull()) {
			pixmapG_ = help::rgb2gray(pixmap_);
		}
		update();
	}
}

void CanvasItem::setPixmapOpacity(double opacity)
{
	if (pixmapOpacity_ != opacity) {
		pixmapOpacity_ = opacity;
		update();
	}
}

//-----------------------------------------------------------------------------------

struct TransformHolder
{
    TransformHolder(QPainter* painter_) {
        painter = painter_;
		orig = painter->transform();
	}
	~TransformHolder() {
		painter->setTransform(orig);
	}
	QPainter* painter;
	QTransform orig;
};

struct RenderHintsHolder
{
	RenderHintsHolder(QPainter* painter_) {
		painter = painter_;
		orig = painter->renderHints();
	}
	~RenderHintsHolder() {
		painter->setRenderHints(~orig, false);
		painter->setRenderHints(orig, true);
	}
	QPainter* painter;
	QPainter::RenderHints orig;
};

//-----------------------------------------------------------------------------------

PolylineItem::PolylineItem(QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, darkPen_(QColor(60, 60, 60), 1.7, Qt::DashLine)
	, lightPen_(QColor(255, 255, 255, 100), 1.7)
{
	darkPen_.setDashPattern({4, 6});
}

void PolylineItem::begin()
{
	prepareGeometryChange();
	polygon_ << QPoint() << QPoint();
}

void PolylineItem::setLastPoint(const QPointF& p)
{
	if (!polygon_.empty() && polygon_.last() != p) {
		prepareGeometryChange();
		polygon_[polygon_.count()-1] = p;
	}
}

void PolylineItem::removeLastCommittedPoint()
{
	if (polygon_.count() > 1) {
		prepareGeometryChange();
		polygon_.remove(polygon_.count()-2);
	}
}

void PolylineItem::commitPoint(const QPointF& p)
{
	if (polygon_.count() >= 2 && polygon_[polygon_.count()-2] != p) {
		prepareGeometryChange();
		polygon_[polygon_.count()-1] = p;
		polygon_ << p;
	}
}

void PolylineItem::commit()
{
	if (!polygon_.empty())
		polygon_.removeLast();
}

void PolylineItem::clear()
{
	if (!polygon_.empty()) {
		prepareGeometryChange();
		polygon_.clear();
	}
}

bool PolylineItem::canClose() const
{
	return polygon_.count() > 3;
}

bool PolylineItem::empty() const
{
	return polygon_.count() < 2;
}

QRectF PolylineItem::boundingRect() const
{
	return polygon_.boundingRect().adjusted(-1,-1,1,1);
}

void PolylineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	TransformHolder holder(painter);
	auto maped = holder.orig.map(polygon_);
	painter->setTransform(QTransform());
	painter->setBrush(Qt::NoBrush);
	painter->setPen(lightPen_);
	painter->drawPolyline(maped);
	painter->setPen(darkPen_);
	painter->drawPolyline(maped);
}

//-----------------------------------------------------------------------------------

OutlineItem::OutlineItem(const QVector<QPointF>& outline, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, outline_(outline)
	, darkPen_(QColor(0, 0, 0), 1, Qt::DashLine)
	, lightPen_(QColor(255, 255, 255), 1)
	, dashOffset_(0)
{
	darkPen_.setDashPattern({4,6});
}

OutlineItem::OutlineItem(const QPolygonF& outline, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, outline_(outline)
	, darkPen_(QColor(0, 0, 0), 1, Qt::DashLine)
	, lightPen_(QColor(255, 255, 255), 1)
	, dashOffset_(0)
{
	darkPen_.setDashPattern({4,6});
}

QRectF OutlineItem::boundingRect() const
{
	return outline_.boundingRect().adjusted(-1,-1,1,1);
}

void OutlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	TransformHolder holder(painter);
	RenderHintsHolder holder2(painter);
	auto maped = holder.orig.map(outline_);
	painter->setTransform(QTransform());
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setBrush(Qt::NoBrush);
	painter->setPen(lightPen_);
	painter->drawPolyline(maped);
	painter->setPen(darkPen_);
	painter->drawPolyline(maped);
}

void OutlineItem::setDashOffset(int offset)
{
	if (offset != dashOffset_) {
		dashOffset_ = offset;
		darkPen_.setDashOffset(offset);
		update();
	}
}

//-----------------------------------------------------------------------------------

StartMarkerItem::StartMarkerItem(QGraphicsItem* parent)
	: QGraphicsItem(parent), hovering_(false)
{
	setFlag(ItemIgnoresTransformations);
}

void StartMarkerItem::clear()
{
	hovering_ = false;
}

QRectF StartMarkerItem::boundingRect() const
{
	return hovering_ ? QRectF(-8,-8,16,16) : QRectF(-6,-6,12,12);
}

QPainterPath StartMarkerItem::shape() const
{
	QPainterPath path;
	path.addEllipse(boundingRect());
	return path;
}

void StartMarkerItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	prepareGeometryChange();
	hovering_ = true;
}

void StartMarkerItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	prepareGeometryChange();
	hovering_ = false;
}

void StartMarkerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(Qt::cyan).darker(hovering_ ? 130 : 100));
	painter->drawEllipse(boundingRect());
}

//-----------------------------------------------------------------------------------

BrushCursorItem::BrushCursorItem(QGraphicsItem* parent)
	: QGraphicsItem(parent), width_(1)
{
}

void BrushCursorItem::setWidth(qreal width)
{
	if (width != width_) {
		prepareGeometryChange();
		width_ = width;
	}
}

QRectF BrushCursorItem::boundingRect() const
{
	return QRectF(-width_/2, -width_/2, width_, width_).adjusted(-2,-2,2,2);
}

void BrushCursorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	TransformHolder holder(painter);
	auto maped = holder.orig.mapRect(QRectF(0, 0, width_, width_));
	painter->setTransform(QTransform());
	painter->setBrush(Qt::NoBrush);
	painter->setPen(Qt::white);
	painter->drawEllipse(maped.topLeft(), maped.width()/2, maped.height()/2);
	painter->setPen(Qt::black);
	painter->drawEllipse(maped.topLeft(), maped.width()/2 + 0.7, maped.height()/2 + 0.7);
}
