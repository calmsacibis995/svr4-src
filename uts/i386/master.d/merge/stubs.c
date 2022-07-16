/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)master:merge/stubs.c	1.3.1.1"

/***************************************************************************

       Copyright (c) 1986, 1988 Locus Computing Corporation.
       All rights reserved.
       This is an unpublished work containing CONFIDENTIAL INFORMATION
       that is the property of Locus Computing Corporation.
       Any unauthorized use, duplication or disclosure is prohibited.

***************************************************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/immu.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/errno.h>

#include "config.h"

int  	mergedebug = 0;
struct vm86	*vmonstate = 0;

/*
	Functions that do nothing but return for stubs.
*/

chkvm86ints() 			{ /* do nothing */ }
vm86exit() 			{ /* do nothing */ }
vm86_clock() 			{ /* do nothing */ }
vm86_swtch() 			{ /* do nothing */ }
win_update()			{ /* do nothing */ }
vm86_regdump()			{ /* do nothing */ }

int find_file()	/* real args are (rp, offset) */
{
	return 0;
}

vm86_trap(r0ptr)
int * r0ptr;
{
	return(1);
}

uint
vm86pdetova()
{
	return(-1);
}

isdosexec()
{
	return(0);
}

vm86()
{
	u.u_error = EINVAL;
}

/**
 **	Stubs for device driver hooks
 **/

portalloc(lo, hi)
int lo, hi;
{
	return (1);
}

portfree(lo, hi)
int lo, hi;
{
	return (1);
}


int asy_is_assigned()
{
	return 0;
}

int com_ppiioctl()
{
	return 0;
}

int floppy_free()
{
	return (1);
}

int flp_for_dos()
{
	return 0;
}

kdppi_ioctl()
{
	return 0;
}
