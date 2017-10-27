#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>

class Slider : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor)
	Q_PROPERTY(QColor grooveColor READ grooveColor WRITE setGrooveColor)
public:
	explicit Slider(QWidget *parent = 0);

	void setRange(int min, int max);
	int minimum() const;
	int maximum() const;

	void setValue(int value);
	int value() const;

	QColor handleColor() const;
	void setHandleColor(const QColor& handleColor);

	QColor grooveColor() const;
	void setGrooveColor(const QColor& grooveColor);

	QSize sizeHint() const override;

signals:
	void valueChanged(int value);

protected:
	virtual int valueFromPos(int pos, int minPos, int maxPos);
	virtual int posFromValue(int value, int minPos, int maxPos);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	int boundValue(int value) const;
	int boundPos(int pos) const;
	void updateValue();
	void updatePos();
	bool overHandle(const QPoint& pos) const;

private:
	QColor handleColor_;
	QColor grooveColor_;
	QPoint pressedMousePos_;
	bool pressed_;
	int pressedPos_;
	int pos_;
	int min_;
	int max_;
	int value_;
};

inline int Slider::minimum() const { return min_; }
inline int Slider::maximum() const { return max_; }
inline int Slider::value() const { return value_; }
inline QColor Slider::handleColor() const { return handleColor_; }
inline QColor Slider::grooveColor() const { return grooveColor_; }

#endif // SLIDER_H
