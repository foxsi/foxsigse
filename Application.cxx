// application class

#include "Application.h"
#include <FL/Fl_File_Chooser.H>
#include "gui.h"
#include "Foxsidata.h"
#include "usbd2xx.h"
#include <pthread.h>
#include <sched.h>

#define XSTRIPS 128
#define YSTRIPS 128

#define CHANNELS 1024

extern Gui *gui;

extern int HistogramFunction[CHANNELS];
extern double detImage[XSTRIPS][YSTRIPS];

int stop_message;

// these are declared in transFunc.cpp
//extern HistogramFunction histFunc[4];
//extern int maxHistFunc;              
//extern int activeHistFunc;       
//extern float histFuncColor[4][3]; 
 
//variables for Application
//int imgx=0, imgy=0; //dimensions of the image
//int nchan;	//the number of color channels an image has
//char *imgname;	//name of the image
//GLubyte *imgpix;	//pixel buffer for image


// the constructor method for the Application class
Application::Application()
{
  
  // add initialization here:
  //nchan = -1;	//value not set yet, i.e. image not read
  //imgx = 0;
  //imgy = 0;
  filename[0]='\0';
}

//zero all analyzed data
void Application::FlushData(void)
{	
	gui->data->FlushHistogram();
	gui->data->FlushImage();
}
	

// the method that gets executed when the readFile callback is called
void Application::readFile()
{
	// launch file browser
	char *file = fl_file_chooser("Pick a file from this list:", "*.*", "");
	if(file == NULL)
		return;
	
	cout << "Loading File:" << file << endl;

	//store image name
	//strcpy(filename,file);
	FlushData();
	
	gui->data->readDatafile(file);
}

// add application routines here:

//quit the program
void Application::Exit()
{
	exit(0);
}

void Application::simulateData(void)
{
}

void Application::readUSBStream(void)
{
	//opens a new window
	//Fl_Double_Window *subWindow = new Fl_Double_Window(400,200, "subWindow");
	//Fl_Text_Display *textDisplay = new Fl_Text_Display(0,0,400,200, 0);
	//Fl_Text_Buffer *textBuffer = new Fl_Text_Buffer;
	//textDisplay->buffer(textBuffer);

	//subWindow->begin();
	//Fl_Widget *box = new Fl_Widget(20, 40, 260, 100, "Hello, World!");
	//box->box(UP_BOX);
	//box->labelfont(HELVETICA_BOLD_ITALIC);
	//box->labelsize(36);
	//box->labeltype(SHADOW_LABEL);
	//subWindow->end();
	//subWindow->show();
	
	if (gui->usb->open() < 0) {
		cout << "Could not open device.\n\n";
	}
	gui->usb->readFrame();
	gui->usb->printFrame();
	gui->usb->close();
}

void Application::readTeleStream(void)
{

}

void Application::WriteSpec(void)
{

}

void Application::WriteLightcurve(void)
{

}

void Application::dataSync(void)
{

}

void Application::setDetector(int detector)
{

}

float Application::getRate(int detector)
{

	switch ( detector )
      	{
        case 0:
		return 0.0;
		break;
	case 1:
        	return 1.0;
        	break;
        case 2:
		return 2.0;
		break;
	case 3:
        	return 3.0;
        	break;
        case 4:
		return 4.0;
		break;
	case 5:
        	return 5.0;
        	break;
        case 6:
		return 6.0;
		break;
	case 7:
        	return 7.0;
        	break;
        default:
        	return -1.0;
	}

}

int Application::getFrameNum(void)
{
	return 1010;
}

int Application::getShutState(void)
{
	return 1;
}

float Application::getTemp(void)
{
	return 1;
}

const char Application::getPixel(void)
{
	return '1';
}

void Application::initialize_data(void)
{
	// Initialize a connection to a data stream
	if (gui->usb->open() < 0)	cout << "Could not open device.\n\n";
//		else	gui->usb->findSync();
	
	gui->syncLightBut->value(1);
	gui->initializeBut->value(1);
	gui->closeBut->value(0);
	
	gui->chipbitValOut0->activate();
	gui->chipbitValOut1->activate();
	gui->chipbitValOut2->activate();
	gui->chipbitValOut3->activate();
	
	gui->trigbitValOut0->activate();
	gui->trigbitValOut1->activate();
	gui->trigbitValOut2->activate();
	gui->trigbitValOut3->activate();
	
	gui->seubitValOut0->activate();
	gui->seubitValOut1->activate();
	gui->seubitValOut2->activate();
	gui->seubitValOut3->activate();
	
	gui->noiseValOut0->activate();
	gui->noiseValOut1->activate();
	gui->noiseValOut2->activate();
	gui->noiseValOut3->activate();
	
	gui->startReadingDataButton->activate();
	gui->sendParamsWindow_sendBut->activate();
	FlushData();
}

