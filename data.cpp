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
#define MAXPATH 128

char obsfilespec[MAXPATH];
int newdisplay;
int displayonce,displaycont;
extern int stop_message;
extern Gui *gui;
extern int stop_message;
unsigned short int buffer[1];
unsigned short int buffer0[1];

extern pthread_mutex_t mymutex;
extern int fout;

void* watch_buffer(void* p)
{
	/* Watches the buffer variable for changes. When changes occur, display 
	 * them on the GUI.
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

void* read_data2(void *p)
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
		simulate_data();
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

void simulate_data(void)
{
	//printf("%hd ", (arc4random() % 10));
	buffer[0] = (unsigned short int) (arc4random() % 10);
}

void start_file(void)
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

void* read_gse(void* p)
{
	ssize_t wlen;
	
	Fl::lock();
	
	Fl::awake();
	Fl::unlock();
	//return j;
	//start_file();
	
	for(unsigned short int i; ;i++ )
	{
		// do some work
		usleep(5000);
		buffer0[0] = i;
		
		// write to file
		if(fout > 0)
		{
			if( (wlen = write(fout,(const void *) buffer0,2) ) != 2){};
		}
		
		// copy the result to the global buffer variable to share it
		if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
		{
			// an unsigned short int is 2 bytes
			memcpy( (void *)buffer, (void *)buffer0,1);			
			pthread_mutex_unlock(&mymutex);
		}
		
		// check if should stop
		if (stop_message == 1){
			if (fout > 0)
				close(fout);
			Fl::lock();
			
			Fl::unlock();
			Fl::awake(p);
			pthread_exit(NULL);
		}
	}
	// Obtain a lock before we access the browser widget...
	Fl::lock();
	
	Fl::unlock();
	
	// Send a message to the main thread, at which point it will
	// process any pending redraws for our browser widget.  The
	// message we pass here isn't used for anything, so we could also
	// just pass NULL.
	Fl::awake(p);
	
	return 0L;
}

