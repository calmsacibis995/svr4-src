/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:crash.h	1.8.4.1"

#include "setjmp.h"
#include "string.h"

/* This file should include only command independent declarations */

#define ARGLEN 40	/* max length of argument */

extern int debugmode;
extern FILE *fp;	/* output file */
extern int Procslot;	/* current process slot number */
extern int Virtmode;	/* current address translation mode */
extern int mem;		/* file descriptor for dumpfile */
extern jmp_buf syn;	/* syntax error label */
extern struct var vbuf;	/* tunable variables buffer */
extern char *args[];	/* argument array */
extern int argcnt;	/* number of arguments */
extern int optind;	/* argument index */
extern char *optarg;	/* getopt argument */
extern long getargs();	/* function to get arguments */
extern long strcon();	/* function to convert strings to long */
extern long eval();	/* function to evaluate expressions */
extern struct syment *symsrch();	/* function for symbol search */
