/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_serve.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/vnode.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/list.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/idtab.h"
#include "sys/debug.h"
#include "sys/mode.h"
#include "sys/rf_adv.h"
#include "rf_serve.h"
#include "sys/file.h"
#include "sys/acct.h"
#include "sys/systm.h"
#include "sys/kmem.h"
#include "vm/seg_kmem.h"
#include "rf_auth.h"

/* imports */
extern void	setsigact();
extern int	setjmp();

extern rcvd_t	*mountrd;
extern rcvd_t	*sigrd;

#define RF_MAX_OPCODE DUIUPDATE

#define ULIMSHIFT 9	/* old servers express write limit in blocks;
			 * we use bytes */

rcvd_t	*rfsr_msgs;		/* list of rds waiting to be serviced */
proc_t	*rfsr_idle_procp;
proc_t	*rfsr_active_procp;	/* pointer to active servers */
int	rfsr_nactive;

STATIC void	rfsr_ret();

/*
 * kernel daemon process that handles requests for file activity
 * from remote unix systems.
 */
void
rf_serve()
{
	register proc_t	*p = u.u_procp;
	register int	sig;
	register vnode_t	*chanvp = NULL;
	rfsr_state_t	state;

	++rfsr_nservers;
	p->p_cstime = p->p_stime = p->p_cutime = p->p_utime = 0;
	u.u_start = hrestime.tv_sec;
	u.u_ticks = lbolt;
	u.u_acflag = AFORK;

	/*
	 * NOTE:  we update u.u_cred *ONLY* because parts of the kernel
	 * continue to look there for credentials, even though they
	 * should not.
	 */

	state.sr_cred = crdup(u.u_cred);
	crfree(u.u_cred);
	u.u_cred = state.sr_cred;
	crhold(u.u_cred);

	/*
	 * rf_daemon may have issued a SIGKILL, but that would be discarded
	 * by any server still in newproc() since the parent (rf_daemon)
	 * ignores all signals.	 Exit if RFS is stopping.
	 */

	if (rf_daemon_flag & RFDKILL) {
		rfsr_exit(&state);
	}

	/*
	 * SIGKILL is posted against servers by rf_daemon iff RFS is shutting
	 * down;  this happens only after all links are down, so that a
	 * SIGKILL won't slip through to a client.
	 *
	 * SIGTERM is posted by rf_rec_cleanup from fumount/rf_recovery, to wake
	 * servers in the resource that is going away.  It is also
	 * overloaded as the server side instance of any signal posted
	 * by a client.
	 *
	 * When various pieces of RFS detect impending resource
	 * exhaustion, SIGUSR1 is posted against a server to make it die
	 * and surrender scarce resources.  Note that this implicitly
	 * assumes that it is okay to break a system call when resources
	 * are low.  The SIGUSR1/EINTR combination is later turned into
	 * a NACK message with an ENOMEM error (historical).
	 *
         * (All other signals are ignored; this is inherited from rf_daemon.)
	 */

	setsigact(SIGTERM, SIG_DFL, 0, 0);
	setsigact(SIGKILL, SIG_DFL, 0, 0);
	setsigact(SIGUSR1, SIG_DFL, 0, 0);
	if (sndd_create(TRUE, &u.u_srchan) != 0) {
		if(rfsr_active_procp) {
			/*
			 * make sure the last server fails to sleep
			 */
			cmn_err(CE_NOTE,
			  "rf_server: send descriptor pre-allocation failed\n");
			psignal(rfsr_active_procp, SIGUSR1);
		} else {
			cmn_err(CE_WARN,
			  "1ST rf_server: send descriptor pre-alloc failed\n");
		}
		u.u_srchan = NULL;
		rfsr_exit(&state);
	}
	bcopy("rf_server", u.u_comm, sizeof("rf_server"));
	state.sr_in_bp = NULL;
	u.u_syscall = 0;
	for (;;) {
		register sndd_t *srchan = u.u_srchan;
		int		error;
		rfsr_ctrl_t	rfsr_ctrl;
		rf_message_t	*msg;
		rf_request_t	*req;
		rf_common_t	*cop;

		bcopy("rf_server", u.u_psargs, sizeof("rf_server"));
		srchan->sd_queue = NULL;
		srchan->sd_stat = (SDUSED | SDSERVE);
		srchan->sd_srvproc =  NULL;

		/*
		 * Before trying to pick up more work, make sure to turn off
		 * signals except for SIGKILL, which could have been sent by the
		 * rf_daemon.
		 */

		sig = 0;
		if (sigismember(&p->p_sig, SIGKILL)) {
			sig = SIGKILL;
		}
		sigemptyset(&p->p_sig);
		sigdelq(p, 0);
		if (sig) {
			sigaddset(&p->p_sig, SIGKILL);
		}
		if (p->p_cursig && p->p_cursig != SIGKILL) {
			p->p_cursig = 0;
			if (p->p_curinfo) {
				kmem_free((caddr_t)p->p_curinfo,
				  sizeof(*p->p_curinfo));
			}
		}
		ASSERT(!state.sr_in_bp);
		/*
		 * rfsr_rcvmsg won't return if too many servers, or
		 * no messages, or signal.
		 */
		rfsr_rcvmsg(&state.sr_in_bp, srchan, &state);
newmblk:
		bcopy ("RF_SERVER", u.u_psargs, sizeof("RF_SERVER"));
		msg = RF_MSG(state.sr_in_bp);
		req = RF_REQ(state.sr_in_bp);
		cop = RF_COM(state.sr_in_bp);
		req->rq_ulimit <<= ULIMSHIFT;
		state.sr_gift = NULL;
 		if (!(state.sr_rdp =
		  rf_gifttord(&msg->m_dest, state.sr_vcver))) {
			rfd_stray(state.sr_in_bp);
			state.sr_in_bp = NULL;
			rfsr_rmactive(p);
			continue;
		}

		ASSERT(state.sr_rdp->rd_qtype == RDSPECIFIC ||
		  state.sr_rdp == mountrd || state.sr_rdp == sigrd ||
		  state.sr_rdp->rd_vp);
		ASSERT(!chanvp);

		chanvp = state.sr_rdp->rd_vp;
		if (chanvp) {
			VN_HOLD(chanvp);
		}

		state.sr_qp = (queue_t *)msg->m_queue;
		state.sr_gdpp = QPTOGP(state.sr_qp);
		state.sr_vcver = state.sr_gdpp->version;
		state.sr_opcode = cop->co_opcode;
		state.sr_ret_val = 0;
		state.sr_out_bp = NULL;
		strcpy(state.sr_client, state.sr_gdpp->token.t_uname);
		error = 0;
		rfsr_ctrl = SR_NORMAL;
		/*
		 * gag - still needed, but only for sanity
		 * check in rcopyin/rcopyout
		 */
		u.u_syscall = state.sr_opcode;
		p->p_epid = cop->co_pid;
		/*
		 * Set sysid to index of stream where message came from.
		 * In case of SRMOUNT, sysid won't be set up until
		 * rfsr_mount().
		 */
		p->p_sysid = state.sr_gdpp->sysid;
		state.sr_cred->cr_uid = state.sr_cred->cr_ruid =
			gluid(state.sr_gdpp, cop->co_uid);
		state.sr_cred->cr_gid = state.sr_cred->cr_rgid =
			glgid(state.sr_gdpp, cop->co_gid);
		if (state.sr_vcver > RFS1DOT0) {
			ushort gn;

			rfsr_adj_timeskew(state.sr_gdpp,
			  req->rq_sec, req->rq_nsec);
			state.sr_cred->cr_ngroups =
			    (ushort)MIN(state.sr_gdpp->ngroups_max,
			    		req->rq_ngroups);
			for (gn = 0; gn < state.sr_cred->cr_ngroups; gn++) {
				state.sr_cred->cr_groups[gn] =
				    glgid(state.sr_gdpp, req->rq_groups[gn]);
			}
		} else {
			state.sr_cred->cr_ngroups = 0;
		}
		srchan->sd_srvproc = p;
		/* keep number of servers w/in bounds */
		if (rfsr_nactive >= maxserve - 1) {
			psignal(p, SIGUSR1);
		} else if (rfsr_nidle == 0 && rfsr_nservers < maxserve) {
			rf_daemon_flag |= RFDSERVE;
			wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);
		}
		if (msg->m_stat & RF_SIGNAL) {
			/*
			 * let remote signals occur in sys call context
			 * to preserve semantics
			 */
			psignal(p, SIGTERM);
		}
		if (state.sr_opcode < 0 || state.sr_opcode > RF_MAX_OPCODE) {
			if (chanvp) {
				VN_RELE(chanvp);
				chanvp = NULL;
			}
			(void)rfsr_undef_op(&state, &rfsr_ctrl);
			rfsr_rmactive(p);
			continue;
		}
		/*
		 * fail if resource is in funny state - e.g., in fumount
		 */
		if (state.sr_opcode != RFMOUNT && state.sr_opcode != RFUSTAT) {
			/* See if resource is fumounted.  Note that this
			 * is ONLY PROBABLISTIC with respect to forgeries
			 */
			if ((state.sr_rsrcp = ind_to_rsc(srchan->sd_mntid))
			    == NULL ||
			  (state.sr_srmp = id_to_srm(state.sr_rsrcp,
			    state.sr_gdpp->sysid)) == NULL ||
			  state.sr_srmp->srm_flags & SRM_FUMOUNT){
				if (chanvp) {
					VN_RELE(chanvp);
					chanvp = NULL;
				}
				SR_FREEMSG(&state);
				if (state.sr_opcode != RFCLOSE) {
					error = EACCES;
				}
				rfsr_rmactive(p);
                                if (state.sr_opcode != RFRSIGNAL &&
				  (state.sr_vcver < RFS2DOT0 ||
				  state.sr_opcode != RFINACTIVE)) {
                                        rfsr_ret(&state, SR_NORMAL, error);
                                }
				continue;
			}
		}
		/*
		 * This setjmp might be superfluous, but we can't
		 * look at the sysent table; not all ops are syscalls.
		 */
		if (setjmp(&u.u_qsav)) {
			error = rfsr_sigck(&rfsr_ctrl, EINTR);
                        if (state.sr_vcver > RFS1DOT0 &&
			  state.sr_opcode == RFINACTIVE) {
				rf_deliver(state.sr_in_bp);
				state.sr_in_bp = NULL;
			} else {
				rfsr_ret(&state, rfsr_ctrl, error);
			}
			if (chanvp) {
				VN_RELE(chanvp);
				chanvp = NULL;
			}
			rfsr_rmactive(p);
		} else {
			error =
			  (*rfsr_ops[state.sr_opcode])(&state, &rfsr_ctrl);
			switch(rfsr_ctrl){
			case SR_NORMAL:
				if (!error) {
					if (!state.sr_out_bp) {
						state.sr_out_bp =
						  rfsr_rpalloc((size_t)0,
						  state.sr_vcver);
					}
					if (!state.sr_gift && state.sr_out_bp) {
						error = rfsr_cacheck(&state,
						  srchan->sd_mntid);
					}
				}
				/* FALLTHROUGH */
			case SR_PATH_RESP:
			case SR_NACK_RESP:
				error = rfsr_sigck(&rfsr_ctrl, error);
				rfsr_rmactive(p);
				rfsr_ret(&state, rfsr_ctrl, error);
				if (chanvp) {
					VN_RELE(chanvp);
					chanvp = NULL;
				}
				continue;
			case SR_NO_RESP:
				rfsr_rmactive(p);
				if (state.sr_out_bp) {
					rf_freemsg(state.sr_out_bp);
					state.sr_out_bp = NULL;
				}
				if (chanvp) {
					VN_RELE(chanvp);
					chanvp = NULL;
				}
				continue;
			case SR_OUT_OF_BAND:
				if (chanvp) {
					VN_RELE(chanvp);
					chanvp = NULL;
				}
				goto newmblk;
			default:
				cmn_err(CE_PANIC,
				    "Undefined rfsr_ops return value\n");
			}
		}
	}
}

