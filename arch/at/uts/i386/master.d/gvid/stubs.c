/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/master.d/gvid/stubs.c	1.3"

/*
 *	stubs.c file for GVID driver
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/kmem.h"
#include "sys/tty.h"
#include "sys/stream.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/session.h"
#include "sys/vnode.h"
#include "sys/genvid.h"
#include "sys/cmn_err.h"
#include "sys/ddi.h"

gvid_t Gvid = {0};
int gvidflg = 0;
