/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnet:netselect/netselect.c	1.9.3.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netconfig.h>
#include "netcspace.h"

#define FAILURE  (unsigned)(-1L)

/*
 *	Local routines used by the library procedures
 */

static int blank();
static int comment();
static struct netconfig *fgetnetconfig();
static void free_netcf();
static unsigned long getflag();
static char **getlookups();
static struct netconfig **getnetlist();
static unsigned long getnlookups();
static char *gettoken();
static unsigned long getvalue();
static void shift1left();

/*
 *	System V routines used by the library procedures.
 */

extern FILE *fopen();
extern void  rewind(), free();
extern int   fclose(), strcmp();
extern char *fgets(), *strcat(), *getenv(), *strcpy(), *malloc();

/*
 *	Static global variables used by the library procedures:
 *	
 *	netpp - points to the beginning of the list of netconfig
 *		entries used by setnetconfig() and setnetpath().
 *
 *	num_calls - number of times setnetpath() and setnetconfig()
 *		    are called
 *
 *	linenum - the current line number of the /etc/netconfig
 *		  file (used for debugging and for nc_perror()).
 *
 *	fieldnum - the current field number of the current line
 *	 	   of /etc/netconfig (used for debugging and for
 *		   nc_perror()).
 *
 *	nc_error - the error condition encountered.
 */

static struct netconfig **netpp = NULL;
static int num_calls = 0;
static int linenum = 0;
static int fieldnum = 0;
static int nc_error = NC_NOERROR;

/*
 *	setnetconfig() has the effect of "initializing" the
 *	network configuration database.   It reads in the
 *	netcf entries (if not already read in) and returns
 *	a handle to the first entry (to be used in subsequent
 *	calls to getnetconfig()).
 */

void *
setnetconfig()
{
	NCONF_HANDLE *retp;

	if((netpp == NULL) && ((netpp = getnetlist()) == NULL)) {
		return(NULL);
	}
	if ((retp = (NCONF_HANDLE *)malloc(sizeof(NCONF_HANDLE))) == NULL) {
		nc_error = NC_NOMEM;
		return(NULL);
	}
	retp->nc_head = retp->nc_curr = netpp;
	num_calls ++;
	return((void *)retp);
}

/*
 *	endnetconfig() frees up all data allocated by setnetconfig()
 *	but only if this is the last call in a potentially nested
 *	sequence of "setnetconfig() - endnetconfig()" calls.
 */

int
endnetconfig(vdata)
void  *vdata;
{
	/*
	 *	The argument is really a NCONF_HANDLE;  cast it here
	 */
	NCONF_HANDLE *nconf_handlep = (NCONF_HANDLE *)vdata;
	struct netconfig **tpp; /* traverses the array of netcf structures */

	if (netpp == NULL || nconf_handlep == NULL) {
		nc_error = NC_NOSET;
		return(-1);
	}

	free(nconf_handlep);

	if (--num_calls == 0) {
		for (tpp = netpp; *tpp; tpp++) {
			free_netcf(*tpp);
		}
		free(netpp);
		netpp = NULL;
	}
	return(0);
}

/*
 *	getnetconfig() returns the current entry in the list
 *	of netconfig structures.  It uses the nconf_handlep argument
 *	to determine the current entry. If setnetconfig() was not
 *	called previously to set up the list, return failure.
 */

struct netconfig *
getnetconfig(vdata)
void *vdata;
{
	/*
	 *	The argument is really a NCONF_HANDLE;  cast it here
	 */
	NCONF_HANDLE *nconf_handlep = (NCONF_HANDLE *)vdata;
	struct netconfig *retp;  /* holds the return value */

	if (netpp == NULL) {
		nc_error = NC_NOSET;
		return(NULL);
	}
	retp = *(nconf_handlep->nc_curr);
	if (retp != NULL)
		++(nconf_handlep->nc_curr);
	return(retp);
}

/*
 *	getnetconfig() searches the netconfig database for a
 *	given network id.  Returns a pointer to the netconfig
 *	structure or a NULL if not found.
 */

struct netconfig *
getnetconfigent(netid)
char *netid;
{
	struct netconfig *cfp; /* holds each entry in NETCONFIG */
	FILE *fp;	       /* file stream for NETCONFIG     */

	if ((fp = fopen(NETCONFIG, "r")) == NULL) {
		nc_error = NC_OPENFAIL;
		return(NULL);
	}

	while ((cfp = fgetnetconfig(fp)) && strcmp(netid, cfp->nc_netid)) {
		free_netcf(cfp);
	}

	(void) fclose(fp);
	return(cfp);
}

