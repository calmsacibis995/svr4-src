/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/ttymodes.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)ttymodes.c	3.11	LCC);	/* Modified: 16:39:41 11/17/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifdef RS232PCI

#include <pci_types.h>
#include <termio.h>

/* Translation from baud-rate bits to actual baudrate. */
unsigned short	ttyspeeds[CBAUD+1] =
{
	0, 50, 75, 110, 134, 150, 200, 300,
	600, 1200, 1800, 2400, 4800, 9600, 19200, 38400
};
int vmin;

#ifdef RS232_7BIT
extern int using_7_bits;
#endif /* RS232_7BIT */

#if	defined(SYS5) || defined(ULTRIX)

get_tty(fd,ttymodes)
int fd;
struct termio *ttymodes;
{
	return(ioctl(fd,TCGETA,ttymodes));
}

set_tty(fd,ttymodes)
int fd;
struct termio *ttymodes;
{
	return(ioctl(fd,TCSETA,ttymodes));
}

void
rawify_tty(ttymodes)
struct termio *ttymodes;
{

#ifdef RS232_7BIT
	if (using_7_bits)
		ttymodes->c_iflag &= (IGNPAR|PARMRK|INPCK|ISTRIP);
	else {
#endif /* RS232_7BIT */
		ttymodes->c_iflag = 0;
		ttymodes->c_cflag = (ttymodes->c_cflag & CBAUD) | CS8 | CREAD;
#ifdef RS232_7BIT
	}
#endif /* RS232_7BIT */
	/* Set up TTY modes for eight bit "raw" mode */
#ifndef IGNORE_BREAK
	ttymodes->c_iflag |= BRKINT;
#else /* IGNORE_BREAK */
	/*
	 * On stips lines (currently the only IX370 ascii lines),
	 * breaks are generated when input overruns the series/1...
	 * I believe this may break a feature that makes
	 * it easier to get a pci/rs232 line back (typing break used
	 * to kill the server... now you have to drop carrier) (joe).
	 */
	ttymodes->c_iflag |= IGNBRK;
#endif /* IGNORE_BREAK */
	ttymodes->c_oflag = 0;
	ttymodes->c_lflag = 0;
	ttymodes->c_cflag |= HUPCL;
	ttymodes->c_cc[VTIME] = TTY_VTIME;
	/*
	 * Set VMIN based on the baudrate.
	 */
#define BPC		10	/* 10 bits per character. */
#define	VMINTICK	10	/* vmin is in 1/10ths of sec. */
#define	MAXVMIN		0x7f	/* Maximum unsigned char. */
	vmin = ((long)TTY_VTIME * ttyspeeds[ttymodes->c_cflag&CBAUD])
		/ (unsigned)(BPC*VMINTICK);
	if (vmin < TTY_VMIN)
		vmin = TTY_VMIN;
	else if (vmin > MAXVMIN)
		vmin = MAXVMIN;
	ttymodes->c_cc[VMIN] = vmin;
}

#ifdef	TIMEOUT
timeout_on(fd, ttymodes)
int fd;
struct termio *ttymodes;
{
	ttymodes->c_cc[VTIME] = 2;
	ttymodes->c_cc[VMIN] = 0;

	set_tty(fd, ttymodes);

	return;
}

timeout_off(fd, ttymodes)
int fd;
struct termio *ttymodes;
{
	ttymodes->c_cc[VTIME] = TTY_VTIME;
	ttymodes->c_cc[VMIN] = TTY_VMIN;

	set_tty(fd, ttymodes);

	return;
}
#endif	/* TIMEOUT */

drain_tty(ttyDesc)
int
	ttyDesc;
{
	ioctl(ttyDesc, TCSBRK, 1);
}

#else	/* !SYS5 || !ULTRIX */

get_tty(fd, pt)
int fd;			/* basic tty file descriptor */
struct termio *pt;	/* pt to tty line state */
{
	if (
		(ioctl(fd, TIOCGETP, &pt->m) < 0) ||	/* Get basic modes */
		(ioctl(fd, TIOCGETC, &pt->tc) < 0) ||	/* Get special chars */
		(ioctl(fd, TIOCLGET, &pt->lm) < 0) || 	/* Get local mode */
		(ioctl(fd, TIOCGLTC, &pt->ltc) < 0)	/* Get local special chars */
		) return (-1);				/* error indication */
	else return(0);				/* success indication */
}

set_tty(fd, pt)
int fd;			/* basic tty file descriptor */
struct termio *pt;	/* pt to tty line state */
{
	if (
		(ioctl(fd, TIOCSETP, &pt->m) < 0) ||	/* Set basic modes */
		(ioctl(fd, TIOCSETC, &pt->tc) < 0) ||	/* Set special chars */
		(ioctl(fd, TIOCLSET, &pt->lm) < 0) ||	/* Set local mode */
		(ioctl(fd, TIOCSLTC, &pt->ltc) < 0)	/* Set local special chars */
		) return (-1);				/* error indication */
	else return(0);				/* success indication */
}

void
rawify_tty(pt)
struct termio *pt;	/* pt to tty line state */
{
	pt->m.sg_flags = RAW;
}

drain_tty(fd)
int
	fd;
{
struct termio pt;

	/* This is the way to drain terminal I/O in 4.2BDS.  The TIOCSETP
	   waits until output is quiescent before changing the modes (in
	   this case to the same thing) 
        */

	ioctl(fd, TIOCGETP, &pt);
	ioctl(fd, TIOCSETP, &pt);
}
#endif /* !SYS5 || !ULTRIX20 */

#endif /* RS232PCI */
