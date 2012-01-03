
#include "mainLightcurve.h"
#include "gui.h"
#include "math.h"

extern Gui *gui;
#define MAX_CHANNEL 1024

unsigned int current_timebin = 0;
extern unsigned int LightcurveFunction[MAX_CHANNEL];
extern long displayLightcurve[MAX_CHANNEL];
extern float mainLightcurve_binsize[MAX_CHANNEL];

// the constructor method
mainLightcurve::mainLightcurve(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymax = 1000;
	ymin = 0;
	xmax = 20;
	xmin = 0;
	
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		LightcurveFunction[i] = i;
		mainLightcurve_binsize[i] = 1;
	}
}

// the drawing method: draws the histFunc into the window
void mainLightcurve::draw() 
{
	unsigned int y = 0;
	int k = 0;
	
	//xmax = MAX_CHANNEL/mainLightcurve_binsize;
	
	if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//set camera
	glViewport(0,0,w(),h());	
	gluOrtho2D(0, xmax, 0, ymax);
   	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//draw the graph
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	
	for (float x = 0; x < xmax; x+=mainLightcurve_binsize[k]){
		glVertex2f(x, LightcurveFunction[k]/mainLightcurve_binsize[k]);
		glVertex2f(x + mainLightcurve_binsize[k], LightcurveFunction[k]/mainLightcurve_binsize[k]);
		
		glVertex2f(x + mainLightcurve_binsize[k]/2.0, (LightcurveFunction[k] + sqrt(LightcurveFunction[k]))/mainLightcurve_binsize[k]);
		glVertex2f(x + mainLightcurve_binsize[k]/2.0, (LightcurveFunction[k] - sqrt(LightcurveFunction[k]))/mainLightcurve_binsize[k]);
		k++;
	}
	
	glEnd();
	glPopMatrix();
	glFinish();
	
	gui->ctsOutput->value(LightcurveFunction[0]/mainLightcurve_binsize[0]);
}

void mainLightcurve::set_xmax(int newxmax){
	xmax = newxmax;
}

