/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_serve.h	1.3.1.1"
#ifndef _FS_RFS_SERVE_H
#define  _FS_RFS_SERVE_H

/*
 * Declarations internal to the RFS/DUFST server
 *
 * REQUIRES signal.h for SIGTERM, etc.
 * REQUIRES stream.h for mblk_t
 * REQUIRES rf_comm.h for rcvd_t
 * REQUIRES types.h for size_t, etc.
 * REQUIRES cred.h for cred_t
 */

/* '-1' because of 1 char in struct dirent */
#define RF_MAXDIRENT	(sizeof(struct dirent) + MAXNAMELEN - 1)

/*
 * Govern control flow for main server loop.  Server ops return these
 * values as function results.
 */
typedef enum {
	SR_NORMAL,		/* continue with the current message */
	SR_OUT_OF_BAND,		/* there is a new message block to process */
	SR_NO_RESP,		/* don't respond to client */
	SR_PATH_RESP,		/* subsumes EDOTDOT and EPATHREVAL */
	SR_NACK_RESP		/* respond with a NACK message */
} rfsr_ctrl_t;

/*
 * This structure records the global server state; it
 * is passed by reference to each of the server system
 * call operations, even though not all operations
 * need all members of the structure.  This is a consequence
 * of the original structure of the server, where cases in
 * a BIG switch statement changed data that was shared across
 * the whole server, much like a Pascal main program with
 * subprograms changing non-local data through up-level references.
 * In effect, we are passing a static link here.
 */
typedef struct rfsr_state {
	rcvd_t		*sr_gift;	/* gifts to client saved here */
	mblk_t		*sr_in_bp;	/* streams message block containing
					 * request
					 */
	mblk_t		*sr_out_bp;	/* streams message block containing
					 * response
					 */
	struct gdp	*sr_gdpp;	/* ptr to virtual circuit */
	off_t		sr_oldoffset;	/* save incoming io offset */
	int		sr_bcount;	/* read block count */
	rcvd_t		*sr_rdp;	/* request arrived on rd denoting local
					 * file */
	queue_t		*sr_qp;		/* ptr to stream queue where sr_in_bp
					   came in */
	int		sr_vcver;	/* virtual circuit version */
	int		sr_opcode;	/* op in progress */
	rf_resource_t	*sr_rsrcp;	/* rf_resource_t of request */
	sr_mount_t	*sr_srmp;	/* srmount_t of request */
	int		sr_ret_val;	/* return value for response message */
	cred_t		*sr_cred;	/* cred structure for server process */
	char		sr_client[MAXDNAME];
} rfsr_state_t;

/*
 * Freeing incoming messages is more ad-hoc than we would like.  Generally,
 * we free a message ASAP, after extracting arguments.  Sometimes, we MUST
 * free it to reuse the pointer.  Other times, we delay until getting to
 * rfsr_ret, which is prepared to clean up.
 *
 * TO DO:  clean up uses of this.
 */

#define SR_FREEMSG(stp) {		\
	rf_freemsg((stp)->sr_in_bp);	\
	(stp)->sr_in_bp = NULL;		\
}

/*
 * Table indexed by opcode points to functions handling remote request.
 */
extern int	(*rfsr_ops[])();
extern int	rfsr_undef_op();	/* Handle spurious requests */

/*
 * Map phony devs handed off to client back into local devs.
 */
extern void	rfsr_dev_init();
extern struct vfs *rfsr_dev_dtov();

/*
 * Subroutines
 */
extern int	rfsr_gift_setup();
extern mblk_t	*rfsr_chkrdq();
extern void	rfsr_exit();
extern int	rfsr_sigck();
extern int	rfsr_cacheck();
extern void	rfsr_adj_timeskew();
extern int	rfsr_rawread();
extern int	rfsr_cookedread();
extern int	rfsr_rawwrite();
extern int	rfsr_cookedwrite();
extern int	rfsr_rdwrinit();
extern void	rfsr_rcvmsg();
extern void	rfsr_rmactive();
extern void	rfsr_addmsg();
extern void	rfsr_rmmsg();
extern int	rfsr_lookupname();
extern int	rfsr_lookuppn();
extern int	rfsr_gift_setup();
extern int	rfsr_gift_setup();
extern int	rfsr_copyflock();
extern int	rfsr_vattr_map();
extern int	rcopyin();
extern int	rcopyout();
extern int	remio();
extern int	unremio();
extern int	rfubyte();
extern int	rfuword();
extern int	rsubyte();
extern int	rsuword();
extern mblk_t	*rfsr_rpalloc();
extern int	rfsr_lastdirents();
extern int	rfsr_discon();
extern int	rfsr_j_accuse();
extern int	rfm_addmap();
extern int	rfm_check();
extern int	rfm_delmap();
extern int	rfm_empty();
extern int	rfm_lock();
extern void	rfm_unlock();

extern void	rf_serve();		/* main program of RFS server */

extern rcvd_t	*rfsr_msgs;		/* rds waiting for servers */
extern int	rfsr_nmsg;		/* defined by lboot for accounting */

/*
 * Servers neither on the active or idle lists are those looking
 * for work, but not sleeping.  We keep such servers off the active
 * list so they won't have signals posted against them erroneously,
 * and take them off the idle list at interrupt level when a message
 * comes in, to avoid overestimating the number of idle servers due
 * to rapidly arriving messages.  If the server were just awakened,
 * and left to take itself off the idle list, several arrivals could
 * see the server as idle.
 */
extern struct proc *rfsr_idle_procp;	/* servers waiting for work */
extern int rfsr_nidle;

extern struct proc *rfsr_active_procp;
extern int rfsr_nactive;

extern int rfsr_nservers;		/* total number of servers */

#ifdef DEBUG
extern int	rfsr_idle_lock;
extern int	rfsr_active_lock;
extern int	rfsr_msg_lock;
#endif

#endif /* _FS_RFS_SERVE_H */
