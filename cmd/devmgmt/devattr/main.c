/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:devattr/main.c	1.7.3.1"



/*
 *  devattr.c
 *
 *  Contains the following:
 *	devattr		Command that returns [specific] attributes for
 *			a device.
 */

/*
 *  devattr [-v] device [attr [...]]
 *
 *	This command searches the device table file for the device specified.
 *	If it finds the device (matched either by alias or major and minor 
 *	device number), it extracts the attribute(s) specified and writes 
 *	that value to the standard output stream (stdout).
 *
 *	The command writes the values of the attributes to stdout one per 
 *	line, in the order that they were requested.  If the -v option is 
 *	requested, it writes the attributes in the form <attr>='<value>' where
 *	<attr> is the name of the attribute and <value> is the value of that 
 *	attribute.
 *
 *  Returns:
 *	0	The command succeeded
 *	1	The command syntax is incorrect,
 *		An invalid option was used,
 *		An internal error occurred that prevented completion
 *	2	The device table could not be opened for reading.
 *	3	The requested device was not found in the device table
 *	4	A requested attribute was not defined for the device
 */

/*
 *  Header files referenced:
 *	<sys/types.h>	UNIX System Data Types
 *	<stdio.h>	C standard I/O definitions
 *	<string.h>	C string manipulation definitions
 *	<errno.h>	Error-code definitions
 *	<fmtmsg.h>	Standard message display definitions
 *	<devmgmt.h>	Device management headers
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<fmtmsg.h>
#include	<devmgmt.h>


/*
 *  External functions and variables referenced (not defined by header files)
 *
 *	getopt()	Function that parses options from the command-line
 *	optind		Index of the next argument to process
 *	opterr		Flag for getopt() TRUE tells the command to write 
 *			an error message, FALSE disables writing error-
 *			messages by getopt().
 *	putenv()	Put a value into the environment
 *	exit()		Return to invocation environment
 *	_opendevtab()	Open the device table
 *	_devtabpath()	Get the pathname of the device table
 */

int		getopt();
int		optind;
int		opterr;
int		putenv();
void		exit();
int		_opendevtab();
char	       *_devtabpath();

/*
 *  Local constant definitions
 *	TRUE		Boolean TRUE
 *	FALSE		Boolean FALSE
 *	NULL		Null address
 */

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	NULL
#define	NULL	0
#endif


/*
 *  Messages
 *	M_USAGE		Usage error
 *	M_ERROR		Unexpected internal error
 *	M_NODEV		Device not found in the device table
 *	M_NOATTR	Attribute not found
 *	M_DEVTAB	Can't open the device table
 */

#define	M_USAGE		"usage: devattr [-v] device [attribute [...]]"
#define	M_ERROR		"Internal error, errno=%d"
#define	M_NODEV		"Device not found in the device table: %s"
#define	M_NOATTR	"Attrubute not found: %s"
#define	M_DEVTAB	"Cannot open the device table: %s"


/* 
 * Exit codes:
 *	EX_OK		All's well that ends well
 *	EX_ERROR	Some problem caused termination
 *	EX_DEVTAB	Device table could not be opened
 *	EX_NODEV	The device wasn't found in the device table
 *	EX_NOATTR	A requested attribute wasn't defined for the device
 */

#define	EX_OK 		0
#define	EX_ERROR	1
#define	EX_DEVTAB	2
#define	EX_NODEV	3
#define	EX_NOATTR	4


/*
 *  Macros
 *	stdmsg(r,l,s,t)	    Standard Message Generator
 *				r	Recoverability flag
 *				l	Standard Label
 *				s	Severity
 *				t	Text
 */

#define	stdmsg(r,l,s,t)	(void) fmtmsg(MM_PRINT|MM_UTIL|r,l,s,t,MM_NULLACT,MM_NULLTAG)


/*
 *  Local static data
 *	lbl	Buffer for the command label (for messages)
 *	txt		Buffer for the text of messages
 */

static char	lbl[MM_MXLABELLN+1];
static char	txt[MM_MXTXTLN+1];

/*
 *  main()
 *
 *	Implements the command "devattr".   This function parses the command 
 *	line, then calls the devattr() function looking for the specified 
 *	device and the requested attribute.  It writes the information to
 *	the standard output file in the requested format.
 *
 * Exits:
 *	0	The command succeeded
 *	1	The command syntax is incorrect,
 *		An invalid option was used,
 *		An internal error occurred that prevented completion
 *	2	The device table could not be opened for reading.
 *	3	The requested device was not found in the device table
 *	4	A requested attribute was not defined for the device
 */

