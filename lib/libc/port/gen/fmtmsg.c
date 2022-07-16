/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/fmtmsg.c	1.9"

/*LINTLIBRARY*/

/*
 * fmtmsg.c
 *
 *  Contains:
 *	fmtmsg()	Writes a message in standard format.
 *	addseverity()	Adds a severity definition to the list of known
 *			severity definitions.
 *
 *	Notes:
 *	  - Using putc() instead of fputc() because putc() is slightly
 *	    faster (but causes the code to be slightly larger).  If space
 *	    becomes a problem, change putc() to fputc().
 *	  - None of these functions can use strtok().
 */

/*
 * Header Files Referenced:
 *	<stdio.h>		C Standard I/O Definitions
 *	<string.h>		C string handling definitions
 *	<fcntl.h>		UNIX file control definitions
 *	<errno.h>		UNIX error numbers and definitions
 *	<fmtmsg.h>		Global definitions for fmtmsg()
 *	<stdlib.h>		miscellaneous function declarations
 */

#ifdef __STDC__
	#pragma weak fmtmsg = _fmtmsg
	#pragma weak addseverity = _addseverity
#endif
#include	"synonyms.h"
#include	"shlib.h"
#include	<stdio.h>
#include	<string.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<fmtmsg.h>
#include	<stdlib.h>

/*
 * External functions referenced:
 *	(Others may be defined in header files above)
 *
 *	getenv		Extracts data from the environment
 *	malloc		Allocates space from main memory
 *	free		Frees space allocated via malloc()
 *	strtol		Convert string to "long"
 *	clearerr	Clears an error on a stream (this is to make "lint" happy)
 */

#ifdef	lint
extern	void		clearerr();
#endif

/*
 * Local Constant Definitions
 */

/*
 * Boolean constants
 *	TRUE	Boolean value for "true" (any bits on)
 *	FALSE	Boolean value for "false" (all bits off)
 */

#ifndef	FALSE
#define	FALSE		(0)
#endif

#ifndef TRUE
#define	TRUE		(1)
#endif


/*
 * Keywords for fields named in the MSGVERB environment variable.
 */

#define	ST_LBL		"label"
#define	ST_SEV		"severity"
#define	ST_TXT		"text"
#define	ST_TAG		"tag"
#define	ST_ACT		"action"


/*
 *	The following constants define the value of the "msgverb"
 *	variable.  This variable tells fmtmsg() which parts of the
 *	standard message it is to display.  If !(msgverb&MV_SET),
 *	fmtmsg() will interrogate the "MSGVERB" environment variable
 *	and set "msgverb" accordingly.
 *
 *	NOTE:  This means that if MSGVERB changes after the first call
 *	       to fmtmsg(), it will be ignored.
 *
 *	Constants:
 *		MV_INV	Check MSGVERB environment variable (invalidates value)
 *		MV_SET	MSGVERB checked, msgverb value valid
 *		MV_LBL	"label" selected
 *		MV_SEV	"severity" selected
 *		MV_TXT	"text" selected
 *		MV_TAG	"messageID" selected
 *		MV_ACT	"action" selected
 *
 *		MV_ALL	All components selected
 *		MV_DFLT	Default value for MSGVERB
 */

#define MV_INV		0
#define	MV_SET		0x0001
#define	MV_LBL		0x0002
#define	MV_SEV		0x0004
#define	MV_TXT		0x0008
#define	MV_TAG		0x0010
#define	MV_ACT		0x0020

#define MV_ALL		(MV_LBL|MV_SEV|MV_TXT|MV_TAG|MV_ACT)
#define MV_DFLT		(MV_LBL|MV_SEV|MV_TXT|MV_TAG|MV_ACT)



/*
 * Strings defining the different severities of a message.
 * Internationalization may demand that these come from the message database
 */

#define	SV_UNK		"UNKNOWN"
#define	SV_HALT		"HALT"
#define SV_ERROR	"ERROR"
#define SV_WARN		"WARNING"
#define SV_INF		"INFO"


