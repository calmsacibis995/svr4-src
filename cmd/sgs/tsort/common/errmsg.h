/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:errmsg.h	1.3"
/*LINTLIBRARY*/

/*	errmsg.h
	Include file for error message interface.
	Command and library version.
*/

#define	errmsg	errtag( __FILE__, __LINE__ ), errtext

extern  void 	erraction();
extern	int     errafter();	/* routine run after text is printed */
extern	void	errbefore();	/* routine run before text is printed */
extern	int	errexit();	/* exit(2) code for actions that exit */
extern	void	_errmsg();
extern	void	errprefix();	/* prefix to error message, unique to each
				supplier */
extern	void	errsource();	/* source of error message, usually command name
				*/
extern	void	errtag();
extern	void	errtext();
extern	void	errtofix();
extern	void	errusage();	/* routine to print command usage message */
extern	void	errverb();	/* adjust verbosity of error messages */

/* severities  first argument to errbefore(), errafter(), errtext() */
#define	EIGNORE	-1	/* special severity, no message, no action, returns */
#define	EINFO	0
#define	EWARN	1
#define	EERROR	2
#define	EHALT	3

/* special errtext() argument that prints a standard message based on errno */
#define	EERRNO	1

/* actions : returned by errafter() used as an argument to erraction()
 */
#define	EEXIT	100
#define	EABORT  200
#define ERETURN 300

/* used to set verbosity */
#define	ENO	0
#define	EYES	1
#define	EDEF	2   /* default value -- setting ersyserr */

struct Err {
			/* flags to adjust verbosity */
	char	vbell;	/* ring bell before message */
	char	vprefix;
	char	vsource;	/* source information, including prefix */
	char	vsevmsg;	/* severity message */
	char	vsyserr;	/* system error message, perror() */
	char	vfix;	/* to fix msg */
	char	vtag;
	char	vtext;
			/* message content and context */
	char	*prefix;	/* usually unique per manufacturer */
	char	*envsource;	/* source from ERRSOURCE environmental variable
				*/
	char	*source;	/* usually command name */
	int	severity;
	char	**sevmsg;	/* error messages that depend on severity */
	char	*tofix;  /* set by errtofix()  */
	int	tagnum;
	char	*tagstr;
	int	exit;	/* exit(2) code to use if error causes exit */
};
extern struct Err	Err;

/* declare Common Library Functions (CLF)
	stdio.h needs to be defined (stdio.h handles itself properly if it is
	included more than once).
*/

#include	<stdio.h>
#include	<sys/types.h>

extern	int	zaccess();
extern	int	zacct();
extern	char	*zcalloc();
extern	int	zchdir();
extern	int	zchmod();
extern	int	zchown();
extern	int	zchroot();
extern	int	zclose();
extern	int	zcreat();
extern	char	*zcuserid();
extern	int	zdup();
extern	int	zexecvp();
extern	int	zfclose();
extern	int	zfcntl();
extern	int	zferror();
extern	int	zfread();
extern	int	zfseek();
extern	int	zfstat();
extern	int	zfwrite();
extern	int	zioctl();
extern	int	zkill();
extern	int	zlink();
extern	long	zlseek();
extern	char	*zmalloc();
extern	int	zmknod();
extern	int	zmount();
extern	int	znice();
extern	int	zopen();
extern	int	zpipe();
extern	int	zplock();
extern	int	zptrace();
extern	int	zread();
extern	char	*zrealloc();
extern	int	zsetgid();
extern	int	zsetuid();
extern	int	zstat();
extern	daddr_t	zulimit();
extern	int	zumount();
extern	int	zuname();
extern	int	zunlink();
extern	int	zwait();
extern	int	zwrite();
extern	FILE    *zfopen();
