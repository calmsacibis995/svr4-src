/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/window/ioctl.h	1.1"
/*	@(#)curses:demo/window/ioctl.h	1.1	(CBOSG) 6/1/85	*/
/*	ioctl.h	4.7	81/03/17	*/
/*
 * ioctl definitions, and special character and local tty definitions
 */
#ifndef	_IOCTL_
#define	_IOCTL_
struct tchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};
struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};

/*
 * local mode settings
 */
#define	LCRTBS	0000001		/* correct backspacing for crt */
#define	LPRTERA 0000002		/* printing terminal \ ... / erase */
#define	LCRTERA	0000004		/* do " \b " to wipe out character */
#define	LTILDE	0000010		/* IIASA - hazeltine tilde kludge */
#define	LMDMBUF	0000020		/* IIASA - start/stop output on carrier intr */
#define	LLITOUT	0000040		/* IIASA - suppress any output translations */
#define	LTOSTOP	0000100		/* send stop for any background tty output */
#define	LFLUSHO	0000200		/* flush output sent to terminal */
#define	LNOHANG 0000400		/* IIASA - don't send hangup on carrier drop */
#define	LETXACK 0001000		/* IIASA - diablo style buffer hacking */
#define	LCRTKIL	0002000		/* erase whole line on kill with " \b " */
#define	LINTRUP 0004000		/* interrupt on every input char - SIGTINT */
#define	LCTLECH	0010000		/* echo control characters as ^X */
#define	LPENDIN	0020000		/* tp->t_rawq is waiting to be reread */
#define	LDECCTQ 0040000		/* only ^Q starts after ^S */

/* local state */
#define	LSBKSL	01		/* state bit for lowercase backslash work */
#define	LSQUOT	02		/* last character input was \ */
#define	LSERASE	04		/* within a \.../ for LPRTRUB */
#define	LSLNCH	010		/* next character is literal */
#define	LSTYPEN	020		/* retyping suspended input (LPENDIN) */
#define	LSCNTTB	040		/* counting width of tab; leave LFLUSHO alone */

/*
 * tty ioctl commands
 */
#define	TIOCGETD	(('t'<<8)|0)	/* get line discipline */
#define	TIOCSETD	(('t'<<8)|1)	/* set line discipline */
#define	TIOCHPCL	(('t'<<8)|2)	/* set hangup line on close bit */
#define	TIOCMODG	(('t'<<8)|3)	/* modem bits get (???) */
#define	TIOCMODS	(('t'<<8)|4)	/* modem bits set (???) */
#define	TIOCGETP	(('t'<<8)|8)	/* get parameters - like old gtty */
#define	TIOCSETP	(('t'<<8)|9)	/* set parameters - like old stty */
#define	TIOCSETN	(('t'<<8)|10)	/* set params w/o flushing buffers */
#define	TIOCEXCL	(('t'<<8)|13)	/* set exclusive use of tty */
#define	TIOCNXCL	(('t'<<8)|14)	/* reset exclusive use of tty */
#define	TIOCFLUSH	(('t'<<8)|16)	/* flush buffers */
#define	TIOCSETC	(('t'<<8)|17)	/* set special characters */
#define	TIOCGETC	(('t'<<8)|18)	/* get special characters */
#define	TIOCIOANS	(('t'<<8)|20)
#define	TIOCSIGNAL	(('t'<<8)|21)
#define	TIOCUTTY	(('t'<<8)|22)
/* locals, from 127 down */
#define	TIOCLBIS	(('t'<<8)|127)	/* bis local mode bits */
#define	TIOCLBIC	(('t'<<8)|126)	/* bic local mode bits */
#define	TIOCLSET	(('t'<<8)|125)	/* set entire local mode word */
#define	TIOCLGET	(('t'<<8)|124)	/* get local modes */
#define	TIOCSBRK	(('t'<<8)|123)	/* set break bit */
#define	TIOCCBRK	(('t'<<8)|122)	/* clear break bit */
#define	TIOCSDTR	(('t'<<8)|121)	/* set data terminal ready */
#define	TIOCCDTR	(('t'<<8)|120)	/* clear data terminal ready */
#define	TIOCGPGRP	(('t'<<8)|119)	/* get pgrp of tty */
#define	TIOCSPGRP	(('t'<<8)|118)	/* set pgrp of tty */
#define	TIOCSLTC	(('t'<<8)|117)	/* set local special characters */
#define	TIOCGLTC	(('t'<<8)|116)	/* get local special characters */
#define	TIOCOUTQ	(('t'<<8)|115)	/* number of chars in output queue */
#define	TIOCSTI		(('t'<<8)|114)	/* simulate a terminal in character */