/*
 * Text string if none is provided:
 */

#define	DEFLT_TEXT	"No text provided with this message"


/*
 * Text string introduction for "action".  This may have to come from
 * the message database because of internationalization.
 */

#define ACTINTRO	"TO FIX: "
#define ACTINTROLN	8


/*
 * SEPSTR is the string that separates the "label" from what follows it,
 * and the severity from what follows it.
 */

#define SEPSTR		": "
#define SEPSTRLN	2


/*
 * Miscellaneous constants:
 *	CONNAME		Filesystem entry name for the system console
 */

#define CONNAME		"/dev/console"

/*
 * Local data type definitions
 */

/*
 * Severity string structure
 *
 *	struct sevstr
 *		sevvalue	Value of the severity-level being defined
 *		sevkywd		Keyword identifying the severity
 *		sevprptr	Pointer to the string associated with the value
 *		sevnext		Pointer to the next value in the list.
 *
 *	Restrictions:
 *		sevvalue	Must be a non-negative integer (>=0)
 *
 *	There are three (possibly null) lists of these structures.
 *	  1)	is the list of standard severities
 *	  2)	is the list of severity-levels defined by SEV_LEVEL
 *	  3)	is the list of severity-levels defined by calls to
 *		addseverity()
 */

struct sevstr {
	int		sevvalue;
	const char     *sevkywd;
	const char     *sevprstr;
	struct sevstr  *sevnext;
};

/*
 * Local Static Data
 *	msgverb		int
 *			Contains the internal representation or the
 *			MSGVERB environment variable.
 *	sevlook		TRUE if fmtmsg() has to look at SEV_LEVEL the
 *			next time it is called.
 *	paugsevs	struct sevstr *
 *			Head of the linked list of structures that define
 *			severities that augment the standard severities,
 *			as defined by addseverity().
 *	penvsevs	struct sevstrs *
 *			Head of the linked list of structures that define
 *			severities that augment the standard severities,
 *			as defined by SEV_LEVEL.
 *	pstdsevs	struct sevstrs *
 *			Head of the linked list of structures that define
 *			the standard severities.
 */

static	int		msgverb		= 0;
static	int		sevlook		= TRUE;

static	struct sevstr  *paugsevs	= (struct sevstr *) NULL;
static	struct sevstr  *penvsevs	= (struct sevstr *) NULL;

static	struct sevstr	sevstrs[]	=
		{{MM_HALT,     "", SV_HALT,	&sevstrs[1]},
		 {MM_ERROR,    "", SV_ERROR,	&sevstrs[2]},
		 {MM_WARNING,  "", SV_WARN, 	&sevstrs[3]},
		 {MM_INFO,     "", SV_INF,  	(struct sevstr *) NULL},
	        };
static	struct sevstr  *pstdsevs	= &sevstrs[0];

/*
 * static char *exttok(str, delims)
 *	const char   *str
 *	const char   *delims
 *
 *	This function examines the string pointed to by "str", looking
 *	for the first occurrence of any of the characters in the string
 *	whose address is "delims".  It returns the address of that
 *	character or (char *) NULL if there was nothing to search.
 *
 * Arguments:
 *	str	Address of the string to search
 *	delims	Address of the string containing delimiters
 *
 * Returns:  char *
 *	Returns the address of the first occurrence of any of the characters
 *	in "delim" in the string "str" (incl '\0').  If there was nothing
 *	to search, the function returns (char *) NULL.
 *
 * Notes:
 *    - This function is needed because strtok() can't be used inside a
 *	function.  Besides, strtok() is destructive in the string, which
 *	is undesirable in many circumstances.
 *    - This function understands escaped delimiters as non-delimiters.
 *	Delimiters are escaped by preceding them with '\' characters.
 *	The '\' character also must be escaped.
 */

