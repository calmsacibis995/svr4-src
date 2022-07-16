/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TERMIOS_H
#define _SYS_TERMIOS_H

#ident	"@(#)head.sys:sys/termios.h	1.31.4.1"

#if !defined(_POSIX_SOURCE) 
#include <sys/ttydev.h>
#endif /* !defined(_POSIX_SOURCE) */ 

#include <sys/types.h>

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0 /* Disable special character functions */
#endif

#if !defined(_POSIX_SOURCE) 
#define	CTRL(c)	((c)&037)
#define IBSHIFT 16

/* required by termio.h and VCEOF/VCEOL */
#define	NCC	8
#endif /* !defined(_POSIX_SOURCE) */ 

/* some defines required by POSIX */
#define	NCCS	19

/*
 * types defined by POSIX. These are better off in types.h, but 
 * the standard says that they have to be in termios.h.
 */
typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
typedef unsigned long speed_t;

/*
 * Ioctl control packet
 */
struct termios {
	tcflag_t	c_iflag;	/* input modes */
	tcflag_t	c_oflag;	/* output modes */
	tcflag_t	c_cflag;	/* control modes */
	tcflag_t	c_lflag;	/* line discipline modes */
	cc_t		c_cc[NCCS];	/* control chars */
};

/* 
 * POSIX termios functions
 * These functions get mapped into ioctls.
 */

#ifndef _KERNEL

#if defined(__STDC__)

extern speed_t cfgetospeed (const struct termios *);
extern int cfsetospeed (struct termios *, speed_t);
extern speed_t cfgetispeed (const struct termios *);
extern int cfsetispeed (struct termios *, speed_t);
extern int tcgetattr (int, struct termios *);
extern int tcsetattr (int, int, const struct termios *);
extern int tcsendbreak (int, int);
extern int tcdrain (int);
extern int tcflush (int, int);
extern int tcflow (int, int);

#else

extern speed_t cfgetospeed ();
extern int cfsetospeed ();
extern speed_t cfgetispeed ();
extern int cfsetispeed ();
extern int tcgetattr ();
extern int tcsetattr ();
extern int tcsendbreak ();
extern int tcdrain ();
extern int tcflush ();
extern int tcflow ();

#endif /* __STDC__ */

#if !defined(_POSIX_SOURCE) 

#if defined(__STDC__)
extern pid_t tcgetsid (int);
#else
extern pid_t tcgetsid ();
#endif /* __STDC__ */

#endif /* !defined(_POSIX_SOURCE) */ 

#endif

/* control characters */
#define	VINTR	0
#define	VQUIT	1
#define	VERASE	2
#define	VKILL	3
#define	VEOF	4
#define	VEOL	5
#if !defined(_POSIX_SOURCE) 
#define	VEOL2	6
#endif /* !defined(_POSIX_SOURCE) */ 
#define	VMIN	4
#define	VTIME	5
#if !defined(_POSIX_SOURCE) 
#define	VSWTCH	7
#endif /* !defined(_POSIX_SOURCE) */ 
#define	VSTART		8
#define	VSTOP		9
#define	VSUSP		10
#if !defined(_POSIX_SOURCE) 
#define	VDSUSP		11
#define	VREPRINT	12
#define	VDISCARD	13
#define	VWERASE		14
#define	VLNEXT		15
/* 16 thru 19 reserved for future use */

/*
 * control characters form Xenix termio.h
 */
#define	VCEOF	NCC		/* RESERVED true EOF char (V7 compatability) */
#define	VCEOL	(NCC + 1)	/* RESERVED true EOL char */

#define	CNUL	0
#define	CDEL	0377

/* S5 default control chars */
#define	CESC	'\\'
#define	CINTR	0177	/* DEL */
#define	CQUIT	034	/* FS, cntl | */
#define	CERASE	'#'
#define	CKILL	'@'
#define CEOT	04
#define CEOL	0
#define CEOL2	0
#define	CEOF	04	/* cntl d */
#define	CSTART	021	/* cntl q */
#define	CSTOP	023	/* cntl s */
#define	CSWTCH	032	/* cntl z */
#define	CNSWTCH	0
#define	CSUSP	CTRL('z')
#define	CDSUSP	CTRL('y')
#define	CRPRNT	CTRL('r')
#define	CFLUSH	CTRL('o')
#define	CWERASE	CTRL('w')
#define	CLNEXT	CTRL('v')
#endif /* !defined(_POSIX_SOURCE) */ 


