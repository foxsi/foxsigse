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
#include "mainHistogram.h"
#include "mainImage.h"
#include "subImage.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Choice.H>
#include "mainLightcurve.h"
#include <FL/Fl_Value_Input.H>
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
  mainHistogram *mainHistogramWindow;
  mainImage *mainImageWindow;
  subImage *subImageWindow;
  Fl_Group *TopOutput;
  Fl_Output *rateOutput0;
  Fl_Output *rateOutput1;
  Fl_Output *rateOutput2;
  Fl_Output *rateOutput3;
  Fl_Output *rateOutput4;
  Fl_Output *rateOutput5;
  Fl_Output *rateOutput6;
  Fl_Output *rateOutput7;
  Fl_Output *shutterstateOutput;
  Fl_Output *TempOutput;
  Fl_Output *histEnergy;
  Fl_Output *histCounts;
  Fl_Group *dataPlayback;
  Fl_Button *nextFrameBut;
private:
  void cb_nextFrameBut_i(Fl_Button*, void*);
  static void cb_nextFrameBut(Fl_Button*, void*);
public:
  Fl_Button *prevFrameBut;
private:
  void cb_prevFrameBut_i(Fl_Button*, void*);
  static void cb_prevFrameBut(Fl_Button*, void*);
public:
  Fl_Light_Button *syncLightBut;
  Fl_Button *flushBut;
private:
  void cb_flushBut_i(Fl_Button*, void*);
  static void cb_flushBut(Fl_Button*, void*);
public:
  Fl_Button *printFrame;
private:
  void cb_printFrame_i(Fl_Button*, void*);
  static void cb_printFrame(Fl_Button*, void*);
public:
  Fl_Value_Output *frameTime;
  Fl_Value_Output *chipbitValOut0;
  Fl_Value_Output *chipbitValOut1;
  Fl_Value_Output *chipbitValOut2;
  Fl_Value_Output *chipbitValOut3;
  Fl_Value_Output *trigbitValOut0;
  Fl_Value_Output *trigbitValOut1;
  Fl_Value_Output *trigbitValOut2;
  Fl_Value_Output *trigbitValOut3;
  Fl_Value_Output *seubitValOut0;
  Fl_Value_Output *seubitValOut1;
  Fl_Value_Output *seubitValOut2;
  Fl_Value_Output *seubitValOut3;
  Fl_Value_Output *noiseValOut0;
  Fl_Value_Output *noiseValOut1;
  Fl_Value_Output *noiseValOut2;
  Fl_Value_Output *noiseValOut3;
  Fl_Value_Output *framenumOutput;
  Fl_Output *pixelNum;
  Fl_Output *pixelCounts;
  Fl_Light_Button *subImageLockbut;
  Fl_Light_Button *mainHistogramLockbut;
  Fl_Value_Output *mainHistogramXlabelmid;
  Fl_Value_Output *mainHistogramXlabelmax;
  Fl_Value_Output *mainHistogramYlabelmax;
  Fl_Value_Output *mainHistogramYlabelmid;
  Fl_Text_Display *consoleBuf;
  static Fl_Menu_Item menu_Detector[];
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
  Fl_Light_Button *glitchBut;
  Fl_Button *sendParamsBut;
private:
  void cb_sendParamsBut_i(Fl_Button*, void*);
  static void cb_sendParamsBut(Fl_Button*, void*);
public:
  Fl_Button *testBut;
private:
  void cb_testBut_i(Fl_Button*, void*);
  static void cb_testBut(Fl_Button*, void*);
public:
  Fl_Value_Input *nEvents;
  Fl_Light_Button *writeFileBut;
private:
  void cb_writeFileBut_i(Fl_Light_Button*, void*);
  static void cb_writeFileBut(Fl_Light_Button*, void*);
public:
  Fl_Value_Input *nEventsDone;
private:
  void cb_BREAK_i(Fl_Button*, void*);
  static void cb_BREAK(Fl_Button*, void*);
  void cb_CLEAR_i(Fl_Button*, void*);
  static void cb_CLEAR(Fl_Button*, void*);
public:
  Fl_Button *stopReadingDataButton;
private:
  void cb_stopReadingDataButton_i(Fl_Button*, void*);
  static void cb_stopReadingDataButton(Fl_Button*, void*);
  void cb_Clear_i(Fl_Button*, void*);
  static void cb_Clear(Fl_Button*, void*);
public:
  Fl_Value_Output *testOutput;
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
public:
  Fl_Value_Input *sendParamsWindow_holdTime;
private:
  void cb_Set_i(Fl_Button*, void*);
  static void cb_Set(Fl_Button*, void*);
  void cb_set_i(Fl_Button*, void*);
  static void cb_set(Fl_Button*, void*);
  void cb_clear_i(Fl_Button*, void*);
  static void cb_clear(Fl_Button*, void*);
public:
  Fl_Double_Window *sendCommandsWindow;
private:
  void cb_Send_i(Fl_Button*, void*);
  static void cb_Send(Fl_Button*, void*);
  void cb_Close1_i(Fl_Button*, void*);
  static void cb_Close1(Fl_Button*, void*);
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
  void show();
  Application *app; 
  Foxsidata *data; 
  USB_d2xx *usb; 
  Fl_Text_Buffer *buff; 
  Fl_Preferences *prefs; 
};
#endif
