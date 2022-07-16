/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:dial/line.c	1.2.1.1"

/* This is a new line.c, which consists of line.c and culine.c
 * merged together.
 */

#include "uucp.h"

static struct sg_spds {
	int	sp_val,
		sp_name;
} spds[] = {
	{  50,   B50},
	{  75,   B75},
	{ 110,  B110},
	{ 134,  B134},
	{ 150,  B150},
	{ 200,  B200},
	{ 300,  B300},
	{ 600,  B600},
	{1200, B1200},
	{1800, B1800},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
#ifdef EXTA
	{19200,	EXTA},
#endif
#ifdef B19200
	{19200,	B19200},
#endif
#ifdef B38400
	{38400,	B38400},
#endif
	{0,    0}
};

#define PACKSIZE	64
#define HEADERSIZE	6

GLOBAL int
     packsize = PACKSIZE,
    xpacksize = PACKSIZE;

#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

GLOBAL int donap;	/* for speedup hook in pk1.c */
static int Saved_line;		/* was savline() successful?	*/
GLOBAL int
	Oddflag = 0,	/* Default is no parity */
	Evenflag = 0,	/* Default is no parity */
	Duplex = 1,	/* Default is full duplex */
	Terminal = 0,	/* Default is no terminal */
	term_8bit = -1,	/* Default to terminal setting or 8 bit */
	line_8bit = -1;	/* Default is same as terminal */

static char *P_PARITY  = "Parity option error\r\n";

#ifdef ATTSV

static struct termio Savettyb;
/*
 * set speed/echo/mode...
 *	tty 	-> terminal name
 *	spwant 	-> speed
 *	type	-> type
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 * return:  
 *	none
 */
/*ARGSUSED*/
GLOBAL void
fixline(tty, spwant, type)
int	tty, spwant, type;
{
	register struct sg_spds	*ps;
	struct termio		ttbuf;
	int			speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);
	if ((*Ioctl)(tty, TCGETA, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		if ( speed < 0 )
		    DEBUG(5, "speed (%d) not supported\n", spwant);
		ASSERT(speed >= 0, "BAD SPEED", "", spwant);
		ttbuf.c_cflag = (unsigned short) speed;
	} else { /* determine the current speed setting */
		ttbuf.c_cflag &= CBAUD;
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_name == ttbuf.c_cflag) {
				spwant = ps->sp_val;
				break;
			}
	}
	ttbuf.c_iflag = ttbuf.c_oflag = ttbuf.c_lflag = (ushort)0;

#ifdef NO_MODEM_CTRL
	/*   CLOCAL may cause problems on pdp11s with DHs */
	if (type == D_DIRECT) {
		DEBUG(4, "fixline - direct\n%s", "");
		ttbuf.c_cflag |= CLOCAL;
	} else
#endif /* NO_MODEM_CTRL */
		ttbuf.c_cflag &= ~CLOCAL;

	if ( EQUALS(Progname, "cu") ) {

		/* set attributes associated with -h, -t, -e, and -o options */

		ttbuf.c_iflag = (IGNPAR | IGNBRK | IXON | IXOFF);
		if ( line_8bit ) {
		    ttbuf.c_cflag |= CS8;
		    ttbuf.c_iflag &= ~ISTRIP;
		} else {
		    ttbuf.c_cflag |= CS7;
		    ttbuf.c_iflag |= ISTRIP;
		}

		ttbuf.c_cc[VEOF] = '\1';
		ttbuf.c_cflag |= ( CREAD | (speed ? HUPCL : 0));
	
		if (Evenflag) {				/*even parity -e */
		    if(ttbuf.c_cflag & PARENB) {
			VERBOSE(P_PARITY, 0);
			exit(1);
		    } else 
			ttbuf.c_cflag |= PARENB;
		} else if(Oddflag) {			/*odd parity -o */
		    if(ttbuf.c_cflag & PARENB) {
			VERBOSE(P_PARITY, 0);
			exit(1);
		    } else {
			ttbuf.c_cflag |= PARODD;
			ttbuf.c_cflag |= PARENB;
		    }
		}

		if(!Duplex)				/*half duplex -h */
		    ttbuf.c_iflag &= ~(IXON | IXOFF);
		if(Terminal)				/* -t */
		    ttbuf.c_oflag |= (OPOST | ONLCR);

	} else { /* non-cu */
		ttbuf.c_cflag |= (CS8 | CREAD | (speed ? HUPCL : 0));
		ttbuf.c_cc[VMIN] = HEADERSIZE;
		ttbuf.c_cc[VTIME] = 1;
	}

	donap = ( spwant > 0 && spwant < 4800 );
	
	ASSERT((*Ioctl)(tty, TCSETAW, &ttbuf) >= 0,
	    "RETURN FROM fixline ioctl", "", errno);
	return;
}

GLOBAL void
sethup(dcf)
int	dcf;
{
	struct termio ttbuf;

	if ((*Ioctl)(dcf, TCGETA, &ttbuf) != 0)
		return;
	if (!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		(void) (*Ioctl)(dcf, TCSETAW, &ttbuf);
	}
	return;
}

GLOBAL void
ttygenbrk(fn)
register int	fn;
{
	if (isatty(fn)) 
		(void) (*Ioctl)(fn, TCSBRK, 0);
	return;
}


/*
 * optimize line setting for sending or receiving files
 * return:
 *	none
 */
