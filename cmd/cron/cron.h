/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)cron:cron.h	1.7.3.3"

#define FALSE		0
#define TRUE		1
#define MINUTE		60L
#define HOUR		60L*60L
#define DAY		24L*60L*60L
#define	NQUEUE		26		/* number of queues available */
#define	ATEVENT		0
#define BATCHEVENT	1
#define CRONEVENT	2

#define ADD		'a'
#define DELETE		'd'
#define	AT		'a'
#define CRON		'c'

#define	QUE(x)		('a'+(x))
#define RCODE(x)	(((x)>>8)&0377)
#define TSTAT(x)	((x)&0377)

#define	FLEN	15
#define	LLEN	9

/* structure used for passing messages from the
   at and crontab commands to the cron			*/

struct	message {
	char	etype;
	char	action;
	char	fname[FLEN];
	char	logname[LLEN];
} msgbuf;

/* anything below here can be changed */

#define CRONDIR		"/var/spool/cron/crontabs"
#define ATDIR		"/var/spool/cron/atjobs"
#define ACCTFILE	"/var/cron/log"
#define CRONALLOW	"/etc/cron.d/cron.allow"
#define CRONDENY	"/etc/cron.d/cron.deny"
#define ATALLOW		"/etc/cron.d/at.allow"
#define ATDENY		"/etc/cron.d/at.deny"
#define PROTO		"/etc/cron.d/.proto"
#define	QUEDEFS		"/etc/cron.d/queuedefs"
#define	FIFO		"/etc/cron.d/FIFO"
#define DEFFILE		"/etc/default/cron"

#define SHELL		"/usr/bin/sh"	/* shell to execute */

#define CTLINESIZE	1000	/* max chars in a crontab line */
#define UNAMESIZE	20	/* max chars in a user name */
