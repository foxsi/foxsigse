
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

int fsercmd,ttyout,devicefile,clockvalue;
unsigned char cmd[20];
struct termios sertty;
struct stat mystat;

main(argc,argv)
int  argc;
char *argv[ ];
{
	int status;

        if (argc < 2) 
        {
                printf("No file specified\n");
                exit(-1);
        }
        if( (fsercmd = open(argv[1],O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
        {
                printf("Error opening file %s  (if disk file must exist)\n" ,argv[1]);
                exit(-1);
        }
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
        if( argc > 2) // parse value
        {
	        sscanf(argv[2],"%u",&clockvalue);
        }
        else
        {
			printf(" No value given \n");
			exit(-1);
        }
		printf("Setting clock to %u  , %x \n",clockvalue,clockvalue);
		cmd[0] = 0xf8;
		cmd[1] = 0x0;
		cmd[2] = (unsigned char) (clockvalue &0xff);
		cmd[3] = 0x0;
		cmd[3] ^= cmd[0];
		cmd[3] ^= cmd[1];
		cmd[3] ^= cmd[2];
		if(devicefile != 1) printf("Writing to file, not serial port\n");
		write(fsercmd,&cmd,4);
		printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
		cmd[0] = 0xf8;
		cmd[1] = 0x01;
		cmd[2] = (unsigned char) ( (clockvalue >> 8) & 0xff);
		cmd[3] = 0x0;
		cmd[3] ^= cmd[0];
		cmd[3] ^= cmd[1];
		cmd[3] ^= cmd[2];
		write(fsercmd,&cmd,4);
		printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
		cmd[0] = 0xf8;
		cmd[1] = 0x02;
		cmd[2] = (unsigned char) ( (clockvalue >> 16) & 0xff);
		cmd[3] = 0x0;
		cmd[3] ^= cmd[0];
		cmd[3] ^= cmd[1];
		cmd[3] ^= cmd[2];
		write(fsercmd,&cmd,4);
		printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
		cmd[0] = 0xf8;
		cmd[1] = 0x03;
		cmd[2] = (unsigned char) ( (clockvalue >> 24) & 0xff);
		cmd[3] = 0x0;
		cmd[3] ^= cmd[0];
		cmd[3] ^= cmd[1];
		cmd[3] ^= cmd[2];
		write(fsercmd,&cmd,4);
		printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
		cmd[0] = 0xf8;
		cmd[1] = 0x07;
		cmd[2] = 0x0;
		cmd[3] = 0x0;
		cmd[3] ^= cmd[0];
		cmd[3] ^= cmd[1];
		cmd[3] ^= cmd[2];
		write(fsercmd,&cmd,4);
		printf("Sending bytes %02x %02x %02x %02x \n",cmd[0],cmd[1],cmd[2],cmd[3]);
		sleep(1);


}

