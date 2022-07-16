/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ansi.h	1.1.2.1"

#ifndef	_SYS_ANSI_H
#define	_SYS_ANSI_H
/*
 * definitions for Integrated Workstation Environment ANSI x3.64 
 * terminal control language parser 
 */

#define ANSI_MAXPARAMS	5	/* maximum number of ANSI paramters */
#define ANSI_MAXTAB	40	/* maximum number of tab stops */
#define ANSI_MAXFKEY	30	/* max length of function key with <ESC>Q */


#define	ANSIPSZ		64	/* max packet size sent by ANSI */

/*
 * Font values for ansistate
 */
#define	ANSI_FONT0	0	/* Primary font (default) */
#define	ANSI_FONT1	1	/* First alternate font */
#define	ANSI_FONT2	2	/* Second alternate font */

#define	ANSI_BLKOUT	0x8000	/* Scroll lock, for M_START, M_STOP */

struct ansi_state {		/* state for ansi x3.64 emulator */
	ushort	a_flags;	/* flags for this x3.64 terminal */
	unchar	a_font;		/* font type */
	unchar	a_state;	/* state in output esc seq processing */
	unchar	a_gotparam;	/* does output esc seq have a param */
	ushort	a_curparam;	/* current param # of output esc seq */
	ushort	a_paramval;	/* value of current param */
	short	a_params[ANSI_MAXPARAMS];  /* parameters of output esc seq */
	char	a_fkey[ANSI_MAXFKEY];	/* work space for function key */
	mblk_t	*a_wmsg;	/* ptr to data message being assembled */
	queue_t	*a_wqp;		/* ptr to write queue for associated stream */
	queue_t	*a_rqp;		/* ptr to read queue for associated stream */
};


typedef struct ansi_state ansistat_t;

#endif /* _SYS_ANSI_H */
