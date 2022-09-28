/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/common/misc.c	1.1.2.1"

/*
 *
 * A few general purpose routines.
 *
 */


#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#include "gen.h"			/* a few general purpose definitions */
#include "ext.h"			/* external variable declarations */


int	nolist = 0;			/* number of specified ranges */
int	olist[50];			/* processing range pairs */


/*****************************************************************************/


out_list(str)


    char	*str;			/* process ranges in this string */


{


    int		start, stop;		/* end points */


/*
 *
 * Called to get the processing ranges that were specified by using the -o option.
 * The range syntax should be identical to the one used in nroff and troff.
 *
 */


    while ( *str && nolist < sizeof(olist) - 2 )  {
	start = stop = str_convert(&str, 0);

	if ( *str == '-' && *str++ )
	    stop = str_convert(&str, 9999);

	if ( start > stop )
	    error(FATAL, "illegal range %d-%d", start, stop);

	olist[nolist++] = start;
	olist[nolist++] = stop;

	if ( *str != '\0' ) str++;

    }	/* End while */

    olist[nolist] = 0;

}   /* End of out_list */


/*****************************************************************************/


in_olist(num)


    int		num;			/* should we print this page? */


{


    int		i;			/* just a loop index */


/*
 *
 * Returns ON if num represents a page that we're supposed to print. If no ranges
 * were selected nolist will be 0 and we'll print everything.
 *
 */


    if ( nolist == 0 )			/* everything's included */
	return(ON);

    for ( i = 0; i < nolist; i += 2 )
	if ( num >= olist[i] && num <= olist[i+1] )
	    return(ON);

    return(OFF);

}   /* End of in_olist */


/*****************************************************************************/


cat(file)


    char	*file;			/* copy this file to stdout */


{


    int		fd_in;			/* for the input */
    int		fd_out;			/* and output files */
    char	buf[512];		/* buffer for reads and writes */
    int		count;			/* number of bytes we just read */


/*
 *
 * Copies *file to stdout - mostly for the prologue. Returns FALSE if there was a
 * problem and TRUE otherwise.
 *
 */


    fflush(stdout);

    if ( (fd_in = open(file, O_RDONLY)) == -1 )
	return(FALSE);

    fd_out = fileno(stdout);
    while ( (count = read(fd_in, buf, sizeof(buf))) > 0 )
	write(fd_out, buf, count);

    close(fd_in);

    return(TRUE);

}   /* End of cat */


/*****************************************************************************/


str_convert(str, err)


    char	**str;			/* get next number from this string */
    int		err;			/* value returned on error */


{


    int		i;			/* just a loop index */


/*
 *
 * Gets the next integer from **str and returns its value to the caller. If **str
 * isn't an integer err is returned. *str is updated after each digit is processed.
 *
 */


    if ( ! isdigit(**str) )		/* something's wrong */
	return(err);

    for ( i = 0; isdigit(**str); *str += 1 )
	i = 10 * i + **str - '0';

    return(i);

}   /* End of str_convert */


/*****************************************************************************/


error(kind, mesg, a1, a2, a3)


    int		kind;			/* FATAL or NON_FATAL error */
    char	*mesg;			/* error message control string */
    unsigned	a1, a2, a3;		/* control string arguments */


{


/*
 *
 * Called when we've run into some kind of program error. *mesg is printed using
 * the control string arguments a?. We'll quit if we're not ignoring errors and
 * kind is FATAL.
 *
 */


    if ( mesg != NULL && *mesg != '\0' )  {
	fprintf(stderr, "%s: ", prog_name);
	fprintf(stderr, mesg, a1, a2, a3);
	if ( lineno > 0 )
	    fprintf(stderr, " (line %d)", lineno);
	if ( position > 0 )
	    fprintf(stderr, " (near byte %d)", position);
	putc('\n', stderr);
    }	/* End if */

    if ( kind == FATAL && ignore == OFF )  {
	if ( temp_file != NULL )
	    unlink(temp_file);
	exit(x_stat | 01);
    }	/* End if */

}   /* End of error */


/*****************************************************************************/


void interrupt(sig)


    int		sig;			/* signal that we caught */


{


/*
 *
 * Called when we get a signal that we're supposed to catch.
 *
 */


    if ( temp_file != NULL )
	unlink(temp_file);

    exit(1);

}   /* End of interrupt */


/*****************************************************************************/


