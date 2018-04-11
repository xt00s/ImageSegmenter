#ifndef OPTIONS_H
#define OPTIONS_H


class Options
{
public:
	bool keepZoomLevel;
	double keepingZoom;
	bool resetToolSelection;
	bool resetCategorySelection;

	static Options& get();

private:
	Options();
};

#endif // OPTIONS_H
