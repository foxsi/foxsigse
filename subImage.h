/*
 *  subImage.h
 *  Untitled
 *
 *  Created by Steven Christe on 7/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _subImage_h_     // prevent multiple includes
#define _subImage_h_

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include "Application.h"

class subImage : public Fl_Gl_Window {
public:
	subImage(int x,int y,int w,int h,const char *l=0);
	void draw();
	int handle(int eventType);
};

#endif