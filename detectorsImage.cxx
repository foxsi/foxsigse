
#include "detectorsImage.h"

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_File_Chooser.H>

#include <time.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include "gui.h"
#include "Application.h"

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

extern Gui *gui;

// preference variable
extern int mainImage_minimum;

detectorsImage::detectorsImage(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{	
	clock_t current_time;
	current_time = clock();
	
	//initialize the image
	for(int i=0;i<XSTRIPS;i++)
	{
		for(int j=0;j<YSTRIPS;j++)
		{
			for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
				image[i][j][detector_num] = 0;
				if (j == 2) {
					image[i][j][detector_num] = 1;
				} else {
					image[i][j][detector_num] = 0;
				}

				imagetime[i][j][detector_num] = current_time;
			}
		}
	}
	
	for (int detector_num = 0; detector_num < (NUM_DETECTORS+1); detector_num++) {ymax[detector_num] = 1;}
	
	detector_buffer[0] = detector_buffer[1] = 35;
	border_buffer = 25;
	detector_angle[6] = -60; 
	// detector_angle[1] = -7.5;
	detector_angle[1] = -7.5+90.0;
	detector_angle[2] = -67.5;
	detector_angle[3] = -75;
	detector_angle[4] = -87.5+180;
	detector_angle[5] = 90;
	detector_angle[0] = 82.5;
	
	// rotate by 90 CW to correct based on lead images
	//for (int detector_num = 0; detector_num < (NUM_DETECTORS+1); detector_num++) {detector_angle[detector_num] += 90;}
}

