/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/size.h	1.3"
/*ident	"@(#)cfront:src/size.h	1.13" */
/*************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


size.h:
	sizes and alignments used to calculate sizeofs

	table and butffer sizes

***************************************************************************/

#ifndef GRAM
extern BI_IN_WORD;
extern BI_IN_BYTE;
				/*	byte sizes */
extern SZ_CHAR;
extern AL_CHAR;

extern SZ_SHORT;
extern AL_SHORT;

extern SZ_INT;
extern AL_INT;

extern SZ_LONG;
extern AL_LONG;

extern SZ_FLOAT;
extern AL_FLOAT;

extern SZ_DOUBLE;
extern AL_DOUBLE;

extern SZ_STRUCT;	/* minimum struct size */
extern AL_STRUCT;

//extern SZ_FRAME;
//extern AL_FRAME;

extern SZ_WORD;

extern SZ_WPTR;
extern AL_WPTR;

extern SZ_BPTR;
extern AL_BPTR;	

//extern SZ_TOP;
//extern SZ_BOTTOM;

extern char* LARGEST_INT;
extern int F_SENSITIVE;	// is field alignment sensitive to the type of the field?
extern int F_OPTIMIZED;	// can the compiler fit a small int field into a char?
#endif
				// default sizes:
#if u3b | u3b2 | u3b5
				/* AT&T 3Bs */
#define DBI_IN_WORD 32
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 4
#define DAL_INT 4
#define DSZ_LONG 4
#define DAL_LONG 4
#define DSZ_FLOAT 4
#define DAL_FLOAT 4
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 4
#define DSZ_STRUCT 4
#define DAL_STRUCT 4
//#define DSZ_FRAME 4
//#define DAL_FRAME 4
#define DSZ_WORD 4
#define DSZ_WPTR 4
#define DAL_WPTR 4
#define DSZ_BPTR 4
#define DAL_BPTR 4
//#define DSZ_TOP 0
//#define DSZ_BOTTOM 0
#define DLARGEST_INT "2147483647"	/* 2**31 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
#if apollo 
#define DBI_IN_WORD 16
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 4
#define DAL_INT 2
#define DSZ_LONG 4
#define DAL_LONG 2
#define DSZ_FLOAT 4
#define DAL_FLOAT 2
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 2
#define DSZ_STRUCT 2
#define DAL_STRUCT 2
//#define DSZ_FRAME 4
//#define DAL_FRAME 4
#define DSZ_WORD 2
#define DSZ_WPTR 4
#define DAL_WPTR 2
#define DSZ_BPTR 4
#define DAL_BPTR 2
//#define DSZ_TOP 0
//#define DSZ_BOTTOM 0
#define DLARGEST_INT "2147483647"	/* 2**31 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
#if sun | mc68k
				/* most M68K boxes */
#define DBI_IN_WORD 32
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 4
#define DAL_INT 2
#define DSZ_LONG 4
#define DAL_LONG 2
#define DSZ_FLOAT 4
#define DAL_FLOAT 2
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 2
#define DSZ_STRUCT 2
#define DAL_STRUCT 2
//#define DSZ_FRAME 4
//#define DAL_FRAME 4
#define DSZ_WORD 4
#define DSZ_WPTR 4
#define DAL_WPTR 2
#define DSZ_BPTR 4
#define DAL_BPTR 2
//#define DSZ_TOP 0
//#define DSZ_BOTTOM 0
#define DLARGEST_INT "2147483647"	/* 2**31 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
#if iAPX286 && LARGE
				/* Intel 80286 large model */
#define DBI_IN_WORD 16
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 2
#define DAL_INT 2
#define DSZ_LONG 4
#define DAL_LONG 2
#define DSZ_FLOAT 4
#define DAL_FLOAT 2
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 2
#define DSZ_STRUCT 2
#define DAL_STRUCT 2
//#define DSZ_FRAME 4
//#define DAL_FRAME 4
#define DSZ_WORD 2
#define DSZ_WPTR 4
#define DAL_WPTR 2
#define DSZ_BPTR 4
#define DAL_BPTR 2
//#define DSZ_TOP 0
//#define DSZ_BOTTOM 0
#define DLARGEST_INT "32767"	/* 2**15 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
#if uts
				/* Amdahl running UTS */
#define DBI_IN_WORD 32
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 4
#define DAL_INT 4
#define DSZ_LONG 4
#define DAL_LONG 4
#define DSZ_FLOAT 4
#define DAL_FLOAT 4
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 8
#define DSZ_STRUCT 1
#define DAL_STRUCT 1
#define DSZ_WORD 4
#define DSZ_WPTR 4
#define DAL_WPTR 4
#define DSZ_BPTR 4
#define DAL_BPTR 4
#define DLARGEST_INT "2147483647"	/* 2**31 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
#if vax || i386
				/* VAX (running V8) */
#define DBI_IN_WORD 32
#define DBI_IN_BYTE 8
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 2
#define DAL_SHORT 2
#define DSZ_INT 4
#define DAL_INT 4
#define DSZ_LONG 4
#define DAL_LONG 4
#define DSZ_FLOAT 4
#define DAL_FLOAT 4
#define DSZ_DOUBLE 8
#define DAL_DOUBLE 4
#define DSZ_STRUCT 1
#define DAL_STRUCT 1
//#define DSZ_FRAME 4
//#define DAL_FRAME 4
#define DSZ_WORD 4
#define DSZ_WPTR 4
#define DAL_WPTR 4
#define DSZ_BPTR 4
#define DAL_BPTR 4
//#define DSZ_TOP 0
//#define DSZ_BOTTOM 0
#define DLARGEST_INT "2147483647"	/* 2**31 - 1 */
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 1
#else
				/* defaults: 0 => error */
#define DBI_IN_WORD 0
#define DBI_IN_BYTE 0
#define DSZ_CHAR 1
#define DAL_CHAR 1
#define DSZ_SHORT 0
#define DAL_SHORT 0
#define DSZ_INT 0
#define DAL_INT 0
#define DSZ_LONG 0
#define DAL_LONG 0
#define DSZ_FLOAT 0
#define DAL_FLOAT 0
#define DSZ_DOUBLE 0
#define DAL_DOUBLE 0
#define DSZ_STRUCT 0
#define DAL_STRUCT 0
#define DSZ_WORD 0
#define DSZ_WPTR 0
#define DAL_WPTR 0
#define DSZ_BPTR 0
#define DAL_BPTR 0
#define DLARGEST_INT "0"
#define DF_SENSITIVE 0
#define DF_OPTIMIZED 0
#endif
#endif
#endif
#endif
#endif
#endif

#define KTBLSIZE	123	/*	initial keyword table size */
#define GTBLSIZE	257	/*	initial global name table size */
#define CTBLSIZE	12	/*	initial class table size */
#define TBLSIZE		20	/*	initial block table size */
#define BLMAX		50	/*	max block nesting */
#define TBUFSZ		48*1024	/*	(lex) input buffer size */
#define MAXFILE		255	/*	max include file nesting */
#define MAXERR		13	/* maximum number of errors before terminating */

#ifndef GRAM
const CHUNK = 8*1024;
void* chunk(int);
#endif
