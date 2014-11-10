
#include "mainLightcurve.h"
#include "gui.h"
#include "math.h"
#include "threads.h"

extern Gui *gui;
#define MAX_CHANNEL 1024
#define NUM_DETECTORS 7

extern pthread_mutex_t timebinmutex;


// the constructor method
mainLightcurve::mainLightcurve(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymax = 1000;
	ymin = 0;
	xmax = 20;
	xmin = 0;

	current_timebin_detectors[0] = 0;
	current_timebin_detectors[1] = 0;
	current_timebin_detectors[2] = 0;
	current_timebin_detectors[3] = 0;
	current_timebin_detectors[4] = 0;
	current_timebin_detectors[5] = 0;
	current_timebin_detectors[6] = 0;
	current_timebin = 0;
	total_counts = 0;
	
	for(int i = 0; i < MAX_CHANNEL; i++)
	{
		CountcurveFunction[i] = i;
		binsize[i] = 1;
		CountRatecurveFunction[i] = i;
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			CountRatecurveDetectors[i][detector_num] = i;
		}
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
	
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
	{
		if (gui->mainHistogramWindow->get_detector_display(detector_num) == 1) {
			k = 0;
			glBegin(GL_LINES);
			glColor3f(0.0, 1.0, 0.0);
			k = 0;
			for (float x = 0; x < xmax; x+=binsize[k])
			{
				CountRatecurveDetectors[k][detector_num] = CountcurveDetectors[k][detector_num]/binsize[k];
				glVertex2f(x, CountRatecurveDetectors[k][detector_num]);
				glVertex2f(x + binsize[k], CountRatecurveDetectors[k][detector_num]);
				k++;
			}
			glEnd();
		}
	}
		
	glPopMatrix();
	glFinish();
	
	// update the display of the current count rate
	gui->ctsOutput->value(CountcurveFunction[0]/binsize[0]);
	gui->totalctsOutput->value(total_counts);
}

void mainLightcurve::set_xmax(int newxmax){
	xmax = newxmax;
}

void mainLightcurve::set_ymax(int newymax){
	ymax = newymax;
}

void mainLightcurve::flush(int detector_number)
{
	if( (detector_number < NUM_DETECTORS) && (detector_number >= 0) ){
		for(int i = 0; i < MAX_CHANNEL; i++){
			CountRatecurveDetectors[i][detector_number] = 0;}}
	if (detector_number == 7) {
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			for(int i = 0; i < MAX_CHANNEL; i++){
				CountcurveFunction[i] = 0;
				CountRatecurveDetectors[i][detector_num] = 0;
			}
		}
	}
	total_counts = 0;
	redraw();
}

void mainLightcurve::add_count(int detector_number)
{
	if( (detector_number < NUM_DETECTORS) && (detector_number >= 0) ){
		pthread_mutex_trylock(&timebinmutex);
		current_timebin_detectors[detector_number]++;
		current_timebin++;
		total_counts++;
		pthread_mutex_unlock(&timebinmutex);
	}
}

void mainLightcurve::reset(float current_binsize)
{
	// save the info into the arrays
	CountcurveFunction[0] = current_timebin;
	binsize[0] = current_binsize;
	
	for(int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
	{
		CountcurveDetectors[0][detector_num] = current_timebin_detectors[detector_num];
	}
	
	// now shift everything back one
	for(int j = MAX_CHANNEL-1; j > 0; j--){ 
		CountcurveFunction[j] = CountcurveFunction[j-1];
		binsize[j] = binsize[j-1];
	}
	
	for(int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
	{
		for(int j = MAX_CHANNEL-1; j > 0; j--){ CountcurveDetectors[j][detector_num] = CountcurveDetectors[j-1][detector_num];}
	}
	
	pthread_mutex_lock(&timebinmutex);
	// now reset all values to zero to start accumulating next time bin
	for(int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
		{ current_timebin_detectors[detector_num] = 0; }
	current_timebin = 0;
	pthread_mutex_unlock(&timebinmutex);
	
}