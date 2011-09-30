//------------------------------------------------------------------------
// gsetest.cpp
//
//
// 
//------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>

#include "okFrontPanelDLL.h"

using namespace std;

//define the number of threads that we will create in this example
#define NUM_THREADS	8

#define MAXPATH 128
#define CONFIGURATION_FILE         "gsesync.bit"

extern int errno;
struct tm *times;
time_t ltime;
char obsfilespec[MAXPATH];

okCFrontPanel *dev;

//  set up phase lock loop
//  load FPGA code

okCFrontPanel *
initializeFPGA()
{
	bool bresult;
	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(NULL);
	}
	
	switch (dev->GetBoardModel()) {
		case okCFrontPanel::brdXEM3005:
			printf("                                        Found a device: XEM3005\n");
			break;
		default:
			printf("Unsupported device.\n");
			delete dev;
			return(NULL);
	}


	// Configure the PLL appropriately
	okCPLL22150 *pll = new okCPLL22150 ;
 	pll->SetReference(48.0f, false);
 	bresult = pll->SetVCOParameters(574, 105); // output 32 x 8.2 MHz - close to 8.192 MHz
	//cout << "Settin VCO Parameters " << bresult << endl;
 	pll->SetDiv1(okCPLL22150::DivSrc_VCO, 8);
// 32.8 approcimately 16 x 2.048 MHz
 	pll->SetDiv2(okCPLL22150::DivSrc_VCO, 127);
 	pll->SetOutputSource(0, okCPLL22150::ClkSrc_Div2By2); // Xilinx pin A8 = SYS_CLK1 (clk1)
 	pll->SetOutputEnable(0, true);
 	pll->SetOutputSource(1, okCPLL22150::ClkSrc_Div1ByN); // Xilinx pin A8 = SYS_CLK1 (clk1)
 	pll->SetOutputEnable(1, true);
 	pll->SetOutputSource(2, okCPLL22150::ClkSrc_Div1ByN); // Xilinx pin E9 = SYS_CLK2 (clk2)
 	pll->SetOutputEnable(2, true);
 	pll->SetOutputSource(3, okCPLL22150::ClkSrc_Div1ByN); // p71 = SYS_CLK4
 	pll->SetOutputEnable(3, true);
 	pll->SetOutputSource(4, okCPLL22150::ClkSrc_Div1ByN); // p69 = SYS_CLK5
 	pll->SetOutputEnable(4, true);
 	pll->SetOutputSource(5, okCPLL22150::ClkSrc_Div2ByN); //p67 = SYS_CLK6
 	pll->SetOutputEnable(5, true);
	dev->SetPLL22150Configuration(*pll);
	// Get some general information about the XEM.
	printf("                                        Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	printf("                                        Device serial number: %s\n", dev->GetSerialNumber().c_str());
	printf("                                        Device ID string: %s\n", dev->GetDeviceID().c_str());

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(CONFIGURATION_FILE)) {
		printf("FPGA configuration failed.\n");
		delete dev;
		return(NULL);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (false == dev->IsFrontPanelEnabled()) {
		printf("                              FrontPanel support is not enabled.\n");
		delete dev;
		return(NULL);
	}

	printf("                                        FrontPanel support is enabled.\n");

	return(dev);
}

char hexnib(int hexval)
{
	hexval &= 0xf;
	if( hexval < 10) return( hexval + 0x30);
	else return(hexval + 0x37);
	
}

void toihex(char * s, unsigned short int v)
{
	*s++ = hexnib( (v >> 12) & 0xf);
	*s++ = hexnib( (v >> 8) & 0xf);
	*s++ = hexnib( (v >> 4) & 0xf);
	*s++ = hexnib( v  & 0xf);
	*s++ = 0;
}

void printframe(unsigned short int * buf)
{
	int i,j;
	char s[80];
	printf("\033[0;0H");
	for(i = 0; i < 64; i++)	
	{
		for (j = 0; j < 16; j++)
		{
			toihex(s , *buf++);
			printf("%s ",s);
		}
		printf("\n");
	}
}

unsigned short int buffer0[1024],buffer1[1024];
unsigned short int buffer[1024];
pthread_mutex_t mymutex;

int fout;
int newdisplay;

