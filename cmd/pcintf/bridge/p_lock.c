/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_lock.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_lock.c	3.12	LCC);	/* Modified: 16:29:44 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<sys/stat.h>
#include	<errno.h>

#include	<string.h>

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

extern long
	lseek(),
	time();

extern void
	free();


extern int
	swap_in(),
	exec_cmd(),
	close_file();

extern	int
	errno;


void
pci_lock(vdescriptor, mode,
#ifdef RLOCK  /* record locking */
			dospid, 	/* dos pid asking for lock */
#endif  /* RLOCK */
			offset, length, addr, request)

    int		vdescriptor;		/* PCI virtual file descriptor */
    int		mode;			/* Lock or unlock */
#ifdef RLOCK  /* record locking */
    unsigned short
		dospid;	 		/* dos pid asking for lock */
#endif  /* RLOCK */
    long	offset, length;		/* Position and size of lock */
    struct	output	*addr;
    int		request;		/* DOS request number simulated */
{
#ifdef RLOCK  /* record locking */
    int		status,			/* Return value from system call */
		adescriptor;		/* Acutual UNIX file descriptor */
    char
		*args[NARGS];		/* Arguments for execvp() */

    struct	stat
		filstat;		/* Buffer contains data from stat() */

	    if ((adescriptor = swap_in(vdescriptor, DONT_CARE)) < 0) {
		    /* returns "invalid file descriptor" to dos, even if
		      the problem is just a lack of available unix descriptors.
		      This doesn't seem right but the way swap_in was written
		      and is used would take too much time to change now.
		    */
		addr->hdr.res =
			(adescriptor == NO_FDESC) ? FILDES_INVALID : FAILURE;
		return;
	    }

	    if (fstat(adescriptor, &filstat) < 0) {
		err_handler(&addr->hdr.res, request, NULL);
		return;
	    }

	    /* lock or unlock the file */
	    if (lock_file(vdescriptor,dospid,mode,offset,length) < 0) {
		addr->hdr.res = LOCK_VIOLATION;
		return;
	    }
#endif  /* RLOCK */

	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
	return;
}