GLOBAL void
setline(type)
register char	type;
{
	static struct termio tbuf;
	int vtime;
	
	DEBUG(2, "setline - %c\n", type);
	if ((*Ioctl)(Ifn, TCGETA, &tbuf) != 0)
		return;
	switch (type) {
	case RCVFILE:
		switch (tbuf.c_cflag & CBAUD) {
		case B9600:
			vtime = 1;
			break;
		case B4800:
			vtime = 4;
			break;
		default:
			vtime = 8;
			break;
		}
		if (tbuf.c_cc[VMIN] != packsize || tbuf.c_cc[VTIME] != vtime) {
		    tbuf.c_cc[VMIN] = packsize;
		    tbuf.c_cc[VTIME] = vtime;
		    if ( (*Ioctl)(Ifn, TCSETAW, &tbuf) != 0 )
			DEBUG(4, "setline Ioctl failed errno=%d\n", errno);
		}
		break;

	case SNDFILE:
	case RESET:
		if (tbuf.c_cc[VMIN] != HEADERSIZE) {
		    tbuf.c_cc[VMIN] = HEADERSIZE;
		    if ( (*Ioctl)(Ifn, TCSETAW, &tbuf) != 0 )
			DEBUG(4, "setline Ioctl failed errno=%d\n", errno);
		}
		break;
	}
	return;
}

GLOBAL int
savline()
{
	if ( (*Ioctl)(0, TCGETA, &Savettyb) != 0 )
		Saved_line = FALSE;
	else {
		Saved_line = TRUE;
		Savettyb.c_cflag = (Savettyb.c_cflag & ~CS8) | CS7;
		Savettyb.c_oflag |= OPOST;
		Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	}
	return(0);
}

#ifdef SYTEK

/*
 *	sytfixline(tty, spwant)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	return codes:  none
 */

GLOBAL void
sytfixline(tty, spwant)
int tty, spwant;
{
	struct termio ttbuf;
	struct sg_spds *ps;
	int speed = -1;
	int ret;

	if ( (*Ioctl)(tty, TCGETA, &ttbuf) != 0 )
		return;
	for (ps = spds; ps->sp_val >= 0; ps++)
		if (ps->sp_val == spwant)
			speed = ps->sp_name;
	DEBUG(4, "sytfixline - speed= %d\n", speed);
	ASSERT(speed >= 0, "BAD SPEED", "", spwant);
	ttbuf.c_iflag = (ushort)0;
	ttbuf.c_oflag = (ushort)0;
	ttbuf.c_lflag = (ushort)0;
	ttbuf.c_cflag = speed;
	ttbuf.c_cflag |= (CS8|CLOCAL);
	ttbuf.c_cc[VMIN] = 6;
	ttbuf.c_cc[VTIME] = 1;
	ret = (*Ioctl)(tty, TCSETAW, &ttbuf);
	ASSERT(ret >= 0, "RETURN FROM sytfixline", "", ret);
	return;
}

GLOBAL void
sytfix2line(tty)
int tty;
{
	struct termio ttbuf;
	int ret;

	if ( (*Ioctl)(tty, TCGETA, &ttbuf) != 0 )
		return;
	ttbuf.c_cflag &= ~CLOCAL;
	ttbuf.c_cflag |= CREAD|HUPCL;
	ret = (*Ioctl)(tty, TCSETAW, &ttbuf);
	ASSERT(ret >= 0, "RETURN FROM sytfix2line", "", ret);
	return;
}

#endif /* SYTEK */

GLOBAL int
restline()
{
	if ( Saved_line == TRUE )
		return((*Ioctl)(0, TCSETAW, &Savettyb));
	return(0);
}

#else /* !ATTSV */

static struct sgttyb Savettyb;

/*
 *	fixline(tty, spwant, type)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 *	return codes:  none
 */

/*ARGSUSED*/
GLOBAL void
fixline(tty, spwant, type)
int tty, spwant, type;
{
	struct sgttyb	ttbuf;
	struct sg_spds	*ps;
	int		 speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);

	if ((*Ioctl)(tty, TIOCGETP, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, "BAD SPEED", "", spwant);
		ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
	} else {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_name == ttbuf.sg_ispeed) {
				spwant = ps->sp_val;
				break;
			}
		ASSERT(spwant >= 0, "BAD SPEED", "", ttbuf.sg_ispeed);
	}
	ttbuf.sg_flags = (ANYP | RAW);
	(void) (*Ioctl)(tty, TIOCSETP, &ttbuf);
	(void) (*Ioctl)(tty, TIOCHPCL, STBNULL);
	(void) (*Ioctl)(tty, TIOCEXCL, STBNULL);
	donap = ( spwant > 0 && spwant < 4800 );
	return;
}

GLOBAL void
sethup(dcf)
int	dcf;
{
	if (isatty(dcf)) 
		(void) (*Ioctl)(dcf, TIOCHPCL, STBNULL);
	return;
}

/*
 *	genbrk		send a break
 *
 *	return codes;  none
 */

GLOBAL void
ttygenbrk(fn)
{
	if (isatty(fn)) {
		(void) (*Ioctl)(fn, TIOCSBRK, 0);
#ifndef V8
		nap(HZ/10);				/* 0.1 second break */
		(void) (*Ioctl)(fn, TIOCCBRK, 0);
#endif
	}
	return;
}

/*
 * V7 and RT aren't smart enough for this -- linebaudrate is the best
 * they can do.
 */
/*ARGSUSED*/
GLOBAL void
setline(dummy) { }

GLOBAL int
savline()
{
	if (  (*Ioctl)(0, TIOCGETP, &Savettyb) != 0 ) {
		Saved_line = FALSE;
	else {
		Saved_line = TRUE;
		Savettyb.sg_flags |= ECHO;
		Savettyb.sg_flags &= ~RAW;
	}
	return(0);
}

GLOBAL int
restline()
{
	if ( Saved_line == TRUE )
		return((*Ioctl)(0, TIOCSETP, &Savettyb));
	return(0);
}
#endif