// the drawing method: draws the histFunc into the window
void detectorsImage::draw() 
{
	double grey, vert, bleu, alpha;
	clock_t current_time, pixel_time;
	float half_life;
	double elapsed_time;
	
	half_life = gui->pixel_halflife_slider->value();
	
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
		if (gui->mainImageMax_slider->value() == 0) {
			// find the max in the image
			ymax[detector_num] = image[0][0][detector_num];
			
			for(int i = 1; i < XSTRIPS; i++)
			{
				for (int j = 1; j < YSTRIPS; j++) {
					if(image[i][j][detector_num] > ymax[detector_num]){
						ymax[detector_num] = image[i][j][detector_num];}
				}
			}
		} else {
			ymax[detector_num] = gui->mainImageMax_slider->value();
		}
	}
	for (int detector_num = 0; detector_num < (NUM_DETECTORS+1); detector_num++) {if (ymax[detector_num] == 0){ymax[detector_num] = 1;}}
	
	if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glViewport(0,0,w(),h());	
	glOrtho(0,3*XSTRIPS+4 + detector_buffer[0]*2 + 2*border_buffer,0,3*YSTRIPS+4 + detector_buffer[1]*2 + 2*border_buffer,0,-1);	
   	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
   	glPushMatrix();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT);  
	
	
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
		char optic_name[50];
		int center[] = {0,0};
		
		switch (detector_num) {
			case 6:
				center[0] = XSTRIPS; center[1] = YSTRIPS;
				sprintf(optic_name, "6");
				break;
			case 1:
				center[0] = XSTRIPS; center[1] = 2*YSTRIPS + detector_buffer[1];
				sprintf(optic_name, "%i", detector_num);
				break;
			case 2:
				center[0] = 2*XSTRIPS + detector_buffer[0]; center[1] = 1.5*YSTRIPS + detector_buffer[1];
				sprintf(optic_name, "%i", detector_num);
				break;
			case 3:
				center[0] = 2*XSTRIPS + detector_buffer[0]; center[1] = 0.5*YSTRIPS - detector_buffer[1];
				sprintf(optic_name, "%i", detector_num);
				break;
			case 4:
				center[0] = XSTRIPS; center[1] = 0.0 - detector_buffer[1];
				sprintf(optic_name, "%i", detector_num);
				break;
			case 5:
				center[0] = 0 - detector_buffer[0]; center[1] = 0.5*YSTRIPS - detector_buffer[1];
				sprintf(optic_name, "%i", detector_num);
				break;
			case 0:
				center[0] = 0 - detector_buffer[0]; center[1] = 1.5*YSTRIPS + detector_buffer[1];
				sprintf(optic_name, "0");
				break;
			default:
				break;
		}
		center[0] = center[0] + 0.5*XSTRIPS + border_buffer;
		center[1] = center[1] + 0.5*YSTRIPS + border_buffer;
		
		glLoadIdentity();
		glTranslatef(center[0] + detector_buffer[0],center[1] + detector_buffer[1], 0.0f); 
		glScaled(1, -1, 0);
		// rotate by 90 CW and flip to correct image based on lead images
		glRotatef(detector_angle[detector_num]+90,0.0f,0.0f,1.0f);
		// glScaled(1, 1, 1);
		glScaled(-1, -1, 1);
		
		// draw the border around the detector
		glColor3f(1, 1, 1);
		
		glBegin(GL_LINE_LOOP);
		glVertex2f(- 0.5*XSTRIPS, - 0.5*XSTRIPS); 
		glVertex2f(XSTRIPS - 0.5*XSTRIPS, - 0.5*XSTRIPS); 
		glVertex2f(XSTRIPS - 0.5*XSTRIPS,  YSTRIPS - 0.5*XSTRIPS); 
		glVertex2f(- 0.5*XSTRIPS, YSTRIPS - 0.5*XSTRIPS);
		glEnd();
		
		// draw colored line for p-side
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex2f(- 0.5*XSTRIPS, - 0.5*XSTRIPS); 
		glVertex2f(XSTRIPS - 0.5*XSTRIPS, - 0.5*XSTRIPS);
		glEnd();
		
		// draw colored line for n-side
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glVertex2f(- 0.5*XSTRIPS, - 0.5*XSTRIPS); 
		glVertex2f(- 0.5*XSTRIPS, YSTRIPS - 0.5*XSTRIPS);
		glEnd();
		
		
		current_time = clock();
		
		// draw the data in the detector
		for(int i = 0; i < XSTRIPS; i++)
		{
			for(int j = 0; j < YSTRIPS; j++)
			{
				grey = image[i][j][detector_num]/ymax[detector_num];
												
				if (grey != 0)
				{
					pixel_time = imagetime[i][j][detector_num];
					if (half_life != 0){
						elapsed_time = ((double) current_time - pixel_time)/(CLOCKS_PER_SEC);
						alpha = exp(-elapsed_time*0.693/half_life);
					} else {alpha = 1.0;}
					
					glColor4f(grey, grey, grey, alpha);
					glBegin(GL_QUADS);
					glVertex2f(i - 0.5*XSTRIPS, j - 0.5*XSTRIPS); glVertex2f(i+1 - 0.5*XSTRIPS, j- 0.5*XSTRIPS); 
					glVertex2f(i + 1 - 0.5*XSTRIPS, j + 1 - 0.5*XSTRIPS); glVertex2f(i - 0.5*XSTRIPS, j + 1- 0.5*XSTRIPS);
					glEnd();
				}
			}
		}
		
		glColor3f(0.0, 1.0, 0.0);
		text_output(0.5*YSTRIPS, 0.50*XSTRIPS, optic_name);
	
		// draw inner border for center of FOV
		glColor3f(0.0, 0.5, 0.0);
		glLineStipple(1, 0x3F07);
		glEnable(GL_LINE_STIPPLE);
		
		// border at 3 arcmin
		float border_factor = 3.0/16.5*0.5;
		glBegin(GL_LINE_LOOP);
		glVertex2f(- border_factor*XSTRIPS, - border_factor*XSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS, - border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS,  - border_factor*YSTRIPS + 2*border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS, - border_factor*YSTRIPS + 2*border_factor*YSTRIPS);
		glEnd();
		
		// border at 3 arcmin
		border_factor = 6.0/16.5*0.5;
		glBegin(GL_LINE_LOOP);
		glVertex2f(- border_factor*XSTRIPS, - border_factor*XSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS, - border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS,  - border_factor*YSTRIPS + 2*border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS, - border_factor*YSTRIPS + 2*border_factor*YSTRIPS);
		glEnd();
		
		// border at 3 arcmin
		border_factor = 9.0/16.5*0.5;
		glBegin(GL_LINE_LOOP);
		glVertex2f(- border_factor*XSTRIPS, - border_factor*XSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS, - border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS + 2*border_factor*XSTRIPS,  - border_factor*YSTRIPS + 2*border_factor*YSTRIPS); 
		glVertex2f(- border_factor*XSTRIPS, - border_factor*YSTRIPS + 2*border_factor*YSTRIPS);
		glEnd();
		
		glDisable(GL_LINE_STIPPLE);
		
		
	}
	
	//flush_mask(7);
	
	
	glPopMatrix();
	glFinish();
}

