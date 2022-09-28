/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:bblk/i386/mac.c	1.3"

/* This file contains the 386 dependent source for basicblk */

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>	/* standard I/O header file */
#include "comdef.h"	/* common header file of defines */
#include "comext.h"	/* common header file of global data declarations */

#include "macdef.h"	/* machine dependent header file of defines */

#define	fprerr() \
	{\
		(void)fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);\
		exit(FATAL);\
	}

/*
*	Check to see if given opcode is branch or jump.
*/
int
CAinstr(text)
char *text;
{
	extern void exit();		/* standard exit function */
	extern int tblsize;		/* size of instruction table */

	register unsigned int i;	/* loop counter */

	int result;

	/* sanity check on input parameter pointer */
	if (text == NULL) {
		(void)fprintf(stderr,"ERROR - CAinstr(): %s\n", err_msg[SR107]);
		exit(FATAL);
	}

	/*
	*   Switches on the first character of the input string.
	*   If a match, then check the input against the command table.
	*/
	switch(*text) {
		case 'c':	/* falls through */
		case 'j':
		case 'l':
		case 'r':

				/* loops through the opcode command table */
			for (i = 0; i < tblsize; i++) {

				/*
				*  Checks the opcode against instructions
				*  in the table
				*/
				result = strcmp(text,cmd_tbl[i]);
				if (result == SUCCESS) {
					return(PASS);
				}
				if (result < 0) {
				/*
				*  table is in alphabetical order, and we
				*  have gone too far
				*/
				    return(FAIL);
				}
			}
			break;
		default:
			break;
	}
	return(FAIL);
}

/* * * * * *
 * This function will generate the memory allocation information
 * needed to define the coverage structure.
 * It assumes that it is given the number of bblks in bct, and
 * the array of start line numbers, in stlnary (has bct entries).
 * 
 * the format of the coverage structure, with our annotations, is:
 * 
 * 	.align 4
 * .Cnnn:
 * 	.uaword .Cnnn.len	# note length of structure.
 * 	.uaword mmm		# note number of bblks.
 * 	.zero jjj		# reserve zeroes for counters.
 * 	.uaword KKK1			# note start line for first bblk.
 * 	.uaword KKK2			# ditto, second bblk.
 * 	...
 * 	.uaword KKKmm			# ditto, last bblk.
 * 	.Cnnn.len=.-.Cnnn	# define length, for reference above.
 * 
 * where
 * nnn		the number of functions, and this function's seq. number.
 * mmm		the number of basic blocks identified by basicblk.
 * jjj		the number of zeroed bytes to reserve, for execution counts
 * 		(jjj = mmm * sizeof(long)).
 * KKK1,2,..mmm	the starting line number for each basicblock.
 * .Cnnn.len	an identifier assigned a value which represents the length
 * 		of this coverage structure.
 */
void
CAsize(bct,stlnary)
unsigned int bct;
unsigned long int *stlnary;
{
	extern void exit();		/* standard exit function */

	unsigned int space;		/* number bytes */
	int i;				/* index var */

	/* sanity check on the input parameter */
	if (bct == ZERO) {
		fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR105]);
		exit(FATAL);
	}

	/* prints alignment */
	if (fprintf(stdout,_ALIGN) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}

	/* prints memory allocation label */
	if (fprintf(stdout,_LABEL,fcnt) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}

	/* prints 'num of bblks' cell datum */
	if (fprintf(stdout,_AWORD,bct) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}


	space = bct * sizeof(long);
	/* allocates memory */
	if (fprintf(stdout,_ZERO,space) <= PNT_FAIL) {
	    fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);
	    exit(FATAL);
	}

		/* prints 'start line number' for each bblk */
	for(i=0; i<bct; i++) {
		if (fprintf(stdout,_AWORD,stlnary[i]) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - CAsize(): %s\n",err_msg[SR109]);
			exit(FATAL);
		}
	}

		/* generates 'size of structure' datum definition */
	if (fprintf(stdout,_LENDEF,fcnt,fcnt) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - CAsize(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}
}