#if __STDC__
static char *
exttok(const char *tok, const char *delims)
#else
static char *
exttok(tok, delims)
	char   *tok;		/* Ptr to the token we're parsing */
	char   *delims;		/* Ptr to string with delimiters */
#endif
{

	/* Automatic Data */
	char   *tokend;		/* Ptr to the end of the token */
	char   *p, *q;	 	/* Temp pointers */
	int	done;		/* Loop control flag */


	/* Algorithm:
	 *    1.  Get the starting address (new string or where we
	 *	  left off).  If nothing to search, return (char *) NULL
	 *    2.  Find the end of the string
	 *    3.  Look for the first unescaped delimiter closest to the 
	 *	  beginning of the string
	 *    4.  Remember where we left off
	 *    5.  Return a pointer to the delimiter we found
	 */

	/* Begin at the beginning, if any */
	if (tok == (char *) NULL) {
	    return ((char *) NULL);
	}

	/* Find end of the token string */
	tokend = tok + strlen(tok);

	/* Look for the 1st occurrence of any delimiter */
	for (p = delims ; *p != '\0' ; p++) {
	    for (q = strchr(tok, *p) ; q && (q != tok) && (*(q-1) == '\\') ; q = strchr(q+1, *p)) ;
	    if (q && (q < tokend)) tokend = q;
	}

	/* Done */
	return(tokend);
}

/*
 * char *noesc(str)
 *	
 *	This function squeezes out all of the escaped character sequences
 *	from the string <str>.  It returns a pointer to that string.
 *
 *  Arguments:
 *	str	char *
 *		The string that is to have its escaped characters removed.
 *
 *  Returns:  char *
 *	This function returns its argument <str> always.
 *
 *  Notes:
 *	This function potentially modifies the string it is given.
 */

static char *
noesc(str) 
	char   *str;		/* String to remove escaped characters from */
{
	char   *p;		/* Temp string pointer */
	char   *q;		/* Temp string pointer */

	/* Look for an escaped character */
	p = str;
	while (*p && (*p != '\\')) p++;


	/* 
	 * If there was at least one, squeeze them out 
	 * Otherwise, don't touch the argument string 
	 */

	if (*p) {
	    q = p++;
	    while (*q++ = *p++) if (*p == '\\') p++;
	}

	/* Finished.  Return our argument */
	return(str);
}

/*
 * struct sevstr *getauxsevs(ptr)
 *
 *	Parses a string that is in the format of the severity definitions.
 *	Returns a pointer to a (malloc'd) structure that contains the
 *	definition, or (struct sevstr *) NULL if none was parsed.
 *
 * Arguments:
 *	ptr	char *
 *		References the string from which data is to be extracted.
 *		If (char *) NULL, continue where we left off.  Otherwise,
 *		start with the string referenced by ptr.
 *
 * Returns: struct sevstr *
 *	A pointer to a malloc'd structure containing the severity definition
 *	parsed from string, or (struct sevstr *) NULL if none.
 *
 * Notes:
 *    - This function is destructive to the string referenced by its argument.
 */

/* Static data */
static	char	       *leftoff = (char *) NULL;

