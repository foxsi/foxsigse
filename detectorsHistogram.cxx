#include "detectorsHistogram.h"
#include "gui.h"
#include <math.h>
#include "UsefulFunctions.h"
#include <FL/Fl_File_Chooser.H>

#define XSTRIPS 128
#define YSTRIPS 128
#define TICKLENGTH 3
#define YNUMTICKS 10
#define XTICKINTERVAL 5
#define XTICKINTERVALM 10
#define NUM_DETECTORS 7
#define XMIN 1
#define MAX_CHANNEL 1024

extern Gui *gui;

detectorsHistogram::detectorsHistogram(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymin = 0;
	ymax[0] = 250;
	xmax = MAX_CHANNEL;
	xmin = 0;

	FLHistcursorX[0] = 500;
	mouseHistPixel[0] = FLHistcursorX[0];
	chan_to_energy = 10.0;
}

void detectorsHistogram::draw()
{
	// the drawing method: draws the histFunc into the window
	int y = 0;
	int k = 0;
	binsize = gui->mainHistogramWindow->get_binsize();
	ymax[0] = gui->mainHistogramWindow->get_ymax();
			
	if (!valid()) {
		make_current();
	}
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0,0,w(),h());
	gluOrtho2D(0,xmax*NUM_DETECTORS/binsize, 0, ymax[0]/6);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
		
	//draw the detectors histogram
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
	{
		char optic_name[50];
		
		// draw a line separating the histograms
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);
		glVertex2f(xmax*detector_num/binsize, 0);
		glVertex2f(xmax*detector_num/binsize, ymax[0]/6);
		glEnd();
		
		glColor3f(0.0, 1.0, 0.0);
		
		switch (detector_num) {
			case 0:
				sprintf(optic_name, "6");
				break;
			case 6:
				sprintf(optic_name, "0");
				break;
			default:
				sprintf(optic_name, "%d", detector_num);
				break;
		}
		
		text_output((xmax*detector_num + xmax*0.8)/binsize, ymax[0]*0.8/6, optic_name);
		
		glColor3f(0.0, 1.0, 0.0);
		for(int i = 0; i < MAX_CHANNEL/binsize; i++)
		{
			long y = gui->mainHistogramWindow->get_detectorDisplayHistogram(i, detector_num);
			glBegin(GL_LINES);
			glVertex2f(i + xmax/binsize*detector_num + detector_num, y);
			glVertex2f(i + 1 + xmax/binsize*detector_num + detector_num, y);
			glEnd();
			
			glBegin(GL_LINES);
			// add error bars
			glVertex2f(i + xmax/binsize*detector_num + detector_num + 0.5, y + sqrt(y));
			glVertex2f(i + xmax/binsize*detector_num + detector_num + 0.5, y - sqrt(y));
			glEnd();
		}
		
		// draw the connecting line
		glBegin(GL_LINE_LOOP);
		glVertex2f(0.5 + xmax/binsize*detector_num + detector_num, 0);
		for(int i = 0; i < MAX_CHANNEL/binsize; i++) { 
			long y = gui->mainHistogramWindow->get_detectorDisplayHistogram(i, detector_num);
			glVertex2f(i + 0.5 + xmax/binsize*detector_num + detector_num, y); }
		glVertex2f(MAX_CHANNEL/binsize + 0.5 + xmax/binsize*detector_num + detector_num, 0);
		glEnd();
	}
			
	glPopMatrix();
	glFinish();
	
}

int detectorsHistogram::handle(int eventType)
{
	char text[8];
	int button;
	button=Fl::event_button();
	if(eventType == FL_PUSH)
	{
		//convert between fltk coordinates and opengl coordinates
		FLHistcursorX[0] = floor(Fl::event_x()*(xmax/binsize*NUM_DETECTORS)/w());
		FLHistcursorY[0] = floor((h()-Fl::event_y())*(ymax[0])/h());
		mouseHistPixel[0] = FLHistcursorX[0];
		mouseHistPixel[1] = FLHistcursorY[0];
		printf("pos = [%i, %i]\n", mouseHistPixel[0], mouseHistPixel[1]);
		redraw();	
	}
	
	return(1);
}

void detectorsHistogram::set_binsize(int newbinsize){
	binsize = newbinsize;
}

void detectorsHistogram::set_xmax(int newxmax){
	xmax = newxmax;
}

void detectorsHistogram::set_ymax(long value){
	if (value != 0) {
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			ymax[detector_num] = value;
		}
	}
}

void detectorsHistogram::text_output(int x, int y, char *string)
{
	int len, i;
	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
	}
}