/* input modes */
#define	IGNBRK	0000001
#define	BRKINT	0000002
#define	IGNPAR	0000004
#define	PARMRK	0000010
#define	INPCK	0000020
#define	ISTRIP	0000040
#define	INLCR	0000100
#define	IGNCR	0000200
#define	ICRNL	0000400
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	IUCLC	0001000
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */ 
#define	IXON	0002000
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	IXANY	0004000
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */ 
#define	IXOFF	0010000
#if !defined(_POSIX_SOURCE) 
#define IMAXBEL 0020000
#define DOSMODE	0100000  /* for 386 compatibility */
#endif /* !defined(_POSIX_SOURCE) */ 

/* output modes */
#define	OPOST	0000001
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	OLCUC	0000002
#define	ONLCR	0000004
#define	OCRNL	0000010
#define	ONOCR	0000020
#define	ONLRET	0000040
#define	OFILL	0000100
#define	OFDEL	0000200
#define	NLDLY	0000400
#define	NL0	0
#define	NL1	0000400
#define	CRDLY	0003000
#define	CR0	0
#define	CR1	0001000
#define	CR2	0002000
#define	CR3	0003000
#define	TABDLY	0014000
#define	TAB0	0
#define	TAB1	0004000
#define	TAB2	0010000
#define	TAB3	0014000
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */ 
#if !defined(_POSIX_SOURCE) 
#define XTABS	0014000
#endif /* !defined(_POSIX_SOURCE) */ 
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	BSDLY	0020000
#define	BS0	0
#define	BS1	0020000
#define	VTDLY	0040000
#define	VT0	0
#define	VT1	0040000
#define	FFDLY	0100000
#define	FF0	0
#define	FF1	0100000
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */ 
#if !defined(_POSIX_SOURCE) 
#define PAGEOUT 0200000
#define WRAP	0400000

/* control modes */
#define	CBAUD	0000017
#endif /* !defined(_POSIX_SOURCE) */ 
#define	CSIZE	0000060
#define	CS5	0
#define	CS6	0000020
#define	CS7	0000040
#define	CS8	0000060
#define	CSTOPB	0000100
#define	CREAD	0000200
#define	PARENB	0000400
#define	PARODD	0001000
#define	HUPCL	0002000
#define	CLOCAL	0004000
#if !defined(_POSIX_SOURCE) 
#define RCV1EN	0010000
#define	XMT1EN	0020000
#define	LOBLK	0040000
#define	XCLUDE	0100000		/* *V7* exclusive use coming fron XENIX */
#define CIBAUD	03600000
#define PAREXT	04000000
#endif /* !defined(_POSIX_SOURCE) */ 

/* line discipline 0 modes */
#define	ISIG	0000001
#define	ICANON	0000002
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	XCASE	0000004
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */ 
#define	ECHO	0000010
#define	ECHOE	0000020
#define	ECHOK	0000040
#define	ECHONL	0000100
#define	NOFLSH	0000200
#define	TOSTOP	0000400
#if !defined(_POSIX_SOURCE) 
#define	ECHOCTL	0001000
#define	ECHOPRT	0002000
#define	ECHOKE	0004000
#define	DEFECHO	0010000
#define	FLUSHO	0020000
#define	PENDIN	0040000
#endif /* !defined(_POSIX_SOURCE) */ 
#define	IEXTEN	0100000  /* POSIX flag - enable POSIX extensions */

#if !defined(_POSIX_SOURCE) 
#define	TIOC	('T'<<8)

#define	TCGETA	(TIOC|1)
#define	TCSETA	(TIOC|2)
#define	TCSETAW	(TIOC|3)
#define	TCSETAF	(TIOC|4)
#define	TCSBRK	(TIOC|5)
#define	TCXONC	(TIOC|6)
#define	TCFLSH	(TIOC|7)

