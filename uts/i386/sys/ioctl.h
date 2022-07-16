/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#ident	"@(#)head.sys:sys/ioctl.h	11.8.3.1"
/*
 *      There are some inherent problems in having a single file
 *      ioctl.h, with both System V and BSD flags. Introducing
 *  BSD flags into this file creates compilation problems
 *  with flags such as ECHO, NL1 etc., if termio.h and ioctl.h
 *      are included by the same file. Since these two files can
 *  be only included by System V applications, /usr/inclule/sys/ioctl.h
 *      will be System V mode and all the BSD flags will be turned off
 *      using #ifdef BSD_COMP. This file will also exist in
 *  /usr/ucbinclude/sys/ioctl.h for BSD applications but without the
 *      BSD flags turned off. System V appliactions can use ioctl.h without
 *      any changes, System V applications requiring BSD flags should
 *      -D BSD_COMP when compiling (and be warned about the common
 *      flags between System V and BSD) and BSD applications should
 *  use /usr/ucbinclude/sys/ioctl.h.
 *
 */

/*
 *	Ioctl commands
 */


/* BSD related defines */

#ifdef BSD_COMP

#include <sys/ttychars.h>
#include <sys/ttydev.h>
#include <sys/ttold.h>


#define	TANDEM		O_TANDEM
#define	CBREAK		O_CBREAK
#ifndef _SGTTY_H
#define	LCASE		O_LCASE
#define	ECHO		O_ECHO
#define	CRMOD		O_CRMOD
#define	RAW		O_RAW
#define	ODDP		O_ODDP
#define	EVENP		O_EVENP
#define	ANYP		O_ANYP
#define	NLDELAY		O_NLDELAY
#define		NL0		O_NL0
#define		NL1		O_NL1
#define		NL2		O_NL2
#define		NL3		O_NL3
#define	TBDELAY		O_TBDELAY
#define		TAB0		O_TAB0
#define		TAB1		O_TAB1
#define		TAB2		O_TAB2
#define	XTABS		O_XTABS
#define	CRDELAY		O_CRDELAY
#define		CR0		O_CR0
#define		CR1		O_CR1
#define		CR2		O_CR2
#define		CR3		O_CR3
#define	VTDELAY		O_VTDELAY
#define		FF0		O_FF0
#define		FF1		O_FF1
#define	BSDELAY		O_BSDELAY
#define		BS0		O_BS0
#define		BS1		O_BS1
#define 	ALLDELAY	O_ALLDELAY
#endif /* _SGTTY_H */
#define	CRTBS		O_CRTBS
#define	PRTERA		O_PRTERA
#define	CRTERA		O_CRTERA
#define	TILDE		O_TILDE
#define	MDMBUF		O_MDMBUF
#define	LITOUT		O_LITOUT
#define	TOSTOP		O_TOSTOP
#define	FLUSHO		O_FLUSHO
#define	NOHANG		O_NOHANG
#define	L001000		O_L001000
#define	CRTKIL		O_CRTKIL
#define	PASS8		O_PASS8
#define	CTLECH		O_CTLECH
#define	PENDIN		O_PENDIN
#define	DECCTQ		O_DECCTQ
#define	NOFLSH		O_NOFLSH

#include <sys/filio.h>
#include <sys/sockio.h>

#endif /* BSD_COMP */


/*
**	Union for use by all device handler ioctl routines.
*/
union ioctl_arg {
	struct termio	*stparg;	/* ptr to termio struct */
	struct Generic	*sparg;		/* ptr to generic struct */
	char		*cparg;		/* ptr to character */
	char		carg;		/* character */
	int		*iparg;		/* ptr to integer */
	int		iarg;		/* integer */
	long            *lparg;         /* ptr to long */
	long            larg;           /* long */
};

/*
 * Commands needed for XENIX ioctl() compatibility
 */

#define	TIOC	('T'<<8)
#define	TCFLSH	(TIOC|7)

#endif	/* _SYS_IOCTL_H */
