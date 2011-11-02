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
FILE *dataFile;
extern int fout;

// filename is set automatically with local time
char dataFilename[MAXPATH];
char dataFileDir[MAXPATH];

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


Application::Application()
{
	// Constructor method for the Application class
	// Add initialization here:
  
	//nchan = -1;	//value not set yet, i.e. image not read
	//imgx = 0;
	//imgy = 0;
	//filename[0]='\0';
	
	// Initialize preferences
	
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
}

void Application::start_file()
{
	// Opens the data file for saving data
	//
	struct tm *times;
	time_t ltime;
	
	// the following variables holds the fully qualified filename (dir + filename)
	// if directory NOT set then will create file inside of current working directory
	// which is likely <foxsi gse dir>/build/Debug/
	char file[200];

	if (gui->writeFileBut->value() == 1) {
		// Open a file to write the data to
		// Name of file is automatically set with current date
		char stringtemp[80];
		time(&ltime);
		times = localtime(&ltime);
		strftime(stringtemp,24,"data_%Y%m%d_%H%M%S.dat",times);
		strncpy(dataFilename,stringtemp,MAXPATH - 1);
		strcpy(file, dataFileDir);
		strcat(file, dataFilename);
		//if (dataFileDir != "./"){ strcat(dataFilename, dataFileDir); }
		//dataFilename[MAXPATH - 1] = '\0';
		if (gui->fileTypeChoice->value() == 0) {
			dataFile = fopen(file, "w");
			
			if (dataFile == NULL)
				printf_to_console("Cannot open file %s.\n", file); 
			else 
				printf_to_console("Opened file %s.\n", file);
			
		}
		if (gui->fileTypeChoice->value() == 1) {
			if((fout = open(file,O_RDWR|O_CREAT,0600)) < 0){
				printf_to_console("Cannot open file %s.\n", file);}
			else {
				printf_to_console("Opened file %s.\n", file);
			}

		}
		
	} else {
		printf_to_console( "Closing file %s.\n", dataFilename);
		fclose(dataFile);
		fout = 0;
	}

}

void Application::set_datafile_dir(void)
{
	char *temp = fl_dir_chooser("Pick a directory:", "", 0);
	strcpy(dataFileDir, temp);
	if(dataFileDir == NULL)
		return;
	printf_to_console("Output directory set to %s.\n", dataFileDir);
	printf("%s", dataFilename);
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
	gui->stopReadingDataButton->activate();

	stop_message = 0;
	
	variable = (int *) malloc(sizeof(int));
	*variable = 6;
	
	// define the custom priority for one of the threads
    int newprio = -10;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
	
	ret = pthread_create(&thread, &tattr, read_data, (void *) variable);
	
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
		usleep(10000);		// adjust this to achieve desired reading speed
				
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
	print_to_console("Reading stopped.\n");
	gui->stopReadingDataButton->deactivate();
	// send the thread the message to stop itself	
	data_stop_reading();
}