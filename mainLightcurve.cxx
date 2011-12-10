
#include "mainLightcurve.h"
#include "gui.h"

extern Gui *gui;

// the constructor method
mainLightcurve::mainLightcurve(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	
}

// the drawing method: draws the histFunc into the window
void mainLightcurve::draw() 
{
	if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//set camera
	glViewport(0,0,w(),h());	
   	glOrtho(0,100,0,100,0,-1);
   	glMatrixMode(GL_MODELVIEW);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
}
