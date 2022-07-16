/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:npat/space.c	1.3"
/* Host configuration word in Status Block */

#include "sys/interlan/il_types.h"
#include "sys/buf.h"
#include "sys/stream.h"
#include "sys/interlan/np600.h"
#include "sys/interlan/np.h"
#include "sys/iobuf.h"

/*
 * Tunables
 */

#define NNPDEV		1	/* Number of NP devices in the system */

#ifdef AT386
#define	NPDMA0		7	/* PC-AT Dma channel */
#define NPCMDP_VA0	0x310	/* base address of controller */
#define NPINT0		9	/* PC-AT Interrupt Level */
#else
#define NPCMDP_VA0	0x0170	/* base address of controller */
#define NPINT0		2	/* Multibus Interrupt Level */
#endif
/*
	for multiple boards define and uncomment the following
#define	NPINT1	
#define	NPDMA1
#define	NPCMDP_VA1
	etc.. for each additional board
	also add these defines to the arrays defined at the beginning
	of np.c
*/

/*
 * reset routines for pseudo drivers
 */

#define NPSEUDO		3	/* number of reset slots */

/* Host configuration word in Status Block */

#define HOSTCONF	HCW_PAN_LIS	/* See Spec for details */

#define	HCW_PAN_LIS	0x100		/* host has a panic listener */
#define	HCW_NO_PREMAP	0x200		/* buffer is not pre mapped */

#include "config.h"	/* for overriding above parameters */

/* variables to hold tunable parameters for use in other modules */

int nnpdev = NNPDEV;
int npseudo = NPSEUDO;
int hostconf = HOSTCONF;

/* Structure of the shared memory area */

struct npspace  npspaces[NNPDEV];

/*
 * Master structure, one per board, contains request queue header,
 * shared memory address, and memory mapping information.
 */

struct npmaster npmasters[NNPDEV];

/* Head of the request queue, one per board */

struct npreq reqhdr[NNPDEV];

/* The request structures, one pool per board */

struct npreq npreqs[NNPDEV][NUMCQE];
struct npreq *npreqbase = &npreqs[0][0];

/* Driver Wide Connection Table */

struct npconn npcnxtab[NNPCNN][NNPDEV];
struct npconn *npcnxbase = &npcnxtab[0][0];

/* buffers for I/O */

struct iobuf np_tab[NNPDEV];	/* read buf structure */

/* fields used for patching */

int	NpInt[NNPDEV] = { NPINT0 };
#ifdef AT386
int	NpDma[NNPDEV] = { NPDMA0 };
#endif
/*
 * Base address(es) of the I-Board(s), indexed by unit number.
 * Consult config.h for appropriate values.
 */

struct npbase npbase[NNPDEV] = { (caddr_t)NPCMDP_VA0 };

/*
 * Number of interrupts since last clear interrupt count
 */
int	np_icount[NNPDEV] = {0};

/* 
 * Delay time for patching
 */

unsigned long	DelayTime = DELAYTIME;

/* 
 * Array of mapped memory (for diagnostics ) 
 */

struct npreq *np_mapreq[NNPDEV];
/*
 * Jump table for inits
 */

int (*Np_init_tab[NPSEUDO]) () ;
