#ifndef _mainHistogram_h_     // prevent multiple includes
#define _mainHistogram_h_

#define NUM_DETECTORS 7
#define XMIN 1
#define MAX_CHANNEL 1024

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/glut.H>
#include <stdio.h>
#include <stdlib.h>
#include "Application.h"
#include <math.h>

#if defined(__APPLE_CC__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

class mainHistogram : public Fl_Gl_Window {
public:
	mainHistogram(int x,int y,int w,int h,const char *l=0);
 	void draw();
	int handle(int eventType);
	void glPrint(float x, float y, char *string );
	int xmax;
	int HistogramFunctionDetectors[MAX_CHANNEL][NUM_DETECTORS];
	double displayHistogramDetectors[MAX_CHANNEL][NUM_DETECTORS];
	int detector_display[NUM_DETECTORS+1];
private:
	double ymax;
	double ymin;
	int xmin;
	float chan_to_energy;
};

#endif
