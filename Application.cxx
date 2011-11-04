// application class

#include "Application.h"
#include <FL/Fl_File_Chooser.H>

#include "data.h"
#include "commands.h"
#include "gui.h"

#define XSTRIPS 128
#define YSTRIPS 128
#define CHANNELS 1024
#define MAXPATH 128

extern Gui *gui;

extern int HistogramFunction[CHANNELS];
extern double detImage[XSTRIPS][YSTRIPS];

extern int stop_message;
extern FILE *dataFile;
extern int fout;

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
int data_source;
float pixel_half_life;

Application::Application()
{
	// Constructor method for the Application class
	// Add initialization here:

}

void Application::FlushData(void)
{	
	// Zeros all the data displays
	//
	gui->data->FlushHistogram();
	gui->data->FlushImage();
}

void Application::save_preferences(void)
{
	gui->prefs->set("pixel_half_life", (float) gui->pixelhalflife_value->value());
	gui->prefs->set("file_type", gui->fileTypeChoice->value());
	gui->prefs->set("data_file_save_dir", gui->datafilesavedir_fileInput->value());
	gui->prefs->set("read_delay", gui->readdelay_value->value());
	gui->prefs->set("data_source", gui->DataSource_choice->value());
}

void Application::read_preferences(void)
{
	// Read the preferences
	gui->prefs->get("pixel_half_life", pixel_half_life,3.0);
	gui->prefs->get("file_type", file_type, 0);
	gui->prefs->get("data_file_save_dir", data_file_save_dir, "/Users/schriste/");
	gui->prefs->get("read_delay", read_delay, 10000);
	gui->prefs->get("data_source", data_source, 0);
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
	printf_to_console("Output directory set to %s.\n", data_file_save_dir);	
}

void Application::start_file()
{
	data_start_file();
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
	
	//gui->data->readDatafile(file);
}

// add application routines here:

//void Application::readUSBStream(void)
//{
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
	
//	if (gui->usb->open() < 0) {
//		cout << "Could not open device.\n\n";
//	}
//	gui->usb->readFrame();
//	gui->usb->printFrame();
//	gui->usb->close();
//}

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
	print_to_console("Reading begun...\n");
	gui->stopReadingDataButton->activate();
	data_start_reading();	
}

void Application::printf_to_console(const char *text, char *string1)
{
	char buffer[200];
	sprintf(buffer, text, string1);
	gui->consoleBuf->insert(buffer);
}

void Application::print_to_console(const char *text)
{
	gui->consoleBuf->insert(text);
}

void* Application::new_read_data()
{
//	unsigned int i;
//	long len;
//	ssize_t wlen;
//	unsigned short int status;
//	unsigned short int buffer0[1024],buffer1[1024];
//
//	while (1) 
//	{
//		// alternate between reading data into buffer0 and buffer1
//		// copy it into buffer with memcopy
//		
//		len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer0);
//		
//		if(fout > 0)
//		{
//			if( (wlen = write(fout,(const void *) buffer0,2048) ) != 2048){};
//		}
//		
//		if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
//		{
//			if(newdisplay == 0)
//			{
//				memcpy((void *) buffer,(void *) buffer0,2048);
//				newdisplay = 1;
//			}
//			pthread_mutex_unlock(&mymutex);
//			
//		}
//		
//		len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer1);
//		
//		if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
//		{
//			memcpy((void *) buffer,(void *) buffer1,2048);
//			pthread_mutex_unlock(&mymutex);
//		}
//		
//		if(fout >0)
//		{
//			if( (wlen = write(fout,(const void *) buffer1,2048) ) != 2048){};
//		}
//	}
	return 0;
}

void* Application::read_data(void* variable)
{
	char buffer[50];
	int badSync = 0;
	int badRead = 0;
	int status = 0;
	char *tmp[1];
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
	
	for (int i = 0; i<gui->nEvents->value(); i++) {
		// get the read delay from the preferences
		int read_delay;
		gui->prefs->get("read_delay", read_delay, 10000);

		usleep(read_delay);		// adjust this to achieve desired reading speed
				
		// check to see if the Stop button was pushed, if so then clean up 
		// and stop this thread
		if (stop_message == 1){
			// clean up code goes here then exit
			if (gui->writeFileBut->value() == 1){
				fclose(dataFile);
				
				Fl::lock();
				print_to_console("Read force stopped!\n");
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



void Application::clear_console(void)
{
	// clear the text in the console
	gui->buff->select(0, gui->buff->length());
	gui->buff->remove_selection();
}
	
void Application::test(void)
{
	print_to_console("Reading begun...\n");
	gui->stopReadingDataButton->activate();
	data_start_reading();	
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
	gui->stopReadingDataButton->deactivate();
	gui->startReadingDataButton->activate();

	// send the thread the message to stop itself	
}