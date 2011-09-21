/*
	FOXSI USB GSE USB Package ----------------------------------------
	
	PURPOSE : Library for the USB D2xx class

	REQUIRED : 
 
 	WRITTEN : Steven Christe (18-Dec-2009), based on code by L. Glesener
	MODIFIED Oct-2010 by Lindsay

	NOTES : This class is shared with the full GSE program.  The commented out lines of 
	code are used by the GSE program, please do not delete!
	
	TO DO : Read the to do in the .h file and under each of the function definitions below.
  	
*/

#include <iostream>
#include "ftd2xx.h"
#include "usbd2xx.h"

using namespace std;

#include "gui.h"
extern Gui *gui;

#define XSTRIPS 128
#define YSTRIPS 128
#define CHANNELS 1024

extern int HistogramFunction[CHANNELS];
extern double detImage[XSTRIPS][YSTRIPS];
extern double detImagemask[XSTRIPS][YSTRIPS];

// default constructor method
USB_d2xx::USB_d2xx()
{
	nBytesFrame = 624;
	frameData = new unsigned short int [nBytesFrame];
	ftHandle = NULL;
}

// construct with initializer
USB_d2xx::USB_d2xx( const int n)
{
	nBytesFrame = n;
	frameData = new unsigned short int [n];
	ftHandle = NULL;
}

int USB_d2xx::open(void)
{	
	// Define variables:
	
	char * 	pcBufLD[2];			// Pointer to cBufLD
	char 	cBufLD[1][64];		// Holds the device serial numbers
//	FILE *	dataFile;
//	char	dataFileName[20];
	unsigned int lostbytes = 0, framesread = 0;
	int	iNumDevs = 0;	//the number of found devices (should only be 1)
	
	// Initialize pcBufLD.  This points to the character string where the device list is stored.
	pcBufLD[0] = cBufLD[0];
	pcBufLD[1] = NULL;
	
	// Initialize USB connection (with errors)
	// first list the devices
	ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);	
	//check if status is okay, if not display an error message
	if(ftStatus != FT_OK) {
		char buffer[50];
		sprintf(buffer, "Error: FT_ListDevices(%d)\n", ftStatus);
		gui->consoleBuf->insert(buffer);
		cout << buffer << endl;
		return -1;
	}
	//if status okay then print out list of found devices
	for(int i = 0; i < iNumDevs; i++) {
		char buffer[50];
		sprintf(buffer, "Found Device %d Serial Number - %s\n", i, cBufLD[i]);
		gui->consoleBuf->insert(buffer);
		cout << buffer << endl;
	}
	
	//status is okay; now open the device
	/* Setup */
	if((ftStatus = FT_OpenEx(cBufLD, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle)) != FT_OK){
		/* 
		 This can fail if the ftdi_sio driver is loaded
		 use lsmod to check this and rmmod ftdi_sio to remove
		 also rmmod usbserial
		 */
		char buffer[50];
		sprintf(buffer, "Error FT_OpenEx(%d), device\n", ftStatus);
		gui->consoleBuf->insert(buffer);
		cout << buffer << endl;

		return -1;
	}
	
	//	printf("Opened device %s\n", cBufLD);
	//FIX this code below
	//gui->consoleBuf->insert("Opened device %s\n", cBufLD);
	
	//fthandle holds the pointer to the device
	// Set data transfer rate to 9600
	if((ftStatus = FT_SetBaudRate(ftHandle, 9600)) != FT_OK)
	{
		char buffer[50];
		sprintf(buffer, "Error FT_SetBaudRate(%d), cBufLD = %s\n", ftStatus, cBufLD);
		gui->consoleBuf->insert(buffer);
		cout << buffer << endl;

		return -1;
	}
	// Set read and write timeouts, in millisec.  (for now, arbitrary value of 1 second)
	if((ftStatus = FT_SetTimeouts(ftHandle, 1000, 1000)) != FT_OK)
	{
		char buffer[50];
		sprintf(buffer, "Error FT_SetTimeouts(%d)\n", ftStatus);
		gui->consoleBuf->insert(buffer);
				cout << buffer << endl;

		return -1;
	}
	if((ftStatus = FT_Purge(ftHandle, 3)) != FT_OK)	// Purge write and read buffers
		printf("Error FT_Purge(%d)\n", ftStatus);
	
	//device is now open and ready!

	// Open data file
	dataFile = fopen("test.dat", "at");	// later, change this to an optional name.
	
	return 1;
}

