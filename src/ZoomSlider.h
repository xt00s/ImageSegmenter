#ifndef ZOOMSLIDER_H
#define ZOOMSLIDER_H

#include <QWidget>

class ZoomSlider : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor)
	Q_PROPERTY(QColor grooveColor READ grooveColor WRITE setGrooveColor)
public:
	explicit ZoomSlider(QWidget *parent = 0);

	void setRange(double min, double max);
	void setTicks(const QVector<double>& ticks);

	void setZoom(double zoom);
	double zoom() const;

	QColor handleColor() const;
	void setHandleColor(const QColor& handleColor);

	QColor grooveColor() const;
	void setGrooveColor(const QColor& grooveColor);

public slots:
	void zoomIn();
	void zoomOut();

signals:
	void zoomChanged(double zoom);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	double boundZoom(double zoom) const;
	int boundPos(int pos) const;
	void updateZoom();
	void updatePos();
	bool overHandle(const QPoint& pos) const;

private:
	QColor handleColor_;
	QColor grooveColor_;
	bool mouseOverHandle_;
	bool pressed_;
	int pos_;
	double zoom_;
	double min_;
	double max_;
	QVector<double> ticks_;
};

inline double ZoomSlider::zoom() const { return zoom_; }
inline QColor ZoomSlider::handleColor() const { return handleColor_; }
inline QColor ZoomSlider::grooveColor() const { return grooveColor_; }

#endif // ZOOMSLIDER_H
