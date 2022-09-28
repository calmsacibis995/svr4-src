/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/glob.h	1.4.5.1"
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * A bunch of global variable declarations lie herein.
 * def.h must be included first.
 */

extern int		Fflag;		/* -F option (followup) */
extern int		Hflag;		/* print headers and exit */
extern char		*Tflag;		/* -T temp file for netnews */
extern int		UnUUCP;		/* -U flag */
#ifndef preSVr4
extern char		altmaildir[];	/* /usr/mail is linked to /var/mail */
extern char		altmailname[];	/* Name of /usr/mail system mailbox */
#endif
extern char		**altnames;	/* List of alternate names for user */
extern int		askme;		/* ???? */
extern int		baud;		/* Output baud rate */
extern char		binmsg[];	/* Message: content unprintable */
extern struct cmd	cmdtab[];	/* ???? */
extern int		cond;		/* Current state of conditional exc. */
extern NODE		*curptr;	/* ???? */
extern int		debug;		/* Debug flag set */
extern char		domain[];	/* ???? */
extern struct message	*dot;		/* Pointer to current message */
extern int		edit;		/* Indicates editing a file */
extern char		*editfile;	/* Name of file being edited */
extern int		escapeokay;	/* Permit ~s even when reading files */
extern int		exitflg;	/* -e for mail test */
extern NODE		*fplist;	/* ???? */
extern struct grouphead	*groups[];	/* Pointer to active groups */
extern struct hdr	header[];	/* Known header types */
extern int		hflag;		/* Sequence number for network -h */
extern struct hdrlines	hdrlines[];	/* ???? */
extern char		homedir[];	/* Name of home directory */
extern char		host[];		/* ???? */
extern struct ignore	*ignore[];	/* Pointer to ignored fields */
extern int		image;		/* File descriptor for image of msg */
extern FILE		*input;		/* Current command input file */
extern int		intty;		/* True if standard input a tty */
extern int		ismail;		/* default to program=mail */
extern FILE		*itf;		/* Input temp file buffer */
extern struct let	let[];		/* ???? */
extern int		lexnumber;	/* Number of TNUMBER from scan() */
extern char		lexstring[];	/* String from TSTRING, scan() */
extern int		loading;	/* Loading user definitions */
extern char		*lockname;	/* named used for locking in /var/mail */
extern char		maildir[];	/* directory for mail files */
extern char		mailname[];	/* Name of /var/mail system mailbox */
extern off_t		mailsize;	/* Size of system mailbox */
extern int		maxfiles;	/* Maximum number of open files */
extern struct message	*message;	/* The actual message structure */
extern char		*metanet;	/* ???? */
extern int		msgCount;	/* Count of messages read in */
extern gid_t		myegid;		/* User's effective gid */
extern uid_t		myeuid;		/* User's effective uid */
extern char		myname[];	/* My login id */
extern pid_t		mypid;		/* Current process id */
extern gid_t		myrgid;		/* User's real gid */
extern uid_t		myruid;		/* User's real uid */
extern int		newsflg;	/* -I option for netnews */
extern char		noheader;	/* Suprress initial header listing */
extern int		noreset;	/* String resets suspended */
extern char		nosrc;		/* Don't source /etc/mail/mailx.rc */
extern int		numberstack[];	/* Stack of regretted numbers */
extern char		origname[];	/* Original name of mail file */
extern FILE		*otf;		/* Output temp file buffer */
extern int		outtty;		/* True if standard output a tty */
extern FILE		*pipef;		/* Pipe file we have opened */
extern char		*progname;	/* program name (argv[0]) */
extern char		*prompt;	/* prompt string */
extern int		rcvmode;	/* True if receiving mail */
extern int		readonly;	/* Will be unable to rewrite file */
extern int		regretp;	/* Pointer to TOS of regret tokens */
extern int		regretstack[];	/* Stack of regretted tokens */
extern char		*rflag;		/* -r address for network */
extern int		rmail;		/* Being called as rmail */
extern int		sawcom;		/* Set after first command */
extern int		selfsent;	/* User sent self something */
extern int		senderr;	/* An error while checking */
extern int		sending;	/* TRUE==>sending mail; FALSE==>printing mail */
extern char		*sflag;		/* Subject given from non tty */
extern int		sourcing;	/* Currently reading variant file */
extern int		space;		/* Current maximum number of messages */
extern jmp_buf		srbuf;		/* ???? */
extern struct strings	stringdope[];	/* pointer for the salloc routines */
extern char		*stringstack[];	/* Stack of regretted strings */
extern char		tempEdit[];	/* ???? */
extern char		tempMail[];	/* ???? */
extern char		tempMesg[];	/* ???? */
extern char		tempQuit[];	/* ???? */
extern char		tempResid[];	/* temp file in :saved */
extern char		tempSet[];	/* ???? */
extern char		tempZedit[];	/* ???? */
extern uid_t		uid;		/* The invoker's user id */
extern struct utimbuf	*utimep;	/* ???? */
extern struct var	*variables[];	/* Pointer to active var list */
extern char		*version;	/* ???? */

/*
 * Standard external variables from the C library.
 */
extern char		*optarg;
extern int		optind;