STATIC void
rfsr_ret(stp, rfsr_ctrl, error)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t rfsr_ctrl;
	register int error;
{
	register rf_response_t *rsp;
	register rf_common_t	*cop;
	register proc_t		*p = u.u_procp;
	sndd_t *srchan = u.u_srchan;

	if (stp->sr_in_bp) {
		SR_FREEMSG(stp);
	}
	/*
	 * set up a response message if that isn't already done.
	 */
	if (!stp->sr_out_bp) {
		stp->sr_out_bp = rfsr_rpalloc((size_t)0, stp->sr_vcver);
	}
	rsp = RF_RESP(stp->sr_out_bp);
	cop = RF_COM(stp->sr_out_bp);
	rsp->rp_rval = stp->sr_ret_val;
	rsp->rp_errno = error;
	cop->co_sysid = p->p_sysid;
	if (rfsr_ctrl == SR_NACK_RESP) {
		cop->co_type = RF_NACK_MSG;
	}
	if (rfsr_ctrl != SR_PATH_RESP) {
		cop->co_opcode = stp->sr_opcode;
		cop->co_mntid = srchan->sd_mntid;
	}
	/*
	 * Return signals except those used by the server internally,
	 * which have already been cleared by rmactive and rfsr_sigck.
	 * This may mask some signals intended for the client, but
	 * can't be avoided, because the server overloads signals
	 * for IPC.
	 */

	if (p->p_cursig) {
		sigaddset(&p->p_sig, p->p_cursig);
		p->p_cursig = 0;
		if (p->p_curinfo) {
			kmem_free((caddr_t)p->p_curinfo, sizeof(*p->p_curinfo));
			p->p_curinfo = NULL;
		}
	}
	rf_setrpsigs(rsp, stp->sr_vcver, &p->p_sig);
	if (rf_sndmsg(srchan, stp->sr_out_bp, RF_MIN_RESP(stp->sr_vcver) +
	    (int)rsp->rp_count, stp->sr_gift, FALSE) == ENOLINK) {
		/*
		 * The circuit back to the client is down.
		 * Reduce the count of servers working for the
		 * client, and wakeup rf_recovery if this is the last
		 * such server.
		 */
		if (stp->sr_srmp) {
			stp->sr_srmp->srm_slpcnt--;
			if (!stp->sr_srmp->srm_slpcnt) {
				wakeprocs((caddr_t)stp->sr_srmp, PRMPT);
			}
		}
	}
	stp->sr_out_bp = NULL;
}
