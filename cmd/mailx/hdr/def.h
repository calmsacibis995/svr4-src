/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/def.h	1.5.6.1"

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>
#include <maillock.h>
#include <ctype.h>
#include <errno.h>
#ifndef preSVr4
# include <unistd.h>
# include <stdlib.h>
# include <ulimit.h>
# include <wait.h>
#endif
#include "local.h"
#include "uparm.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 */

#ifdef preSVr4	/* support for void* */
# define VOID		char
#else
# define VOID		void
#endif

#define	SENDESC		'~'		/* Default escape for sending */
#define	NMLSIZE		1024		/* max names in a message list */
#define	PATHSIZE	1024		/* Size of pathnames throughout */
#define	HSHSIZE		19		/* Hash size for aliases and vars */
#define	LINESIZE	5120		/* max readable line width */
#define	STRINGSIZE	((unsigned) 128)/* Dynamic allocation units */
#define	MAXARGC		1024		/* Maximum list of raw strings */
#define	NOSTR		((char *) 0)	/* Nill string pointer */
#define	NOSTRPTR	((char **) 0)	/* Nill pointer to string pointer */
#define	NOINTPTR	((int *) 0)	/* Nill pointer */
#define	MAXEXP		25		/* Maximum expansion of aliases */

#define	equal(a, b)	(strcmp(a,b)==0)/* A nice function to string compare */
#define fopen(s,t)	my_fopen(s,t)	/* Keep a list of all opened files */
#define fclose(s)	my_fclose(s)	/* delete closed file from the list*/
#define	plural(n)	((n) == 1 ? "" : "s")

struct message {
	long	m_block;		/* block number of this message */
	long	m_offset;		/* offset in block of message */
	long	m_size;			/* Bytes in the message */
	long	m_lines;		/* Lines in the message */
	long	m_clen;			/* Content-Length of the mesg   */
	short	m_flag;			/* flags, see below */
	char	m_text;			/* TRUE if the contents is text */
					/* False otherwise		*/
};

typedef struct fplst {
	FILE	*fp;
	struct	fplst	*next;
} NODE;

/*
 * flag bits.
 */

#define	MUSED		(1<<0)		/* entry is used, but this bit isn't */
#define	MDELETED	(1<<1)		/* entry has been deleted */
#define	MSAVED		(1<<2)		/* entry has been saved */
#define	MTOUCH		(1<<3)		/* entry has been noticed */
#define	MPRESERVE	(1<<4)		/* keep entry in sys mailbox */
#define	MMARK		(1<<5)		/* message is marked! */
#define	MODIFY		(1<<6)		/* message has been modified */
#define	MNEW		(1<<7)		/* message has never been seen */
#define	MREAD		(1<<8)		/* message has been read sometime. */
#define	MSTATUS		(1<<9)		/* message status has changed */
#define	MBOX		(1<<10)		/* Send this to mbox, regardless */

#define	H_AFWDCNT	1		/* "Auto-Forward-Count:"  */
#define	H_AFWDFROM	2		/* "Auto-Forwarded-From:" */
#define	H_CLEN		3		/* "Content-Length:"      */
#define	H_CTYPE		4		/* "Content-Type:"        */
#define	H_DATE		5		/* "Date:" 		  */
#define	H_DEFOPTS	6		/* "Default-Options:" 	  */
#define	H_EOH		7		/* "End-of-Header:" 	  */
#define	H_FROM		8		/* "From " 		  */
#define	H_FROM1		9		/* ">From " 		  */
#define	H_FROM2		10		/* "From: " 		  */
#define	H_MTSID		11		/* "MTS-Message-ID:" 	  */
#define	H_MTYPE		12		/* "Message-Type:" 	  */
#define	H_MVERS		13		/* "Message-Version:" 	  */
#define	H_MSVC		14		/* "Message-Service:" 	  */
#define	H_RECEIVED	15		/* "Received:"	 	  */
#define	H_RVERS		16		/* "Report-Version:" 	  */
#define	H_STATUS	17		/* "Status:"		  */
#define	H_SUBJ		18		/* "Subject:" 		  */
#define	H_TO		19		/* "To:" 		  */
#define	H_TCOPY		20		/* ">To:" 		  */
#define	H_TROPTS	21		/* "Transport-Options:"   */
#define	H_UAID		22		/* "UA-Content-ID:"	  */
#define	H_DAFWDFROM	23		/* Hold A-F-F when sending Del. Notf. */
#define	H_DTCOPY	24		/* Hold ">To:" when sending Del. Notf.*/
#define	H_DRECEIVED	25		/* Hold Rcvd: when sending Del. Notf.*/
#define H_CONT		26		/* Continuation of previous line */
#define H_NAMEVALUE	27		/* unrecognized "name: value" hdr line*/