int USB_d2xx::findSync(void)
{
	DWORD	nBytes = 2;		// number of bytes to read
	DWORD	nBytesRead = 0; // actual number of bytes read	
	unsigned short int dataByte = 0;
	int		i = 0;			// keep track of bytes read before sync word is found
	int		iMax = 4*nBytesFrame;	// maximum times to try
	
	ftStatus = FT_SetTimeouts(ftHandle, 500, 500);
	
	while ( ( dataByte != 0xEB90) && (i < iMax) ) {
		i++;
		ftStatus = FT_Read(ftHandle, &dataByte, nBytes, &nBytesRead);
		//cout << "Attempt " << i << " value " << dataByte << " nBytesRead " << nBytesRead << endl;
		if (ftStatus != FT_OK) {
			cout << "Error FT_Read(" << ftStatus << ")" << endl;
			return -1;
		}
	}
	
	if ( i < iMax ) {
		cout << "Found sync byte after " << i << " attempts." << endl;
		return 0;
	}
	else {
		cout << "Max sync attempts; no sync byte found." << endl;
		return -1;
	}
}


int USB_d2xx::readFrame(void)
{
	/* Read */
	int		nFrames = 1;
	DWORD	nBytesToRead = nFrames*nBytesFrame;
	DWORD 	nBytesRead = 0;	// actual number of bytes read.

	char buffer[50];

	ftStatus = FT_GetQueueStatus(ftHandle, &nBytesRead);	// probably not needed.
	ftStatus = FT_SetTimeouts(ftHandle, 500, 500);
	
	for(int i=0; i<nBytesToRead; i++)  frameData[i] = 0;	// initialize buffer
	
	if(ftStatus == FT_OK) {

		cout << "Attempting to read " << nBytesToRead << " bytes." << endl;
		sprintf(buffer, "Attempting to read %d bytes.\n", nBytesToRead);
		gui->consoleBuf->insert(buffer);

		if((ftStatus = FT_Read(ftHandle, frameData, nBytesToRead, &nBytesRead)) != FT_OK){
			sprintf(buffer, "Error FT_Read(%d)\n", ftStatus);
			gui->consoleBuf->insert(buffer);
			cout << buffer << endl;

		}
		//else {			
		//	sprintf(buffer, "Read %d bytes.", nBytesRead);
			//gui->consoleBuf->insert(buffer);
		//			cout << buffer << endl;


		//}
	} else {
		gui->consoleBuf->insert("Could not get USB queue status.\n");
				cout << "Could not get USB queue status." << endl;
	}
	
	return nBytesRead;
	
	gui->mainImageWindow->redraw();
	gui->subImageWindow->redraw();
	gui->mainHistogramWindow->redraw();
}

void USB_d2xx::close(void)
{
	if(ftHandle != NULL) {
		FT_Close(ftHandle);
		ftHandle = NULL;
		cout << "Closed device." << endl;
		fclose(dataFile);
		
		gui->consoleBuf->insert("Closing connection.\n");

	}
	//gui->usbReadBut->deactivate();
	//gui->usbCloseBut->deactivate();
}

