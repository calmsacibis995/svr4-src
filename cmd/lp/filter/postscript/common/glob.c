/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/common/glob.c	1.1.2.1"

/*
 *
 * Definition and initialization of some global variables.
 *
 */


#include <stdio.h>
#include "gen.h"			/* general purpose definitions */


char	**argv;				/* global so everyone can use them */
int	argc;

int	x_stat = 0;			/* program exit status */
int	debug = OFF;			/* debug flag */
int	ignore = OFF;			/* what we do with FATAL errors */

long	lineno = 0;			/* line number */
long	position = 0;			/* byte position */
char	*prog_name = "";		/* and program name - for errors */
char	*temp_file = NULL;		/* temporary file - for some programs */

