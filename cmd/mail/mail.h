/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:mail.h	1.11.3.1"
 /*
  * All global externs defined here. All variables are initialized
  * in init.c
  *
  * !!!!!IF YOU CHANGE (OR ADD) IT HERE, DO IT THERE ALSO !!!!!!!!
  *
  */
#include	<errno.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<pwd.h>
#include	<signal.h>
#include	<string.h>
#include	<grp.h>
/* The following is a kludge to allow for inconsistent header files in SVR4 */
#define		_CLOCK_T
#include	<time.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<sys/utsname.h>

#ifdef SVR3
   struct utimbuf {
	time_t	actime;
	time_t	modtime;
   };
#else
#  include	<utime.h>
#endif
/* #ifdef SVR3 */
# include	"myregexpr.h"
/* #else */
/* # include	<regexpr.h> */
/* #endif */
#include	"libmail.h"

/* The following typedefs must be used in SVR4 */
#ifdef SVR3
# ifndef sun
typedef int gid_t;
typedef int uid_t;
# endif
typedef int pid_t;
#endif

#define CHILD		0
#define SAME		0

#define	BELL		07

#ifdef DEBUG
#   define	PIPER	"./mail_pipe"
#else
#   define	PIPER	"/usr/lib/mail/mail_pipe"
#endif

#define CERROR		-1
#define CSUCCESS	0

#define TRUE	1
#define FALSE	0

#define	HEAD	1
#define TAIL	0

#define	REAL	1
#define DEFAULT	0

/* findSurg() return values */
#define	NOMATCH		-1
#define	DELIVER		0
#define	POSTDELIVER	1
#define	DENY		2
#define	TRANSLATE	3

/* sendsurg() return values */
#define	FAILURE		0
#define	CONTINUE	1
#define	SUCCESS		2
/*	TRANSLATE	3 */

#define	HDRSIZ	256	/* maximum length of header line */

#define E_FLGE	1	/* flge error */
#define	E_REMOTE 1	/* unknown remote */
#define E_FILE	2	/* file error */
#define E_SPACE	3	/* no space */
#define E_FRWD	4	/* cannot forward */
#define E_SYNTAX 5      /* syntax error */
#define E_FRWL	6	/* forwarding loop */
#define E_SNDR  7	/* invalid sender */
#define E_USER  8	/* invalid user */
#define E_FROM  9	/* too many From lines */
#define E_PERM  10 	/* bad permissions */
#define E_MBOX  11 	/* mbox problem */
#define E_TMP	12 	/* temporary file problem */
#define E_DEAD  13 	/* Cannot create dead.letter */
#define E_UNBND 14 	/* Unbounded forwarding */
#define E_LOCK  15 	/* cannot create lock file */
#define E_GROUP	16	/* no group id of 'mail' */
#define	E_MEM	17	/* malloc failure */
#define E_FORK	18	/* could not fork */
#define	E_PIPE	19	/* could not pipe */
#define	E_OWNR	20	/* invoker does not own mailfile */
#define	E_DENY	21	/* permission denied by mailsurr file */
#define E_SURG	22	/* surrogate command failed - rc != 0 || 99 */

#define	H_AFWDCNT		1	/* "Auto-Forward-Count:"  */
#define	H_AFWDFROM		2	/* "Auto-Forwarded-From:" */
#define	H_CLEN			3	/* "Content-Length:"      */
#define	H_CTYPE			4	/* "Content-Type:"        */
#define	H_DATE			5	/* "Date:" 		  */
#define	H_DEFOPTS		6	/* "Default-Options:" 	  */
#define	H_EOH			7	/* "End-of-Header:" 	  */
#define	H_FROM			8	/* "From " 		  */
#define	H_FROM1			9	/* ">From " 		  */
#define	H_FROM2			10	/* "From: " 		  */
#define	H_MTSID			11	/* "MTS-Message-ID:" 	  */
#define	H_MTYPE			12	/* "Message-Type:" 	  */
#define	H_MVERS			13	/* "Message-Version:" 	  */
#define	H_MSVC			14	/* "Message-Service:" 	  */
#define	H_RECEIVED		15	/* "Received:"	 	  */
#define	H_RVERS			16	/* "Report-Version:" 	  */
#define	H_SUBJ			17	/* "Subject:" 		  */
#define	H_TO			18	/* "To:" 		  */
#define	H_TCOPY			19	/* ">To:" 		  */
#define	H_TROPTS		20	/* "Transport-Options:"   */
#define	H_UAID			21	/* "UA-Content-ID:"	  */
#define	H_DAFWDFROM		22	/* Hold A-F-F when sending Del. Notf. */
#define	H_DTCOPY		23	/* Hold ">To:" when sending Del. Notf.*/
#define	H_DRECEIVED		24	/* Hold Rcvd: when sending Del. Notf.*/
#define H_CONT			25	/* Continuation of previous line */
#define H_NAMEVALUE		26	/* unrecognized "name: value" hdr line*/

