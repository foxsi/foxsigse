// application class

#include "Application.h"
#include <FL/Fl_File_Chooser.H>
#include "data.h"
#include "commands.h"
#include "gui.h"
#include "Foxsidata.h"
#include "usbd2xx.h"
#include <pthread.h>
#include <sched.h>

#include <sys/time.h>
#include <time.h>


#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024
#define MAXPATH 128

extern Gui *gui;

extern int HistogramFunction[MAX_CHANNEL];
extern double detImage[XSTRIPS][YSTRIPS];
extern double detImagemask[XSTRIPS][YSTRIPS];

extern int stop_message;
extern FILE *dataFile;
extern int fout;
extern int nreads;

// filename is set automatically with local time
extern char dataFilename[MAXPATH];
extern char dataFileDir[MAXPATH];

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

// Preference variables
int file_type;
char *data_file_save_dir;
int read_delay;
int data_source;	// 0 means simulation, 1 means ASIC, 2 means Formatter
float pixel_half_life;
int mainImage_minimum;
int detector_display[7];

extern int low_threshold;

Application::Application()
{
	// Constructor method for the Application class
	// Add initialization here:
	
}

void Application::flush_histogram(void)
{	
	// Zero the Histogram

	for(int i = 0;i < MAX_CHANNEL; i++)
	{
		HistogramFunction[i] = 0;
	}
	gui->mainHistogramWindow->redraw();
	
}

void Application::flush_image(void)
{
	for(int i=0;i<XSTRIPS;i++)
	{
		for(int j=0;j<YSTRIPS;j++)
		{
			detImage[i][j] = 0;
			detImagemask[i][j] = 0;
		}
	}
	gui->mainImageWindow->redraw();
	gui->subImageWindow->redraw();
}

void Application::save_preferences(void)
{
	gui->prefs->set("pixel_half_life", (float) gui->pixelhalflife_value->value());
	gui->prefs->set("file_type", gui->fileTypeChoice->value());
	gui->prefs->set("data_file_save_dir", gui->datafilesavedir_fileInput->value());
	gui->prefs->set("read_delay", gui->readdelay_value->value());
	gui->prefs->set("data_source", gui->DataSource_choice->value());
	gui->prefs->set("mainImage_minimum", gui->mainImageMin_slider->value());
}

void Application::read_preferences(void)
{
	// Read the preferences
	gui->prefs->get("pixel_half_life", pixel_half_life,3.0);
	gui->prefs->get("file_type", file_type, 0);
	gui->prefs->get("data_file_save_dir", data_file_save_dir, "/Users/schriste/");
	gui->prefs->get("read_delay", read_delay, 10000);
	gui->prefs->get("data_source", data_source, 0);
	
	gui->prefs->get("mainImage_minimum", mainImage_minimum, 0);
}

void Application::update_preferencewindow(void)
{	
	// Update them in the Preferences window
	gui->pixelhalflife_value->value(pixel_half_life);
	gui->fileTypeChoice->value(file_type);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	gui->readdelay_value->value(read_delay);
	gui->DataSource_choice->value(data_source);	
}

void Application::set_datafile_dir(void)
{
	char *temp = fl_dir_chooser("Pick a directory:", "", 0);
	strcpy(data_file_save_dir, temp);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	printf_to_console("Output directory set to %s.\n", data_file_save_dir, NULL);	
}

void Application::start_file()
{
	data_start_file();
}

void Application::write_header(FILE *file)
{
	gui->usb->writeHeader(file);
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
	flush_image();
	flush_histogram();
	
	//gui->data->readDatafile(file);
}
 
void Application::initialize_data(void)
{
	// Initialize a connection to a data stream
	if (gui->usb->open() < 0)	cout << "Could not open device.\n\n";
//		else	gui->usb->findSync();
	
	//gui->syncLightBut->value(1);
	gui->initializeBut->value(1);
	gui->closeBut->value(0);
	
	gui->startReadingDataButton->activate();
	gui->sendParamsWindow_sendBut->activate();
	gui->setHoldTimeWindow_setBut->activate();
	gui->setHoldTimeWindow_autorunBut->activate();
	flush_image();
	flush_histogram();
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
	print_to_console("Reading started.\n");
	gui->stopReadingDataButton->activate();
	gui->startReadingDataButton->deactivate();
	
	data_start_reading();	
}

void Application::printf_to_console(const char *text, char *string1, int number)
{
	char buffer[200];
	sprintf(buffer, text, string1);
	gui->consoleBuf->insert(buffer);
	gui->consoleBuf->show_insert_position();
}
								   
void Application::print_to_console(const char *text)
{
	gui->consoleBuf->insert(text);
	gui->consoleBuf->show_insert_position();
}