void
*readgse(void *threadid)
{
unsigned int i;
long len;
ssize_t wlen;
unsigned short int status;

        for(i=0;;i++)
        {
       
			
       len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer0);
	if(fout >0)
	{
		if( (wlen = write(fout,(const void *) buffer0,2048) ) != 2048){};
     }
	if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
	{
		if(newdisplay == 0)
		{
			memcpy((void *) buffer,(void *) buffer0,2048);
			newdisplay = 1;
		}
		pthread_mutex_unlock(&mymutex);
		
	}
       len = dev->ReadFromBlockPipeOut(0xA0,1024,2048,(unsigned char *) buffer1);
	if (pthread_mutex_trylock(&mymutex) == 0) /* if fail missed as main hasn't finished */
	{
		memcpy((void *) buffer,(void *) buffer1,2048);
		pthread_mutex_unlock(&mymutex);
		
	}
 			if(fout >0)
			{
				if( (wlen = write(fout,(const void *) buffer1,2048) ) != 2048){};
		    }
        }
}


void Startfile()
{
	char stringtemp[80];
	time(&ltime);
	times = localtime(&ltime);
	 strftime(stringtemp,23,"data_%y%m%d_%H%M.dat",times);
	strncpy(obsfilespec,stringtemp,MAXPATH - 1);
	obsfilespec[MAXPATH - 1] = '\0';
	printf("%s \r",obsfilespec);
	{
		if((fout = open(obsfilespec,O_RDWR|O_CREAT,0600)) < 0)
		printf("Cannot open file\n");
    }    
	
}

int displayonce,displaycont;
char inbuffer[80];

void
*charcommand(void *threadid)
{
int mychar;
if(setvbuf(stdin,(char *) NULL,_IONBF,0) != 0) printf("Didn't set buffer\n");
for(;;)
{
	mychar = getchar();
	if( (mychar == 'c') || (mychar == 'C') )
	{
		if( displaycont != 0) displaycont = 0;
		else displaycont = 1;
	}
	if((mychar == 'd') || (mychar == 'D'))
	{
		displayonce = 1;
	}
	if((mychar == 'f') || (mychar == 'F'))
	{
		if(fout <= 0)
		{
			printf(" Opening ");
			Startfile();
		}
		else
		{
			printf(" Closing %s \r",obsfilespec);
			close(fout);
			fout = 0;
		}
	}
	if(mychar == 0xa) printf("\033[1A");
}	
}

int
main(int argc, char *argv[])
{
	
	char dll_date[32], dll_time[32];
	char waitnum;
	int nofile;
	char *parseptr;
    pthread_t threads[NUM_THREADS];
    struct sched_param param;
    pthread_attr_t tattr;

    int *taskids[NUM_THREADS];
    int ret, t;
	
	displayonce = 0;
	displaycont = 1;
	
    // define the custom priority for one of the threads
    int newprio = -10;
    // define the thread that will get the custom priority
    int prioid = 0;
    
    param.sched_priority = newprio;
    ret = pthread_attr_init(&tattr);

    // set the new scheduling param
    ret = pthread_attr_setschedparam (&tattr, &param);
	
	printf("\033[2J \033[65;0H                                        ---- FOXSI gse setup ----\n");
	if (FALSE == okFrontPanelDLL_LoadLib(NULL)) 
	{
		printf("FrontPanel DLL could not be loaded.\n");
		return(-1);
	}
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf("                              FrontPanel DLL loaded.Built: %s %s\n", dll_date, dll_time);


	while (argc > 1) 
	{
		++argv;
		parseptr = *argv;
		if(*parseptr == '-')
		{
			if(*(parseptr + 1) == 'n') nofile = 1;
		}
		argc--;
	}

	// Initialize the FPGA with our configuration bitfile.
	okCFrontPanel *devi = initializeFPGA();
	if (NULL == devi) 
	{
		printf("FPGA could not be initialized.\n");
		return(-1);
	}
	
	fout = 0;
	printf("\033[65;0H ");
	if( nofile == 0)
	{
		printf(" Opening ");
		Startfile();
	}

        if (NULL != devi)
        {
	
           ret = pthread_create(&threads[0], &tattr, readgse, (void *) taskids[0]);
			pthread_mutex_init(&mymutex,NULL);
			ret = pthread_create(&threads[0], NULL, charcommand, (void *) taskids[1]);
			
        }
		for(;;)
		{
			if( newdisplay == 1)
			{
				pthread_mutex_lock( &mymutex); /* wait on readgse */
				if((displayonce != 0) ||(displaycont != 0))
				{
					displayonce = 0;
			    	printframe(buffer);
					newdisplay = 0;
					fflush(stdout);
				}
				pthread_mutex_unlock(&mymutex);
			}

		}

	return(0);
}