int detectorsImage::handle(int eventType)
{
	int button;
	char text[8];
	
	button=Fl::event_button();
	
	//convert between fltk coordinates and openGL coordinates
	GL_cursor[0]=Fl::event_x()*(XSTRIPS*NUM_DETECTORS)/w();
	GL_cursor[1]=(h()-Fl::event_y())*(YSTRIPS + 2.0*YBORDER)/h();
	
	//translate to pixel number but keep within bounds 
	mousePixel[0] = GL_cursor[0];
	mousePixel[1] = GL_cursor[1];
	
	//printf("mousepixel = [%i, %i]\n", mousePixel[0], mousePixel[1]);
	
	// glRasterPos2f(x, y);
	//      //find the length of the string
	//      len = (int) strlen(string);
	//      //print each character individually
	//      for ( i = 0; i < len; i++){ glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
	//      }
	redraw();
	return(1);
}

void detectorsImage::flush_image(int detector_number){
	if( (detector_number < NUM_DETECTORS) && (detector_number >= 0) ){
		for(int i = 0; i < XSTRIPS; i++)
		{
			for(int j = 0; j < YSTRIPS; j++)
			{
				image[i][j][detector_number] = 0;
				mask[i][j][detector_number] = 0;
			}
		}
	}
	if (detector_number == 7) {
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			for(int i = 0; i < XSTRIPS; i++)
			{
				for(int j = 0; j < YSTRIPS; j++)
				{
					image[i][j][detector_num] = 0;
					mask[i][j][detector_num] = 0;
				}
			}
		}
	}
}

void detectorsImage::flush_mask(int detector_number){
	if( (detector_number < NUM_DETECTORS) && (detector_number >= 0) ){
		for(int i = 0; i < XSTRIPS; i++)
		{
			for(int j = 0; j < YSTRIPS; j++)
			{
				mask[i][j][detector_number] = 0;
			}
		}
	}
	if (detector_number == 7) {
		for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
			for(int i = 0; i < XSTRIPS; i++)
			{
				for(int j = 0; j < YSTRIPS; j++)
				{
					mask[i][j][detector_num] = 0;
				}
			}
		}
	}
}

void detectorsImage::add_count_to_image(int x, int y, int detector_number){
	if( (x < XSTRIPS) && (x >= 0) && (y >= 0) && (y < YSTRIPS) && (detector_number < NUM_DETECTORS) && (detector_number >= 0) )
	{
		image[x][y][detector_number] += 1;
		imagetime[x][y][detector_number] = clock();
	}
}

void detectorsImage::add_count_to_mask(int x, int y, int detector_number){
	if( (x < XSTRIPS) && (x >= 0) && (y >= 0) && (y < YSTRIPS) && (detector_number < NUM_DETECTORS) && (detector_number >= 0) )
	{mask[x][y][detector_number] = 1;}
}

void detectorsImage::set_ymax(double value)
{
	for (int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++) {
		ymax[detector_num] = value;
	}
}

long detectorsImage::get_ymax(int detector_number)
{
	if ((detector_number < NUM_DETECTORS) && (detector_number >= 0)) {
		return ymax[detector_number];
	} else {
		return 1;
	}
}

void detectorsImage::text_output(int x, int y, char *string)
{
	int len, i;
	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
	}
}

long detectorsImage::get_image(int x, int y, int detector_number){
	if ((x < XSTRIPS) && (x > 0) && (y < YSTRIPS) && (y > 0) && (detector_number >= 0) && (detector_number < NUM_DETECTORS)){
		return image[x][y][detector_number];
	} else {
		return 0;
	}
}

unsigned int detectorsImage::get_mask(int x, int y, int detector_number){
	if ((x < XSTRIPS) && (x > 0) && (y < YSTRIPS) && (y > 0) && (detector_number >= 0) && (detector_number < NUM_DETECTORS)){
		return mask[x][y][detector_number];
	} else {
		return 0;
	}
}

long detectorsImage::get_imagetime(int x, int y, int detector_number){
	if ((x < XSTRIPS) && (x > 0) && (y < YSTRIPS) && (y > 0) && (detector_number >= 0) && (detector_number < NUM_DETECTORS)){
		return imagetime[x][y][detector_number];
	} else {
		return 0;
	}
}

void detectorsImage::save(void)
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
			
			fprintf(file, "FOXSI Image\n");
			fprintf(file, "Created %s\n", stringtemp);
			fprintf(file, "128 x 128\n");
			fprintf(file, "-----------\n");
			
			for (int j=0; j<YSTRIPS; j++) {
				for (int i=0; i<XSTRIPS; i++) 
				{	
					fprintf(file, "%d, ", image[i][j][7]);
				}
				fprintf(file, "\n");
			}
			
			fclose(file);
		} else {
			gui->app->print_to_console("Could not open file\n");
		}
    }
}