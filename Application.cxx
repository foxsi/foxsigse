// application class

#include "Application.h"
#include <FL/Fl_File_Chooser.H>
#include "data.h"
#include "commands.h"
#include "gui.h"
#include "Foxsidata.h"
#include "usbd2xx.h"
#include "mainLightcurve.h"
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
unsigned int CountcurveFunction[MAX_CHANNEL];
double displayHistogram[MAX_CHANNEL];

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
int newFPGA_register;
char *formatter_configuration_file;

extern int low_threshold;
extern int mainHistogram_binsize;

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
		displayHistogram[i] = 0;
	}
	gui->mainHistogramWindow->redraw();
}

void Application::flush_timeseries(void)
{	
	// Zero the time series
	for(int i = 0;i < MAX_CHANNEL; i++)
	{
		CountcurveFunction[i] = 0;
	}
	gui->mainLightcurveWindow->redraw();
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
	//gui->subImageWindow->redraw();
}

void Application::save_preferences(void)
{
	gui->prefs->set("pixel_half_life", (float) gui->pixelhalflife_value->value());
	gui->prefs->set("file_type", gui->fileTypeChoice->value());
	gui->prefs->set("data_file_save_dir", gui->datafilesavedir_fileInput->value());
	gui->prefs->set("read_delay", gui->readdelay_value->value());
	gui->prefs->set("data_source", gui->DataSource_choice->value());
	gui->prefs->set("mainImage_minimum", gui->mainImageMin_slider->value());
	gui->prefs->set("newFPGA_register", gui->newControlRegisters_check->value());
	gui->prefs->set("formatter_configuration_file", gui->gsesyncfile_fileInput->value());
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
	gui->prefs->get("newFPGA_register", newFPGA_register,0);
	gui->prefs->get("formatter_configuration_file", formatter_configuration_file, "/Users/schriste/");
}

void Application::update_preferencewindow(void)
{	
	// Update them in the Preferences window
	gui->pixelhalflife_value->value(pixel_half_life);
	gui->fileTypeChoice->value(file_type);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	gui->readdelay_value->value(read_delay);
	gui->DataSource_choice->value(data_source);
	gui->newControlRegisters_check->value(newFPGA_register);
	gui->gsesyncfile_fileInput->value(formatter_configuration_file);
}

void Application::set_datafile_dir(void)
{
	char *temp = fl_dir_chooser("Pick a directory:", "", 0);
	strcpy(data_file_save_dir, temp);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	printf_to_console("Output directory set to %s.\n", data_file_save_dir, NULL);	
}

void Application::set_gsesync_file(void)
{
	char *temp = fl_file_chooser("Pick gsesync", "", 0);
	strcpy(formatter_configuration_file, temp);
	gui->gsesyncfile_fileInput->value(formatter_configuration_file);
	printf_to_console("Output directory set to %s.\n", formatter_configuration_file, NULL);	
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
 
void Application::initialize(void)
{
	data_initialize();
	gui->initializeBut->deactivate();
	gui->closeBut->activate();
}

void Application::close_data(void)
{
	// Close a connection to a data stream
	gui->usb->close();
	gui->initializeBut->activate();
	gui->closeBut->deactivate();
	gui->startReadingDataButton->deactivate();
}

void Application::start_reading_data(void)
{
	print_to_console("Reading started.\n");
	gui->stopReadingDataButton->activate();
	gui->closeBut->deactivate();
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
	gui->closeBut->activate();
	Fl::unlock();
	
	// send the thread the message to stop itself	
}

void Application::reset_read_counter(void)
{
	nreads = 0;
	gui->nEventsDone->value(0);
}

void Application::update_histogrambinsize(void)
{
	mainHistogram_binsize = gui->histogrambinsize_counter->value();
	gui->mainHistogramWindow->redraw();
}

void Application::update_timebinsize(void)
{
	// mainHistogram_binsize = gui->binsize_counter->value();
	gui->mainLightcurveWindow->binsize[0] = gui->timebinsize_counter->value();
}

void Application::update_lightcurvexmax(void)
{
	gui->mainLightcurveWindow->xmax = gui->lightcurvexmax_counter->value();
	gui->mainLightcurveWindow->redraw();
}

void Application::update_histogramxmax(void)
{
	gui->mainHistogramWindow->xmax = gui->histogramxmax_counter->value();
	gui->mainHistogramWindow->redraw();
}

void Application::set_lowthreshold(void)
{
	low_threshold = gui->mainImageMin_slider->value();
	gui->histLow->value(low_threshold);
	gui->mainHistogramWindow->redraw();
	gui->histLow->redraw();
}

void Application::set_energy_histogram(void)
{
	//gui->histogrambinsize_counter->value(1);
	//mainHistogram_binsize = 1;
	//gui->mainHistogramWindow->redraw();

}

void Application::set_channel_histogram(void)
{
	//gui->histogrambinsize_counter->value(25);
	//mainHistogram_binsize = 25;
	//gui->mainHistogramWindow->redraw();
}

void Application::toggle_image_integrate(void)
{
	if (gui->mainImage_integrate_button->value() == 1){
		pixel_half_life = 0;
		printf("pixel_half_life = %f\n", pixel_half_life);
		gui->mainImageWindow->redraw();
	}
	if (gui->mainImage_integrate_button->value() == 0){
		gui->prefs->get("pixel_half_life", pixel_half_life, 3.0);
		printf("pixel_half_life = %f\n", pixel_half_life);
		gui->mainImageWindow->redraw();
	}
}

void Application::save_histogram_to_file(void)
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
				fprintf(file, "%i, %i\n", i, HistogramFunction[i]);
			}
			
			fclose(file);
		} else {
			print_to_console("Could not open file\n");
		}

    }	
}

void Application::save_image_to_file(void)
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
					fprintf(file, "%f, ", detImage[i][j]);
				}
				fprintf(file, "\n");
			}
			
			fclose(file);
		} else {
			print_to_console("Could not open file\n");
		}
    }	
}

void Application::set_lightcurve_ymax(void)
{
	gui->mainLightcurveWindow->ymax = gui->mainLightcurve_ymaxslider->value();
	gui->mainLightcurveWindow->redraw();
}