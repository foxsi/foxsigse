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

extern Gui *gui;

char obsfilespec[MAXPATH];
int newdisplay;
int displayonce,displaycont;
int stop_message;
unsigned short int buffer[1];
unsigned short int buffer0[1];
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

			displayonce = 0;
			printf("watch_buffer: buffer0 = %hu\n", buffer0[0]);
			gui->testOutput->value(buffer0[0]);
			newdisplay = 0;
			fflush(stdout);
		
			pthread_mutex_unlock(&mymutex);
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
	/* Read the data in continuously. Data is moved from the buffer0 variable
	 * to buffer and will write the data to a file is needed.
	 */
	ssize_t wlen;
	displayonce = 0;
	displaycont = 1;
	
	// This function never stops unless told to do so by setting stop_message to 1
	while (1) {

		// read the data
		data_simulate_data();
		if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
		{
			if (newdisplay == 0)
			{
				memcpy((void *) buffer0,(void *) buffer,2);
				newdisplay = 1;
			}
			pthread_mutex_unlock(&mymutex);
		}
		
		if(fout >0)
		{
			if( (wlen = write(fout,(const void *) buffer0,2048) ) != 2048){};
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
	//printf("%hd ", (arc4random() % 10));
	buffer[0] = (unsigned short int) (arc4random() % 10);
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
}
