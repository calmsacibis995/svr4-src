/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_exit.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_exit.c	3.5	LCC);	/* Modified: 15:38:01 7/14/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"

extern void
	deldir_pid(),	/* Delete directory contexts of process exiting  */
	delfile_pid();	/* Delete file contexts of process exiting */


void
pci_exit(pid, addr)
    int		pid;			/* Id of MS-DOS process exiting */
    struct	output	*addr;		/* Pointer to response buffer */
{

    /* Delete all search contexts and file contexts with process id */
	deldir_pid((int)pid);
	delfile_pid((int)pid);
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
}
