/*
 *  FoxsidataSI.cpp
 *  Untitled
 *
 *  Created by Steven Christe on 7/20/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *
 */

//To Do
//Add a write file command
//add each frame into memory so reading backwards is easy


#include "Foxsidata.h"

#include <string.h>
#include "UsefulFunctions.h"
#include "gui.h"
using namespace std;

#define XSTRIPS 128
#define YSTRIPS 128

// unsigned short int has 16 bit 
//frame size in units of words

#define WORD 16
#define FRAMESIZE 295
#define ASIC_FRAMESIZE 73
#define NUM_ASICS 4
#define NUM_STRIPS 64

extern Gui *gui;

Foxsidata::Foxsidata() 
{
	//listofPhotons = new ListOfPhotons();
	
	ChanneltoEnergySlope = 1.0;
	ChanneltoEnergyIntercept = 0.0;
	tick = 0;
	
	//initialize the image
	for(int i=0;i<XSTRIPS;i++)
	{
		for(int j=0;j<YSTRIPS;j++)
	   	{
			image[i][j] = exp(-(pow((float) i-XSTRIPS/2,2) + pow((float) j-YSTRIPS/2,2))/(0.5*XSTRIPS));
		}
	}
	image[15][15] = 1.0;
	image[75][75] = 0.5;
	currentPosition = 0;
	currentFrame = 0;
}

Foxsidata::~Foxsidata()
{
	//cleanup
	fclose (file);
	//delete listofPhotons;
}

void Foxsidata::addPhoton(double channel, double time, int detector, int xstrip, int ystrip, int asic)
{
	int energy;	
	//listofPhotons->Add(channel, time, detector, xstrip, ystrip, asic);
	
	//update histogram, each bin in histrogram is assumed to be 1 keV
	energy = ChanneltoEnergy(channel);
	histogram[energy][detector]++;
	// update the overall histogram
	histogram[energy][0]++;
	
	//begin calculate rate
	if (tick == 5) 
	{
		rate = (5.0-1.0)/(time - t0);
		tick = 0;
	}
	if (tick == 0)
	{
		t0 = time;
	}
	tick++;
	//end calculate rate
	
	//cout << tick << endl;
	//cout << t0 << endl;
	//cout << "time:" << time << endl;
	
	//update image
	//image[xstrip][ystrip][detector]++;
	image[xstrip][ystrip]++;
	
	//update overall image, currently assumes the pixels line up (no tilt between detectors)
	//image[xstrip][ystrip][0]++;
}

void Foxsidata::setChanneltoEnergy(double m, double b)
{
	ChanneltoEnergySlope = m;
	ChanneltoEnergyIntercept = b;
}

int Foxsidata::ChanneltoEnergy(double channel)
{
	return (int) channel*ChanneltoEnergySlope + ChanneltoEnergyIntercept;
}

int Foxsidata::EnergytoChannel(double energy)
{
	return (int) (energy - ChanneltoEnergyIntercept)/ChanneltoEnergySlope;
}

void Foxsidata::FlushHistogram(void)
{
	gui->mainHistogramWindow->flush(7);
	gui->mainHistogramWindow->redraw();
}

void Foxsidata::FlushImage(void)
{
	gui->mainImageWindow->redraw();
	//gui->subImageWindow->redraw();
}

void Foxsidata::Properties(void) const
{
	cout << "---Foxsidata Properties---" << endl;
	//cout << "Number of Photons: " << listofPhotons->Count() << endl;
	//listofPhotons->Properties();
	cout << "Rate: " << getRate() << endl;
}

/*void Foxsidata::simulateData(void)
 {
 int	numphotons;
 struct timeval tv;
 struct timezone tz;
 struct tm *tm;
 double museconds;
 double seconds;
 
 gettimeofday(&tv, &tz);
 tm=localtime(&tv.tv_sec);
 
 museconds = tv.tv_usec;
 
 srand ( time(NULL) );
 
 for(int i = 0; i < 10; i++)
 {
 numphotons = rand() % 7 + 5;
 
 for(int j = 0; j < numphotons; j++)
 {
 int detector, channel;
 detector = rand() % 7 + 1;
 channel = rand() % 128 + 1;
 
 tm=localtime(&tv.tv_sec);
 gettimeofday(&tv, &tz);
 
 museconds = tv.tv_usec;
 seconds = tv.tv_sec;
 
 cout << "museconds--" << seconds+museconds/1e6 << endl;
 listofPhotons->Add(channel,seconds+museconds/1e6,2, 0, 0, 0);
 }
 
 //cout << "---Data Rate---" << getRate() << endl;
 
 }
 Properties();
 
 }
 */

