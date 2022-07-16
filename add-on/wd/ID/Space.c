#ident	"@(#)Space.c	1.2	92/02/17	JPB"

/*
 * Module: WD8003
 * Project: System V ViaNet
 *
 *		Copyright (c) 1987, 1988 by Western Digital Corporation.
 *		All rights reserved.  Contains confidential information and
 *		trade secrets proprietary to
 *			Western Digital Corporation
 *			2445 McCabe Way
 *			Irvine, California 92714
 */

#ident "@(#)/usr/src/add-on/pkg.wd/../../add-on/wd/ID/Space.c.sl 1.1 4.0 02/20/90 35157 USO"
#ident "$Header: Space.c 2.3 90/06/08 $"

/*
 * Configuration file for WD8003S (Starlan) / WD8003E (Ethernet).
 * All user configurable options are here (automatically set on PS-2).
 */

#include "sys/types.h"
#include "sys/stream.h"
#include "sys/wd.h"
#include "sys/socket.h"
#include "net/if.h"

#define WDDEBUG		0		/* trace transmit attempts */

#include "config.h"

#define NWDDEV		WD_UNITS	/* Number of WD 8003 (sub) devices */

#define WDIRQ0		WD_0_VECT	/* IRQ value */
#define WDBASEPORT0     WD_0_SIOA	/* Base I/O port */
#define WDBASEADDR0     WD_0_SCMA	/* Base shared memory address */
#define WDBOARDSIZE0	(WD_0_ECMA-WD_0_SCMA)+1		/* Board size */
#define WDMAJOR0        WD_CMAJOR_0     /* Board major device number */

#define NWD		WD_CNTLS	/* Number of WD 8003 boards */
#define NSTR		(NWDDEV/NWD)	/* Number of streams/board */
#define MAXMULTI	16		/* Number of multicast addrs/board */


struct wddev wddevs[NSTR*NWD];
struct wdstat wdstats[NWD];
struct ifstats wdifstats[NWD];
int    wd_boardcnt = NWD;

struct wdmaddr wdmultiaddrs[NWD * MAXMULTI];	/* multicast addr storage */
int    wd_multisize = MAXMULTI;			/* # of multicast addrs/board */

int 	     wd_debug = WDDEBUG;		/* can be enabled dynamically */

struct wdparam wdparams[NWD] = {
    {
	0,				/* board index */
	WDIRQ0,				/* interrupt level */
	WDBASEPORT0,			/* I/O port for device */
	(caddr_t)WDBASEADDR0,		/* address of board's memory */
	WDBOARDSIZE0, 			/* memory size */
	0,				/* pointer to mapped memory */
	0,				/* board type */
	0,				/* board present flag */
	0,				/* board status */
	0,				/* number of streams open */
	WDMAJOR0,			/* major device number */
	NSTR				/* number of minor devices allowed */
    }
};