void USB_d2xx::printFrame(void)
{
	// Data are stored as 16-bit words (short int)
	
//	unsigned short int data[2];
	int nData = 78; // n bytes per frame.
	int pixel_value = 0;
	int ydata[YSTRIPS];
	int xdata[XSTRIPS];
	unsigned int xmask[XSTRIPS];
	unsigned int ymask[YSTRIPS];
	int good = 0;

	// Loop through the 4 ASICS
	for(int j=0; j<4; j++)
	{
		int index = j*nData;	// used for counting words in the data packet
		// temporary variables to hold values from the data stream
		int chipBit, trigBit, seuBit, commonMode, pedestal;
		int mask1, mask2, mask3, mask4;
				
		cout << "\n ASIC " << j << endl << endl;
		
		// Sync word again.
		cout << "Sync word:\t\t"		<< hex << uppercase	<< frameData[ index ] << endl;	index++;
		cout << "Detector time:\t\t"	<< dec << frameData[ index ] << endl;	index++;
		cout << "Start bit:\t\t"		<< frameData[ index ] << endl;	index++;

		cout << "Chip data bit:\t\t"	<< frameData[ index ] << endl;
		chipBit = frameData[ index ];		
		index++;

		cout << "Analog Trigger bit: "	<< frameData[ index ] << endl;
		trigBit = frameData[ index ];
		index++;

		cout << "SEU bit:\t\t"			<< frameData[ index ] << endl;
		seuBit = frameData[ index ];
		index++;
		
		// channel mask displayed in hex
		// First two channel mask bits are stored as one word each.
		cout << "Channel mask:\t\t:"	<< hex << uppercase << frameData[ index ]; index++;
		cout << frameData[ index ]; index++;
		cout << frameData[ index ]; mask1 = frameData[ index ]; index++;
		cout << frameData[ index ]; mask2 = frameData[ index ]; index++;
		cout << frameData[ index ]; mask3 = frameData[ index ]; index++;
		cout << frameData[ index ] << endl; mask4 = frameData[ index ]; index++;
		
		cout << "Common mode:\t\t"	<< dec << frameData[ index ] << endl;
		commonMode = frameData[ index ];
		index++;
		
//		for(int i=0; i<64; i++){
//			cout << "Strip " << i << ":\t\t" << frameData[ index ] << endl;	index++;
//		}
		
		cout << endl << mask1 << endl << mask2 << endl << mask3 << endl << mask4 << endl << seuBit << endl << endl;
		
		if((mask1 == 65535) && (mask2 == 65535) && (mask3 == 65535) && (mask4 == 65535) && (seuBit == 0)){
			good++;
		for(int i=0; i< XSTRIPS/2; i++){	
			pixel_value = (int)(frameData[ index ]);// - (unsigned short int)commonMode;
			cout << "Strip " << i << " data:\t" << (pixel_value) << endl;
			
			// Update the Histogram but limit values to 1024
			// only update for the first two ASICs (p-side)
			if (j == 0 || j == 1){HistogramFunction[pixel_value < 1024 ? pixel_value : 1024]++;}
			
			//// update for both p-side and n-side
			//HistogramFunction[pixel_value < 1024 ? pixel_value : 1024]++;
			
			// Update the  but limit values to 1024
			if (j == 0 || j == 1) {
			xdata[i + j*XSTRIPS/2] = pixel_value < 1024 ? pixel_value : 1024;	}
//			xmask[i + j*XSTRIPS/2] = getbits(frameData[j*nData+3+i], i, 1);
			
			if (j == 2 || j == 3){
//				if(j == 3) pixel_value = 20;
				ydata[i + (j-1)*YSTRIPS/2] = pixel_value > 0 ? pixel_value : 0;	
				ydata[(j-1)*YSTRIPS/2 - i - 1] = pixel_value > 0 ? pixel_value : 0;		// FIX THIS!!!
			}
			
			index++;
		}
		}
		
		cout << "Pedestal:\t\t" << frameData[index] << endl; index++;

		switch (j) {
			case 0:
				gui->chipbitValOut0->value(chipBit);
				gui->trigbitValOut0->value(trigBit);
				gui->seubitValOut0->value(seuBit);
				gui->noiseValOut0->value(commonMode);
				break;
			case 1:
				gui->chipbitValOut1->value(chipBit);
				gui->trigbitValOut1->value(trigBit);
				gui->seubitValOut1->value(seuBit);
				gui->noiseValOut1->value(commonMode);
				break;
			case 2:
				gui->chipbitValOut2->value(chipBit);
				gui->trigbitValOut2->value(trigBit);
				gui->seubitValOut2->value(seuBit);
				gui->noiseValOut2->value(commonMode);
				break;
			case 3:
				gui->chipbitValOut3->value(chipBit);
				gui->trigbitValOut3->value(trigBit);
				gui->seubitValOut3->value(seuBit);
				gui->noiseValOut3->value(commonMode);
				break;
			default:
				break;
		}

		frame[j] = * (asic_dataframe*) (&frameData[ j*nData ]);
		cout << "start bit:" << frame[j].start_bit << endl;
		cout << "chip bit:" << frame[j].chip_bit << endl;
		cout << "trig bit:" << frame[j].trig_bit << endl;
		cout << "seu bit:" << frame[j].seu_bit << endl;
		cout << "pedestal_bit:" << frame[j].pedestal_bit << endl;
		cout << "asic mask:" << frame[j].asic_mask << endl;
		cout << "common mode:" << frame[j].common_mode << endl;
		cout << "noise:" << frame[j].noise << endl;
		
		/*  Some leftover code
		
		// first two ASICs are the p-side ASICs and give the energy measurements
		// define these as the X strips
		
		cout << "Pedestal value:\t" << frameData[j*nData+72] << endl;
		
		data[0] = frameData[ j*nData+73] % 0x100;
		data[1] = frameData[ j*nData+73] / 0x100;

		printf("Stop bit\t%u\n", data[0]);
		printf("\n\nSYNC WORD\t\t%X\n", data[1]);	
						
		//for( int k = 0; k<64; k++){
		//	cout << k << " " << ;
		//update displays
		//HistogramFunction[dataWord%1024/10]++;
					
		//for(int j=0;j<XSTRIPS;j++)
		//{
		//	detImage[i-8][j] = dataWord%1024/10;
		//}
					
		//sprintf(buffer, "%u", dataWord%1024);
					//gui->consoleBuf->insert(buffer);
		//			cout << buffer << endl;
	}

*/
	}
		
	for (int i = 0; i<XSTRIPS; i++) {
		for (int j = 0; j<YSTRIPS; j++) {
			detImage[i][j] = xdata[i]*ydata[j];
			// If the glitch button is not ON then multiply the image by the 
			// mask image to kill the bad pixel data
			if (gui->glitchBut->value() == 0) {
			//detImage[i][j] *= xmask[i]*ymask[j];}
			detImage[i][j] *= xmask[i]*ymask[j];}
			if(i == 0 && j == 0) detImage[i][j]=1000;  // set scale of intensity plot
		}
	}
		
	cout << "good = " << good << endl << endl;
	
	if(good >= 3){
		gui->mainHistogramWindow->redraw();
		gui->mainImageWindow->redraw();
	}
}

