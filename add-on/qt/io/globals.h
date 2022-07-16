/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)qt:io/globals.h	1.3"
/* I/O controller addresses */
static int	STATPORT = 0;
static int	CTLPORT = 0;
static int	DATAPORT = 0;
static int	CMDPORT = 0;

static struct iqt *iqt = 0;

static ALIGN space[SPACEWORDS];

	/* Hardware control port bits */
static int	CTL_INTRENA	= 0;
static int	CTL_ONLINE	= 0;
static int	CTL_RESET	= 0;
static int	CTL_REQUEST	= 0;
static int	CTL_DONEN	= 0;

	/* Hardware status port bits */
static int	STS_DONE	= 0;
static int	STS_NRDY	= 0;
static int	STS_NEXC	= 0;
static int	STS_NINT	= 0;

static int	DMAGO		= 0;
static int	RSTDMA		= 0;

#define	BT_UNKNOWN	0		/* Controller unknown		*/
#define	BT_WANGTEK	1		/* Wangtek PC-36 controller	*/
#define	BT_ARCHIVE	2		/* Archive SC499R controller	*/

static int	board_type	= BT_UNKNOWN;
