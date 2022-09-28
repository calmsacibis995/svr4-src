/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/template/template.c	1.1.2.1"

/*
 *
 * template for PostScript translators
 *
 *
 * A file that should give you a decent start if you're writing a new PostScript
 * translator. The basic outline is here, but there's still plenty to do.
 *
 * The PostScript prologue is copied from *prologue before any of the input files
 * are translated. The program expects that the following PostScript procedures
 * are defined in that file:
 *
 *
 *	setup
 *
 *	  mark ... setup -
 *
 *	    Handles special initialization stuff that depends on how this program
 *	    was called. Expects to find a mark followed by key/value pairs on the
 *	    stack. The def operator is applied to each pair up to the mark, then
 *	    the default state is set up.
 *
 *	done
 *
 *	  done
 *
 *	    Makes sure the last page is printed. Only needed when we're printing
 *	    more than one page on each sheet of paper.
 *
 *
 */


#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>

#include "comments.h"			/* PostScript file structuring comments */
#include "gen.h"			/* general purpose definitions */
#include "path.h"			/* for the prologue */
#include "ext.h"			/* external variable declarations */
#include "template.h"			/* a few special definitions */


char	*optnames = "a:c:m:n:o:p:x:y:A:C:J:L:P:R:DI";

char	*prologue = TEMPLATE;		/* default PostScript prologue */
char	*formfile = FORMFILE;		/* stuff for multiple pages per sheet */

int	formsperpage = 1;		/* page images on each piece of paper */
int	copies = 1;			/* and this many copies of each sheet */

int	page = 0;			/* page we're working on */
int	printed = 0;			/* printed this many pages */

FILE	*fp_in = stdin;			/* read from this file */
FILE	*fp_out = stdout;		/* and write stuff here */
FILE	*fp_acct = NULL;		/* for accounting data */


/*****************************************************************************/


main(agc, agv)


    int		agc;
    char	*agv[];


{


/*
 *
 * Template that may help if you're writing a new PostScript translator.
 *
 */


    argc = agc;				/* other routines may want them */
    argv = agv;

    prog_name = argv[0];		/* really just for error messages */

    init_signals();			/* sets up interrupt handling */
    header();				/* PostScript header and prologue */
    options();				/* handle the command line options */
    setup();				/* for PostScript */
    arguments();			/* followed by each input file */
    done();				/* print the last page etc. */
    account();				/* job accounting data */

    exit(x_stat);			/* not much could be wrong */

}   /* End of main */


/*****************************************************************************/


init_signals()


{


    int		interrupt();		/* signal handler */


/*
 *
 * Makes sure we handle interrupts.
 *
 */


    if ( signal(SIGINT, interrupt) == SIG_IGN )  {
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
    } else {
	signal(SIGHUP, interrupt);
	signal(SIGQUIT, interrupt);
    }   /* End else */

    signal(SIGTERM, interrupt);

}   /* End of init_signals */


/*****************************************************************************/


header()


{


    int		ch;			/* return value from getopt() */
    int		old_optind = optind;	/* for restoring optind - should be 1 */


/*
 *
 * Scans the option list looking for things, like the prologue file, that we need
 * right away but could be changed from the default. Doing things this way is an
 * attempt to conform to Adobe's latest file structuring conventions. In particular
 * they now say there should be nothing executed in the prologue, and they have
 * added two new comments that delimit global initialization calls. Once we know
 * where things really are we write out the job header, follow it by the prologue,
 * and then add the ENDPROLOG and BEGINSETUP comments.
 *
 */


    while ( (ch = getopt(argc, argv, optnames)) != EOF )
	if ( ch == 'L' )
	    prologue = optarg;
	else if ( ch == '?' )
	    error(FATAL, "");

    optind = old_optind;		/* get ready for option scanning */

    fprintf(stdout, "%s", CONFORMING);
    fprintf(stdout, "%s %s\n", VERSION, PROGRAMVERSION);
    fprintf(stdout, "%s %s\n", DOCUMENTFONTS, ATEND);
    fprintf(stdout, "%s %s\n", PAGES, ATEND);
    fprintf(stdout, "%s", ENDCOMMENTS);

    if ( cat(prologue) == FALSE )
	error(FATAL, "can't read %s", prologue);

    fprintf(stdout, "%s", ENDPROLOG);
    fprintf(stdout, "%s", BEGINSETUP);
    fprintf(stdout, "mark\n");

}   /* End of header */


