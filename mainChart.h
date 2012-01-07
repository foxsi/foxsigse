#ifndef _mainChart_h_     // prevent multiple includes
#define _mainChart_h_

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Chart.H>
#include <FL/gl.h>
//#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include "Application.h"

class mainChart : public Fl_Chart {
public:
	mainChart(int x,int y,int w,int h,const char *l=0);
};

#endif
