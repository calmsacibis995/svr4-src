/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:vars.h	1.1"

#ifndef MAIN
#define         def(X)  extern X
#else
#define         def(X)  X
#endif

#define MAXDSEGS        1024            /* max. number of data segments */
struct dseg {
	unsigned size;			/* size in bytes */
	char * base;			/* address it starts at */
};
def(struct dseg dsegs[ MAXDSEGS ]);
def(int firstds);                       /* number of first data segment */
def(int lastds);                        /* number of last data segment */

#define	BAD_ADDR	((char *)0xFFFF0000)

def(int uu_model);                      /* small or large model proc */
#define U_MOD_LARGE     1
#define U_MOD_SMALL     0

def(int smssize);                       /* small model stack size */
def(int smdsize);                       /* small model stack+data+bss */


def(unsigned short stacksegsel);        /* selector for 286 stack */
def(unsigned short *stackbase);         /* the stack base ( large model ) or
					 * stack/data base ( small model ) */

def(unsigned nodeath);			/* no death on failed system calls */

char * malloc(), * getmem();
char * cvtptr(), * cvtchkptr();

def(char * pgmname);                    /* name of emulator */

#include <sys/types.h>

#define	ctob(X) ((X)<<9)
#define btoc(X) (((X)+511)>>9)
