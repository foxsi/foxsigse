/*
 *  subImage.cxx
 *  Untitled
 *
 *  Created by Steven Christe on 7/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui.h"
#include "subImage.h"

#define XSTRIPS 128
#define YSTRIPS 128
#define	XBORDER 0
#define YBORDER 0
#define ZOOMNUM 5

extern Gui *gui;
extern float detImage[XSTRIPS][YSTRIPS];
extern int mousePixel[2];
extern int chosenPixel[2];
int subImageMousePixel[2];
int subImageChosenPixel[2];

float GL_subImageCursor[2]; 

subImage::subImage(int x,int y,int w,int h,const char *l)
: Fl_Gl_Window(x,y,w,h,l)
{
	
}

// the drawing method: draws the histFunc into the window
void subImage::draw() 
{
	float grey;
	double alpha;
	
	if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0,0,w(),h());	
   	glOrtho(0,ZOOMNUM+2*XBORDER,0,ZOOMNUM+2*YBORDER,0,-1);
   	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT);  
	
	//draw the pixels
	if(gui->Lockbut->value() == 0)
	{
		for(int j=0;j<ZOOMNUM;j++)
		{
			for(int i=0;i<ZOOMNUM;i++)
			{
				int curPixel[2];
				curPixel[0] = mousePixel[0] + i - ZOOMNUM/2;
				curPixel[1] = mousePixel[1] + j - ZOOMNUM/2;
				//if ((curPixel[0] < 0)||(curPixel[1] < 0)){ grey = 0.0; } else { grey = detImage[curPixel[0]][curPixel[1]]; }
				//grey = detImage[curPixel[0]][curPixel[1]];
				//grey = detsubImage[i][j];
				//alpha = detsubImagealpha[i][j];
				//grey = detImage[i][j];
				glColor4f(grey, grey, grey,alpha);
				glBegin(GL_QUADS);
				glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
				glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
				glEnd();
			}
		}
	} else
	{
		for(int j=0;j<ZOOMNUM;j++)
		{
			for(int i=0;i<ZOOMNUM;i++)
			{
				//grey = detsubImage[i][j];
				//alpha = detsubImagealpha[i][j];
				glColor4f(grey, grey, grey,alpha);
				glBegin(GL_QUADS);
				glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
				glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
				glEnd();
			}
		}
		
		//draw a red box under the cursor selection
		glColor3f(1, 0, 0);	//red
		glBegin(GL_LINE_LOOP);
		glVertex2f(GL_subImageCursor[0], GL_subImageCursor[1]); glVertex2f(GL_subImageCursor[0]+1, GL_subImageCursor[1]); 
		glVertex2f(GL_subImageCursor[0]+1, GL_subImageCursor[1]+1); glVertex2f(GL_subImageCursor[0], GL_subImageCursor[1]+1);
		glEnd();
	}
}

int subImage::handle(int eventType)
{
	/* int button;
	char text[8];

	button=Fl::event_button();

	//convert between fltk coordinates and opengl coordinates
	GL_subImageCursor[0]=floor(Fl::event_x()*(ZOOMNUM + 2*XBORDER)/w());
	GL_subImageCursor[1]=floor((h()-Fl::event_y())*(ZOOMNUM + 2*YBORDER)/h());

	//convert between fltk coordinates and openGL coordinates
	//GL_cursor[0]=Fl::event_x()*(XSTRIPS + 2.0*XBORDER)/w();
	//GL_cursor[1]=(h()-Fl::event_y())*(YSTRIPS + 2.0*YBORDER)/h();

	//translate to pixel number but keep within bounds 
	//subImageMousePixel[0] = (GL_subImageCursor[0] - XBORDER) > 1 ? chosenPixel[0] - XBORDER : 1;
	//subImageMousePixel[0] = subImageMousePixel[0] < ZOOMNUM ? subImageMousePixel[0]: ZOOMNUM;

	//subImageMousePixel[1] = (GL_subImageCursor[1] - YBORDER) > 1 ? chosenPixel[1] - YBORDER : 1;
	//subImageMousePixel[1] = subImageMousePixel[1] < ZOOMNUM ? subImageMousePixel[1]: ZOOMNUM;

	subImageMousePixel[0] = GL_subImageCursor[0] - XBORDER + chosenPixel[0]-ZOOMNUM/2;
	subImageMousePixel[1] = GL_subImageCursor[1] - YBORDER + chosenPixel[1]-ZOOMNUM/2;
	//printf("pixel: (%4.2f,%4.2f)\n", FLcursorX[0] - XBORDER+1, FLcursorY[0]-YBORDER+1);

	if((eventType==FL_PUSH)&&(button==1))
	{
		subImageChosenPixel[0] = subImageMousePixel[0]; subImageChosenPixel[1] = subImageMousePixel[1];
	}

	//update the text info
	sprintf( text, "%d,%d", subImageMousePixel[0], subImageMousePixel[1]);
	gui->pixelNum->value(text);
	//sprintf( text, "%4.2f", detImage[subImageMousePixel[0]][subImageMousePixel[1]]);
		sprintf( text, "%4.2f", detImage[subImageMousePixel[0]][subImageMousePixel[1]]);
	gui->pixelCounts->value(text);

	redraw(); */
	
	
	return(1);
}