float Foxsidata::getImage(void)
{
    //really should return a pointer to the Foxsidata and should include a way to pull only a particular image or histogram
	//return image;
	return 0.0;
}

//int Foxsidata:getHistogram(void)
//{
//	return histogram;
//}

double Foxsidata::getRate(void) const
{
	return rate;
}

/*
 *  readData.cpp
 *  Untitled
 *
 *  Created by Steven Christe on 7/7/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *  This function reads data in from a USB stream type file.
 */

void Foxsidata::readDatafile(char* filename)
{
	long fileSize;
	size_t result;
	char text[8];
	
	//struct asic_data adata;
	//struct asic_frame aframe;
	
	file = fopen ( filename , "rb" );
	if (file==NULL) {fputs ("File error",stderr); exit (1);}
	
	//this section does not seem to load the frame in correctly...do it by hand
	//asic_frame *buffer;
	//result = fread (buffer,sizeof(asic_frame),1,file);
	
	// obtain file size in units of bytes:
	fseek (file , 0 , SEEK_END);
	fileSize = ftell (file);
	rewind (file);
	
	// allocate memory to contain the whole file:
	ASICfile_wordbuffer = (unsigned short int*) malloc (sizeof(unsigned short int)*fileSize);
	if (ASICfile_wordbuffer == NULL) {fputs ("Memory error",stderr); exit (2);}
	
	// copy the file into the ASICfile_wordbuffer:
	result = fread (ASICfile_wordbuffer,1,fileSize,file);
	if (result != fileSize) {fputs ("Reading error",stderr); exit (3);}  
	
	
	currentFrame = 0;
	currentPosition = 0;
	//clear out the variables	
	FlushImage();
	FlushHistogram();
	
	parseBuffer();
	cout << "Filesize : " << fileSize << endl;
	cout << "Current Frame : " << currentFrame << " : current position : " << currentPosition << endl;
	gui->framenumOutput->value(currentFrame);
	
}

void Foxsidata::nextFrame()
{
	char text[8];
	
	currentFrame++;
	gui->framenumOutput->value(currentFrame);
	
	parseBuffer();
}

void Foxsidata::previousFrame()
{
	char text[8];
	
	currentFrame--;
	gui->framenumOutput->value(currentFrame);
	currentPosition = currentPosition - FRAMESIZE-1;
	
	parseBuffer();
}

//parse the buffer which contains the whole file
//the second variable gives the position of the buffer to parse at in integer
//values of the FRAMESIZES
//i.e. position = 0, read the first frame, position = 1 read the second frame, etc.
void Foxsidata::parseBuffer()
{
	//char text[8];
	unsigned int xmask;
	unsigned int ymask;	
	
	//STRIP_DATA strip;
	strip_data strip;
	
	//check for data sync, if synced light the sync button and read the frame
	//if ((ASICfile_wordbuffer[0 + currentPosition] == 0xeb90) && (ASICfile_wordbuffer[1 + currentPosition] == 0xeb90))
	//{gui->syncLightBut->value(1);} 
	//else 
	//{
		//if not sync then try to sync
		//gui->syncLightBut->value(0); 
	//	syncBuffer();
	//}
	
	cout << "Beginning of frame at position " << currentPosition << endl;
	cout << "Parsing..." << endl;
	
	current_ASICframe.sync1 = ASICfile_wordbuffer[currentPosition + 0 ];
	current_ASICframe.sync2 = ASICfile_wordbuffer[currentPosition + 1];
	current_ASICframe.det_time = ASICfile_wordbuffer[currentPosition + 2];
	
	gui->frameTime->value(current_ASICframe.det_time);
	
	/* //asic data
		for(int i = 0; i<NUM_ASICS; i++)
		{
			current_ASICframe.adata[i].chip_bit = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 3 + currentPosition];
			current_ASICframe.adata[i].trig_bit = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 4 + currentPosition];
			current_ASICframe.adata[i].seu_bit = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 5 + currentPosition];
			
			current_ASICframe.adata[i].asic_mask0 = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 6 + currentPosition];
			current_ASICframe.adata[i].asic_mask1 = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 7 + currentPosition];
			current_ASICframe.adata[i].asic_mask2 = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 8 + currentPosition];
			current_ASICframe.adata[i].asic_mask3 = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 9 + currentPosition];
			
			current_ASICframe.adata[i].noise = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 10 + currentPosition];
			for(int j = 0; j<NUM_STRIPS; j++)
			{
				strip = * (strip_data*) (&ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 11 + j + currentPosition]);
				
				current_ASICframe.adata[i].sdata[j].number = strip.number;
				current_ASICframe.adata[i].sdata[j].data = strip.data;
				
				//now update the functions
			}
			current_ASICframe.adata[i].pedestal = ASICfile_wordbuffer[ASIC_FRAMESIZE*i + 11 + NUM_STRIPS + currentPosition];
		}
		
		
			
			for(int j=0;j<YSTRIPS;j++){
				if( j==0 ){ ymask = current_ASICframe.adata[j / (YSTRIPS/2)+2].asic_mask0;}
				if( j==1 ){ ymask = current_ASICframe.adata[j / (YSTRIPS/2)+2].asic_mask1;}
				if( j==2 ){ ymask = current_ASICframe.adata[j / (YSTRIPS/2)+2].asic_mask2;}
				if( j==3 ){ ymask = current_ASICframe.adata[j / (YSTRIPS/2)+2].asic_mask3;}
				
				// update the Image Mask
				detImagemask[i][j] = getbits(xmask, XSTRIPS/8 - i % (XSTRIPS/8)-1,1) * getbits(ymask, YSTRIPS/8 - j % (YSTRIPS/8)-1,1);
				
				//update the Image
				detImage[i][j] += sqrt(current_ASICframe.adata[i / (XSTRIPS/2)].sdata[i % (XSTRIPS/2)].data * current_ASICframe.adata[(j / (YSTRIPS/2))+2].sdata[j % (YSTRIPS/2)].data);
				//detImage[i][j] = exp(-(pow((float) i-XSTRIPS/2,2) + pow((float) j-YSTRIPS/2,2))/(0.5*XSTRIPS));
			}
		}
		
		currentPosition = ASIC_FRAMESIZE*(NUM_ASICS-1) + 11 + NUM_STRIPS + currentPosition;
		
		//done loading the one frame
		//printFrame();
		cout << "End of frame at position " << currentPosition << endl;
		//now update the windows
		gui->subImageWindow->redraw();
		gui->mainImageWindow->redraw();
		gui->mainHistogramWindow->redraw(); */
}

