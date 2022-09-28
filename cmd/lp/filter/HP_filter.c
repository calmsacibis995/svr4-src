/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/HP_filter.c	1.1.2.1"

/*****************************************************************************/
/* HP_filter: Works on the AT&T 495 printer. This filter can print file with */
/*            escape sequences in the HP mode, Qume mode or Ibm-graphics     */
/*            mode. Right now all user options are processed if the printer  */
/*            type is: 495hp. In 495hp term type - we can still change the   */
/*            printer to a Qume or Ibm-graphics type and print files. But no */
/*            user options of the Qume or Ibm-graphics type are right now    */
/*            supported.                                                     */
/*****************************************************************************/

#include <stdio.h>
#include "signal.h"
#include <string.h>

#define E_SUCCESS	0
#define E_WRITE_FAILED	1
#define E_HANGUP	2
#define E_INTERRUPT	3
#define E_MALLOC_FAILED	4

#define BUFSIZE		1024

/* DEFINE PRINTER CONTROL SEQUENCES */
#define IBMGRAP		 "\033[12~"
#define HPLASER		 "\033[10~"
#define QUME		"\033[11~"
#define ROMAN_EIGHT	"\033(8U"
#define	LANDSCAPE	"\033&l1O"
#define NO_LANDSCAPE	"\033&l0O"
#define	LP_FONT	"\033(s0T\033)s3T\033(s16.6H\033)s16.6H\033(s8.5V\033)s8.5V"
#define COURIER_FONT	"\033(s3T\033)s0T"
#define BOLD_WEIGHT	"\033(s1B\033)s0B"
#define LIGHT_WEIGHT	"\033(s-1B\033)s0B"
#define MEDIUM_WEIGHT	"\033(s0B\033)s1B"
#define ITALIC_STYLE	"\033(s1S\033)s0S"
#define UPRIGHT_STYLE	"\033(s0S\033)s1S"
#define AT_END		"\033&166p2e7.6c66F"

char *termtype = 0;     /* printer terminfo type */

int portrait_mode = 0,  /* user opted portait mode - default NO       */
    landscape_mode = 0, /* user opted landscape mode - default NO     */
    courier_font = 0,   /* user opted courier font - default NO       */
    lp_font = 0,        /* user opted lp font - default NO            */
    bold_weight = 0,    /* user opted bold weight - default NO        */
    light_weight = 0,   /* user opted light weight - default NO       */
    medium_weight = 0,  /* user opted medium weight - default NO      */
    upright_style = 0,  /* user opted upright style - default NO      */
    italic_style = 0,   /* user opted italic style - default NO       */
    do_reset = 0,       /* user opted printer to be reset             */
    term = 0;           /* user opted to specify printer command set: */
                        /* 495hp, Qume, or Ibm-Graphics?              */


void    sighup(),
        sigint(),
        sigpipe(),
        sigquit(),
        sigterm();

main(argc,argv)
int argc;
char *argv[];
{
    extern char *optarg;
    extern int optind;
    extern int errno, opterr;
    int c;

    int nin, nout;
    char buffer[BUFSIZE];

    char *malloc();


/* USER OPTIONS ACCEPTED BY THE HPLASERJET FILTER:                       */
/* ----------------------------------------------                        */
/* p = Portrait Output.                                                  */
/* s = Landscape Output.                                                 */
/* c = Courier Font.                                                     */
/* l = LP Font.                                                          */
/* b = BOLD Weight.                                                      */
/* g = LIGHT Weight.                                                     */
/* m = MEDIUM Weight.                                                    */
/* u = UPRIGHT Style.                                                    */
/* i = ITALIC Style.                                                     */
/* H = Change to Command set HP LASER JET                                */
/* Q = Change to Command set Qume                                        */
/* I = Change to Command set Ibm-Graphics                                */
/* t = Terminfo entry printer set to. Always passed by the Scheduler     */
/* r = Reset the PRINTER. By default all archived Filters will NOT reset */
/*     the Printer. This is so because the output from a FILTER might be */
/*     PIPED to yet another FILTER - in such a case we do not want the   */
/*     RESET PRINTER ESCAPE CODES to be Input to some other Filter.      */


    opterr = 0;      /* getopt will return error messages on the Standard  */
                     /* Error. This will lead to a PRINTER FAULT with the  */
                     /* new LP Spooler. Setting opterr to zero disables it */

    while ((c = getopt(argc,argv,"psclbgmuirHQIt:")) != EOF) {
	switch (c) {
              case 'p' : portrait_mode =  1;   /* set portrait mode */
                         break;
              case 's' : landscape_mode = 1;   /* set landscape mode */
                         break;
              case 'c' : courier_font = 1;     /* set courier font */
                         break;
              case 'l' : lp_font = 1;          /* set lp font */
                         break;
              case 'b' : bold_weight = 1;      /* set bold font weight */
                         break;
              case 'g' : light_weight = 1;     /* set light font weight */
                         break;
              case 'm' : medium_weight = 1;     /* set medium font weight */
                         break;
              case 'u' : upright_style = 1;     /* set upright font style */
                         break;
              case 'i' : italic_style = 1;      /* set italic font style */
                         break;
              case 'r' : do_reset = 1;          /* reset the printer */
                         break;
              case 'H' : term = 1;              /* HP Laser Jet command set */
			break;
              case 'Q' : term = 2;              /* Qume or Sprint command set */
			break;
              case 'I' : term = 3;              /* Ibm-Graphics  command set */
			break;
              case 't' : if ((termtype = malloc(strlen(optarg) + 1)) == NULL) {
fprintf(stderr,"HP filter: Failure to allocate memory for Terminal Type.\n");
                         exit(E_MALLOC_FAILED);
                        }
                        else strcpy(termtype,optarg);  /* printer terminfo */
                        break;
	} /* of switch c ... */
    } /* of while getopt ... */


if (do_reset) reset_modes(); /* RESET THE PHYSICAL PRINTER iff the user opted */


/* NOW PROCESS ALL USER OPTIONS  -- IFF it is a 495p terminfo type and the */
/* user did not specify Qume or Ibm-graphics mode to be printed            */

if ((termtype) && (strcmp(termtype,"495hp") == 0) && 
    (term != 2) && (term != 3)) {
/* Landscape or Portrait ? */
 if (landscape_mode) fprintf(stdout,"%s",LANDSCAPE) ;
 else if (portrait_mode) fprintf(stdout,"%s",NO_LANDSCAPE) ;

/* Lp font or Courier font ? */
 if (lp_font) fprintf(stdout,"%s",LP_FONT) ;
 else if (courier_font) fprintf(stdout,"%s",COURIER_FONT) ;

/* Bold, Light or Medium font weight ? */
 if (bold_weight)        fprintf(stdout,"%s",BOLD_WEIGHT) ;
 else if (light_weight)  fprintf(stdout,"%s",LIGHT_WEIGHT) ;
 else if (medium_weight) fprintf(stdout,"%s",MEDIUM_WEIGHT) ;

/* Italic or Upright font style - iff Courier font is opted for by the user */
 if (courier_font) {
   if (italic_style)       fprintf(stdout,"%s",ITALIC_STYLE) ;
   else if (upright_style) fprintf(stdout,"%s",UPRIGHT_STYLE);
 }
 fflush(stdout);           /* first flush out printer sequences if any */
} /* end of if termtype is set and is of 495hp terminfo type */

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

   while ((nin = read(0,buffer,BUFSIZE)) > 0) {
        if ((nout = write(1,buffer,nin)) != nin) {
          if (nout < 0)
            fprintf(stderr,"HP Filter: Write Failed; perhaps the \
PRINTER has gone off-line.\n") ;
          else fprintf(stderr,"HP Filter: Incomplete Write; perhaps \
the PRINTER has gone off-line.\n") ;
          exit(E_WRITE_FAILED);
         } 
   } 

 if (do_reset) end_reset_modes();     /* AT END RESET BACK PRINTER iff the    */
                                      /* USER DID NOT SPECIFY ITS SUPPRESSION */

exit(E_SUCCESS);      /* ALL FILTERS MUST SPECIFICALLY EXIT WITH A CODE OF 0 */
                      /* UPON SUCCESSFUL COMPLETETION.                       */
}

