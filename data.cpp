/*
 *  data.cpp
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/31/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

#include "data.h"
#include "threads.h"
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "gui.h"
#include "usbd2xx.h"
#include "telemetry.h"

// for reading from the formatter board
#include "okFrontPanelDLL.h"

#define MAXPATH 128
#define NUM_THREADS 8
#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024

extern int HistogramFunction[MAX_CHANNEL];
extern double detImage[XSTRIPS][YSTRIPS];
extern double detImagetime[XSTRIPS][YSTRIPS];

// Note that an unsigned short int is 2 bytes
#define FRAME_SIZE_IN_SHORTINTS 1024
#define FRAME_SIZE_IN_BYTES 2048
#define CONFIGURATION_FILE "/Users/schriste/Dropbox/foxsigse/gsesync.bit"

extern Gui *gui;
extern Application *app;
char dataFilename[MAXPATH];
FILE *dataFile;

int newdisplay;
int stop_message;
unsigned short int buffer[FRAME_SIZE_IN_SHORTINTS];
unsigned short int buffer0[FRAME_SIZE_IN_SHORTINTS];
unsigned short int framecount;

int *taskids[NUM_THREADS];
int fout;
pthread_mutex_t mymutex;
double nreads;

okCFrontPanel *dev;

extern char *data_file_save_dir;
extern int file_type;

void* data_watch_buffer(void* p)
{
	/* Watches the buffer variable for changes. When changes occur, parse 
	 * the buffer variable into a data packet and update the display.
	 */
	
	// This function never stops unless told to do so by setting stop_message to 1
	while(1)
	{
		if (newdisplay == 1){

			// update the display
			pthread_mutex_lock( &mymutex); /* wait on readgse */

			Fl::lock();
			//data_frame_print_to_file(buffer0);
			//data_update_display(buffer0);
			
			printf("setting newdisplay\n");
			newdisplay = 0;
			printf("got here\n");

			fflush(stdout);
			printf("got here2\n");

			pthread_mutex_unlock(&mymutex);
			printf("got here3\n");

			gui->mainHistogramWindow->redraw();
			gui->mainImageWindow->redraw();
			Fl::awake(); // Without this it may not redraw until next event (like a mouse movement)!
			Fl::unlock();
		}
		
		if (stop_message == 1){
			pthread_exit(NULL);
		}
	}
}

void* data_read_data(void *p)
{
	/* Read the data in continuously. A data frame is read into the buffer 
	 * variable and written to disk (if a file has been opened), 
	 * and then copied into the buffer0 variable. This variable is being watched 
	 * by data_watch_buffer which uses it to update the displays.
	 */
	ssize_t wlen;
	long len;
	
	int badSync = 0;
	int badRead = 0;
	int status = 0;
	char buffer[50];

	int maxreads;
	
	maxreads = gui->nEvents->value();
	
	// Read the read delay from the preferences
	int read_delay;
	gui->prefs->get("read_delay", read_delay, 0);
	
	// Read the data source from the preferences
	int data_source;
	gui->prefs->get("data_source", data_source, 0);
	
	if (data_source == 2) {
		data_init_formatter();
	}
	
	// This function never stops unless told to do so by setting stop_message to 1
	while (1) {
		
		// For the desired reading speed		
		if (read_delay != 0) {usleep(read_delay);}
		
		// read the data from usb
		if (data_source == 0){
			data_simulate_data();
		}
		
		if (data_source == 1) {
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
		}
		
		if (data_source == 2){
			len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer);
			// printf("%d", len);
		}
		
		if(fout > 0)
		{
			if( (wlen = write(fout,(const void *) buffer,FRAME_SIZE_IN_BYTES) ) != FRAME_SIZE_IN_BYTES){};
		}
		
		if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
		{
			if (newdisplay == 0)
			{
				memcpy((void *) buffer0,(void *) buffer,FRAME_SIZE_IN_BYTES);
				newdisplay = 1;
			}
			pthread_mutex_unlock(&mymutex);
		}
		
		// Check to see if if only a fixed number of frames should be read
		// if so set the stop message
		if ((maxreads != 0) && (nreads >= maxreads)){
			stop_message = 1;
		}
		
		if (stop_message == 1){
			Fl::lock();
			app->print_to_console("Read finished or stopped.\n");
						
			if (data_source == 1) {
				sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
				gui->consoleBuf->insert(buffer);						
			}
			gui->stopReadingDataButton->deactivate();

			Fl::unlock();	
			
			app->printf_to_console("Closing file: %s.\n", dataFilename);
			if (fout > 0)
				close(fout);
			if (dataFile != NULL)
				fclose(dataFile);
			
			pthread_mutex_destroy(&mymutex);
			pthread_exit(NULL);
			
			if (data_source == 1)
			{
				//cleam up usb interface
			}
			if (data_source == 2) {
				//clean up formatter interface
			}
			
		}
		nreads++;	
	}
}