void Foxsidata::syncBuffer(void)
{
	//need to add code to deal with reaching the end of the file!
	
	//try to sync with data again
	// gui->syncLightBut->value(0);
	cout << "Seeking sync..." << endl;
	while ((ASICfile_wordbuffer[0 + currentPosition] != 0xeb90) && (ASICfile_wordbuffer[1 + currentPosition] != 0xeb90))
	{
		cout << "Current position " << currentPosition << " contents (base 16) " << convertBase(ASICfile_wordbuffer[0 + currentPosition], 16);
		cout << " " << convertBase(ASICfile_wordbuffer[1 + currentPosition], 16) << endl;
		currentPosition++;
	} 
	currentPosition++;
	// gui->syncLightBut->value(1);
}

void Foxsidata::printFrame(void)
{	
	//print out the frame
	cout << "==== Frame " << currentFrame << " ==== current pos " << currentPosition << endl;
	cout << "Sync 1 : " << convertBase(current_ASICframe.sync1,16) << endl;
	cout << "Sync 2 : " << convertBase(current_ASICframe.sync2,16) << endl;
	cout << "Detector Time : " << current_ASICframe.det_time << endl;
	cout << "ASIC Data " << endl;
	for(int i = 0; i<4; i++)
	{
		cout << "Chip Bit : " << current_ASICframe.adata[i].chip_bit << endl;
		cout << "Trig Bit : " << current_ASICframe.adata[i].trig_bit << endl;
		cout << "SEU Bit : " << current_ASICframe.adata[i].seu_bit << endl;
		cout << "MASK " << endl;
		
		//for(int j=0; j<XSTRIPS; j++){ cout << convertBase(current_ASICframe.adata[i].asic_mask[j],2) << endl; }
		cout << convertBase(current_ASICframe.adata[i].asic_mask0,2) << endl;
		cout << convertBase(current_ASICframe.adata[i].asic_mask1,2) << endl;
		cout << convertBase(current_ASICframe.adata[i].asic_mask2,2) << endl;
		cout << convertBase(current_ASICframe.adata[i].asic_mask3,2) << endl;
		cout << "Noise : " << current_ASICframe.adata[i].noise << endl;
		cout << "Pixels Number" << endl;
		for(int j = 0; j<64; j++)
		{
			cout << current_ASICframe.adata[i].sdata[j].number << " ";
		}
		cout << endl;
		cout << "Pixels Data" << endl;
		for(int j = 0; j<64; j++)
		{
			cout << current_ASICframe.adata[i].sdata[j].data << " ";
		}
		cout << endl;
		cout << "Pedestal : " << current_ASICframe.adata[i].pedestal << endl;
	}
	//cout << "Blank: " << current_ASICframe.blank << endl;	
}

