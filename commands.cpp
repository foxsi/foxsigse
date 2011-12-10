/*
 *  commands.cpp
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/26/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <string>

int fsercmd,ttyout,devicefile,clocklo;
struct termios sertty;
struct stat mystat;

char serial_device_fname[20] = {"/dev/tty.KeySerial1"};
unsigned char cmd[20];

//
//filename to open
// /dev/tty.KeySerial1
//
// 32 bits of clock, lower 16 bits are sent to the formatter, rools over every few minutes, another 16 bits. 
//
//clock of 48 bits.
//
//find the command that does the transfer check the lower 32 bit command.
//
//will change at every 2 volts/seconds.

void command_attenuator_state(bool state)
{
	fsercmd = command_initialize_serial();
	
	create_cmd_attenuator(state);
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0) {write(fsercmd,&cmd,4);}
	sleep(1);
}

void command_voltage_set(int hv_value)
{
	int status;
	
	//sscanf(hv_value,"%u",&hvvalue);
	
	if( hv_value > 4095)
	{
		printf (" HV value greater than 4095, too large %d \n",hv_value);
	}
	printf("Setting HV to %u  , %x \n",hv_value,hv_value);
	
	create_cmd_hv(hv_value);
	
	fsercmd = command_initialize_serial();
	
	if (fsercmd > 0){write(fsercmd,&cmd[0],4);}
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	sleep(1);
}

void command_clock_set(int clockhi, int clocklo)
{
	int status;
	
	fsercmd = command_initialize_serial();

	printf("Setting low clock to %u  , %x \n",clocklo,clocklo);
	cmd[0] = 0xf8;
	cmd[1] = 0x0;
	cmd[2] = (unsigned char) (clocklo &0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}

	cmd[0] = 0xf8;
	cmd[1] = 0x01;
	cmd[2] = (unsigned char) ( (clocklo >> 8) & 0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	cmd[0] = 0xf8;
	cmd[1] = 0x02;
	cmd[2] = (unsigned char) ( (clocklo >> 16) & 0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	cmd[0] = 0xf8;
	cmd[1] = 0x03;
	cmd[2] = (unsigned char) ( (clocklo >> 24) & 0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	cmd[0] = 0xf8;
	cmd[1] = 0x07;
	cmd[2] = 0x0;
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	printf("Setting hi clock to %u  , %x \n",clockhi,clockhi);
	cmd[0] = 0xf8;
	cmd[1] = 0x04;
	cmd[2] = (unsigned char) (clockhi &0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	cmd[0] = 0xf8;
	cmd[1] = 0x05;
	cmd[2] = (unsigned char) ( (clockhi >> 8) & 0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (fsercmd > 0){write(fsercmd,&cmd,4);}
	
	sleep(1);
}

int command_initialize_serial(void)
{
	int status;
	
	if( (fsercmd = open(serial_device_fname,O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		printf("Error opening file %s  (if disk file must exist)\n" ,serial_device_fname);
		return 0;
	} else {
		
		fstat(fsercmd,&mystat);
		if( S_IFCHR & mystat.st_mode )
		{
			devicefile = 1;
			tcgetattr(fsercmd,&sertty); /* get serial line properties */
			sertty.c_iflag = IGNBRK;
			sertty.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG); /* raw input */
			sertty.c_oflag &= ~OPOST;
			cfsetospeed(&sertty,B1200);
			cfsetispeed(&sertty,B1200);
			sertty.c_cflag = (sertty.c_cflag & ~CSIZE) | CS8; /* 8 bits */
			sertty.c_cflag &= ~( CRTSCTS | PARENB | PARODD | CSTOPB); /*no CTS, no parity, 1 stop */
			sertty.c_cflag |= (CLOCAL | CREAD | CSTOPB); /* ok, 2 stop for safety */
			tcsetattr(fsercmd,TCSANOW,&sertty);
			//      printf(" %d  %x \n", devicefile, mystat.st_mode);
			ioctl(fsercmd, TIOCMGET, &status);
			status |= TIOCM_LE;
			status |= TIOCM_DTR;
			ioctl(fsercmd, TIOCMSET, &status);
		}
		//      fstat(fileno(stdout),&mystat);
		//      if((S_IFREG & mystat.st_mode) == 0) ttyout = 1;
		ttyout = isatty(fileno(stdout));
		return fsercmd;
	}
}

void create_cmd_attenuator(bool state)
{
	cmd[0] = 0xe8;
	if (state == 0) {
		cmd[1] = 0x00;
	} else {
		cmd[1] = 0x01;
	}
	cmd[2] = 0x0;
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
}

void create_cmd_hv(int hvvalue){	
	cmd[0] = 0xf0;
	cmd[1] = (unsigned char) ( (hvvalue >> 8) & 0xf);
	cmd[2] = (unsigned char) (hvvalue &0xff);
	cmd[3] = 0x0;
	cmd[3] ^= cmd[0];
	cmd[3] ^= cmd[1];
	cmd[3] ^= cmd[2];
	
}