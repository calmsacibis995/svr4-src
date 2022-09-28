/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/ATT_s_filter.c	1.1.2.1"

#define		LENGTH		66
#define		LENMIN		1
#define		LENMAX		120

#define		CR		'\015'		/* Carriage Return	*/
#define		LF		'\012'		/* Line Feed		*/
#define		FF		'\014'		/* Form Feed		*/
#define		ESC		'\033'		/* Escape		*/
#define		FT1455         "\033i"         /* Feed from Tray 1	*/	
#define		FT1457         "\033f"         /* Feed from front tray.*/
#define		FT2455         "\033j"         /* Feed from Tray 2	*/	
#define		FT2457         "\033s"         /* Feed from back tray. */
#define		FORCEXEC       "\033X"         /* Execute accumulated motions*/
#define		FTEJECT        "\033e"         /* Eject to forms tray. */

#include	<stdio.h>
#include "signal.h"
#include <string.h>

#define E_SUCCESS	0
#define E_WRITE_FAILED	1
#define E_HANGUP	2
#define E_INTERRUPT	3
#define E_MALLOC_FAILED	4


void    sighup(),
        sigint(),
        sigpipe(),
        sigquit(),
        sigterm();

char *printer = 0;
int traynum, firstpage;

main(argc,argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;
	extern int errno, opterr;
	int c; 

	char *malloc();

	char buf[1024];
	int paglen, linecnt,  i, j, r;
	firstpage = 1; 	
	paglen  = LENGTH;	/* in case no arguement specified, set to */
	traynum = 1; 		/* default values */



        opterr = 0;  /* getopt will return error messages on the Standard  */
                     /* Error. This will lead to a PRINTER FAULT with the  */
                     /* new LP Spooler. Setting opterr to zero disables it */

    while ((c = getopt(argc,argv,"w:t:T:")) != EOF) {
	switch (c) {
              case 'w' : paglen = atoi(optarg);
		if (paglen < LENMIN || paglen > LENMAX) {
			fprintf(stdout,"Length should be between %d and\
 %d, defaulting to %d.\n",LENMIN,LENMAX,LENGTH);
			paglen = LENGTH;
		} /* of if paglen */
                         break;
              case 'T' : traynum = atoi(optarg);
			 if (traynum < 1 || traynum > 3) {
				fprintf(stdout,"Tray Number should be:\n\t1=feed\ from tray 1 (front) only,\n\t2=feed from tray 2 (back) only,\n\t3=feed first she\et from tray 1 and remainder from tray 2.\nDefaulting to 1.\n");
				traynum =1;
			} /* of if traynum */
                         break;
              case 't' :  
                     if ((printer = malloc(strlen(optarg) + 1)) == NULL) {
fprintf(stderr,"ATT_sf filter: Failure to allocate memory for Terminal Type\n");
                             exit(E_MALLOC_FAILED);
                         }
                         if (printer) {
                           strcpy(printer,optarg);
                           if ( ! ( (strcmp(printer,"455") == 0)    ||
                                    (strcmp(printer,"att455") == 0) ||
                                    (strcmp(printer,"458") == 0)    ||
                                    (strcmp(printer,"att458") == 0) ||
                                    (strcmp(printer,"457") == 0)    ||
                                    (strcmp(printer,"att457") == 0) ) ) { 
				fprintf(stdout,"Printer type must be one of the types: 455, att455, 457, att457, 458, or att458\n");
				fprintf(stdout,"Defaulting to 455\n");
                                strcpy(printer,"455");
			   } /* of if */
                         } /* of if printer */
                         break;
	} /* of switch c ... */
    } /* of while getopt ... */


/*
CATCHING OF PRINTER FAULT SIGNALS:
---------------------------------
sighup  - catch hangups or drop of carrier.
sigint  - interrupt, printer sent a break or quit character.
sigpipe - output port was closed early, no one to read pipe.
sigterm - software termination, exit with success - NOT a PRINTER FAULT.
*/

  signal(SIGHUP,sighup);
  signal(SIGINT,sigint);
  signal(SIGQUIT,sigint);
  signal(SIGPIPE,sigpipe);
  signal(SIGTERM,sigterm);


	/* read a block of characters at  a time */
	while ((r = read(0,buf,1024)) > 0)
	{
		i = 0;
		while (i < r)
		{
			/* regular characters */
			if (buf[i] >= ' ')
				putchar(buf[i++]);

			/* form feed to be changed */
			else if (buf[i] == FF)
			     { feed();
			       linecnt = 0;
			       i++;	/* bypass this char */
			      }
			else if (buf[i] == LF)
			{
				putchar(buf[i++]);
				++linecnt;
			}
			else putchar(buf[i++]);
			/* check if line limit reached */
			if (linecnt >= paglen)
			{
				feed();
				linecnt = 0;
			}
		}
	}
	printf("%s",FORCEXEC);   /*execute accumulated motions*/
	printf("%s",FTEJECT);   /*clear last sheet*/
exit(E_SUCCESS) ;
} 
feed()
{
	printf("%s",FORCEXEC);   /*execute accumulated motions*/
	switch (traynum)
		{
	case 1: printf("%s",FTEJECT);     
		if ( (strcmp(printer,"455") == 0) || 
                     (strcmp(printer,"att455") == 0) )
			printf("%s",FT1455);
		else
			printf("%s",FT1457);
		break;   
	case 2: printf("%s",FTEJECT);     
		if ( (strcmp(printer,"455") == 0) || 
                     (strcmp(printer,"att455") == 0) )
			printf("%s",FT2455);
		else
			printf("%s",FT2457);
		break;   
	case 3:
		printf("%s",FTEJECT);     
		if ( firstpage == 1 )
		{
		firstpage = 0;
		if ( (strcmp(printer,"455") == 0) || 
                     (strcmp(printer,"att455") == 0) )
			printf("%s",FT1455);
		else
			printf("%s",FT1457);
		break;
		}	
		if ( (strcmp(printer,"455") == 0) || 
                     (strcmp(printer,"att455") == 0) )
			printf("%s",FT2455);
		else
			printf("%s",FT2457);
		break;   
	default:
		break;
		}
	}

/* sighup: CATCH A HANGUP - A LOSS OF CARRIER */
void sighup()
{
 signal(SIGHUP,SIG_IGN);
 fprintf(stderr,"ATT_sf filter: The connection to the Printer Dropped; perhaps \
it has gone off-line.\n");
 exit (E_HANGUP);
} /* end of sighup */

/* sigint - CATCH AN INTERRUPT */
void sigint()
{
 signal(SIGINT,SIG_IGN);
 fprintf(stderr,"ATT_sf filter: Received an interrupt from the printer. The \
reason is unknown,\n a common cause is that the baud rate is too high.\n");
 exit (E_INTERRUPT);
}

/* sigpipe - CATCH EARLY CLOSE OF PIPE */
void sigpipe()
{
 signal(SIGPIPE,SIG_IGN);
 fprintf(stderr,"ATT_sf filter: The Output Port was closed before all output \
could be written.\n");
exit (E_INTERRUPT);
}

/* SIGTERM - CATCH A TERMINATION SIGNAL */
void sigterm()
{
 signal(SIGTERM,SIG_IGN);
 fprintf(stderr,"ATT_sf filter: Caught software termination signal.\n");
 exit(E_SUCCESS);
}
