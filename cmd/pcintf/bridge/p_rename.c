/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_rename.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_rename.c	3.17	LCC);	/* Modified: 16:30:48 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#define	INNAME(c)	((c) != DOT && (c) != NULL)

#include "pci_types.h"
#include "const.h"
#include "xdir.h"
#include <string.h>
#include <log.h>
#include <sys/stat.h>

#ifdef D_RENAME
#define sDbg(argument)  debug(0x100, argument)
#else
#define sDbg(argument)
#endif

#define	INNAME(c)	((c) != DOT && (c) != NULL)
#define	TMPDIR		"/tmp/"
#define	MV		"mv"

#ifndef	EOS
#define	EOS		0
#endif	/* EOS */

extern void bkslash(), lowercase(), changename();
extern int matchf(), exec_cmd(), unmapfilename();
extern char *memory(), *get_pattern(), *getpname();
extern struct temp_slot temp_slot[];

typedef struct filelist {
	char srcname[MAX_FNAME+1];    /* room for a simple filename (no path) */
	char dstname[MAX_FNAME+1];
	int tslot;			/* -1 indicates not redirected */
	struct filelist *next;
} FILELIST;
FILELIST flhead;

static char *args[] = { MV, NULL, NULL, NULL };	/* gets passed to exec_cmd() */
static char *form_target();
extern char *fnQualify();

#ifdef	MULT_DRIVE
pci_rename(source, target, request, mode, drvNum, addr)
	int drvNum;
#else    /* !MULT_DRIVE */
pci_rename(source, target, request, mode, addr)
#endif   /* !MULT_DRIVE */
	char *source, *target;
	int request;			/* distinguish between
					    old style/new style rename	*/
	int mode;
	struct output *addr;
{
	register FILELIST *flp = &flhead;
	register struct direct *dp;	/* dir ptr from readdir() */
	register DIR *dirp;		/* dir ptr from opendir() */
	char s_pattern[MAX_FNAME+1];
	char d_pattern[MAX_FNAME+1];
	char s_path[MAX_PATH+1];
	char d_path[MAX_PATH+1];
	char s_name[MAX_PATH+1];
	char d_name[MAX_PATH+1];
	char s_buf[MAX_PATH+1];
	char statname[MAX_PATH+1];
	struct stat filestat;
	int tslot;

#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

	addr->hdr.stat = NEW;

	sDbg(("rename(%s,%s,%d,%d)\n",source,target,mode,drvNum));
	strcpy(s_name, source); bkslash(s_name);      /* resolve backslashes */
	strcpy(d_name, target); bkslash(d_name);
	if (mode == MAPPED)     /* map DOS filename to UNIX format */
	{
		lowercase(s_name, strlen(s_name));
		lowercase(d_name, strlen(d_name));
	}

	/* kludge to allow access to protected directories for tmps */
	tslot = -1;
	while ((tslot = redirdir(s_name, tslot + 1)) >= 0) {
	    strcpy(flp->srcname, temp_slot[tslot].fname);
	    if (mode == MAPPED)
		{
#ifdef HIDDEN_FILES
			attrib = 0;			/* NEEDS_WORK unmap return val s/b checked */
			unmapfilename(TMPDIR, flp->srcname, &attrib);
#else														
			unmapfilename(TMPDIR, flp->srcname);
#endif /* HIDDEN_FILES */
		}
	    flp->tslot = tslot;
	    flp->next = (FILELIST *)memory(sizeof(FILELIST));
	    flp = flp->next;
	}

	if (mode == MAPPED)     /* map DOS filename to UNIX format */
#ifdef HIDDEN_FILES
		attrib = 0;
		unmapfilename(CurDir, s_name, &attrib);
#else
		unmapfilename(CurDir, s_name);
#endif /* HIDDEN_FILES */

	getpath(s_name, s_path, s_pattern);
	getpath(d_name, d_path, d_pattern);
	if (request == RENAME_NEW && (strpbrk(s_pattern, "?*") ||
	                              strpbrk(d_pattern, "?*"))) {
		/* wildcards are disallowed in file handle renames */
		addr->hdr.res = PATH_NOT_FOUND;		/* sic */
		return;
	}
	if (access(d_path, 00) < 0) {
		addr->hdr.res = PATH_NOT_FOUND;
		return;
	}

#ifdef	MULT_DRIVE
	if (strcmp(s_path, ".") == 0)
	    strcpy(s_path, CurDir);
#endif   /* !MULT_DRIVE */
	sDbg(("s_path=%s\n", s_path));
	if ((dirp = opendir(s_path)) == NULL)
	{
		addr->hdr.res = ACCESS_DENIED;
		return;
	}
	sDbg(("s_pattern=%s\n", s_pattern));
	while (dp = readdir(dirp))  	 /* find all valid files */
	{
		/* check for filenames "." and ".." */
		if (dp->d_name[0] == '.' &&
		  ((dp->d_name[1] == '.' && dp->d_name[2] == NULL) ||
		    dp->d_name[1] == NULL))
			continue;
		/* see if we've found a filename match */
		sDbg(("   match %s ?\n", dp->d_name));
		if (matchf(dp->d_name, s_pattern))
		{
    		/* Construct pathname for stat() */
		    strcpy(statname, s_path);
		    strcat(statname, "/");
		    strcat(statname, dp->d_name);
		    sDbg(("pathname for stat=%s\n",statname));

		    if ((stat(statname,&filestat)) == 0)
		/*   Check to see if the name matched on is
		     directory -- old style REN cannot be used to rename
		     directories under DOS -- non-wildcard directory
		     names are caught before this point */
			if (((0xf000 & filestat.st_mode) == 0x4000) &&
			 (request == RENAME))	/* check for old style REN */
			{   sDbg(("  mode=%x\n", filestat.st_mode));
			    sDbg((" ** match=%s, but is directory\n",
				flp->srcname));
			    continue;
			}
			else	/* New style REN or Not a directory	*/
			{   sDbg(("  mode=%x\n", filestat.st_mode));
			    strcpy(flp->srcname, dp->d_name);
			    sDbg((" ** match=%s\n", flp->srcname));
			    flp->tslot = -1;
			    flp->next = (FILELIST *) memory(sizeof(FILELIST));
			    flp = flp->next;
			}
		}
	}
	flp->srcname[0] = NULL;
	closedir(dirp);

