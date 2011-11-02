
#ifndef __Application_H
#define __Application_H

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
	
	void initialize_data(void);		// Initialize a connection to a data stream
	void close_data(void);			// Close a connection to a data stream
	void openSendParamsWindow(void);		// Open the send params window
	
	//END Menu Item Actions -----------------------------------------------------------
	
	//START Text Output Actions -------------------------------------------------------
	float getRate(int detector);   //get the count rate 
	int getFrameNum(void);  //get the frame number
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
	
	// clear the existing text displayed in the console
	// executed when someone clicks the "Clear" Button next to the console
	void clear_console(void);
	void save_preferences(void);

	// convienience function to print text to the console
	// not yet implemented
	static void printf_to_console(const char *string1, char *string2);
	static void print_to_console(const char *text);
	float get_pixel_half_life(void);
	
	void update_PreferenceWindow(void);
	
private:
	char filename[40];
	static void *read_data(void *variable);		// Begin reading data from a data stream
	static void* new_read_data();

};

#endif
