// generated by Fast Light User Interface Designer (fluid) version 1.0300

#ifndef gui_h
#define gui_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "Application.h"
#include <stdlib.h>
#include "Foxsidata.h"
#include "usbd2xx.h"
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include "mainImage.h"
#include <FL/Fl_Output.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Output.H>
#include "mainHistogram.h"
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include "mainLightcurve.h"
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Input.H>

class Gui {
public:
  Gui();
  Fl_Double_Window *mainWindow;
  Fl_Menu_Bar *menuBar;
  static Fl_Menu_Item menu_menuBar[];
private:
  void cb_About_i(Fl_Menu_*, void*);
  static void cb_About(Fl_Menu_*, void*);
  void cb_Preferences_i(Fl_Menu_*, void*);
  static void cb_Preferences(Fl_Menu_*, void*);
  void cb_Quit_i(Fl_Menu_*, void*);
  static void cb_Quit(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *fileMenu;
  static Fl_Menu_Item *readFile;
  static Fl_Menu_Item *readUSBStream;
  static Fl_Menu_Item *readTeleStream;
  static Fl_Menu_Item *WritePicScreen;
  static Fl_Menu_Item *WriteLightcurve;
  static Fl_Menu_Item *menuProc;
private:
  void cb_Commanding_i(Fl_Menu_*, void*);
  static void cb_Commanding(Fl_Menu_*, void*);
  void cb_ACTEL_i(Fl_Menu_*, void*);
  static void cb_ACTEL(Fl_Menu_*, void*);
public:
  mainImage *mainImageWindow;
  Fl_Output *pixelNum;
  Fl_Output *pixelCounts;
  Fl_Light_Button *subImageLockbut;
private:
  void cb_Flush_i(Fl_Button*, void*);
  static void cb_Flush(Fl_Button*, void*);
public:
  Fl_Value_Slider *mainImageMin_slider;
private:
  void cb_mainImageMin_slider_i(Fl_Value_Slider*, void*);
  static void cb_mainImageMin_slider(Fl_Value_Slider*, void*);
public:
  Fl_Light_Button *mainImage_integrate_button;
private:
  void cb_mainImage_integrate_button_i(Fl_Light_Button*, void*);
  static void cb_mainImage_integrate_button(Fl_Light_Button*, void*);
  void cb_Save_i(Fl_Button*, void*);
  static void cb_Save(Fl_Button*, void*);
public:
  Fl_Output *rateOutput0;
  Fl_Output *rateOutput1;
  Fl_Output *rateOutput2;
  Fl_Output *rateOutput3;
  Fl_Output *rateOutput4;
  Fl_Output *rateOutput5;
  Fl_Output *rateOutput6;
  Fl_Output *rateOutput7;
  Fl_Value_Output *shutterstateOutput;
  Fl_Value_Output *tempOutput2;
  Fl_Value_Output *HVOutput;
  Fl_Value_Output *tempOutput1;
  Fl_Value_Output *tempOutput0;
  Fl_Value_Output *tempOutput3;
  Fl_Value_Output *tempOutput;
  Fl_Value_Output *tempOutput10;
  Fl_Value_Output *tempOutput9;
  Fl_Value_Output *tempOutput8;
  Fl_Value_Output *tempOutput6;
  Fl_Value_Output *tempOutput5;
  Fl_Value_Output *tempOutput4;
  Fl_Value_Output *tempOutput7;
  Fl_Value_Output *tempOutput11;
  Fl_Value_Output *VoltageOutput1;
  Fl_Value_Output *VoltageOutput0;
  Fl_Value_Output *VoltageOutput2;
  Fl_Value_Output *VoltageOutput3;
  mainHistogram *mainHistogramWindow;
  Fl_Value_Output *mainHistogramYlabelmid;
  Fl_Value_Output *mainHistogramYlabelmax;
  Fl_Choice *mainHistogram_choice;
  static Fl_Menu_Item menu_mainHistogram_choice[];
private:
  void cb_Energy_i(Fl_Menu_*, void*);
  static void cb_Energy(Fl_Menu_*, void*);
public:
  Fl_Value_Output *histLow;
  Fl_Value_Output *histCounts;
  Fl_Value_Output *histEnergy;
private:
  void cb_Flush1_i(Fl_Button*, void*);
  static void cb_Flush1(Fl_Button*, void*);
public:
  Fl_Counter *histogrambinsize_counter;
private:
  void cb_histogrambinsize_counter_i(Fl_Counter*, void*);
  static void cb_histogrambinsize_counter(Fl_Counter*, void*);
public:
  Fl_Counter *histogramxmax_counter;
private:
  void cb_histogramxmax_counter_i(Fl_Counter*, void*);
  static void cb_histogramxmax_counter(Fl_Counter*, void*);
  void cb_Save1_i(Fl_Button*, void*);
  static void cb_Save1(Fl_Button*, void*);
public:
  Fl_Value_Output *frameTime;
  Fl_Value_Output *framenumOutput;
private:
  void cb_reset_i(Fl_Button*, void*);
  static void cb_reset(Fl_Button*, void*);
public:
  Fl_Light_Button *initializeBut;
private:
  void cb_initializeBut_i(Fl_Light_Button*, void*);
  static void cb_initializeBut(Fl_Light_Button*, void*);
public:
  Fl_Button *startReadingDataButton;
private:
  void cb_startReadingDataButton_i(Fl_Button*, void*);
  static void cb_startReadingDataButton(Fl_Button*, void*);
public:
  Fl_Light_Button *closeBut;
private:
  void cb_closeBut_i(Fl_Light_Button*, void*);
  static void cb_closeBut(Fl_Light_Button*, void*);
public:
  mainLightcurve *mainLightcurveWindow;
private:
  void cb_Flush2_i(Fl_Button*, void*);
  static void cb_Flush2(Fl_Button*, void*);
public:
  Fl_Value_Output *ctsOutput;
  Fl_Counter *timebinsize_counter;
private:
  void cb_timebinsize_counter_i(Fl_Counter*, void*);
  static void cb_timebinsize_counter(Fl_Counter*, void*);
public:
  Fl_Counter *lightcurvexmax_counter;
private:
  void cb_lightcurvexmax_counter_i(Fl_Counter*, void*);
  static void cb_lightcurvexmax_counter(Fl_Counter*, void*);
public:
  Fl_Value_Slider *mainLightcurve_ymaxslider;
private:
  void cb_mainLightcurve_ymaxslider_i(Fl_Value_Slider*, void*);
  static void cb_mainLightcurve_ymaxslider(Fl_Value_Slider*, void*);
public:
  Fl_Button *sendParamsBut;
private:
  void cb_sendParamsBut_i(Fl_Button*, void*);
  static void cb_sendParamsBut(Fl_Button*, void*);
public:
  Fl_Button *setTrigBut;
private:
  void cb_setTrigBut_i(Fl_Button*, void*);
  static void cb_setTrigBut(Fl_Button*, void*);
public:
  Fl_Button *setHoldBut;
private:
  void cb_setHoldBut_i(Fl_Button*, void*);
  static void cb_setHoldBut(Fl_Button*, void*);
public:
  Fl_Light_Button *glitchBut;
  Fl_Value_Input *nEvents;
  Fl_Light_Button *writeFileBut;
private:
  void cb_writeFileBut_i(Fl_Light_Button*, void*);
  static void cb_writeFileBut(Fl_Light_Button*, void*);
public:
  Fl_Button *stopReadingDataButton;
private:
  void cb_stopReadingDataButton_i(Fl_Button*, void*);
  static void cb_stopReadingDataButton(Fl_Button*, void*);
public:
  Fl_Text_Display *consoleBuf;
private:
  void cb_Clear_i(Fl_Button*, void*);
  static void cb_Clear(Fl_Button*, void*);
public:
  Fl_Check_Button *printasicframe_button;
  Fl_Value_Output *nEventsDone;
  Fl_Group *detector_choice;
  Fl_Check_Button *detector1_checkbox;
  Fl_Check_Button *detector2_checkbox;
  Fl_Check_Button *detector3_checkbox;
  Fl_Check_Button *detector4_checkbox;
  Fl_Check_Button *detector5_checkbox;
  Fl_Check_Button *detector6_checkbox;
  Fl_Check_Button *detector7_checkbox;
  Fl_Value_Output *inttimeOutput;
  Fl_Double_Window *sendParamsWindow;
  Fl_Button *sendParamsWindow_sendBut;
private:
  void cb_sendParamsWindow_sendBut_i(Fl_Button*, void*);
  static void cb_sendParamsWindow_sendBut(Fl_Button*, void*);
  void cb_Close_i(Fl_Button*, void*);
  static void cb_Close(Fl_Button*, void*);
public:
  Fl_Value_Input *sendParamsWindow_value[45];
  Fl_Light_Button *sendParamsWindow_chan[64];
  Fl_Value_Input *sendParamsWindow_asic;
private:
  void cb_sendParamsWindow_asic_i(Fl_Value_Input*, void*);
  static void cb_sendParamsWindow_asic(Fl_Value_Input*, void*);
  void cb_set_i(Fl_Button*, void*);
  static void cb_set(Fl_Button*, void*);
  void cb_clear_i(Fl_Button*, void*);
  static void cb_clear(Fl_Button*, void*);
public:
  Fl_Light_Button *sendParamsWindow_test[64];
private:
  void cb_set1_i(Fl_Button*, void*);
  static void cb_set1(Fl_Button*, void*);
  void cb_clear1_i(Fl_Button*, void*);
  static void cb_clear1(Fl_Button*, void*);
public:
  Fl_Double_Window *setHoldTimeWindow;
  Fl_Value_Input *setHoldTimeWindow_holdTime;
  Fl_Button *setHoldTimeWindow_setBut;
private:
  void cb_setHoldTimeWindow_setBut_i(Fl_Button*, void*);
  static void cb_setHoldTimeWindow_setBut(Fl_Button*, void*);
public:
  Fl_Button *setHoldTimeWindow_autorunBut;
private:
  void cb_setHoldTimeWindow_autorunBut_i(Fl_Button*, void*);
  static void cb_setHoldTimeWindow_autorunBut(Fl_Button*, void*);
  void cb_Close1_i(Fl_Button*, void*);
  static void cb_Close1(Fl_Button*, void*);
public:
  Fl_Double_Window *setTrigWindow;
  Fl_Value_Input *setTrigWindow_delayTime;
  Fl_Value_Input *setTrigWindow_timeoutTime;
private:
  void cb_Close2_i(Fl_Button*, void*);
  static void cb_Close2(Fl_Button*, void*);
public:
  Fl_Button *setTrigWindow_setDelay;
private:
  void cb_setTrigWindow_setDelay_i(Fl_Button*, void*);
  static void cb_setTrigWindow_setDelay(Fl_Button*, void*);
public:
  Fl_Button *setTrigWindow_setTimeout;
private:
  void cb_setTrigWindow_setTimeout_i(Fl_Button*, void*);
  static void cb_setTrigWindow_setTimeout(Fl_Button*, void*);
public:
  Fl_Value_Input *setTrigWindow_useTimeout;
private:
  void cb_setTrigWindow_useTimeout_i(Fl_Value_Input*, void*);
  static void cb_setTrigWindow_useTimeout(Fl_Value_Input*, void*);
public:
  Fl_Button *setTrigWindow_setTrigMode;
private:
  void cb_setTrigWindow_setTrigMode_i(Fl_Button*, void*);
  static void cb_setTrigWindow_setTrigMode(Fl_Button*, void*);
public:
  Fl_Double_Window *sendCommandsWindow;
private:
  void cb_Send_i(Fl_Button*, void*);
  static void cb_Send(Fl_Button*, void*);
  void cb_Close3_i(Fl_Button*, void*);
  static void cb_Close3(Fl_Button*, void*);
  void cb_Send1_i(Fl_Button*, void*);
  static void cb_Send1(Fl_Button*, void*);
  void cb_Strobe_i(Fl_Button*, void*);
  static void cb_Strobe(Fl_Button*, void*);
  void cb_Strobe1_i(Fl_Button*, void*);
  static void cb_Strobe1(Fl_Button*, void*);
public:
  Fl_Value_Input *highVoltage_input;
  Fl_Value_Input *clockLow_input;
  Fl_Value_Input *clockHigh_input;
  Fl_Double_Window *AboutWindow;
  Fl_Double_Window *PreferenceWindow;
  Fl_Value_Input *pixelhalflife_value;
  Fl_Choice *fileTypeChoice;
  static Fl_Menu_Item menu_fileTypeChoice[];
private:
  void cb_OK_i(Fl_Button*, void*);
  static void cb_OK(Fl_Button*, void*);
  void cb_Cancel_i(Fl_Button*, void*);
  static void cb_Cancel(Fl_Button*, void*);
  void cb_Change_i(Fl_Button*, void*);
  static void cb_Change(Fl_Button*, void*);
public:
  Fl_File_Input *datafilesavedir_fileInput;
  Fl_Value_Input *readdelay_value;
  Fl_Choice *DataSource_choice;
  static Fl_Menu_Item menu_DataSource_choice[];
  Fl_Check_Button *newControlRegisters_check;
private:
  void cb_Change1_i(Fl_Button*, void*);
  static void cb_Change1(Fl_Button*, void*);
public:
  Fl_File_Input *gsesyncfile_fileInput;
  void show();
  Application *app; 
  Foxsidata *data; 
  USB_d2xx *usb; 
  Fl_Text_Buffer *buff; 
  Fl_Preferences *prefs; 
};
#endif
