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
	void update_detector_display(bool on, int detector_number);
	int get_detector_display(int detector_number);
	long get_detectorDisplayHistogram(int bin, int detector_number);
	long get_detectorHistogram(int bin, int detector_number);
	void set_binsize(int newbinsize);
	void set_xmax(int newxmax);
	void add_count(int bin_number, int detector_number);
	void flush(int detector_number);
	void save(void);
	void set_ymax(unsigned long value);
	int get_binsize(void);
	int get_ymax(void);
	void set_lowthreshold(int value);
	int get_lowthreshold(void);
private:
	void text_output(int x, int y, char *string);
	unsigned long HistogramFunction[MAX_CHANNEL];
	unsigned long displayHistogramFunction[MAX_CHANNEL];
	unsigned long HistogramFunctionDetectors[MAX_CHANNEL][NUM_DETECTORS];
	unsigned long displayHistogramDetectors[MAX_CHANNEL][NUM_DETECTORS];
	unsigned long ymax;
	unsigned long ymin;
	int xmin;
	float chan_to_energy;
	float FLHistcursorX[2];
	float FLHistcursorY[2];
	int mouseHistPixel;
	int detector_display[NUM_DETECTORS+1];
	int binsize;
	int xmax;
	unsigned short int low_threshold;
};

#endif
