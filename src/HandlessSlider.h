#ifndef HANDLESSSLIDER_H
#define HANDLESSSLIDER_H

#include "Slider.h"
#include <functional>

class HandlessSlider : public Slider
{
public:
	typedef std::function<QString(double)> TextFormatter;

public:
	HandlessSlider(QWidget *parent = 0);

	bool textVisible() const;
	void setTextVisible(bool textVisible);

	void setTextFormatter(const TextFormatter& formatter);

	QFont textFont() const;
	void setTextFont(const QFont& textFont);

protected:
	void paint(QPainter& p) override;
	QSize handleSize() const override;
	QRect handleSpaceRect() const override;

private:
	bool textVisible_;
	TextFormatter textFormatter_;
	TextFormatter defaultTextFormatter_;
	QFont textFont_;
};

inline bool HandlessSlider::textVisible() const { return textVisible_; }
inline QFont HandlessSlider::textFont() const { return textFont_; }

#endif // HANDLESSSLIDER_H
