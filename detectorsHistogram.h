#ifndef _detectorsHistogram_h_     // prevent multiple includes
#define _detectorsHistogram_h_

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

class detectorsHistogram : public Fl_Gl_Window {
public:
	detectorsHistogram(int x,int y,int w,int h,const char *l=0);
 	void draw();
	int handle(int eventType);
	void set_binsize(int newbinsize);
	void set_xmax(int newxmax);
	void add_count(int bin_number, int detector_number);
	void flush(int detector_number);
	void save(void);
	void set_ymax(long value);
private:
	void text_output(int x, int y, char *string);
	long ymax[NUM_DETECTORS];
	long ymin;
	int xmin;
	float chan_to_energy;
	float FLHistcursorX[2];
	float FLHistcursorY[2];
	int mouseHistPixel[2];
	int detector_display[NUM_DETECTORS+1];
	int binsize;
	int xmax;
};

#endif
