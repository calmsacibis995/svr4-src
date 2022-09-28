/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/475_filter.c	1.1.2.1"

#include <stdio.h>
#include "signal.h"
#include <string.h>

#define		WIDTH		80
#define		WIDMIN		1
#define		WIDMAX		218

#define		BS		'\010'		/* Back Space		*/
#define		TAB		'\011'		/* Tab			*/
#define		CR		'\015'		/* Carriage Return	*/
#define		LF		'\012'		/* Line Feed		*/
#define		FF		'\014'		/* Form Feed		*/
#define		ESC		'\033'		/* Escape		*/
#define		UNDLINOFF	"\033Y" 	/* Underline off	*/
#define		UNDLINON	"\033X" 	/* Underline on		*/
#define		EMPHAON		"\033!"		/* Emphasize on		*/
#define		EMPHAOFF	"\033\042"	/* Emphasize off	*/
#include	<stdio.h>

char *esc[] =	{
	UNDLINOFF,
	UNDLINON,
	EMPHAON,
	EMPHAOFF,
	0
};






#define E_SUCCESS	0
#define E_WRITE_FAILED	1
#define E_HANGUP	2
#define E_INTERRUPT	3
#define E_MALLOC_FAILED	4

/* DEFINE PRINTER CONTROL SEQUENCES */ 
#define RESET		"\033\042\033<\033N\033]\033Y\033f"
#define BOLD		"\033!"
#define ELITE		"\033E"
#define COMPRESSED	"\033Q"

void    sighup(),
        sigint(),
        sigpipe(),
        sigquit(),
        sigterm();

int linwidth = 132,
    bold = 0,
    elite = 0,
    compressed = 0,
    pitch = 0,
    do_reset = 0,
    wide_platen = 0,
    max_line_size = 80;

char *term = 0;

main(argc,argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;
	extern int errno, opterr;
	extern char *malloc();
	int c;

	char buf[1024];
	int charcnt, bscnt, i, j, r, found;




/* USER OPTIONS ACCEPTED BY THE HPLASERJET FILTER:                       */
/* ----------------------------------------------                        */
/* b = Bold Print Mode.                                                  */
/* e = Elite Character Pitch - 12.                                       */
/* c = Compressed Character Pitch - 17.                                  */
/* p = Character Pitch User supplied value.                              */
/* t = Terminfo Type.                                                    */
/* w = LineSize.                                                         */
/* r = Reset the PRINTER. By default all archived Filters will NOT reset */
/*     the Printer. This is so because the output from a FILTER might be */
/*     PIPED to yet another FILTER - in such a case we do not want the   */
/*     RESET PRINTER ESCAPE CODES to be Input to some other Filter.      */

   opterr = 0;       /* getopt will return error messages on the Standard  */
                     /* Error. This will lead to a PRINTER FAULT with the  */
                     /* new LP Spooler. Setting opterr to zero disables it */

    while ((c = getopt(argc,argv,"becrp:t:w:")) != EOF) {
	switch (c) {
              case 'e' : elite = 1;
                         break;
              case 'c' : compressed = 1;
                         break;
              case 'p' : pitch = atoi(optarg);
                       if ( ! ((pitch == 10) || (pitch == 12) || (pitch == 17)))
                           fprintf(stdout,"475 filter: Invalid Character Pitch Value. Valid cpi values are: 10, 12, or 17\n");
                         if (pitch == 12) elite = 1 ;
                         else if (pitch == 17) compressed = 1 ;
                         break;
              case 'b' : bold = 1;
                         break;
              case 't' : if ((term = malloc(strlen(optarg) + 1)) == NULL) {
fprintf(stderr,"475 filter: Failure to allocate memory for Terminal Type.\n\n");
                             exit(E_MALLOC_FAILED);
                         }
                           else  strcpy(term,optarg);
                         break;
              case 'w' : linwidth = atoi(optarg) ;
		if (linwidth < WIDMIN || linwidth > WIDMAX)
		{
			fprintf(stdout,"Width should be between %d and\
 %d, defaulting to %d.\n",WIDMIN,WIDMAX,WIDTH);
			linwidth = WIDTH;
		}
                         break;
              case 'r' : do_reset = 1;
                         break;
	} /* of switch c ... */
    } /* of while getopt ... */


if (do_reset) reset_modes();          /* RESET THE PHYSICAL PRINTER iff the */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */


/* NOW PERFORM USER OPTIONS TO PRINTERS */

   if (bold) fprintf(stdout,"%s",BOLD);

   if ((term) && ( (strcmp(term,"471") == 0) || (strcmp(term,"att471") == 0) ||
                   (strcmp(term,"476") == 0) || (strcmp(term,"att476") == 0) ))
   {
       wide_platen = 1;  
       max_line_size = 132;
   }

/* In Case User Opted Both Elite and Compressed - then Elite Mode overrides */
/* the Compressed Mode.                                                     */
   if (elite) {
     if (wide_platen) max_line_size = 150 ;
     else             max_line_size = 96 ;
     fprintf(stdout,"%s",ELITE);
   }
   else if (compressed)  {
          if (wide_platen) max_line_size = 218 ;
          else             max_line_size = 150 ;
          fprintf(stdout,"%s",COMPRESSED);
   }

   if ( linwidth > max_line_size) linwidth = max_line_size ; 

   /* Must Flush Out Buffer if Printer was Not Reset but Some User Options */
   /* to Send Escape Sequences were used.                                  */
   if ( bold || elite || compressed ) fflush(stdout);

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

/* NOW PRINT THE INPUT FILE */

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


if (do_reset) reset_modes();          /* AT END RESET BACK PRINTER iff the    */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */

exit(E_SUCCESS);      /* ALL FILTERS MUST SPECIFICALLY EXIT WITH A CODE OF 0 */
                      /* UPON SUCCESSFUL COMPLETETION.                       */
}

/*************** RESET_MODES - RESETS THE PRINTER *****************/
reset_modes()
{
fprintf(stdout,"%s",RESET);
fflush(stdout);         /* MUST FLUSH OUT OUTPUT BUFFER - ELSE ESCAPE */
                        /* SEQUENCES WILL BE FLUSHED AFTER INPUT HAS  */
                        /* BEEN READ AND WRITTEN.                     */
} /* end of reset_modes */


/* sighup: CATCH A HANGUP - A LOSS OF CARRIER */
void sighup()
{
 signal(SIGHUP,SIG_IGN);
 fprintf(stderr,"475 filter: The connection to the Printer Dropped; perhaps \
it has gone off-line.\n");
 exit (E_HANGUP);
} /* end of sighup */

/* sigint - CATCH AN INTERRUPT */
void sigint()
{
 signal(SIGINT,SIG_IGN);
 fprintf(stderr,"475 filter: Received an interrupt from the printer. The \
reason is unknown,\n a common cause is that the baud rate is too high.\n");
 exit (E_INTERRUPT);
}

/* sigpipe - CATCH EARLY CLOSE OF PIPE */
void sigpipe()
{
 signal(SIGPIPE,SIG_IGN);
 fprintf(stderr,"475 filter: The Output Port was closed before all output \
could be written.\n");
exit (E_INTERRUPT);
}

/* SIGTERM - CATCH A TERMINATION SIGNAL */
void sigterm()
{
 signal(SIGTERM,SIG_IGN);
 fprintf(stderr,"475 filter: Caught software termination signal.\n");
 exit(E_SUCCESS);
}
