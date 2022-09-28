/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/455_filter.c	1.1.2.1"

#include <stdio.h>
#include "signal.h"
#include <string.h>

/*
* Printer filter for ATT455 derived from the ATT473 filter. 12/18/85
*/
#define		WIDTH		132	/* 10 cpi */
#define		WIDMIN		1
#define		WIDMAX		197	/* 15cpi */

#define		BS		'\010'		/* Back Space		*/
#define		TAB		'\011'		/* Tab			*/
#define		CR		'\015'		/* Carriage Return	*/
#define		LF		'\012'		/* Line Feed		*/
#define		FF		'\014'		/* Form Feed		*/
#define		ESC		'\033'		/* Escape		*/
#define		UNDLINOFF	"\033I" 	/* Underline off	*/
#define		UNDLINON	"\033J"		/* Underline on		*/
#define		EMPHAON		"\033Q"		/* Emphasize on		*/
#define		EMPHAOFF	"\033R"		/* Emphasize off	*/
#define		DBLSTRON	"\033K2"	/* Double Strike on	*/
#define		DBLSTROFF	"\033M"		/* Double Strike off	*/
#include	<stdio.h>

char *esc[] =	{
	UNDLINOFF,
	UNDLINON,
	EMPHAON,
	EMPHAOFF,
	DBLSTRON,
	DBLSTROFF,
	0
};



#define E_SUCCESS	0
#define E_WRITE_FAILED	1
#define E_HANGUP	2
#define E_INTERRUPT	3
#define E_MALLOC_FAILED	4

/* DEFINE PRINTER CONTROL SEQUENCES */ 
#define FOR_TABS	"\033(08,16,24,32,40,48,56,64,72,80,88,96,A4,B2,C0,C8"
#define EMPHASIZED		"\033K2"
#define DOUBLESTRIKE		"\033Q"
#define PROPORTIONAL		"\033$"
#define NO_PROPORTIONAL		"\033%"
#define LINEFEEDOFF		"\033."
#define LINEFEEDON		"\033,"
#define RESET_455		"\033x"
#define RESET			"\033M\033R\033T\033J\0334\0332"


void    sighup(),
        sigint(),
        sigpipe(),
        sigquit(),
        sigterm();

