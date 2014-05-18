/*******************************************************************
 *                     MYM Player to Serial port
 *                 (c) 2014 Manoel "Godzil" Trapier
 *
 * This file is base on the mym2ym by 
 * Marq/Lieves!Tuore & Fit (marq@iki.fi)
 *
 * The YM writing part have been removed and replaced by code to 
 * send register dump to the serial port.
 **************************** Licence ******************************
 * This file is licenced under the licence:
 *                    WTFPL v2 Postal Card Edition:
 *
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                    Version 2, December 2004
 *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 *  1. If you like this software you can send me a (virtual) postals
 *     card. Details bellow:
 *
 *             < godzil-nospambot at godzil dot net >
 *
 * If you wan't to send a real postal card, send me an email, I'll
 * give you my address. Of course remove the -nospambot from my
 * e-mail address.
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define REGS 14
#define FRAG 128    /*  Nuber of rows to compress at a time   */
#define OFFNUM 14   /*  Bits needed to store off+num of FRAG  */

unsigned readbits(int bits,FILE *f);

int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes", errno);
}

int main(int argc,char *argv[])
{
    unsigned char   *data[REGS],    /*  The unpacked YM data    */
                    c;
    int serial_fd;


    unsigned    current[REGS];

    FILE    *f;

    long    n,i,row,index,compoff,compnum,
            bytes=0,
            regbits[REGS]={8,4,8,4, 8,4,5,8, 5,5,5,8, 8,8}; /* Bits per PSG reg */

    unsigned long   rows;

    if(argc!=3)
    {
        printf("Usage: mym2serial source.mym /dev/serial\n");
        return(EXIT_FAILURE);
    }

    if((f=fopen(argv[1],"rb"))==NULL)
    {
        printf("File open error.\n");
        return(EXIT_FAILURE);
    }

    rows=fgetc(f);    /*  Read the number of rows */
    rows+=fgetc(f)<<8u;

    for(n=0;n<REGS;n++)     /*  Allocate memory for rows    */
    {
        if((data[n]=(unsigned char *)malloc(rows+FRAG))==NULL)
        {
            printf("Out of memory.\n");
            exit(EXIT_FAILURE);
        }
    }

    for(n=0;n<rows;n+=FRAG) /*  Go through rows...  */
    {
        for(i=0;i<REGS;i++) /*  ... and registers   */
        {
            index=0;
            if(!readbits(1,f))  /*  Totally unchanged fragment */
            {
                for(row=0;row<FRAG;row++)
                    data[i][n+row]=current[i];
                continue;
            }

            while(index<FRAG)   /*  Packed fragment */
            {
                if(!readbits(1,f))  /*  Unchanged register  */
                {
                    
                    data[i][n+index]=current[i];
                    index++;
                }
                else
                {
                    if(readbits(1,f))   /*  Raw data    */
                    {
                        c=readbits(regbits[i],f);
                        current[i]=data[i][n+index]=c;
                        index++;
                    }
                    else    /*  Reference to previous data */
                    {
                        compoff=readbits(OFFNUM/2,f)+index;
                        compnum=readbits(OFFNUM/2,f)+1;

                        for(row=0;row<compnum;row++)
                        {
                            c=data[i][n-FRAG+compoff+row];
                            data[i][n+index]=current[i]=c;
                            index++;
                        }
                    }
                }
            }
        }
    }

    fclose(f);

    serial_fd = open (argv[2], O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd > 0)
    {
        char buffer[14];
        printf("Serial opened...\n");
        set_interface_attribs (serial_fd, B115200, 0); // 115200 8n1
        set_blocking (serial_fd, 0);
        /* Wait a little bit to be sure that the other end is ready... */
        sleep(2);
        printf("Playing now!\n");
        for(n=0;n<rows;n++)
        {
            for(i=0;i<REGS;i++)
            {
                buffer[i] = data[i][n];
            }
            write(serial_fd, buffer, 14);
            // 50Hz so wait a little bit...
            printf("\rVBL%ld ", n);
            fflush(stdout);
            usleep(20*1000); /* 50Hz */
        }
        printf("\n");
    }
    else
    {
        printf("Opening error...\n");
    }

    return(EXIT_SUCCESS);
}

/*  Reads bits from a while */
unsigned readbits(int bits,FILE *f)
{
    static unsigned char    byte;

    static int  off=7;

    unsigned    n,data=0;

    /* Go through the bits and read a whole byte if needed */
    for(n=0;n<bits;n++) 
    {
        data<<=1;
        if(++off==8)
        {
            byte=fgetc(f);
            off=0;
        }
        
        if(byte&(0x80>>off))
            data++;
    }
    return(data);
}
