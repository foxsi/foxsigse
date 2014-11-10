#include "mainHistogram.h"
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

mainHistogram::mainHistogram(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	ymax = 1024;
	ymin = 0;
	xmax = MAX_CHANNEL;
	xmin = 0;
	detector_display[0] = 0;
	detector_display[1] = 0;
	detector_display[2] = 0;
	detector_display[3] = 0;
	detector_display[4] = 0;
	detector_display[5] = 0;
	detector_display[6] = 0;
	detector_display[7] = 1;
	
	//initialize the histogram
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		HistogramFunction[i] = 7*i;
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			HistogramFunctionDetectors[i][detector_num] = i;
		}
	}
	FLHistcursorX[0] = 500;
	mouseHistPixel = FLHistcursorX[0];
	chan_to_energy = 10.0;
	binsize = 25;
	
	low_threshold = 100;
}

void mainHistogram::draw()
{
	// the drawing method: draws the histFunc into the window
	int sum = 0;
	int index = 0;
	
	// recalculate the displayed histogram (rebinned)
	for(int i = 0; i < MAX_CHANNEL; i+=binsize)
	{
		sum = 0;
		if (binsize > 1) {		
			for (int j = 0; j < binsize; j++) {
				sum += HistogramFunction[i+j];}}
		else { sum = HistogramFunction[i]; }
		displayHistogramFunction[index] = sum;
		
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) 
		{
			sum = 0;
			if (binsize > 1) {				
				for (int j = 0; j < binsize; j++) {
					sum += HistogramFunctionDetectors[i+j][detector_num];}
			} else {
				sum = HistogramFunctionDetectors[i][detector_num];
			}
			displayHistogramDetectors[index][detector_num] = sum;
		}
		index++;
	}
	
	if (gui->mainHistogram_ymax_slider->value() == 0) {	
		ymax = maximumValue(displayHistogramFunction, xmax/binsize, low_threshold/binsize);
	} 
	
	if (ymax == 0){ ymax = 100; }
		
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
	gluOrtho2D(0,xmax/binsize, 0, ymax);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(detector_display[7] == 1)
	{
		// draw the total histogram
		glColor3f(1.0, 0.0, 0.0);
		
		// draw the points and error bars
		glBegin(GL_LINES);
		for(int i = 0; i < MAX_CHANNEL/binsize; i++)
		{
			glVertex2f(i, displayHistogramFunction[i]);
			glVertex2f(i+1, displayHistogramFunction[i]);	
			glVertex2f(i+0.5, displayHistogramFunction[i] - sqrt(displayHistogramFunction[i]));
			glVertex2f(i+0.5, displayHistogramFunction[i] + sqrt(displayHistogramFunction[i]));
		}
		glEnd();
		
		// draw the connecting line
		glBegin(GL_LINE_LOOP);
		glVertex2f(0,0);
		for(int i = 0; i < MAX_CHANNEL/binsize; i++) { glVertex2f(i+0.5, displayHistogramFunction[i]); }
		glVertex2f(MAX_CHANNEL/binsize - 0.5,0);
		glEnd();
	}
	
	//draw the detectors histogram
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
	{
		if (detector_display[detector_num] == 1) {
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINES);
			for(int i = 0; i < MAX_CHANNEL/binsize; i++)
			{
				glVertex2f(i, 7*displayHistogramDetectors[i][detector_num]);
				glVertex2f(i+1, 7*displayHistogramDetectors[i][detector_num]);	
				//glVertex2f(i+0.5, 7*displayHistogramDetectors[i][detector_num] - 7*sqrt(displayHistogramDetectors[i][detector_num]));
				//glVertex2f(i+0.5, 7*displayHistogramDetectors[i][detector_num] + 7*sqrt(displayHistogramDetectors[i][detector_num]));
			}
			glEnd();
			
			// draw the connecting line
			glBegin(GL_LINE_LOOP);
			glVertex2f(0,0);
			for(int i = 0; i < MAX_CHANNEL/binsize; i++) { glVertex2f(i+0.5, 7*displayHistogramDetectors[i][detector_num]); }
			glVertex2f(MAX_CHANNEL/binsize,0);
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
	glRectf(0, 0, (int) low_threshold/binsize, ymax);
	
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f((int) low_threshold/binsize, 0);
	glVertex2f((int) low_threshold/binsize, ymax);
	glEnd();
	
	glColor3f(0.0, 1.0, 0.0);
	char buffer [50];
	sprintf(buffer, "%u", ymax);
	text_output(xmax/binsize*0.8, ymax*0.8, buffer);
	
	glPopMatrix();
	glFinish();
	
	gui->histCounts->value(displayHistogramFunction[mouseHistPixel]);

}