void USB_d2xx::writeFrame(FILE *dataFile)
{
	// This function is very close to the last one except that data is printed to file instead of screen.
	// Also identifiers like "start bit", "strip 1 data", etc are not written.
	
	// To do: 1) modify this so that it's exactly like the packet we agreed on.
	//	      2) finish upgrading stray c commands to c++.
	// NOTES FROM NOV 15 2010:
	//		-- removed writing of all non-data bits (header, mask, sync, etc)
	//		-- for now, writing p- and n-side in two different files
	
	unsigned short int data[2];
	int nData = 74; // n bytes per frame.
	
	// Loop through the 4 ASICS
	for(int j=0; j<4; j++)
	{
		fprintf( dataFile, "\n\n" );
		
		// Header information, one bit each.
		data[0] = frameData[ j*nData ] % 0x100;
		data[1] = frameData[ j*nData ] / 0x100;
		
		fprintf( dataFile, "%u\t%u\t", data[0], data[1]);
		
		data[0] = frameData[ j*nData+1] % 0x100;
		data[1] = frameData[ j*nData+1] / 0x100;
		
		fprintf( dataFile, "%u\t%u\t", data[0], data[1]);
		
		// channel mask displayed in hex
		// First two channel mask bits are stored as one bit each in data stream; compress this.
		data[0] = frameData[j*nData+2] % 0x100;
		data[1] = frameData[j*nData+2] / 0x100;	
		fprintf( dataFile, "%x", (data[0]*2 + data[1]));
		
		// Then the rest of the mask (64 bits or 4 words)
		for(int i=3; i<7; i++)
		{
			data[0] = frameData[j*nData+i] % 0x1000000;
			data[1] = frameData[j*nData+i] % 0x10000 / 0x100;
			fprintf( dataFile, "%x%x ", data[0], data[1]);
		}
		
		// Common mode noise value, followed by 64 strip data values, followed by pedestal value.
		//short int commonMode = frameData[j*nData+7];
		fprintf( dataFile,  "\n%u\t", frameData[j*nData+7] );
		
		for(int i=0; i< 65; i++)	// 63 strips + pedestal
			fprintf( dataFile,  "%u\t", (frameData[j*nData+i+8]) );
		
		data[0] = frameData[ j*nData+73] % 0x100;
		data[1] = frameData[ j*nData+73] / 0x100;
		
		fprintf( dataFile, "%u\t%u\n\n\n", data[0], data[1]);	// stop bit and new sync word
				
	}	
	
}	

