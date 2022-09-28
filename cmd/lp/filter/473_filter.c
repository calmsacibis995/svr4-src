/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/473_filter.c	1.1.2.1"

#include <stdio.h>
#include "signal.h"
#include <string.h>

#define E_SUCCESS	0
#define E_WRITE_FAILED	1
#define E_HANGUP	2
#define E_INTERRUPT	3
#define E_MALLOC_FAILED	4

/* Define Initial Reset Modes to Printers */
#define CLEAR_PRINT_BUF '\030'                 /* Clear Printer Buffer   */
#define CANCEL_COMPRESS_MODE '\022'            /* Cancel Compressed Mode */
#define BIDIRECT_PRINT "\033U0"                /* Bidirectional Printing */
#define COMPRESS_SIZE '\017'                   /* Compressed Size        */


/* Define Other Escape Sequences */

#define		WIDTH		80
#define		WIDMIN		1
#define		WIDMAX		218

#define		BS		'\010'		/* Back Space		*/
#define		TAB		'\011'		/* Tab			*/
#define		CR		'\015'		/* Carriage Return	*/
#define		LF		'\012'		/* Line Feed		*/
#define		FF		'\014'		/* Form Feed		*/
#define		ESC		'\033'		/* Escape		*/
#define		UNDLINOFF	"\033-0"	/* Underline off	*/
#define		UNDLINON	"\033-1"	/* Underline on		*/
#define		SUPSCPTON	"\033S0"	/* Superscript on	*/
#define		SUBSCPTON	"\033S1"	/* Subscript on		*/
#define		SCRIPTOFF	"\033T"		/* Scripting off	*/
#define		EMPHAON		"\033E"		/* Emphasize on		*/
#define		EMPHAOFF	"\033F"		/* Emphasize off	*/
#define		DBLSTRON	"\033G"		/* Double Strike on	*/
#define		DBLSTROFF	"\033H"		/* Double Strike off	*/
#include	<stdio.h>

char *esc[] =	{
	UNDLINOFF,
	UNDLINON,
	SUPSCPTON,
	SUBSCPTON,
	SCRIPTOFF,
	EMPHAON,
	EMPHAOFF,
	DBLSTRON,
	DBLSTROFF,
	0
};

void    sighup(),
        sigint(),
        sigpipe(),
        sigquit(),
        sigterm();

char *printer = 0;

