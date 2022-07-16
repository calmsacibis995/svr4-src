/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mallint.h	1.2"

#include <stdlib.h>
#include <memory.h>

/* debugging macros */
#ifdef	DEBUG
#define	ASSERT(p)	((void) ((p) || (abort(),0)))
#define COUNT(n)	((void) n++)
static int		nmalloc, nrealloc, nfree;
#else
#define	ASSERT(p)	((void)0)
#define COUNT(n)	((void)0)
#endif /*DEBUG*/

/* function to copy data from one area to another */
#define MEMCOPY(to,fr,n)	((void)memcpy(to,fr,n))

/* for conveniences */
#ifndef NULL
#define	NULL		(0)
#endif

#define reg		register
#define WORDSIZE	((int)(sizeof(WORD)))
#define MINSIZE		((int)(sizeof(TREE)-sizeof(WORD)))
#define ROUND(s)	if(s%WORDSIZE) s += (WORDSIZE - (s%WORDSIZE))

#ifdef	DEBUG32
/* The following definitions ease debugging
** on a machine in which sizeof(pointer) == sizeof(int) == 4.
** These definitions are not portable.
*/
#define	ALIGN	4
typedef int	WORD;
typedef struct _t_
	{
	size_t		t_s;
	struct _t_	*t_p;
	struct _t_	*t_l;
	struct _t_	*t_r;
	struct _t_	*t_n;
	struct _t_	*t_d;
	}	TREE;
#define SIZE(b)		((b)->t_s)
#define AFTER(b)	((b)->t_p)
#define PARENT(b)	((b)->t_p)
#define LEFT(b)		((b)->t_l)
#define RIGHT(b)	((b)->t_r)
#define LINKFOR(b)	((b)->t_n)
#define LINKBAK(b)	((b)->t_p)

#else	/* !DEBUG32 */
/* The following definitions assume that "char *" has the largest size
** among all pointer types. If this is not true, PTRSIZE should
** be redefined to be the size of the largest pointer type (void * ?).
** All of our allocations will be aligned on the least multiple of 4
** that is >= PTRSIZE. So the two low order bits are guaranteed to be
** available.
*/
#define PTRSIZE		((int)(sizeof(VOID *)))
#define ALIGN		(PTRSIZE%4 ? PTRSIZE+(4-(PTRSIZE%4)) : PTRSIZE)

/* the proto-word */
typedef union _w_
	{
	size_t		w_i;		/* an unsigned int */
	struct _t_	*w_p;		/* a pointer */
	char		w_a[ALIGN];	/* to force alignment */
	} WORD;

/* structure of a node in the free tree */
typedef struct _t_
	{
	WORD	t_s;	/* size of this element */
	WORD	t_p;	/* parent node */
	WORD	t_l;	/* left child */
	WORD	t_r;	/* right child */
	WORD	t_n;	/* next in link list */
	WORD	t_d;	/* dummy to reserve space for self-pointer */
	} TREE;

/* usable # of bytes in the block */
#define SIZE(b)		(((b)->t_s).w_i)

/* free tree pointers */
#define PARENT(b)	(((b)->t_p).w_p)
#define LEFT(b)		(((b)->t_l).w_p)
#define RIGHT(b)	(((b)->t_r).w_p)

/* forward link in lists of small blocks */
#define AFTER(b)	(((b)->t_p).w_p)

/* forward and backward links for lists in the tree */
#define LINKFOR(b)	(((b)->t_n).w_p)
#define LINKBAK(b)	(((b)->t_p).w_p)

#endif	/* DEBUG32 */

/* set/test indicator if a block is in the tree or in a list */
#define SETNOTREE(b)	(LEFT(b) = (TREE *)(-1))
#define ISNOTREE(b)	(LEFT(b) == (TREE *)(-1))

/* functions to get information on a block */
#define DATA(b)		(((char *) (b)) + WORDSIZE)
#define BLOCK(d)	((TREE *) (((char *) (d)) - WORDSIZE))
#define SELFP(b)	((TREE **) (((char *) (b)) + SIZE(b)))
#define LAST(b)		(*((TREE **) (((char *) (b)) - WORDSIZE)))
#define NEXT(b)		((TREE *) (((char *) (b)) + SIZE(b) + WORDSIZE))
#define BOTTOM(b)	((DATA(b)+SIZE(b)+WORDSIZE) == Baddr)

/* functions to set and test the lowest two bits of a word */
#define BIT0		(01)		/* ...001 */
#define BIT1		(02)		/* ...010 */
#define BITS01		(03)		/* ...011 */
#define ISBIT0(w)	((w) & BIT0)	/* Is busy? */
#define ISBIT1(w)	((w) & BIT1)	/* Is the preceding free? */
#define SETBIT0(w)	((w) |= BIT0)	/* Block is busy */
#define SETBIT1(w)	((w) |= BIT1)	/* The preceding is free */
#define CLRBIT0(w)	((w) &= ~BIT0)	/* Clean bit0 */
#define CLRBIT1(w)	((w) &= ~BIT1)	/* Clean bit1 */
#define SETBITS01(w)	((w) |= BITS01)	/* Set bits 0 & 1 */
#define CLRBITS01(w)	((w) &= ~BITS01)/* Clean bits 0 & 1 */
#define SETOLD01(n,o)	((n) |= (BITS01 & (o)))

/* system call to get more core */
#define GETCORE		sbrk
#define ERRCORE		((char *)(-1))
#define CORESIZE	(1024*ALIGN)

extern VOID	*GETCORE();
