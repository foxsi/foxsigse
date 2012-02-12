#include "mainHistogram.h"
#include "gui.h"
#include <math.h>
#include "UsefulFunctions.h"

#define XSTRIPS 128
#define YSTRIPS 128
#define TICKLENGTH 3
#define YNUMTICKS 10
#define XTICKINTERVAL 5
#define XTICKINTERVALM 10
#define NUM_DETECTORS 7
#define XMIN 1
#define MAX_CHANNEL 1024

int HistogramFunction[MAX_CHANNEL];
extern double displayHistogram[MAX_CHANNEL];

int chosenHistPixel;
int mouseHistPixel;
int low_threshold = 5;
int mainHistogram_binsize = 25;

float FLHistcursorX[2], FLHistcursorY[2];

float YTICKINTERVAL;
float YTICKINTERVALM;

extern Gui *gui;

mainHistogram::mainHistogram(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymax = 1024;
	ymin = 0;
	xmax = MAX_CHANNEL;
	xmin = 0;
	detector_display[0] = 1;
	detector_display[1] = 0;
	detector_display[2] = 0;
	detector_display[3] = 0;
	detector_display[4] = 0;
	detector_display[5] = 0;
	detector_display[6] = 0;
	detector_display[7] = 0;
	/* initialize random seed: */
	
	//initialize the histogram
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		HistogramFunction[i] = i*1;
		displayHistogram[i] = HistogramFunction[i];
		for (int detector_num = 0; detector_num < NUM_DETECTORS+1; detector_num++) {
			HistogramFunctionDetectors[i][detector_num] = i*2 + detector_num;
			displayHistogramDetectors[i][detector_num] = HistogramFunctionDetectors[i][detector_num];}
	}
	FLHistcursorX[0] = 500;
	mouseHistPixel = FLHistcursorX[0];
	chan_to_energy = 10.0;
}

void mainHistogram::draw()
{
	// the drawing method: draws the histFunc into the window
	int y = 0;
	int k = 0;
	
	ymax = maximumValue(displayHistogram, xmax/mainHistogram_binsize, low_threshold/mainHistogram_binsize);

	if (ymax == 0){ ymax = 100; }
		
	YTICKINTERVALM = (ymax-ymin)/YNUMTICKS;
	YTICKINTERVAL = YTICKINTERVALM/2;
	
	if (!valid()) {
		make_current();
	}
	
	//The following line causes the display to not refresh after being moved
	//gui->mainChartWindow->bounds(0.0, ymax);
	
	//float FL_Yconv = (float) (MAX_CHANNEL)/(ymax - ymin);
	//float FL_Xconv = (float) (MAX_CHANNEL)/(MAX_CHANNEL*mainHistogram_binsize- 0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0,0,w(),h());
	gluOrtho2D(0,xmax/mainHistogram_binsize, 0, ymax);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
		
	// recalculate the displayed histogram (rebinned)
	for(int i = 0; i < MAX_CHANNEL; i+=mainHistogram_binsize)
	{
		y = 0;
		for (int j = 0; j < mainHistogram_binsize; j++) {
			y += HistogramFunction[i+j];}
		displayHistogram[k] = y;
		
		for (int detector_num = 0; detector_num < NUM_DETECTORS+1; detector_num++) {
			y = 0;
			for (int j = 0; j < mainHistogram_binsize; j++) {
				y += HistogramFunctionDetectors[i+j][detector_num];}
			displayHistogramDetectors[k][detector_num] = y;
		}
		k++;
	}
	
	if (detector_display[0] == 1)
	{
		//draw the total detector histogram
		glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);	
		
		for(int i = 0; i < MAX_CHANNEL/mainHistogram_binsize; i++)
		{
			glVertex2f(i, displayHistogram[i]);
			glVertex2f(i+1, displayHistogram[i]);
			
			glVertex2f(i+0.5, displayHistogram[i] - sqrt(displayHistogram[i]));
			glVertex2f(i+0.5, displayHistogram[i] + sqrt(displayHistogram[i]));
		
		}
		//glVertex2f(MAX_CHANNEL+XBORDER, YBORDER);
		//glVertex2f(XBORDER, YBORDER);
		glEnd();
	}
	
	//draw the total detector histogram
	for (int detector_num = 1; detector_num < NUM_DETECTORS+1; detector_num++)
	{
		if (detector_display[detector_num] == 1)
		{
			glBegin(GL_LINES);
			glColor3f(0.0, 0.0, 1.0);	
			for(int i = 0; i < MAX_CHANNEL/mainHistogram_binsize; i++)
			{
				glVertex2f(i, 7*displayHistogramDetectors[i][detector_num]);
				glVertex2f(i+1, 7*displayHistogramDetectors[i][detector_num]);	
				//glVertex2f(i+0.5, 7*displayHistogramDetectors[i][detector_num] - sqrt(displayHistogramDetectors[i][detector_num]));
				//glVertex2f(i+0.5, 7*displayHistogramDetectors[i][detector_num] + sqrt(displayHistogramDetectors[i][detector_num]));
			}
			glEnd();
		}
	}	
	
	// draw vertival select line from mouse click
	glBegin(GL_LINES);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(FLHistcursorX[0]+0.5, 0);
	glVertex2f(FLHistcursorX[0]+0.5, ymax);
	glEnd();
	
	// draw the line showing the low energy cutoff
	glColor4f(0.0, 0.0, 1.0, 0.5);
	glRectf(0, 0, (int) low_threshold/mainHistogram_binsize, ymax);
	
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f((int) low_threshold/mainHistogram_binsize, 0);
	glVertex2f((int) low_threshold/mainHistogram_binsize, ymax);
	glEnd();
	
	glPopMatrix();
	glFinish();
	
	//update the axis labels
	gui->mainHistogramYlabelmax->value(ymax);
	gui->mainHistogramYlabelmid->value(ymax/2.0);
	gui->histCounts->value(displayHistogram[mouseHistPixel]);

}

