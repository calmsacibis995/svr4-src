/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/common.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)common.c	3.10	LCC);	/* Modified: 16:23:10 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#ifdef	SYS5
#include	<sys/utsname.h>
#endif	/* SYS5 */

char *myhostname()
{
	char *p, *dp;
#ifdef SYS5
	static struct utsname uName;
#else	/* !SYS5 */
#ifdef UDP42
	static	char hostName[MAX_NAME];
#endif	/* UDP42 */
#endif	/* SYS5 */

#ifdef SYS5
	if (uname(&uName) >= 0)
		p = uName.nodename;
	else
#else	/* !SYS5 */
#ifdef UDP42
	     if (gethostname(hostName, MAX_NAME) >= 0) 
		p = hostName;
	else
#endif	/* UDP42 */
#endif	/* SYS5 */

		p = "unix";

	/*
	 *  Truncate name so that it will fit into a map entry.
	 *  Also, truncate it to contain only the first domain component.
	 */
	for (dp = p; *dp && *dp != '.' && dp < &p[SZHNAME-1]; dp++)
		;
	*dp = '\0';

	return(p);
}



static char
	qName[MAX_PATH];		/* Fully qualified name (result) */

/*
   fnQualify: Fully qualify and canonicalize a path name
	   w.r.t a current directory.
*/

char *
fnQualify(fName, dName)
register char
	*fName;				/* File name to qualify */
char
	*dName;				/* Current working directory */
{
register char
	*qApp;				/* Appends fName to qName */

	/* If file name is absolute, initialize qName to /, otherwise dName */
	if (*fName != '/') {
		(void) strcpy(qName, dName);
		qApp = &qName[strlen(qName) - 1];
	} else {
		qApp = qName;
		*qApp = '/';
		fName++;
	}

	for (;;) {
		/* Insert path separator */
		if (*qApp != '/')
			*++qApp = '/';

		/*
		   If leading component of fName is .. or ../,
		   remove trailing component of qName
		*/
		if ((*fName == '.') && (
		    (fName[1] == '.'  && (fName[2] == '/' || fName[2] == '\0'))
		    || (fName[1] == '/'))) {
		        if (fName[1] == '.') {
			/* Remove trailing component of qName, if any */
				if (qApp > qName)
					do {
						*qApp-- = '\0';
					} while (*qApp != '/');
			}
			/* Skip over .. or ./ in fName */
			fName += 2;
		} else
			/* ..else append leading component of fName to qName */
			do {
				*++qApp = *fName++;
			} while (*fName != '\0' && *fName != '/');

		/* Skip leading /'s of next fName component */
		while (*fName == '/')
			fName++;

		/* If at end of fName, quit */
		if (*fName == '\0')
			break;
	}

	/* remove trailing slash, but not if full name is only '/' */
	if (*qApp == '/' && qApp != qName)
		*qApp = '\0';
	else
		qApp[1] = '\0';

	return qName;
}

