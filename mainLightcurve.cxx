
#include "mainLightcurve.h"
#include "gui.h"

extern Gui *gui;
#define MAX_CHANNEL 1024

int current_timebin;
int timebins[1024];

// the constructor method
mainLightcurve::mainLightcurve(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymax = 1000;
	ymin = 0;
	xmax = MAX_CHANNEL;
	xmin = 0;
	
	for(int i=0; i < 1024; i++)
	{
		timebins[i] = 100;
	}
	
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
   	glOrtho(0,MAX_CHANNEL,0,ymax,0,-1);
   	glMatrixMode(GL_MODELVIEW);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//draw the graph
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0, 0.0, 0.0);
	
	glVertex2f(0, 0);
	
	for (int i = 0; i < MAX_CHANNEL; i++) {
		glVertex2f(i, timebins[i]);
		//glVertex2f(i+1, timebins[i]);
	}
	glVertex2f(MAX_CHANNEL, 0);
	
	glEnd();
	
}
