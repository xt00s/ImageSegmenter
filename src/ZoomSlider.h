#ifndef ZOOMSLIDER_H
#define ZOOMSLIDER_H

#include "Slider.h"

class ZoomSlider : public Slider
{
	Q_OBJECT
public:
	explicit ZoomSlider(QWidget *parent = 0);

	void setZoomRange(double min, double max);
	void setZoomTicks(const QVector<double>& ticks);

	void setZoom(double zoom);
	double zoom() const;

public slots:
	void zoomIn();
	void zoomOut();

signals:
	void zoomChanged(double zoom);

protected:
	int valueFromPos(int pos, int minPos, int maxPos) override;
	int posFromValue(int value, int minPos, int maxPos) override;

private slots:
	void valueChanged(int value);

private:
	double boundZoom(double zoom) const;

private:
	double zoom_;
	double min_;
	double max_;
	QVector<double> ticks_;
};

inline double ZoomSlider::zoom() const { return zoom_; }

#endif // ZOOMSLIDER_H
