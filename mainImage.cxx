
#include "mainImage.h"

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>

#include <time.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include "gui.h"
#include "Application.h"
#include "UsefulFunctions.h"

#define XSTRIPS 128
#define YSTRIPS 128
#define	XBORDER 5
#define YBORDER 5
#define ZOOMNUM 5
#define NUM_DETECTORS 7

//todo
//need to auto scale the image, add scalling types (auto, set/linear, logarithmic)
//this needs maximumValue function, add in a functions.cxx file

//each pixel is exactly 1x1 in GL coordinates to make it easier

float GL_cursor[2];
int mousePixel[2];
int chosenPixel[2];

extern Gui *gui;
extern Foxsidata *data;

// preference variable
extern int mainImage_minimum;

mainImage::mainImage(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{	
	detector_to_display = 0;
}

// the drawing method: draws the histFunc into the window
void mainImage::draw() 
{
	double grey, vert, bleu, alpha;
	clock_t pixel_time, current_time;
	float half_life;
	double elapsed_time;
	int show_mask;
		
	ymax = gui->detectorsImageWindow->get_ymax(detector_to_display);
	show_mask = gui->showmask_checkbox->value();
	
	if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glViewport(0,0,w(),h());	
   	glOrtho(0,XSTRIPS+2*XBORDER,0,YSTRIPS+2*YBORDER,0,-1);
   	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT);  
	
	half_life = gui->pixel_halflife_slider->value();
	current_time = clock();

	for(int j = 0; j < XSTRIPS; j++)
	{
		for(int i = 0; i < YSTRIPS; i++)
		{
			grey = gui->detectorsImageWindow->get_image(i, j, detector_to_display)/ymax;
			
			if (grey != 0)
			{
				pixel_time = gui->detectorsImageWindow->get_imagetime(i, j, detector_to_display);
				if (half_life != 0){
					elapsed_time = ((double) current_time - pixel_time)/(CLOCKS_PER_SEC);
					alpha = exp(-elapsed_time*0.693/half_life);
				} else {
					alpha = 1.0;
				}
				
				glColor4f(grey, grey, grey, alpha);
				glBegin(GL_QUADS);
				glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
				glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
				glEnd();
			}
			//now draw the mask
			
			if (show_mask == 1)
			{
				vert = gui->detectorsImageWindow->get_mask(i, j, detector_to_display);
				if (vert == 1){
					glColor4f(0, vert, 0, 0.5);
					glBegin(GL_QUADS);
					glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
					glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
					glEnd();
				}
			}
			
		}
	}

	//draw a border around the detector
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(XBORDER, YBORDER); glVertex2f(XBORDER+XSTRIPS, YBORDER); 
	glVertex2f(XBORDER+XSTRIPS, YBORDER+YSTRIPS); glVertex2f(XBORDER, YBORDER+YSTRIPS);
	glEnd();
	
	//draw a border around the detector
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
	glVertex2f(XBORDER, YBORDER); glVertex2f(XBORDER+XSTRIPS, YBORDER); 
	glVertex2f(XBORDER+XSTRIPS, YBORDER+YSTRIPS); glVertex2f(XBORDER, YBORDER+YSTRIPS);
	glEnd();
	
	// draw a cross under the cursor selection
	if(gui->Lockbut->value() == 0)
	{
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
			glVertex2f(mousePixel[0] + XBORDER - 3.5, mousePixel[1] + YBORDER);
			glVertex2f(mousePixel[0] + XBORDER + 4.5, mousePixel[1] + YBORDER);
		glEnd();
		
		glBegin(GL_LINES);
		glVertex2f(mousePixel[0] + XBORDER - 3.5, mousePixel[1] + YBORDER + 1);
		glVertex2f(mousePixel[0] + XBORDER + 4.5, mousePixel[1] + YBORDER + 1);
		glEnd();	
		
		glBegin(GL_LINES);
			glVertex2f(mousePixel[0] + XBORDER, mousePixel[1] + YBORDER - 3.5); 
			glVertex2f(mousePixel[0] + XBORDER, mousePixel[1] + YBORDER + 4.5); 
		glEnd();
		
		glBegin(GL_LINES);
		glVertex2f(mousePixel[0] + XBORDER + 1, mousePixel[1] + YBORDER - 3.5); 
		glVertex2f(mousePixel[0] + XBORDER + 1, mousePixel[1] + YBORDER + 4.5); 
		glEnd();
	} else {
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glVertex2f(chosenPixel[0] + XBORDER - 3.5, chosenPixel[1] + YBORDER);
		glVertex2f(chosenPixel[0] + XBORDER + 4.5, chosenPixel[1] + YBORDER);
		glEnd();
		
		glBegin(GL_LINES);
		glVertex2f(chosenPixel[0] + XBORDER - 3.5, chosenPixel[1] + YBORDER + 1);
		glVertex2f(chosenPixel[0] + XBORDER + 4.5, chosenPixel[1] + YBORDER + 1);
		glEnd();	
		
		glBegin(GL_LINES);
		glVertex2f(chosenPixel[0] + XBORDER, chosenPixel[1] + YBORDER - 3.5); 
		glVertex2f(chosenPixel[0] + XBORDER, chosenPixel[1] + YBORDER + 4.5); 
		glEnd();
		
		glBegin(GL_LINES);
		glVertex2f(chosenPixel[0] + XBORDER + 1, chosenPixel[1] + YBORDER - 3.5); 
		glVertex2f(chosenPixel[0] + XBORDER + 1, chosenPixel[1] + YBORDER + 4.5); 
		glEnd();
		
		//now update the text box with the current chosen pixel
		Fl::lock();
		gui->pixelCounts->value(gui->detectorsImageWindow->get_image(chosenPixel[0], chosenPixel[1], detector_to_display));
		Fl::unlock();
	}

	// draw vertical line at center of detector
	glColor3f(0, 0.5, 0);
	glBegin(GL_LINES);
	glVertex2f(XSTRIPS/2.0 + XBORDER + 0.5, YBORDER);
	glVertex2f(XSTRIPS/2.0 + XBORDER + 0.5, YSTRIPS + YBORDER);	glEnd();
	glEnd();
	
	// draw vertical line at center of detector
	glColor3f(0, 0.5, 0);
	glBegin(GL_LINES);
	glVertex2f(XBORDER, YSTRIPS/2.0 + YBORDER + 0.5);
	glVertex2f(XSTRIPS + XBORDER, YSTRIPS/2.0 + YBORDER + 0.5);
	glEnd();	
	
	glPopMatrix();	
}