/* MTA Transport Options */
#define	DELIVERY	001
#define	NODELIVERY	002
#define	REPORT		010
#define	RETURN		020
#define	IGNORE		040

/*
	copylet flags
*/
#define	REMOTE		1		/* remote mail, add rmtmsg */
#define ORDINARY	2
#define ZAP		3		/* zap header and trailing empty line */
#define FORWARD		4
#define TTY		5		/* suppress binary to tty */

#define	LSIZE		BUFSIZ		/* maximum size of a line */
#define	MAXLET		1000		/* maximum number of letters */
#define FROMLEVELS	20		/* maxium number of forwards */
#ifdef FILENAME_MAX
# define MAXFILENAME	FILENAME_MAX	/* max length of a filename */
#else
# define MAXFILENAME	512		/* max length of a filename */
#endif
#define DEADPERM	0600		/* permissions of dead.letter */

#ifndef	MFMODE
#define	MFMODE		0660		/* create mode for `/var/mail' files */
#endif

#define A_OK		0		/* return value for access */
#define A_EXECUTE	1
#define A_EXIST		0		/* access check for existence */
#define A_WRITE		2		/* access check for write permission */
#define A_READ		4		/* access check for read permission */

#ifdef DEBUG
#  define MAILSURR "./Nmailsurr"
#else
#  define MAILSURR "/etc/mail/mailsurr"
#  define MAILCNFG "/etc/mail/mailcnfg"
#endif

struct hdr {
	char	*tag;
	int	default_display;
};

struct hdrs {
	struct	hdrs	*next;
	struct	hdrs	*prev;
	struct	hdrs	*cont;	/* Continuation lines */
		char	value[HDRSIZ+1];
};

struct hdrlines {
	struct	hdrs	*head;
	struct	hdrs	*tail;
};

typedef struct recip {
	struct recip	*next;
	char		*name;
} recip;

typedef struct reciplist {
	recip *last_recip;
	recip recip_list;
} reciplist;

struct let {
	long	adr;		/* offset in mailfile of letter n */
	char	change;		/* disposition status of letter n */
	char	text;		/* 1 ==> text content, 0 ==> binary content.
				 * This is determined INDEPENDENTLY of what
				 * the Content-type, if present, says...
				 */
};

typedef enum t_surrtype
{
    t_eof, t_transport = '<',
    t_accept = 'a', t_deny = 'd',
    t_translate = 't', t_postprocess = '>'
} t_surrtype;

typedef struct t_surrfile
{
    /* originator's regular expression */
    string *orig_pattern;
    char *orig_regex;
    int orig_reglen;
    int orig_nbra;

    /* recipient's regular expression */
    string *recip_pattern;
    char *recip_regex;
    int recip_reglen;
    int recip_nbra;

    /* the type of the command string */
    t_surrtype surr_type;

    int batchsize;	/* transport	translate	postprocess */
    char *statlist;	/* transport				    */
    string *cmd_left;	/* transport	translate	postprocess */
    string *cmd_right;	/* transport	translate	postprocess */
    int fullyresolved;	/*		translate		    */
} t_surrfile;

#ifdef __STDC__
# include <stdlib.h>
# include <unistd.h>
#else
extern	int	chmod();
extern	int	close();
extern	char	*ctime();
extern	int	errno;
extern	int	execl();
extern	int	execvp();
extern	void	exit();
extern	char	*getenv();
extern	char	*getlogin();
extern	long	ftell();
extern	struct group *getgrnam();
extern	struct passwd *getpwent();
extern	struct passwd *getpwnam();
extern	struct passwd *getpwuid();
extern	char	*malloc();
extern	char	*memcpy();
extern	char	*memmove();
extern	char	*mktemp();
extern	char	*realloc();
extern	void	setpwent();
extern	unsigned	sleep();
extern	char	*strchr();
extern	char	*strcpy();
extern	char	*strncpy();
extern	char	*strpbrk();
extern	char	*strrchr();
extern	time_t	time();
extern	char	*tempnam();
extern	FILE	*tmpfile();
extern	int	unlink();
#endif

