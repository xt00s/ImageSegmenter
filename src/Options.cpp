#include "Options.h"

Options& Options::get()
{
	static Options options;
	return options;
}

Options::Options()
{
	keepZoomLevel = false;
	keepingZoom = 1;
	resetToolSelection = false;
	resetCategorySelection = false;
}
