/*
 *  data.cpp
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/31/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */
#include "Application.h"

#include "data.h"
#include "threads.h"
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui.h"
#include "usbd2xx.h"
#include "okFrontPanelDLL.h"
#include "telemetry.h"

#define MAXPATH 128
#define NUM_THREADS 8
#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024
#define FORMATTER_CONFIGURATION_FILE "gsesync.bit"

extern int HistogramFunction[MAX_CHANNEL];
extern double detImage[XSTRIPS][YSTRIPS];
extern double detImagetime[XSTRIPS][YSTRIPS];
extern double detImagemask[XSTRIPS][YSTRIPS];

// Note that an unsigned short int is 2 bytes
// for formatter
// #define FRAME_SIZE_IN_SHORTINTS 295
// #define FRAME_SIZE_IN_BYTES 590

// for ASIC
#define FRAME_SIZE_IN_SHORTINTS 784
#define FRAME_SIZE_IN_BYTES 1568

extern Gui *gui;

char dataFilename[MAXPATH];
FILE *dataFile;

okCFrontPanel *dev;

int newdisplay;
int stop_message;
time_t start_time;
time_t current_time;
extern int data_source;
unsigned short int buffer[FRAME_SIZE_IN_SHORTINTS];
unsigned short int buffer0[FRAME_SIZE_IN_SHORTINTS];
unsigned short int framecount;

extern int current_timebin;
extern int timebins[1024];

int *taskids[NUM_THREADS];
int fout;
pthread_mutex_t mymutex;
double nreads;

extern char *data_file_save_dir;
extern int file_type;

void data_initialize(void)
{
	// Initialize a connection to a data stream
	
	if (data_source == 0)
	{
		gui->app->print_to_console("Initializing Simulated data.\n");
		gui->startReadingDataButton->activate();
		gui->closeBut->activate();
		gui->app->flush_image();
		gui->app->flush_histogram();
		gui->app->flush_timeseries();
		gui->app->print_to_console("Done initializing.\n");
		
	}
	
	if (data_source == 1)
	{
		gui->app->print_to_console("Initializing USB/ASIC connection.\n");
		if (gui->usb->open() < 0)
		{
			gui->app->print_to_console("Could not open device.\n");
			gui->app->print_to_console("Initialization Failed!\n");
		}
		else
		{		
			gui->startReadingDataButton->activate();
			gui->closeBut->activate();
			gui->sendParamsWindow_sendBut->activate();
			gui->setHoldTimeWindow_setBut->activate();
			gui->setHoldTimeWindow_autorunBut->activate();
			gui->app->flush_image();
			gui->app->flush_histogram();
			gui->app->print_to_console("Done initializing.\n");
		}

	}
	
	if (data_source == 2)
	{
		char dll_date[32], dll_time[32];
		int init_state = 1;
		
		gui->app->print_to_console("Initializing USB/Formatter connection.\n");

		if (FALSE == okFrontPanelDLL_LoadLib(NULL)) 
		{
			gui->app->print_to_console("FrontPanel DLL could not be loaded.\n");
			init_state = 0;
		}
		okFrontPanelDLL_GetVersion(dll_date, dll_time);
		printf("FrontPanel DLL loaded.Built: %s %s\n", dll_date, dll_time);
		
		okCFrontPanel *devi = data_initialize_formatter_FPGA();
		if (NULL == devi) 
		{
			gui->app->print_to_console("FPGA could not be initialized.\n");
			gui->app->print_to_console("Initialization Failed!\n");
			init_state = 0;
		}
		
		if (init_state == 1){
			gui->startReadingDataButton->activate();
			gui->closeBut->activate();
			gui->detector_choice->activate();
			gui->app->print_to_console("Done initializing.\n");
		} else gui->app->print_to_console("Initialization failed!\n");
	}

}

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
			
			if (gui->writeFileBut->value() == 1){
				data_frame_print_to_file(buffer0);
			}
			data_update_display(buffer0);
			
			newdisplay = 0;
			fflush(stdout);
		
			pthread_mutex_unlock(&mymutex);
			
			gui->mainHistogramWindow->redraw();
			gui->mainImageWindow->redraw();
			gui->mainLightcurveWindow->redraw();
			Fl::awake(); // Without this it may not redraw until next event (like a mouse movement)!
			Fl::unlock();
		}
		
		if (stop_message == 1){
			pthread_exit(NULL);
		}
	}
}