// ADDED JULY 9 2011
void USB_d2xx::setConfig(void)
{
	const int n=39;		// number of components in write array
	const int nV=45;	// number of values in sendParamsWindow
	char cBufWrite[n];	// write array
	DWORD 	dwBytesWritten;  // returns number of bytes written.

	// load user input configuration settings from sendParamsWindow.
	int value[45];
	int chan[64];
	int asic = gui->sendParamsWindow_asic->value();
	
	// initialize configuration arrays
	for(int i=0; i<nV+1; i++)	value[i] = 0;
	for(int i=0; i<64; i++)		chan[i] = 0;
	
	// initialize write buffer.
	for(int i=0; i<n; i++) cBufWrite[i] = 0;
	
	// fill arrays with values from sendParameters window.
	// CHAN values are from the channel disable buttons.
	for(int i=0; i<nV+1; i++)	value[i] = gui->sendParamsWindow_value[i]->value();
	for(int i=0; i<64; i++)		chan[i]  = gui->sendParamsWindow_chan[i]->value();
	
/*	// Get disable register from disable buttons
	float chan0, chan1, chan2, chan3, chan4, chan5, chan6, chan7, chan8, chan9;
	float chan10, chan11, chan12, chan13, chan14, chan15, chan16, chan17, chan18, chan19;
	float chan20, chan21, chan22, chan23, chan24, chan25, chan26, chan27, chan28, chan29;
	float chan30, chan31, chan32, chan33, chan34, chan35, chan36, chan37, chan38, chan39;
	float chan40, chan41, chan42, chan43, chan44, chan45, chan46, chan47, chan48, chan49;
	float chan50, chan51, chan52, chan53, chan54, chan55, chan56, chan57, chan58, chan59;
	float chan60, chan61, chan62, chan63;
	
	chan0 = gui->sendParamsWindow_chan0->value();
	chan1 = gui->sendParamsWindow_chan1->value();
	chan2 = gui->sendParamsWindow_chan2->value();
	chan3 = gui->sendParamsWindow_chan3->value();
	chan4 = gui->sendParamsWindow_chan4->value();
	chan5 = gui->sendParamsWindow_chan5->value();
	chan6 = gui->sendParamsWindow_chan6->value();
	chan7 = gui->sendParamsWindow_chan7->value();
	chan8 = gui->sendParamsWindow_chan8->value();
	chan9 = gui->sendParamsWindow_chan9->value();
	chan10 = gui->sendParamsWindow_chan10->value();
	chan11 = gui->sendParamsWindow_chan11->value();
	chan12 = gui->sendParamsWindow_chan12->value();
	chan13 = gui->sendParamsWindow_chan13->value();
	chan14 = gui->sendParamsWindow_chan14->value();
	chan15 = gui->sendParamsWindow_chan15->value();
	chan16 = gui->sendParamsWindow_chan16->value();
	chan17 = gui->sendParamsWindow_chan17->value();
	chan18 = gui->sendParamsWindow_chan18->value();
	chan19 = gui->sendParamsWindow_chan19->value();
	chan20 = gui->sendParamsWindow_chan20->value();
	chan21 = gui->sendParamsWindow_chan21->value();
	chan22 = gui->sendParamsWindow_chan22->value();
	chan23 = gui->sendParamsWindow_chan23->value();
	chan24 = gui->sendParamsWindow_chan24->value();
	chan25 = gui->sendParamsWindow_chan25->value();
	chan26 = gui->sendParamsWindow_chan26->value();
	chan27 = gui->sendParamsWindow_chan27->value();
	chan28 = gui->sendParamsWindow_chan28->value();
	chan29 = gui->sendParamsWindow_chan29->value();
	chan30 = gui->sendParamsWindow_chan30->value();
	chan31 = gui->sendParamsWindow_chan31->value();
	chan32 = gui->sendParamsWindow_chan32->value();
	chan33 = gui->sendParamsWindow_chan33->value();
	chan34 = gui->sendParamsWindow_chan34->value();
	chan35 = gui->sendParamsWindow_chan35->value();
	chan36 = gui->sendParamsWindow_chan36->value();
	chan37 = gui->sendParamsWindow_chan37->value();
	chan38 = gui->sendParamsWindow_chan38->value();
	chan39 = gui->sendParamsWindow_chan39->value();
	chan40 = gui->sendParamsWindow_chan40->value();
	chan41 = gui->sendParamsWindow_chan41->value();
	chan42 = gui->sendParamsWindow_chan42->value();
	chan43 = gui->sendParamsWindow_chan43->value();
	chan44 = gui->sendParamsWindow_chan44->value();
	chan45 = gui->sendParamsWindow_chan45->value();
	chan46 = gui->sendParamsWindow_chan46->value();
	chan47 = gui->sendParamsWindow_chan47->value();
	chan48 = gui->sendParamsWindow_chan48->value();
	chan49 = gui->sendParamsWindow_chan49->value();
	chan50 = gui->sendParamsWindow_chan50->value();
	chan51 = gui->sendParamsWindow_chan51->value();
	chan52 = gui->sendParamsWindow_chan52->value();
	chan53 = gui->sendParamsWindow_chan53->value();
	chan54 = gui->sendParamsWindow_chan54->value();
	chan55 = gui->sendParamsWindow_chan55->value();
	chan56 = gui->sendParamsWindow_chan56->value();
	chan57 = gui->sendParamsWindow_chan57->value();
	chan58 = gui->sendParamsWindow_chan58->value();
	chan59 = gui->sendParamsWindow_chan59->value();
	chan60 = gui->sendParamsWindow_chan60->value();
	chan61 = gui->sendParamsWindow_chan61->value();
	chan62 = gui->sendParamsWindow_chan62->value();
	chan63 = gui->sendParamsWindow_chan63->value();
 
 */	
	// logic to assemble configuration settings into write array.
	// Note the pattern is not the same for each register and some bits are intentionally unused!
	cBufWrite[0] = value[0]*16 + value[1]*8 + value[2]*4 + value[3]*2 + value[4] + 32*asic;	
	cBufWrite[1] = value[5]*16 + value[6]*8 + value[7]*4 + value[8]*2 + value[9] + 32*asic;
	cBufWrite[2] = value[10]*16 + value[11]*8 + value[12]*4 + value[13]*2 + value[14] + 32*asic;
	cBufWrite[3] = value[15]*16 + value[16]*8 + value[17]*4 + value[18]*2 + value[19] + 32*asic;
	cBufWrite[4] = value[20]*16 + value[21]*8 + value[22]*4 + value[23]*2 + getbits(value[24],0,1) + 32*asic; // 4 single bits + LSB of dummy digital delay.
	cBufWrite[5] = reversebits( getbits(value[24], 5, 5), 5 ) + 32*asic; // 5 MSB of dummy digital delay.
	cBufWrite[6] = reversebits( getbits(value[25], 4, 5), 5) + 32*asic;  // 5 LSB of digital threshold.
	cBufWrite[7] = reversebits( getbits(value[25], 9, 5), 5) + 32*asic;  // 5 MSB of digital threshold.
	cBufWrite[8] = chan[0]*16 + chan[1]*8 + chan[2]*4 + chan[3]*2 + chan[4] + 32*asic;
	cBufWrite[9] = chan[5]*16 + chan[6]*8 + chan[7]*4 + chan[8]*2 + chan[9] + 32*asic;
	cBufWrite[10] = chan[10]*16 + chan[11]*8 + chan[12]*4 + chan[13]*2 + chan[14] + 32*asic;
	cBufWrite[11] = chan[15]*16 + chan[16]*8 + chan[17]*4 + chan[18]*2 + chan[19] + 32*asic;
	cBufWrite[12] = chan[20]*16 + chan[21]*8 + chan[22]*4 + chan[23]*2 + chan[24] + 32*asic;
	cBufWrite[13] = chan[25]*16 + chan[26]*8 + chan[27]*4 + chan[28]*2 + chan[29] + 32*asic;
	cBufWrite[14] = chan[30]*16 + chan[31]*8 + chan[32]*4 + chan[33]*2 + chan[34] + 32*asic;
	cBufWrite[15] = chan[35]*16 + chan[36]*8 + chan[37]*4 + chan[38]*2 + chan[39] + 32*asic;
	cBufWrite[16] = chan[40]*16 + chan[41]*8 + chan[42]*4 + chan[43]*2 + chan[44] + 32*asic;
	cBufWrite[17] = chan[45]*16 + chan[46]*8 + chan[47]*4 + chan[48]*2 + chan[49] + 32*asic;
	cBufWrite[18] = chan[50]*16 + chan[51]*8 + chan[52]*4 + chan[53]*2 + chan[54] + 32*asic;
	cBufWrite[19] = chan[55]*16 + chan[56]*8 + chan[57]*4 + chan[58]*2 + chan[59] + 32*asic;
	cBufWrite[20] = chan[60]*8 + chan[61]*4 + chan[62]*2 + chan[63]*1 + 32*asic;  // No MSB for the last disable register.
	cBufWrite[21] = value[26]*4 + value[27]*2 + value[28] + 32*asic;
	cBufWrite[22] = reversebits( getbits(value[29], 4, 5), 5 ) + 32*asic; // digital threshold
	cBufWrite[23] = reversebits( getbits(value[30], 3, 4), 4 ) + 32*asic;
	cBufWrite[24] = reversebits( getbits(value[31], 3, 4), 4 ) + 32*asic;
	cBufWrite[25] = reversebits( getbits(value[32], 3, 4), 4 ) + 32*asic;
	cBufWrite[26] = reversebits( getbits(value[33], 3, 4), 4 ) + 32*asic;
	cBufWrite[27] = reversebits( getbits(value[34], 2, 3), 3 ) + 32*asic;
	cBufWrite[28] = reversebits( getbits(value[35], 2, 3), 3 ) + 32*asic;
	cBufWrite[29] = reversebits( getbits(value[36], 2, 3), 3 ) + 32*asic;
	cBufWrite[30] = reversebits( getbits(value[37], 2, 3), 3 ) + 32*asic;
	cBufWrite[31] = reversebits( getbits(value[38], 2, 3), 3 ) + 32*asic;
	cBufWrite[32] = reversebits( getbits(value[39], 2, 3), 3 ) + 32*asic;
	cBufWrite[33] = reversebits( getbits(value[40], 2, 3), 3 ) + 32*asic;
	cBufWrite[34] = reversebits( getbits(value[41], 2, 3), 3 ) + 32*asic;
	cBufWrite[35] = reversebits( getbits(value[42], 2, 3), 3 ) + 32*asic;
	cBufWrite[36] = reversebits( getbits(value[43], 2, 3), 3 ) + 32*asic;
	cBufWrite[37] = reversebits( getbits(value[44], 2, 3), 3 ) + 32*asic;
	cBufWrite[38] = 1;

	// Testing purposes
	for(int i=0; i<39; i++)
		printf("%d\n", cBufWrite[i]);
//		cout << dec << cBufWrite[i] << endl;
//	cout << "First data value is " << value[0]*16 + value[1]*8 + value[2]*4 + value[3]*2 + value[4] + 32*asic << endl;
//	cout << value[0] << "  "  << value[1] << "  "  << value[2] << "  " << value[3] << "  " << value[4] << endl << endl;
	
	/* Write */
	dwBytesWritten = 0;
	if((ftStatus = FT_Write(ftHandle, cBufWrite, 38, &dwBytesWritten)) != FT_OK) {
		printf("Error FT_Write(%d)\n", ftStatus);
		return;
	}
			
	cout << "Wrote " << dwBytesWritten << " bytes." << endl << endl;
			
}
