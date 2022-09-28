/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:getdgrp/main.c	1.7.4.1"



/*
 *  main.c
 *
 *  Implements the main body of the "getdgrp" command.
 */

/*
 *  Header Files Referenced
 *	<sys/types.h>	Standard UNIX(r) data types
 *	<stdio.h>	Standard I/O
 *	<errno.h>	Error-handling
 *	<string.h>	Character-string handling
 *	<devmgmt.h>	Device management
 *	<fmtmsg.h>	Standard message generation
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<devmgmt.h>
#include	<fmtmsg.h>


/*
 *  External References:
 *	malloc()	Allocate memory from main memory
 *	exit()		Return to the parent process
 *	getopt()	Parse the command-line
 *	opterr		FLAG for getopt(), TRUE means write messages
 *	optind		Value from getopt(), next argument number
 *	optarg		Value from getopt(), option's argument
 *	putenv()	Put a value into the environment
 *	getdgrp()	Get device groups w/ devices matching criteria
 *	_opendevtab()	Open the device table
 *	_devtabpath()	Get the pathname of the device table
 *	_opendgrptab()	Open the device-group table
 *	_dgrptabpath()	Get the pathname of the device-group table
 */

extern	void   *malloc();
extern	void	exit();
extern	int	getopt();
extern	int	opterr;
extern	int	optind;
extern	char   *optarg;
extern	int	putenv();
extern	char  **getdgrp();
extern	int	_opendevtab();
extern	char   *_devtabpath();
extern	int	_opendgrptab();
extern	char   *_dgrptabpath();

/*
 *  Local Definitions
 *	TRUE		Boolean TRUE value
 *	FALSE		Boolean FALSE value
 *	NULL		Null address
 */

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define FALSE	0
#endif


/*
 * Exit codes:
 *	EX_OK		All's well that ends well
 *	EX_ERROR	Some other error occurred
 *	EX_DTAB		Device table couldn't be opened
 *	EX_DGRP		Device-group table couldn't be open.
 */

#define	EX_OK		0
#define	EX_ERROR	1
#define	EX_DTAB		2
#define	EX_DGRP		2


/*
 *  Messages:
 *	M_USAGE		Command usage error
 *	M_ERROR		Some unexpected error
 *	M_DEVTAB	Device table couldn't be opened
 *	M_DGROUP	Device-group table couldn't be opened
 */
 
#define	M_USAGE		"usage: getdgrp [-ael] [criterion [...]] [dgroup [...]]"
#define	M_ERROR		"Internal error, errno=%d"
#define	M_DEVTAB	"Cannot open the device table: %s"
#define	M_DGROUP	"Cannot open the device-group table: %s"


/*
 *  Internal References
 *	buildcriterialist()	Builds a list of the criteria on the
 *				command line
 *	buildgrouplist()	Builds a list of the device-groups mentioned
 *				on the command line
 */

static	char  **buildcriterialist();	/* Builds criteria list from command line */
static	char  **builddgrouplist();	/* Builds dgroup list from command line */


/*
 *  Macros
 *	stdmsg(r,l,s,t)	    Generate a standard message
 *				r	Recoverability flag
 *				l	Standard label
 *				s	Severity
 *				t	Text
 *	isacriterion(p)	    Returns TRUE if *p is a criterion, FALSE otherwise
 */

#define	stdmsg(r,l,s,t)	(void) fmtmsg(MM_PRINT|MM_UTIL|r,l,s,t,MM_NULLACT,MM_NULLTAG)
#define	isacriterion(p)	(strchr(*arglist,'=')||strchr(*arglist,':'))


/*
 *  Static Variables
 *	lbl		Buffer for standard message label
 *	txt		Buffer for standard message text
 */

static	char	lbl[MM_MXLABELLN+1];
static	char	txt[MM_MXTXTLN+1];

