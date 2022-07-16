/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:ttysrv.h	1.1"
#ident	"@(#)ttysrv.h	1.3"

#define PROC_L		"/usr/net/adm/cfg/ttysrv.pl"
#define	CHGE_PROC_L	"/usr/net/adm/tmp/ttysrv.cp"
#define TSTAB		"/usr/net/adm/cfg/ttysrv.db"

#define	MAXTRIES	5
#define DECREMENT	0
#define	INCREMENT	1
#define	TRUE		1
#define	FALSE		0

/* Streams modules pushed by ttysrv */

#define	NTTY		"ntty"
#define	LD0		"ld0"
#define	RDWR		"tirdwr"

/* Control characters for the menu */

#define	DEL		0177
#define	FS		034
#define	ERASE		010
#define	KILL		0100
#define	EOT		04

/*  Maximum page sizes */

#define	MAXPAGSRV	18
#define	FIELDLEN	80
#define	SRVFIELD	18

#define	MAXLNLEN	512
#define	MAXSYS		256

#define	TSMENU		01	/* Menu was made already */
#define	TSMENUSNT	02	/* Menu of systems sent to user */
#define	TSERROR		04	/* Error message to user */
#define	TSWAIT		010	/* Wait for answer from user */
#define	TSCHGERR	020	/* Error in changing proc file */
#define	TSLOGERR	040	/* Error in opening log */
#define	TSSIGHUP	0100	/* SIGHUP indication */
#define	TSDEFAULT	0200	/* Execute the default entry in the database */
#define	TSEDIT		0400	/* Edited the process limit file successfully*/
#define TSNOLIMIT	01000	/* No process limit file */
#define TSSERVICE	02000	/* Service Executed properly */

#define	HSIZE		2048
#define	FDOUTBNDS	21

struct proclim {
	int count;
	int maxcnt;
};

typedef	char	SERVICES[15];

extern char *calspace[];
extern short calindex;

typedef	char	bool;