int linwidth = 132,
    linefeedoff = 0,
    linefeedon = 0,
    do_reset = 0,
    emphasized = 0,
    doublestrike = 0,
    proportional = 0,
    no_proportional = 0;

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
/* F = Automatic Line Feed On Carriage Return OFF.                       */
/* O = Automatic Line Feed On Carriage Return ON.                        */
/* e = Emphasized or Bold Mode ON.                                       */
/* d = Doublestrike of Shadow Print Mode ON.                             */
/* p = Proportional Spacings ON - only for 455 Printer Type.             */
/* n = Proportional Spacings OFF - only for 455 Printer Type.            */
/* t = Terminfo Type.                                                    */
/* w = LineSize.                                                         */
/* r = Reset the PRINTER. By default all archived Filters will NOT reset */
/*     the Printer. This is so because the output from a FILTER might be */
/*     PIPED to yet another FILTER - in such a case we do not want the   */
/*     RESET PRINTER ESCAPE CODES to be Input to some other Filter.      */

      opterr = 0;   /* getopt will return error messages on the Standard  */
                     /* Error. This will lead to a PRINTER FAULT with the  */
                     /* new LP Spooler. Setting opterr to zero disables it */

    while ((c = getopt(argc,argv,"FOredpnt:w:")) != EOF) {
	switch (c) {
              case 'F' : linefeedoff =  1;
                         break;
              case 'O' : linefeedon = 1;
                         break;
              case 'e' : emphasized = 1;
                         break;
              case 'd' : doublestrike = 1;
                         break;
              case 'p' : proportional = 1;
                         break;
              case 'n' : no_proportional = 1;
                         break;
              case 't' : if ((term = malloc(strlen(optarg) + 1)) == NULL) {
fprintf(stderr,"455filter: Failure to allocate memory for Terminal Type\n\n\n");
                             exit(E_MALLOC_FAILED);
                         }
                           else  strcpy(term,optarg);
                         break;
              case 'w' : linwidth = atoi(optarg) ;
		         if (linwidth < WIDMIN || linwidth > WIDMAX) {
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

   /* Cannot be both Emphasized and Doublestrike */
   /* Emphasized overrules Doublestrike          */
   if (emphasized) fprintf(stdout,"%s",EMPHASIZED) ;
   else  if (doublestrike) fprintf(stdout,"%s",DOUBLESTRIKE);
  
   /* Proportional Spacings can only be applied if Term is set to 455 type */
   if ( (term) && 
        ( (strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0) ) ) {
     if (proportional)  fprintf(stdout,"%s",PROPORTIONAL) ;
    else if ((do_reset) || (no_proportional))
           fprintf(stdout,"%s",NO_PROPORTIONAL) ;
   }

   /* Line feed on Carriage Return ON or OFF */
   if (linefeedoff)   fprintf(stdout,"%s",LINEFEEDOFF) ; 
   else if (linefeedon) fprintf(stdout,"%s",LINEFEEDON);

   /* Must Flush Out Buffer if Printer was Not Reset but Some User Options */
   /* to Send Escape Sequences were used.                                  */
   if ( emphasized || doublestrike || linefeedoff || linefeedon || 
        ( proportional || no_proportional  &&
          ( term && 
            ((strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0)) ) ) )
     fflush(stdout); 

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

if (do_reset) end_reset_modes();      /* AT END RESET BACK PRINTER iff the    */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */

exit(E_SUCCESS);      /* ALL FILTERS MUST SPECIFICALLY EXIT WITH A CODE OF 0 */
                      /* UPON SUCCESSFUL COMPLETETION.                       */
}

/*************** RESET_MODES - RESETS THE PRINTER *****************/
reset_modes()
{
/* Reset of Printer Can Only be applied to Printers of types:             */
/* 455, 457, or 458. Cautious approach as other Printers were not tested. */

if ( (term) && ( (strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0) ||  
     (strcmp(term,"457") == 0) || (strcmp(term,"att457") == 0) ||
     (strcmp(term,"458") == 0) || (strcmp(term,"att458") == 0) ) ) {
  if ( (strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0) )
    fprintf(stdout,"%s",RESET_455) ; 
  fprintf(stdout,"%s%s",RESET,FOR_TABS);
}
fflush(stdout);         /* MUST FLUSH OUT OUTPUT BUFFER - ELSE ESCAPE */
                        /* SEQUENCES WILL BE FLUSHED AFTER INPUT HAS  */
                        /* BEEN READ AND WRITTEN.                     */
} /* of reset_modes */

/* end_reset_modes - AT END OF PRINTING RESET BACK THE PRINTER TO DEFAULT */
/*                   CONFIGURATION.                                       */
end_reset_modes()
{
if ( (term) && ( (strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0) ||  
     (strcmp(term,"457") == 0) || (strcmp(term,"att457") == 0) ||
     (strcmp(term,"458") == 0) || (strcmp(term,"att458") == 0) ) ) {
  if ( (strcmp(term,"455") == 0) || (strcmp(term,"att455") == 0) )
    fprintf(stdout,"%s",RESET_455) ; 
  fprintf(stdout,"%s",RESET);
} /* of if */
} /* of end_reset_modes */

/* sighup: CATCH A HANGUP - A LOSS OF CARRIER */
void sighup()
{
 signal(SIGHUP,SIG_IGN);
 fprintf(stderr,"455 filter: The connection to the Printer Dropped; perhaps \
it has gone off-line.\n");
 exit (E_HANGUP);
} /* end of sighup */

/* sigint - CATCH AN INTERRUPT */
void sigint()
{
 signal(SIGINT,SIG_IGN);
 fprintf(stderr,"455 filter: Received an interrupt from the printer. The \
reason is unknown,\n a common cause is that the baud rate is too high.\n");
 exit (E_INTERRUPT);
}

/* sigpipe - CATCH EARLY CLOSE OF PIPE */
void sigpipe()
{
 signal(SIGPIPE,SIG_IGN);
 fprintf(stderr,"455 filter: The Output Port was closed before all output \
could be written.\n");
exit (E_INTERRUPT);
}

/* SIGTERM - CATCH A TERMINATION SIGNAL */
void sigterm()
{
 signal(SIGTERM,SIG_IGN);
 fprintf(stderr,"455 filter: Caught software termination signal.\n");
 exit(E_SUCCESS);
}