/*****************************************************************************/


options()


{


    int		ch;			/* return value from getopt() */


/*
 *
 * Reads and processes the command line options. Added the -P option so arbitrary
 * PostScript code can be passed through. Expect it could be useful for changing
 * definitions in the prologue for which options have not been defined.
 *
 */


    while ( (ch = getopt(argc, argv, optnames)) != EOF )  {

	switch ( ch )  {

	    case 'a':			/* aspect ratio */
		    fprintf(stdout, "/aspectratio %s def\n", optarg);
		    break;

	    case 'c':			/* copies */
		    copies = atoi(optarg);
		    fprintf(stdout, "/#copies %s store\n", optarg);
		    break;

	    case 'm':			/* magnification */
		    fprintf(stdout, "/magnification %s def\n", optarg);
		    break;

	    case 'n':			/* forms per page */
		    formsperpage = atoi(optarg);
		    fprintf(stdout, "%s %s\n", FORMSPERPAGE, optarg);
		    fprintf(stdout, "/formsperpage %s def\n", optarg);
		    break;

	    case 'o':			/* output page list */
		    out_list(optarg);
		    break;

	    case 'p':			/* landscape or portrait mode */
		    if ( *optarg == 'l' )
			fprintf(stdout, "/landscape true def\n");
		    else fprintf(stdout, "/landscape false def\n");
		    break;

	    case 'x':			/* shift things horizontally */
		    fprintf(stdout, "/xoffset %s def\n", optarg);
		    break;

	    case 'y':			/* and vertically on the page */
		    fprintf(stdout, "/yoffset %s def\n", optarg);
		    break;

	    case 'A':			/* force job accounting */
	    case 'J':
		    if ( (fp_acct = fopen(optarg, "a")) == NULL )
			error(FATAL, "can't open accounting file %s", optarg);
		    break;

	    case 'C':			/* copy file straight to output */
		    if ( cat(optarg) == FALSE )
			error(FATAL, "can't read %s", optarg);
		    break;

	    case 'L':			/* PostScript prologue file */
		    prologue = optarg;
		    break;

	    case 'P':			/* PostScript pass through */
		    fprintf(stdout, "%s\n", optarg);
		    break;

	    case 'R':			/* special global or page level request */
		    saverequest(optarg);
		    break;

	    case 'D':			/* debug flag */
		    debug = ON;
		    break;

	    case 'I':			/* ignore FATAL errors */
		    ignore = ON;
		    break;

	    case '?':			/* don't understand the option */
		    error(FATAL, "");
		    break;

	    default:			/* don't know what to do for ch */
		    error(FATAL, "missing case for option %c\n", ch);
		    break;

	}   /* End switch */

    }   /* End while */

    argc -= optind;			/* get ready for non-option args */
    argv += optind;

}   /* End of options */


/*****************************************************************************/


setup()


{


/*
 *
 * Handles things that must be done after the options are read but before the
 * input files are processed.
 *
 */


    writerequest(0, stdout);		/* global requests eg. manual feed */
    fprintf(stdout, "setup\n");

    if ( formsperpage > 1 )  {
	if ( cat(formfile) == FALSE )
	    error(FATAL, "can't read %s", formfile);
	fprintf(stdout, "%d setupforms\n", formsperpage);
    }	/* End if */

    fprintf(stdout, "%s", ENDSETUP);

}   /* End of setup */


