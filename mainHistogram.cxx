#include "mainHistogram.h"
#include "gui.h"

#define XBORDER 0
#define YBORDER 0
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

int chosenHistPixel;
int mouseHistPixel;
int low_threshold = 0;

float FLHistcursorX[2], FLHistcursorY[2];

float xmid = 50;
float xmax = 100;

float YTICKINTERVAL;
float YTICKINTERVALM;

extern Gui *gui;

mainHistogram::mainHistogram(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	/* initialize random seed: */
	//srand ( time(NULL) );
	
	//initialize the histogram
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		HistogramFunction[i] = i*1;
	}
	FLHistcursorX[0] = 500;
	mouseHistPixel = FLHistcursorX[0] - XBORDER;
}

void mainHistogram::draw()
{
	// the drawing method: draws the histFunc into the window
	int ymax;
	int ymin = 0;
	YTICKINTERVALM = (ymax-ymin)/YNUMTICKS;
	YTICKINTERVAL = YTICKINTERVALM/2;
	ymax = maximumValue(HistogramFunction);
	
	if (!valid()) {
		make_current();
	}
	
	//The following line causes the display to not refresh after being moved
	//gui->mainChartWindow->bounds(0.0, ymax);
	
	float FL_Yconv = (float) (MAX_CHANNEL)/(ymax - ymin);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0,0,w(),h());
	glOrtho(0,MAX_CHANNEL+2*XBORDER,0,MAX_CHANNEL+2*YBORDER,0,-1);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	//draw the graph
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(XBORDER, YBORDER);
	
	for(int i=0; i<MAX_CHANNEL; i++)
	{
		glVertex2f(i+XBORDER, (YBORDER + HistogramFunction[i])*FL_Yconv);
	}
	glVertex2f(MAX_CHANNEL+XBORDER, YBORDER);
	glVertex2f(XBORDER, YBORDER);
	glEnd();
	
	// draw vertival select line from mouse click
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(FLHistcursorX[0], YBORDER);
	glVertex2f(FLHistcursorX[0], MAX_CHANNEL);
	glEnd();
	
	// draw the line showing the low energy cutoff
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(low_threshold, YBORDER);
	glVertex2f(low_threshold, MAX_CHANNEL);
	glEnd();
	
	glPopMatrix();
	glFinish();
	
	//update the axis labels
	gui->mainHistogramYlabelmax->value(ymax);
	gui->mainHistogramYlabelmid->value(ymax/2.0);
	
	//Should not make this update everytime
	//gui->mainHistogramXlabelmid->value(xmid);
	//gui->mainHistogramXlabelmax->value(xmax);
	
	//if(gui->mainHistogramLockbut->value() == 0)
	//      {
	//              //now update the text box with the current chosen pixel
	//              sprintf( text, "%d", mouseHistPixel);
	//              gui->histEnergy->value(text);
	//
	//              sprintf( text, "%d", HistogramFunction[mouseHistPixel]);
	//              gui->histCounts->value(text);
	//      } else
	//      {
	//              sprintf( text, "%d", HistogramFunction[chosenHistPixel]);
	//              gui->histCounts->value(text);
	//      }
	
}

int mainHistogram::handle(int eventType)
{
	char text[8];
	int button;
	button=Fl::event_button();
	if(eventType == FL_PUSH)
	{
		//convert between fltk coordinates and opengl coordinates
		FLHistcursorX[0] = floor(Fl::event_x()*(MAX_CHANNEL + 2*XBORDER)/w());
		FLHistcursorY[0] = floor((h()-Fl::event_y())*(YSTRIPS + 2*YBORDER)/h());

		mouseHistPixel = FLHistcursorX[0] - XBORDER;
	}
	
	// now update the value displayed
	gui->histEnergy->value(mouseHistPixel);	
	gui->histCounts->value(HistogramFunction[mouseHistPixel]);
	
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

int mainHistogram::maximumValue(int *array)
{
	//float length = sizeof(array)/sizeof(array[0]);  // establish size of array
	//cout << "size of array " << length << endl;
	//do not consider below xmin
	int max = array[XMIN];       // start with max = first element
	for(int i = XMIN + 1; i < MAX_CHANNEL; i++)
	{
		if(array[i] > max){
			max = array[i];}
	}
	
	return max;                // return highest value in array
}