main(argc,argv)
int argc;
char *argv[];
{
    extern char *optarg;
    extern int optind;
    extern int errno, opterr;
    int c,
        linwidth_set = 0;

    char *malloc();

    int emphasized = 0,
        doublestrike = 0,
        compressed = 0,
        do_reset = 0;


	char buf[1024];
	int linwidth = WIDTH, 
            charcnt, bscnt, i, j, r, found;


/* USER OPTIONS ACCEPTED BY THE ATT473 FILTER:                           */
/* ------------------------------------------                            */
/* e = EMPHASIZED MODE ON. Ouput is of BOLD type.                        */
/* d = DOUBLESTRIKE MODE ON. Overwriting of each Single Line of OUTPUT.  */
/* c = Compressed MODE ON. Bigger CPI Value USED.                        */
/* w = LINE WIDTH SIZE. User Control of MAX. No. of Characters per line. */
/* t = Printer type - Terminfo Entry for the Printer.                    */
/* r = Reset the PRINTER. By default all archived Filters will NOT reset */
/*     the Printer. This is so because the output from a FILTER might be */
/*     PIPED to yet another FILTER - in such a case we do not want the   */
/*     RESET PRINTER ESCAPE CODES to be Input to some other Filter.      */

    opterr = 0;      /* getopt will return error messages on the Standard  */
                     /* Error. This will lead to a PRINTER FAULT with the  */
                     /* new LP Spooler. Setting opterr to zero disables it */

    while ((c = getopt(argc,argv,"edcrw:t:")) != EOF) {
	switch (c) {
              case 'e' : emphasized =  1;          /* EMPHASIZED MODE ON     */
                         break;
              case 'd' : doublestrike = 1;         /* DOUBLE STRIKE MODE ON  */
                         break;
              case 'c' : compressed =  1;          /* COMPRESSED MODE ON     */
                         break;
              case 'r' : do_reset = 1;       /* NO PRINTER RESET       */
                         break;
              case 't' :  
                     if ((printer = malloc(strlen(optarg) + 1)) == NULL) {
fprintf(stderr,"ATT473 filter: Failure to allocate memory for Terminal Type\n");
                             exit(E_MALLOC_FAILED);
                         }
                     else strcpy(printer,optarg);
                         if (! linwidth_set) {
                           if ( (printer) &&  
                                ( (strcmp(printer,"474") == 0)    ||
                                  (strcmp(printer,"att474") == 0) ||
                                  (strcmp(printer,"479") == 0)    ||
                                  (strcmp(printer,"att479") == 0) ) )
                             linwidth = 132 ;     /* wide platten printers    */
                            else linwidth = 80 ;  /* narrow plattern printers */
                         } /* of if not linwidth_set */
                         break;
              case 'w' : linwidth = atoi(optarg);  /* USER LINE WIDTH OPTION */
		if (linwidth < WIDMIN || linwidth > WIDMAX)
		{

/* USER ERROR - INVALID LINEWIDTH.                                           */
/* NOTE: According to the NEW LP SPOOLER - all User Errors should go to the  */
/*       STANDARD OUTPUT, the Printer,  (and NOT the STANDARD ERROR) - where */
/*       the User can see the Error. STANDARD ERROR Messages are trapped by  */
/*       the Interface and detected solely as PRINTER FAULTS only.           */

	        fprintf(stdout,"ATT473 Filter: Width should be between %d and\
 %d, defaulting to %d.\n",WIDMIN,WIDMAX,WIDTH);
			linwidth = WIDTH;
		} /* of if linwidth */
                        linwidth_set = 1;
                       break;
	} /* of switch c ... */
    } /* of while getopt ... */

if (do_reset) reset_modes();  /* RESET THE PHYSICAL PRINTER iff the */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */

/* NOW THAT PRINTER IS RESET - CHECK OPTIONS GIVEN BY THE USER             */

/*  If Emphasized ON then do NOT check for Double-Strike. This is how the  */
/*  Old Model User to Work.                                                */

    if (emphasized) printf("%s",EMPHAON); 
    else if (doublestrike) printf("%s",DBLSTRON);

    if (compressed) {
/* IN COMPRESSED MODE LINESIZE - linwith should be greater or equal to 132 */
      if (linwidth < 132) linwidth = 132;
      printf("%c",COMPRESS_SIZE);
    }

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

/* NOW PRINT THE INPUT FILE - Decoding Necessary Escape Sequences - if any */

	charcnt = 0;
	/* read a block of characters at  a time */
	while ((r = read(0,buf,1024)) > 0)
	{
		i = 0;
		while (i < r)
		{
			/* regular characters */
			if (buf[i] >= ' ')
				putchar(buf[i++]);

			/* control characters */
			else if (buf[i] == CR)
			{
				putchar(buf[i++]);
				charcnt = 0;
				continue;
			}
			else if (buf[i] == BS)	/* back space gets converted
						   to CR. The amount of chars
						   already on the line minus
						   the amount of back spaces,
						   is the amount to space over.
						 */
			{
				bscnt = 1;
				while (buf[++i] == BS)
					++bscnt; /* count back spaces */
				if (bscnt > charcnt)
					charcnt = 0;
				else
					charcnt -= bscnt;
				putchar(CR);
				j = charcnt;
				while (j--)
					putchar(' ');
				continue;
			}
			else if (buf[i] == TAB)
			{
				putchar(buf[i++]);
				/* add only 7, because 1 will be added later*/
				charcnt += 7 - (charcnt % 8);
			}
			else if (buf[i] == LF || buf[i] == FF)
			{
				putchar(CR);
				putchar(buf[i++]);
				charcnt = 0;
				continue;
			}
			/* escape sequences */
			else if (buf[i] == ESC)
			{
				j = found = 0;
				while (esc[j] != 0)
				{
					if (strncmp(&buf[i],esc[j],
						strlen(esc[j])) == 0)
					{
						found = 1;
						printf("%s",esc[j]);
						i += strlen(esc[j]);
						break;
					}
					++j;
				}
				if (found) /* escape was part of string */
					continue;
				else
					putchar(buf[i++]);
			}
			else
				putchar(buf[i++]);
			++charcnt;
			/* check if line limit reached */
			if (charcnt >= linwidth)
			{
				putchar(CR);
				putchar(LF);
				charcnt = 0;
			}
		}
	}

if (do_reset) reset_modes();  /* AT END RESET BACK PRINTER iff the    */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */

exit(E_SUCCESS);      /* ALL FILTERS MUST SPECIFICALLY EXIT WITH A CODE OF 0 */
                      /* UPON SUCCESSFUL COMPLETETION.                       */
}