void Application::close_data(void)
{
	// Close a connection to a data stream
	gui->usb->close();
	gui->initializeBut->value(0);
	gui->closeBut->value(1);
	gui->startReadingDataButton->deactivate();
}

void Application::start_reading_data(void)
{
	pthread_t thread;
    struct sched_param param;
    pthread_attr_t tattr;
	
	int *variable;
	int ret;
	
	stop_message = 0;
	
	variable = (int *) malloc(sizeof(int));
	*variable = 6;
	
	// define the custom priority for one of the threads
    int newprio = -10;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	ret = pthread_create(&thread, &tattr, read_data, (void *) variable);
	
}

void* Application::read_data(void* variable)
{
	FILE *	dataFile;
	char buffer[50];
	int badSync = 0;
	int badRead = 0;
	int status = 0;
	
	gui->stopReadingDataButton->activate();
	
	if (gui->writeFileBut->value() == 1){
		// Open data file
		dataFile = fopen(gui->filenameInput->value(), "a+");	// later, change this to an option
		if(dataFile == NULL){
			Fl::lock();
			sprintf(buffer, "Invalid filename.\n");
			gui->consoleBuf->insert(buffer);
			Fl::unlock();
			return NULL;
		}
	}

	sprintf(buffer, "Reading...\n");
	Fl::lock();
	gui->consoleBuf->insert(buffer);
	Fl::unlock();	

	for (int i = 0; i<gui->nEvents->value(); i++) {
		usleep(10000);		// adjust this to achieve desired reading speed
				
		// check to see if the Stop button was pushed, if so then clean up 
		// and stop this thread
		if (stop_message == 1){
			// clean up code goes here then exit
			if (gui->writeFileBut->value() == 1){
				fclose(dataFile);
				Fl::lock();
				sprintf(buffer, "Read Stopped.\n");
				gui->consoleBuf->insert(buffer);
				sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
				gui->consoleBuf->insert(buffer);
				gui->stopReadingDataButton->deactivate();
				Fl::unlock();	
				
			}
            pthread_exit(NULL);
		}

		status = gui->usb->findSync();
		if(status<1){
			badSync++;
			continue;
		}

		status = gui->usb->readFrame();
		if(status<1){
			badRead++;
			continue;
		}
		
		Fl::lock();
		// print frame to XCode console (unless nEvents is large, then skip it to save time).
		if(gui->nEvents->value() < 5)  gui->usb->printFrame();
		// write to file if the write button is enabled
		if (gui->writeFileBut->value() == 1)  gui->usb->writeFrame(dataFile);
		gui->nEventsDone->value(i);
		Fl::unlock();
		
	}
	
	Fl::lock();
	sprintf(buffer, "Read finished.\n");
	gui->consoleBuf->insert(buffer);
	sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
	gui->consoleBuf->insert(buffer);		
	if (gui->writeFileBut->value() == 1){
		fclose(dataFile);
	}
	gui->stopReadingDataButton->deactivate();
	Fl::unlock();	

	return 0;
}

void Application::openSendParamsWindow(void)
{
	gui->sendParamsWindow->show();
	if (gui->initializeBut->value() == 0) {
//		gui->sendParamsWindow_sendBut->deactivate();
	}
}

void Application::send_params(void)
{	
	gui->usb->setConfig();	
	gui->consoleBuf->insert("Sending Params to controller.\n");
}

void Application::send_global_params(void)
{
	gui->usb->setGlobalConfig();
	gui->consoleBuf->insert("Sending global Params to controller.\n");
}

void Application::break_acq(int data)
{
	gui->usb->breakAcq(data);

}

void Application::save_settings(void)
{
	gui->usb->saveSettings();
}

void Application::restore_settings(void)
{
	gui->usb->restoreSettings();
}

void Application::test(void)
{

}

void Application::stop_reading_data(void)
{
	// send the thread the message to stop itself
	stop_message = 1;	
}
