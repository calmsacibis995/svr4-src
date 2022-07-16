/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DDI_H
#define _SYS_DDI_H

#ident	"@(#)head.sys:sys/ddi.h	1.21.4.1"

/*
 * ddi.h -- the flag and function definitions needed by DDI-conforming
 * drivers.  This header file contains #undefs to undefine macros that
 * drivers would otherwise pick up in order that function definitions
 * may be used. Programmers should place the include of "sys/ddi.h"
 * after any header files that define the macros #undef'ed or the code
 * may compile incorrectly.
 */

/*
 * define min() and max() as macros so that drivers will not pick up the
 * min() and max() kernel functions since they do unsigned comparison only.
 */
#define min(a, b)   ((a) < (b) ? (a) : (b))
#define max(a, b)   ((a) < (b) ? (b) : (a))


/*
 * The following macros designate a kernel parameter for drv_getparm 
 * and drv_setparm. Implementation-specific parameter defines should
 * start at 100.
 */

#define	TIME	1
#define	UPROCP	2
#define	PPGRP	3
#define	LBOLT	4
#define	SYSRINT	5
#define	SYSXINT	6
#define	SYSMINT	7
#define	SYSRAWC	8
#define	SYSCANC	9
#define	SYSOUTC	10
#define	PPID	11
#define	PSID	12
#define UCRED	13

#ifndef NMAJORENTRY
#define NMAJORENTRY	256
#endif

extern int drv_getparm();
extern int drv_setparm();

extern int drv_getevtoken();
extern void drv_relevtoken();

extern void drv_usecwait();
extern clock_t drv_hztousec();
extern clock_t drv_usectohz();

/* convert external to internal major number */
extern int etoimajor();
/* convert internal to extern major number */
extern int itoemajor();

extern addr_t physmap();
extern void physmap_free();

extern u_int hat_getkpfnum();
extern u_int hat_getppfnum();

/* The following declaration takes the place of an inline function
 * defined in sys/inline.h .
 */

extern paddr_t kvtophys();	

#include	<sys/buf.h>
#include	<sys/uio.h>

#if defined(__STDC__)
extern int physiock(void(*)(), struct buf*, dev_t, int, daddr_t, struct uio*);
extern int drv_priv(struct cred *);
#else
extern int drv_priv();
extern int physiock();
#endif

/* The following declarations take the place of macros in 
 * sysmacros.h The undefs are for any case where a driver includes 
 * sysmacros.h, even though DDI conforming drivers must not.
 */

#undef getemajor
#undef geteminor
#undef getmajor
#undef getminor
#undef makedevice
#undef cmpdev
#undef expdev

extern major_t getemajor();
extern minor_t geteminor();
extern major_t getmajor();
extern minor_t getminor();
extern dev_t makedevice();
extern dev_t cmpdev();
extern dev_t expdev();

/* The following macros from param.h are also being converted to
 * functions and #undefs must be done here as well since param.h
 * will be included by most if not every driver 
 */

#undef btop
#undef btopr
#undef ptob

extern unsigned long btop();
extern unsigned long btopr();
extern unsigned long ptob();


/* Drivers must include map.h to pick up the structure definition */
/* for the map structure and the declaration of the function malloc(). */
/* Unfortunately, map.h also includes definitions of macros that */
/* drivers should be calling as functions. The following #undefs allow */
/* kernel code to use the macros while drivers call the functions */

#undef mapinit
#undef mapwant

extern void mapinit();
extern unsigned long mapwant();
extern void setmapwant();

/* when DKI changes are folded back in to DDI, the functions mapinit
 * mapwant and setmapwant should be updated to be the following DKI
 * functions:
 */

extern void rminit();
extern unsigned long rmwant();
extern void rmsetwant();



/* STREAMS drivers and modules must include stream.h to pick up the */
/* needed structure and flag definitions. As was the case with map.h, */
/* macros used by both the kernel and drivers in times past now have */
/* a macro definition for the kernel and a function definition for */
/* drivers. The following #undefs allow drivers to include stream.h */
/* but call the functions rather than macros. */

#undef OTHERQ
#undef RD
#undef WR
#undef datamsg
#undef putnext
#undef splstr

extern struct queue *OTHERQ();	/* stream.h */
extern struct queue *RD();
extern struct queue *WR();
extern int datamsg();
extern int putnext();
extern int splstr();

/* declarations of functions for allocating and deallocating the space */
/* for a buffer header (just a header, not the associated buffer) */

extern struct buf *getrbuf();
extern void freerbuf();

/* end of ddi.h */
#endif	/* _SYS_DDI_H */
