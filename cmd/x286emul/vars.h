/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:vars.h	1.1"

#include <stdio.h>

/*
 * def() and as() are used so that exactly the same declarations can be used
 * to declare and define the variables mentioned in this file.
 */
#ifndef MAIN
#define         def(X)  extern X
#define		as(X)
#else
#define         def(X)  X
#define		as(X) = X
#endif

#include "h/a.out.h"

/*
 * structure to keep track of 386 location of 286 segments
 */
struct dseg {
	unsigned lsize;			/* size in LDT */
	unsigned psize;			/* size in memory */
	char * base;			/* address it starts at */
	int type;			/* what the segment is used for */
};

	/* values for "type" field of dseg struct */
#define	TEXT	0			/* text segment */
#define	DATA	1			/* data segment */
#define	SHARE	2			/* some sort of shared thingy */

def(struct dseg *Dsegs) as(0);
def(int Numdsegs) as(0);		/* how many dsegs */

#define	BAD_ADDR	((char *)0xFF00FF00)
#define ANYWHERE	0		/* nextfreedseg() can allocate freely */

def(int Stacksize) as(0);		/* number of bytes in stack */
def(int Stacksel) as(0);		/* selector for segment with stack */
def(int Stackbase);			/* offset of stack in it's segment */
def(char * TheStack) as(0);		/* the segment containing the stack */

char * malloc(), * getmem();
char * cvtptr(), * cvtchkptr();

def(char * Pgmname);                    /* name of emulator */
def(int My_PID) as(0);			/* emulator's PID */

def(char * Tfile) as(0);		/* base of mapped text file */
def(long  Tfsize) as(0);		/* size of mapped text file */
def(char * Dfile) as(0);		/* base of mapped data file */
def(long  Dfsize) as(0);		/* size of mapped data file */

	/* macros to find out if an address is mapped from the x.out */
#define ISDMAPPED(x)	((x) > Dfile && (x) < Dfile+Dfsize)
#define ISTMAPPED(x)	((x) > Tfile && (x) < Tfile+Tfsize)
#define ISMAPPED(x)	(ISDMAPPED(x) || ISTMAPPED(x))

def(struct xexec *X) as(0);		/* x.out header */
def(struct xext  *Xe) as(0);		/* x.out header extension */
def(struct xseg  *Xs) as(0);		/* x.out segment table entry */
def(int Xnumsegs) as(0);		/* number of entries in seg. table */
def(int Lastdseg) as(0);		/* last data segment in x.out file */

def(int BADVISE_flags) as(0);		/* current setting of SI86BADVISE */

def(int Ldata) as(0);			/* != 0 ==> large data */
def(int Ltext) as(0);			/* != 0 ==> large text */

	/* Nodeath != 0 causes certain otherwise fatal errors to be ignored */
def(int Nodeath) as(0);

/*
 * addressing convenience macros
 */
	/* make a 286 selector:offset pointer */
#define MAKEPTR(sel,off) (((sel)<<16)|off)
#define	SEL(X) (((X)>>16)&0xFFFF)	/* get the selector from a 286 ptr */
#define OFF(X) ((X)&0xFFFF)		/* get the offset from a 286 ptr */
#define SELTOIDX(X) (((X)>>3)&0x1FFF)	/* turn a selector into a Dsegs index */
#define IDXTOSEL(X) (((X)<<3)|7)	/* convert Dsegs index to selector */
#define NBPS	0x10000			/* number of bytes per (286) segment */

/*
 * flags to turn on various debugging options.  this stuff will go
 * away when this is finished
 */
#if defined(TRACE) || defined(DEBUG)
def(int systrace) as(0);		/* system call trace */
#endif

extern int errno;

#define	MAXSHMSEGS	30		/* number of shared memory segments */
	/* base 386 address for shared memory segments */
#define	SHMBASE		((char *)(0x90000000-(MAXSHMSEGS<<22)))
	/* addressing for shared memory segments */
#define	SHMSEG(i)	(SHMBASE+((i)<<22))

#if defined(TRACE) || defined(DEBUG)
/*
 * dbgfd is the stream upon which debugging information is written when
 * the emulator is built with -DTRACE or -DDEBUG.
 */
def(FILE *dbgfd) as(stdout);	/* stdout by default */
def(int dbgdesc) as(1);		/* Close() needs the actual descriptor */
#endif
