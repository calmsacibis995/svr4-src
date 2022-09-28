/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/history.h	1.1.3.1"
/*
 *	UNIX shell
 *	Header File for history mechanism
 *	written by David Korn
 *
 */



#ifndef IOBSIZE
#   define IOBSIZE	1024
#endif
#define FC_CHAR		'!'
#define HIS_DFLT	128		/* default size of history list */
#define HISMAX		(sizeof(int)*IOBSIZE)
#define HISBIG		(0100000-1024)	/* 1K less than maximum short */
#define HISLINE		16		/* estimate of average sized history line */
#define MAXLINE		258		/* longest history line permitted */

#define H_UNDO		0201		/* invalidate previous command */
#define H_CMDNO		0202		/* next 3 bytes give command number */
#define H_VERSION	1		/* history file format version no. */

struct history
{
	struct fileblk	*fixfp;		/* file descriptor for history file */
	int 		fixfd;		/* file number for history file */
	char		*fixname;	/* name of history file */
	off_t		fixcnt;		/* offset into history file */
	int		fixind;		/* current command number index */
	int		fixmax;		/* number of accessible history lines */
	int		fixflush;	/* set if flushed outside of hflush() */
	off_t		fixcmds[1];	/* byte offset for recent commands */
};

typedef struct
{
	short his_command;
	short his_line;
} histloc;

extern struct history	*hist_ptr;

#ifndef KSHELL
    extern char *getenv();
    extern void	p_flush();
    extern void	p_setout();
    extern void	p_char();
    extern void	p_str();
#   define nam_strval(s)	getenv("s")
#   define NIL		((char*)0)
#   define sh_fail	ed_failed
#   define sh_copy	ed_movstr
#endif	/* KSHELL */

/* the following are readonly */
extern const char	hist_fname[];
extern const char	e_history[];

/* these are the history interface routines */
#ifdef PROTO
    extern int		hist_open(void);
    extern void 	hist_cancel(void);
    extern void 	hist_close(void);
    extern int		hist_copy(char*,int,int);
    extern void 	hist_eof(void);
    extern histloc	hist_find(char*,int,int,int);
    extern void 	hist_flush(void);
    extern void 	hist_list(off_t,int,char*);
    extern int		hist_match(off_t,char*,int);
    extern off_t	hist_position(int);
    extern void 	hist_subst(const char*,int,char*);
    extern char 	*hist_word(char*,int);
#   ifdef ESH
	extern histloc	hist_locate(int,int,int);
#   endif	/* ESH */
#else
    extern int		hist_open();
    extern void 	hist_cancel();
    extern void 	hist_close();
    extern int		hist_copy();
    extern void 	hist_eof();
    extern histloc	hist_find();
    extern void 	hist_flush();
    extern void 	hist_list();
    extern int		hist_match();
    extern off_t	hist_position();
    extern void 	hist_subst();
    extern char 	*hist_word();
#   ifdef ESH
	extern histloc	hist_locate();
#   endif	/* ESH */
#endif /* PROTO */

#ifdef ESH
    extern histloc	hist_locate();
#endif	/* ESH */
