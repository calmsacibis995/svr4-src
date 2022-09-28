/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:putdgrp/main.c	1.6.6.1"



/*
 * main.c
 *
 *	Implements the "putdgrp" command.
 */

/*
 *  G L O B A L   R E F E R E N C E S
 */

/*
 * Header files needed:
 *	<sys/types.h>	UNIX(r) data types
 *	<stdio.h>	Standard I/O definitions
 *	<string.h>	String handling definitions
 *	<errno.h>	Error codes
 *	<unistd.h>	Standard Identifiers
 *	<fmtmsg.h>	Standard Message Generation definitions
 *	<devmgmt.h>	Device Management definitions
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<unistd.h>
#include	<fmtmsg.h>
#include	<devmgmt.h>


/*
 * Externals referenced (and not defined in header files)
 *	optind		index to next arg on the command line (from getopt())
 *	opterr		FLAG, TRUE tells getopt() to suppress error messages
 *	optarg		Ptr to current option's argument
 *	getopt()	Command-line parsing function
 *	malloc()	Allocate a chunk of memory
 *	putenv()	Put a value into the environment
 *	access()	Check the accessibility of a file
 *	exit()		Exit a process, returning to the parent
 */

extern	int	optind;
extern	int	opterr;
extern	char   *optarg;
extern	int	getopt();
extern	void   *malloc();
extern	int	putenv();
extern	int	access();
extern	void	exit();

/*
 * Device Management Globals referenced:
 *	_adddgrptabrec()	Add a device-group table entry to the 
 *				device-group table
 *	_rmdgrptabrec()		Remove a device-group table entry in the 
 *				device-group table
 *	_rmdgrpmems()		Remove members from a device-group
 *	_dgrptabpath()		Get the pathname of the device-group table
 */

int	_adddgrptabrec();
int	_rmdgrptabrec();
int	_rmdgrpmems();
char   *_dgrptabpath();

/*
 *  L O C A L   D E F I N I T I O N S
 */

/*
 * General Purpose Constants
 *	TRUE		Boolean TRUE (if not already defined)
 *	FALSE		Boolean FALSE (if not already defined)
 *	NULL		Null address (if not already defined)
 */

#ifndef	TRUE
#define	TRUE	(1)
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	NULL
#define	NULL	(0)
#endif


/*
 * Exit codes
 *	EX_OK		All went well
 *	EX_ERROR	Usage or internal error 
 *	EX_DGROUP	Had trouble accessing/reading/writing the 
 *			device-group table
 *	EX_NODGRP	The specified device-group does not exist
 *	EX_NOMEM	One or more device-group members requested for 
 *			removal was not defined for the device
 */

#define	EX_OK		0
#define	EX_ERROR	1
#define	EX_DGROUP	2
#define	EX_NODGRP	3
#define	EX_NOMEM	4


/*
 * Error messages
 */

#define	E_USAGE		"usage: putdgrp [-d] dgroup [device [...]]"
#define	E_NODGRP	"Device-group does not exist in table: %s"
#define	E_NOTAMEM	"Device-group member not found: %s"
#define	E_NODGRPTAB	"Cannot open the device-group table: %s"
#define	E_NOMKTAB	"Cannot create a new device-group table: %s"
#define	E_INTERNAL	"Internal error, errno=%d"


/*
 * Macros
 *	stdmsg(r,l,s,t)	    Using fmtmsg(), write a standard message to the
 *			    standard error stream.
 *			    Where:
 *				r   The recoverability of the error
 *				l   The label-component
 *				s   The severity-component
 *				t   The text-component
 */

#define stdmsg(r,l,s,t) (void) fmtmsg(MM_PRINT|MM_UTIL|r,l,s,t,MM_NULLACT,MM_NULLTAG)


/*
 * Static data
 *	msg		Space for message's text-component
 */

static	char		msg[256];	/* Space for text of message */

/*
 * char *mklbl(cmd)
 *	char   *cmd
 *
 *	This function builds a standard label from the command used to invoke
 *	this process and the standard label prefix ("UX:")
 *
 * Arguments:
 *	char *cmd	The command used to invoke this process.
 *
 * Returns:  char *
 *	Pointer to malloc()ed space containing the standard label,
 *	or (char *) NULL if an error occurred.
 */
  
static char *
mklbl(cmd)
	char   *cmd;
{
	/* Automatic data */
	char   *rtn;		/* Value to return */
	char   *p;		/* Temporary */

	/* Find the 1st char of the basename of the command */
	if (p = strrchr(cmd, '/')) p++;
	else p = cmd;

	/* Allocate and build the string value to return */
	if (rtn = (char *) malloc(strlen("UX:")+strlen(p)+1)) {
	    (void) strcpy(rtn, "UX:");
	    (void) strcat(rtn, p);
	}

	/* Now that we've done all of this work, set up the environemnt
	 * so that only the text-component is written (some requirements
	 * say that standard messages are to be non-standard in SVR4.0,
	 * this is supposed to change in SVR4.1)
	 */

	(void) putenv("MSGVERB=text");

	/* Done */
	return(rtn);
}

/*
 * putdgrp [-d] dgroup [device [...]]
 *
 * Options:
 *	-d		
 *
 * Arguments:
 *	dgroup
 *	device
 *
 * Exit values:
 */

