/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)listen:lsparam.h	1.11.5.1"

/*
 * lsparam.h:	listener parameters.  Includes default pathnames.
 */

/* DEBUGMODE causes debug statements to be compiled in. */

/*  #define DEBUGMODE   */

#ifdef	DEBUGMODE
#define	DEBUG(ARGS)	debug ARGS
#else
#define	DEBUG(ARGS)
#endif

/*
 * CHARADDR is a debug aid only!!!!
 * with DEBUGMODE, if CHARADDR is defined, logical addresses which
 * are represented by printable characters, will be displayed in the
 * debug/log files
 */

#ifdef	DEBUGMODE
#define CHARADDR
#endif

/* listener parameters							*/

#define MAXNAMESZ	15		/* must coexist with ms-net (5c) */
#define SNNMBUFSZ	16		/* starlan network only		*/
#define NAMEBUFSZ	64
#define MINMSGSZ	(SMBIDSZ+2)	/* smallest acceptable msg size	*/
#define RCVBUFSZ	BUFSIZ		/* receive buffer size		*/
#define DBFLINESZ	BUFSIZ		/* max line size in data base 	*/
#define ALARMTIME	45		/* seconds to wait for t_rcv	*/
#define PATHSIZE	64		/* max size of pathnames	*/

/*
 * LOGMAX is default no of entries maintained
 */

#define LOGMAX	1000			/* default value for Logmax	*/

/*
 * if SMB server is defined, code is included to parse MS-NET messages
 * if undef'ed, the parsing routine logs an approp. error and returns an err.
 */

#define	SMBSERVER	1		/* undef to remove SMBSERVICE support*/

/*
 * if listener (or child) dies, dump core for diagnostic purposes
 */

/* #define COREDUMP */

/* the following filenames are used in homedir:	*/

#define BASEDIR	"/etc/saf"		/* base directory for listen	*/
#define ALTDIR "/var/saf"		/* alternate directory for files*/
#define	LOGNAME	"./log"			/* listener's logfile		*/
#define	OLOGNAME "./o.log"		/* listener's saved logfile	*/
#define	PDEBUGNAME "p_debug"		/* protoserver's debugfile	*/
#define DBGNAME	"debug"			/* debug output file		*/
#define PIDNAME	"./_pid"		/* listener's process id's	*/
#define DBFNAME	"./_pmtab"		/* listener data base file	*/

/* defines for SAC compatibility */

#define	SACPIPE	"../_sacpipe"		/* outgoing messages to SAC	*/
#define	PMPIPE	"./_pmpipe"		/* incoming messages from SAC	*/
#define MAXCLASS	1		/* maximum SAC protocol version */


/*
 * defaults which are normally overriden by cmd line/passwd file, etc
 */

#define NETSPEC	"starlan"
