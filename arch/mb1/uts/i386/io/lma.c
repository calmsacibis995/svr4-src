/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ifndef lint
static char lma_copyright[] = "Copyright 1989 Intel Corporation 465604-010";
#endif /* lint */

#ident	"@(#)mb1:uts/i386/io/lma.c	1.1"

/*
**	Kernel low memory allocator.
*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/immu.h>
#include <sys/ddi.h>

extern char lma_start[];
extern int  lma_size;
caddr_t	lma_ptr;
static unsigned long lma_allocated;
short lma_alive = 0;

/*
**	lmainit - called at system initialization time.
*/
void
lmainit()
{
	int i;
	if (lma_alive)
		return;
	lma_alive = 1;
	lma_allocated = 0;
	lma_ptr = lma_start;

	/*
	 * Now zero out the space allocated for lma driver.
	 */
	bzero ( (caddr_t) lma_start, lma_size);
}

/*
**	lma_alloc - called by clients requiring low memory addresses for their 
**	kernel data structures.
*/
caddr_t
lma_alloc(size)
	int size;
{
	caddr_t ret_val;

	if (!lma_alive)
		return (NULL);	/* Insure that the LMA init routine has been called */
	if (size < (lma_size - lma_allocated)) {
		ret_val = lma_ptr + lma_allocated;
		lma_allocated += size;
		return(ret_val);
	}
	return(NULL);
}
