/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_mkdir.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_mkdir.c	3.12	LCC);	/* Modified: 14:55:13 2/7/90 */

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

extern	int
	unmapfilename();

extern	char
	*legal_pathname();

#if defined(XENIX)
int mkdir(path, mode)
	char *path;
	int mode;
{
	static char buf[6];
	static char *argv[] = {
		"mkdir", (char *)0, (char *)0
	};

	argv[1] = path;
	if (exec_cmd(argv[0], argv, -1) && !chmod(path,mode))
		return 0;
	errno = EACCES;
	return -1;
}
#endif

void
#ifdef	MULT_DRIVE
pci_mkdir(directory, drvNum, addr, request)
int
	drvNum;
#else
pci_mkdir(directory, addr, request)
#endif
    char	*directory;		/* Name of directory to create */
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* Number of DOS request simulated */
{
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

    /* Check for leading&trailing blanks, empty strings, illegal char's, etc. */
    if ( !(directory = legal_pathname(directory)) ) 
	{
		addr->hdr.res = PATH_NOT_FOUND;
		return;
    }

    /* Translate MS-DOS to UNIX directory name */
    cvt2unix(directory);
#if	0
    cleanup(directory, strlen(directory));
#endif	/* 0 */

#ifdef	MULT_DRIVE
    directory = fnQualify(directory, CurDir);
#endif

#ifdef HIDDEN_FILES
	attrib = 0;
    if ((unmapfilename(CurDir, directory, &attrib)) == DUP_FILE_IN_DIR)
#else
    if ((unmapfilename(CurDir, directory)) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */
	{
	addr->hdr.res = FILE_EXISTS;
	return;
    }

    if (mkdir(directory, 0777) < 0)	/* mode modified by umask */
		err_handler(&addr->hdr.res, request, directory);
    else
		addr->hdr.res = SUCCESS;
    addr->hdr.stat = NEW;
}