/*
 *	freenetconfigent frees the data allocated by getnetconfigent()
 */

void
freenetconfigent(netp)
struct netconfig *netp;
{
	free_netcf(netp);
}

/*
 *	getnetlist() reads the netconfig file and creates a
 *	NULL-terminated list of entries.
 *	Returns the pointer to the head of the list or a NULL
 *	on failure.
 */

static struct netconfig **
getnetlist()
{
	char  line[BUFSIZ];         /* holds each line of NETCONFIG        */
	FILE *fp;	            /* file stream for NETCONFIG           */
	struct netconfig **listpp;  /* the beginning of the netconfig list */
	struct netconfig **tpp;     /* used to traverse the netconfig list */
	int num_validentries;	    /* the number of valid entries in file */
	int count;		    /* the number of entries in file       */

	if ((fp = fopen(NETCONFIG, "r")) == NULL) {
		nc_error = NC_OPENFAIL;
		return(NULL);
	}

	/*
	 *	Set count to the number of non-blank and comment lines
	 * 	in the NETCONFIG file plus 1 (since a NULL entry will
	 *	terminate the list).
	 */

	count = 1;
	while (fgets(line, BUFSIZ, fp)) {
		if (!(blank(line) || comment(line))) {
			++count;
		}
	}
	rewind(fp);

	if ((listpp = (struct netconfig **)malloc(count * sizeof(struct netconfig *))) == NULL) {
		nc_error = NC_NOMEM;
		return(NULL);
	}

	/*
	 *	The following loop fills in the list (loops until
	 *	fgetnetconfig() returns a NULL) and counts the
	 *	number of entries placed in the list.  Note that
	 *	when the loop is completed, the last entry in the
	 *	list will contain a NULL (signifying the end of
	 *	the list).
	 */

	num_validentries = 0;
	linenum = 0;
	for (tpp = listpp; *tpp = fgetnetconfig(fp); tpp++) {
		num_validentries ++;
	}
	(void) fclose(fp);

	/*
	 *	If the number of valid entries is not the same
	 *	as the number of lines in the file, then some of the
	 *	lines did not parse correctly (so free up the
	 *	space and return NULL).  Note that count must be
	 *	decremented since it is the number of entries + 1.
	 */

	if (num_validentries != --count) {
		for (tpp = listpp; *tpp; tpp++) {
			free_netcf(*tpp);
		}
		free(listpp);
		listpp = NULL;
	}
	return(listpp);
}

/*
 *	fgetnetconfig() parses a line of the netconfig file into
 *	a netconfig structure.  It returns a pointer to the
 *	structure of success and a NULL on failure or EOF.
 */

static struct netconfig *
fgetnetconfig(fp)
FILE *fp;
{
	register char *linep;	      /* pointer to a line in the file     */
	struct netconfig *netconfigp; /* holds the new netconfig structure */
	char  *tokenp;		      /* holds a token from the line       */
	char  *retvalp;		      /* the return value of fgets()       */

	if (((linep = malloc(BUFSIZ)) == NULL)
	 || ((netconfigp = (struct netconfig *)malloc(sizeof(struct netconfig))) == NULL)) {
		nc_error = NC_NOMEM;
		return(NULL);
	}

	/*
	 *	skip past blank lines and comments.
	 */

	while (retvalp = fgets(linep, BUFSIZ, fp)) {
		linenum ++;
		if (!(blank(linep) || comment(linep))) {
			break;
		}
	}
	if (retvalp == NULL) {
		free(linep);
		free(netconfigp);
		return(NULL);
	}

	fieldnum = 0;
	if (((netconfigp->nc_netid = gettoken(linep)) == NULL)
	 || ((tokenp = gettoken(NULL)) == NULL)
	 || ((netconfigp->nc_semantics = getvalue(tokenp, nc_semantics)) == FAILURE)
	 || ((tokenp = gettoken(NULL)) == NULL)
	 || ((netconfigp->nc_flag = getflag(tokenp)) == FAILURE)
	 || ((netconfigp->nc_protofmly = gettoken(NULL)) == NULL)
	 || ((netconfigp->nc_proto = gettoken(NULL)) == NULL)
	 || ((netconfigp->nc_device = gettoken(NULL)) == NULL)
	 || ((tokenp = gettoken(NULL)) == NULL)
	 || ((netconfigp->nc_nlookups = getnlookups(tokenp)) == FAILURE)
	 || ((netconfigp->nc_lookups = getlookups(tokenp)) == NULL)) {
		free(linep);
		free(netconfigp);
		nc_error = NC_BADLINE;
		return(NULL);
	}
	return(netconfigp);
}