/* Slots reserved for 386/XENIX compatibility - keyboard control */

#ifndef _KB_386 /* These are also defined in kd.h   */
#define _KB_386
#define TIOCKBON	(TIOC|8)
#define TIOCKBOF 	(TIOC|9)
#define KBENABLED 	(TIOC|10)
#endif  /* _KB_386 */

#ifndef IOCTYPE
#define	IOCTYPE	0xff00
#endif

#define	TCDSET	(TIOC|32)
#define	RTS_TOG	(TIOC|33)	/* 386 - "RTS" toggle define 8A1 protocol */


/* MERGE386 scancode input flag bits */
#define KB_RESERVED1    1
#define KB_RESERVED2    2
#define KB_XSCANCODE    4   /* Translate scancode to ascii */
#define KB_ISSCANCODE   8   /* TTY sends scancodes */
/* OBSOLETE MERGE386 DEFINES -- HERE JUST INCASE FILES ACCESS THEM */


#define TIOCGWINSZ (TIOC|104)
#define TIOCSWINSZ (TIOC|103)


/* termios ioctls */

#define TCGETS		(TIOC|13)
#define TCSETS		(TIOC|14)
#endif /* !defined(_POSIX_SOURCE) */ 
#define TCSANOW		(('T'<<8)|14) /* same as TCSETS */
#if !defined(_POSIX_SOURCE) 
#define TCSETSW		(TIOC|15)
#endif /* !defined(_POSIX_SOURCE) */ 
#define TCSADRAIN	(('T'<<8)|15) /* same as TCSETSW */
#if !defined(_POSIX_SOURCE) 
#define	TCSETSF		(TIOC|16)
#endif /* !defined(_POSIX_SOURCE) */ 
#define	TCSAFLUSH	(('T'<<8)|16) /* same as TCSETSF */

/* termios option flags */

#define TCIFLUSH	0  /* flush data received but not read */
#define TCOFLUSH	1  /* flush data written but not transmitted */
#define TCIOFLUSH	2  /* flush both data both input and output queues */	

#define TCOOFF		0  /* suspend output */
#define TCOON		1  /* restart suspended output */
#define TCIOFF		2  /* suspend input */
#define TCION		3  /* restart suspended input */

/* TIOC ioctls for BSD, ptys, job control and modem control */

#if !defined(_POSIX_SOURCE) 
#define	tIOC	('t'<<8)
#endif /* !defined(_POSIX_SOURCE) */ 


/* Slots for 386/XENIX compatibility */
/* BSD includes these ioctls in ttold.h */

#ifndef _SYS_TTOLD_H

#if !defined(_POSIX_SOURCE) 
#define TIOCGETD	(tIOC|0)
#define TIOCSETD	(tIOC|1)
#define TIOCHPCL	(tIOC|2)
#define TIOCGETP	(tIOC|8)
#define TIOCSETP  	(tIOC|9)
#define TIOCSETN	(tIOC|10)
#define TIOCEXCL	(tIOC|13)
#define TIOCNXCL	(tIOC|14)
#define TIOCFLUSH	(tIOC|16)
#define TIOCSETC	(tIOC|17)
#define TIOCGETC	(tIOC|18)
/*
 * BSD ioctls that are not the same as XENIX are included here.
 * There are also some relevant ioctls from SUN/BSD sys/ttycom.h
 * BSD pty ioctls like TIOCPKT are not supported in SVR4.
 */