main(argc, argv)
	int	argc;		/* Number of arguments */
	char   *argv[];		/* Pointer to arguments */
{

	/* Automatic data */
	char   *cmdname;		/* Pointer to command name */
	char   *device;			/* Pointer to device name */
	char   *attr;			/* Pointer to current attribute */
	char   *value;			/* Pointer to current attr value */
	char   *p;			/* Temporary character pointer */
	char  **argptr;			/* Pointer into argv[] list */
	int	syntaxerr;		/* TRUE if invalid option seen */
	int	noerr;			/* TRUE if all's well in processing */
	int	v_seen;			/* TRUE if -v is on the command-line */
	int	exitcode;		/* Value to return */
	int	severity;		/* Message severity */
	int	c;			/* Temp char value */


	/*
	 *  Parse the command-line.
	 */

	syntaxerr = FALSE;
	v_seen = FALSE;

	/* Extract options */
	opterr = FALSE;
	while ((c = getopt(argc, argv, "v")) != EOF) switch(c) {

	    /* -v option:  No argument, may be seen only once */
	    case 'v':
		if (!v_seen) v_seen = TRUE;
		else syntaxerr = TRUE;
		break;

	    /* Unknown option */
	    default:
		syntaxerr = TRUE;
	    break;
	}

	/* Build the command name */
	cmdname = argv[0];
	if ((p = strrchr(cmdname, '/')) != (char *) NULL) cmdname = p+1;
	(void) strcat(strcpy(lbl, "UX:"), cmdname);

	/* Make only the text-component of messages appear (remove this in SVR4.1) */
	(void) putenv("MSGVERB=text");

	/* 
	 * Check for a usage error 
	 *  - invalid option
	 *  - arg count < 2
	 *  - arg count < 3 && -v used 
	 */

	if (syntaxerr || (argc < (optind+1))) {
	    stdmsg(MM_NRECOV, lbl, MM_ERROR, M_USAGE);
	    exit(EX_ERROR);
	}

	/* Open the device file (if there's one to be opened) */
	if (!_opendevtab("r")) {
	    if (p = _devtabpath()) {
		(void) sprintf(txt, M_DEVTAB, p);
		exitcode = EX_DEVTAB;
		severity = MM_ERROR;
	    } else {
		(void) sprintf(txt, M_ERROR, errno);
		exitcode = EX_ERROR;
		severity = MM_HALT;
	    }
	    stdmsg(MM_NRECOV, lbl, severity, txt);
	    exit(exitcode);
	}


	/* 
	 *  Get the list of known attributes for the device.  This does 
	 *  two things.  First, it verifies that the device is known in the 
	 *  device table.  Second, it gets the attributes to list just in 
	 *  case no attributes were specified.  Then, set a pointer to the 
	 *  list of attributes to be extracted and listed...  
	 */

	device = argv[optind];
	if ((argptr = listdev(device)) == (char **) NULL) {
	    if (errno == ENODEV) {
		(void) sprintf(txt, M_NODEV, device);
		exitcode = EX_NODEV;
		severity = MM_ERROR;
	    } else {
		(void) sprintf(txt, M_ERROR, errno);
		exitcode = EX_ERROR;
		severity = MM_HALT;
	    }
	    stdmsg(MM_NRECOV, lbl, severity, txt);
	    exit(exitcode);
	}
	if (argc > (optind+1)) argptr = &argv[optind+1];


	/*
	 *  List attributes.  If a requested attribute is not defined, 
	 *  list the value of that attribute as null.  (Using shell 
	 *  variables as the model for this.)
	 */

	exitcode = EX_OK;
	noerr = TRUE;
	while (noerr && ((attr = *argptr++) != (char *) NULL)) {
	    if (!(value = devattr(device, attr))) {
		if (errno == EINVAL) {
		    value = "";
		    (void) sprintf(txt, M_NOATTR, attr);
		    /* stdmsg(MM_RECOVER, lbl, MM_WARNING, txt); */
		    exitcode = EX_NOATTR;
		} else {
		    noerr = FALSE;
		    (void) sprintf(txt, M_ERROR, errno);
		    stdmsg(MM_NRECOV, lbl, MM_ERROR, txt);
		    exitcode = EX_ERROR;
		}
	    }
	    if (noerr && v_seen) {
		(void) fputs(attr, stdout);
		(void) fputs("='", stdout);
		for (p = value ; *p ; p++) {
		    (void) putc(*p, stdout);
		    if (*p == '\'') (void) fputs("\"'\"'", stdout);
		}
		(void) fputs("'\n", stdout);
	    } else if (noerr) {
		(void) fputs(value, stdout);
		(void) putc('\n', stdout);
	    }
	}

	/* Exit */
	exit(exitcode);
#ifdef	lint
	return(0);
#endif
}
