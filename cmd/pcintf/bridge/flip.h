/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/flip.h	1.1"
/* SCCSID(@(#)flip.h	3.14	LCC);	/* Modified: 16:51:23 2/12/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*
   This file provides constants and macros used by the low level
   network code to restore byte and word orders of foreign sites
   to those of the local site.
*/

/*
   The originator of a message placed SENSEORDER into the message
   This allows the receiver of the message to find out the byte
   ordering of the CPU that sent the message.
*/
#define	SENSEORDER	0x00020103L

/*
   If SENSEORDER is received as SENSES,
   only bytes within shorts were flipped.
*/
#define	SENSES		0x02000301L

/*
   If SENSEORDER is received as SENSEL, only shorts
   within longs were flipped.
*/
#define	SENSEL		0x01030002L

/*
   If SENSEORDER is received as SENSESL, both bytes
   within shorts and shorts within longs were flipped.
*/
#define	SENSESL		0x03010200L

/*
   If the received value of SENSEORDER & FLIPLONGS is
   non-zero, shorts must be flipped in longs
*/
#define	FLIPLONGS	0x00010000L

/*
   If the received value of SENSEORDER & FLIPSHORTS is
   non-zero, chars must be flipped in shorts
*/
#define	FLIPSHORTS	0x00000200L


/*
   The following are constants for the `how' argument to the flipping
   macros and functions, telling how to flip the value passed to them.
   This avoids passing long order detection constants, allowing the
   flipping functions and macros to be more efficient on 16 bit
   machines.
*/
#define	NOFLIP	0x0			/* No flip necessary		*/
#define	LFLIP	0x1			/* Flip shorts in longs		*/
#define	SFLIP	0x2			/* Flip chars in shorts		*/
#define	LSFLIP	0x3			/* Flip both longs and shorts	*/
#define	SLFLIP	0x3			/* Convenience			*/

#if	vax
#define	HOW_TO_FLIP	NOFLIP
#define	LSB	0
#define	LSW	0
#endif	/* vax */

/* DECstation 3100; "mips" is pre-defined by the compiler */
#if	mips
#define	HOW_TO_FLIP	NOFLIP
#define	LSB	0
#define	LSW	0
#endif	/* mips */

#if	iAPX286 || M_I286 || iAPX386
#   ifdef	AIX_RT
#	define	HOW_TO_FLIP	LSFLIP
#	define	LSB	3
#	define	LSW	2
#   else	/* !AIX_RT */
#	define	HOW_TO_FLIP	NOFLIP
#	define	LSB	0
#	define	LSW	0
#   endif	/* AIX_RT */
#endif	/* iAPX286 */

#if     u3b2 || u3b5 || u3b15 || u3b
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif  /* u3b2 || u3b5  || u3b15  || u3b */

#if	mc68k || m68k || M68000 || MC68000 || tower || m88k || m88000
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif	/* mc68k || m68k || M68000 || MC68000 || tower || m88k || m88000 */

#if	tahoe
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif	/* tahoe - Sperry 700/40 aka CCI */

#if	ridge
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif	/* ridge */

#if	accel
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif	/* accel */

#if	u370
#define	HOW_TO_FLIP	LSFLIP
#define	LSB	3
#define	LSW	2
#endif	/* u370 */

#if	ns32000
#define	HOW_TO_FLIP	NOFLIP
#define	LSB	0
#define	LSW	0
#endif	/* ns32000 */

#ifndef	HOW_TO_FLIP
#include	"Unknown Host CPU type"
#endif	/* !HOW_TO_FLIP */

/*
   FLIPHOW: create `how' arg to flippers
   
   Turns the order detection long into a `how' argument for the flipping
   macros and functions
*/
#ifdef JANUS
#define FLIPHOW(order)
#else
#define	FLIPHOW(order)\
    (((order) & FLIPLONGS ? LFLIP : 0) | (((order) & FLIPSHORTS) ? SFLIP : 0))
#endif

extern short	sflipf();		/* Short flip func returns flip	*/
extern long	lflipf();		/* Long flip func returns flip	*/
extern void	sflipa();		/* Flip short array in place	*/
extern void	lflipa();		/* Flip long array in place	*/

