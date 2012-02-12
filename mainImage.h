#ifndef _mainImage_h_     // prevent multiple includes
#define _mainImage_h_

#include <time.h>
#include <FL/Fl_Gl_Window.H>

#define XSTRIPS 128
#define YSTRIPS 128
#define NUM_DETECTORS 7

class mainImage : public Fl_Gl_Window {
public:
	mainImage(int x,int y,int w,int h,const char *l=0);
	void draw();
	int handle(int eventType);
	double maximumValue(double *array);
	clock_t start_time;
	bool show_mask;
	
	double detectorsImage[XSTRIPS][YSTRIPS][NUM_DETECTORS];
	double detectorsImagealpha[XSTRIPS][YSTRIPS][NUM_DETECTORS];
	double detectorsImagetime[XSTRIPS][YSTRIPS][NUM_DETECTORS];
};

#endif
