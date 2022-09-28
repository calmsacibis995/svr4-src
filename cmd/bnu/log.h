/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:log.h	1.1.3.1"

#define	MCHAR		'M'	/* Indicates master */
#define SCHAR		'S'	/* Indicates slave */

#define	CLOSED		(-1)	/* Shows log file is closed. */
#define	EOR		"\n"	/* Unity end of record character. */
#define	LOGSIZE		1024	/* Maximum size of a log record. */
#define	MODSTR		50	/* Value to use for moderate sized strings. */
#define	COMPLETE	'C'	/* Value to use for completed transfer. */
#define	PARTIAL		'P'	/* Value to use for partial transfer. */
#define	NOTAVAIL	"\"\""	/* String to show that a field is not
				 *   available. */
#define	NOTIME		(-1)	/* Value to be used when no times have been
				 *   recorded. */
#ifndef STATIC_FUNC
#define	STATIC_FUNC	static	/* For debugging may not want static
				 *   functions. */
#endif

/* Debug levels: */

#define	DB_IMPORTANT	1	/* This message is printed if debugging is
				 *   turned on at all.  Thus, it should be
				 *   used for the most important messages. */
#define DB_TRACE	4	/* This level is useful in tracing program
				 *   actions. */
#define	DB_DETAIL	9	/* This level will only be printed when
				 *   great detail is needed. */

extern void pfConnected();		/* perfstat.c */
extern void pfEndFile();		/* perfstat.c */
extern void pfEndXfer();		/* perfstat.c */
extern void pfFindFile();		/* perfstat.c */
extern void pfFound();			/* perfstat.c */
extern void pfInit();			/* perfstat.c */
extern void pfStrtConn();		/* perfstat.c */
extern void pfStrtXfer();		/* perfstat.c */
extern void pfPtcl();			/* perfstat.c */
extern void acConnected();		/* account.c */
extern void acDojob();			/* account.c */
extern void acInc();			/* account.c */
extern void acInit();			/* account.c */
extern void acEnd();			/* account.c */
extern void acRexe();			/* account.c */
extern void acEndexe();			/* account.c */
extern void scInit();			/* security.c */
extern void scReqsys();			/* security.c */
extern void scRequser();		/* security.c */
extern void scDest();			/* security.c */
extern void scSrc();			/* security.c */
extern void scStime();			/* security.c */
extern void scEtime();			/* security.c */
extern void scWrite();			/* security.c */
extern void scRexe();			/* security.c */
extern void scWlog();			/* security.c */
extern char * scMtime();		/* security.c */
extern char * scOwn();			/* security.c */
extern char * scSize();			/* security.c */
extern void copyText();			/* perfstat.c */
extern void writeLog();			/* perfstat.c */
extern openLog();			/* perfstat.c */
extern void closeLog();			/* perfstat.c */
extern char *	gmt();			/* perfstat.c */
extern time_t  	cpucycle();		/* account.c */
