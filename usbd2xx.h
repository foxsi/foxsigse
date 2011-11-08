/*
	FOXSI USB GSE USB Package ----------------------------------------
	
	PURPOSE : Library for the USB D2xx class

	REQUIRED : 
 
 	WRITTEN : Steven Christe (18-Dec-2009), based on code by L. Glesener
	MODIFIED Oct-2010 by Lindsay

	NOTES : This class is shared with the full GSE program.  The commented out lines of 
	code are used by the GSE program, please do not delete!
	
	TO DO : 
		1] Define the destructor method
		2] The read method should probably only read one word at a time instead of a frame so that the GSE can take care of syncing.  Hopefully using the read method does not have much of an overhead so that it can read quickly since it will need to called often.  Other code will then parse the this.
		3] The type of the private pcBufRead, currently unsigned, is probably not correct.  A single element should be the exact size of a word.
 	
*/

#ifndef __usbd2xx_H
#define __usbd2xx_H

#include "ftd2xx.h"

class USB_d2xx {
public:
	USB_d2xx();
	USB_d2xx( const int n );	// construct with initialization

	int open();			// Initialize and open the device and data file.
	int findSync();		// Read data until sync word is found, then stop.
	int readFrame();	// Read one frame from the device.
	void writeFrame(FILE *dataFile);	// Write last frame to file.
	void close();		// Close and cleanup the device and data file.
	void printFrame();	// Print out the last frame that was read
	void setConfig();	// Write configuration register to FPGA.
	void setGlobalConfig(int option);	// Write config settings that are global, not just one ASIC.
	void breakAcq(int data);	// Break ACQ loop.
	void saveSettings();	// Save current control settings for one ASIC.
	void restoreSettings();	// Restore last control settings for one ASIC.
	void loadDefaultSettings();	// set or restore default settings for one ASIC.

		
private:
	int			nBytesFrame;	// Number of bytes per frame; set in main.
	FILE*		dataFile;		// Data file
	FT_HANDLE	ftHandle;		// p-sdie File handle for the usb connection
	FT_STATUS	ftStatus;		// Status of connection defined by ft2xx
	unsigned short int*	frameData;		// The last readout frame
	int			asic0settings[45];	// control settings for first ASIC
	int			asic1settings[45];	// control settings for second ASIC
	int			asic2settings[45];	// control settings for third ASIC
	int			asic3settings[45];	// control settings for fourth ASIC
	int			disable0[64];	// disable mask for ASIC 0
	int			disable1[64];	// disable mask for ASIC 1
	int			disable2[64];	// disable mask for ASIC 2
	int			disable3[64];	// disable mask for ASIC 3
	int			test0[64];	// test enable for ASIC 0
	int			test1[64];	// test enable for ASIC 1
	int			test2[64];	// test enable mask for ASIC 2
	int			test3[64];	// test enable mask for ASIC 3
	
	struct strip_data 
	{
		unsigned channel : 6;
		unsigned value : 10;
	};
	
	struct asic_dataframe	// defining bit fields
	{
		unsigned sync: 8;
		unsigned start_bit: 1;	//1 means packet is starting
		unsigned chip_bit : 1;	//1 if there is data in the packet
		unsigned trig_bit : 1;	//
		unsigned seu_bit : 1;	//1 single event upset
		unsigned pedestal_bit : 1;
		long asic_mask : 64;
		unsigned common_mode : 1;
		unsigned noise : 16;
		strip_data sdata[64];
		unsigned pedestal : 16;
		unsigned stop_bit : 1;
	};
	
	asic_dataframe frame[4];
};

#endif
