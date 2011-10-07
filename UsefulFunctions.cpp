/*
 *  UsefulFunctions.cpp
 *  Untitled
 *
 *  Created by Steven Christe on 7/22/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *
 */
#define MAXPATH 128

#include "UsefulFunctions.h"
//#include <pthread.h>
#include "gui.h"

#include "threads.h"
#include <time.h>
#include <sys/time.h>

char obsfilespec[MAXPATH];

extern int stop_message;
extern Gui *gui;
extern int stop_message;
extern unsigned short int buffer[1];
extern pthread_mutex_t mymutex;
extern unsigned short int buffer0[1];
extern int fout;

string convertBase(unsigned long v, long base)
{
	string digits = "0123456789abcdef";
	string result;
	if((base < 2) || (base > 32)) {
		result = "Error: base out of range.";
	}
	else {
		do {
			result = digits[v % base] + result;
			v /= base;
		}
		while(v);
	}
	return result;
}

unsigned getbits(unsigned x, int p, int n)
{
	// This function extracts n bits, starting at position p, from integer x.
	// The most common use is n=1 to extract a single bit at position p.
	// p=0 refers to the rightmost (LSB) bit.
	// The full description is at http://www.java-samples.com/showtutorial.php?tutorialid=500
	
	return (x >> (p+1-n)) & ~(~0 << n);
	
}

unsigned reversebits(unsigned x, int n)
{
	// This function reverses the bit order of an integer and returns it.  Only the n number of LSB are included;
	// all other bits are zero.
	
	// example for 5 bits; the code does:
	// getbits(x,4,1)*1 + getbits(x,3,1)*2 + getbits(x,2,1)*4 + getbits(x,1,1)*8 + getbits(x,0,1)*16
	
	unsigned y = 0;
	
	for(int i=0; i<n; i++){
		
		y = y + getbits(x, n-i-1, 1)*pow(2,i);
		//		cout << y << '\t';  // debug
		
	}
	
	return y;
	
}

void* testfunction(void* p)
{
	ssize_t wlen;
	
	Fl::lock();

	Fl::awake();
	Fl::unlock();
	// This function does nothing useful. It is a test function for multithreading.
	//return j;
	startfile();
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

void* watchfunction(void* p)
{
	while(1)
	{
		// update the display
		pthread_mutex_lock( &mymutex); /* wait on readgse */
		Fl::lock();

		pthread_mutex_unlock(&mymutex);
		Fl::awake(); // Without this it may not redraw until next event (like a mouse movement)!
		Fl::unlock();
		
		if (stop_message == 1){
			pthread_exit(NULL);
		}
	}
}

void startfile(void)
{
	// Open a file to write the data to
	// file pointer is set to fout
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
