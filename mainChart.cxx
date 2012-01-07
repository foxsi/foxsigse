
#include "mainChart.h"
#include "gui.h"

extern Gui *gui;

mainChart::mainChart(int x,int y,int w,int h,const char *l)
: Fl_Chart(x,y,w,h,l)
{
}

// the drawing method: draws the histFunc into the window