/*************** RESET_MODES - RESETS THE PRINTER *****************/
/* Functionilities:  Clear Printer Buffer           - (030)
                     Cancel Compress Mode           - (ESC) + (022)
                     Super-sub Script Mode off      - (ESC) + (T)
                     Underline Mode off             - (ESC) + (-0)
                     Enable Bidirectional Printing  - (ESC) + (U0)
                     Emphasized Mode off            - (ESC) + (F)
                     Double Strike Mode off         - (ESC) + (H)
*/ 
reset_modes()
{
 fprintf(stdout,"%c",CLEAR_PRINT_BUF);     /* Clear Printer Buffer          */
 fprintf(stdout,"%c",ESC);                 /* Cancel Compress Mode          */
 fprintf(stdout,"%c",CANCEL_COMPRESS_MODE);
 fprintf(stdout,"%s",SCRIPTOFF);           /* Super-Sub Script Mode Off     */
 fprintf(stdout,"%s",UNDLINOFF);           /* UnderLine Mode Off            */
 fprintf(stdout,"%s",BIDIRECT_PRINT);      /* Enable Bidirectional Printing */
 fprintf(stdout,"%s",EMPHAOFF);            /* Emphasized Mode Off           */
 fprintf(stdout,"%s",DBLSTROFF);           /* Double Strike Mode Off        */
 fflush(stdout);
} /* end of reset_modes */

/* sighup: CATCH A HANGUP - A LOSS OF CARRIER */
void sighup()
{
 signal(SIGHUP,SIG_IGN);
 fprintf(stderr,"ATT473 filter: The connection to the Printer Dropped; perhaps \
it has gone off-line.\n");
 exit (E_HANGUP);
} /* end of sighup */

/* sigint - CATCH AN INTERRUPT */
void sigint()
{
 signal(SIGINT,SIG_IGN);
 fprintf(stderr,"ATT473 filter: Received an interrupt from the printer. The \
reason is unknown,\n a common cause is that the baud rate is too high.\n");
 exit (E_INTERRUPT);
}

/* sigpipe - CATCH EARLY CLOSE OF PIPE */
void sigpipe()
{
 signal(SIGPIPE,SIG_IGN);
 fprintf(stderr,"ATT473 filter: The Output Port was closed before all output \
could be written.\n");
exit (E_INTERRUPT);
}

/* SIGTERM - CATCH A TERMINATION SIGNAL */
void sigterm()
{
 signal(SIGTERM,SIG_IGN);
 fprintf(stderr,"ATT473 filter: Caught software termination signal.\n");
 exit(E_SUCCESS);
}
