/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_chdir.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_chdir.c	3.8	LCC);	/* Modified: 14:56:12 11/27/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include "pci_types.h"

#define	TOLOWER(c)	(isupper(c) ? tolower(c) : (c))
extern	void
	getcwd();		/* Update current directory working string */

extern	int
	unmapfilename();	/* Translate PCI filename if necessary */

extern	int errno;		/* contains error code from system calls */

void
#ifdef	MULT_DRIVE
pci_chdir(directory, drvNum, addr, request)
int
	drvNum;
#else
pci_chdir(directory, addr, request)
#endif
    char	*directory;		/* Target directory */
    struct	output	*addr;
    int		request;		/* DOS request number */
{
	register char *p;
	extern char *strrchr();
#ifdef HIDDEN_FILES 
	int attrib = 0;
#endif /* HIDDEN_FILES */ 
					 


    /* Translate MS-DOS directory to UNIX */
	cvt2unix(directory);
		
	/* this is dumb, but DOS doesn't like doing chdirs to places ending
	*  with backslashes
	*/

	if ( ((p = strrchr(directory,'/')) && (p != directory) && (*++p == 0))
		|| (*directory == 0) ) {
		addr->hdr.res = PATH_NOT_FOUND;
		return;
	}


    /* If unmapping filename returns collision return failure */
#ifdef	MULT_DRIVE
	directory = fnQualify(directory, CurDir);
#endif	/* MULT_DRIVE */
#ifdef HIDDEN_FILES
	attrib = 0;			/* don't change to hidden dirs */
	if ((unmapfilename(CurDir, directory,&attrib)) != FILE_IN_DIR) 
#else
	if ((unmapfilename(CurDir, directory)) == DUP_FILE_IN_DIR) 
#endif	/* HIDDEN_FILES */
	{
	    addr->hdr.res = PATH_NOT_FOUND;
	    return;
	}

    /* Update current working directory string */
	if (chdir(directory)) {
	    err_handler(&addr->hdr.res, request, directory);
	    return;
	}

    /* Construct new cwd and store in MS-DOS form */
	getcwd(directory, CurDir);	

/*
 * Becuase MS-DOS fills in the DPB with the current working directory
 * we must get the current directory and send it back to the bridge.
 * The mapped mode is used, because we want it to look like a standard
 * MS-DOS entry.
 */

#ifdef MULT_DRIVE		/* fill in current directory */
	pci_pwd(0, drvNum, addr);
#else
	pci_pwd(0, addr);
#endif
	p = addr->text;
	while (*p = (TOLOWER(*p)))	/* convert path to lower case */
		p++;
	
    /* Fill-in response header */
	addr->hdr.res = SUCCESS;
}