union lparts
{
	struct { char	lp_lc1, lp_lc2, lp_lc3, lp_lc4; }	lp_lc;
	struct { short	lp_ls1, lp_ls2; }			lp_ls;
	long							lp_ll;
};

union sparts
{
	struct { char	sp_sc1, sp_sc2; }	sp_sc;
	short					sp_ss;
};


/*
   dosflipm: Flip short in place unconditionally

   args:
	s	short to flip in place
	ts	temporary short

   The (union sparts *)(&s) notation allows the caller to pass a short
   instead of a `union sparts'.
*/
#ifdef JANUS
#define dosflipm(s, ts)
#else
#define	dosflipm(s, ts)\
  ((union sparts *)(&ts))->sp_sc.sp_sc1 = ((union sparts *)(&s))->sp_sc.sp_sc2;\
  ((union sparts *)(&ts))->sp_sc.sp_sc2 = ((union sparts *)(&s))->sp_sc.sp_sc1;\
  s = ts;
#endif

/*
   sflipm: Flip a short in place if necessary

   args:
	s	short to flip in place
	ts	temporary short
	how	how to flip it
*/
#ifdef JANUS
#define sflipm(s, ts, how)
#else
#define	sflipm(s, ts, how) \
	if ((how) & SFLIP) { \
		dosflipm(s, ts); \
	}
#endif

/*
 * Due to the preprocessor constraints of XENIX the flipm macro has been
 * broken into several macros inside of the single lflipm macro.
 * The next 5 macros are support for the switch statement of lflipm.
 */

#ifdef JANUS
#define mS1(l, tl)
#define mS2(l, tl)
#define mLS(l, tl)
#define mSL1(l, tl)
#define mSL2(l, tl)
#else
#define mS1(l, tl)\
  ((union lparts *)(&tl))->lp_lc.lp_lc1 = ((union lparts *)(&l))->lp_lc.lp_lc2;\
  ((union lparts *)(&tl))->lp_lc.lp_lc2 = ((union lparts *)(&l))->lp_lc.lp_lc1;

#define mS2(l, tl)\
  ((union lparts *)(&tl))->lp_lc.lp_lc3 = ((union lparts *)(&l))->lp_lc.lp_lc4;\
  ((union lparts *)(&tl))->lp_lc.lp_lc4 = ((union lparts *)(&l))->lp_lc.lp_lc3;

#define mLS(l, tl)\
  ((union lparts *)(&tl))->lp_ls.lp_ls1 = ((union lparts *)(&l))->lp_ls.lp_ls2;\
  ((union lparts *)(&tl))->lp_ls.lp_ls2 = ((union lparts *)(&l))->lp_ls.lp_ls1;

#define mSL1(l, tl)\
  ((union lparts *)(&tl))->lp_lc.lp_lc1 = ((union lparts *)(&l))->lp_lc.lp_lc4;\
  ((union lparts *)(&tl))->lp_lc.lp_lc2 = ((union lparts *)(&l))->lp_lc.lp_lc3;

#define mSL2(l, tl)\
  ((union lparts *)(&tl))->lp_lc.lp_lc3 = ((union lparts *)(&l))->lp_lc.lp_lc2;\
  ((union lparts *)(&tl))->lp_lc.lp_lc4 = ((union lparts *)(&l))->lp_lc.lp_lc1;
#endif

/*
   lflipm: long in place flip macro

   args:
	l	long to flip in place
	tl	temporary long
	how	how to flip it

   The (union lparts *)(&l) notation allows the caller to pass a long
   instead of a `union lparts'.
*/
#if	defined(JANUS) || defined(XENIX)
#define lflipm(l, tl, how)
#else	/* not (defined(JANUS) || defined(XENIX)) */
#define	lflipm(l, tl, how)\
 switch (how)\
 {\
 case NOFLIP: break;\
 case SFLIP:\
 mS1(l, tl)\
 mS2(l, tl)\
 l = tl;\
 break;\
 case LFLIP:\
 mLS(l, tl)\
 l = tl;\
 break;\
 case LSFLIP:\
 mSL1(l, tl)\
 mSL2(l, tl)\
 l = tl;\
 break;\
 }
#endif	/* not (defined(JANUS) || defined(XENIX)) */
