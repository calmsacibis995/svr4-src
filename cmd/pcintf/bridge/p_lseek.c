/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_lseek.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_lseek.c	3.6	LCC);	/* Modified: 20:31:18 10/25/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include <errno.h>
#include "pci_types.h"


#ifdef ATT3B2
typedef unsigned short u_short;
#endif


extern long
	lseek(),
	time();

void
pci_lseek(vdescriptor, offset, whence, inode, addr, request)
    int		vdescriptor;		/* PCI virtual file descriptor */
    long	offset;			/* Location to move file */
    int		whence;
    u_short	inode;
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    register int
	adescriptor;			/* Actual UNIX descriptor */

    long
	status;				/* Return from system call */

    /* Get actual UNIX file descriptor */
	if ((adescriptor = swap_in(vdescriptor, inode)) < 0) {
	    addr->hdr.res =
			(adescriptor == NO_FDESC) ? FILDES_INVALID : FAILURE;
	    return;
	}

	if (whence<0 || whence>2) {
		addr->hdr.res = INVALID_FUNCTION;
		return;
	}

	if ((status = lseek(adescriptor, offset, whence)) < 0)
	{
	    if (errno != EINVAL)
	    {
		err_handler(&addr->hdr.res, request, NULL);
		return;
	    }
	    /*
	       Was a seek to before the begginning,  which does not produce an
	       immediate error response from DOS, so we cannot return an error
	       code now.  For now, just lseek to end of file plus one.  A real
	       fix must remember that the offset was bad, and report the error
	       on the next read or write.
	    */
	    lseek(adescriptor, 1L, 2);
	}

    /* Fill-in response buffer */
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
	addr->hdr.offset  = status;
}
