/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_RFS_RF_ADMIN_H
#define _FS_RFS_RF_ADMIN_H

#ident	"@(#)kern-fs:rfs/rf_admin.h	1.3"

/*
 * declarations of RFS internal administrative interfaces.
 */

/* rf_recovery opcodes */
#define REC_DISCONN	1	/* circuit gone */
#define REC_KILL	2	/* exit */
#define REC_FUMOUNT	3	/* forced unmount */
#define REC_MSG		4

/* rf_recovery_flag values */
#define RFRECDISCON	0x01
#define RFRECFUMOUNT	0x02
/*
 * rf_daemon_flag flag */
#define  RFDSERVE	0x01
#define  RFDDISCON	0x02
#define  RFDKILL	0x04
#define  RFDASYNC	0x08

/* This is the structure passed to the user daemon */
#define ULINESIZ 120	/* want a 128-char buffer */
struct u_d_msg {
	int opcode;
	int count;
	char res_name[ULINESIZ];
};

/*
 * Structure that may be registered for rf_daemon processing via the
 * rfa_workenq operation.  rf_daemon always processes its work queue
 * by dequeuing a structure, then calling rfaw_func with the structure
 * address.
 *
 * The user is reponsible for ALL cleanup, including structure disposal.
 *
 * Passed by reference to rfa_workenq.
 *
 * NOTE: rfaw_func may be called with RFS in intermediate state,
 * because the work queue is cleaned up when RFS is started and stopped.
 */
typedef struct rfa_work {
	ls_elt_t	rfaw_elt;
	void		(*rfaw_func)();
	caddr_t		rfaw_farg;
} rfa_work_t;

extern int	rfa_workenq();

extern void	rf_signal_serve();
extern void	rf_checkq();
extern void	(*rf_daemon())();
extern void	rfd_stray();	/* Handle stray messages.*/


extern int		rf_recovery_flag;
extern struct proc	*rf_recovery_procp;
extern struct rcvd	*rf_daemon_rd;
extern int		rf_daemon_flag;
extern int		rf_daemon_lock;
#endif /* _FS_RFS_RF_ADMIN_H */