/*
void* Application::read_data(void* variable)
{
	char buffer[50];
	int badSync = 0;
	int badRead = 0;
	int status = 0;
	
	Fl::lock();
	gui->stopReadingDataButton->activate();
//	gui->stopReadingDataButton->value(0);
	gui->startReadingDataButton->deactivate();
	Fl::unlock();
	
	// data file should be open as long as the "Write to file" has been clicked
	//if (gui->writeFileBut->value() == 1){
	// Open data file
	//dataFile = fopen(gui->filenameInput->value(), "a+");	// later, change this to an option
	//	if(dataFile == NULL){
	//		Fl::lock();
	//		sprintf(buffer, "Invalid filename.\n");
	//		gui->consoleBuf->insert(buffer);
	//		Fl::unlock();
	//		return NULL;
	//	}
	//}
	
	Fl::lock();
	print_to_console("Reading...\n");
	Fl::unlock();
	
	int i = 0;
	int nEnd = gui->nEvents->value(); 
	while ( i<nEnd ){
//	for (int i = 0; i<gui->nEvents->value(); i++) {
		usleep(20000);		// adjust this to achieve desired reading speed
				
		// check to see if the Stop button was pushed, if so then clean up 
		// and stop this thread
		if (stop_message){
			// clean up code goes here then exit
			Fl::lock();
			sprintf(buffer, "Read Stopped.\n");
			gui->consoleBuf->insert(buffer);
			sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
			gui->consoleBuf->insert(buffer);
			gui->stopReadingDataButton->deactivate();
//			gui->stopReadingDataButton->value(0);
			gui->startReadingDataButton->activate();
			if (gui->writeFileBut->value() == 1){
				fclose(dataFile);
				gui->writeFileBut->activate();
				
			}
			Fl::unlock();	
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
		if (gui->writeFileBut->value() == 1)  gui->usb->writeFrame(dataFile, i);
		gui->nEventsDone->value(i);
		Fl::unlock();
		Fl::awake();
		
		i++;
	}
	
	Fl::lock();
	sprintf(buffer, "Read finished.\n");
	gui->consoleBuf->insert(buffer);
	sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
	gui->consoleBuf->insert(buffer);		
	if (gui->writeFileBut->value() == 1){
		fclose(dataFile);
		gui->writeFileBut->value(0);
	}
	gui->stopReadingDataButton->deactivate();
//	gui->stopReadingDataButton->value(0);
	gui->startReadingDataButton->activate();
	Fl::unlock();	

	return 0;
}
 */

void Application::openSendParamsWindow(void)
{
	gui->sendParamsWindow->show();
}

void Application::openSetHoldTimeWindow(void)
{
	gui->setHoldTimeWindow->show();
}

void Application::openSetTrigWindow(void)
{
	gui->setTrigWindow->show();
}

void Application::send_params(void)
{	
	gui->usb->setConfig();	
	gui->consoleBuf->insert("Sending Params to controller.\n");
}

void Application::send_global_params(int option)
{
	gui->usb->setGlobalConfig(option);
	gui->consoleBuf->insert("Sending global Params to controller.\n");
}

void Application::start_auto_run(void)
{
	print_to_console("Can't autorun; disabled.\n");

	/*
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
	
	ret = pthread_create(&thread, &tattr, auto_run_sequence, (void *) variable);
	*/
}

void* Application::auto_run_sequence(void* variable)
{
	/*
	for(int i=1; i<32; i++){
		Fl::lock();
		gui->nEventsDone->value(0);
		gui->setHoldTimeWindow_holdTime->value(i);
		Fl::unlock();
		gui->app->send_global_params(0);
		gui->app->send_global_params(0);
		gui->writeFileBut->value(1);
		gui->app->start_file();
		gui->app->start_reading_data();
		
		while(1){

			Fl::lock();
			int nCtr  = gui->nEventsDone->value();
			int nEvts = gui->nEvents->value();
			Fl::unlock();

			if(nCtr == (nEvts-1) ) break;
			sleep(1);			
		}
		
	}	
	return 0;
	*/	
	return 0;
}


//void Application::break_acq(int data)
//{
//	gui->usb->breakAcq(data);
//}

void Application::save_settings(void)
{
	gui->usb->saveSettings();
}

void Application::restore_settings(void)
{
	gui->usb->restoreSettings();
}

void Application::clear_console(void)
{
	// clear the text in the console
	gui->buff->select(0, gui->buff->length());
	gui->buff->remove_selection();
}
	
void Application::set_lowthreshold(void)
{
	low_threshold = gui->mainImageMin_slider->value();
	gui->histLow->value(low_threshold);
	gui->mainHistogramWindow->redraw();
	gui->histLow->redraw();
}

void Application::test(void)
{
}

void Application::send_atten_state(bool value)
{
	command_attenuator_state(value);
}

void Application::send_voltage_command(void)
{
	command_voltage_set(gui->highVoltage_input->value());
}

void Application::send_clockset_command(void)
{
	command_clock_set(gui->clockHigh_input->value(), gui->clockLow_input->value());
}

void Application::stop_reading_data(void)
{
	data_stop_reading();
	
	Fl::lock();
	gui->stopReadingDataButton->deactivate();
	gui->startReadingDataButton->activate();
	Fl::unlock();
	
	// send the thread the message to stop itself	
}

void Application::reset_read_counter(void)
{
	nreads = 0;
	gui->nEventsDone->value(0);
}