extern	char	*optarg;	/* for getopt */
extern	int	optind;
extern	char	*sys_errlist[];

#ifdef __STDC__
extern	void	Dout(char *subname, int level, char *fmt, ...);
extern	void	Tout(char *subname, char *msg, ...);
extern	int	add_recip(reciplist *plist, char *name, int checkdups);
extern	char	*altcompile(const char *instring, char *expbuf, char *endbuf);
extern	int	areforwarding(char *mailfile);
extern	void	cat(char*, char*, char*);
extern	int	ckdlivopts(int tcopy_hdr, int *svopts);
extern	void	cksaved(char *user);
extern	int	cksurg_rc(int surr_num, int rc);
extern	void	clr_hinfo(void);
extern	void	clrhdr(int hdrtype);
extern	void	cmdexpand(int letnum, string *instr, string *outstr, char **lbraslist, char **lbraelist);
extern	void	copyback(void);
extern	int	copylet(int letnum, FILE *f, int type);
extern	void	copymt(FILE *f1, FILE *f2);
extern	void	createmf(uid_t uid, char *file);
extern	void	del_reciplist (reciplist *list);
extern	void	delete(int);
extern	void	doFopt(void);
extern	void	done(int);
extern	FILE	*doopen(char *file, char *type, int errnum);
extern	int	dowait(pid_t pidval);
extern	void	dumpaff(int type, int htype, int *didafflines, int *suppress, FILE *f);
extern	void	dumprcv(int type, int htype, int *didrcvlines, int *suppress, FILE *f);
extern	void	errmsg(int error_value, char *error_message);
extern	int	findSurg(int letnum, string *execbuf, int flag, int *psurr_num, int *paccept, string *lorig, string *lrecipname);
extern	void	gendeliv(FILE *fp, int rc, char *name);
extern	int	getcomment(char *s, char *q);
extern	int	gethead(int	current, int all);
extern	int	getline(char *ptr2line, int max, FILE *f);
extern	int	getnumbr(char *s);
extern	int	getsurr(FILE *fp, string *buf, int firstfield);
extern	void	goback(int letnum);
extern	int	init(void);
extern	void	initsurrfile(void);
extern	int	isheader(char *lp, int *ctfp);
extern	int	isit(char *lp, int type);
extern	int	islocal(char *user, uid_t *puid);
extern	int	istext(unsigned char *s, int size);
extern	int	legal(char *file);
extern	void	lock(char	*user);
extern	int	madd_recip(reciplist *plist, char *name, int checkdups);
extern	char	*mailcompile(string *pattern, int *retlen, int *retnbra);
extern	void	mkdead(void);
extern	void	mktmp(void);
extern	void	mta_ercode(FILE *outfile);
extern	void	new_reciplist (reciplist *list);
extern	int	notme(char *fto, char *myname);
extern	int	parse(int argc, char **argv);
extern	int	pckaffspot(void);
extern	int	pckrcvspot(void);
extern	void	pickFrom(char *lineptr);
extern	int	pipletr(int letter, char *command, int cltype);
extern	void	poplist (int hdrtype, int where);
extern	int	printhdr (int type, int hdrtype, struct hdrs *hptr, FILE *fp);
extern	void	printmail(void);
extern	void	pushlist(int hdrtype, int where, char *s, int contflg);
extern	void	savdead(void);
extern	void	savehdrs(char *s, int hdrtype);
extern	int	sel_disp (int type, int hdrtype, char *s);
extern	int	send(reciplist *plist, int letnum, char *name, int level);
extern	int	sendlist(reciplist *list, int letnum, int level);
extern	void	sendmail(int argc, char **argv);
extern	int	sendsurg(reciplist *plist, int  letnum, int flag, int local);
extern	void	setletr(int letter, int status);
extern	void	(*setsig(int i, void(*f)()))();
extern	void	setsurg_bt(string *st, int *pbatchsize, int *presolved);
extern	char	*setsurg_rc(string *st, int defreal, int *pbatchsize);
extern	char	**setup_exec(char*);
extern	void	stamp(void);
extern	int	systm(char *s);
extern	void	tmperr(void);
extern	string	*tokdef(string *fld, string *tok, char *name);
extern	int	translate(reciplist *plist, char *cmdstr, char *origname);
extern	void	unlock(void);
extern	int	validmsg(int);
extern	int	wtmpf(char *str, int length);
#else
extern	void	Dout();
extern	void	Tout();
extern	int	add_recip();
extern	char	*altcompile();
extern	int	areforwarding();
extern	void	cat();
extern	int	ckdlivopts();
extern	void	cksaved();
extern	int	cksurg_rc();
extern	void	clr_hinfo();
extern	void	clrhdr();
extern	void	cmdexpand();
extern	void	copyback();
extern	int	copylet();
extern	void	copymt();
extern	void	createmf();
extern	void	del_reciplist ();
extern	void	delete();
extern	void	doFopt();
extern	void	done();
extern	FILE	*doopen();
extern	int	dowait();
extern	void	dumpaff();
extern	void	dumprcv();
extern	void	errmsg();
extern	int	findSurg();
extern	void	gendeliv();
extern	int	getcomment();
extern	int	gethead();
extern	int	getline();
extern	int	getnumbr();
extern	int	getsurr();
extern	void	goback();
extern	int	init();
extern	void	initsurrfile();
extern	int	isheader();
extern	int	isit();
extern	int	islocal();
extern	int	istext();
extern	int	legal();
extern	void	lock();
extern	int	madd_recip();
extern	char	*mailcompile();
extern	void	mkdead();
extern	void	mktmp();
extern	void	mta_ercode();
extern	void	new_reciplist ();
extern	int	notme();
extern	int	parse();
extern	int	pckaffspot();
extern	int	pckrcvspot();
extern	void	pickFrom ();
extern	int	pipletr();
extern	void	poplist ();
extern	int	printhdr ();
extern	void	printmail();
extern	void	pushlist();
extern	void	savdead();
extern	void	savehdrs();
extern	int	sel_disp ();
extern	int	send();
extern	int	sendlist();
extern	void	sendmail();
extern	int	sendsurg();
extern	void	setletr();
extern	void	(*setsig())();
extern	void	setsurg_bt();
extern	char	*setsurg_rc();
extern	char	**setup_exec();
extern	void	stamp();
extern	int	systm();
extern	void	tmperr();
extern	string	*tokdef();
extern	int	translate();
extern	void	unlock();
extern	int	validmsg();
extern	int	wtmpf();
#endif