static	struct sevstr *
getauxsevs(ptr)
	char   *ptr;
{

	/* Automatic data */
	char	       *current;	/* Ptr to current sev def'n */
	char	       *tokend;		/* Ptr to end of current sev def'n */
	char	       *kywd;		/* Ptr to extracted kywd */
	char	       *valstr;		/* Ptr to extracted sev value */
	char	       *prstr;		/* Ptr to extracted print str */
	char	       *p;		/* Temp pointer */
	int		val;		/* Converted severity value */
	int		done;		/* Flag, sev def'n found and ok? */
	struct sevstr  *rtnval;		/* Value to return */


	/* Start anew or start where we left off? */
	current = (ptr == (char *) NULL) ? leftoff : ptr;


	/* If nothing to parse, return (char *) NULL */
	if (current == (char *) NULL) {
	    return ((struct sevstr *) NULL);
	}


	/*
	 * Look through the string "current" for a token of the form
	 * <kywd>,<sev>,<printstring> delimited by ':' or '\0'
	 */

	/* Loop initializations */
	done = FALSE;
	rtnval = (struct sevstr *) NULL;
	while (!done) {

	    /* Eat leading junk */
	    while (*(tokend = exttok(current, ":,")) == ':') {
		current = tokend + 1;
	    }

	    /* If we've found a <kywd>,... */
	    if (*tokend == ',') {
		kywd = current;
		*tokend = '\0';

		/* Look for <kywd>,<sev>,... */
		current = tokend + 1;
		if (*(tokend = exttok(current, ":,")) == ',') {
		    valstr = current;
		    *tokend = '\0';

		    current = tokend+1;
		    prstr = current;

		    /* Make sure <sev> > 4 */
		    val = (int) strtol(noesc(valstr), &p, 0);
		    if ((val > 4) && (p == tokend)) {

			/*
			 * Found <kywd>,<sev>,<printstring>.
			 * remember where we left off
			 */

		        if (*(tokend = exttok(current, ":")) == ':') {
			    *tokend = '\0';
			    leftoff = tokend + 1;
			} else leftoff = (char *) NULL;

			/* Alloc structure to contain severity definition */
			rtnval = (struct sevstr *) malloc(sizeof(struct sevstr));
			if (rtnval != (struct sevstr *) NULL) {

			    /* Fill in structure */
			    rtnval->sevkywd = noesc(kywd);
			    rtnval->sevvalue = val;
			    rtnval->sevprstr = noesc(prstr);
			    rtnval->sevnext = (struct sevstr *) NULL;
			}

			done = TRUE;

		    } else {

			/* Invalid severity value, eat thru end of token */
			current = tokend;
			if (*(tokend = exttok(prstr, ":")) == ':')
			    current++;
		    }

		} else {

		    /* Invalid severity definition, eat thru end of token */
		    current = tokend;
		    if (*tokend == ':')
			current++;
		}

	    } else {

		/* End of string found */
		done = TRUE;
		leftoff = (char *) NULL;
	    }

	} /* while (!done) */

	/* Finished */
	return(rtnval);
}

/*
 * void msgverbset()
 *
 *	Parces the argument of the MSGVERB environment variable and places
 *	a representation of the value of that value in "msgverb"
 *
 * Arguments:
 *	None:
 *
 * Returns: void
 *
 * Notes:
 */

static void
msgverbset()
{
	/*
	 * Automatic data
	 */

	char   *opts;			/* Pointer to MSGVERB's value */
	char   *alloced;		/* Pointer to MSGVERB's value */
	char   *tok;			/* Pointer to current token */
	char   *tokend;			/* Pointer to end of current token */
	char   *nexttok;		/* Pointer to next token */


	/* Rid ourselves of junk in "msgverb" */
	msgverb = 0;

	/* Get the value of MSGVERB.  If none, use default value */
	if ((opts = getenv(MSGVERB)) == (char *) NULL)
	    msgverb = MV_DFLT;

	/* MSGVERB has a value.  Interpret it */
	else {

	    if (!(alloced = (char *) malloc((unsigned int) strlen(opts)+1))) msgverb = MV_DFLT;
	    else {

		/* Make a copy of the value of MSGVERB */
		nexttok = strcpy(alloced, opts);

		/* Parse the options given by the user */
		while ((tok = nexttok) != (char *) NULL) {

		    /* Find end of the next token and squeeze out escaped characters */
		    tokend = exttok(tok, ":");
		    tok = noesc(tok);

		    /* Delimit token and mark next, if any */
		    if (*tokend == ':') {
			nexttok = tokend+1;
			*tokend = '\0';
		    } else nexttok = (char *) NULL;

		    /* Check for "text" */
		    if (strcmp(tok, ST_TXT) == 0) msgverb |= MV_TXT;

		    /* Check for "label" */
		    else if (strcmp(tok, ST_LBL) == 0) msgverb |= MV_LBL;

		    /* Check for "action */
		    else if (strcmp(tok, ST_ACT) == 0) msgverb |= MV_ACT;

		    /* Check for "severity" */
		    else if (strcmp(tok, ST_SEV) == 0) msgverb |= MV_SEV;

		    /* Check for "tag" */
		    else if (strcmp(tok, ST_TAG) == 0) msgverb |= MV_TAG;

		    /* Unknown, ignore the whole MSGVERB value */
		    else {
			msgverb = MV_DFLT;
			nexttok = (char *) NULL;
		    }

		} /* do while */

		/* Use default if no keywords on MSGVERB environment variable */
		if (msgverb == 0) msgverb = MV_DFLT;

		/* Free allocated space */
		free(alloced);

	    } /* else */

	}

	/* Finished */
	return;
}