void data_simulate_data(void)
{
	// Add some delay to simulate the reading time
	usleep(100);
	
	framecount++;
	
	unsigned short int tempx;
	unsigned short int tempy;
	unsigned short int tempenergy;

	struct strip_data  // 2 bytes
	{
		unsigned data : 10;
		unsigned number : 6;
	};
	
	struct voltage_data {
		unsigned value: 12;
		unsigned status: 4;
	};
	
	strip_data strip;
	voltage_data volt;
	
	buffer[0] = 60304; 	// Sync 1 - Hex EB90
	buffer[1] = 63014;  // Sync 2 - Hex F626
	buffer[2] = (unsigned short int) (arc4random() % 100); 	// Time 1 (MSB)
	buffer[3] = (unsigned short int) (arc4random() % 100); 	// Time 2 (MSB)
	buffer[4] = (unsigned short int) (arc4random() % 100); 	// Time 3 (LSB)
	buffer[5] = framecount; 	// Frame Counter 1
	buffer[6] = framecount;		// Frame Counter 2
	// 7 Housekeeping 0
	// 8 Cmd Count
	// 9 Command 1
	// 10 Command 2
	// 11 Housekeeping 1
	// 12 Formatter Status
	// 13 0
	// 14 HV value/status
	volt.value = telemetry_voltage_convert_hvvalue(250);
	volt.status = 4;
	memcpy(&buffer[14],&volt,2);
	// 15 Housekeeping 2
	// 16 Status 0
	// 17 Status 1
	// 18 Status 2
	// 19 Housekeeping 3
	// 20 Status 3
	// 21 Status 4
	// 22 Status 5
	// 23 Status 6
	// 24 Detector 0 Time
	buffer[25] = (unsigned short int) (arc4random()); // 25 0ASIC0 mask0
	buffer[26] = (unsigned short int) (arc4random()); // 26 0ASIC0 mask1
	buffer[27] = (unsigned short int) (arc4random()); // 27 0ASIC0 mask2
	buffer[28] = (unsigned short int) (arc4random()); // 28 0ASIC0 mask3
	// 29 0Strip0A Energy
	
	// Choose some random pixels to light up
	tempx = (arc4random() % 128 + 1);
	tempy = (arc4random() % 128 + 1);
	tempenergy = (arc4random() % MAX_CHANNEL + 1);

	// if (tempx < 64){ buffer[25] = ~(~0 << 1) << (tempx - 
	
	strip.data = tempx;
	strip.number = tempenergy;
	memcpy(&buffer[29],&strip,2);
	// 30 0Strip0B Energy
}

