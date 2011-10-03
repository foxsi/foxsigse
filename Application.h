
#ifndef __Application_H
#define __Application_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// put your global typedefs here

class Application {
public:
	Application();
	
	// Menu Item Actions
	
	// File Menu
	// Start simulating data (NOT IMPLEMENTED)
	void simulateData();	
	// Read data from file 
	void readFile();
	// Quite and close the program
	void Exit();
	// Read data coming in from USB stream, straight from ASIC (DEPRECATED!)
	void readUSBStream();  
	// Read data coming in from Telemetry stream (DEPRECATED)
	void readTeleStream(); 
	// Write the spectrum to a file
	void WriteSpec();  
	// Write a lightcurve to a file
	void WriteLightcurve();  
	// Flush all data buffers
	void FlushData(void); 
	
	void dataSync();   //sync data...may not be necessary (Menu Menu)
	
	void setDetector(int detector);  //set the detector to show in main windows (Detector Menu)
	
	
	// Various GUI Buttons
	
	void initialize_data(void);		// Initialize a connection to a data stream
	void close_data(void);			// Close a connection to a data stream
	void openSendParamsWindow(void);		// Open the send params window
	
	//END Menu Item Actions -----------------------------------------------------------
	
	//START Text Output Actions -------------------------------------------------------
	float getRate(int detector);   //get the count rate 
	int getFrameNum(void);  //get the frame number
	int getShutState(void);  //get the shutter state
	float getTemp(void);  //get the temperature
	const char getPixel(void);
	
	// Callbacks for the send params window
	void send_params(void);
	void send_global_params(void);
	void break_acq(int data);
	void save_settings(void);
	void restore_settings(void);
	void test(void);
	void stop_reading_data(void);
	void start_reading_data(void);	// start thread to control read functions.
	
	// open the data file for saving data
	// executed when someone clicks on "Write to File" Button
	void start_file(void);			
	
	
private:
	char filename[40];
	static void *read_data(void *variable);		// Begin reading data from a data stream

	
	
};

#endif
