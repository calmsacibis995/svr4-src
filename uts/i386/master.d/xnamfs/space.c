/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:xnamfs/space.c	1.3"

#include "sys/types.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fs/xnamnode.h"
#include "sys/sd.h"
#include "config.h"	/* to collect tunables */

struct	xsem xsem[XSEMMAX];
int	nxsem = XSEMMAX;
struct	xsd xsd[XSDSEGS];
int	nxsd = XSDSEGS;
struct  sd sdtab[XSDSEGS * XSDSLOTS];