/*****************************************************************************/


arguments()


{


/*
 *
 * Makes sure all the non-option command line arguments are processed. If we get
 * here and there aren't any arguments left, or if '-' is one of the input files
 * we'll translate stdin.
 *
 */


    if ( argc < 1 )
	conv();
    else {				/* at least one argument is left */
	while ( argc > 0 )  {
	    if ( strcmp(*argv, "-") == 0 )
		fp_in = stdin;
	    else if ( (fp_in = fopen(*argv, "r")) == NULL )
		error(FATAL, "can't open %s", *argv);
	    conv();
	    if ( fp_in != stdin )
		fclose(fp_in);
	    argc--;
	    argv++;
	}   /* End while */
    }   /* End else */

}   /* End of arguments */


/*****************************************************************************/


done()


{


/*
 *
 * Finished with all the input files, so mark the end of the pages with a TRAILER
 * comment, make sure the last page prints, and add things like the PAGES comment
 * that can only be determined after all the input files have been read.
 *
 */


    fprintf(stdout, "%s", TRAILER);
    fprintf(stdout, "done\n");
    fprintf(stdout, "%s %d\n", PAGES, printed);

}   /* End of done */


/*****************************************************************************/


account()


{


/*
 *
 * Writes an accounting record to *fp_acct provided it's not NULL. Accounting is
 * requested using the -A or -J options.
 *
 */


    if ( fp_acct != NULL )
	fprintf(fp_acct, " print %d\n copies %d\n", printed, copies);

}   /* End of account */


/*****************************************************************************/


conv()


{


    int		ch;			/* next input character */


/*
 *
 * Controls the translation of the next input file into PostScript. This is where
 * you'll have to start with the real work. *fp_in is already set to the next input
 * file.
 *
 */


    redirect(-1);			/* get ready for the first page */
    formfeed();				/* force the initial PAGE comment etc. */

}   /* End of text */


/*****************************************************************************/


formfeed()


{


/*
 *
 * Called whenever we've finished with the last page and want to get ready for the
 * next one. Also used at the beginning and end of each input file, so we have to
 * be careful about what's done. The first time through (up to the redirect() call)
 * output goes to /dev/null.
 *
 * Adobe now recommends that the showpage operator occur after the page level
 * restore so it can be easily redefined to have side-effects in the printer's VM.
 * Although it seems reasonable I haven't implemented it, because it makes other
 * things, like selectively setting manual feed or choosing an alternate paper
 * tray, clumsy - at least on a per page basis. 
 *
 */


    if ( fp_out == stdout )		/* count the last page */
	printed++;

    fprintf(fp_out, "cleartomark\n");
    fprintf(fp_out, "showpage\n");
    fprintf(fp_out, "restore\n");
    fprintf(fp_out, "%s %d %d\n", ENDPAGE, page, printed);

    if ( ungetc(getc(fp_in), fp_in) == EOF )
	redirect(-1);
    else redirect(++page);

    fprintf(fp_out, "%s %d %d\n", PAGE, page, printed+1);
    fprintf(fp_out, "save\n");
    fprintf(fp_out, "mark\n");
    writerequest(printed+1, fp_out);
    fprintf(fp_out, "%d pagesetup\n", printed+1);

}   /* End of formfeed */


/*****************************************************************************/


redirect(pg)


    int		pg;			/* next page we're printing */


{


    static FILE	*fp_null = NULL;	/* if output is turned off */


/*
 *
 * If we're not supposed to print page pg, fp_out will be directed to /dev/null,
 * otherwise output goes to stdout.
 *
 */


    if ( pg >= 0 && in_olist(pg) == ON )
	fp_out = stdout;
    else if ( (fp_out = fp_null) == NULL )
	fp_out = fp_null = fopen("/dev/null", "w");

}   /* End of redirect */


/*****************************************************************************/