/*
 * void sevstrset()
 *
 *	This function builds a structure containing auxillary severity
 *	definitions.
 *
 *  Arguments:  None
 *
 *  Returns:  Void
 */

static char *sevspace = (char *) NULL;

static void
sevstrset()
{
	/* Automatic data */
	struct sevstr  *plast;
	struct sevstr  *psev;
	char	       *value;


	/* Look for SEV_LEVEL definition */
	if ((value = getenv(SEV_LEVEL)) != (char *) NULL) {

	    /* Allocate space and make a copy of the value of SEV_LEVEL */
	    if (sevspace = (char *) malloc((unsigned int) strlen(value)+1)) {
		(void) strcpy(sevspace, value);

		/* Continue for all severity descriptions */
		psev = getauxsevs(sevspace);
		plast = (struct sevstr *) NULL;
		if (psev != (struct sevstr *) NULL) {
		    penvsevs = psev;
		    plast = psev;
		    while (psev = getauxsevs((char *) NULL)) {
			plast->sevnext = psev;
			plast = psev;
		    }
		}

	    } /* if sevspace != (char *) NULL */

	} /* if value != (char *) NULL */

}

/*
 * int addseverity(value, string)
 *	int	value		Value of the severity
 *	const char   *string	Print-string for the severity
 *
 *  Arguments:
 *	value		int
 *			The integer value of the severity being added
 *	string		char *
 *			A pointer to the character-string to be printed
 *			whenever a severity of "value" is printed
 *
 *  Returns:  int
 *	Zero if successful, -1 if failed. The function can fail under
 *	the following circumstances:
 *	  - malloc() fails
 *	  - The "value" is one of the reserved values.
 *
 *	This function permits C applications to define severity-levels
 *	that augment the standard levels and those defined by the
 *	SEV_LEVEL environment variable.
 */

int
addseverity(value, string)
	int	value;
	const char   *string;
{
	/*
	 * Automatic data
	 */

	struct sevstr  *p;		/* Temp ptr to severity structs */
	struct sevstr  *q;		/* Temp ptr (follower) to severity structs */
	int		found;		/* FLAG, element found in the list */
	int		rtnval;		/* Value to return to the caller */

	/* Make sure we're not trying to redefine one of the reserved values */
	if (value <= 4) {
	   errno = EINVAL;
	   return(-1);
	}

	/* Make sure we've interpreted SEV_LEVEL */

	if (sevlook) {
	    sevstrset();
	    sevlook = FALSE;
	}


	/* Leaf through the list.  We may be redefining or removing a definition */
	q = (struct sevstr *) NULL;
	found = FALSE;
	for (p = paugsevs ; !found && (p != (struct sevstr *) NULL) ; p = p->sevnext) {
	    if (p->sevvalue == value) {

		/* We've a match.  Remove or modify the entry */
		if (string == (char *) NULL) {
		    if (q == (struct sevstr *) NULL) paugsevs = p->sevnext;
		    else q->sevnext = p->sevnext;
		    free(p);
		}
		else p->sevprstr = string;

		found = TRUE;
	    }
	    q = p;
	}

	/* Adding a definition */
	if (!found && (string != (char *) NULL)) {

	    /* Allocate space for the severity structure */
	    if ((p = (struct sevstr *) malloc(sizeof(struct sevstr))) == (struct sevstr *) NULL)
		return(-1);

	    /*
	     * Fill in the new structure with the data supplied and add to
	     * the head of the augmented severity list.
	     */

	    p->sevkywd = (char *) NULL;
	    p->sevprstr = string;
	    p->sevvalue = value;
	    p->sevnext = paugsevs;
	    paugsevs = p;

	    /* Successfully added a new severity */
	    rtnval = 0;
	}
	else if (string == (char *) NULL) {

	    /* Attempting to undefined a non-defined severity */
	    rtnval = -1;
	    errno = EINVAL;
	}
	else
	    /* Successfully redefined a severity */
	    rtnval = 0;

	/* Finished, successful */
	return(rtnval);
}

