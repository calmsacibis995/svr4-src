/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:listopts.c	1.2.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/param.h>

#include <signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "systable.h"
#include "proto.h"

/*
 * option procesing ---
 * Routines for scanning syscall, signal, fault
 * and file descriptor lists.
 */

/* This module is carefully coded to contain only read-only data */
/* All read/write data is defined in ramdata.c (see also ramdata.h) */

extern long strtol();

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	void	upcase( char * );

#else	/* defined(__STDC__) */

static	void	upcase();

#endif	/* defined(__STDC__) */

static CONST char sepr[] = " ,\t\n";	/* list separator characters */

int			/* scan list of syscall names */
syslist(str, setp, fp)	/* return 0 on success, != 0 on any failure */
	char *str;			/* string of syscall names */
	register sysset_t *setp;	/* syscall set */
	int *fp;			/* first-time flag */
{
	register char * name;
	register int exclude = FALSE;
	int rc = 0;

	name = strtok(str, sepr);

	if (name != NULL && *name == '!') {	/* exclude from set */
		exclude = TRUE;
		if (*++name == '\0')
			name = strtok(NULL, sepr);
	}
	else if (!*fp) {	/* first time, clear the set */
		premptyset(setp);
		*fp = TRUE;
	}

	for ( ; name; name = strtok(NULL, sepr)) {
		register int sys;
		char * next;

		if (*name == '!') {	/* exclude remainder from set */
			exclude = TRUE;
			while (*++name == '!')
				;
			if (*name == '\0')
				continue;
		}

		sys = strtol(name, &next, 0);
		if (sys < 0 || sys > PRMAXSYS || *next != '\0')
			sys = 0;
		if (sys == 0) {
			register CONST struct systable *stp = systable;
			for ( ; sys == 0 && stp->nargs >= 0; stp++)
				if (stp->name && strcmp(stp->name, name) == 0)
					sys = stp-systable;
		}
		if (sys == 0) {
			register CONST struct sysalias *sap = sysalias;
			for ( ; sys == 0 && sap->name; sap++)
				if (strcmp(sap->name, name) == 0)
					sys = sap->number;
		}
		if (sys > 0 && sys <= PRMAXSYS) {
			switch (sys) {
			case SYS_exec:	/* set both if either */
			case SYS_execve:
				if (exclude) {
					prdelset(setp, SYS_exec);
				} else {
					praddset(setp, SYS_exec);
				}
				sys = SYS_execve;
				/* fall through */
			default:
				if (exclude) {
					prdelset(setp, sys);
				} else {
					praddset(setp, sys);
				}
				break;
			}
		}
		else if (strcmp(name, "all") == 0
		      || strcmp(name, "ALL") == 0) {
			if (exclude) {
				premptyset(setp);
			} else {
				prfillset(setp);
			}
		}
		else {
			(void) fprintf(stderr,
				"%s: unrecognized syscall: %s\n",
				command, name);
			rc = -1;
		}
	}

	return rc;
}

int			/* list of signals to trace */
siglist(str, setp, fp)	/* return 0 on success, != 0 on any failure */
	char *str;			/* string of signal names */
	register sigset_t *setp;	/* signal set */
	int *fp;			/* first-time flag */
{
	register char * name;
	register int exclude = FALSE;
	int rc = 0;

	upcase(str);
	name = strtok(str, sepr);

	if (name != NULL && *name == '!') {	/* exclude from set */
		exclude = TRUE;
		if (*++name == '\0')
			name = strtok(NULL, sepr);
	}
	else if (!*fp) {	/* first time, clear the set */
		premptyset(setp);
		*fp = TRUE;
	}

	for ( ; name; name = strtok(NULL, sepr)) {
		register int sig;
		char * next;

		if (*name == '!') {	/* exclude remainder from set */
			exclude = TRUE;
			while (*++name == '!')
				;
			if (*name == '\0')
				continue;
		}

		sig = strtol(name, &next, 0);
		if (sig <= 0 || sig > PRMAXSIG || *next != '\0') {
			for (sig = 1; sig <= PRMAXSIG; sig++) {
				register CONST char * sname = rawsigname(sig);
				if (sname == NULL)
					continue;
				if (strcmp(sname, name) == 0
				 || strcmp(sname+3, name) == 0)
					break;
			}
			if (sig > PRMAXSIG)
				sig = 0;
		}
		if (sig > 0 && sig <= PRMAXSIG) {
			if (exclude) {
				prdelset(setp, sig);
			} else {
				praddset(setp, sig);
			}
		}
		else if (strcmp(name, "ALL") == 0) {
			if (exclude) {
				premptyset(setp);
			} else {
				prfillset(setp);
			}
		}
		else {
			(void) fprintf(stderr,
				"%s: unrecognized signal name/number: %s\n",
				command, name);
			rc = -1;
		}
	}

	return rc;
}

