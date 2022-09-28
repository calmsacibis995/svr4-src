/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:bblk/comglb.c	1.4"

	/* counters used in the parsing of the input file */
unsigned int fcnt = 0;			/* function counter */
unsigned int bkcnt = 0;			/* logical block counter */

	/* error message table of all message strings */
char *err_msg[] = {
	"too many command line arguments",
	"assuming stdin and stdout",
	"assuming stdout",
	"missing a function ending",
	"missing a function beginning",
	"zero byte size for coverage array",
	"can't open",
	"parameter is a null pointer",
	"bad switch on size of coverage array",
	"error on fprintf",
	"missing -x or -l option",
	"usage:  basicblk [-Q] [-x|l] [in] [out]"
};