/*
 * Format of the command description table.
 * The actual table is declared and initialized
 * in lex.c
 */

struct cmd {
	char	*c_name;		/* Name of command */
	int	(*c_func)();		/* Implementor of the command */
	short	c_argtype;		/* Type of arglist (see below) */
	short	c_msgflag;		/* Required flags of messages */
	short	c_msgmask;		/* Relevant flags of messages */
};

/* can't initialize unions */

#define	c_minargs c_msgflag		/* Minimum argcount for RAWLIST */
#define	c_maxargs c_msgmask		/* Max argcount for RAWLIST */

/*
 * Argument types.
 */

#define	MSGLIST	 0		/* Message list type */
#define	STRLIST	 1		/* A pure string */
#define	RAWLIST	 2		/* Shell string list */
#define	NOLIST	 3		/* Just plain 0 */
#define	NDMLIST	 4		/* Message list, no defaults */

#define	P	040		/* Autoprint dot after command */
#define	I	0100		/* Interactive command bit */
#define	M	0200		/* Legal from send mode bit */
#define	W	0400		/* Illegal when read only bit */
#define	F	01000		/* Is a conditional command */
#define	T	02000		/* Is a transparent command */
#define	R	04000		/* Cannot be called from collect */

/*
 * Oft-used mask values
 */

#define	MMNORM		(MDELETED|MSAVED)/* Look at both save and delete bits */
#define	MMNDEL		MDELETED	/* Look only at deleted bit */

/*
 * Structure used to return a break down of a head
 * line
 */

struct headline {
	char	*l_from;	/* The name of the sender */
	char	*l_tty;		/* His tty string (if any) */
	char	*l_date;	/* The entire date string */
};

#define	GTO	1		/* Grab To: line */
#define	GSUBJECT 2		/* Likewise, Subject: line */
#define	GCC	4		/* And the Cc: line */
#define	GBCC	8		/* And also the Bcc: line */
#define	GDEFOPT	16		/* And the Default-Options: lines */
#define	GNL	32		/* Print blank line after */
#define GOTHER	64		/* Other header lines */
#define	GMASK	(GTO|GSUBJECT|GCC|GBCC|GDEFOPT|GNL|GOTHER)
				/* Mask of all header lines */
#define	GDEL	128		/* Entity removed from list */

/*
 * Structure used to pass about the current
 * state of the user-typed message header.
 */

struct header {
	char	*h_to;			/* Dynamic "To:" string */
	char	*h_subject;		/* Subject string */
	char	*h_cc;			/* Carbon copies string */
	char	*h_bcc;			/* Blind carbon copies */
	char	*h_defopt;		/* Default options */
	char	**h_others;		/* Other header lines */
	int	h_seq;			/* Sequence for optimization */
};

/*
 * Structure of namelist nodes used in processing
 * the recipients of mail and aliases and all that
 * kind of stuff.
 */

struct name {
	struct	name *n_flink;		/* Forward link in list. */
	struct	name *n_blink;		/* Backward list link */
	short	n_type;			/* From which list it came */
	char	*n_name;		/* This fella's name */
	char	*n_full;		/* Full name */
};

/*
 * Structure of a variable node.  All variables are
 * kept on a singly-linked list of these, rooted by
 * "variables"
 */

struct var {
	struct	var *v_link;		/* Forward link to next variable */
	char	*v_name;		/* The variable's name */
	char	*v_value;		/* And it's current value */
};

