/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef _SYS_TERMIO_H
#define _SYS_TERMIO_H

#ident	"@(#)head.sys:sys/termio.h	11.12.3.1"

#include <sys/termios.h>


/* all the ioctl codes and flags are now in termios.h */

/*
 * Ioctl control packet
 */
struct termio {
	unsigned short	c_iflag;	/* input modes */
	unsigned short	c_oflag;	/* output modes */
	unsigned short	c_cflag;	/* control modes */
	unsigned short	c_lflag;	/* line discipline modes */
	char	c_line;		/* line discipline */
	unsigned char	c_cc[NCC];	/* control chars */
};

#define	IOCTYPE	0xff00


/*
 * structure of ioctl arg for LDGETT and LDSETT
 */
struct	termcb	{
	char	st_flgs;	/* term flags */
	char	st_termt;	/* term type */
	char	st_crow;	/* gtty only - current row */
	char	st_ccol;	/* gtty only - current col */
	char	st_vrow;	/* variable row */
	char	st_lrow;	/* last row */
};

#ifndef u3b15
#define	SSPEED	7	/* default speed: 300 baud */
#else
#define SSPEED	9	/* default speed: 1200 baud */
#endif

#ifdef u3b15
#define TTYTYPE (TIOC|8)
#endif
#define	TCDSET	(TIOC|32)

/*
 * Terminal types
 */
#define	TERM_NONE	0	/* tty */
#define	TERM_TEC	1	/* TEC Scope */
#define	TERM_V61	2	/* DEC VT61 */
#define	TERM_V10	3	/* DEC VT100 */
#define	TERM_TEX	4	/* Tektronix 4023 */
#define	TERM_D40	5	/* TTY Mod 40/1 */
#define	TERM_H45	6	/* Hewlitt-Packard 45 */
#define	TERM_D42	7	/* TTY Mod 40/2B */

/*
 * Terminal flags
 */
#define TM_NONE		0000	/* use default flags */
#define TM_SNL		0001	/* special newline flag */
#define TM_ANL		0002	/* auto newline on column 80 */
#define TM_LCF		0004	/* last col of last row special */
#define TM_CECHO	0010	/* echo terminal cursor control */
#define TM_CINVIS	0020	/* do not send esc seq to user */
#define TM_SET		0200	/* must be on to set/res flags */

/*
 * structure of ioctl arg for AIOCSETSS (defined is asy.h)
 */

struct	termss {
	char	ss_start; 	/* output start char */
	char 	ss_stop;	/* output stop char */
};


#endif	/* _SYS_TERMIO_H */
