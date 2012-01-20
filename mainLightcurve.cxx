
#include "mainLightcurve.h"
#include "gui.h"
#include "math.h"

extern Gui *gui;
#define MAX_CHANNEL 1024

unsigned int current_timebin = 0;
extern unsigned int CountcurveFunction[MAX_CHANNEL];

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
		CountcurveFunction[i] = i;
		binsize[i] = 1;
		CountRatecurveFunction[i] = i;
	}
}

// the drawing method: draws the histFunc into the window
void mainLightcurve::draw() 
{
	int k = 0;
	float count_error = 0;
	
	if (!valid()) {
		make_current();
	}
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (gui->mainLightcurve_ymaxslider->value() == 0){
		// find the maximum
		ymax = CountRatecurveFunction[0];
		for (float x = 1; x < xmax; x+=binsize[k])
		{
			if (ymax < CountRatecurveFunction[k]) {
				ymax = CountRatecurveFunction[k];
			}
			k++;
		}
	} else {
		ymax = gui->mainLightcurve_ymaxslider->value();
	}

	
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
	k = 0;
	for (float x = 0; x < xmax; x+=binsize[k])
	{
		CountRatecurveFunction[k] = CountcurveFunction[k]/binsize[k];

		count_error = sqrt((float) CountcurveFunction[k]);
		
		glVertex2f(x, CountRatecurveFunction[k]);
		glVertex2f(x + binsize[k], CountRatecurveFunction[k]);
		
		glVertex2f(x + binsize[k]/2.0, (CountcurveFunction[k] + count_error)/binsize[k]);
		glVertex2f(x + binsize[k]/2.0, (CountcurveFunction[k] - count_error)/binsize[k]);
		k++;
	}
	
	glEnd();
	glPopMatrix();
	glFinish();
	
	// update the display of the current count rate
	gui->ctsOutput->value(CountcurveFunction[0]/binsize[0]);
}