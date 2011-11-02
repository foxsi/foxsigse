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

#define MAXPATH 128
#define NUM_THREADS 8
#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024

extern int HistogramFunction[MAX_CHANNEL];
extern double detImage[XSTRIPS][YSTRIPS];
extern double detImagealpha[XSTRIPS][YSTRIPS];

// Note that an unsigned short int is 2 bytes
#define FRAME_SIZE_IN_SHORTINTS 295
#define FRAME_SIZE_IN_BYTES 590

extern Gui *gui;

char obsfilespec[MAXPATH];
int newdisplay;
int stop_message;
unsigned short int buffer[FRAME_SIZE_IN_SHORTINTS];
unsigned short int buffer0[FRAME_SIZE_IN_SHORTINTS];
unsigned short int framecount;

int *taskids[NUM_THREADS];
int fout;
pthread_mutex_t mymutex;

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
			
			data_frame_printf(buffer0);
			data_update_display(buffer0);
			
			newdisplay = 0;
			fflush(stdout);
		
			pthread_mutex_unlock(&mymutex);
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
	// This function never stops unless told to do so by setting stop_message to 1
	while (1) {

		// read the data
		data_simulate_data();
		
		if(fout >0)
		{
			if( (wlen = write(fout,(const void *) buffer,2048) ) != 2048){};
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
		
		if (stop_message == 1){
			if (fout > 0)
				close(fout);
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

	struct strip_data  // 2 bytes
	{
		unsigned data : 10;
		unsigned number : 6;
	};
	
	strip_data strip;
	
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

	char stringtemp[80];
	struct tm *times;
	time_t ltime;
	
	time(&ltime);
	times = localtime(&ltime);
	strftime(stringtemp,25,"data_%Y%m%d_%H%M%S.dat",times);
	strncpy(obsfilespec,stringtemp,MAXPATH - 1);
	obsfilespec[MAXPATH - 1] = '\0';
	printf("%s \r",obsfilespec);
	{
		if((fout = open(obsfilespec,O_RDWR|O_CREAT,0600)) < 0)
			printf("Cannot open file\n");
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
	pthread_mutex_destroy(&mymutex);
	if (fout > 0) {
		close(fout);
	}
}

void data_frame_printf(unsigned short int *frame)
{
	printf("watch_buffer: frame[0] = %hu\n frame[1] = %hu\n", frame[0], frame[1]);
}

void data_update_display(unsigned short int *frame)
{
	unsigned short int tmp;
	
	struct strip_data  // 2 bytes
	{
		unsigned data : 10;
		unsigned number : 6;
	};
	strip_data strip;
	
	memcpy(&strip,&buffer[29],2);
	
	gui->framenumOutput->value(frame[5]);
	gui->testOutput->value(strip.number);
	
	HistogramFunction[strip.data]++;
	tmp = arc4random() % 64 + 1;
	detImage[tmp][strip.number] = 1;
	detImagealpha[tmp][strip.number] = clock();
	// detImagemask[i][j] = getbits(xmask, XSTRIPS/8 - i % (XSTRIPS/8)-1,1) * getbits(ymask, YSTRIPS/8 - j % (YSTRIPS/8)-1,1);
	
}
	