main(argc, argv)
	int	argc;			/* Argument count */
	char   *argv[];			/* Argument list */
{
	/* Automatic data */
	char	      **plist;		/* Ptr to list of nonmembers */
	char	       *lbl;		/* Ptr to label for messages */
	char	       *dgroup;		/* Ptr to <dgroup> on command-line */
	char	       *p;		/* Temp ptr to char */
	int		noerr;		/* FLAG, TRUE if all's well */
	int		d_seen;		/* TRUE if -a seen on command-line */
	int		optchar;	/* Option extracted */
	int		exitcd;		/* Value to return at exit */
	int		nmems;		/* Number of members on the cmd */

	
	/* Generate the label for messages */
	lbl = mklbl(argv[0]);

	/* Extract arguments - validate usage */
	noerr = TRUE;
	d_seen = FALSE;
	opterr = FALSE;
	while ((optchar = getopt(argc, argv, "d:")) != EOF) switch (optchar) {

	case 'd':
	    if (!d_seen) 
	    {
		d_seen = TRUE;
		dgroup = optarg;
	    }
	    else noerr = FALSE;
	    break;

	case '?':
	default:
	    noerr = FALSE;
	}


	/* Write a usage message if we've seen a blatant error */
	if (!noerr || (!d_seen && ((nmems = argc - optind - 1) < 0)) || 
		      (d_seen && ((nmems = argc - optind) < 0))) {
	    stdmsg(MM_NRECOV, lbl, MM_ERROR, E_USAGE);
	    exit(EX_ERROR);
	}


	/* Set up */
	exitcd = EX_OK;


	/* -d on the command line ? */
	if (d_seen) {

	    /* 
	     * Determine case (removing a device group or members
	     * of that device group.
	     */

	    if (nmems == 0) {

		/* putdgrp -d dgroup */

		/* Attempt to remove the specified device */
		if (!(_rmdgrptabrec(dgroup))) switch(errno) {

		    /*
		     * EINVAL indicates that the named device-group was
		     * not found in the device-group table.
		     */

		case EINVAL:
		    (void) sprintf(msg, E_NODGRP, dgroup);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_NODGRP;
		    break;

		    /*
		     * ENOENT indicates that the device-group table can't
		     * be found.
		     */

		case ENOENT:
		    (void) sprintf(msg, E_NODGRPTAB, _dgrptabpath());
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_DGROUP; 
		    break;

		    /*
		     * EACCES indicates that there was a problem reading the 
		     * old device-group table or creating the new table.  If the
		     * old table is readable, assume that we can't create the
		     * new table.  Otherwise, assume that the old table isn't
		     * accessible.
		     */

		case EACCES:
		    p = _dgrptabpath();
		    if (access(p, R_OK) == 0) 
			(void) sprintf(msg, E_NOMKTAB, p);
		    else 
			(void) sprintf(msg, E_NODGRPTAB, p);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_DGROUP; 
		    break;

		    /*
		     * Some strange problem...
		     */

		default:
		    (void) sprintf(msg, E_INTERNAL, errno);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_ERROR;

		}   /* End switch */
	    }
	    else {

		/* putdgrp -d dgroup device [device [...]] */

		/* 
		 * Attempt to remove the specified devices from the
		 * specified device-group.
		 */
		if (!(_rmdgrpmems(dgroup, &argv[optind], &plist))) switch(errno) {

		    /*
		     * ENODEV indicates that a named device was not part
		     * of the specified device group.
		     */

		case ENODEV:
		    exitcd = EX_NOMEM;
		    for (; *plist; plist++) {
			(void) sprintf(msg, E_NOTAMEM, *plist);
			stdmsg(MM_RECOVER, lbl, MM_WARNING, msg);
		    }
		    break;

		    /*
		     * EINVAL indicates that the named device-group is not 
		     * defined in the device-group table.
		     */

		case EINVAL:
		    (void) sprintf(msg, E_NODGRP, dgroup);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_NODGRP;
		    break;

		    /*
		     * ENOENT indicates that the device table can't
		     * be found.
		     */

		case ENOENT:
		    (void) sprintf(msg, E_NODGRPTAB, _dgrptabpath());
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_DGROUP; 
		    break;

		    /*
		     * EACCES indicates that there was a problem reading the 
		     * old device table or creating the new table.  If the
		     * old table is readable, assume that we can't create the
		     * new table.  Otherwise, assume that the old table isn't
		     * accessible.
		     */

		case EACCES:
		    p = _dgrptabpath();
		    if (access(p, R_OK) == 0) 
			(void) sprintf(msg, E_NOMKTAB, p);
		    else 
			(void) sprintf(msg, E_NODGRPTAB, p);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_DGROUP; 
		    break;

		    /*
		     * Some strange problem...
		     */

		default:
		    (void) sprintf(msg, E_INTERNAL, errno);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		    exitcd = EX_ERROR;

		}  /* End switch */

	    }   /* End "putdgrp -d device attr [...]" case */


	}   /* End -d case */

	else {
	    /* Standard case (no -d on the command) */
	    if (!(_adddgrptabrec(argv[optind], &argv[optind+1]))) switch(errno) {

		/*
		 * ENOENT indicates that the device-group table does not exist.
		 */

	    case ENOENT:
		(void) sprintf(msg, E_NODGRPTAB, _dgrptabpath());
		stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		exitcd = EX_DGROUP; 
		break;

		/*
		 * EACCES indicates that the device-group table could not be
		 * opened or the new device-group table could not be created.
		 */

	    case EACCES:
		p = _dgrptabpath();
		if (access(p, R_OK) == 0)
			(void) sprintf(msg, E_NOMKTAB, p);
		else
			(void) sprintf(msg, E_NODGRPTAB, p);
		stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		exitcd = EX_DGROUP; 
		break;

		/* 
		 * Some strange error (memory?) 
		 */

	    default:
		(void) sprintf(msg, E_INTERNAL, errno);
		stdmsg(MM_NRECOV, lbl, MM_ERROR, msg);
		exitcd = EX_ERROR;
	    }
	}
	

	/* Done.  Return exit code (determined above) */
	exit(exitcd);

#ifdef	lint
	return(exitcd);
#endif
}  /* main() */
