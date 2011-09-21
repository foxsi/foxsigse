
#include "mainImage.h"
#include "gui.h"

#define XSTRIPS 128
#define YSTRIPS 128
#define	XBORDER 5
#define YBORDER 5
#define ZOOMNUM 5

//todo
//need to auto scale the image, add scalling types (auto, set/linear, logarithmic)
//this needs maximumValue function, add in a functions.cxx file

//each pixel is exactly 1x1 in GL coordinates to make it easier

double detImage[XSTRIPS][YSTRIPS];
double detImagemask[XSTRIPS][YSTRIPS];
double ymax;
float GL_cursor[2];
int mousePixel[2];
int chosenPixel[2];
extern float detsubImage[ZOOMNUM][ZOOMNUM];

extern Gui *gui;
extern Foxsidata *data;

mainImage::mainImage(int x,int y,int w,int h,const char *l)
        : Fl_Gl_Window(x,y,w,h,l)
{
	/* initialize random seed: */
	srand ( time(NULL) );
	
	//initialize the image
	for(int i=0;i<XSTRIPS;i++)
	{
		for(int j=0;j<YSTRIPS;j++)
	   	{
			detImage[i][j] = 0;
		}
	}
	detImage[15][15] = 1.0;
	detImage[75][75] = 0.5;
	//detImage = FOXSIdata->getImage();
}
// the drawing method: draws the histFunc into the window
void mainImage::draw() 
{
	double grey;
	double bleu;

	ymax = maximumValue(*detImage);
	
	//int cursorBox = 2;
		if (!valid()) {
		make_current();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0,0,w(),h());	
   	glOrtho(0,XSTRIPS+2*XBORDER,0,YSTRIPS+2*YBORDER,0,-1);
   	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
   	glPushMatrix();
   	glLoadIdentity();
   	glClearColor(0.0,0.0,0.0,0.0);
   	glClear(GL_COLOR_BUFFER_BIT);  
	
	for(int j=0;j<XSTRIPS;j++)
	{
		for(int i=0;i<YSTRIPS;i++)
	   	{
			grey = detImage[i][j]/ymax;
			glColor3f(grey, grey, grey);
			glBegin(GL_QUADS);
				glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
				glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
			glEnd();
			
			//now draw the mask
			bleu = detImagemask[i][j];
			if (bleu == 1){
				glColor3f(0, 0, bleu);
			glBegin(GL_QUADS);
			glVertex2f(i+XBORDER, j+YBORDER); glVertex2f(i+1+XBORDER, j+YBORDER); 
			glVertex2f(i+1+XBORDER, j+1+YBORDER); glVertex2f(i+XBORDER, j+1+YBORDER);
			glEnd();}
	   	}
	}
	//draw a border around the detector
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
		glVertex2f(XBORDER, YBORDER); glVertex2f(XBORDER+XSTRIPS, YBORDER); 
		glVertex2f(XBORDER+XSTRIPS, YBORDER+YSTRIPS); glVertex2f(XBORDER, YBORDER+YSTRIPS);
	glEnd();
	
	//draw a red box under the cursor selection
	//glColor3f(1, 0, 0);	//red
	//glBegin(GL_LINE_LOOP);
	//	glVertex2f(mousePixel[0] + XBORDER, mousePixel[1] + YBORDER); 
	//	glVertex2f(mousePixel[0]+1 + XBORDER, mousePixel[1] + YBORDER); 
	//	glVertex2f(mousePixel[0]+1 + XBORDER, mousePixel[1]+1 + YBORDER); 
	//	glVertex2f(mousePixel[0] + XBORDER, mousePixel[1]+1 + YBORDER);
	//glEnd();

	
	//draw an expanded view
	/*if ((FLcursorX[1]!=0)&&(FLcursorY[1]!=0))
	{
		for(int j=0;j<zoomNum;j++)
		{
			for(int i=0;i<zoomNum;i++)
			{
				int curPixel[2];
				
				curPixel[0] = chosenPixel[0] + i - zoomNum/2;
				curPixel[1] = chosenPixel[1] + j - zoomNum/2;
				if ((curPixel[0] < 0)||(curPixel[1] < 0)){ grey = 0.0; } else { grey = detImage[curPixel[0]][curPixel[1]]; }
				
				glColor3f(grey, grey, grey);
				glBegin(GL_QUADS);
					glVertex2f(FLcursorX[1]+i*zoomPixSize - zoomNum/2*zoomPixSize, FLcursorY[1]+j*zoomPixSize - zoomNum/2*zoomPixSize); 
					glVertex2f(FLcursorX[1]+(i+1)*zoomPixSize - zoomNum/2*zoomPixSize, FLcursorY[1]+j*zoomPixSize - zoomNum/2*zoomPixSize); 
					glVertex2f(FLcursorX[1]+(i+1)*zoomPixSize - zoomNum/2*zoomPixSize, FLcursorY[1]+(j+1)*zoomPixSize - zoomNum/2*zoomPixSize); 
					glVertex2f(FLcursorX[1]+i*zoomPixSize - zoomNum/2*zoomPixSize, FLcursorY[1]+(j+1)*zoomPixSize - zoomNum/2*zoomPixSize);
				glEnd();
			}
		}
	}
	 */
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

	//printf("pixel: (%d,%d)\n", Fl::event_x(), Fl::event_y());
	
	if((eventType==FL_PUSH)&&(button==1))
	{
		//set the view lock button to ON
		gui->subImageLockbut->set();
		//save the location
		chosenPixel[0] = mousePixel[0];	chosenPixel[1] = mousePixel[1];

	}
	
		//save the current view 
		for(int j=0;j<ZOOMNUM;j++)
		{
			for(int i=0;i<ZOOMNUM;i++)
			{
				int curPixel[2];

				curPixel[0] = mousePixel[0] + i - ZOOMNUM/2;
				curPixel[1] = mousePixel[1] + j - ZOOMNUM/2;
				if ((curPixel[0] < 0)||(curPixel[1] < 0)||(curPixel[0] > XSTRIPS)||(curPixel[1] > YSTRIPS)){ detsubImage[i][j] = 0.0; } else { detsubImage[i][j] = detImage[curPixel[0]][curPixel[1]]; }
			}
		}

	
	//now update the text box with the current chosen pixel
	if(gui->subImageLockbut->value() == 0)
	{
		sprintf( text, "%d,%d", mousePixel[0], mousePixel[1]);
		gui->pixelNum->value(text);
		sprintf( text, "%4.2f", detImage[mousePixel[0]][mousePixel[1]]);
		gui->pixelCounts->value(text);
	}
	
	//redraw();
	if(gui->subImageLockbut->value() == 0){gui->subImageWindow->redraw();}
	return(1);
}

double mainImage::maximumValue(double *array)
{
	//float length = sizeof(array)/sizeof(array[0]);  // establish size of array
	//cout << "size of array " << length << endl;
	//do not consider below xmin
	int max = array[0];       // start with max = first element
	for(int i = 1; i<XSTRIPS*YSTRIPS; i++)
	{
		if(array[i] > max){
			max = array[i];}
	}
	
	return max;                // return highest value in array
}
