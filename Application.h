
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
	void send_clockset_command(void);
	void send_atten_state(bool value);
	void send_voltage_command(void);
	
	// --------------------- File Menu -------------------------------
	// Set the directory that the data file will be written to
	void set_datafile_dir(void);
	// Read data from file 
	void readFile();
	
	// Flush all data buffers
	void FlushData(void); 
	
	// void dataSync();   //sync data...may not be necessary (Menu Menu)
	
	void setDetector(int detector);  //set the detector to show in main windows (Detector Menu)
	
	
	// Various GUI Buttons
	
	void initialize(void);		// Initialize a connection to a data stream
	void close_data(void);			// Close a connection to a data stream
	void openSendParamsWindow(void);		// Open the send params window
	void openSetHoldTimeWindow(void);		// Open the set hold time window
	void openSetTrigWindow(void);			// Open the trigger options window
	
	//END Menu Item Actions -----------------------------------------------------------
	
	//START Text Output Actions -------------------------------------------------------
	float getRate(int detector);   //get the count rate 
	int getFrameNum(void);  //get the frame number
	int getShutState(void);  //get the shutter state
	float getTemp(void);  //get the temperature
	const char getPixel(void);
	
	// Callbacks for the send params window
	void send_params(void);
	//void break_acq(int data);
	void save_settings(void);
	void restore_settings(void);
	void test(void);
	void stop_reading_data(void);
	void start_reading_data(void);	// start thread to control read functions.

	// Callbacks for the set hold time window
	void send_global_params(int option);	// set trigger options
	void start_auto_run(void);	// auto-run sequence of acquisitions with varying hold times
	
	// open the data file for saving data
	// executed when someone clicks on "Write to File" Button
	void start_file(void);	
	void write_header(FILE *file);
	//void set_datafile_dir(void);
	
	// clear the existing text displayed in the console
	// executed when someone clicks the "Clear" Button next to the console
	void clear_console(void);
	void save_preferences(void);
	void read_preferences(void);
	void update_preferencewindow(void);	
	
	// convienience function to print text to the console
	// not yet implemented
	static void printf_to_console(const char *string1, char *string2, int number);
	static void print_to_console(const char *text);
	float get_pixel_half_life(void);
	void reset_read_counter(void);
	void set_lowthreshold(void);
	void flush_histogram(void);
	void flush_image(void);
	void flush_timeseries(void);
	void update_histogrambinsize(void);
	void update_timebinsize(void);
	void set_energy_histogram(void);
	void set_channel_histogram(void);
	void update_lightcurvexmax(void);
	void update_histogramxmax(void);
	void toggle_image_integrate(void);
	
	int elapsed_time_sec;
private:
	char filename[40];
	static void *read_data(void *variable);		// Begin reading data from a data stream
	static void *auto_run_sequence(void *variable);	// auto-run sequence of acquisitions with varying hold times
};

#endif
