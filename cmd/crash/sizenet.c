/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:sizenet.c	1.7.9.1"

/*
 * This file contains code for the crash function size.  The RFS and
 * Streams tables and sizes are listed here to allow growth and not
 * overrun the compiler symbol table.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#define _KERNEL
#include <sys/stream.h>
#include <sys/strsubr.h>
#undef _KERNEL
#include <sys/list.h>
#include <sys/rf_messg.h>
#include <sys/rf_comm.h>
#include <sys/nserve.h>
#include <sys/rf_cirmgr.h> 
#include <sys/rf_adv.h>
#include "crash.h"

struct sizetable {
	char *name;
	char *symbol;
	unsigned size;
	int indirect;
};
struct sizetable siznettab[] = {
	"datab","dblock",sizeof (struct datab),0,
	"dblk","dblock",sizeof (struct datab),0,
	"dblock","dblock",sizeof (struct datab),0,
	"gdp","gdp",sizeof (struct gdp), 0,
	"linkblk","linkblk",sizeof (struct linkblk),0,
	"mblk","mblock",sizeof (struct msgb),0,
	"mblock","mblock",sizeof (struct msgb),0,
	"msgb","mblock",sizeof (struct msgb),0,
	"queue","queue",sizeof (struct queue),0,
	"rcvd","rcvd",sizeof (struct rcvd),0,
	"sndd","sndd",sizeof (struct sndd),0,
	"sr_mount","sr_mount",sizeof (struct sr_mount),0,
	"resource","resource",sizeof (struct rf_resource),0,
	"stdata","streams",sizeof (struct stdata),0,
	"streams","streams",sizeof (struct stdata),0,
	NULL,NULL,0,0
};	
