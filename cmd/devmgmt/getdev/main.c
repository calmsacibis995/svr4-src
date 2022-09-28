/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:getdev/main.c	1.5.4.1"



/*
 *  getdev.c
 *
 *  Contains
 *	getdev	Writes on the standard output stream a list of devices
 *		that match certain criteria.
 */

/*
 *  Header Files Referenced
 *	<sys/types.h>	Standard UNIX data types
 *	<stdio.h>	Standard I/O 
 *	<errno.h>	Error handling
 *	<string.h>	String handling
 *	<fmtmsg.h>	Standard message generation 
 *	<devmgmt.h>	Device management
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<fmtmsg.h>
#include	<devmgmt.h>


/*
 *  External References:
 *	malloc()	Allocate space from main memory
 *	getopt()	Parse command-line options
 *	opterr		Error flag for getopt()
 *	optarg		Argument to just parsed option (from getopt())
 *	getdev()	Get the devices that match criteria
 *	exit()		Exit the command
 *	putenv()	Modify the environment
 *	_opendevtab()	Open the device table
 *	_devtabpath()	Get the pathname of the device table
 */

void	       *malloc();
int		getopt();
int		opterr;	
int		optind;
char	       *optarg;
char	      **getdev();
void		exit();		
int		putenv();
int		_opendevtab();
char	       *_devtabpath();

/*
 *  Local Definitions
 *	TRUE		Boolean TRUE value
 *	FALSE		Boolean FALSE value
 *	EX_OK		Exit Value if all went well
 *	EX_ERROR	Exit Value if an error occurred
 *	EX_DEVTAB	Exit Value if the device table couldn't be opened
 
 */

#ifndef	TRUE
#define	TRUE		(1)
#endif

#ifndef	FALSE
#define FALSE		(0)
#endif

#define	EX_OK		0
#define	EX_ERROR	1
#define	EX_DEVTAB	2


/*
 *  Messages:
 *	M_USAGE		Usage error
 *	M_DEVTAB	Can't open the device table
 *	M_NODEV		Device not found in the device table
 *	M_ERROR		Unexpected or internal error
 */

#define	M_USAGE		"usage: getdev [-ae] [criterion [...]] [device [...]]"
#define	M_DEVTAB	"Cannot open the device table: %s"
#define	M_NODEV		"Device not found in the device table: %s"
#define	M_ERROR		"Internal error, errno=%d"


/*
 *  Local (Static) Definitions and macros
 *	buildcriterialist()	Builds the criteria list from the command-line
 *	builddevlist()		Builds the device list from the command-line
 *	lbl			Buffer for the standard message label
 *	txt			Buffer for the standard message text
 *	stdmsg(r,l,s,t)		Write a standard message:
 *				    r	Recoverability flag
 *				    l	Standard label
 *				    s	Severity
 *				    t	Text
 */

static	char  **buildcriterialist();
static	char  **builddevlist();

static	char	lbl[MM_MXLABELLN+1];
static	char	txt[MM_MXTXTLN+1];

#define	stdmsg(r,l,s,t)	(void) fmtmsg(MM_PRINT|MM_UTIL|r,l,s,t,MM_NULLACT,MM_NULLTAG)

/*
 *  getdev [-ae] [criterion [...]] [device [...]]
 *
 *	This command generates a list of devices that match the
 *	specified criteria.
 *  
 *  Options:
 *	-a		A device must meet all of the criteria to be
 *			included in the generated list instead of just
 *			one of the criteria (the "and" flag)
 *	-e		Exclude the devices mentioned from the generated
 *			list.  If this flag is omitted, the devices in the
 *			list are selected from the devices mentioned.
 *
 *  Arguments:
 *	criterion	An <attr><op><value> expression that describes
 *			a device attribute.  
 *			<attr>	is a device attribute
 *			<op> 	may be = != : !: indicating equals, does not
 *				equal, is defined, and is not defined 
 *				respectively
 *			<value>	is the attribute value.  Currently, the only
 *				value supported for the : and !: operators
 *				is *
 *	device		A device to select for or exclude from the generated
 *			list
 *
 *  Exit values:
 *	EX_OK		All went well
 *	EX_ERROR	An error (syntax, internal, or resource) occurred
 *	EX_DEVTAB	The device-table could not be opened for reading
 */

