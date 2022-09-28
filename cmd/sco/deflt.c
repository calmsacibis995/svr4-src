/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:deflt.c	1.1"

/*
 *	@(#) deflt.c 1.1 90/02/15 sco:deflt.c
 *
 *	Copyright (C) Microsoft Corporation, 1983
 *
 *	This Module contains Proprietary Information of Microsoft
 *	Corporation and AT&T, and should be treated as Confidential.
 */

/*	deflt.c - Default Reading Package
 *
 *      this package consists of the routines:
 *
 *              defopen()
 *              defread()
 *		defcntl()		M000
 *
 *      These routines allow one to conveniently pull data out of
 *      files in
 *                      /etc/default/
 *
 *      and thus make configuring a utility's defopens standard and
 *      convenient.
 *
 */

#include <stdio.h>

#define	FOREVER		for ( ; ; )
#define	TSTBITS(flags, mask)	(((flags) & (mask)) == (mask))
#define	ISON(flags, mask)	TSTBITS(flags, mask)
#define	ISOFF(flags, mask)	(!ISON(flags, mask))

extern	int	errno;

static		strlower(), strnlower();

/*
 * M000
 * Following for defcntl(3).
 * If things get more complicated we probably will want to put these
 * in an include file.
 * If you add new args, make sure that the default is
 *	OFF	new-improved-feature-off, i.e. current state of affairs
 *	ON	new-improved-feature-on
 * (for compatibility).
 */
/* ... flags */
static	int	Dcflags;		/* [re-]initialized on each call to defopen() */

/* ... cmds */
#define	DC_GETFLAGS	0	/* get current flags */
#define	DC_SETFLAGS	1	/* set flags */

/* ... args */
#define	DC_CASE		0001	/* ON: respect case; OFF: ignore case */

	/* 'standard'; must remain compatible !!! */
#define	DC_STD		((0) | (DC_CASE))


/*      defopen() - declare defopen filename
 *
 *      defopen(cp)
 *              char *cp
 *
 *      If 'cp' is non-null; it is a full pathname of a file
 *      which becomes the one read by subsequent defread() calls.
 *      If 'cp' is null the defopen file is closed.
 *
 *      see defread() for more details.
 *
 *      EXIT    returns 0 if ok
 *              returns -1 if error
 */


static	FILE	*dfile	= NULL;


int
defopen(fn)
char *fn;
{

	if (dfile != (FILE *) NULL)
		fclose(dfile);

	if (fn == (char *) NULL) {	/* M001 */
		dfile = NULL;
		return(0);
	}

	if ((dfile = fopen(fn, "r")) == (FILE *) NULL)
		return(-1);

	Dcflags = DC_STD;	/* M000 */

	return(0);
}



/*      defread() - read an entry from the defopen file
 *
 *      defread(cp)
 *              char *cp
 *
 *      The defopen data file must have been previously opened by
 *      defopen().  defread scans the data file looking for a line
 *      which begins with the string '*cp'.  If such a line is found,
 *      defread returns a pointer to the first character following
 *      the matched string (*cp).  If no line is found or no file
 *      is open, defread() returns NULL.
 *
 *      Note that there is no way to simulatniously peruse multiple
 *      defopen files; since there is no way of indicating 'which one'
 *      to defread().  If you want to peruse a secondary file you must
 *      recall defopen().  If you need to go back to the first file,
 *      you must call defopen() again.
 */

char *
defread(cp)
char *cp;
{
	static char buf[256];	/* M002 */
	register int len;
	register int patlen;

	if (dfile == (FILE *) NULL)
		return((char *) NULL);
	patlen = strlen(cp);

	rewind(dfile);
	while (fgets(buf, sizeof(buf), dfile)) {
		len = strlen(buf);
		if (buf[len-1] == '\n')
			buf[len-1] = 0;
		else
			return((char *) NULL);		/* line too long */
		if (!TSTBITS(Dcflags, DC_CASE)) {	/* M000 ignore case */
			strlower(cp, cp);
			strnlower(buf, buf, patlen);
		}
		if (strncmp(cp, buf, patlen) == 0)
			return(&buf[patlen]);           /* found it */
	}
	return((char *) NULL);
}

/* M000 new routine */
/***	defcntl -- default control
 *
 *	SYNOPSIS
 *	  oldflags = defcntl(cmd, arg);
 *
 *	ENTRY
 *	  cmd		Command.  One of DC_GET, DC_SET.
 *	  arg		Depends on command.  If DC_GET, ignored.  If
 *		DC_GET, new flags value, created by ORing the DC_* bits.
 *	RETURN
 *	  oldflags	Old value of flags.  -1 on error.
 *	NOTES
 *	  Currently only one bit of flags implemented, namely respect/
 *	  ignore case.  The routine is as general as it is so that we
 *	  leave our options open.  E.g. we might want to specify rewind/
 *	  norewind before each defread.
 */

int
defcntl(cmd, newflags)
register int  cmd;
int  newflags;
{
	register int  oldflags;

	switch (cmd) {
	case DC_GETFLAGS:		/* query */
		oldflags = Dcflags;
		break;
	case DC_SETFLAGS:		/* set */
		oldflags = Dcflags;
		Dcflags = newflags;
		break;
	default:			/* error */
		oldflags = -1;
		break;
	}

	return(oldflags);
}

/* M000 new routine */
/***	strlower -- convert upper-case to lower-case.
 *
 *	Like strcpy(3), but converts upper to lower case.
 *	All non-upper-case letters are copied as is.
 *
 *	ENTRY
 *	  from		'From' string.  ASCIZ.
 *	  to		'To' string.  Assumed to be large enough.
 *	EXIT
 *	  to		filled in.
 */

static
strlower(from, to)
register char  *from, *to;
{
	register int  ch;

	FOREVER {
		ch = *from++;
		if ((*to++ = tolower(ch)) == '\0')
			break;
	}

	return;
}

/* M000 new routine */
/***	strnlower -- convert upper-case to lower-case.
 *
 *	Like strncpy(3), but converts upper to lower case.
 *	All non-upper-case letters are copied as is.
 *
 *	ENTRY
 *	  from		'from' string.  May be ASCIZ.
 *	  to		'to' string.
 *	  cnt		Max # of chars to copy.
 *	EXIT
 *	  to		Filled in.
 */

static
strnlower(from, to, cnt)
register char  *from, *to;
register int  cnt;
{
	register int  ch;

	while (cnt-- > 0) {
		ch = *from++;
		if ((*to++ = tolower(ch)) == '\0')
			break;
	}
}