extern	int	ac;		/* argument list count */
extern	char	**av;		/* argument list */
extern	int	affbytecnt;     /* Total bytes of Auto-Fwd. info in msg. */
extern	int	affcnt;		/* Number of Auto-Fwd.-From: lines in msg. */
extern	int	Daffbytecnt;    /* Hold affbytecnt when sending Deliv. Notif. */
extern	int	Daffcnt;	/* Hold affcnt when sending Deliv. Notif. */
extern	char	binmsg[];
extern	int	changed;	/* > 0 says mailfile has changed */
extern	char	datestring[60];
extern	char	dbgfname[20];	/* name of file for debugging output */
extern	FILE	*dbgfp;		/* FILE* for debugging output */
extern	char	dead[];		/* name of dead.letter */
extern	int	debug;		/* Controls debugging level. 0 ==> no debugging */
extern	int	delflg;
extern	int	dflag;		/* 1 says returning unsendable mail */
extern	char	*errlist[];
extern	int	error;		/* Local value for error */
extern	char	*failsafe;	/* $FAILSAFE */
extern	int	file_size;
extern	int	flge;		/* 1 ==> 'e' option specified */
extern	int	flgF;		/* 1 ==> Installing/Removing  Forwarding */
extern	int	flgf;		/* 1 ==> 'f' option specified */
extern	int	flgh;		/* 1 ==> 'h' option specified */
extern	int	flgm;
extern	int	flgp;		/* 1 ==> 'p' option specified */
extern	int	flgP;		/* 1 ==> 'P' option specified */
extern	int	flgr;		/* 1 ==> 'r' option -- print in fifo order */
extern	int	flgt;		/* 1 ==> 't' option -- add To: line to letter */
extern	int	flgT;		/* 1 ==> 'T' option specified */
extern	int	flgw;		/* 1 ==> 'w' option specified */
extern	int	fnuhdrtype;	/* type of first non-UNIX header line */
extern	char	forwmsg[];	/* " forwarded by %s" */
extern	char	frwlmsg[];	/* "Forwarding loop detected in mailfile" */
extern	char	fromS[1024];	/* stored here by sendmail for sendsurg */
extern	char	fromU[1024];	/* stored here by sendmail for sendsurg */
extern	char	frwrd[];	/* forwarding sentinel */
extern	char	fwdFrom[1024];
extern	int	goerr;		/* counts parsing errors */
extern	struct	group *grpptr;	/* pointer to struct group */
extern	struct hdrlines hdrlines[H_CONT];
extern	struct hdr header[];	/* H_* #define's used to index into array */
extern	char	*help[];
extern	char	*hmbox;		/* pointer to $HOME/mbox */
extern	char	*hmdead;	/* pointer to $HOME/dead.letter */
extern	char	*home;		/* pointer to $HOME */
extern	long	iop;
extern	int	interactive;	/* 1 says user is interactive */
extern	int	ismail;		/* default to program=mail */
extern	int	keepdbgfile;	/* does debug file get deleted at end? */
extern	struct let let[MAXLET];
extern	char	*lettmp;	/* pointer to tmp filename */
extern	char	lfil[MAXFILENAME];
extern	char	line[LSIZE];	/* holds a line of a letter in many places */
extern	char	*mailfile;	/* pointer to mailfile */
extern	char	mailcnfg[];	/* /etc/mail/mailcnfg */
extern	char	maildir[];	/* directory for mail files */
extern	gid_t	mailgrp;	/* numeric id of group 'mail' */
extern	char	mailsave[];	/* dir for save files */
extern	char	*mailsurr;	/* surrogate file name */
extern	FILE	*malf;		/* File pointer for mailfile */
extern	int	maxerr;		/* largest value of error */
extern	char	mbox[];		/* name for mbox */
extern	uid_t	mf_uid;		/* uid of user's mailfile */
extern	gid_t	mf_gid;		/* gid of user's mailfile */
extern	char	*msgtype;
extern	char	my_name[20];	/* user's name who invoked this command */
extern	uid_t	my_euid;	/* user's euid */
extern	gid_t	my_egid;	/* user's egid */
extern	uid_t	my_uid;		/* user's uid */
extern	gid_t	my_gid;		/* user's gid */
extern	int	nlet;		/* current number of letters in mailfile */
extern	int	onlet;		/* number of letters in mailfile at startup*/
extern	int	optcnt;		/* Number of options specified */
extern	int	orig_aff;	/* orig. msg. contained H_AFWDFROM lines */
extern	int	orig_dbglvl;	/* argument to -x invocation option */
extern	int	orig_rcv;	/* orig. msg. contained H_RECEIVED lines */
extern	int	orig_tcopy;	/* orig. msg. contained H_TCOPY lines */
extern	struct	passwd *pwd;	/* holds passwd entry for this user */
extern	int	pflg;		/* binary message display override flag */
extern	int	Pflg;		/* Selective display flag; 1 ==> display all */
extern	char	*program;	/* program name */
extern	int	rcvbytecnt;     /* Total bytes of Received: info in msg. */
extern	int	Drcvbytecnt;    /* Hold rcvbytecnt when sending Del. Notif. */
extern	char	*recipname;	/* full recipient name/address */
extern	int	replying;	/* 1 says we are replying to a letter */
extern	char	RFC822datestring[60];/* Date in RFC822 date format */
extern	char	rmtmsg[];	/* "remote from %s" */
extern	char	Rpath[1024];	/* return path to sender of message */
extern	char	rtrnmsg[];	/* "UNDELIVERABLE MAIL being returned by %s" */
extern	int	sav_errno;
extern	char	savefile[MAXFILENAME];	/* holds filename of save file */
#ifdef __STDC__
extern	void	(*saveint)(int);
#else
extern	void	(*saveint)();
#endif
extern	char	*seldisp[];
extern	int	sending;	/* TRUE==>sending mail; FALSE==>printing mail */
extern	char	sendto[1024];
extern	jmp_buf	sjbuf;
extern	int	surg_rc;	/* exit code of surrogate command */
extern	t_surrfile *surrfile;	/* the compiled surrogate file */
extern	int	surr_len;	/* # entries in surrogate file */
extern	char	*SURRcmdstr;	/* save in case of FAILURE */
extern	FILE	*SURRerrfile;	/* stderr from surrogate in case of failure */
extern	char	*thissys;	/* Holds name of the system we are on */
extern	FILE	*tmpf;		/* file pointer for temporary files */
extern	char	tmpdir[];	/* default directory for tmp files */
extern	mode_t	umsave;
extern	struct	utsname utsn;
extern	struct utimbuf *utimep;
extern	char	uval[1024];

#ifdef DEBUG
#  define open(s,n) my_open(s,n)
#  define close(n) my_close(n)
   extern FILE	*my_fopen();
#  define fopen(s,t) my_fopen(s,t)
#  define fclose(n) my_fclose(n)
#endif

#ifdef sun
#define _NFILE getdtablesize()
#endif