int mainImage::handle(int eventType)
{
	int button;
	char text[8];
	
	button=Fl::event_button();
	
	//convert between fltk coordinates and openGL coordinates
	GL_cursor[0]=Fl::event_x()*(XSTRIPS + 2.0*XBORDER)/w();
	GL_cursor[1]=(h()-Fl::event_y())*(YSTRIPS + 2.0*YBORDER)/h();
	
	//translate to pixel number but keep within bounds 
	mousePixel[0] = (GL_cursor[0] - XBORDER) > 1 ? GL_cursor[0] - XBORDER : 1;
	mousePixel[0] = mousePixel[0] < XSTRIPS ? mousePixel[0]: XSTRIPS;
	
	mousePixel[1] = (GL_cursor[1] - YBORDER) > 1 ? GL_cursor[1] - YBORDER : 1;
	mousePixel[1] = mousePixel[1] < YSTRIPS ? mousePixel[1]: YSTRIPS;
		
	if((eventType==FL_PUSH)&&(button==1))
	{
		//set the view lock button to ON
		gui->Lockbut->set();
		//save the location
		chosenPixel[0] = mousePixel[0];	chosenPixel[1] = mousePixel[1];
	}
	
	//now update the text box with the current chosen pixel
	Fl::lock();
	if (gui->Lockbut->value() == 0) {
		gui->pixelCounts->value(gui->detectorsImageWindow->get_image(mousePixel[0], mousePixel[1], detector_to_display));
		sprintf( text, "%d,%d", mousePixel[0], mousePixel[1]);
		gui->pixelNum->value(text);
		
		gui->pixelCounts->value(gui->detectorsImageWindow->get_image(mousePixel[0], mousePixel[1], detector_to_display));
		float arcminX;
		float arcminY;
		arcminX = (mousePixel[0] - XSTRIPS/2) * 7.7/60.0;
		arcminY = (mousePixel[1] - YSTRIPS/2) * 7.7/60.0;
		sprintf( text, "%d,%d", mousePixel[0], mousePixel[1]);
		gui->pixelNum->value(text);
		sprintf( text, "%2.1f,%2.1f", arcminX, arcminY);
		gui->arcminOffset->value(text);
	}
	Fl::unlock();
	redraw();
	
	return(1);
}

void mainImage::set_ymax(double value)
{
	ymax = value;
}

void mainImage::set_detector_to_display(int detector_number)
{
	if ((detector_number >= 0) && (detector_number < NUM_DETECTORS)) {
		detector_to_display = detector_number;
	} else {detector_to_display = 0;}
}