main(argc, argv)
	int	argc;		/* Number of items on the command line */
	char  **argv;		/* List of pointers to the arguments */
{

	/* 
	 *  Automatic data
	 */

	char	      **arglist;	/* List of arguments */
	char	      **criterialist;	/* List of criteria */
	char	      **devicelist;	/* List of devices to search/ignore */
	char	      **fitdevlist;	/* List of devices that fit criteria */
	char	       *cmdname;	/* Simple command name */
	char	       *device;		/* Device name in the list */
	char	       *devtab;		/* The device table name */
	int		exitcode;	/* Value to return to the caller */
	int		sev;		/* Message severity */
	int		optchar;	/* Option character (from getopt()) */
	int		andflag;	/* TRUE if criteria are to be anded */
	int		excludeflag;	/* TRUE if exclude "devices" lists */
	int		options;	/* Options to pass to getdev() */
	int		usageerr;	/* TRUE if syntax error */


	/* Build the message label from the (simple) command name */
	if ((cmdname = strrchr(argv[0], '/')) != (char *) NULL) cmdname++;
	else cmdname = argv[0];
	(void) strcat(strcpy(lbl, "UX:"), cmdname);

	/* Write text-component of messages only (goes away in SVR4.1) */
	(void) putenv("MSGVERB=text");

	/* 
	 *  Parse the command line:
	 *	- Options
	 *	- Selection criteria
	 *	- Devices to include or exclude
	 */

	/* 
	 *  Extract options from the command line 
	 */

	/* Initializations */
	andflag = FALSE;	/* No -a -- Or criteria data */
	excludeflag = FALSE;	/* No -e -- Include only mentioned devices */
	usageerr = FALSE;	/* No errors on the command line (yet) */

	/* 
	 *  Loop until all of the command line options have been parced 
	 */
	opterr = FALSE;			/* Don't let getopt() write messages */
	while ((optchar = getopt(argc, argv, "ae")) != EOF) switch (optchar) {

	/* -a  List devices that fit all of the criteria listed */
	case 'a': 
	    if (andflag) usageerr = TRUE;
	    else andflag = TRUE;
	    break;

	/* -e  Exclude those devices mentioned on the command line */
	case 'e':
	    if (excludeflag) usageerr = TRUE;
	    else excludeflag = TRUE;
	    break;

	/* Default case -- command usage error */
	case '?':
	default:
	    usageerr = TRUE;
	    break;
	}

	/* If there is a usage error, write an appropriate message and exit */
	if (usageerr) {
	    stdmsg(MM_NRECOV, lbl, MM_ERROR, M_USAGE);
	    exit(EX_ERROR);
	}

	/* Open the device file (if there's one to be opened) */
	if (!_opendevtab("r")) {
	    if (devtab = _devtabpath()) {
		(void) sprintf(txt, M_DEVTAB, devtab);
		sev = MM_ERROR;
		exitcode = EX_DEVTAB;
	    } else {
		(void) sprintf(txt, M_ERROR, errno);
		sev = MM_HALT;
		exitcode = EX_ERROR;
	    }
	    stdmsg(MM_NRECOV, lbl, sev, txt);
	    exit(exitcode);
	}

	/* Build the list of criteria and devices */
	arglist = argv + optind;
	criterialist = buildcriterialist(arglist);
	devicelist = builddevlist(arglist);
	options = (excludeflag?DTAB_EXCLUDEFLAG:0)|(andflag?DTAB_ANDCRITERIA:0);

	/* 
	 *  Get the list of devices that meets the criteria requested.  If we 
	 *  got a list (that might be empty), write that list to the standard 
	 *  output file (stdout).
	 */

	exitcode = 0;
	if (!(fitdevlist = getdev(devicelist, criterialist, options))) {
	    exitcode = 1;
	}
	else for (device = *fitdevlist++ ; device ; device = *fitdevlist++)
		(void) puts(device);

	/* Finished */
	exit(exitcode);

#ifdef	lint
	return(exitcode);
#endif
}

/*
 *  char **buildcriterialist(arglist)
 *	char  **arglist
 *
 *	Build a list of pointers to the criterion on the command-line
 *
 *  Arguments:
 *	arglist		The list of arguments on the command-line
 *	
 *  Returns:  char **
 *	The address of the first item of the list of criterion on the
 *	command-line.  This is a pointer to malloc()ed space.
 */

static char  **
buildcriterialist(arglist) 
	char  **arglist;	/* Pointer to the list of argument pointers */
{
	/*
	 *  Automatic data
	 */
	
	char  **pp;			/* Pointer to a criteria */
	char  **allocbuf;		/* Pointer to the allocated data */
	int	ncriteria;		/* Number of criteria found */


	/*
	 *  Search the argument list, looking for the end of the list or 
	 *  the first thing that's not a criteria.  (A criteria is a 
	 *  character-string that contains a colon (':') or an equal-sign ('=')
	 */

	pp = arglist;
	ncriteria = 0;
	while (*pp && (strchr(*pp, '=') || strchr(*pp, ':'))) {
	    ncriteria++;
	    pp++;
	}

	if (ncriteria > 0) {

	    /* Allocate space for the list of criteria pointers */
	    allocbuf = (char **) malloc((ncriteria+1)*sizeof(char **));

	    /* 
	     *  Build the list of criteria arguments 
	     */
	    pp = allocbuf;	/* Beginning of the list */
	    while (*arglist &&			/* If there's more to do ... */
		   (strchr(*arglist, '=') ||	/* and it's a = criterion ... */
		    strchr(*arglist, ':')))	/* or it's a : criterion ... */
			*pp++ = *arglist++;	/* Include it in the list */
	    *pp = (char *) NULL;	/* Terminate the list */

	} else allocbuf = (char **) NULL;	/* NO criteria */
	

	return (allocbuf);
}

/*
 *  char **builddevlist(arglist)
 *	char  **arglist
 *
 *	Builds a list of pointers to the devices mentioned on the command-
 *	line and returns the address of that list.
 *
 *  Arguments:
 *	arglist		The address of the list of arguments to the
 *			getdev command.
 *
 *  Returns:  char **
 *	A pointer to the first item in the list of pointers to devices
 *	specified on the command-line
 */

static char  **
builddevlist(arglist) 
	char  **arglist;	/* Pointer to the list of pointers to args */
{
	/*
	 *  Automatic data
	 */

	/*
	 *  Search the argument list, looking for the end of the list or the 
	 *  first thing that's not a criteria.  It is the first device in the 
	 *  list of devices (if any).
	 */

	while (*arglist && (strchr(*arglist, '=') || strchr(*arglist, ':'))) arglist++;

	/* Return a pointer to the argument list. */
	return(*arglist?arglist:(char **) NULL);
}