/*
 *  getdgrp [-ael] [criterion [...]] [dgroup [...]]
 *
 *	This function gets the device groups that contain as members devices
 *	that match the given criteria.
 *
 *  Options:
 *	-a	A device must meet all criteria before the device-group in
 *		which it is a member can be selected for inclusion in the
 *		generated list.  If this option is missing, a device must
 *		meet at least one criterion before it's group can be 
 *		selected.  This option has no affect if there are no criterion
 *		on the command-line.
 *	-e	The list of device groups specifies groups to exclude from
 *		the generated list.  If this option is omitted, the list
 *		of groups is the set of groups that can be selected.  This
 *		option has no effect if there are no device-groups on the
 *		command-line.
 *	-l	List all device groups, even those that have no valid
 *		members (this option has no effect if criterion are specified
 *
 *  Arguments:
 *	criterion	A device criterion of the form <attr><op><val> where
 *			<attr> is the name of an attribute, <op> is "=", "!=",
 *			":", or "!:" for "is equal to", "is not equal to", 
 *			"is defined," or "is not defined."  <val> is the value
 *			that the attribute must be equal to or not equal to.
 *			(<val> must be "*" if <op> is ":" or "!:").
 *	dgroup		A device group that is to be exclude selected for the
 *			generated list or excluded from the the generated
 *			list.
 *
 *  Exit values:
 *	0	Success
 *	1	Usage or an internal error
 *	2	The device table or the device-group table could not be
 *		opened for reading
 */

main(argc, argv)
	int	argc;		/* Number of items on the command line */
	char  **argv;		/* List of pointers to the arguments */
{

	/* 
	 *  Automatic data
	 */

	char  **arglist;	/* List of arguments (subset of argv) */
	char  **criterialist;	/* List of criteria */
	char  **dgrouplist;	/* List of device groups to search or ignore */
	char  **fitgrouplist;	/* List of device groups that fit criteria */
	char   *cmdname;	/* Simple command name */
	char   *dgroup;		/* Pointer to device group name in list */
	char   *filename;	/* Pointer to filename in "error" */
	int	exitcode;	/* Value to return to the caller */
	int	sev;		/* Message severity */
	int	optchar;	/* Option character (returned by getopt()) */
	int	andflag;	/* TRUE if anding criteria, FALSE if or'ed */
	int	excludeflag;	/* TRUE if the dgroups list those to exclude */
	int	allflag;	/* TRUE if all device grps are to be displayed */
	int	options;	/* Options to pass to getdgrp() */
	int	usageerr;	/* TRUE if syntax error */


	/* Build the message label from the (simple) command name */
	if (cmdname = strrchr(argv[0], '/')) cmdname++;
	else cmdname = argv[0];
	(void) strcat(strcpy(lbl, "UX:"), cmdname);

	/* Only write the text-component of messages (this goes away in SVR4.1) */
	(void) putenv("MSGVERB=text");

	/* 
	 *  Parse the command line:
	 *	- Options
	 *	- Selection criteria
	 *	- Device groups to include or exclude
	 */

	/* 
	 *  Extract options from the command line 
	 */

	/* Initializations */
	andflag = FALSE;		/* No -a */
	excludeflag = FALSE;		/* No -e */
	allflag = FALSE;		/* No -l */
	usageerr = FALSE;		/* No errors yet */

	/* 
	 *  Loop until all of the command line options have been parced 
	 */
	opterr = FALSE;			/* Don't let getopt() write messages */
	while ((optchar = getopt(argc, argv, "ael")) != EOF) switch (optchar) {

	/* -a  List device groups that fit all of the criteria listed */
	case 'a': 
	    if (andflag) usageerr = TRUE;
	    else andflag = TRUE;
	    break;

	/* -e  Exclude those device groups mentioned on the command line */
	case 'e':
	    if (excludeflag) usageerr = TRUE;
	    else excludeflag = TRUE;
	    break;

	/* -l  List all device groups (if no criteria is specified) */
	case 'l':
	    if (allflag) usageerr = TRUE;
	    else allflag = TRUE;
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
	    if (filename = _devtabpath()) {
		(void) sprintf(txt, M_DEVTAB, filename);
		exitcode = EX_DTAB;
		sev = MM_ERROR;
	    } else {
		(void) sprintf(txt, M_ERROR, errno);
		exitcode = EX_ERROR;
		sev = MM_HALT;
	    }
	    stdmsg(MM_NRECOV, lbl, sev, txt);
	    exit(exitcode);
	}

	/* Open the device file (if there's one to be opened) */
	if (!_opendgrptab("r")) {
	    if (filename = _dgrptabpath()) {
		(void) sprintf(txt, M_DGROUP, filename);
		exitcode = EX_DGRP;
		sev = MM_ERROR;
	    } else {
		(void) sprintf(txt, M_ERROR, errno);
		exitcode = EX_ERROR;
		sev = MM_HALT;
	    }
	    stdmsg(MM_NRECOV, lbl, sev, txt);
	    exit(exitcode);
	}

	/* Build the list of criteria and device groups */
	arglist = argv + optind;
	criterialist = buildcriterialist(arglist);
	dgrouplist = builddgrouplist(arglist);
	options = (excludeflag ? DTAB_EXCLUDEFLAG : 0) | 
		  (andflag ? DTAB_ANDCRITERIA : 0) | 
		  (allflag ? DTAB_LISTALL : 0) ;

	/* 
	 *  Get the list of device groups that meets the criteria requested.  
	 *  If we got a list (that might be empty), write that list to the 
	 *  standard output file (stdout).
	 */

	exitcode = EX_OK;
	if (!(fitgrouplist = getdgrp(dgrouplist, criterialist, options))) {
	    exitcode = EX_ERROR;
	}
	else for (dgroup = *fitgrouplist++ ; dgroup ; dgroup = *fitgrouplist++)
		(void) puts(dgroup);

	/* Finished */
	exit(exitcode);

#ifdef	lint
	return(exitcode);
#endif
}

