/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-vm:memprobe.c	1.1"

#include "sys/types.h"
#include "sys/user.h"
#include "vm/faultcatch.h"

/* memprobe(vaddr)
 * assumes vaddr is a potentially valid virtual address.
 * returns 0 if the referenced page is valid, non-zero otherwise.
 */

int
memprobe(vaddr)
	caddr_t	vaddr;
{
	int	dummy;

	CATCH_FAULTS(CATCH_ALL_FAULTS)
		dummy = *(int *)((int)vaddr & ~(sizeof (int) - 1));
	return END_CATCH();
}
