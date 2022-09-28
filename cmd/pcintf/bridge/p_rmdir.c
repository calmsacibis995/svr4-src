/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_rmdir.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_rmdir.c	3.14	LCC);	/* Modified: 14:57:14 2/7/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"

#include	<string.h>
#include	<errno.h>

#if	0
extern	void
	cleanup();
#endif	/* 0 */

extern  char *malloc();

extern	int
	unmapfilename();

/*		Imported Structure		*/
extern	int
	errno;		/* contains error code returned from system calls */

#if defined(XENIX)
int rmdir(path)
	char *path;
{
	static char *argv[] = { "rmdir", (char *)0, (char *)0 };

	if (access(path,0))		/* check if the path exists */
		return -1;

	argv[1] = path;
	if (exec_cmd(argv[0], argv, -1))
		return 0;
	errno = EACCES;
	return -1;
}
#endif

void
#ifdef	MULT_DRIVE
pci_rmdir(dName, drvNum, addr, request)
int
	drvNum;
#else
pci_rmdir(dName, addr, request)
#endif
    char	*dName;			/* Name of directory to remove */
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    char
	*dotdir;			/* copy of CurDir to compare against */
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

    /* Translate MS-DOS to UNIX directory name */
	cvt2unix(dName);
#ifdef	MULT_DRIVE
	dName = fnQualify(dName, CurDir);
#endif	/* MULT_DRIVE */
#if	0
	cleanup(dName, strlen(dName));
#endif	/* 0 */

	/*  Different systems set different errno values on attempt to
	**  remove root, so we check for it here.
	*/

#ifdef HIDDEN_FILES
	attrib = 0;
#endif /* HIDDEN_FILES */

	if (!strcmp(dName, "/") ||
#ifdef HIDDEN_FILES
		/* NEEDS_WORK return val needs to be checked */
		unmapfilename(CurDir, dName, &attrib) == DUP_FILE_IN_DIR) 
#else
		unmapfilename(CurDir, dName) == DUP_FILE_IN_DIR) 
#endif /* HIDDEN_FILES */
	{
		addr->hdr.res = ACCESS_DENIED;
		return;
	}

	/* Error if attempting to remove the current working directory */
	dotdir = (char *) malloc(strlen(CurDir) + 3);	/* make a copy of cwd */
	strcpy (dotdir, CurDir);		/* with '/.' appended to */
	strcat (dotdir, "/.");			/* compare against */

	if (!strcmp(dName, CurDir) || !strcmp(dName, dotdir)) 
	{
	    free( dotdir );
	    addr->hdr.res = ATTEMPT_TO_REMOVE_DIR;
	    return;
	}
	free( dotdir );

	if (rmdir(dName) < 0) 
	{
	    if (errno == ENOENT)	/* DOS rd command can't return F_N_F */
		addr->hdr.res = PATH_NOT_FOUND;
	    else
		err_handler(&addr->hdr.res, request, dName);
	}
	else
	    addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
}