/*
 *  char **buildcriterialist(arglist)
 *	char **arglist
 *	
 *	This function builds a list of criteria descriptions from the 
 *	list of arguments given.  The list returned is in malloc()ed 
 *	space.
 *
 *  Arguments:
 *	arglist		The address of the first element in the list
 *			of arguments (possibly) containing criterion
 *
 *  Returns:  char **
 *	A pointer to the first element in the list of criterion.
 *	If there was a problem, the function returns (char **) NULL.
 *	If there are no criteria in the list, the function returns
 *	an empty list.
 */

static char  **
buildcriterialist(arglist) 
	char  **arglist;	/* Pointer to the list of argument pointers */
{
	/*
	 *  Automatic data
	 */
	
	char	      **pp; 		/* Pointer to a criteria */
	void	       *allocbuf;	/* Pointer to the allocated data */
	int		ncriteria;	/* Number of criteria found */


	/*
	 *  Search the argument list, looking for the end of the list or 
	 *  the first thing that's not a criteria.  (A criteria is a 
	 *  character-string that contains a colon (':') or an equal-sign ('=')
	 */

	pp = arglist;
	ncriteria = 1;
	while (*pp && (strchr(*pp, '=') || strchr(*pp, ':'))) {
	    ncriteria++;
	    pp++;
	}

	/* Allocate space for the list of criteria pointers */
	if (allocbuf = malloc(ncriteria*sizeof(char **))) {

	    /* Build the list of criteria arguments */
	    pp = (char **) allocbuf;
	    while ((*arglist != (char *) NULL) && isacriterion(*arglist)) *pp++ = *arglist++;
	    *pp = (char *) NULL;
	}

	return ((char **) allocbuf);
}

/*
 *  char **builddgrouplist(arglist)
 *	char  **arglist
 *
 *	This function returns a pointer to the first element in a list of
 *	device-groups (i.e. not criteria) specified in the list of arguments 
 *	whose first element is pointed to by <arglist>.
 *
 *  Arguments:
 *	arglist		The address of the first element in the list of
 *			arguments to be searched for non-criteria
 *	
 *  Returns:  char **
 *	The address of the first item in the list of arguments that are
 *	not criteria.  If none, the function returns a pointer to a
 *	null list.
 *	
 *  Note:
 *	- The current implementation returns a pointer to an element in
 *	  <arglist>.
 */

static char  **
builddgrouplist(arglist) 
	char  **arglist;	/* First item in the list of arguments */
{
	/*
	 *  Automatic data
	 */

	/*
	 *  Search the argument list, looking for the end of the list or 
	 *  the first thing that's not a criteria.  It is the first device 
	 *  group in the list of device groups (if any).
	 */

	while (*arglist && isacriterion(*arglist)) arglist++;

	/* Return a pointer to the argument list. */
	return(arglist);
}
