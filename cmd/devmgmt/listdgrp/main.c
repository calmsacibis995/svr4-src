/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:listdgrp/main.c	1.5.3.1"



/*
 *  listdgrp.c
 *
 *  Contains
 *	listdgrp	Writes on the standard output stream a list of devices
 *			that belong to the specified device group
 */

/*
 *  Header Files Referenced
 *	<sys/types.h>	UNIX(r) System Data Types
 *	<stdio.h>	Standard I/O definitions
 *	<string.h>	String definitions
 *	<errno.h>	Error definitions
 *	<devmgmt.h>	Device management definitions
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<devmgmt.h>
#include	<fmtmsg.h>


/*
 *  External References:
 *	getopt()	Command line parser
 *	optind		Index of the next arg in argv[] to process in getopt()
 *	opterr		Verbosity flag for getopt()
 *	optarg		Argument found for the current option
 *	exit()		Exit from the command
 *	putenv()	Put a value into the environment
 *	listdgrp()	List members of a device group
 *	_opendgrptab()	Open the device group table
 *	_dgrptabpath()	Pathname to the device-group table
 */

extern	int		getopt();
extern	int		opterr;
extern	int		optind;
extern	char	       *optarg;
extern	void		exit();
extern	int		putenv();
extern	char	      **listdgrp();
extern	int		_opendgrptab();
extern	char	       *_dgrptabpath();

/*
 *  Local Definitions
 *	TRUE		Boolean TRUE value (if not already defined)
 *	FALSE		Boolean not-TRUE value (if not already defined)
 *	NULL		Null address (if not already defined)
 */

#ifndef	TRUE
#define	TRUE		('t')
#endif

#ifndef	FALSE
#define	FALSE		(0)
#endif

#ifndef	NULL
#define	NULL		0
#endif


/*
 *  Messages:
 *	M_USAGE		Command usage error
 *	M_NODGRP	Device group not found
 *	M_DGRPTAB	Device-group table not found
 *	M_ERROR		Internal error
 */
 
#define	M_USAGE		"usage: listdgrp dgroup"
#define	M_NODGRP	"Device group not found: %s"
#define	M_DGRPTAB	"Cannot open device-group table: %s"
#define	M_ERROR		"Internal error, errno=%d"


/*
 *  Exit codes
 *	EX_OK		Exiting okay, no problem
 *	EX_ERROR	Some problem with the command
 *	EX_NODGRPTAB	Device group table could not be opened
 *	EX_NODGROUP	Device group couldn't be found
 */

#define	EX_OK		0
#define	EX_ERROR	1
#define	EX_NODGRPTAB	2
#define	EX_NODGROUP	3


/*
 *  Macros
 *	stdmsg(r,l,s,t)	    Write a message in standard format 
 *				r	Recoverability flag
 *				l	Label
 *				s	Severity
 *				t	Tag
 */

#define	stdmsg(r,l,s,t)	(void) fmtmsg(MM_PRINT|MM_UTIL|r,l,s,t,MM_NULLACT,MM_NULLTAG)

/*
 *  Global Variables
 */


/*
 *  Static Variables
 *
 *	lbl	Buffer for the message label
 */

static	char	lbl[MM_MXLABELLN+1];
static	char	msg[MM_MXTXTLN+1];

/*
 *  listdgrp <dgroup>
 *
 *	List the devices that belong to the device group <dgroup>.
 *	It writes the list to the standard output file (stdout)
 *	in a new-line list.
 *
 *  Returns:
 *	0	Ok
 *	1	Syntax or other error
 *	2	Device table can't be opened
 *	3	Device group doesn't exist
 */

main(argc, argv)
	int	argc;			/* Number of items in command */
	char  **argv;			/* List of pointers to the arguments */
{

	/* 
	 *  Automatic data
	 */

	char	      **devices;	/* List of devices in the group */
	char	      **pp;		/* Running pointer to device names */
	char	       *cmdname;	/* Simple command name */
	char	       *dgrptab;	/* The device-group table name */
	char	       *dgroup;		/* Device group to list */
	int		exitcode;	/* Value to return to the caller */
	int		sev;		/* Message severity */
	int		optchar;	/* Option char (from getopt()) */
	int		usageerr;	/* TRUE if syntax error on command */


	/* Build the message label from the (simple) command name */
	if ((cmdname = strrchr(argv[0], '/')) != (char *) NULL) cmdname++;
	else cmdname = argv[0];
	(void) strcat(strcpy(lbl, "UX:"), cmdname);

	/* Only write the text component of a message (this goes away in SVR4.1) */
	(void) putenv("MSGVERB=text");

	/* 
	 *  Parse the command line:
	 *	- Options
	 *	- Device group to display
	 */

	/* 
	 *  Extract options from the command line 
	 */

	/* Initializations */
	usageerr = FALSE;	/* No errors on the command line (yet) */

	/* 
	 *  Loop until all of the command line options have been parced 
	 *  (and don't let getopt() write messages) 
	 */

	opterr = FALSE;	
	while ((optchar = getopt(argc, argv, "")) != EOF) switch (optchar) {

	/* Default case -- command usage error */
	case '?':
	default:
	    usageerr = TRUE;
	    break;
	}

	/* Check the argument count and extract the device group name */
	if (usageerr || (optind != argc-1)) usageerr = TRUE;
	else dgroup = argv[optind];

	/* If there is a usage error, write an appropriate message and exit */
	if (usageerr) {
	    stdmsg(MM_NRECOV, lbl, MM_ERROR, M_USAGE);
	    exit(EX_ERROR);
	}

	/* Open the device-group file (if there's one to be opened) */
	if (!_opendgrptab("r")) {
	    if (dgrptab = _dgrptabpath()) {
		(void) sprintf(msg, M_DGRPTAB, dgrptab);
		exitcode = EX_NODGRPTAB;
		sev = MM_ERROR;
	    } else {
		(void) sprintf(msg, M_ERROR, errno);
		exitcode = EX_ERROR;
		sev = MM_HALT;
	    }
	    stdmsg(MM_NRECOV, lbl, sev, msg);
	    exit(exitcode);
	}


	/*
	 * Get the list of devices associated with the device group.
	 * If we get one, write the list to the standard output.
	 * Otherwise, write an appropriate error message
	 */

	exitcode = EX_OK;
	if (devices = listdgrp(dgroup)) {
	    for (pp = devices ; *pp ; pp++) (void) puts(*pp);
	}
	else {
	    if (errno == EINVAL) {
		(void) sprintf(msg, M_NODGRP, dgroup);
		stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		exitcode = EX_NODGROUP;
	    }
	    else {
		(void) sprintf(msg, M_ERROR, errno);
		stdmsg(MM_NRECOV, lbl, MM_HALT, msg);
		exitcode = EX_ERROR;
	    }
	}

	/* Finished (now wasn't that special?) */
	exit(exitcode);

#ifdef	lint
	return(exitcode);
#endif
}
