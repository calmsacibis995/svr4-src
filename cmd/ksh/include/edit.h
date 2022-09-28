/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/edit.h	1.2.3.1"
/*
 *  edit.h -  common data structure for vi and emacs edit options
 *
 *   David Korn
 *   AT&T Bell Laboratories
 *   Room 3C-526B
 *   Murray Hill, N. J. 07974
 *   Tel. x7975
 *
 */

#ifndef KSHELL
#   include	<setjmp.h>
#   include	<signal.h>
#   include	<ctype.h>
#endif /* KSHELL */
#define LOOKAHEAD	80
#ifdef VENIX
#   define READAHEAD	1
#else
#   define READAHEAD	LOOKAHEAD
#endif	/* VENIX */

#ifdef MULTIBYTE
#   ifndef ESS_MAXCHAR
#   include	"national.h"
#   endif /* ESS_MAXCHAR */
#   if ESS_MAXCHAR<=2
	typedef unsigned short genchar;
#   else
	typedef long genchar;
#   endif
#   define CHARSIZE	2
#else
    typedef char genchar;
#   define CHARSIZE	1
#endif /* MULTIBYTE */

#define TABSIZE	8
#define PRSIZE	80

struct edit
{
	int	e_kill;
	int	e_erase;
	int	e_eof;
	int	e_fchar;
	char	e_plen;		/* length of prompt string */
	char	e_crlf;		/* zero if cannot return to beginning of line */
	jmp_buf e_env;
	int	e_llimit;	/* line length limit */
	int	e_hline;	/* current history line number */
	int	e_hloff;	/* line number offset for command */
	int	e_hismin;	/* minimum history line number */
	int	e_hismax;	/* maximum history line number */
	int	e_raw;		/* set when in raw mode or alt mode */
	int	e_cur;		/* current line position */
	int	e_eol;		/* end-of-line position */
	int	e_pcur;		/* current physical line position */
	int	e_peol;		/* end of physical line position */
	int	e_mode;		/* edit mode */
	int	e_index;	/* index in look-ahead buffer */
	int	e_repeat;
	int	e_saved;
	int	e_fcol;		/* first column */
	int	e_ucol;		/* column for undo */
	int	e_addnl;	/* set if new-line must be added */
	int	e_wsize;	/* width of display window */
	char	*e_outbase;	/* pointer to start of output buffer */
	char	*e_outptr;	/* pointer to position in output buffer */
	char	*e_outlast;	/* pointer to end of output buffer */
	genchar	*e_inbuf;	/* pointer to input buffer */
	char	*e_prompt;	/* pointer to buffer containing the prompt */
	genchar	*e_ubuf;	/* pointer to the undo buffer */
	genchar	*e_killbuf;	/* pointer to delete buffer */
	char	*e_search;	/* temporary workspace buffer */
	genchar	*e_Ubuf;	/* temporary workspace buffer */
	genchar	*e_physbuf;	/* temporary workspace buffer */
	int	e_lbuf[LOOKAHEAD];/* pointer to look-ahead buffer */
	int	e_fd;		/* file descriptor */
	int	e_ttyspeed;	/* line speed, also indicates tty parms are valid */
	int	*e_globals;	/* global variables */
	genchar	*e_window;	/* display window  image */
	char	e_inmacro;	/* processing macro expansion */
#ifndef KSHELL
	char	e_prbuff[PRSIZE]; /* prompt buffer */
#endif /* KSHELL */
};

#define FEMAX		50	/* maximum number of file matches for q_expand */
#undef MAXWINDOW
#define MAXWINDOW	160	/* maximum width window */
#define MINWINDOW	15	/* minimum width window */
#define DFLTWINDOW	80	/* default window width */
#define	MAXPAT		100	/* maximum length for pattern word */
#define	YES	1
#define NO	0
#define FAST	2
#define SLOW	1
#define RAWMODE	1
#define ALTMODE	2
#define DELETE	'\177'
#define BELL	'\7'
#define ESC	033
#define	UEOF	-2			/* user eof char synonym */
#define	UERASE	-3			/* user erase char synonym */
#define	UINTR	-4			/* user intr char synonym */
#define	UKILL	-5			/* user kill char synonym */
#define	UQUIT	-6			/* user quit char synonym */

#define	cntl(x)		(x&037)

#ifndef KSHELL
#   define STRIP	0377
#   define TO_PRINT	0100
#   define GMACS	1
#   define EMACS	2
#   define VIRAW	4
#   define EDITVI	8
#   define NOHIST	16
#   define EDITMASK	15
#   define is_option(m)	(opt_flag&(m))
    extern char opt_flag;
#   ifdef SYSCALL
#	define read(fd,buff,n)	syscall(3,fd,buff,n)
#   else
#	define read(fd,buff,n)	rEAd(fd,buff,n)
#   endif /* SYSCALL */
#endif	/* KSHELL */

extern struct edit editb;
#ifdef PROTO
    extern void ed_crlf(void);
    extern void ed_putchar(int);
    extern void ed_ringbell(void);
    extern void ed_setup(int);
    extern void ed_flush(void);
    extern int	ed_getchar(void);
    extern int	ed_virt_to_phys(genchar*,genchar*,int,int,int);
    extern int	ed_window(void);
    extern void ed_ungetchar(int);
#   ifdef KSHELL
	extern int ed_macro(int);
	extern int ed_expand(char[],int*,int*,int);
#   endif /* KSHELL */
#else
    extern void ed_crlf();
    extern void ed_putchar();
    extern void ed_ringbell();
    extern void ed_setup();
    extern void ed_flush();
    extern int	ed_getchar();
    extern int	ed_virt_to_phys();
    extern int	ed_window();
    extern void ed_ungetchar();
#   ifdef KSHELL
	extern int ed_macro();
	extern int ed_expand();
#   endif /* KSHELL */
#endif /* PROTO */

extern const char	e_runvi[];
#ifndef KSHELL
   extern const char	e_version[];
#endif /* KSHELL */
