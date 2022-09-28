/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/stak.h	1.1.3.1"
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	AT&T Bell Laboratories
 *
 */

typedef char		*STKPTR;

/*
 * To use stack as temporary workspace across
 * possible storage allocation (eg name lookup)
 * a) get ptr from `stak_offset'
 * b) can now use `stak_push'
 * c) then reset with `stak_set'
 * d) `stak_address' gives real address if needed
 */

#define		stak_offset()	(sh.staktop-sh.stakbot)
#define		stak_address(x)	(sh.stakbot+(x))
#define		stak_set(x)	(sh.staktop=sh.stakbot+(x))
#define		stak_push(c)	(*sh.staktop++=(c))
#define		stak_zero()	(*sh.staktop=0)

/*
 * Used to address current word on the top of
 * the stack (very temporary)
 */
#define 	stak_word()	(sh.stakbot)

/*
 * for local use only since it hands
 * out a real address for the stack top
 */
#ifdef PROTO
    extern STKPTR		stak_begin(void);
#else
    extern STKPTR		stak_begin();
#endif /* PROTO */

/*
 * Will allocate the item being used and return its
 * address (safe now).
 */
#define		stak_fix()	stak_end(sh.staktop)

/*
 * For use after `stak_begin' to hand back
 * new stack top and then allocate item
 */
#ifdef PROTO
    extern STKPTR		stak_end(char*);
#else
    extern STKPTR		stak_end();
#endif /* PROTO */

/*
 * Copy a string onto the stack and
 * allocate the space.
 */
#ifdef PROTO
    extern STKPTR		stak_copy(const char*);
#else
    extern STKPTR		stak_copy();
#endif /* PROTO */

/* Allocate given amount of stack space */
#ifdef PROTO
    extern STKPTR		stak_alloc(unsigned);
#else
    extern STKPTR		stak_alloc();
#endif /* PROTO */

/* Bring the stack back to a given address */
#ifdef PROTO
    extern void 	stak_reset(STKPTR);
#else
    extern void 	stak_reset();
#endif /* PROTO */

/* Check to see if stack can be shrunk */
#ifdef PROTO
    extern int		stak_check(void);
#else
    extern int		stak_check();
#endif /* PROTO */

