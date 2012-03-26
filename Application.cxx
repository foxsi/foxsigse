// application class

#include "Application.h"
#include "data.h"
#include "commands.h"
#include "gui.h"
#include "Foxsidata.h"
#include "usbd2xx.h"
#include "mainLightcurve.h"
#include <pthread.h>
#include <sched.h>
#include <FL/Fl_File_Chooser.H>

#include <sys/time.h>
#include <time.h>


#define XSTRIPS 128
#define YSTRIPS 128
#define MAX_CHANNEL 1024
#define MAXPATH 128

extern Gui *gui;

unsigned int CountcurveFunction[MAX_CHANNEL];

extern int stop_message;
extern FILE *dataFile;
extern int fout;

// filename is set automatically with local time
extern char dataFilename[MAXPATH];
extern char dataFileDir[MAXPATH];

// Preference variables
int file_type;
char *data_file_save_dir;
int read_delay;
int data_source;	// 0 means simulation, 1 means ASIC, 2 means Formatter
int mainImage_minimum;
int newFPGA_register;
char *formatter_configuration_file;

//extern int mainHistogram_binsize;

Application::Application()
{
	// Constructor method for the Application class
	// Add initialization here:
	pixel_half_life = 5;
	frame_miss_count = 0;
	frame_number = 0;
	bad_check_sum_count = 0;
	no_trigger_count = 0;
	formatter_start_time = 0;
}

int Application::get_data_source(void){
	return data_source;
}

void Application::set_data_source(int value){
	if ((data_source == 0) || (data_source == 1) || (data_source == 2)){
		data_source = value;
	}
}

void Application::flush_histogram(void)
{	
	// Zero the Histogram
	gui->mainHistogramWindow->flush(7);
}

void Application::flush_timeseries(void)
{	
	// Zero the time series
	for(int i = 0;i < MAX_CHANNEL; i++)
	{
		CountcurveFunction[i] = 0;
	}
	gui->mainLightcurveWindow->redraw();
}

void Application::flush_image(void)
{
	gui->detectorsImageWindow->flush_image(7);
	gui->mainImageWindow->redraw();
	gui->detectorsImageWindow->redraw();
}

void Application::save_preferences(void)
{
	gui->prefs->set("pixel_half_life", (float) gui->pixelhalflife_value->value());
	gui->prefs->set("file_type", gui->fileTypeChoice->value());
	gui->prefs->set("data_file_save_dir", gui->datafilesavedir_fileInput->value());
	gui->prefs->set("read_delay", gui->readdelay_value->value());
	gui->prefs->set("data_source", gui->DataSource_choice->value());
	gui->prefs->set("mainImage_minimum", gui->mainImageMin_slider->value());
	gui->prefs->set("newFPGA_register", gui->newControlRegisters_check->value());
	gui->prefs->set("formatter_configuration_file", gui->gsesyncfile_fileInput->value());
}

void Application::read_preferences(void)
{
	// Read the preferences
	gui->prefs->get("pixel_half_life", pixel_half_life,3.0);
	gui->prefs->get("file_type", file_type, 0);
	gui->prefs->get("data_file_save_dir", data_file_save_dir, "/Users/schriste/");
	gui->prefs->get("read_delay", read_delay, 10000);
	gui->prefs->get("data_source", data_source, 0);
	gui->prefs->get("mainImage_minimum", mainImage_minimum, 0);
	gui->prefs->get("newFPGA_register", newFPGA_register,0);
	gui->prefs->get("formatter_configuration_file", formatter_configuration_file, "/Users/schriste/");
}

void Application::update_preferencewindow(void)
{	
	// Update them in the Preferences window
	gui->pixelhalflife_value->value(pixel_half_life);
	gui->fileTypeChoice->value(file_type);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	gui->readdelay_value->value(read_delay);
	gui->DataSource_choice->value(data_source);
	gui->newControlRegisters_check->value(newFPGA_register);
	gui->gsesyncfile_fileInput->value(formatter_configuration_file);
}

void Application::set_datafile_dir(void)
{
	char *temp = fl_dir_chooser("Pick a directory:", "", 0);
	strcpy(data_file_save_dir, temp);
	gui->datafilesavedir_fileInput->value(data_file_save_dir);
	printf_to_console("Output directory set to %s.\n", data_file_save_dir, NULL);	
}

