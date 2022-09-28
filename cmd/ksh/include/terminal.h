/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/terminal.h	1.3.3.1"

/*
 * terminal interface
 */

#ifndef _terminal_
#define _terminal_	1
#ifdef _termios_
#   include	<termios.h>
#else
#   ifdef _sys_termios_
#	include	<sys/termios.h>
#	define _termios_
#   endif /* _sys_termios_ */
#endif /* _termios_ */

#ifdef _termios_
#   ifndef TCSANOW
#	define TCSANOW		TCSETS
#	define TCSADRAIN	TCSETSW
#	define TCSAFLUSH	TCSETSF
#	define tcgetattr(fd,tty)	ioctl(fd, TCGETS, tty)
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#   endif /* TCSANOW */
#   undef TIOCGETC
#   undef _termio_
#   undef _sys_termio_
#   undef _sgtty_
#   undef _sys_ioctl_
#   undef _sys_bsdtty_
#endif /* _termios_ */

#ifdef _termio_
#   include	<termio.h>
#else
#   ifdef _sys_termio_
#	include	<sys/termio.h>
#   define _termio_ 1
#   endif /* _sys_termio_ */
#endif /* _termio_ */
#ifdef _termio_
#   define termios termio
#   undef _sgtty_
#   undef TIOCGETC
#   undef _sys_ioctl_
#   define tcgetattr(fd,tty)		ioctl(fd, TCGETA, tty)
#   define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#endif /* _termio_ */

#ifdef _sys_bsdtty_
#   include	<sys/bsdtty.h>
#endif /* _sys_bsdtty_ */

#ifdef _sgtty_
#   include	<sgtty.h>
#   ifdef _sys_nttyio_
#	ifndef LPENDIN
#	    include	<sys/nttyio.h>
#	endif /* LPENDIN */
#   endif /* _sys_nttyio_ */
#   ifdef _sys_filio_
#	ifndef FIONREAD
#	    include	<sys/filio.h>
#	endif /* FIONREAD */
#   endif /* _sys_filio_ */
#   define termios sgttyb
#   undef _sys_ioctl_
#   ifdef TIOCSETN
#	undef TCSETAW
#   endif /* TIOCSETN */
#   ifdef _SELECT_
#	define included_sys_time_
#	include	<sys/time.h>
	extern const int tty_speeds[];
#   endif /* _SELECT_ */
#   ifdef TIOCGETP
#	define tcgetattr(fd,tty)		ioctl(fd, TIOCGETP, tty)
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#   else
#	define tcgetattr(fd,tty)	gtty(fd, tty)
#	define tcsetattr(fd,action,tty)	stty(fd, tty)
#   endif /* TIOCGETP */
#endif /* _sgtty_ */

#ifndef TCSANOW
#   ifdef TCSETAW
#	define TCSANOW	TCSETA
#	ifdef u370
	/* delays are too long, don't wait for output to drain */
#	    define TCSADRAIN	TCSETA
#	else
#	   define TCSADRAIN	TCSETAW
#	endif /* u370 */
#	define TCSAFLUSH	TCSETAF
#   else
#	ifdef TIOCSETN
#	    define TCSANOW	TIOCSETN
#	    define TCSADRAIN	TIOCSETN
#	    define TCSAFLUSH	TIOCSETP
#	endif /* TIOCSETN */
#   endif /* TCSETAW */
#endif /* TCSANOW */
#endif /* _terminal_ */

/* set ECHOCTL if driver can echo control charaters as ^c */
#ifdef LCTLECH
#   ifndef ECHOCTL
#	define ECHOCTL	LCTLECH
#   endif /* !ECHOCTL */
#endif /* LCTLECH */

/* set FIORDCHK if you can check for characters in input queue */
#ifdef FIONREAD
#   ifndef FIORDCHK
#	define FIORDCHK	FIONREAD
#   endif /* !FIORDCHK */
#endif /* FIONREAD */

#ifdef PROTO
    extern int	tty_alt(int);
    extern void tty_cooked(int);
    extern int	tty_get(int,struct termios*);
    extern int	tty_raw(int);
    extern int	tty_check(int);
#else
    extern int	tty_alt();
    extern void tty_cooked();
    extern int	tty_get();
    extern int	tty_raw();
    extern int	tty_check();
#endif /* PROTO */
extern int	tty_set();
