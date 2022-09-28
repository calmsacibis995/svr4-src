/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:bblk/comext.h	1.3"
/*
* FILE: CAcomext.h
*
* DESCRIPTION: The common global variables used in the different versions
*              of the coverage analyzer.
*/

extern short qflag;			/* On for version stamping */

	/* counters used in the parsing of the input file */
extern unsigned int fcnt;		/* function counter */
extern unsigned int bkcnt;		/* logical block counter */

	/* tables of character strings */
extern char *err_msg[];			/* error message table */
extern char *cmd_tbl[];			/* opcode command table */