void Application::set_gsesync_file(void)
{
	char *temp = fl_file_chooser("Pick gsesync", "", 0);
	strcpy(formatter_configuration_file, temp);
	gui->gsesyncfile_fileInput->value(formatter_configuration_file);
	printf_to_console("Output directory set to %s.\n", formatter_configuration_file, NULL);	
}
					
void Application::start_file()
{
	data_start_file();
}

void Application::write_header(FILE *file)
{
	gui->usb->writeHeader(file);
}

// the method that gets executed when the readFile callback is called
void Application::readFile()
{
	// launch file browser
	char *file = fl_file_chooser("Pick a file from this list:", "*.*", "");
	if(file == NULL)
		return;
	
	cout << "Loading File:" << file << endl;

	//store image name
	//strcpy(filename,file);
	flush_image();
	flush_histogram();
	
	//gui->data->readDatafile(file);
}
 
void Application::initialize(void)
{
	data_initialize();
	gui->initializeBut->deactivate();
	//toggle_detector_display();
	gui->closeBut->activate();
}

void Application::close_data(void)
{
	// Close a connection to a data stream
	gui->usb->close();
	gui->initializeBut->activate();
	gui->closeBut->deactivate();
	gui->startReadingDataButton->deactivate();
}

void Application::start_reading_data(void)
{
	print_to_console("Reading started.\n");
	gui->stopReadingDataButton->activate();
	gui->closeBut->deactivate();
	gui->startReadingDataButton->deactivate();
	
	data_start_reading();	
}

void Application::printf_to_console(const char *text, char *string1, int number)
{
	char buffer[200];
	sprintf(buffer, text, string1);
	gui->consoleBuf->insert(buffer);
	gui->consoleBuf->show_insert_position();
}
								   
void Application::print_to_console(const char *text)
{
	gui->consoleBuf->insert(text);
	gui->consoleBuf->show_insert_position();
}


void Application::openSendParamsWindow(void)
{
	gui->sendParamsWindow->show();
}

void Application::openSetHoldTimeWindow(void)
{
	gui->setHoldTimeWindow->show();
}

void Application::openSetTrigWindow(void)
{
	gui->setTrigWindow->show();
}

void Application::send_params(void)
{	
	gui->usb->setConfig();	
	gui->consoleBuf->insert("Sending Params to controller.\n");
}

void Application::send_global_params(int option)
{
	gui->usb->setGlobalConfig(option);
	gui->consoleBuf->insert("Sending global Params to controller.\n");
}

void Application::save_settings(void)
{
	gui->usb->saveSettings();
}

void Application::restore_settings(void)
{
	gui->usb->restoreSettings();
}

void Application::clear_console(void)
{
	// clear the text in the console
	gui->buff->select(0, gui->buff->length());
	gui->buff->remove_selection();
}

void Application::test(void)
{
}

void Application::send_atten_state(bool value)
{
	command_attenuator_state(value);
}

void Application::send_voltage_command(void)
{
	command_voltage_set(gui->highVoltage_input->value());
}

void Application::send_clockset_command(void)
{
	command_clock_set(gui->clockHigh_input->value(), gui->clockLow_input->value());
}

void Application::stop_reading_data(void)
{
	data_stop_reading();
	
	Fl::lock();
	gui->stopReadingDataButton->deactivate();
	gui->startReadingDataButton->activate();
	gui->closeBut->activate();
	Fl::unlock();
	
	// send the thread the message to stop itself	
}

void Application::reset_read_counter(void)
{
	gui->nEventsDone->value(0);
	
	frame_display_count = 0;
	frame_miss_count = 0;
	bad_check_sum_count = 0;
	frame_read_count = 0;
	no_trigger_count = 0;
	formatter_start_time = 0;
}

void Application::update_histogrambinsize(void)
{
	//mainHistogram_binsize = gui->histogrambinsize_counter->value();
	gui->mainHistogramWindow->set_binsize(gui->histogrambinsize_counter->value());
	gui->mainHistogramWindow->redraw();
}

void Application::update_timebinsize(void)
{
	// mainHistogram_binsize = gui->binsize_counter->value();
	gui->mainLightcurveWindow->binsize[0] = gui->timebinsize_counter->value();
}

void Application::update_lightcurvexmax(void)
{
	gui->mainLightcurveWindow->set_xmax(gui->lightcurvexmax_counter->value());
	gui->mainLightcurveWindow->redraw();
}

