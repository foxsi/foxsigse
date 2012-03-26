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
	bool show_mask;
		
	void set_ymax(double value);
	double get_ymax(void);
	void set_detector_to_display(int detector_number);
private:
	int detector_to_display;
	double ymax;
};

#endif
