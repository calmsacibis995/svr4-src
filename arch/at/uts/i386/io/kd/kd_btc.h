/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/kd_btc.h	1.1"

#define	KD_BLIT		KD_VDC600+1	/* Bell Tech blit card */
#define	MCAP_BLIT	MCAP_MONO+1	/* Bell Tech blit graphics card */

/* defined in btc.c */
extern	int	btcpresent;
extern	ushort	*btcpbuf;	/* kernel virtual buffer address */
extern	caddr_t	btcbuf;		/* physical buffer address */
