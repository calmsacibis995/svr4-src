/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.usr:rpcsvc/rstat.h	1.1.5.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
/*	@(#)rstat.h 1.5 89/03/24  */

/*
 * This file should not be replaced with the output of rpcgen as
 * some of it has changed from the generic rpcgen output
 */

#ifndef _rpcsvc_rstat_h
#define _rpcsvc_rstat_h



#ifndef CPUSTATES
#define	CPUSTATES	4
#define	CP_USER		0
#define	CP_NICE		1
#define	CP_SYS		2
#define	CP_IDLE		3
#endif

#ifndef DK_NDRIVE
#define	DK_NDRIVE	10
#endif

#include <rpc/types.h>

struct stats {				/* version 1 */
	int cp_time[4];
	int dk_xfer[4];
	u_int v_pgpgin;		/* these are cumulative sum */
	u_int v_pgpgout;
	u_int v_pswpin;
	u_int v_pswpout;
	u_int v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
};
typedef struct stats stats;
bool_t xdr_stats();

struct statsswtch {				/* version 2 */
	int cp_time[4];
	int dk_xfer[4];
	u_int v_pgpgin;
	u_int v_pgpgout;
	u_int v_pswpin;
	u_int v_pswpout;
	u_int v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	u_int v_swtch;
	long avenrun[3];
	struct timeval boottime;
};
typedef struct statsswtch statsswtch;
bool_t xdr_statsswtch();

struct statstime {				/* version 3 */
	int cp_time[4];
	int dk_xfer[4];
	u_int v_pgpgin;
	u_int v_pgpgout;
	u_int v_pswpin;
	u_int v_pswpout;
	u_int v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	u_int v_swtch;
	long avenrun[3];
	struct timeval boottime;
	struct timeval curtime;
};
typedef struct statstime statstime;
bool_t xdr_statstime();

struct statsvar {				/* version 4 */
	struct {
		u_int cp_time_len;
		int *cp_time_val;
	} cp_time;			/* variable sized */
	struct {
		u_int dk_xfer_len;
		int *dk_xfer_val;
	} dk_xfer;			/* variable sized */
	u_int v_pgpgin;
	u_int v_pgpgout;
	u_int v_pswpin;
	u_int v_pswpout;
	u_int v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	u_int v_swtch;
	long avenrun[3];
	struct timeval boottime;
	struct timeval curtime;
};
typedef struct statsvar statsvar;
bool_t xdr_statsvar();

#define RSTATPROG ((u_long)100001)
#define RSTATVERS_SWTCH ((u_long)2)
#define RSTATPROC_STATS ((u_long)1)
extern statsswtch *rstatproc_stats_2();
#define RSTATPROC_HAVEDISK ((u_long)2)
extern long *rstatproc_havedisk_2();
#define RSTATVERS_TIME ((u_long)3)
extern statstime *rstatproc_stats_3();
extern long *rstatproc_havedisk_3();
#define RSTATVERS_VAR ((u_long)4)
extern statsvar *rstatproc_stats_4();
extern long *rstatproc_havedisk_4();
int havedisk();

#endif /*!_rpcsvc_rstat_h*/
