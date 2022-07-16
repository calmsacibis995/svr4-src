/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:xt/space.c	1.3"

#include "sys/types.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/xtproto.h"
#ifdef VPIX
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/v86.h"
#endif
#include "sys/xt.h"

#define LINKSIZE  (sizeof(struct Link) + sizeof(struct Channel) * (MAXPCHAN-1))

#include "config.h"	/* to collect other tunable parameters */

struct  tty     xt_tty[NUMXT] ;
char    xt_buf[NUMXT * LINKSIZE] ;
int     xt_cnt ={NUMXT} ;