struct mgroup {
	struct	mgroup *ge_link;	/* Next person in this group */
	char	*ge_name;		/* This person's user name */
};

struct grouphead {
	struct	grouphead *g_link;	/* Next grouphead in list */
	char	*g_name;		/* Name of this group */
	struct	mgroup *g_list;		/* Users in group. */
};

#define	NIL	((struct name *) 0)	/* The nil pointer for namelists */
#define	NONE	((struct cmd *) 0)	/* The nil pointer to command tab */
#define	NOVAR	((struct var *) 0)	/* The nil pointer to variables */
#define	NOGRP	((struct grouphead *) 0)/* The nil grouphead pointer */
#define	NOGE	((struct mgroup *) 0)	/* The nil group pointer */
#define	NOFP	((struct fplst *) 0)	/* The nil file pointer */

#define TRUE	1
#define FALSE	0

#define	LSIZE		BUFSIZ		/* maximum size of a line */
#define DEADPERM	0600		/* permissions of dead.letter */
#define TEMPPERM	0600		/* permissions of temp files */
#define MBOXPERM	0600		/* permissions of ~/mbox */

#ifndef	MFMODE
# define MFMODE		0660		/* create mode for `/var/mail' files */
#endif

/*
 * Structure of the hash table of ignored header fields
 */
struct ignore {
	struct ignore	*i_link;	/* Next ignored field in bucket */
	char		*i_field;	/* This ignored field */
};

/* Default_display indicates whether to display this header line to the TTY */
/* when in default mode. Can be overridden via 'P' command at ? prompt */
struct hdr {
	char	*tag;
	int	default_display;
};

#ifdef preSVr4
struct utimbuf {
	time_t	actime;
	time_t	modtime;
};
#else
#  include	<utime.h>
#endif

/*
 * Token values returned by the scanner used for argument lists.
 * Also, sizes of scanner-related things.
 */

#define	TEOL		0		/* End of the command line */
#define	TNUMBER		1		/* A message number */
#define	TDASH		2		/* A simple dash */
#define	TSTRING		3		/* A string (possibly containing -) */
#define	TDOT		4		/* A "." */
#define	TUP		5		/* An "^" */
#define	TDOLLAR		6		/* A "$" */
#define	TSTAR		7		/* A "*" */
#define	TOPEN		8		/* An '(' */
#define	TCLOSE		9		/* A ')' */
#define TPLUS		10		/* A '+' */

#define	REGDEP		2		/* Maximum regret depth. */
#define	STRINGLEN	64		/* Maximum length of string token */

/*
 * Constants for conditional commands.  These describe whether
 * we should be executing stuff or not.
 */

#define	CANY		0		/* Execute in send or receive mode */
#define	CRCV		1		/* Execute in receive mode only */
#define	CSEND		2		/* Execute in send mode only */

/*
 * VM/UNIX has a vfork system call which is faster than forking.  If we
 * don't have it, fork(2) will do . . .
 */

#ifndef VMUNIX
#define	vfork()	fork()
#endif

/*
 * The pointers for the string allocation routines,
 * there are NSPACE independent areas.
 * The first holds STRINGSIZE bytes, the next
 * twice as much, and so on.
 */

#define	NSPACE	25			/* Total number of string spaces */
struct strings {
	char	*s_topFree;		/* Beginning of this area */
	char	*s_nextFree;		/* Next alloctable place here */
	unsigned s_nleft;		/* Number of bytes left here */
};

/* The following typedefs must be used in SVR4 */
#ifdef preSVr4
#ifndef sun
typedef int gid_t;
typedef int uid_t;
typedef int mode_t;
#endif
typedef int pid_t;
#endif

/*
 * Forward declarations of routine types to keep lint and cc happy.
 */