/*
 *	setnetpath() has the effect of "initializing" the
 *	NETPATH variable.  It reads in the netcf entries (if not
 *	already read in), creates a list corresponding to the entries
 *	in the NETPATH variable (or the "visible" entries og netconfig
 *	if NETPATH is not set), and returns a pointer to the
 *	first value in the list.
 */

void *
setnetpath()
{
	int count;	            /* the number of entries in NETPATH     */
	char valid_netpath[BUFSIZ]; /* holds the valid entries if NETPATH   */
	char templine[BUFSIZ];	    /* has value of NETPATH when scanning   */
	struct netconfig **curr_pp; /* scans the list from NETPATH          */
	struct netconfig **tpp;     /* scans the list from netconfig file   */
	struct netconfig **rnetpp;  /* the list of entries from NETPATH     */
	char *netpath;		    /* value of NETPATH from environment    */
	char *netid;		    /* holds a component of NETPATH         */
	register char *tp;	    /* used to scan NETPATH string          */
	NCONF_HANDLE *retp;	    /* the return value                     */

	/*
	 *	Allocate space to hold the return value
	 */

	if ((retp = (NCONF_HANDLE *)malloc(sizeof(NCONF_HANDLE))) == NULL) {
		nc_error = NC_NOMEM;
		return(NULL);
	}

	/*
	 *	Read in the netconfig database if not already read in
	 */

	if ((netpp == NULL) && ((netpp = getnetlist()) == NULL)) {
		return(NULL);
	}

	/*
	 *	Get the valid entries of the NETPATH variable (and
	 *	count the number of entries while doing it).
	 *
	 *	This is done every time the procedure is called just
	 *	in case NETPATH has changed from call to call.
	 */

	count = 0;
	valid_netpath[0] = '\0';
	if ((netpath = getenv(NETPATH)) == NULL) {

		/*
		 *	Since no NETPATH variable,
		 *	the valid NETPATH consist of all "visible"
		 *	netids from the netconfig database.
		 */

		for (tpp = netpp; *tpp; tpp++) {
			if ((*tpp)->nc_flag & NC_VISIBLE) {
				(void)strcat(valid_netpath, (*tpp)->nc_netid);
				(void)strcat(valid_netpath, ":");
				count ++;
			}
		}
	} else {

		/*
		 *	Copy the value of NETPATH (since '\0's will be
		 *	put into the string) and create the valid NETPATH
		 *	(by throwing away all netids not in the database).
		 *	If an entry appears more than one, it *will* be
		 *	listed twice in the list of valid netpath entries.
		 */

		(void)strcpy(templine, netpath);

		/*
		 *	Skip all leading ':'s
		 */

		tp = templine;
		while (*tp && *tp == ':')
			tp++;

		/*
		 *	Set the first token and scan to the next.
		 */

		netid = tp;
		while (*tp && *tp != ':')
			tp++;
		if (*tp)
			*tp++ = '\0';
		while (*tp == ':')
			tp++;

		while (*netid) {
			for (tpp = netpp; *tpp; tpp++) {
				if (!strcmp(netid, (*tpp)->nc_netid)) {
					(void)strcat(valid_netpath, (*tpp)->nc_netid);
					(void)strcat(valid_netpath, ":");
					count ++;
					break;
				}
			}
			/*
			 *	Set netid and scan to the next token
			 */

			netid = tp;
			while (*tp && *tp != ':')
				tp++;
			if (*tp)
				*tp++ = '\0';
			while (*tp == ':')
				tp++;
		}
	}

	/*
	 *	Get space to hold the valid list (+1 for the NULL)
	 */

	if ((rnetpp = (struct netconfig **)malloc(++count * sizeof(struct netconfig *))) == NULL) {
		nc_error = NC_NOMEM;
		return(NULL);
	}

	/*
	 *	Populate the NETPATH list, ending it with a NULL.
	 *	Each entry in the list points to the structure in the
	 *	"netpp" list (the entry must exist in the list, otherwise
	 *	it wouldn't appear in valid_netpath[]).
	 */

	curr_pp = rnetpp;
	netid = tp = valid_netpath;
	while (*tp && *tp != ':')
		tp++;
	if (*tp)
		*tp++ = '\0';
	while (*netid) {
		for (tpp = netpp; *tpp; tpp++) {
			if (!strcmp(netid, (*tpp)->nc_netid)) {
				*curr_pp++ = *tpp;
				break;
			}
		}
		netid = tp;
		while (*tp && *tp != ':')
			tp++;
		if (*tp)
			*tp++ = '\0';
	}
	*curr_pp = NULL;

	/*
	 *	Return the pointer to the first entry in the list
	 */

	retp->nc_curr = retp->nc_head = rnetpp;
	num_calls ++;
	return((void *)retp);
}