/*
 * int writemsg(stream, verbosity, label, severity, text, action, tag)
 *
 * Arguments:
 * 	FILE   *stream		The standard I/O stream to which to write
 *				the message to
 * 	int	verbosity	A bit-string that indicates which components
 *				are to be written
 * 	const char   *label	The address of the label-component
 * 	int	severity	The severity value of the message
 * 	const char   *text	The address of the text-component
 * 	const char   *action	The address of the action-component
 * 	const char   *tag	The address of the tag-component
 *
 *	This function writes the message consisting of the label-component,
 *	severity-component, text-component, action-component, and tag-
 *	component to the standard I/O stream "stream".  The "verbosity"
 *	argument tells which components can be selected.  Any or all of the
 *	components can be their null-values.
 *
 * Returns:  int
 *	0 if no error was encountered,
 *	!0 if an error was encountered
 *
 * Notes:
 */

static int
writemsg(stream, verbosity, label, severity, text, action, tag)
	FILE   *stream;		/* Stream to write the message to */
	int	verbosity;	/* Components to include in the message */
	const char   *label;	/* Label-component */
	int	severity;	/* Severity value */
	const char   *text;	/* Text-component */
	const char   *action;	/* Action-component */
	const char   *tag;	/* Tag-component */
{
	struct sevstr  *psev;		/* Ptr for severity str list */
	const char     *p;		/* General purpose pointer */
	const char     *sevpstr;	/* Pointer to severity string */
	int		l1indent;	/* # chars to indent line 1 */
	int		l2indent;	/* # chars to indent line 2 */
	int		textindent;	/* # spaces to indent text */
	int		actindent;	/* # spaces to indent action */
	int		i;		/* General purpose counter */
	int		dolabel;	/* TRUE if label to be written */
	int		dotext;		/* TRUE if text to be written */
	int		dosev;		/* TRUE if severity to be written */
	int		doaction;	/* TRUE if action to be written */
	int		dotag;		/* TRUE if tag to be written */
	char		c;		/* Temp, multiuse character */
	char		sevpstrbuf[15];	/* Space for SV=%d */

	char		lcllbl[MM_MXLABELLN+1];	/* Space for (possibly truncated) label */
	char		lcltag[MM_MXTAGLN+1];	/* Space for (possibly truncated) tag */


	/*
	 * Figure out what fields are to be written (all are optional)
	 */

	dolabel  = (verbosity & MV_LBL) && (label != MM_NULLLBL);
	dosev    = (verbosity & MV_SEV) && (severity != MM_NULLSEV);
	dotext   = (verbosity & MV_TXT) && (text != MM_NULLTXT);
	doaction = (verbosity & MV_ACT) && (action != MM_NULLACT);
	dotag    = (verbosity & MV_TAG) && (tag != MM_NULLTAG);

	/*
	 * Figure out how much we'll need to indent the text of the message
	 */

	/* Count the label of the message, if requested */
	textindent = 0;
	if (dolabel) {
	    (void) strncpy(lcllbl, label, MM_MXLABELLN);
	    lcllbl[MM_MXLABELLN] = '\0';
	    textindent = strlen(lcllbl) + SEPSTRLN;
	}

	/*
	 * If severity req'd, determine the severity string and factor
	 * into indent count.  Severity string generated by:
	 *	1.  Search the standard list of severities.
	 *	2.  Search the severities added by the application.
	 *	3.  Search the severities added by the environment.
	 *	4.  Use the default (SV=n where n is the value of the severity).
	 */

	if (dosev) {

	    /* Search the default severity definitions */
	    psev = pstdsevs;
	    while (psev != (struct sevstr *) NULL)
		if (psev->sevvalue == severity) break;
		else psev = psev->sevnext;

	    if (psev == (struct sevstr *) NULL) {

		/* Search the severity definitions added by the application */
		psev = paugsevs;
		while (psev != (struct sevstr *) NULL)
		    if (psev->sevvalue == severity) break;
		    else psev = psev->sevnext;

		if (psev == (struct sevstr *) NULL) {

		    /* Search the severity definitions added by the environment */
		    psev = penvsevs;
		    while (psev != (struct sevstr *) NULL)
			if (psev->sevvalue == severity) break;
			else psev = psev->sevnext;

		    if (psev == (struct sevstr *) NULL) {

			/* Use default string, SV=severity */
			(void) sprintf(sevpstrbuf, "SV=%d", severity);
			sevpstr = sevpstrbuf;

		    }
		    else sevpstr = psev->sevprstr;

		}
		else sevpstr = psev->sevprstr;

	    }
	    else sevpstr = psev->sevprstr;

	    /* Factor into indent counts */
	    textindent += strlen(sevpstr) + SEPSTRLN;
	}


	/*
	 * Figure out the indents.
	 */

	if (doaction && dotext) {
	    if (textindent > ACTINTROLN) {
		l1indent = 0;
		l2indent = textindent - ACTINTROLN;
		actindent = textindent;
	    }
	    else {
		l2indent = 0;
		actindent = ACTINTROLN;
		if (dosev || dolabel) {
		    l1indent = ACTINTROLN - textindent;
		    textindent = ACTINTROLN;
		}
		else {
		    textindent = 0;
		    l1indent = 0;
		}
	    }
	}
	else {
	    l1indent = 0;
	    l2indent = 0;
	    if (doaction) {
		actindent = textindent + ACTINTROLN;
	    }
	    else if (dotext) {
		actindent = 0;
	    }
	}

	/* No characters written yet, no errors either */
	clearerr(stream);


	/*
	 * Write the message.
	 */


	/* Write the LABEL, if requested */
	if (dolabel) {

	    /* Write spaces to align on the ':' char, if needed */
	    while (--l1indent >= 0) (void) putc(' ', stream);

	    /* Write the label */
	    (void) fputs(lcllbl, stream);

	    /* Write the separator string (if another component is to follow) */
	    if (dosev || dotext || doaction || dotag)
		(void) fputs(SEPSTR, stream);
	}

	/* Write the SEVERITY, if requested */
	if (dosev) {

	    /* Write spaces to align on the ':' char, if needed */
	    while (--l1indent >= 0) (void) putc(' ', stream);

	    /* Write the severity print-string */
	    (void) fputs(sevpstr, stream);

	    /* Write the separator string (if another component is to follow) */
	    if (dotext || doaction || dotag)
		(void) fputs(SEPSTR, stream);
	}

	/* Write the TEXT, if requested */
	if (dotext) {
	    p = text;
	    while (c = *p++) {
		(void) putc(c, stream);
		if (c == '\n') {
		    for (i = 0 ; i < textindent ; i++)
			(void) putc(' ', stream);
		}
	    }
	}


	/*
	 * Write ACTION if requested.
	 */

	if (doaction) {
	    if (dotext) {
		(void) putc('\n', stream);
		while (--l2indent >= 0) {
		    (void) putc(' ', stream);
		}
	    }

	    /* Write the action-string's introduction */
	    (void) fputs(ACTINTRO, stream);

	    /* Write the "action" string */
	    p = action;
	    while (c = *p++) {
		(void) putc(c, stream);
		if (c == '\n') {
		    for (i = 0 ; i < actindent ; i++)
			(void) putc(' ', stream);
		}
	    }
	}


	/*
	 * Write the TAG if requested
	 */

	if (dotag) {

	    if (doaction) (void) fputs("  ", stream);
	    else if (dotext) (void) putc('\n', stream);
	    (void) strncpy(lcltag, tag, MM_MXTAGLN);
	    lcltag[MM_MXTAGLN] = '\0';
	    (void) fputs(lcltag, stream);
	}

	/* Write terminating carriage control */
	(void) putc('\n', stream);

	return(ferror(stream));
}