#define	TIOCLBIS	(tIOC|127)	/* bis local mode bits */
#define	TIOCLBIC	(tIOC|126)	/* bic local mode bits */
#define	TIOCLSET	(tIOC|125)	/* set entire local mode word */
#define	TIOCLGET	(tIOC|124)	/* get local modes */
#define	TIOCSBRK	(tIOC|123)	/* set break bit */
#define	TIOCCBRK	(tIOC|122)	/* clear break bit */
#define	TIOCSDTR	(tIOC|121)	/* set data terminal ready */
#define	TIOCCDTR	(tIOC|120)	/* clear data terminal ready */
#define	TIOCSLTC	(tIOC|117)	/* set local special chars */
#define	TIOCGLTC	(tIOC|116)	/* get local special chars */
#define	TIOCOUTQ	(tIOC|115)	/* driver output queue size */
#define	TIOCNOTTY	(tIOC|113)	/* void tty association */
#define	TIOCSTOP	(tIOC|111)	/* stop output, like ^S */
#define	TIOCSTART	(tIOC|110)	/* start output, like ^Q */
#endif /* !defined(_POSIX_SOURCE) */ 

#endif /* end _SYS_TTOLD_H */

/* POSIX job control ioctls */

#if !defined(_POSIX_SOURCE) 
#define	TIOCGPGRP	(tIOC|20)	/* get pgrp of tty */
#define	TIOCSPGRP	(tIOC|21)	/* set pgrp of tty */
#define	TIOCGSID	(tIOC|22)	/* get session id on ctty*/
#define	TIOCSSID	(tIOC|24)	/* set session id on ctty*/

/* Miscellanous */
#define	TIOCSTI		(tIOC|23)	/* simulate terminal input */

/* Modem control */
#define	TIOCMSET	(tIOC|26)	/* set all modem bits */
#define	TIOCMBIS	(tIOC|27)	/* bis modem bits */
#define	TIOCMBIC	(tIOC|28)	/* bic modem bits */
#define	TIOCMGET	(tIOC|29)	/* get all modem bits */
#define		TIOCM_LE	0001		/* line enable */
#define		TIOCM_DTR	0002		/* data terminal ready */
#define		TIOCM_RTS	0004		/* request to send */
#define		TIOCM_ST	0010		/* secondary transmit */
#define		TIOCM_SR	0020		/* secondary receive */
#define		TIOCM_CTS	0040		/* clear to send */
#define		TIOCM_CAR	0100		/* carrier detect */
#define		TIOCM_CD	TIOCM_CAR
#define		TIOCM_RNG	0200		/* ring */
#define		TIOCM_RI	TIOCM_RNG
#define		TIOCM_DSR	0400		/* data set ready */

/* pseudo-tty */

#define	TIOCREMOTE	(tIOC|30)	/* remote input editing */
#define TIOCSIGNAL	(tIOC|31)	/* pty: send signal to slave */


/* Some more 386 xenix stuff */

#define	LDIOC	('D'<<8)

#define	LDOPEN	(LDIOC|0)
#define	LDCLOSE	(LDIOC|1)
#define	LDCHG	(LDIOC|2)
#define	LDGETT	(LDIOC|8)
#define	LDSETT	(LDIOC|9)

/* Slots for 386 compatibility */

#define	LDSMAP		(LDIOC|10)
#define	LDGMAP		(LDIOC|11)
#define	LDNMAP		(LDIOC|12)

/*
 * These are retained for 386/XENIX compatibility.
 */

#define	DIOC	('d'<<8)
#define DIOCGETP        (DIOC|8)                /* V7 */
#define DIOCSETP        (DIOC|9)                /* V7 */

/*
 * Returns a non-zero value if there
 * are characters in the input queue.
 */
#define FIORDCHK        (('f'<<8)|3)            /* V7 */
#endif /* !defined(_POSIX_SOURCE) */ 

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define B19200	14
#define B38400	15

#ifndef _SYS_TTOLD_H
#ifndef _SYS_PTEM_H

#if !defined(_POSIX_SOURCE) 
/* Windowing structure to support JWINSIZE/TIOCSWINSZ/TIOCGWINSZ */
struct winsize {
	unsigned short ws_row;       /* rows, in characters*/
	unsigned short ws_col;       /* columns, in character */
	unsigned short ws_xpixel;    /* horizontal size, pixels */
	unsigned short ws_ypixel;    /* vertical size, pixels */
};
#endif /* !defined(_POSIX_SOURCE) */ 

#endif /* end _SYS_PTEM_H */
#endif /* end _SYS_TTOLD_H */

#endif	/* _SYS_TERMIOS_H */
