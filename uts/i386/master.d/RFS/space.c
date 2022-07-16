/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)//usr/src/uts/i386/master.d/RFS/space.c.sl 1.1 4.0 12/08/90 62007 AT&T-USL"

#include "sys/types.h"
#include "sys/list.h"
#include "sys/vnode.h"
#include "sys/nserve.h"
#include "sys/stream.h"
#include "sys/proc.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/rf_cirmgr.h"
#include "sys/fs/rf_acct.h"
#include "config.h"   		/* for tunable parameters */

int	nsrmount = NSRMOUNT;

int	nrcvd = NRCVD;

int	nsndd = NSNDD;

int	maxgdp = MAXGDP;

int	minserve = MINSERVE;
int	maxserve = MAXSERVE;

int	nrduser = NRDUSER;

int	rc_time = RCACHETIME;

int	rf_maxkmem = RF_MAXKMEM;