void data_start_file(void)
{
	/* Open a file to write the data to. The file pointer is set to fout.
	 * 
	 */

	// Opens the data file for saving data
	// the following variables holds the fully qualified filename (dir + filename)
	// if directory NOT set then will create file inside of current working directory
	// which is likely <foxsi gse dir>/build/Debug/
	
	if (gui->writeFileBut->value() == 1) {
		data_set_datafilename();
		app->printf_to_console("Trying to open file: %s\n", dataFilename);
		if (gui->fileTypeChoice->value() == 0) {
			dataFile = fopen(dataFilename, "w");
			
			if (dataFile == NULL)
				app->printf_to_console("Cannot open file %s.\n", dataFilename); 
			else 
				app->printf_to_console("Opened file %s.\n", dataFilename);
			
		}
		if (gui->fileTypeChoice->value() == 1) {
			if((fout = open(dataFilename,O_RDWR|O_CREAT,0600)) < 0){
				app->printf_to_console("Cannot open file %s.\n", dataFilename);}
			else {
				app->printf_to_console("Opened file %s.\n", dataFilename);
			}
			
		}
		
	} else {
		app->printf_to_console( "Closing file %s.\n", dataFilename);
		if (gui->fileTypeChoice->value() == 1){close(fout);}
		if (gui->fileTypeChoice->value() == 0){fclose(dataFile);}
		fout = 0;
	}
	
}

void data_start_reading(void)
{
	pthread_t thread;
    struct sched_param param;
    pthread_attr_t tattr;
	
	int *variable;
	int ret;
	gui->stopReadingDataButton->activate();
	
	stop_message = 0;
	
	// define a high (custom) priority for the read_data thread
    int newprio = -10;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
	
	app->print_to_console("Reading...\n");
	
	// start the read_data thread
	ret = pthread_create(&thread, &tattr, data_read_data, (void *) taskids[0]);
	pthread_mutex_init(&mymutex,NULL);

	//newprio = -5;
	//param.sched_priority = newprio;
	//ret = pthread_attr_init(&tattr);
	//ret = pthread_attr_setschedparam (&tattr, &param);
	
	// start the watch_buffer thread
	ret = pthread_create(&thread, NULL, data_watch_buffer, (void *) taskids[1]);	
	
	
}

void data_stop_reading(void)
{
	stop_message = 1;
}

void data_frame_print_to_file(unsigned short int *frame)
{
	if (gui->writeFileBut->value() == 1) {gui->usb->writeFrame(dataFile);}
}

void data_update_display(unsigned short int *frame)
{
	unsigned short int tmp;
	unsigned voltage_status;
	unsigned voltage;
	// parse the buffer variable and update the display
	// some example code is below
	
	struct strip_data  // 2 bytes
	{
		unsigned data : 10;
		unsigned number : 6;
	};
	
	struct voltage_data {
		unsigned value: 12;
		unsigned status: 4;
	};
	
	strip_data strip;
	voltage_data volt;

	printf("sync: %x\n", buffer[0]);
	printf("counter: %x\n", buffer[4]);
	
	memcpy(&strip,&buffer[29],2);
	memcpy(&volt,&buffer[13],2);
	
	voltage_status = buffer[13] & 0xF;
	voltage = (buffer[13] >> 12) & 0xFFF;
	
	gui->nEventsDone->value(nreads); 
	
	gui->framenumOutput->value(frame[5]);
	gui->testOutput->value(strip.number);
	gui->HVOutput->value(voltage/8.0);
	if (voltage_status == 1){gui->HVOutput->textcolor(FL_RED);}
	if (voltage_status == 2){gui->HVOutput->textcolor(FL_BLUE);}
	if (voltage_status == 4){gui->HVOutput->textcolor(FL_BLACK);}
	
	HistogramFunction[strip.data]++;
	tmp = arc4random() % 64 + 1;
	detImage[tmp][strip.number] = 1;
	detImagetime[tmp][strip.number] = clock();
	// detImagemask[i][j] = getbits(xmask, XSTRIPS/8 - i % (XSTRIPS/8)-1,1) * getbits(ymask, YSTRIPS/8 - j % (YSTRIPS/8)-1,1);
	
}

