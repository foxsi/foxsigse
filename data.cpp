/*
 *  data.cpp
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/31/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 *	pthread tutorial
 *	https://computing.llnl.gov/tutorials/pthreads/#ConVarSignal
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
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Preferences.H>


#define MAXPATH 128
#define NUM_THREADS 8
#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024

// Note that an unsigned short int is 2 bytes
// for formatter
//#define FRAME_SIZE_IN_SHORTINTS 295
//#define FRAME_SIZE_IN_BYTES 590
#define FRAME_SIZE_IN_SHORTINTS 1024
#define FRAME_SIZE_IN_BYTES 2048

// for ASIC
//#define FRAME_SIZE_IN_SHORTINTS 784
//#define FRAME_SIZE_IN_BYTES 1568

extern Gui *gui;

char dataFilename[MAXPATH];
FILE *dataFile;

okCFrontPanel *dev;

int newdisplay;
int stop_message;
time_t start_time;
time_t current_time;

int nreads;


extern int data_source;
unsigned short int buffer[FRAME_SIZE_IN_SHORTINTS];
unsigned short int buffer0[FRAME_SIZE_IN_SHORTINTS];
unsigned short int framecount;

extern unsigned int current_timebin;
extern unsigned int CountcurveFunction[MAX_CHANNEL];

int *taskids[NUM_THREADS];
int fout;
pthread_mutex_t mymutex;
pthread_mutex_t timebinmutex;

extern char *data_file_save_dir;
extern char *formatter_configuration_file;
extern int file_type;

void data_initialize(void)
{
	// Initialize a connection to a data stream
	
	// Read the data source from the preferences
	gui->prefs->get("data_source", data_source, 0);
	gui->frame_missOutput->value(0);
	
	if (data_source == 0)
	{
		gui->app->print_to_console("Initializing Simulated data.\n");
		gui->startReadingDataButton->activate();
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
			gui->closeBut->value(0);
			gui->sendParamsWindow_sendBut->activate();
			gui->setHoldTimeWindow_setBut->activate();
			//gui->setHoldTimeWindow_autorunBut->activate();
			gui->app->flush_image();
			gui->app->flush_histogram();
			gui->app->flush_timeseries();
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
			
			//gui->closeBut->activate();
			//gui->detector_choice->activate();
			gui->app->flush_image();
			gui->app->flush_histogram();
			gui->app->flush_timeseries();
			gui->shutterstateOutput->value(0);
			gui->app->print_to_console("Done initializing.\n");
		}
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
			gui->detectorsImageWindow->redraw();
			gui->detectorsHistogramWindow->redraw();
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
	while(1)
	{
		// a sleep statement so that it does not pole the time too often
		usleep(0.5/1000000.0);
		current_time = time(NULL);
		gui->app->elapsed_time_sec = difftime(time(NULL), start_time);
		
		if (stop_message == 1){
			pthread_exit(NULL);}		
	}
}

void* data_countrate(void *p)
{
	/* Keep track of the timer */
	unsigned int i = 1;
	while(1)
	{
		int microseconds;
		microseconds = gui->mainLightcurveWindow->binsize[0]*1000000.0;
		
		// System activity may lengthen the sleep by an indeterminate amount.
		// therefore this is not the best way to measure count rate
		// quick and dirty
		usleep(microseconds);
		
		pthread_mutex_lock(&timebinmutex);
		
		CountcurveFunction[0] = current_timebin;
		for(int detector_num = 1; detector_num < (NUM_DETECTORS+1); detector_num++)
		{
			gui->mainLightcurveWindow->CountcurveDetectors[0][detector_num] = gui->mainLightcurveWindow->current_timebin_detectors[detector_num];
		}
		
		for(int j = MAX_CHANNEL-1; j > 0; j--){ 
			CountcurveFunction[j] = CountcurveFunction[j-1];
			gui->mainLightcurveWindow->binsize[j] = gui->mainLightcurveWindow->binsize[j-1];
		}
		
		for(int detector_num = 1; detector_num < (NUM_DETECTORS+1); detector_num++)
		{
			gui->mainLightcurveWindow->CountcurveDetectors[0][detector_num];

			for(int j = MAX_CHANNEL-1; j > 0; j--){ 
				gui->mainLightcurveWindow->CountcurveDetectors[0][j] = gui->mainLightcurveWindow->CountcurveDetectors[0][j-1];
			}
			
		}
		
		current_timebin = 0;
		for(int detector_num = 1; detector_num < (NUM_DETECTORS+1); detector_num++)
			{gui->mainLightcurveWindow->current_timebin_detectors[detector_num] = 0;}
		
		pthread_mutex_unlock(&timebinmutex);
		
		if (stop_message == 1){
			pthread_exit(NULL);}		
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
	char textbuffer[50];
	int maxreads;
	int read_status = 0;
	
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
		read_status = 0;
		
		// For the desired reading speed		
		if (read_delay != 0) {usleep(read_delay);}
		
		// read the data
		if (data_source == 0){
			// read from the simulation
			data_simulate_data();
			read_status = 1;
			gui->app->frame_read_count++;
		}
		
		if (data_source == 1) {
			// read from the USB/ASIC
			status = gui->usb->findSync();
			
			if(status < 1){
				badSync++;
			}
			printf("reading frame\n");
			status = gui->usb->readFrame();
			if(status < 1){
				badRead++;
				printf("Bad read\n");
			} else {
				read_status = 1;
				gui->app->frame_read_count++;
			}

		}
		
		if (data_source == 2){
			// read from the USB/Formatter, reads 4 frames at a time.
			len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer);
			// set the read status based on how len returned 
			if (len == 2048){ 
				read_status = 1;
				gui->app->frame_read_count++;
				//printf("read okay\n");
			} else {
				printf("bad read!\n");
			}

		}
				
		if (read_status == 1) {
	
			if(fout > 0)
			{
				if( (wlen = write(fout,(const void *) buffer,FRAME_SIZE_IN_BYTES) ) != FRAME_SIZE_IN_BYTES)
					{
						// then good write
					} else {
						printf("bad data write!\n");
					}

			}
			
			if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
			{
				if (newdisplay == 0)
				{
					memcpy((void *) buffer0,(void *) buffer, FRAME_SIZE_IN_BYTES);
					newdisplay = 1;
					gui->app->frame_display_count++;
				}
				pthread_mutex_unlock(&mymutex);
			} else {
				// printf("failed to pass off data\n");
				//gui->app->frame_miss_count++;
			}
			// Check to see if if only a fixed number of frames should be read
			// if so set the stop message
			if ((maxreads != 0) && (nreads >= maxreads)){
				stop_message = 1;
			}
		
		}
		
		if (stop_message == 1){
			Fl::lock();
			gui->app->print_to_console("Read finished or stopped.\n");
						
			//gui->stopReadingDataButton->deactivate();
			//gui->startReadingDataButton->deactivate();
			//gui->writeFileBut->deactivate();
			//gui->closeBut->deactivate();
			//gui->initializeBut->activate();

			
			if(nreads > maxreads-1){
				gui->nEventsDone->value(0);
				nreads = 0;
			}

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
				sprintf(textbuffer, "%d bad syncs, %d bad reads.\n", badSync, badRead);
				gui->consoleBuf->insert(textbuffer);	
				
				//clean up usb interface
				gui->usb->close();
				
			}
			if (data_source == 2) {
				// clean up formatter interface
				// nothing to do
				// quitting the program will clean up the interface
			}
			
			pthread_mutex_destroy(&mymutex);
			pthread_mutex_destroy(&timebinmutex);
			pthread_exit(NULL);
		}
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
	int newprio;
	
	stop_message = 0;
	start_time = time(NULL);
	
	// define a high (custom) priority for the read_data thread
    newprio = -10;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
		
	// start the read_data thread
	ret = pthread_create(&thread, &tattr, data_read_data, (void *) taskids[0]);
	pthread_mutex_init(&mymutex,NULL);
	pthread_mutex_init(&timebinmutex,NULL);

	newprio = -5;
	param.sched_priority = newprio;
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setschedparam (&tattr, &param);
	
	// start the watch_buffer thread
	ret = pthread_create(&thread, NULL, data_watch_buffer, (void *) taskids[1]);	
	
	//newprio = -5;
	//param.sched_priority = newprio;
	//ret = pthread_attr_init(&tattr);
	//ret = pthread_attr_setschedparam (&tattr, &param);
	
	// start the data reading clock
	ret = pthread_create(&thread, NULL, data_timer, (void *) taskids[2]);
	// start the count rate thread
	ret = pthread_create(&thread, NULL, data_countrate, (void *) taskids[3]);
	
	gui->stopReadingDataButton->activate();
	gui->app->print_to_console("Reading...\n");
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

	//int ymask[128] = {0};
	//int xmask[128] = {0};
	// xstrips are defined as p strips
	//signed int xstrips[128] = {0};
	// ystrips are defined as n strips
	//signed int ystrips[128] = {0};
	
	unsigned int asic0n_data[64] = {0};
	unsigned int asic1n_data[64] = {0};
	unsigned int asic2p_data[64] = {0};
	unsigned int asic3p_data[64] = {0};
	
	int num_hits = 0;
	int num_hits_detectors[NUM_DETECTORS] = {0};
	
	gui->nEventsDone->value(nreads); 
	gui->inttimeOutput->value(gui->app->elapsed_time_sec);
	
	if (data_source == 0) {
		// Parse simulated data

		unsigned high_voltage_status;
		unsigned high_voltage;

		high_voltage_status = buffer[13] & 0xF;
		high_voltage = (buffer[13] >> 12) & 0xFFF;
		
		gui->framenumOutput->value(frame[5]);
		
		gui->HVOutput->value(high_voltage);
		if (high_voltage_status == 1){gui->HVOutput->textcolor(FL_RED);}
		if (high_voltage_status == 2){gui->HVOutput->textcolor(FL_BLUE);}
		if (high_voltage_status == 4){gui->HVOutput->textcolor(FL_BLACK);}
		
		for(int detector_num = 0; detector_num < NUM_DETECTORS; detector_num++)
		{
			int z, x, y;
			// simulate up to 5 hits, only the first one goes to image, all others go to mask
			num_hits = arc4random() % 5;
			
			for(int i = 0; i < num_hits; i++)
			{
				z = arc4random() % MAX_CHANNEL + 1;
				x = arc4random() % XSTRIPS + 1;
				y = arc4random() % YSTRIPS + 1;
				
				if (i == 0) { 
					gui->mainHistogramWindow->add_count(z, detector_num); 
					gui->detectorsImageWindow->add_count_to_image(x, y, detector_num);
				} else {
					gui->detectorsImageWindow->add_count_to_mask(x, y, detector_num);
				}
			}
		}
		
		gui->frame_missOutput->value((float) 100 * gui->app->frame_miss_count/gui->app->frame_read_count);
		pthread_mutex_lock( &timebinmutex);
		current_timebin += num_hits;
		for(int detector_num = 0; detector_num < (NUM_DETECTORS+1); detector_num++)
		{
			gui->mainLightcurveWindow->current_timebin_detectors[detector_num] += num_hits;
		}
		pthread_mutex_unlock(&timebinmutex);
	}
	
	if (data_source == 1){
		// Parse the data from the USB/ASIC connection
		// 
		// Notes
		// -----
		//
		// * the order for asics n n then p p
		// * the asic mask does not work
		//
		
		unsigned short int xstrips[128];
		unsigned short int ystrips[128];
		
		
		int index = 0;
		unsigned int common_mode = 0;
		unsigned int common_mode_nside[2] = {0};
		int read_status = 1;
		int frame_number = 0;
		
		// Loop through the 4 ASICS
		for( int j = 0; j < 4; j++)
		{
			read_status = 1;
			
			if (j == 0){ frame_number = frame[0]; }
			if ((j == 1) && (frame[index] != 0xEB91)) { read_status = 0; }
			if ((j == 2) && (frame[index] != 0xEB92)) { read_status = 0; }
			if ((j == 3) && (frame[index] != 0xEB93)) { read_status = 0; }

			if (gui->printasicframe_button->value() == 1)
			{
				if (j == 0){ printf("FRAME START\n-----------\neb90\n"); }
				else {
					printf( "%x\n", frame[index]);	index++;
				}
				printf( "frame number: %u\n", frame[index]);//frame counter
				index++;	
				printf( "%x\t", frame[index]);	index++;	//start
				printf(  "%x\t", frame[index]);	index++;	//chip
				printf(  "%x\t", frame[index]);	index++;	//trigger
				printf(  "%x\t", frame[index]);	index++;	//seu
				
				// channel mask displayed in hex
				// First two channel mask bits are stored as one word each.
				printf(  "%x", frame[index]);	index++;
				printf(  "%x", frame[index]);	index++;
				printf(  "%x", frame[index]);	index++;
				printf(  "%x", frame[index]);	index++;
				printf(  "%x", frame[index]);	index++;
				printf(  "%x\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;	//pedestal
			} else {
				if (j == 0){ index += 12;} else { index += 13; }
			}

			// store the common mode
			// note the common mode is not calculated for n sides
			// need to calculate it in software later
			common_mode = frame[index+64];
			
			// strip data
			for(int i = 0; i < 64; i++){
				if (gui->printasicframe_button->value() == 1){
					printf(  "%u\t", frame[index]);}
				
				if (j == 0){ asic0n_data[i] = frame[index]; }
				if (j == 1){ asic1n_data[i] = frame[index]; }
				if (j == 2){ 
					asic2p_data[i] = frame[index]; 
					xstrips[i + (j-2)*64] = frame[index] - common_mode;
				}
				if (j == 3){ 
					asic3p_data[i] = frame[index]; 
					xstrips[i + (j-2)*64] = frame[index] - common_mode;
				}
				index++;
			}
			
			if (gui->printasicframe_button->value() == 1){
				printf(  "common mode: %u\n", frame[index]);	
				index++;	//common mode
				// 10 extra words of readout information
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;
				printf(  "%u\t", frame[index]);	index++;		
				printf("\n\n");
			} else {
				index += 11;
			}
		}
		if ((gui->printasicframe_button->value() == 1) && (read_status == 0))
			{ printf("Bad frame skipping\n"); }
			
		if (read_status == 1) {
			common_mode_nside[0] = median(asic0n_data, 64);
			common_mode_nside[1] = median(asic1n_data, 64);
			
			gui->framenumOutput->value(frame_number);
			
			if (gui->printasicframe_button->value() == 1){
				printf("common mode: %u %u\n\n", common_mode_nside[0], common_mode_nside[1]);}
			
			for(int i = 0; i < XSTRIPS; i++)
			{
				if (xstrips[i] > 0){ gui->mainHistogramWindow->add_count(xstrips[i], 0); }
				if (xstrips[i] >= gui->mainHistogramWindow->get_lowthreshold()){
					num_hits++;
					for(int j = 0; j < YSTRIPS; j++)
					{
						if (j < YSTRIPS){
							if (((asic0n_data[j] - common_mode_nside[0]) > 50) && (asic0n_data[j] < 1023)){ystrips[j] = 1;}
						}
						if (j >= YSTRIPS){
							if (((asic1n_data[j] - common_mode_nside[1]) > 50) && (asic1n_data[j] < 1023)){ystrips[j] = 1;}
						}
						if (xstrips[i] * ystrips[j] != 0) {
							gui->detectorsImageWindow->add_count_to_image(i, j, 0);}
					}
				}
			}
			for(int i = 0; i < XSTRIPS; i++){printf("%u\t", ystrips[i]);}
			printf("\n");
			
			pthread_mutex_lock(&timebinmutex);
			current_timebin += num_hits;
			pthread_mutex_unlock(&timebinmutex);
		}
	}
	
	if (data_source == 2) {
		// parse and display the data from the USB/Formatter connection
		// 
		// Notes
		// -----
		//
		// * the order for asics n n then p p
		// * the asic mask does not work
		// * the data read in, contains four frames
		// * the sync words are f628 at the beginning of the frame and eb90 at
		//   the end of the frame.
		// * the asic data contains the three max strips as well as the common
		//   mode in the fourth data word.
		// * the word right before the ending sync word is a checksum
		
		unsigned short int tmp;
		unsigned short int high_voltage_status;
		unsigned short int high_voltage;
		unsigned short int strip_data;
		unsigned short int strip_number;
		unsigned short int formatter_status;
		bool attenuator_actuating = 0;
		int index = 0;
		unsigned short int read_status;
		uint32_t frameNumber;

		// parse the buffer variable and update the display
		unsigned short int temperature_monitors[12] = {0};
		unsigned short int voltage_monitors[4] = {0};			// order is 5V, -5V, 1.5V, -3.3V
		unsigned short int frame_value;					// to know which frame are provided what telemetry data
		
		for( int frame_num = 0, index = 0; frame_num < 4; frame_num++)
		{
			//printf("f628: %x\n", buffer[index++]);
			if (buffer[index] == 0xf628) {
				read_status = 1;
				printf("-----Frame Start------\n");
				printf("frame number = %u\n", frame_num);				
				printf("eb90: %x\n", buffer[index+255]);
				for(int blah = 0; blah < 16; blah++){printf("%i-%04hX: %i\n", blah, buffer[blah], buffer[blah]);}

				// check the checksum
				for( int word_number = 0; word_number < 256; word_number++ ){read_status ^= buffer[word_number];}
				
				if( read_status == 1 ){
					long long time;
					unsigned short int command_count;
					uint32_t command_value;
					
					nreads++;
					index++;
					
					time = (((unsigned long long) buffer[index] << 32) & 0xFFFF00000000);
					index++;
					time |= ((unsigned long long) (buffer[index] << 16) & 0xFFFF0000);
					index++;
					time |= (unsigned long long) buffer[index];
					time /= 10e6;
					if (gui->app->formatter_start_time = 0){ gui->app->formatter_start_time = time; }
					index++;
					
					//index++;
					frameNumber = ((uint32_t)(buffer[index] << 16) & 0xFFFF0000) | buffer[index+1];
					
					if (gui->app->frame_number != 0) {gui->app->frame_miss_count += frameNumber - gui->app->frame_number - 1;}
					gui->app->frame_number = frameNumber;
					gui->app->frame_display_count++;
					
					frame_value = buffer[index+1] & 0x3;
					
					index++;
					index++;
					
					// Housekeeping 0
					switch (frame_value) {
						case 0:
							// temperature reference
							temperature_monitors[0] = buffer[index++];
							break;
						case 1:
							//1.5 V monitor
							voltage_monitors[2] = buffer[index++];
							break;
						case 2:
							temperature_monitors[4] = buffer[index++];
							break;
						case 3:
							temperature_monitors[8] = buffer[index++];
							break;
						default:
							printf("housekeeping 0: %u\n", buffer[index++]);
							break;
					}

					command_count = buffer[index];
					//printf("cmd count: %x\n", buffer[index++]);
					index++;
					command_value = ((uint32_t)(buffer[index] << 16) & 0xFFFF0000) | buffer[index+1];
					index++;
					index++;
					//printf("command 1: %x\n", buffer[index++]);
					//printf("command 2: %x\n", buffer[index++]);

					// Housekeeping 1
					//printf("%i, housekeeping 1: %x\n", index, buffer[index]);
					switch (frame_value) {
						case 0:
							// 5 V monitor
							voltage_monitors[0] = buffer[index++];
							break;
						case 1:
							temperature_monitors[1] = buffer[index++];
							break;
						case 2:
							temperature_monitors[5] = buffer[index++];
							break;
						case 3:
							temperature_monitors[9] = buffer[index++];
							break;
						default:
							printf("Housekeeping 1: %u\n", buffer[index++]);
							break;
					}
					
					formatter_status = buffer[index++]; // printf("FormatterStatus: %u\n", buffer[index++]);
					
					if (getbits(formatter_status, 2, 1)) {
						attenuator_actuating = 1;
					}
					index++;

					//printf("HV index %i\n", index);
					high_voltage_status = buffer[index] & 0xF;
					high_voltage = ((buffer[index] >> 4) & 0xFFF)/8.0;
					index++;
					// Housekeeping 2
					//printf("%i, housekeeping 2: %x\n", index, buffer[index]);
					switch (frame_value) {
						case 0:
							// -5 V monitor
							voltage_monitors[1] = buffer[index++];
							break;
						case 1:
							temperature_monitors[2] = buffer[index++];
							break;
						case 2:
							temperature_monitors[6] = buffer[index++];
							break;
						case 3:
							temperature_monitors[10] = buffer[index++];
							break;
						default:
							printf("Housekeeping 2: %u\n", buffer[index++]);
							break;
					}
					
					//printf("Status 0: %x\n", buffer[index++]);
					index++;
					index++;
					index++;
					//printf("Status 1: %x\n", buffer[index++]);
					//printf("Status 2: %x\n", buffer[index++]);
					
					// Housekeeping 3
					//printf("%i, housekeeping 3: %x\n", index, buffer[index]);
					switch (frame_value) {
						case 0:
							// 3.3 V monitor
							voltage_monitors[3] = buffer[index++];
							break;
						case 1:
							temperature_monitors[3] = buffer[index++];
							break;
						case 2:
							temperature_monitors[7] = buffer[index++];
							break;
						case 3:
							temperature_monitors[11] = buffer[index++];
							break;
						default:
							printf("Housekeeping 3: %u\n", buffer[index++]);
							break;
					}
					
					//printf("Status 3: %x\n", buffer[index]);
					index++;
					//printf("Status 4: %x\n", buffer[index]);
					index++;
					//printf("Status 5: %x\n", buffer[index]);
					index++;
					//printf("Status 6: %x\n", buffer[index]);
					index++;
					
					for(int detector_num = 0; detector_num < 7; detector_num++){
						unsigned short int common_mode[2] = {0};
						
						unsigned short int ymask[128] = {0};
						unsigned short int xmask[128] = {0};
						// xstrips are defined as p strips
						unsigned short int xstrips[128] = {0};
						// ystrips are defined as n strips
						unsigned short int ystrips[128] = {0};
						//printf("Detector %u time, index %i: %u\n", detector_num, buffer[index], index);
						
						unsigned short int detector_time = 0;
						detector_time = buffer[index];
						
						// check to see if detector data exists
						bool detector_data_exists;
						detector_data_exists = !((buffer[index] == buffer[index+1]) && (buffer[index+2] == buffer[index+3]) && (buffer[index+4] == buffer[index+5]) && (buffer[index+6] && buffer[index+7]));
						
						if ( detector_data_exists ) {
							
							for(int asic_number = 0; asic_number < 4; asic_number++)
							{
								index++;
								// 0ASIC0 mask0
								tmp = buffer[index];
								
								for(int position = 0; position < 16; position++){
									if (asic_number == 0) {xmask[position] = getbits(tmp, position, 1);}
									if (asic_number == 1) {xmask[position+64] = getbits(tmp, position, 1);}
									if (asic_number == 2) {ymask[position] = getbits(tmp, position, 1);}
									if (asic_number == 3) {ymask[position+64] = getbits(tmp, position, 1);}	
								}
								// 0ASIC0 mask1
								index++;
								tmp = buffer[index];
								
								for(int position = 0; position < 16; position++){
									if (asic_number == 0) {xmask[position+16] = getbits(tmp, position, 1);}
									if (asic_number == 1) {xmask[position+64+16] = getbits(tmp, position, 1);}
									if (asic_number == 2) {ymask[position+16] = getbits(tmp, position, 1);}
									if (asic_number == 3) {ymask[position+64+16] = getbits(tmp, position, 1);}	
								}
								// 0ASIC0 mask2tmp = buffer[index++];
								index++;
								tmp = buffer[index];
								
								for(int position = 0; position < 16; position++){
									if (asic_number == 0) {xmask[position+32] = getbits(tmp, position, 1);}
									if (asic_number == 1) {xmask[position+64+32] = getbits(tmp, position, 1);}
									if (asic_number == 2) {ymask[position+32] = getbits(tmp, position, 1);}
									if (asic_number == 3) {ymask[position+64+32] = getbits(tmp, position, 1);}	
								}
								// 0ASIC0 mask3
								index++;
								tmp = buffer[index];
								for(int position = 0; position < 16; position++){
									if (asic_number == 0) {xmask[position+48] = getbits(tmp, position, 1);}
									if (asic_number == 1) {xmask[position+64+48] = getbits(tmp, position, 1);}
									if (asic_number == 2) {ymask[position+48] = getbits(tmp, position, 1);}
									if (asic_number == 3) {ymask[position+64+48] = getbits(tmp, position, 1);}	
								}
								
								// 3 strips are read out, middle strip should be biggest one,
								// data layout for strips is 
								// 6 bits - strip number
								// 12 bits - strip data
								// 4th strip is the common mode and takes up entire 16 bits
								unsigned short int strip_data[3] = {0};
								unsigned short int h[4] = {buffer[index+1], buffer[index+2], buffer[index+3], buffer[index+4]};
								for(int strip_num = 0; strip_num < 4; strip_num++)
								{
									index++;
									
									//printf("strip number%u\n", strip_number);
									
									if ((buffer[index] != 0xFFFF) && (buffer[index] != 0)){
										// only keep the middle strip which is the max
										if (strip_num == 1) {							
											strip_data[strip_num] = buffer[index] & 0x3FF;
											strip_number = (buffer[index] >> 10) & 0x3F;
											
											// n side asic
											if (asic_number == 0) {ystrips[strip_number] = strip_data[strip_num];}
											if (asic_number == 1) {ystrips[strip_number + 63] = strip_data[strip_num];}
											// p side asic
											if (asic_number == 2) {xstrips[63 - strip_number] = strip_data[strip_num];}
											if (asic_number == 3) {xstrips[127 - strip_number] = strip_data[strip_num];}
											
										} 
										
										if (strip_num == 3) {
											if (asic_number == 2) {common_mode[0] = buffer[index];}
											if (asic_number == 3) {common_mode[1] = buffer[index];}
										}
										//printf("common mode: %u\n", common_mode);
									}
								}
							}
							
							for(int i = 0; i < XSTRIPS; i++)
							{
								int bin = 0;
								if (xstrips[i] != 0) {
									if (gui->minus_common_mode_checkbox->value() == 1) {bin = xstrips[i] - common_mode[i % 64];}
									else {
										bin = xstrips[i];
									}
									gui->mainHistogramWindow->add_count(bin, detector_num);
								}
								if (bin > gui->mainHistogramWindow->get_lowthreshold()) {
									num_hits++;
									//num_hits_detectors[detector_num]++;
									
									for(int j = 0; j < YSTRIPS; j++)
									{
										if (xmask[i] * ymask[j] != 0) {
											gui->detectorsImageWindow->add_count_to_mask(i, j, detector_num);
										}
										//detImagemask[i][j] = xmask[i]*ymask[j];
										if (xstrips[i] * ystrips[j] != 0){
											gui->detectorsImageWindow->add_count_to_image(i, j, detector_num);
										}
									}
								}
							}
							index++;
						} else {
							//printf("No detector %d data found\n", detector_num);
							gui->app->no_trigger_count++;
							index+=33;
						}
						//printf("Detector %u:\n", detector_num);
					}
					
					//index++;
					//printf("checksum: %x\n", buffer[index]);
					//printf("index = %d, value = %x\n", index, buffer[index]);
					index++;
					//printf("index = %d, sync: %x\n", index, buffer[index]);
					index++;
					
					Fl::lock();
					gui->CommandCntOutput->value(command_count);
					char text_buffer[50];
					sprintf(text_buffer, "%x", command_value);
					gui->CommandOutput->value(text_buffer);
					
					gui->frameTime->value(time);
					gui->int_timeOutput->value(time - (gui->app->formatter_start_time));
					
					gui->HVOutput->value(high_voltage);										
					//printf("voltage: %i status: %i", voltage, HVvoltage_status);
					if (high_voltage_status == 1) {gui->HVOutput->textcolor(FL_RED);}
					if (high_voltage_status == 2) {gui->HVOutput->textcolor(FL_BLUE);}
					if (high_voltage_status == 4) {gui->HVOutput->textcolor(FL_BLACK); gui->HVOutput->redraw();}
					gui->nEventsDone->value(gui->app->frame_display_count); 
					gui->framenumOutput->value(frameNumber);
					if (attenuator_actuating == 1) {gui->shutterstateOutput->value(1);}
					Fl::unlock();
					
				} else {
					// bad checksum, skip this frame and go to the next
					//printf("Bad checksum, skipping frame!\n");
					gui->app->frame_miss_count++;
					index+=256;
				}
							
			}
			
		}
		//parsing for four frames (the whole data drop), now update
				
		Fl::lock();
		gui->tempOutput->value(temperature_convert_ref(temperature_monitors[0]));
		gui->tempOutput1->value(temperature_convert_ysi44031(temperature_monitors[1]));
		gui->tempOutput2->value(temperature_convert_ysi44031(temperature_monitors[2]));
		gui->tempOutput3->value(temperature_convert_ysi44031(temperature_monitors[3]));
		gui->tempOutput4->value(temperature_convert_ysi44031(temperature_monitors[4]));
		gui->tempOutput5->value(temperature_convert_ysi44031(temperature_monitors[5]));
		gui->tempOutput6->value(temperature_convert_ysi44031(temperature_monitors[6]));
		gui->tempOutput7->value(temperature_convert_ysi44031(temperature_monitors[7]));
		gui->tempOutput8->value(temperature_convert_ysi44031(temperature_monitors[8]));
		gui->tempOutput9->value(temperature_convert_ysi44031(temperature_monitors[9]));
		gui->tempOutput10->value(temperature_convert_ysi44031(temperature_monitors[10]));
		gui->tempOutput11->value(temperature_convert_ysi44031(temperature_monitors[11]));
		gui->frame_miss_countOutput->value(gui->app->frame_miss_count);
		
		// order is 5V, -5V, 1.5V, 3.3V		
		gui->VoltageOutput0->value(voltage_convert_5v(voltage_monitors[0]));
		gui->VoltageOutput1->value(voltage_convert_m5v(voltage_monitors[1]));		
		gui->VoltageOutput2->value(voltage_convert_15v(voltage_monitors[2]));
		gui->VoltageOutput3->value(voltage_convert_33v(voltage_monitors[3]));
		
		gui->frame_missOutput->value((float) 100 * gui->app->frame_display_count/(gui->app->frame_display_count+gui->app->frame_miss_count));
		gui->bad_frameOutput->value((float) 100 * gui->app->bad_check_sum_count/gui->app->frame_display_count);
		gui->no_triggerOutput->value((float) 100 * gui->app->no_trigger_count/gui->app->frame_display_count);
		
		Fl::unlock();
		
		pthread_mutex_lock( &timebinmutex);
		current_timebin += num_hits;
		for(int detector_number = 0; detector_number < NUM_DETECTORS; detector_number++)
		{
			gui->mainLightcurveWindow->current_timebin_detectors[detector_number] += num_hits_detectors[detector_number];
		}
		pthread_mutex_unlock(&timebinmutex);
		
		
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
	FILE *file;
	bool bresult;
	Fl_File_Chooser *chooser = NULL;
	
	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
	
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(0);
	}
	
	switch (dev->GetBoardModel()) {
		case okCFrontPanel::brdXEM3005:
			printf("Found a device: XEM3005\n");
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
	
	gui->prefs->get("formatter_configuration_file", formatter_configuration_file, "/Users/schriste/");
	
	file = fopen(formatter_configuration_file, "r");
	if (file == NULL){
		gui->app->print_to_console("Could not find gsesync.bit file\nDid you set the location of gsesyn.bit in prefs?\n");
		fclose(file);
		delete dev;
		return(0);
	}
	
	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(formatter_configuration_file)) {
		printf("formatter_configuration_file: %s\n", formatter_configuration_file);
		gui->app->print_to_console("FPGA configuration failed.\nDid you set the location of gsesyn.bit in prefs?\n");
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
