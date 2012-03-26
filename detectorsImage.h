#ifndef _detectorsImage_h_     // prevent multiple includes
#define _detectorsImage_h_

#include <time.h>
#include <FL/Fl_Gl_Window.H>

#define XSTRIPS 128
#define YSTRIPS 128
#define NUM_DETECTORS 7

class detectorsImage : public Fl_Gl_Window {
public:
	detectorsImage(int x,int y,int w,int h,const char *l=0);
	void draw();
	void flush_image(int detector_number);
	void flush_mask(int detector_number);
	void add_count_to_image(int x, int y, int detector_num);
	void add_count_to_mask(int x, int y, int detector_num);
	int handle(int eventType);
	void set_ymax(double value);
	void save(void);
	long get_image(int x, int y, int detector_number);
	long get_imagetime(int x, int y, int detector_number);
	unsigned int get_mask(int x, int y, int detector_number);
	long get_ymax(int detector_number);
private:
	unsigned long image[XSTRIPS][YSTRIPS][NUM_DETECTORS];
	unsigned int mask[XSTRIPS][YSTRIPS][NUM_DETECTORS];
	clock_t imagetime[XSTRIPS][YSTRIPS][NUM_DETECTORS];
	
	void text_output(int x, int y, char *string);
				
	float GL_cursor[2];
	int mousePixel[2];
	int chosenPixel[2];
	
	int detector_angle[NUM_DETECTORS];
	int detector_buffer[2];
	int border_buffer;
	double ymax[NUM_DETECTORS];
};

#endif