/*
 *	endnetpath() frees up all of the memory allocated by setnetpath().
 *	It returns -1 (error) if setnetpath was never called.
 */

int
endnetpath(vdata)
void *vdata;
{
	/*
	 *	The argument is really a NCONF_HANDLE;  cast it here
	 */
	NCONF_HANDLE *nconf_handlep = (NCONF_HANDLE *)vdata;
	struct netconfig **tpp; /* traverses the array of netcf structures */

	if (netpp == NULL || nconf_handlep == NULL) {
		nc_error = NC_NOSET;
		return(-1);
	}

	free(nconf_handlep->nc_head);
	free(nconf_handlep);

	if (--num_calls == 0) {
		for (tpp = netpp; *tpp; tpp++) {
			free_netcf(*tpp);
		}
		free(netpp);
		netpp = NULL;
	}
	return(0);
}

/*
 *	getnetpath() returns the current entry in the list
 *	from the NETPATH variable.  If setnetpath() was not called
 *	previously to set up the list, return NULL.
 */

struct netconfig *
getnetpath(vdata)
void *vdata;
{
	/*
	 *	The argument is really a NCONF_HANDLE;  cast it here
	 */
	NCONF_HANDLE *nconf_handlep = (NCONF_HANDLE *)vdata;
	struct netconfig *retp;  /* holds the return value */

	if (netpp == NULL) {
		nc_error = NC_NOSET;
		return(NULL);
	}
	retp = *(nconf_handlep->nc_curr) == NULL? NULL: *(nconf_handlep->nc_curr);
	if (retp != NULL)
		++(nconf_handlep->nc_curr);
	return(retp);
}

/*
 *	free_netcf() simply frees up the memeory allocated for the
 *	given netcf structure.
 */

static void
free_netcf(netcfp)
struct netconfig *netcfp;
{
	free(netcfp->nc_lookups);
	free(netcfp->nc_netid);
	free(netcfp);
}

/*
 *	blank() returns true if the line is a blank line, 0 otherwise
 */

static int
blank(cp)
char *cp;
{
	while (*cp && isspace(*cp)) {
		cp++;
	}
	return(*cp == '\0');
}

/*
 *	comment() returns true if the line is a comment, 0 otherwise.
 */

static int
comment(cp)
char *cp;
{
	while (*cp && isspace(*cp)) {
		cp ++;
	}
	return(*cp == '#');
}

/*
 *	getvalue() searches for the given string in the given array,
 *	and return the integer value associated with the string.
 */

static unsigned long
getvalue(cp, nc_data)
char *cp;
struct nc_data nc_data[];
{
	int i;	/* used to index through the given struct nc_data array */

	for (i = 0; nc_data[i].string; i++) {
		if (!strcmp(nc_data[i].string, cp)) {
			break;
		}
	}
	return(nc_data[i].value);
}

/*
 *	getflag() creates a bitmap of the one-character flags in
 *	the given string.  It uses nc_flags array to get the values.
 */

static unsigned long
getflag(cp)
char *cp;
{
	int i;	                 /* indexs through the nc_flag array */
	unsigned long mask = 0;  /* holds bitmask of flags           */

	while (*cp) {
		for (i = 0; nc_flag[i].string; i++) {
			if (*nc_flag[i].string == *cp) {
				mask |= nc_flag[i].value;
				break;
			}
		}
		cp ++;
	}
	return(mask);
}

