/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/ex_argv.h	1.8"
/*
 * The current implementation of the argument list is poor,
 * using an argv even for internally done "next" commands.
 * It is not hard to see that this is restrictive and a waste of
 * space.  The statically allocated glob structure could be replaced
 * by a dynamically allocated argument area space.
 */
var unsigned char	**argv;
var unsigned char	**argv0;
var unsigned char	*args;
var unsigned char	*args0;
var short	argc;
var short	argc0;
var short	morargc;		/* Used with "More files to edit..." */

var int	firstln;		/* From +lineno */
var unsigned char	*firstpat;		/* From +/pat	*/

struct	glob {
	short	argc;			/* Index of current file in argv */
	short	argc0;			/* Number of arguments in argv */
	unsigned char	*argv[NARGS + 1];	/* WHAT A WASTE! */
	unsigned char	argspac[NCARGS + sizeof (int)];
};
var struct	glob frob;
