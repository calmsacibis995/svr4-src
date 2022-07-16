/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_NSXT_H
#define _SYS_NSXT_H

#ident	"@(#)head.sys:sys/nsxt.h	1.2.2.1"

/*	nsxt.h: STREAMS SXT driver header */

/*
 **	Multiplexed channels driver header
 */

#define	SXTRACE		0		/* 1 to include tracing */

#define	MAXLINKS	32
#define	CHAN(dev)	(dev&CHANMASK)
#define	LINK(dev)	((dev>>CHANBITS)&(0xff>>CHANBITS))

#if	(MAXPCHAN*MAXLINKS) > 256
	ERROR -- product cannot be greater than minor(dev)
#endif

/*
 * Flags for virtual TTY channels
 */
#define	SXTCTL	1
#define SXTBLK	2
#define SXT_IOCTL 4

/*
 * Flags for control channel
 */
#define WAITSW  2	/* M_CTL (+ others, maybe) queued waiting for ACK/NAK */
#define SXTIOCWAIT 4	/* waiting for ACK/NAK on active virtual TTY */

struct Channel
{
	struct strtty	tty;		/* Virtual tty for this channel */
};

typedef struct Channel *Ch_p;

struct Link
{
	struct strtty  *line;		/* Real tty for this link */
	char		controllingtty;	/* the current top dog */
	char		old;		/* Old line discipline for line */
	char		nchans;		/* Number of channels allowed */
	unsigned char	chanmask;	/* Allowable channel bits */
	char		open;		/* Channel open bits */
	char		xopen;		/* Exclusive open bits */
	char		wpending;	/* pending writes/channel */
	char		iblocked;	/* channels blocked for input */
	char		oblocked;	/* channels blocked for output*/
	char		lwchan;		/* Last channel written bit */
	char		wrcnt;		/* Number of writes on last channel written */
	dev_t		dev;		/* major and minor device # */
	struct Channel	chans[1];	/* Array of channels for this link */
};

typedef	struct Link *	Link_p;

/*
**	Ioctl args
*/

#define	SXTIOCLINK	('b'<<8)
#define	SXTIOCTRACE	(SXTIOCLINK|1)
#define	SXTIOCNOTRACE	(SXTIOCLINK|2)
#define SXTIOCSWTCH	(SXTIOCLINK|3)
#define	SXTIOCWF	(SXTIOCLINK|4)
#define SXTIOCBLK	(SXTIOCLINK|5)
#define SXTIOCUBLK	(SXTIOCLINK|6)
#define SXTIOCSTAT	(SXTIOCLINK|7)


/* the following structure is used for the SXTIOCSTAT ioctl call */
struct sxtblock
{
	char	input;		/* channels blocked on input  */
	char	output;		/* channels blocked on output */
};



#define	t_link		t_dstat		/* Use dstat in real tty for linknumber */

#define	MAXPCHAN	8			/* Maximum channel number */
#define	CHANBITS	3			/* Bits for channel number */
#define	CHANMASK	07			/* 2**CHANBITS - 1 */
#define	SXTHOG		2			/* Channel consecutive write limit */

#endif	/* _SYS_NSXT_H */