	if (flhead.srcname[0] == NULL)      /* check for no files found */
	{
		sDbg(("No match\n"));
		addr->hdr.res = FILE_NOT_FOUND;
		return;
	}

      /* found the source filename(s), so form target(s) and do the moving */
	for (flp = &flhead; flp->srcname[0]; flp = flp->next)
	{
		register FILELIST *p;

		strcpy(flp->dstname, form_target(flp->srcname, d_pattern));
		/* check for duplicate target name */
		for (p = &flhead; p != flp; p = p->next)
		{
			if (strcmp(p->dstname, flp->dstname) == 0)
			{
				addr->hdr.res = ACCESS_DENIED; /* dup target */
				return;
			}
		}
	      /* Convert to full path file names */
		strcpy(d_name, fnQualify(flp->dstname, d_path));
		strcpy(s_name, fnQualify(flp->srcname,
					(flp->tslot >= 0) ? TMPDIR : s_path));
		sDbg(("rename %s to %s\n", s_name, d_name));
		if (access(d_name, 0) == 0)  /* fail if target exists */
		{
			addr->hdr.res = ACCESS_DENIED;	/* target exists */
			return;
		}
		else
		{
			args[1] = s_name;
			args[2] = d_name;
			if (exec_mv(MV, args)) /* do the move */
			{
				/* update name/pathname in open file table */
				changename(s_name, d_name);
				if (flp->tslot >= 0)
					temp_slot[flp->tslot].s_flags = 0;
			}
			else
			{
				addr->hdr.res = FILE_NOT_FOUND; /* MV failed */
				return;
			}
		}
	}
	addr->hdr.res = SUCCESS;
	return;
}

/*
 * form_target:	Forms the target name based on the source name.
 *		Returns the final target name.
 *
 * input	s- source name, a real file name
 *		t- target pattern, may contain wildcards
 *
 * output-	returns a pointer to a new file name
 *
 * processing	creates a new file name by modifying the given
 *		source name to fit the given target pattern
 *
 * This function is a cleaned up version of the original and was lifted
 * out of PCI/SMB. It fixes the bug with renaming wild cards that was
 * in the original code.	[11/4/88 efh]
 */

static char *
form_target(s, t)
register char	*s;			/* ptr to source name */
register char	*t;			/* ptr to target name */
{
static char	target[MAX_FNAME];	/* buffer for final target name */
register char	*tp = target;		/* ptr to buffer */

	/* Process basename (part before the '.') first */
	for (;;) {
		if (*s == EOS)
			goto baseOut;
		else if (*s == DOT) {
			while (INNAME(*t)) { /* process rest of target prefix */
				if (*t == QUESTION || *t == STAR) {
					while (INNAME(*t))
						t++;
					break;
				}
				else
					*tp++ = *t++;
			}
			goto baseOut;
		} else {
			switch (*t) {
			case STAR:
				/* copy source chars */
				while (INNAME(*s))
					*tp++ = *s++;
				/* skip stuff after * in dest */
				while (INNAME(*t))
					t++;
				break;

			case QUESTION:	/* copy single source char */
				*tp++ = *s++;
				t++;
				break;

			case DOT:	/* skip rest of source prefix */
				while (INNAME(*s))
					s++;
				break;

			default:	/* regular character - copy over */
				*tp++ = *t++;
				s++;
				break;
			}
		}
	}

baseOut:
	/* handle suffix */
	if (*t != EOS) {
		/* Skip over dot */
		if (*s != EOS)
			s++;

		if (*t == DOT)
			*tp++ = *t++;

		while (*t) {
			switch (*t) {
			case STAR:
			case EOS:
				while (*s)
					*tp++ = *s++;
				*tp = EOS;
				return target;

			case QUESTION:
				if (*s) *tp++ = *s;
				t++;
				break;

			default:	/* regular character */
				*tp++ = *t++;
				break;
			}

			if (*s != EOS)
				s++;
		}
	}

	*tp = EOS;
	return target;
}

int
exec_mv(mv, args)
char *mv, *args[];
{
    char *cp1, *cp2;
    int flag;
    
    log("renaming %s to %s\n",args[1],args[2]);
    cp1 = strrchr(args[1],'/');
    if (cp1) *cp1 = 0;
    cp2 = strrchr(args[2],'/');
    if (cp2) *cp2 = 0;
    flag = strcmp(args[1], args[2]);
    if (cp1) *cp1 = '/';
    if (cp2) *cp2 = '/';
    
    if (flag == 0) {
	log("attempting fast rename\n");
	if (link(args[1], args[2]) == 0) {
	    if (unlink(args[1]) == 0) {
		return(1);			/* successful */
	    }
	    else {
		log("unlink failed\n");
		unlink(args[2]);
	    }
	}
    }
    log("exec'ing mv\n");
    return (exec_cmd(mv, args, -1));
}
