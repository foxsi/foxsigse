#ifndef _mainImage_h_     // prevent multiple includes
#define _mainImage_h_

#include <time.h>
#include <FL/Fl_Gl_Window.H>

class mainImage : public Fl_Gl_Window {
   public:
	mainImage(int x,int y,int w,int h,const char *l=0);
	void draw();
	int handle(int eventType);
	double maximumValue(double *array);
	clock_t start_time;
};

#endif
