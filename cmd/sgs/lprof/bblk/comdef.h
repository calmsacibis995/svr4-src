/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:bblk/comdef.h	1.3"
/*
* FILE: CAcomdef.h
*
* DESCRIPTION: This file contains the common #defines for the different versions
*              of the coverage analyzer.
*/

	/* values for lxprint */
#define XOPT		0		/* for lxprint option */
#define LOPT		1		/* for normal operation */

	/* values for function returns and test conditions */
#define TRUE		1		/* flag value */
#define FALSE		0		/* flag value */
#define PASS		1		/* function return code */
#define FAIL		0		/* function return code */
#define SUCCESS		0		/* match on string compare */
#define ZERO		0		/* test condition */

	/* values used in the calculations of the coverage array */
#define MASK		0x1		/* bit position mask */
#define BYTESIZE	8		/* byte size */

	/* values used in finding the last line number in a function */
#define LNUMS		6		/* start index for line number in lastnum[] */
#define YTNUMS		4		/* start index for line number in yytext[] */

	/* indexes into the error table of messages */
#define SR100		0		/* too many command line agruments */
#define SR101		1		/* assuming stdin and stdout */
#define SR102		2		/* assuming stdout */
#define SR103		3		/* missing a function ending */
#define SR104		4		/* missing a function beginning */
#define SR105		5		/* zero byte size for coverage array */
#define SR106		6		/* can't open */
#define SR107		7		/* parameter is a null pointer */
#define SR108		8		/* bad switch on size of coverage array */
#define SR109		9		/* error on fprintf */
#define SR110	       10		/* missing -x or -l option */
#define SR111	       11		/* bad option */

#define PNT_FAIL	0		/* fprintf return value on error */

#define FATAL		1		/* return value for exit() */

typedef char BOOL;			/* boolean type definition */
