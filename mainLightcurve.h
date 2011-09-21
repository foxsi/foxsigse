#ifndef _mainLightcurve_h_     // prevent multiple includes
#define _mainLightcurve_h_

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#if defined(__APPLE_CC__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "Application.h"

class mainLightcurve : public Fl_Gl_Window {
   public:
	mainLightcurve(int x,int y,int w,int h,const char *l=0);
	void draw();   
};

#endif
