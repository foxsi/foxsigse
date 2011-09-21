#ifndef _mainImage_h_     // prevent multiple includes
#define _mainImage_h_

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
//#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include "Application.h"
#include <time.h>
#include <math.h>

class mainImage : public Fl_Gl_Window {
   public:
	mainImage(int x,int y,int w,int h,const char *l=0);
	void draw();
	int handle(int eventType);
	double maximumValue(double *array);
};

#endif