extern int		Copy();
extern FILE		*Fdopen();
extern int		Followup();
extern char		*Getf();
extern int		Respond();
extern int		Save();
extern int		Sendm();
extern int		Type();
extern void		Verhogen();
extern char		*addone();
extern char		*addto();
extern void		alter();
extern int		alternates();
extern void		announce();
extern int		any();
extern int		anyof();
extern int		argcount();
extern void		assign();
extern int		blankline();
extern int		blockof();
extern struct name	*cat();
extern void		clrbuf();
extern FILE		*collect();
extern void		commands();
extern char		*copy();
extern int		copycmd();
extern int		deassign();
extern int		delete();
extern struct name	*delname();
extern int		deltype();
extern char		*detract();
extern int		docomma();
extern int		dopipe();
extern int		dosh();
extern int		echo();
extern int		editor();
extern int		edstop();
extern struct name	*elide();
extern int		elsecmd();
extern int		endifcmd();
extern int		execute();
extern char		*expand();
extern struct name	*extract();
extern int		file();
extern struct grouphead	*findgroup();
extern void		findmail();
extern int		first();
extern void		flush();
extern int		folders();
extern int		followup();
extern int		from();
extern off_t		fsize();
extern int		getfold();
extern int		gethfield();
extern int		getline();
extern int		getmsglist();
extern int		getname();
extern int		getrawlist();
extern void		getrecf();
extern uid_t		getuserid();
extern int		grabh();
extern int		group();
extern void		hangup();
extern int		hash();
extern char		*hcontents();
extern int		headerp();
extern int		headers();
extern int		help();
extern char		*hfield();
extern void		holdsigs();
extern int		icequal();
extern int		ifcmd();
extern int		igfield();
extern void		inithost();
extern int		isdir();
extern int		isheader();
extern int		ishfield();
extern int		ishost();
extern int		isign();
extern int		istext();
extern void		istrcpy();
extern void		lcwrite();
extern int		linecount();
extern void		load();
extern int		lock();
extern void		lockmail();
extern int		mail();
extern void		mail1();
extern void		mapf();
extern int		mboxit();
extern void		mechk();
extern int		messize();
extern void		minit();
extern int		my_fclose();
extern FILE		*my_fopen();
extern int		name();
extern char		*nameof();
extern char		*netmap();
extern int		newfileinfo();
extern int		next();
extern int		npclose();
extern FILE		*npopen();
extern int		null();
extern int		offsetofaddr();
extern int		outof();
extern struct name	*outpre();
extern int		panic();
extern void		parse();
extern int		pcmdlist();
extern int		pdot();
extern int		preserve();
extern void		printgroup();
extern void		printhead();
extern int		puthead();
extern int		putline();
extern int		pversion();
extern void		quit();
extern int		readline();
extern void		receipt();
extern void		relsesigs();
extern int		removefile();
extern int		respond();
extern int		rexit();
extern char		*safeexpand();
extern VOID		*salloc();
extern VOID		*srealloc();
extern int		samebody();
extern int		save();
extern void		savedead();
extern char		*savestr();
extern int		schdir();
extern int		scroll();
extern long		send();
extern int		sendm();
extern int		set();
extern int		setfile();
extern FILE		*setinput();
extern void		setptr();
extern int		shell();
extern void		sigchild();
extern void		(*sigset())();
extern char		*skin();
extern char		*snarf();
extern int		source();
extern char		*splice();
extern void		sreset();
extern void		stop();
extern int		stouch();
extern void		stradd();
extern int		swrite();
extern struct name	*tailof();
extern void		tinit();
extern int		top();
extern void		touch();
extern struct name	*translate();
extern int		type();
extern int		undelete();
extern int		unigfield();
extern void		unlockmail();
extern char		**unpack();
extern int		unset();
extern int		unstack();
extern char		*unuucp();
extern struct name	*usermap();
extern int		username();
extern char		*value();
extern char		*vcopy();
extern int		visual();
extern char		*yankword();

/*
 * These functions are defined in libmail.a
 */
extern int		delempty();
extern char		*maildomain();
extern int		substr();
extern char		*xgetenv();
extern char		*Xgetenv();
extern int		xsetenv();

/*
 * Standard functions from the C library.
 * These are all defined in <stdlib.h> and <wait.h> in SVr4.
 */
#ifdef preSVr4
extern long		atol();
extern char		*getcwd();
extern char		*calloc();
extern char		*getenv();
extern void		exit();
extern void		free();
extern char		*malloc();
extern time_t		time();
extern long		ulimit();
extern int		utime();
extern int		wait();
#endif