void data_set_datafilename(void)
{
	// Open a file to write the data to
	// Name of file is automatically set with current date
	struct tm *times;
	time_t ltime;
	
	// The directory the file will go in is set in the preferences
	gui->prefs->get("data_file_save_dir", data_file_save_dir, "/Users/schriste/");
	
	gui->prefs->get("file_type", file_type, 0);
	
	char stringtemp[80];
	time(&ltime);
	times = localtime(&ltime);

	if (file_type == 0) {
		strftime(stringtemp,25,"data_%Y%m%d_%H%M%S.txt",times);
	}
	if (file_type == 1) {
		strftime(stringtemp,25,"data_%Y%m%d_%H%M%S.dat",times);
	}
	
	strncpy(dataFilename, data_file_save_dir, MAXPATH - 1);
	strcat(dataFilename, stringtemp);
	//dataFilename[MAXPATH - 1] = '\0';
	printf("%s\n",dataFilename);
}

void data_init_formatter(void)
{
	bool bresult;
	char dll_date[32], dll_time[32];
	
	if (FALSE == okFrontPanelDLL_LoadLib(NULL)) 
	{
		printf("FrontPanel DLL could not be loaded.\n");
	}
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf(" FrontPanel DLL loaded.Built: %s %s\n", dll_date, dll_time);
	
	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		//return(NULL);
	}
	
	switch (dev->GetBoardModel()) {
		case okCFrontPanel::brdXEM3005:
			printf("                                        Found a device: XEM3005\n");
			break;
		default:
			printf("Unsupported device.\n");
			delete dev;
			//return(NULL);
	}
	
	// Configure the PLL appropriately
	okCPLL22150 *pll = new okCPLL22150 ;
	pll->SetReference(48.0f, false);
	bresult = pll->SetVCOParameters(574, 105); // output 32 x 8.2 MHz - close to 8.192 MHz
	//cout << "Settin VCO Parameters " << bresult << endl;
	pll->SetDiv1(okCPLL22150::DivSrc_VCO, 8);
	// 32.8 approcimately 16 x 2.048 MHz
	pll->SetDiv2(okCPLL22150::DivSrc_VCO, 127);
	pll->SetOutputSource(0, okCPLL22150::ClkSrc_Div2By2); // Xilinx pin A8 = SYS_CLK1 (clk1)
	pll->SetOutputEnable(0, true);
	pll->SetOutputSource(1, okCPLL22150::ClkSrc_Div1ByN); // Xilinx pin A8 = SYS_CLK1 (clk1)
	pll->SetOutputEnable(1, true);
	pll->SetOutputSource(2, okCPLL22150::ClkSrc_Div1ByN); // Xilinx pin E9 = SYS_CLK2 (clk2)
	pll->SetOutputEnable(2, true);
	pll->SetOutputSource(3, okCPLL22150::ClkSrc_Div1ByN); // p71 = SYS_CLK4
	pll->SetOutputEnable(3, true);
	pll->SetOutputSource(4, okCPLL22150::ClkSrc_Div1ByN); // p69 = SYS_CLK5
	pll->SetOutputEnable(4, true);
	pll->SetOutputSource(5, okCPLL22150::ClkSrc_Div2ByN); //p67 = SYS_CLK6
	pll->SetOutputEnable(5, true);
	dev->SetPLL22150Configuration(*pll);
	// Get some general information about the XEM.
	printf("                                        Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	printf("                                        Device serial number: %s\n", dev->GetSerialNumber().c_str());
	printf("                                        Device ID string: %s\n", dev->GetDeviceID().c_str());
	
	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(CONFIGURATION_FILE)) {
		printf("FPGA configuration failed.\n");
		delete dev;
		//return(NULL);
	}
	
	// Check for FrontPanel support in the FPGA configuration.
	if (false == dev->IsFrontPanelEnabled()) {
		printf("                              FrontPanel support is not enabled.\n");
		delete dev;
		//return(NULL);
	}
	
	printf("                                        FrontPanel support is enabled.\n");
	
}
