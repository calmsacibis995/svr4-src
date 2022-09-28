/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/glob.h	1.2"
/* This file contains global constants. These constants are used to dynamically
allocate chunks of space for arrays that are defined in the global "command"
structure in the file env.h.  One chunk of space is allocated, and if this
fills, more space is allocated as needed.  The arrays contain the names
of items input on the command line.					*/

# define COV_SZ		50	/* coverage file names  -m option */
# define FNC_SZ		1000	/* function names   -f option */
# define SOURC_SZ	100     /* source file names  -r option */
# define INC_SZ		10	/* include directories  -I option */