#ifdef CCA_PAGE
#define TIOCSSCR	(('t'<<8)|64)	/* Set screen length */
#define TIOCGSCR	(('t'<<8)|65)	/* Get screen length */
#endif
#define TIOCLTTY	(('t'<<8)|63)	/* clear controlling tty */

#define	OTTYDISC	0		/* old, v7 std tty driver */
#define	NETLDISC	1		/* line discip for berk net */
#define	NTTYDISC	2		/* new tty discipline */
#define	PKDISC		3		/* packet driver */
#define	TRDISC		4		/* datakit trailer protocol */
#define	TDKDISC		5		/* datakit terminal protocol */
#define	TRBDISC		6		/* datakit block trailer protocol */

#define	FIOCLEX		(('f'<<8)|1)
#define	FIONCLEX	(('f'<<8)|2)
#define	FIOPUSHLD	(('f'<<8)|3)
#define	FIOPOPLD	(('f'<<8)|4)
/* another local */
#define	FIONREAD	(('f'<<8)|127)	/* get # bytes to read */

/* fast timer ioctls */
#define FTIOCSET	(('T'<<8)|0)
#define FTIOCCANCEL	(('T'<<8)|1)
#endif

#define	DIOCLSTN	(('d'<<8)|1)
#define	DIOCMD		(('d'<<8)|2)
#define	DIOCNTRL		(('d'<<8)|2)
#define	DIOCMPX		(('d'<<8)|3)
#define	DIOCNMPX	(('d'<<8)|4)
#define	DIOCSCALL	(('d'<<8)|5)
#define	DIOCRCALL	(('d'<<8)|6)
#define	DIOCPGRP	(('d'<<8)|7)
#define	DIOCGETP	(('d'<<8)|8)
#define	DIOCSETP	(('d'<<8)|9)
#define	DIOCLOSE	(('d'<<8)|10)
#define	DIOCTIME	(('d'<<8)|11)
#define	DIOCRESET	(('d'<<8)|12)
#define	DIOCSMETA	(('d'<<8)|13)
#define	DIOCMERGE	(('d'<<8)|14)
#define	DIOCICHAN	(('d'<<8)|15)
#define	DIOCUMERGE	(('d'<<8)|16)
#define	DIOCRMETA	(('d'<<8)|17)
#define	DIOCXOUT	(('d'<<8)|18)
#define	DIOCBMETA	(('d'<<8)|19)
#define	DIOCAMETA	(('d'<<8)|20)
#define	DIOCSBMETA	(('d'<<8)|21)
#define	DIOCLOOP	(('d'<<8)|22)
#define	DIOCPROTOCOL	(('d'<<8)|23)
#define	DIOCTRL		(('d'<<8)|24)
#define	DIOCDMETA	(('d'<<8)|25)
#define	DIOCSWR		(('d'<<8)|26)


#define	MXLSTN		(('x'<<8)|1)
#define	MXNBLK		(('x'<<8)|2)
#define MXAUTOBLK	(('x'<<8)|3)

/*
 * The WINDOW manager uses codes compatible with the Blit.
 * They MUST be in /usr/include/sys/ioctl.h, and the JSWINSIZE
 * ioctl must be added to the standard Blit ioctls.
 */

/*
** Jerq I/O control codes
*/

#define	JTYPE		('j'<<8)
#define	JBOOT		(JTYPE|1)
#define	JTERM		(JTYPE|2)
#define	JMPX		(JTYPE|3)
#define	JTIMO		(JTYPE|4)
#define	JWINSIZE	(JTYPE|5)
#define	JTIMOM		(JTYPE|6)
#define	JSWINSIZE	(JTYPE|7)

struct winsize
{
	char	bytesx, bytesy;
	short	bitsx, bitsy;
};

/* End of WINDOW additions */