int mainHistogram::handle(int eventType)
{
	char text[8];
	int button;
	button=Fl::event_button();
	if(eventType == FL_PUSH)
	{
		//convert between fltk coordinates and opengl coordinates
		FLHistcursorX[0] = floor(Fl::event_x()*(xmax/mainHistogram_binsize)/w());
		FLHistcursorY[0] = floor((h()-Fl::event_y())*(ymax)/h());

		mouseHistPixel = FLHistcursorX[0];
	}
	
	// now update the value displayed
	if( gui->mainHistogram_choice->value() == 0 ){
		gui->histEnergy->value(mouseHistPixel*mainHistogram_binsize);	
	}
	if( gui->mainHistogram_choice->value() == 1 ){
		gui->histEnergy->value(mouseHistPixel*mainHistogram_binsize/chan_to_energy);	
	}
	gui->histCounts->value(displayHistogram[mouseHistPixel]);
	
	redraw();
	
	      //if((eventType==FL_PUSH)&&(button==1))
	      //{
	//      //      FLHistcursorX[1]=FLHistcursorX[0];
	//      //      FLHistcursorY[1]=FLHistcursorY[0];
	//      //      gui->mainHistogramLockbut->set();
	//      //}
	//      //if((eventType==FL_PUSH)&&(button==2))
	//      //{
	//      //      FLHistcursorX[1]=0;
	//      //      FLHistcursorY[1]=0;
	//      //}
	//
	//      //if(gui->mainHistogramLockbut->value() == 0){chosenHistPixel = FLHistcursorX[1] - XBORDER;}
	//
	//      //if(gui->mainHistogramLockbut->value() == 0){gui->mainHistogramWindow->redraw();}
	//      //redraw();
	//
	return(1);
}

//void mainHistogram::glPrint(float x, float y, char *string )
//{
//      int len, i;
//      glRasterPos2f(x, y);
//      //find the length of the string
//      len = (int) strlen(string);
//      //print each character individually
//      for ( i = 0; i < len; i++){ glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
//      }
//}