int mainHistogram::handle(int eventType)
{
	char text[8];
	int button;
	button=Fl::event_button();
	if(eventType == FL_PUSH)
	{
		//convert between fltk coordinates and opengl coordinates
		FLHistcursorX[0] = floor(Fl::event_x()*(xmax/binsize)/w());
		FLHistcursorY[0] = floor((h()-Fl::event_y())*(ymax)/h());
		mouseHistPixel = FLHistcursorX[0];
		redraw();	
	}
	
	// now update the value displayed
	if( gui->mainHistogram_choice->value() == 0 ){
		gui->histEnergy->value(mouseHistPixel*binsize);	
	}
	if( gui->mainHistogram_choice->value() == 1 ){
		gui->histEnergy->value(mouseHistPixel*binsize/chan_to_energy);	
	}
	gui->histCounts->value(displayHistogramFunction[mouseHistPixel]);
	
	return(1);
}

void mainHistogram::update_detector_display(bool on, int detector_number)
{
	if ((detector_number >= 0) && (detector_number < NUM_DETECTORS+1)){
		detector_display[detector_number] = on;
	}
}

int mainHistogram::get_detector_display(int detector_number){
	return detector_display[detector_number];
}

int mainHistogram::get_binsize(void){
	return binsize;
}

void mainHistogram::set_binsize(int newbinsize){
	binsize = newbinsize;
}

void mainHistogram::set_xmax(int value){
	xmax = value;
}

int mainHistogram::get_ymax(void){
	return ymax;
}

void mainHistogram::add_count(int bin_number, int detector_number){
	if ((bin_number > 0) && (bin_number < 1024) && (detector_number < NUM_DETECTORS) && (detector_number >= 0)){
		HistogramFunctionDetectors[bin_number][detector_number] += 1;
		HistogramFunction[bin_number] += 1;
	}
}

void mainHistogram::flush(int detector_number)
{
	if( (detector_number < NUM_DETECTORS) && (detector_number >= 0) ){
		for(int i = 0; i < MAX_CHANNEL; i++){
			HistogramFunctionDetectors[i][detector_number] = 0;
			HistogramFunction[i] = 0;}}
	if (detector_number == 7) {
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			for(int i = 0; i < MAX_CHANNEL; i++){
				HistogramFunctionDetectors[i][detector_num] = 0;
				HistogramFunction[i] = 0;
			}
		}
	}
	redraw();
}

void mainHistogram::save(void)
{
	Fl_File_Chooser *chooser    = NULL;
	FILE *file;
	
	chooser = new Fl_File_Chooser("", "", Fl_File_Chooser::CREATE, "Save File");
	
	chooser->show();
	
	while(chooser->shown()) {
		Fl::wait();
	}
	
	if ( chooser->value() != NULL ) {		
		file = fopen(chooser->value(), "w");
		if(file != NULL){
			
			struct tm *times;
			time_t ltime;
			char stringtemp[80];
			
			time(&ltime);
			times = localtime(&ltime);
			
			strftime(stringtemp,25,"%Y/%m/%d %H:%M:%S\0",times);
			
			fprintf(file, "FOXSI Histogram\n");
			fprintf(file, "Created %s\n", stringtemp);
			fprintf(file, "---------------\n");
			fprintf(file, "channel, counts\n");
			for (int i; i<MAX_CHANNEL; i++) {
				fprintf(file, "%i, %d\n", i, HistogramFunction[i]);
			}
			
			fclose(file);
		} else {
			gui->app->print_to_console("Could not open file\n");
		}
		
	}	
}

void mainHistogram::set_ymax(unsigned long value){
	if (value != 0) {
		ymax = value;
	}
	redraw();
	gui->detectorsImageWindow->redraw();
}

void mainHistogram::text_output(int x, int y, char *string)
{
	int len, i;
	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
	}
}

long mainHistogram::get_detectorDisplayHistogram(int bin, int detector_number)
{
	if ((bin < MAX_CHANNEL) && (detector_number < NUM_DETECTORS)){
		return displayHistogramDetectors[bin][detector_number];
	} else {
		return 0;
	}
}

long mainHistogram::get_detectorHistogram(int bin, int detector_number)
{
	if ((bin < MAX_CHANNEL) && (detector_number < NUM_DETECTORS)){
		return HistogramFunctionDetectors[bin][detector_number];
	} else {
		return 0;
	}
}

void mainHistogram::set_lowthreshold(int value)
{
	if ((low_threshold > 0) && (low_threshold < 1024)) {low_threshold = value;}
}

int mainHistogram::get_lowthreshold(void)
{
	return low_threshold;
}

