/* Temperv14-LKM By Justin S. Yang 
 * Modified to use a linux kernel driver instead of libusb
 * 
 * temperv14.c By Steffan Slot (dev-random.net) based on the version 
 * below by Juan Carlos.
 * 
 * pcsensor.c by Juan Carlos Perez (c) 2011 (cray@isp-sl.com)
 * based on Temper.c by Robert Kavaler (c) 2009 (relavak.com)
 * All rights reserved.
 *
 * Temper driver for linux. This program can be compiled either as a library
 * or as a standalone program (-DUNIT_TEST). The driver will work with some
 * TEMPer usb devices from RDing (www.PCsensor.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 * THIS SOFTWARE IS PROVIDED BY Justin Yang ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Justin Yang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/*
 * Modified from original version (0.0.1) by Patrick Skillen (pskillen@gmail.com)
 * - 2013-01-10 - If outputting only deg C or dec F, do not print the date (to make scripting easier)
 */
/*
 * 2013-08-19
 * Modified from abobe version for better name, and support to 
 * subsctract degress in celsius for some TEMPer devices that displayed 
 * too much.
 *
 *
 * 2016-05-09
 * Modified to use LKM. -Justin Yang
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_DEVICE "/dev/temp0"
#define BUF_SIZE 8
#define DEFAULT_DURATION        800
#define VERSION "1.0"

static int formato=0;
static int bsalir=1;
static int seconds=5;
static int mrtg=0;

static int subtract = 0;
const static char tempQ[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };


int main( int argc, char **argv) {
 
     float tempc;
     int c;
     time_t t;
     struct tm *local;
     unsigned char buffer[BUF_SIZE];
     int fd;
     char *dev = DEFAULT_DEVICE;


     while ((c = getopt (argc, argv, "mfchl::")) != -1)
     switch (c)
       {
       case 'c':
         formato=1; //Celsius
         break;
       case 'f':
         formato=2; //Fahrenheit
         break;
       case 'm':
         mrtg=1;
         break;
       case 'l':
         if (optarg!=NULL){
           if (!sscanf(optarg,"%i",&seconds)==1) {
             fprintf (stderr, "Error: '%s' is not numeric.\n", optarg);
             exit(EXIT_FAILURE);
           } else {           
              bsalir = 0;
              break;
           }
         } else {
           bsalir = 0;
           seconds = 5;
           break;
         }
       case '?':
       case 'h':
         printf("pcsensor version %s\n",VERSION);
	 printf("      Aviable options:\n");
	 printf("          -h help\n");
	 printf("          -v verbose\n");
	 printf("          -l[n] loop every 'n' seconds, default value is 5s\n");
	 printf("          -c output only in Celsius\n");
	 printf("          -f output only in Fahrenheit\n");
	 printf("          -m output for mrtg integration\n");
  
	 exit(EXIT_FAILURE);
       default:
         if (isprint (optopt))
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
           fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
         exit(EXIT_FAILURE);
       }

     if (optind < argc) {
        fprintf(stderr, "Non-option ARGV-elements, try -h for help.\n");
        exit(EXIT_FAILURE);
     }

	fd = open(dev,O_RDWR);
     do {
		write(fd,tempQ,1);
		read(fd,&buffer,BUF_SIZE);
		tempc = (buffer[3] & 0xFF) + (buffer[2] << 8);
		tempc *= (125.0 / 32000.0);
		tempc = (tempc - subtract);

           t = time(NULL);
           local = localtime(&t);

           if (mrtg) {
              if (formato==2) {
                  printf("%.2f\n", (9.0 / 5.0 * tempc + 32.0));
                  printf("%.2f\n", (9.0 / 5.0 * tempc + 32.0));
              } else {
                  printf("%.2f\n", tempc);
                  printf("%.2f\n", tempc);
              }
              
              printf("%02d:%02d\n", 
                          local->tm_hour,
                          local->tm_min);

              printf("pcsensor\n");
           } else {
              if (formato==2) {
                  printf("%.2f\n", (9.0 / 5.0 * tempc + 32.0));
              } else if (formato==1) {
                  printf("%.2f\n", tempc);
              } else {
				  printf("%04d/%02d/%02d %02d:%02d:%02d ", 
							  local->tm_year +1900, 
							  local->tm_mon + 1, 
							  local->tm_mday,
							  local->tm_hour,
							  local->tm_min,
							  local->tm_sec);

				  printf("Temperature %.2fF %.2fC\n", (9.0 / 5.0 * tempc + 32.0), tempc);
              }
           }
           
           if (!bsalir)
              sleep(seconds);
     } while (!bsalir);

return 0;
}
