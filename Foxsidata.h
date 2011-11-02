/*
 *  FoxsidataSI.h
 *  Untitled
 *
 *  Created by Steven Christe on 7/20/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *
 */

#ifndef _FoxsidataSI_h_
#define _FoxsidataSI_h_

#define XSTRIPS 128
#define YSTRIPS 128
#define NUMDETECTORS 7

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "UsefulFunctions.h"

// Description of a frame - describes in bit fields
struct strip_data  // 2 bytes
{
	unsigned data : 10;
	unsigned number : 6;
};

struct asic_data // 146 bytes
{
	unsigned chip_bit : 16;
	unsigned trig_bit : 16;
	unsigned seu_bit : 16;
	unsigned asic_mask0 : 16;
	unsigned asic_mask1 : 16;
	unsigned asic_mask2 : 16;
	unsigned asic_mask3 : 16;
	unsigned noise : 16;
	strip_data sdata[64];
	unsigned pedestal : 16;
};

struct asic_frame // 592 bytes
{
	unsigned sync1 : 16;  // (16 bits) 
	unsigned sync2 : 16;
	unsigned frame_num : 16;
	unsigned det_time : 16;
	asic_data adata[4];
	// unsigned blank : 16;
};

class Foxsidata
	{
		// Builder
	public:
		Foxsidata();
		~Foxsidata();
		//ListOfPhotons();
		
		//methods for reading an ASIC file
		void readDatafile(char* filename);
		void parseBuffer(void);
		void nextFrame(void);
		void previousFrame(void);
		void printFrame(void);
		void syncBuffer(void);
		
		void setChanneltoEnergy(double m, double b);
		int ChanneltoEnergy(double channel);
		int EnergytoChannel(double energy);
		
		void addPhoton(double channel, double time, int detector, int xstrip, int ystrip, int asic);
		void Properties(void) const;
		int getHistogram(void) const;
		double getRate(void) const;
		float getImage(void);
		void FlushImage(void);
		void FlushHistogram(void);
		//void simulateData(void);
	private:
		double ChanneltoEnergySlope;
		double ChanneltoEnergyIntercept;
		int    histogram[XSTRIPS][NUMDETECTORS+1];   //
		double	rate;  	// rate is in counts per second
		float image[XSTRIPS][YSTRIPS];   // image is in counts
		int tick;	// used to calculate rate, counter for number of photons
		double t0;	//used to calculate rate, initial time
		
		FILE* file;
		unsigned short int* ASICfile_wordbuffer;  // 16 bits
		unsigned long currentPosition;
		int currentFrame;
		
		//variables for USB file
		strip_data current_strip;
		asic_frame current_ASICframe;
		
	};

#endif
