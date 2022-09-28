/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/space.c	1.3.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>

/* Run queue of methods */
int	runqueue = 0;
int	runpriority = -1;

/* Number of active methods */
int nactive = 0;

/* Number of talking owners */
int ntalking = 0;

/* name of command for messages. */
char *brcmdname = "bkdaemon";

/* Complete state of things */
int state = 0;

/* process table, number of slots allocated, size of process table */
proc_t	*proctab;
int	nprocs = 0, proctabsz = 0;

/* Controling backup table */
control_t	*controltab;
int controltabsz = 0;

/* Method table */
method_t	*methodtab;
int	methodtabsz = 0;

/* Owner's table */
owner_t	*ownertab;
int	ownertabsz = 0;

/* Number of levels of nested Critical Regions */
int bklevels = 0;
