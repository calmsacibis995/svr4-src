/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nlsadmin:nlsadmin.h	1.15.4.1"

/*
 * nlsadmin.h:  nlsadmin defines and structs
 */

#define VERSION		4		/* current database version */
#define MAXNAMESZ	15
#define SVC_CODE_SZ	14
#define DEFAULTID	"listen"	/* default id for servers */
#define LISTENTYPE	"listen"	/* pm type for listener service */
#define	ROOT		(uid_t) 0

#define NLPSSRV		"/usr/lib/saf/nlps_server"	/* full path of nlps server */
#define LISTENCMD	"/usr/lib/saf/listen"		/* full path of the listener */
#define	NLPSSVCCODE	"0"		/* (string!) protoserv service code */
#define	TTYSVCCODE	"1"		/* (string!) ttyserv service code */

/* 
 * bit flags for argument processing
 */

#define NONE    0x0000
#define	CMDFLAG	0x0001
#define	PIPFLAG	0x0002
#define	VERFLAG	0x0004
#define DISFLAG 0x0008
#define ENAFLAG 0x0010
#define KILFLAG 0x0020
#define ADRFLAG 0x0040
#define REMFLAG 0x0080
#define STAFLAG 0x0100
#define VBSFLAG 0x0200
#define NETFLAG 0x0400
#define ZZZFLAG 0x0800
#define INIFLAG	0x1000

/*
 * other misc defines
 */

#define INCONSISTENT	1
#define MISSINGARG	2
#define USAGE		3

#define NOTLISTEN	1
#define BADPMFMT	2
#define BADLISFMT	3

#define CFLAG		0x1
#define PFLAG		0x2
#define DFLAG		0x4

extern  char    *optarg;
extern  int     optind;
extern  int     errno;

/*
 * error returns from nlsadmin (1 is reserved for passing non-error
 * information (-q))
 */

#define NLS_OK		0		/* no error */
#define NLS_FAILED	2		/* failure in command */
#define NLS_PERM	3		/* must be root */
#define NLS_SERV	4		/* error in service code */
#define	NLS_CMD		5		/* command line error */
#define	NLS_SYSERR	6		/* system error during cmd */
#define	NLS_BADPM	7		/* bad port monitor */

/*
 * command lines for SAC functions
 */

#define SAC_LSPM	"/usr/sbin/sacadm -L -p %s 2>/dev/null"
#define SAC_LSTY	"/usr/sbin/sacadm -L -t %s 2>/dev/null"
#define SAC_KILLPM	"/usr/sbin/sacadm -k -p %s 2>/dev/null"
#define SAC_STARTPM	"/usr/sbin/sacadm -s -p %s 2>/dev/null"
#define SAC_ENABLPM	"/usr/sbin/sacadm -e -p %s 2>/dev/null"
#define SAC_ADDPM	"/usr/sbin/sacadm -a -p %s -t %s -c \"%s\" -v %d 2>/dev/null"
#define PM_DISABLE	"/usr/sbin/pmadm -d -p %s -s %s 2>/dev/null"
#define PM_ENABLE	"/usr/sbin/pmadm -e -p %s -s %s 2>/dev/null"
#define PM_LSALL	"/usr/sbin/pmadm -L -p %s 2>/dev/null"
#define PM_LSONE	"/usr/sbin/pmadm -L -p %s -s %s 2>/dev/null"
#define PM_REMSVC	"/usr/sbin/pmadm -r -p %s -s %s 2>/dev/null"
#define PM_ADDSVC	"/usr/sbin/pmadm -a -p %s -s %s -i %s -m %s -v %d -y \"%s\" 2>/dev/null"
#define PM_ADDSVCF	"/usr/sbin/pmadm -a -p %s -s %s -i %s -f %s -m %s -v %d -y \"%s\" 2>/dev/null"
