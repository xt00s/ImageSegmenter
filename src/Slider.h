#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>

class Slider : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor)
	Q_PROPERTY(QColor grooveColor READ grooveColor WRITE setGrooveColor)
	Q_PROPERTY(QColor valueGrooveColor READ valueGrooveColor WRITE setValueGrooveColor)

public:
	Slider(QWidget *parent = 0);

	double value() const;
	void setValue(double value);

	QColor handleColor() const;
	void setHandleColor(const QColor& handleColor);

	QColor grooveColor() const;
	void setGrooveColor(const QColor& grooveColor);

	QColor valueGrooveColor() const;
	void setValueGrooveColor(const QColor& valueGrooveColor);

	QSize sizeHint() const override;

signals:
	void valueChanged(double value);

protected:
	int position() const;

	virtual double valueFromPos(int pos, int minPos, int maxPos) const;
	virtual int posFromValue(double value, int minPos, int maxPos) const;

	virtual void paint(QPainter& p);
	virtual QSize handleSize() const;
	virtual QRect handleSpaceRect() const;

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	double boundValue(double value) const;
	int boundPos(int pos) const;
	void updateValue();
	void updatePos();
	bool overHandle(const QPoint& pos) const;

private:
	QColor handleColor_;
	QColor grooveColor_;
	QColor valueGrooveColor_;
	QPoint pressedMousePos_;
	bool pressed_;
	int pressedPos_;
	int pos_;
	double value_;
};

inline QColor Slider::handleColor() const { return handleColor_; }
inline QColor Slider::grooveColor() const { return grooveColor_; }
inline QColor Slider::valueGrooveColor() const { return valueGrooveColor_; }
inline int Slider::position() const { return pos_; }
inline double Slider::value() const { return value_; }

#endif // SLIDER_H