void Application::update_histogramxmax(void)
{
	gui->mainHistogramWindow->set_xmax(gui->histogramxmax_counter->value());
	gui->mainHistogramWindow->redraw();
}

void Application::set_lowthreshold(void)
{
	gui->mainHistogramWindow->set_lowthreshold(gui->mainImageMin_slider->value());
	gui->mainHistogramWindow->redraw();
}

void Application::set_imagemax(void)
{
	gui->mainImageWindow->set_ymax(gui->mainImageMax_slider->value());
	gui->detectorsImageWindow->set_ymax(gui->mainImageMax_slider->value());
	
	gui->mainImageWindow->redraw();
	gui->detectorsImageWindow->redraw();
}

void Application::set_histogram_max(void)
{
	gui->mainHistogramWindow->set_ymax(gui->mainHistogram_ymax_slider->value());
}

void Application::set_energy_histogram(void)
{
	//gui->histogrambinsize_counter->value(1);
	//mainHistogram_binsize = 1;
	//gui->mainHistogramWindow->redraw();

}

void Application::set_channel_histogram(void)
{
	//gui->histogrambinsize_counter->value(25);
	//mainHistogram_binsize = 25;
	//gui->mainHistogramWindow->redraw();
}

void Application::toggle_image_integrate(void)
{
	if (gui->mainImage_integrate_button->value() == 1){
		//gui->prefs->set("pixel_half_life", pixel_half_life);
		//pixel_half_life = 0;
		//printf_to_console("pixel_half_life = %f\n", "", pixel_half_life);
		gui->pixel_halflife_slider->value(0);
		gui->mainImageWindow->redraw();
	}
	if (gui->mainImage_integrate_button->value() == 0){
		//gui->prefs->get("pixel_half_life", pixel_half_life, 3.0);
		//printf_to_console("pixel_half_life = %f\n", "", pixel_half_life);
		gui->pixel_halflife_slider->value(1);
		gui->mainImageWindow->redraw();
	}
}

void Application::save_histogram_to_file(void)
{
	gui->mainHistogramWindow->save();
}

void Application::save_image_to_file(void)
{
	gui->detectorsImageWindow->save();
}

void Application::set_lightcurve_ymax(void)
{
	// update the max of the y range of the light curve
	gui->mainLightcurveWindow->set_ymax(gui->mainLightcurve_ymaxslider->value());
	gui->mainLightcurveWindow->redraw();
}

void Application::toggle_show_mask(void)
{
	// update whether to show the asic mask in the mainImage
	gui->mainImageWindow->show_mask = gui->showmask_checkbox->value();
}

void Application::toggle_detector_display(void)
{	
	gui->mainHistogramWindow->update_detector_display(gui->detector0_checkbox->value(), 0);
	gui->mainHistogramWindow->update_detector_display(gui->detector1_checkbox->value(), 1);
	gui->mainHistogramWindow->update_detector_display(gui->detector2_checkbox->value(), 2);
	gui->mainHistogramWindow->update_detector_display(gui->detector3_checkbox->value(), 3);
	gui->mainHistogramWindow->update_detector_display(gui->detector4_checkbox->value(), 4);
	gui->mainHistogramWindow->update_detector_display(gui->detector5_checkbox->value(), 5);
	gui->mainHistogramWindow->update_detector_display(gui->detector6_checkbox->value(), 6);
	
	gui->mainHistogramWindow->redraw();
	gui->mainImageWindow->redraw();
	gui->mainLightcurveWindow->redraw();
}

void Application::testfunction(void)
{
	//gui->mainHistogramWindow->detector_display[0] = gui->detectorall_checkbox->value();
	//for (int i = 0; i < 8; i++) {
	//	cout << gui->mainHistogramWindow->detector_display[i] << endl;
	//}
	//cout << endl;
		
	gui->mainHistogramWindow->redraw();
	gui->mainImageWindow->redraw();
	gui->mainLightcurveWindow->redraw();
}

float Application::get_pixel_half_life(void){
	return pixel_half_life;
}

void Application::set_pixel_half_life(float new_value){
	pixel_half_life = new_value;
}

void Application::set_detector_to_display(int detector_number)
{
	gui->mainImageWindow->set_detector_to_display(detector_number);
}
