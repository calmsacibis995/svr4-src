/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:weitek/space.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/immu.h"
#include "sys/map.h"
#include "sys/proc.h"
#include "sys/vnode.h"
#include "sys/fs/s5inode.h"
#include "sys/file.h"
#include "sys/cred.h"
#include "sys/callo.h"
#include "sys/var.h"
#include "sys/mount.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/sysinfo.h"
#include "sys/tty.h"
#include "sys/conf.h"
#include "sys/utsname.h"
#include "sys/sema.h"
#include "sys/acct.h"
#include "vm/page.h"
#include "sys/weitek.h"

#include "config.h"	/* to override parameters in kdef.h */

/*
 * Tables and initializations
 */

/*  All bit fields are #define'd in sys/weitek.h.
 *  must always have WFPB17, and WFPB24.
 *  can have one of WFPR? plus one of WFPRI?,
 *  and to turn off a particular exception, may have any of 
 *  the exception bit masks.
 */
unsigned long   weitek_cfg = WFPRN | WFPRIZ | WFPB24 | WFPPM | WFPUM | WFPB17;

unsigned long	weitek_paddr = 0xC0000000;	/* chip physical address */

int	weitek_pt = -1;				/* No weitek page table as yet */