void* data_timer(void *p)
{
	/* Keep track of the timer */
	int i = 1;
	while(1)
	{
		current_time = time(NULL);
		if (data_source == 0){

			if (difftime(current_time, start_time) > i)
			{
				timebins[MAX_CHANNEL-1] = current_timebin;
				for(int j = 0; j < MAX_CHANNEL-1; j++){ 
					timebins[j] = timebins[j+1]; }

				i++;
				current_timebin = 0;
			}
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
	gui->prefs->get("data_source", data_source, 0);
	
	// Check to see if a file is open and write header to it before starting
	// to write data to it
	if((dataFile != NULL) && (data_source == 1))
	{
		gui->usb->writeHeader(dataFile);
	}
	
	// This function never stops unless told to do so by setting stop_message to 1
	while (1) {
		
		// For the desired reading speed		
		if (read_delay != 0) {usleep(read_delay);}
		
		// read the data
		if (data_source == 0){
			// read from the simulation
			data_simulate_data();
		}
		
		if (data_source == 1) {
			// read from the USB/ASIC
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
			// read from the USB/Formatter, reads 4 frames at a time.
			len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer);
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
			gui->app->print_to_console("Read finished or stopped.\n");
						
			if (data_source == 1) {
				sprintf(buffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
				gui->consoleBuf->insert(buffer);						
			}
			gui->stopReadingDataButton->deactivate();
			gui->startReadingDataButton->activate();

			Fl::unlock();	
			
			if (fout > 0){
				gui->app->printf_to_console("Closing file: %s.\n", dataFilename, NULL);
				close(fout);
			}
			if (dataFile != NULL){
				gui->app->printf_to_console("Closing file: %s.\n", dataFilename, NULL);
				fclose(dataFile);
			}
			
			if (data_source == 1)
			{
				//cleam up usb interface
			}
			if (data_source == 2) {
				//clean up formatter interface
			}
			
			pthread_mutex_destroy(&mymutex);
			pthread_exit(NULL);
		}
		nreads++;	
	}
}

void data_simulate_data(void)
{
	// Add some delay for the actual read
	usleep(100);
	
	framecount++;
	
	unsigned short int tempx;
	unsigned short int tempy;
	unsigned short int tempenergy;

	struct voltage_data {
		unsigned value: 12;
		unsigned status: 4;
	};
	
	strip_data strip;
	voltage_data volt;
	
	struct strip_data  // 2 bytes
	{
		unsigned data : 10;
		unsigned number : 6;
	};
		
	buffer[0] = 60304; 	// Sync 1 - Hex EB90 - missing in formatter data
	buffer[0] = 63014;  // Sync 2 - Hex F626
	buffer[1] = (unsigned short int) (arc4random() % 100); 	// Time 1 (MSB)
	buffer[2] = (unsigned short int) (arc4random() % 100); 	// Time 2 (MSB)
	buffer[3] = (unsigned short int) (arc4random() % 100); 	// Time 3 (LSB)
	buffer[4] = framecount; 	// Frame Counter 1
	buffer[5] = framecount;		// Frame Counter 2
	// 7 Housekeeping 0
	// 8 Cmd Count
	// 9 Command 1
	// 10 Command 2
	// 11 Housekeeping 1
	// 12 Formatter Status
	// 13 0
	volt.value = telemetry_voltage_convert_hvvalue(250);
	volt.status = 4;
	memcpy(&buffer[13],&volt,2);
	
	// 14 HV value/status
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

	// the following variables holds the fully qualified filename (dir + filename)
	// if directory NOT set then will create file inside of current working directory
	// which is likely <foxsi gse dir>/build/Debug/
	
	if (gui->writeFileBut->value() == 1) {
		data_set_datafilename();
		gui->app->printf_to_console("Trying to open file: %s\n", dataFilename, NULL);
		if (gui->fileTypeChoice->value() == 0) {
			dataFile = fopen(dataFilename, "w");
			
			if (dataFile == NULL)
				gui->app->printf_to_console("Cannot open file %s.\n", dataFilename, NULL); 
			else 
				gui->app->printf_to_console("Opened file %s.\n", dataFilename, NULL);
			
			//gui->app->write_header(dataFile);
			
		}
		if (gui->fileTypeChoice->value() == 1) {
			if((fout = open(dataFilename,O_RDWR|O_CREAT,0600)) < 0){
				gui->app->printf_to_console("Cannot open file %s.\n", dataFilename, NULL);}
			else {
				gui->app->printf_to_console("Opened file %s.\n", dataFilename, NULL);
			}
			
		}
		
	} else {
		gui->app->printf_to_console( "Closing file %s.\n", dataFilename, NULL);
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
	start_time = time(NULL);
	
	// define a high (custom) priority for the read_data thread
    int newprio = -10;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
	
	gui->app->print_to_console("Reading...\n");
	
	// start the read_data thread
	ret = pthread_create(&thread, &tattr, data_read_data, (void *) taskids[0]);
	pthread_mutex_init(&mymutex,NULL);

	//newprio = -5;
	//param.sched_priority = newprio;
	//ret = pthread_attr_init(&tattr);
	//ret = pthread_attr_setschedparam (&tattr, &param);
	
	// start the watch_buffer thread
	ret = pthread_create(&thread, NULL, data_watch_buffer, (void *) taskids[1]);	
	
	//newprio = -5;
	//param.sched_priority = newprio;
	//ret = pthread_attr_init(&tattr);
	//ret = pthread_attr_setschedparam (&tattr, &param);
	
	// start the data reading clock
	ret = pthread_create(&thread, NULL, data_timer, (void *) taskids[2]);
}

void data_stop_reading(void)
{
	stop_message = 1;	
}

void data_frame_print_to_file(unsigned short int *frame)
{
	if (data_source == 1)
	{
		gui->usb->writeFrame(dataFile);
	}
}

void data_update_display(unsigned short int *frame)
{
	// Parse the buffer variable and update the display
	// reading from the ASIC one frame at a time
	// reading from the Formater four frames at a time
	unsigned short int strip_data;
	unsigned short int strip_number;

	int ymask[128] = {0};
	int xmask[128] = {0};
	int xstrips[128] = {0};
	int ystrips[128] = {0};
	
	gui->nEventsDone->value(nreads); 
	
	if (data_source == 0) {
		int num_hits;
		
		// if parsing simulated data
		unsigned voltage_status;
		unsigned voltage;

		voltage_status = buffer[13] & 0xF;
		voltage = (buffer[13] >> 12) & 0xFFF;
		
		gui->nEventsDone->value(nreads); 
		gui->framenumOutput->value(frame[5]);
		gui->inttimeOutput->value(difftime(current_time,start_time));

		gui->HVOutput->value(voltage);
		if (voltage_status == 1){gui->HVOutput->textcolor(FL_RED);}
		if (voltage_status == 2){gui->HVOutput->textcolor(FL_BLUE);}
		if (voltage_status == 4){gui->HVOutput->textcolor(FL_BLACK);}
		
		gui->testOutput->value(timebins[1023]);
		
		num_hits = arc4random() % 10;
		for(int i = 0; i < num_hits; i++)
		{
			int x, y, z;
			// HistogramFunction[strip_data]++;
			x = arc4random() % XSTRIPS + 1;
			y = arc4random() % YSTRIPS + 1;
			z = arc4random() % MAX_CHANNEL + 1;
			HistogramFunction[z] += 1;
			detImage[x][y] = z;
			detImagetime[x][y] = clock();
			// detImagemask[i][j] = getbits(xmask, XSTRIPS/8 - i % (XSTRIPS/8)-1,1) * getbits(ymask, YSTRIPS/8 - j % (YSTRIPS/8)-1,1);		
		}
		
		current_timebin+=num_hits;
		//if (<#condition#>) {
//			current_timebin+=num_hits;
//		} else {
//			current_timebin = 0;
//		}

		
	}
	
	if (data_source == 1){
		// parse and display the data from the USB/ASIC connection
		// order for asics n n then p p
		int index = 0;

		// Loop through the 4 ASICS
		for( int j = 0; j < 4; j++)
		{
			printf("ASIC %d frame START ---------------------- \n", j);

			// for the other ASICs, it's already in the data stream.
			if(j==0){
				printf( "eb90\n");
			} else{
				printf( "%x\n", frame[index]);	index++;
			}
						
			printf( "frame counter: %u\n", frame[index]);	index++;	//frame counter
			printf( "start bit: %x\n", frame[index]);	index++;	//start
			printf( "chip bit: %x\n", frame[index]);	index++;	//chip
			printf( "trigger bit: %x\n", frame[index]);	index++;	//trigger
			printf( "seu bit: %x\n", frame[index]);	index++;	//seu
			
			// channel mask displayed in hex
			// First two channel mask bits are stored as one word each.
			printf("Channel Mask:\n");
			
			printf( "%x", frame[index]);	index++;	// dummy pedestal
			printf( "%x", frame[index]);	index++;	// common mode bit
			
			for( int i = 0; i < 8; i++ ){ xmask[i] = getbits(frame[index], i, 1); }
			for( int i = 8; i < 16; i++ ){ xmask[i] = getbits(frame[index], i, 1);}
			for( int i = 16; i < 32; i++ ){ xmask[i] = getbits(frame[index], i, 1);}
			for( int i = 32; i < 64; i++ ){ xmask[i] = getbits(frame[index], i, 1);}
			
			printf( "%x", frame[index]);	index++;
			printf( "%x", frame[index]);	index++;
			printf( "%x", frame[index]);	index++;
			printf( "%x\n", frame[index]);	index++;
			
			printf( "Pedestal: %u\n", frame[index]);	index++;	//pedestal
			
			printf("Strip Data (%i):\n", index);
			printf( "\nCommon mode %i: %u\n", index+64, frame[index+64]);	//common mode

			// strip data
			
			for(int i = 0; i < XSTRIPS/2; i++){
				// strip_data = frame[index] & 0x3ff;
				// strip_number = (frame[index] << 10) & 0x3f;
				strip_data = frame[index] - frame[index+64];
				if (j == 3){if (strip_data > 0){ HistogramFunction[strip_data]++; }}
				
				if (strip_data != 0){
					// if (j < 2){ xstrips[i + j*64] = strip_data; }
					if (j < 2){ xstrips[i + j*64] = strip_data; }
					if (j >= 2){ ystrips[i + (j-2)*64] = strip_data; }
				}
				printf( "%u\t", frame[index]);	index++;
			}
			
			printf( "\nCommon mode %i: %u\n", index, frame[index]);	index++;	//common mode
			printf("Readout info:\n");
			// 10 extra words of readout information
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\t", frame[index]);	index++;
			printf( "%u\n", frame[index]);	index++;		
			printf("ASIC %u frame END  -------------------------------- \n", j);
			
		}
		
		for(int i = 0; i<XSTRIPS; i++)
		{
			for(int j = 0; j<YSTRIPS; j++)
			{
				detImage[i][j] = xstrips[i] * ystrips[j];
				detImagemask[i][j] = xmask[i] * ymask[j];
				detImagetime[i][j] = clock();
			}	
		}
	}
	
	if (data_source == 2) {
		// parse and display the data from the USB/ASIC connection
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
		gui->HVOutput->value(voltage);
		printf("voltage: %i status: %i", voltage, voltage_status);
		if (voltage_status == 1){gui->HVOutput->textcolor(FL_RED);}
		if (voltage_status == 2){gui->HVOutput->textcolor(FL_BLUE);}
		if (voltage_status == 4){gui->HVOutput->textcolor(FL_BLACK);}
		
		HistogramFunction[strip.data]++;
		tmp = arc4random() % 64 + 1;
		detImage[tmp][strip.number] = 1;
		detImagetime[tmp][strip.number] = clock();
	}

}

void data_set_datafilename(void)
{
	// Open a file to write the data to
	// Name of file is automatically set with current date
	struct tm *times;
	time_t ltime;
	
	// The directory the file will go in is set in the preferences
	gui->prefs->get("data_file_save_dir", data_file_save_dir, "./");
	
	gui->prefs->get("file_type", file_type, 0);
	
	char stringtemp[80];
	time(&ltime);
	times = localtime(&ltime);

	if (file_type == 0) {
		strftime(stringtemp,25,"data_%Y%m%d_%H%M%S.txt\0",times);
	}
	if (file_type == 1) {
		strftime(stringtemp,25,"data_%Y%m%d_%H%M%S.dat\0",times);
	}
	
	strncpy(dataFilename, data_file_save_dir, MAXPATH - 1);
	strcat(dataFilename, stringtemp);
	dataFilename[MAXPATH - 1] = '\0';
	printf("%s\n",dataFilename);

}

okCFrontPanel *data_initialize_formatter_FPGA(void)
{
	bool bresult;
	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
	
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(0);
	}
	
	switch (dev->GetBoardModel()) {
		case okCFrontPanel::brdXEM3005:
			printf("                                        Found a device: XEM3005\n");
			break;
		default:
			printf("Unsupported device.\n");
			delete dev;
			return(NULL);
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
	printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	printf("Device serial number: %s\n", dev->GetSerialNumber().c_str());
	printf("Device ID string: %s\n", dev->GetDeviceID().c_str());
	
	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(FORMATTER_CONFIGURATION_FILE)) {
		printf("FPGA configuration failed.\n");
		delete dev;
		return(0);
	}
	
	// Check for FrontPanel support in the FPGA configuration.
	if (false == dev->IsFrontPanelEnabled()) {
		printf("FrontPanel support is not enabled.\n");
		delete dev;
		return(0);
	}
	
	printf("FrontPanel support is enabled.\n");	
	return dev;
}
