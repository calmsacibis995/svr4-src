/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RF_SYS_H
#define _SYS_RF_SYS_H

#ident	"@(#)head.sys:sys/rf_sys.h	1.3.3.1"

/* 
 * external interfaces to the rfsys system call 
 */
#include <sys/types.h>
#include <sys/nserve.h>
#include <sys/errno.h>

#ifdef _KERNEL
extern char rfs_domain[];       	/* local domain name */
extern struct queue *sysid_to_queue();
#endif

/* opcodes for rfsys system call */
#define RF_FUMOUNT      1       /* forced unmount */
#define RF_SENDUMSG     2       /* send buffer to remote user-level */
#define RF_GETUMSG      3       /* wait for buffer from remote user-level */
#define RF_LASTUMSG     4       /* wakeup from GETUMSG */
#define RF_SETDNAME     5       /* set domain name */
#define RF_GETDNAME     6       /* get domain name */
#define RF_SETIDMAP     7
#define RF_FWFD         8
#define RF_VFLAG        9
#define RF_VERSION      10
#define RF_RUNSTATE     11      /* see if RFS is running */
/*
 * return the value of one of the RFS system variables, needs one of the
 * values defined below
 */
#define RF_TUNEABLE     12
#define RF_CLIENTS      13      /* fill a buffer with info about remote use of a
                                 * local resource
                                 */
#define RF_RESOURCES    14      /* fill a buffer with info about local resources
                                 */
#define RF_ADVFS	15
#define RF_UNADVFS	16
#define RF_START	17
#define RF_STOP		18
#define RF_DEBUG	19

#ifdef RFSUNMOUNTHACK
/*
 * Support unmounting of stuff below gone rf_vfs.  This will be supplanted
 * by a general mechanism in a later release.
 */
#define RF_GETCAP	20
#define RF_PUTCAP	21
#define RF_SUBMNTS	22
#define RF_FUSERS	23
#define RF_UNMOUNT	24	/* cap expires after call */
#endif

/* RF_VFLAG options */
#define V_CLEAR 0
#define V_SET   1
#define V_GET   2

/* RF_VERSION options */
#define VER_CHECK       1
#define VER_GET         2

/* RF_TUNEABLE options */
#define T_NSRMOUNT      1
#define T_NADVERTISE    2

/* struct client is used by the RF_CLIENTS option */
struct client {
        sysid_t cl_sysid;
        int     cl_bcount;
        char    cl_node[MAXDNAME];
};

/* codes passed to the user daemon */
#define	RFUD_DISCONN	1
#define RFUD_FUMOUNT	2
#define RFUD_GETUMSG	3
#define RFUD_LASTUMSG	4

#endif	/* _SYS_RF_SYS_H */