/*
 *	getlookups() creates and returns an array of string representing
 *	the directory lookup libraries, given as a comma-seperated list
 *	in the argument "cp".
 */

static char **
getlookups(cp)
char *cp;
{
	unsigned long num;     /* holds the number of entries in the list   */
	char **listpp;	       /* the beginning of the list of dir routines */
	register char **tpp;   /* traverses the list, populating it         */

	num = getnlookups(cp);
	if ((listpp = (char **)malloc((num + 1) * sizeof(char *))) == NULL) {
		return(NULL);
	}

	tpp = listpp;
	while (num--) {
		*tpp  = cp;

		/*
		 *	Travserse the string looking for the next entry
		 *	of the list (i.e, where the ',' or end of the
	 	 *	string appears).  If a "\" is found, shift the
		 *	token over 1 to the left (taking the next char
		 *	literally).
		 */

		while (*cp && *cp != ',') {
			if (*cp == '\\' && *(cp + 1)) {
				shift1left(cp);
			}
			cp ++;
		}
		if (*cp)
			*cp++ ='\0';
		tpp ++;
	}
	*tpp = NULL;
	return(listpp);
}

/*
 *	getnlookups() returns the number of entries in a comma-separated
 *	string of tokens.  A "-" means no strings are present.
 */

static unsigned long
getnlookups(cp)
char *cp;
{
	unsigned long count;	/* the number of tokens in the string */

	if (!strcmp(cp, "-")) {
		return(0);
	}

	count = 1;
	while (*cp) {
		if (*cp == ',') {
			count++;
		}

		/*
		 *	If a "\" is in the string, take the next character
		 *	literally.  Onlly skip the character if "\" is
		 *	not the last character of the token.
		 */

		if (*cp == '\\' && *(cp + 1)) {
			cp ++;
		}
		cp ++;
	}
	return(count);
}

/*
 *	gettoken() behaves much like strtok(), except that
 *	it knows about escaped space characters (i.e., space characters
 *	preceeded by a '\' are taken literally).
 */

static char *
gettoken(cp)
char	*cp;
{
	static char	*savep;	   /* the place where we left off    */
	register char	*p;	   /* the beginning of the new token */
	register char	*retp;	   /* the token to be returned       */

	fieldnum ++;

	/*
	 *	Determine if first or subsequent call
	 */

	p = (cp == NULL)? savep: cp;

	/*
	 *	Return if no tokens remain.
	 */

	if (p == 0) {
		return(NULL);
	}

	while (isspace(*p))
		p++;

	if (*p == '\0') {
		return(NULL);
	}

	/*
	 *	Save the location of the token and then skip past it
	 */

	retp = p;
	while (*p) {
		if (isspace(*p)) {
			break;
		}
		/*
		 *	Only process the escape of the space seperaror;
		 *	since the token may contain other separators,
		 *	let the other routines handle the escape of
		 *	specific characters in the token.
		 */

		if (*p == '\\' && *(p + 1) != '\n' && isspace(*(p + 1))) {
			shift1left(p);
		}
		p ++;
	}
	if(*p == '\0') {
		savep = 0;	/* indicate this is last token */
	} else {
		*p = '\0';
		savep = ++p;
	}
	return(retp);
}

/*
 *	shift1left() moves all characters in the string over 1 to
 *	the left.
 */

static void
shift1left(p)
char *p;
{
	for (; *p; p++)
		*p = *(p + 1);
}

char *
nc_sperror()
{
	static char retstr[BUFSIZ];  /* the return string */

	switch (nc_error) {
	   case NC_NOERROR:
		(void)strcpy(retstr, "no error");
		break;
	   case NC_NOMEM:
		(void)strcpy(retstr, "out of memory");
		break;
	   case NC_NOSET:
		(void)strcpy(retstr, "routine called before calling setnetpath() or setnetconfig()");
		break;
	   case NC_OPENFAIL:
		(void)strcpy(retstr, "cannot open /etc/netconfig");
		break;
	   case NC_BADLINE:
		(void)sprintf(retstr, "error in /etc/netconfig: field %d of line %d\n", fieldnum, linenum);
		break;
	}
	return(retstr);
}

void
nc_perror(string)
char *string;
{
	fprintf(stderr, "%s: %s\n", string, nc_sperror());
}