/*
 * int	fmtmsg(class, label, severity, text, action, tag)
 *	long	class
 *	const char   *label
 *	int	severity
 *	const char   *text
 *	const char   *action
 *	const char   *tag
 *
 *	If requested, the fmtmsg() function writes a message to the standard error stream in the
 *	standard message format.  Also if requested, it will write a message to the system
 *	console.
 *
 *	Arguments:
 *	    class	Fields which classify the message for the system
 *			logging facility
 *	    label	A character-string that is printed as the "label"
 *			of the message.  Typically identifies the source
 *			of the message
 *	    severity	Identifies the severity of the message.  Either one
 *			of the standard severities, or possibly one of the
 *			augmented severities
 *	    text	Pointer to the text of the message
 *	    action	Pointer to a char string that describes some type
 *			of corrective action.
 *	    tag		A character-string that is printed as the "tag" or
 *			the message.  Typically a pointer to documentation
 *
 *	Returns:
 *	    -1 if nothing was generated, 0 if everything requested was
 *	    generated, or flags if partially generated.
 *
 *	Needs:
 *	  - Nothing special for 4.0.
 */

int
fmtmsg(class, label, severity, text, action, tag)
	long		class;		/* Message "classification" */
	const char      *label;		/* Ptr to "label" */
	int		severity;	/* Message "severity" */
	const char     *text;		/* Ptr to the "text" */
	const char     *action;		/* Ptr to the text of the "action" */
	const char     *tag;		/* Ptr to the "tag" */
{

	/* Local automatic data */
	int		rtnval;		/* Value to return */
	FILE	       *console;	/* Ptr to "console" stream */


	/*
	 * Determine the "verbosity" of the message.  If "msgverb" is
	 * already set, don't interrogate the "MSGVERB" environment vbl.
	 * If so, interrogate "MSGVERB" and do initialization stuff also.
	 */

	if (!(msgverb & MV_SET)) {

	    msgverbset();
	    msgverb |= MV_SET;

	}


	/*
	 * Extract the severity definitions from the SEV_LEVEL
	 * environment variable and save away for later.
	 */

	if (sevlook) {
	    sevstrset();
	    sevlook = FALSE;
	}


	/* Set up the default text component [if text==(char *) NULL] */
	if (text == (char *) NULL) text = DEFLT_TEXT;

	rtnval = MM_OK;


	/* Write the message to stderr if requested */
	if (class & MM_PRINT) {
	    if (writemsg(stderr, msgverb, label, severity, text, action, tag)) rtnval |= MM_NOMSG;
	}

	/* Write the message to the console if requested */
	if (class & MM_CONSOLE) {
	    if (console = fopen(CONNAME, "w")) {
		if (writemsg(console, MV_ALL, label, severity, text, action, tag)) rtnval |= MM_NOCON;
		(void) fclose(console);
	    }
	    else rtnval |= MM_NOCON;
	}

	if ((rtnval & (MM_NOCON | MM_NOMSG)) == (MM_NOCON | MM_NOMSG)) rtnval = MM_NOTOK;
	return (rtnval);
}