int			/* list of faults to trace */
fltlist(str, setp, fp)	/* return 0 on success, != 0 on any failure */
	char *str;			/* string of fault names */
	register fltset_t *setp;	/* fault set */
	int *fp;			/* first-time flag */
{
	register char * name;
	register int exclude = FALSE;
	int rc = 0;

	upcase(str);
	name = strtok(str, sepr);

	if (name != NULL && *name == '!') {	/* exclude from set */
		exclude = TRUE;
		if (*++name == '\0')
			name = strtok(NULL, sepr);
	}
	else if (!*fp) {	/* first time, clear the set */
		premptyset(setp);
		*fp = TRUE;
	}

	for ( ; name; name = strtok(NULL, sepr)) {
		register int flt;
		char * next;

		if (*name == '!') {	/* exclude remainder from set */
			exclude = TRUE;
			while (*++name == '!')
				;
			if (*name == '\0')
				continue;
		}

		flt = strtol(name, &next, 0);
		if (flt <= 0 || flt > PRMAXFAULT || *next != '\0') {
			for (flt = 1; flt <= PRMAXFAULT; flt++) {
				register CONST char * fname = rawfltname(flt);
				if (fname == NULL)
					continue;
				if (strcmp(fname, name) == 0
				 || strcmp(fname+3, name) == 0)
					break;
			}
			if (flt > PRMAXFAULT)
				flt = 0;
		}
		if (flt > 0 && flt <= PRMAXFAULT) {
			if (exclude) {
				prdelset(setp, flt);
			} else {
				praddset(setp, flt);
			}
		}
		else if (strcmp(name, "ALL") == 0) {
			if (exclude) {
				premptyset(setp);
			} else {
				prfillset(setp);
			}
		}
		else {
			(void) fprintf(stderr,
				"%s: unrecognized fault name/number: %s\n",
				command, name);
			rc = -1;
		}
	}

	return rc;
}

int			/* gather file descriptors to dump */
fdlist(str, setp)		/* return 0 on success, != 0 on any failure */
	register char *str;		/* string of filedescriptors */
	register fileset_t *setp;	/* set of boolean flags */
{
	register char * name;
	register int exclude = FALSE;
	int rc = 0;

	upcase(str);
	name = strtok(str, sepr);

	if (name != NULL && *name == '!') {	/* exclude from set */
		exclude = TRUE;
		if (*++name == '\0')
			name = strtok(NULL, sepr);
	}

	for ( ; name; name = strtok(NULL, sepr)) {
		register int fd;
		char * next;

		if (*name == '!') {	/* exclude remainder from set */
			exclude = TRUE;
			while (*++name == '!')
				;
			if (*name == '\0')
				continue;
		}

		fd = strtol(name, &next, 0);
		if (fd >= 0 && fd < NOFILES_MAX && *next == '\0') {
			fd++;
			if (exclude) {
				prdelset(setp, fd);
			} else {
				praddset(setp, fd);
			}
		}
		else if (strcmp(name, "ALL") == 0) {
			if (exclude) {
				premptyset(setp);
			} else {
				prfillset(setp);
			}
		}
		else {
			(void) fprintf(stderr,
				"%s: filedescriptor not in range[0..%d]: %s\n",
				command, NOFILES_MAX-1, name);
			rc = -1;
		}
	}

	return rc;
}

static void
upcase(str)
	register char *str;
{
	register int c;

	while ((c = *str) != '\0')
		*str++ = toupper(c);
}