/*************** RESET_MODES - RESETS THE PRINTER *****************/
reset_modes()
{
if (term) {
 if (term == 1) fprintf(stdout,"%s",HPLASER) ;
else if (term == 2) fprintf(stdout,"%s",QUME) ;
else if (term = 3) fprintf(stdout,"%s",IBMGRAP) ;
fflush(stdout);
}
if ((termtype) && (strcmp(termtype,"495hp") == 0) && 
     (term != 2) && (term != 3)) {
fprintf(stdout,"%s",ROMAN_EIGHT);
fprintf(stdout,"%s%s%s%s",NO_LANDSCAPE,COURIER_FONT,MEDIUM_WEIGHT,UPRIGHT_STYLE);
fflush(stdout);         /* MUST FLUSH OUT OUTPUT BUFFER - ELSE ESCAPE */
                        /* SEQUENCES WILL BE FLUSHED AFTER INPUT HAS  */
                        /* BEEN READ AND WRITTEN.                     */
} /* end of if termtype... */
} /* end of reset_modes */

/* end_reset_modes - AT END OF PRINTING RESET BACK THE PRINTER TO DEFAULT */
/*                   CONFIGURATION - IFF this is of 495hp term type       */
/*                   DEFAULT: No Landscape.                               */
/*                            Courier Font.                               */
/*                            Medium Weight.                              */
/*                            Upright Style.                              */
end_reset_modes()
{
if ((termtype) && (strcmp(termtype,"495hp") == 0) && 
    (term != 2) && (term != 3)) {
fprintf(stdout,"%s%s%s%s",NO_LANDSCAPE,COURIER_FONT,MEDIUM_WEIGHT,UPRIGHT_STYLE);
fprintf(stdout,"%s",AT_END);
} /* end of if termtype... */
} /* end of end_reset_modes */

/* sighup: CATCH A HANGUP - A LOSS OF CARRIER */
void sighup()
{
 signal(SIGHUP,SIG_IGN);
 fprintf(stderr,"HP filter: The connection to the Printer Dropped; perhaps \
it has gone off-line.\n");
 exit (E_HANGUP);
} /* end of sighup */

/* sigint - CATCH AN INTERRUPT */
void sigint()
{
 signal(SIGINT,SIG_IGN);
 fprintf(stderr,"HP filter: Received an interrupt from the printer. The \
reason is unknown,\n a common cause is that the baud rate is too high.\n");
 exit (E_INTERRUPT);
}

/* sigpipe - CATCH EARLY CLOSE OF PIPE */
void sigpipe()
{
 signal(SIGPIPE,SIG_IGN);
 fprintf(stderr,"HP filter: The Output Port was closed before all output \
could be written.\n");
exit (E_INTERRUPT);
}

/* SIGTERM - CATCH A TERMINATION SIGNAL */
void sigterm()
{
 signal(SIGTERM,SIG_IGN);
 fprintf(stderr,"HP filter: Caught software termination signal.\n");
 exit(E_SUCCESS);
}
