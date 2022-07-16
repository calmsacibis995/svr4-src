/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_sys.c	1.3.1.1"
/*
 *  System calls for remote file sharing.
 */
#include "sys/list.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/rf_debug.h"
#include "sys/debug.h"
#include "sys/rf_adv.h"
#include "sys/rf_sys.h"
#include "sys/inline.h"
#include "sys/idtab.h"
#include "sys/hetero.h"
#include "sys/cmn_err.h"
#include "sys/systm.h"
#include "sys/fs/rf_vfs.h"
#include "sys/var.h"
#include "sys/fcntl.h"
#include "sys/uio.h"
#include "sys/stat.h"
#include "sys/statvfs.h"
#include "sys/statfs.h"
#include "rf_auth.h"
#include "sys/dirent.h"
#include "sys/cred.h"
#include "sys/buf.h"
#include "vm/page.h"
#include "rf_cache.h"
#include "sys/file.h"
#include "rfcl_subr.h"
#include "sys/pathname.h"
#include "sys/kmem.h"
#include "rf_serve.h"

char rfs_domain[MAXDNAME+1] = "";	/* domain name for this machine	*/
STATIC int rf_vflg = 0;			/* require (1) host verification
					 * or not (0) */
STATIC int	rfud_lastumsg;

STATIC int	rfs_vhigh = RFS2DOT0;
STATIC int	rfs_vlow = RFS1DOT0;

extern int	rfs_vhigh;
extern int	rfs_vlow;
extern int	rf_state;
extern int	nsrmount;
extern ls_elt_t	rf_umsgq;		/* queue for user-level daemon */
extern int	rf_umsgcnt;		/* length of rf_umsgq */
extern int	nsndd;

/* imports */
extern int	dofusers();
extern int	dounmount();
extern int	strprefix();

STATIC int	rfsys_fumount();
STATIC int	rfsys_sendumsg();
STATIC int	rfsys_getumsg();
STATIC int	rfsys_lastumsg();
STATIC int	rfsys_fwfd();
STATIC int	rfsys_setdname();
STATIC int	rfsys_getdname();
STATIC int	rfsys_setidmap();
STATIC int	rfsys_vflag();
STATIC int	rfsys_version();
STATIC int	rfsys_runstate();
STATIC int	rfsys_tuneable();
STATIC int	rfsys_clients();
STATIC int	rfsys_resources();
STATIC int	rfsys_advfs();
STATIC int	rfsys_unadvfs();
STATIC int	rfsys_start();
STATIC int	rfsys_stop();
STATIC int	rfsys_debug();

STATIC void	rf_fuserve();
#ifndef DEBUG	/* allows RFS to build without DEBUG */
extern long	dudebug;
#endif

/*
 * nadvertise is the number of resources currently advertised.
 */
STATIC uint	nadvertise;

int
rfsys(uap, rvp)
	caddr_t	uap;
	rval_t	*rvp;
{
	/*
	 * rfsys is invoked with at least one argument, an integer  opcode
	 * that distinguishes the action to perform.  Most of these opcodes
	 * have further arguments, but the opcode is the first structure
	 * member, and can therefore be used to discriminate all operations.
	 */
	switch (((int *)uap)[0]) {
	case RF_FUMOUNT:
		return rfsys_fumount(uap, rvp);
	case RF_SENDUMSG:
		return rfsys_sendumsg(uap, rvp);
	case RF_GETUMSG:
		return rfsys_getumsg(uap, rvp);
	case RF_LASTUMSG:
		return rfsys_lastumsg(uap, rvp);
	case RF_FWFD:
		return rfsys_fwfd(uap, rvp);
	case RF_SETDNAME:
		return rfsys_setdname(uap, rvp);
	case RF_GETDNAME:
		return rfsys_getdname(uap, rvp);
	case RF_SETIDMAP:
		return rfsys_setidmap(uap, rvp);
	case RF_VFLAG:
		return rfsys_vflag(uap, rvp);
	case RF_VERSION:
		return rfsys_version(uap, rvp);
	case RF_RUNSTATE:
		return rfsys_runstate(uap, rvp);
	case RF_TUNEABLE:
		return rfsys_tuneable(uap, rvp);
	case RF_CLIENTS:
		return rfsys_clients(uap, rvp);
	case RF_RESOURCES:
		return rfsys_resources(uap, rvp);
	case RF_ADVFS:
		return rfsys_advfs(uap, rvp);
	case RF_UNADVFS:
		return rfsys_unadvfs(uap, rvp);
	case RF_START:
		return rfsys_start(uap, rvp);
	case RF_STOP:
		return rfsys_stop(uap, rvp);
	case RF_DEBUG:
		return rfsys_debug(uap, rvp);
	default:
		return EINVAL;
	}
}

/*
 * Notify each client machine using the named resource that it is being
 * fumounted and wait for their reply.
 *
 * Return nonzero errno if any such exchange fails, but try
 * all of them while we can get communications data structures.
 *
 * We don't use rfcl* because the rf_recovery rd on the other side
 * is a specific rd, therefore not subject to flow control.
 * Then call rf_recovery to pretend the client gave up all
 * references to the rf_rsrc.
 */

typedef struct rfsys_fumounta {
	int opcode;
	char *resource;
} rfsys_fumounta_t;

/* ARGSUSED */
STATIC int
rfsys_fumount(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_fumounta_t	*fmp = (rfsys_fumounta_t *)uap;
	rf_resource_t		*rscp;
	char			name[RFS_NMSZ];
	sr_mount_t		*srp;		/* sr_mount cursor */
	sndd_t			*sdp = NULL;	/* channel to client */
	rcvd_t			*rd = NULL;	/* to catch client response */
	int			error = 0;	/* result */

	if (!suser(u.u_cred)) {
		error = EPERM;
		goto out;
	}
	if (rf_state != RF_UP)  {
		error = ENONET;
		goto out;
	}
	if (copyin(fmp->resource, name, RFS_NMSZ)) {
		DUPRINT1(DB_MNT_ADV, "rfsys_fumount copyin failed\n");
		error = EFAULT;
		goto out;
	}
	if ((rscp = name_to_rsc(name)) == NULL) {
		DUPRINT1(DB_MNT_ADV, "rfsys_fumount name_to_rsc failed\n");
		error = ENOENT;
		goto out;
	}
	if (!(rscp->r_flags & R_UNADV)) {
		DUPRINT1(DB_MNT_ADV, "rfsys_fumount: resource is advertised\n");
		error = EADV;
		goto out;
	}
	if (rscp->r_flags & R_FUMOUNT) {
		DUPRINT1(DB_MNT_ADV, "rfsys_fumount:  already fumounted\n");
		error = EINVAL;
		goto out;
	}
	rscp->r_flags |= R_FUMOUNT;
	DUPRINT2(DB_MNT_ADV, "rfsys_fumount: vnode %x\n", rscp->r_rootvp);
	(void)sndd_create(FALSE, &sdp);
	(void)rcvd_create(FALSE, RDSPECIFIC, &rd);
	rd->rd_sdp = sdp;
	srp = rscp->r_mountp;
	while (srp) {
		sr_mount_t		*nextsrp = srp->srm_nextp;
		queue_t			*client = gdp_sysidtoq(srp->srm_sysid);
		mblk_t			*bp;
		register rf_common_t	*cop;
		int			tmperr;	/* avoid overwriting error */
		mblk_t			*in_bp;
		size_t			bufsize =
					  RF_MIN_REQ(QPTOGP(client)->version);

		ASSERT(client);
		sndd_set(sdp, client, &rf_daemon_gift);
		rf_signal_serve(rscp->r_mntid, srp);
		(void)rf_allocmsg(bufsize, (size_t)0, BPRI_MED, FALSE,
		  NULLCADDR, NULLFRP, &bp);
		ASSERT(bp);
		cop = RF_COM(bp);
		RF_REQ(bp)->rq_rec_fumount.srmntid = rscp->r_mntid;
		cop->co_opcode = REC_FUMOUNT;
		cop->co_type = RF_REQ_MSG;
		cop->co_mntid = srp->srm_mntid;
		if ((tmperr = rf_sndmsg(sdp, bp, bufsize, rd, FALSE)) == 0
		  && (tmperr = rf_rcvmsg(rd, &in_bp)) == 0) {
			rf_fuserve(rscp->r_mntid);
			srp->srm_flags |= SRM_FUMOUNT;
			wakeprocs((caddr_t)client->q_ptr, PRMPT);
			rf_freemsg(in_bp);
			in_bp = NULL;
		} else {
			error = tmperr;
		}
		srp = nextsrp;
	}
	rf_recovery_flag |= RFRECFUMOUNT;
	wakeprocs((caddr_t)&rf_recovery_procp, PRMPT);
out:
	sndd_free(&sdp);
	rcvd_free(&rd);
	return error;
}

/*
 * Mark the RDs of server processes sleeping in fumounted resource,
 * and wake up the servers.
 */
STATIC void
rf_fuserve(srm_mntid)
	long		srm_mntid;
{
	register rcvd_t	*rd;
	register rcvd_t	*endrcvd = rcvd + nrcvd;

	for (rd = rcvd; rd < endrcvd; rd++)  {
		if (ACTIVE_SRD(rd) && rd->rd_vp
		    && (((sndd_t *)(rd->rd_vp))->sd_stat & SDSERVE)
		    && ((sndd_t *)(rd->rd_vp))->sd_mntid == srm_mntid) {
			DUPRINT1(DB_MNT_ADV, "rfsys_fumount: waking server\n");
			rd->rd_stat |= RDLINKDOWN;
			rf_checkq(rd, srm_mntid);
			wakeprocs((caddr_t)&rd->rd_qslp, PRMPT);
		}
	}
}

/*
 * Construct and send message to remote user-level, and wait for reply.
 */

typedef struct rfsys_sendumsga {
        int     opcode;
        int     cl_sysid;       /* destination sysid */
        char    *bufp;          /* message to send */
        uint    size;           /* strlen(bufp) */
} rfsys_sendumsga_t;

/* ARGSUSED */
STATIC int
rfsys_sendumsg(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_sendumsga_t	*smp = (rfsys_sendumsga_t *)uap;
	queue_t			*cl_queue;
	sndd_t			*chansdp;
	register int		error;
	mblk_t			*bp;
	rcvd_t			*rdp;
	register int		vcver;
	register size_t		datasz = smp->size;
	register size_t		rqsz;
	register size_t		totalsz;
	int			sink;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (datasz > ULINESIZ) {
		return EINVAL;
	}
	if (rf_state != RF_UP)  {
		return ENONET;
	}
	if ((cl_queue = gdp_sysidtoq((sysid_t)smp->cl_sysid)) == NULL) {
		return ECOMM;
	}
	(void)sndd_create(FALSE, &chansdp);
	sndd_set(chansdp, cl_queue, &rf_daemon_gift);
	vcver = QPTOGP(chansdp->sd_queue)->version;
	rqsz = RF_MIN_REQ(vcver);
	totalsz = rqsz + datasz;
	(void)rcvd_create(FALSE, RDSPECIFIC, &rdp);
	rdp->rd_sdp = chansdp;
	if ((error = rf_allocmsg(rqsz, datasz, BPRI_LO, TRUE, NULLCADDR,
	  NULLFRP, &bp)) == 0) {
		rfcl_reqsetup(bp, chansdp, u.u_cred, REC_MSG, R_ULIMIT);
		RF_REQ(bp)->rq_rec_msg.count = datasz;
		if (copyin((caddr_t)smp->bufp, rf_msgdata(bp, rqsz),
		  datasz) < 0) {
			error = EFAULT;
		}
	}
	if (!error &&
	  (error = rfcl_xac(&bp, totalsz, rdp, vcver, FALSE, &sink)) == 0) {
		error = RF_RESP(bp)->rp_errno;
		rf_freemsg(bp);
	}
	rcvd_free(&rdp);
	sndd_free(&chansdp);
	return error;
}

/*
 * User-level daemon sleeps here, waiting for something to happen.
 */

typedef struct rfsys_getumsga {
	int 		opcode;
	char		*buf;
	unsigned	size;
} rfsys_getumsga_t;

STATIC int
rfsys_getumsg(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_getumsga_t	*gmp = (rfsys_getumsga_t *)uap;
	mblk_t			*bp;
	struct u_d_msg		*udm;
	int			size;
	int			error = 0;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (rf_state != RF_UP)  {
		rvp->r_val1 = RFUD_LASTUMSG;
		return 0;
	}
	while ((bp = (mblk_t *)LS_REMQUE(&rf_umsgq)) == NULL) {
		if (sleep((caddr_t)&rf_daemon_lock, PZERO + 1 | PCATCH)) {
			while((bp = (mblk_t *)LS_REMQUE(&rf_umsgq)) != NULL) {
				rf_freemsg(bp);
			}
			rfud_lastumsg = 0;
			return EINTR;
		} else if (rfud_lastumsg) {
			while((bp = (mblk_t *)LS_REMQUE(&rf_umsgq)) != NULL) {
				rf_freemsg(bp);
			}
			rfud_lastumsg = 0;
			rvp->r_val1 = RFUD_LASTUMSG;
			return 0;
		}
	}
	udm = (struct u_d_msg *)(bp->b_wptr);
	size = (gmp->size > udm->count) ? udm->count : gmp->size;
	rvp->r_val1 = udm->opcode;
	switch (rvp->r_val1) {
	case RFUD_DISCONN:	/* link is down */
	case RFUD_FUMOUNT:	/* forced unmount */
	case RFUD_GETUMSG: 	/* message for user level from remote */
		if (copyout(udm->res_name, gmp->buf, size) < 0) {
			error = EFAULT;
		}
		break;
	default:
		error = EINVAL;
		break;
	}
	--rf_umsgcnt;
	rf_freemsg(bp);
	return error;
}

/*
 * wakeup rfsys_getumsg
 */
/* ARGSUSED */
STATIC int
rfsys_lastumsg(uap, rvp)
	caddr_t	uap;
	rval_t	*rvp;
{
	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (rf_state != RF_UP)  {
		return ENONET;
	}
	rfud_lastumsg = 1;
	wakeprocs((caddr_t)&rf_daemon_lock, PRMPT);
	return 0;
}

/*
 * force working file descriptor
 */

typedef struct rfsys_fwfda {
	unsigned	opcode;
	int		fd;
	rf_token_t	*token;
	gdpmisc_t	*gdpmisc;
} rfsys_fwfda_t;

/* ARGSUSED */
STATIC int
rfsys_fwfd(uap, rvp)
	caddr_t		uap;
	rval_t		*rvp;
{
	rfsys_fwfda_t	*fwp = (rfsys_fwfda_t *)uap;
	queue_t		*qp;
	rf_token_t	name;
	gdpmisc_t	gdpmisc;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (rf_state != RF_UP) {
		return ENONET;
	}
	if (fwp->fd < 0) {
		return EINVAL;
	}
	if (copyin((caddr_t)fwp->token, (caddr_t)&name, sizeof(rf_token_t))) {
		DUPRINT1(DB_MNT_ADV, "fwfd token copyin failed\n");
		return EFAULT;
	}
	DUPRINT3(DB_MNT_ADV, "fwfd: token.t_id=%x, token.t_uname=%s\n",
	    name.t_id, name.t_uname);
	if (copyin((caddr_t)fwp->gdpmisc, (caddr_t)&gdpmisc, 
	  sizeof(gdpmisc_t))) {
		DUPRINT1(DB_MNT_ADV, "fwfd gdpmisc copyin failed\n");
		return EFAULT;
	}
	return gdp_get_circuit(fwp->fd, &name, &qp, &gdpmisc);
}

/*
 * Set domain name.
 */

typedef struct rfsys_setdnamea {
	int	opcode;
	char	*dname;
	int	size;
} rfsys_setdnamea_t;

/* ARGSUSED */
STATIC int
rfsys_setdname(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_setdnamea_t	*sdap = (rfsys_setdnamea_t *)uap;
	int			error;
	uint			c_count;	/* copyinstr out parameter */

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (rf_state != RF_DOWN) {
		return EEXIST;
	}
	if (sdap->size > MAXDNAME) {
		return EINVAL;
	}
	error = copyinstr(sdap->dname, rfs_domain, sdap->size, &c_count);
	if (!error && c_count == 1) {
		/*
		 * No fault, but a NULL domain name which isn't allowed.
		 */
		error = EINVAL;
	}
	return error;
}

/*
 * Get domain name.
 */

typedef struct rfsys_getdnamea {
	int	opcode;
	char	*dname;
	int	size;
} rfsys_getdnamea_t;

/* ARGSUSED */
STATIC int
rfsys_getdname(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_getdnamea_t	*getd_ap = (rfsys_getdnamea_t *)uap;
	int			error = 0;

	if (getd_ap->size > MAXDNAME || getd_ap->size <= 0) {
		return EINVAL;
	}
	if (*rfs_domain == '\0') {
		return ENOENT;
	}
	if (copyout(rfs_domain, getd_ap->dname, getd_ap->size)) {
		error = EFAULT;
	}
	return error;
}

/*
 * Add an id map.
 */

typedef struct rfsys_setidmapa {
	int		opcode;
	char		*name;
	int		flag;
	struct idtab	*map;
} rfsys_setidmapa_t;

/* ARGSUSED */
STATIC int
rfsys_setidmap(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_setidmapa_t	*sap = (rfsys_setidmapa_t *)uap;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	return rf_setidmap(sap->name, sap->flag, sap->map, u.u_cred);
}

/*
 * Handle verification option.
 */

typedef struct rfsys_vflaga {
	int	opcode;
	int	vcode;
} rfsys_vflaga_t;

STATIC int
rfsys_vflag(uap, rvp)
	caddr_t		uap;
	rval_t		*rvp;
{
	rfsys_vflaga_t	*vfap = (rfsys_vflaga_t *)uap;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	switch (vfap->vcode) {
	case V_SET:
		rf_vflg = 1;
		return 0;
	case V_CLEAR:
		rf_vflg = 0;
		return 0;
	case V_GET:
		rvp->r_val1 = rf_vflg;
		return 0;
	default:
		return EINVAL;
	}
}

/*
 * Handle version information.
 */

typedef struct rfsys_versiona {
	int	opcode;
	int	vcode;
	int	*vhigh;
	int	*vlow;
} rfsys_versiona_t;

STATIC int
rfsys_version(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_versiona_t	*verp = (rfsys_versiona_t *)uap;
	int			uvhigh;
	int			uvlow;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	switch (verp->vcode) {
	case VER_CHECK:
		if (copyin((caddr_t)verp->vhigh, (caddr_t)&uvhigh,
		  sizeof(int))|| copyin((caddr_t)verp->vlow, 
		  (caddr_t)&uvlow, sizeof(int))) {
			return EFAULT;
		} else if (uvhigh < uvlow) {
			return EINVAL;
		} else if (uvhigh < rfs_vlow || uvlow > rfs_vhigh) {
			return ERANGE;
		} else {
			rvp->r_val1 = (uvhigh < rfs_vhigh) ?
			    uvhigh : rfs_vhigh;
			return 0;
		}
		/* NOT REACHED */
	case VER_GET:
		if (copyout((caddr_t)&rfs_vhigh, (caddr_t)verp->vhigh,
		  sizeof(int)) || copyout((caddr_t)&rfs_vlow,
		  (caddr_t)verp->vlow, sizeof(int))) {
			return EFAULT;
		} else {
			return 0;
		}
		/* NOT REACHED */
	default:
		return EINVAL;
	}
}

/*
 * Return the current runstate of RFS.
 */
/* ARGSUSED */
STATIC int
rfsys_runstate(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rvp->r_val1 = rf_state;
	return 0;
}

/*
 * Return the value of the requested tuneable system parameter.
 */
typedef struct rfsys_tuneablea {
	int	opcode;		/* RF_TUNEABLE */
	int	param;		/* code for which param */
} rfsys_tuneablea_t;

STATIC int
rfsys_tuneable(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	switch (((rfsys_tuneablea_t *)uap)->param) {
	case T_NSRMOUNT:
		rvp->r_val1 = nsrmount;
		return 0;
		/* NOT REACHED */
	case T_NADVERTISE:
		rvp->r_val1 = nadvertise;
		return 0;
		/* NOT REACHED */
	default:
		return EINVAL;
	}
}

/*
 * Fill a user supplied buffer with information about the remote use
 * of a locally advertised resource.  For each mount of the resource,
 * fill a structure with the client machine name, (with domain prepended),
 * the sysid, and the corresponding bcount of the number of blocks
 * of data read or written by that machine.
 *
 * Returns 0 for success, a non-zero errno for failure, and in
 * rvp->r_val1, returns a count of the number of client structures
 * filled in, undefined for failure.
 */

typedef struct rfsys_clientsa {
	int		opcode;
	char		*rscnmp;
	struct client	*clbufp;
} rfsys_clientsa_t;

STATIC int
rfsys_clients(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_clientsa_t	*clap = (rfsys_clientsa_t *)uap;
	register unsigned	clcount = 0;
	register gdp_t		*gdpp;
	register gdp_t		*gdpend = gdp + maxgdp;
	rf_resource_t		*rsrcp;
	register sr_mount_t	*srmp;
	register struct client	*cbp = clap->clbufp;
	uint			cpin_count;
	int			error;
	struct client		client;
	char			rsrc_name[RFS_NMSZ];

	if (rf_state != RF_UP)  {
		return ENONET;
	}
	if (error = copyinstr(clap->rscnmp, rsrc_name, RFS_NMSZ, &cpin_count)) {
		return error;
	}
	if ((rsrcp = (name_to_rsc(rsrc_name))) == NULL) {
		return ENODEV;
	}
	srmp = rsrcp->r_mountp;
	while (srmp) {
		register sysid_t	this_sysid = srmp->srm_sysid;

		client.cl_sysid = this_sysid;
		client.cl_bcount = srmp->srm_kbcnt;
		/* find uname for this sr_mount */
		for (gdpp = gdp; gdpp->sysid != this_sysid; gdpp++) {
			/*
			 * The following could occur if interleaved
			 * with rf_recovery.
			 */
			if (gdpp == gdpend) {
				return EAGAIN;
			}
		}
		strcpy(client.cl_node, gdpp->token.t_uname);
		if (copyout((caddr_t)&client, (caddr_t)cbp,
		  sizeof(struct client))) {
			return EFAULT;
		}
		cbp++;
		clcount++;
		srmp = srmp->srm_nextp;
	}
	rvp->r_val1 = clcount;
	return 0;
}

/*
 * Fill a user supplied buffer with the names of all local resources
 * that are advertised and/or remotely mounted, but are not in an
 * intermediate state.  Return a non-zero error for failure or a
 * non-negative count of the number of names placed in the buffer.
 */

typedef struct rfsys_resourcesa {
	int	opcode;
	char	*rsbufp;
} rfsys_resourcesa_t;

STATIC int
rfsys_resources(uap, rvp)
	caddr_t				uap;
	rval_t				*rvp;
{
	register rfsys_resourcesa_t	*rap = (rfsys_resourcesa_t *)uap;
	register unsigned		rscount = 0;
	register char			*rsbp;
	register rf_resource_t		*rsrcp = rf_resource_head.rh_nextp;
	register rf_resource_t		*endrsc =
					  (rf_resource_t *)&rf_resource_head;

	if (rf_state != RF_UP)  {
		return ENONET;
	}
	rsbp = rap->rsbufp;
	while (rsrcp != endrsc) {
		if (!(rsrcp->r_flags & R_FUMOUNT)) {
			if (copyout(rsrcp->r_name, rsbp, RFS_NMSZ)) {
				return EFAULT;
			}
			rsbp += RFS_NMSZ;
			rscount++;
		}
		rsrcp = rsrcp->r_nextp;
	}
	rvp->r_val1 = rscount;
	return 0;
}

#ifndef MIN
#define MIN(a,b)	((a) <= (b) ? (a) : (b))
#endif

/*
 * Start remote file sharing.
 */
/* ARGSUSED */
STATIC int
rfsys_start(uap, rvp)
	caddr_t		uap;
	rval_t		*rvp;
{
	register proc_t	*p;
	pid_t		childpid;	/* newproc out arg */
	int		error = 0;
	size_t		datasz = MIN(PAGESIZE, DU_DATASIZE);

	extern int	rc_time;	/* defined by boot program */

	DUPRINT1(DB_RFSTART, "rfsys_start\n");
	if (!suser(u.u_cred)) {
		return EPERM;
	}

	/*
	 * Implicit assumption made explicit.
	 * TO DO:
	 * These should be based on canonical representations.  Others???
	 */

	if (datasz < sizeof(flock_t)
	    || datasz < sizeof(o_flock_t)
	    || datasz < sizeof(rf_attr_t)
	    || datasz < sizeof(struct stat)
	    || datasz < sizeof(struct statfs)
	    || datasz < sizeof(struct statvfs)
	    || datasz < RF_MAXDIRENT) {
		cmn_err(CE_NOTE, "rfsys_start: RF_DATASIZE too small");
		return ENOSYS;
	}

	if (!sndd) {

		/* NULLed by rf_init */

		cmn_err(CE_NOTE, "rfsys_start: no memory configured");
		return ENOSYS;
	}

	while (rf_state == RF_INTER) {
		sleep((caddr_t)&rf_state, PREMOTE);
	}

	/* This is a critical section that ends in rf_daemon. */

	if (rf_state == RF_UP)  {
		DUPRINT1 (DB_RFSTART, "rfsys_start: system already booted\n");
		wakeprocs((caddr_t)&rf_state, PRMPT);
		return EEXIST;
	}

	rf_state = RF_INTER;   /* RFS in an intermediate state */
	rfud_lastumsg = 0;

	if (rf_comminit()) {
		rf_state = RF_DOWN;
		wakeprocs((caddr_t)&rf_state, PRMPT);
		return EAGAIN;  /* compatability */
	}
	DUPRINT1(DB_RFSTART, "rfsys_start comm initialized\n");

	auth_init();
	if (error = gdp_init()) {
		DUPRINT2 (DB_RFSTART, "rfsys_start gdp_init error %d\n", error);
		rf_commdinit();
		rf_state = RF_DOWN;
		wakeprocs((caddr_t)&rf_state, PRMPT);
		return error;
	}
	DUPRINT1(DB_RFSTART, "rfsys_start gdp initialized\n");

	/* enable caching */

	if (rc_time != -1) {
		rfc_time = rc_time * HZ;
	} else {
		rfc_time = -1;
	}

	/*
	 * Newproc sets child's pid in out arg; caller
	 * normally passes in &rvp->r_val1, but we don't want
	 * to return it
	 */

	u.u_procp->p_flag |= SNOWAIT;

	switch (newproc(NP_FAILOK | NP_NOLAST | NP_SYSPROC,
	  &childpid, &error))  {
	case 0:
		break;
	case 1:

		/*
                 * rf_daemon forks new processes when more daemons are
                 * required, then returns the function to execute.
		 * The call is from here so that the stack is smaller.
                 */

                (*rf_daemon())();

		/* NOTREACHED */
		break;
	default:
		cmn_err(CE_WARN, "rfsys_start: cannot fork rf_daemon\n");
		rf_commdinit();
		return EAGAIN;
	}

	/*  now allow advertise calls, set all sysid's in proc table  */

	for (p = practive; p; p = p->p_next) {
		p->p_sysid = 0;
	}

	/* initialize server "device" table */
	rfsr_dev_init();

	return error;
}

/*
 * Stop remote file sharing if everything is quiet.
 */
/* ARGSUSED */
STATIC int
rfsys_stop(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	register vfs_t		*vfsp = rootvfs;
	register rf_resource_t	*rsrcp = rf_resource_head.rh_nextp;

	DUPRINT1(DB_RFSTART, "rfsys_stop\n");
	if (!suser(u.u_cred)) {
		return EPERM;
	}

	/* Critical section as in rfsys_start. */

	while (rf_state == RF_INTER) {
		sleep((caddr_t)&rf_state, PREMOTE);
	}
	if (rf_state == RF_DOWN) {
		DUPRINT1(DB_RFSTART, "rfsys_stop: system already stopped\n");
		wakeprocs((caddr_t)&rf_state, PRMPT);
		return ENONET;
	}
	rf_state = RF_INTER;

	/* can't stop if anything is remotely mounted */

	while (vfsp) {
		if (ISRFSVFSP(vfsp)) {
			DUPRINT1 (DB_RFSTART,
			    "rfsys_stop: can't stop with remote mounts.\n");
			rf_state = RF_UP;
			wakeprocs((caddr_t)&rf_state, PRMPT);
			return EBUSY;
		}
		vfsp = vfsp->vfs_next;
	}

	while (rsrcp != (struct rf_resource*)&rf_resource_head) {
		if (rsrcp->r_mountp) {
			DUPRINT1 (DB_RFSTART,
			    "rfsys_stop: can't stop with clients.");
			rf_state = RF_UP;
			wakeprocs((caddr_t)&rf_state, PRMPT);
			return ESRMNT;
		}
		if (!(rsrcp->r_flags & R_UNADV)) {
			DUPRINT1 (DB_RFSTART,
			 "rfsys_stop: can't stop with advertised resources.\n");
			rf_state = RF_UP;
			wakeprocs((caddr_t)&rf_state, PRMPT);
			return EADV;
		}
		rsrcp = rsrcp->r_nextp;
	}

	rfud_lastumsg = 1;
	wakeprocs((caddr_t)&rf_daemon_lock, PRMPT);

	DUPRINT1(DB_RFSTART, "rfsys_stop: taking down links\n");
	gdp_kill();             /* cut all connections */

	/* kill daemons - rf_state goes to DOWN after both die */

	DUPRINT1(DB_RFSTART, "rfsys_stop: killing daemon\n");
	rf_daemon_flag |= RFDKILL;
	wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);

	DUPRINT1(DB_RFSTART, "rfsys_stop: done\n");
	return 0;
}

typedef struct rfsys_debuga {
	int	opcode;
	int	level;
} rfsys_debuga_t;

STATIC int
rfsys_debug (uap, rp)
	caddr_t		uap;
	rval_t		*rp;
{
	rfsys_debuga_t	*dbp = (rfsys_debuga_t *)uap;

	if (!suser(u.u_cred)) {
		return EPERM;
	}
	if (dbp->level !=  -2) {
		if (!dbp->level) {
			dudebug = 0;
		} else {
			dudebug |= dbp->level;
		}
	}
	rp->r_val1 = dudebug;
	return 0;
}

/*
 * Advertise a local specified by path as an RFS resource
 * named by resource.  Flag specifies whether the advertisement
 * should be read-only or not and whether this is a modification
 * of a previous advertisement.  Clistpp refers to a list of names
 * of client machines authorized to use the rf_rsrc.
 *
 * Returns 0 for success, a non-zero errno for failure.
 *
 */
typedef struct rfsys_advfsa {
	int	opcode;
	char    *path;          /* path to directory being advertised */
	char    *resource;      /* resource name given to name server */
	int     flag;           /* read-only/read-write, modify */
	char    **clistpp;      /* client list */
} rfsys_advfsa_t;

/* ARGSUSED */
STATIC int
rfsys_advfs(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	register rfsys_advfsa_t	*advp = (rfsys_advfsa_t *)uap;
	vnode_t			*vp = NULLVP;  	/* vnode of file specified */
	register rf_resource_t	*rp;
	rf_resource_t		*newrp = NULL;
	struct sr_mount		*srp;		/* cursor for srmount list */
	rcvd_t			*giftrd = NULL;	/* rcvd for resource */
	char			adv_name[RFS_NMSZ];
	char			*newclp = NULL;	/* client list pointer */
	gdp_t			*gdpp;		/* cursor to scan gdp list */
	int			error = 0;	/* return value */
	uint			c_count;	/* copyinstr out parameter */

	if (rf_state != RF_UP)  {
		return ENONET;
	}
	if (!maxserve) {
		return ENOMEM;
	}
	if (!suser(u.u_cred)) {
		return EPERM;
	}
	/* get the name to be advertised */
	if (error = copyinstr(advp->resource, adv_name, RFS_NMSZ, &c_count)) {
		return error;
	}
	if (c_count == 1) {
		return EINVAL;		/* don't allow a NULL resource name */
	}
	if (advp->clistpp &&
	  (error = rf_addalist(&newclp, advp->clistpp)) != 0) {
		return error;
	}
	/*
	 * if this is a modify request, all we need to do is replace
	 * the client list and return.
	 */
	if (advp->flag & A_MODIFY) {
		if ((rp = name_to_rsc(adv_name)) == NULL) {
			/* the specified resource name was not found */
			error = ENODEV;
			goto bad;
		}
		if (rp->r_flags & R_UNADV) {
			/* the resource has been unadvertised */
			error = ENODEV;
			goto bad;
		}
		if (newclp) {
			register char *oldclp = rp->r_clistp;

			rp->r_clistp = newclp;
			if (oldclp) {
				rf_remalist(oldclp);
			}
		}
		return 0;
	}

	/* Get a vnode reference for the directory to be advertised */
	if ((error = lookupname(advp->path, UIO_USERSPACE, FOLLOW,
	  NULLVPP, &vp)) != 0) {
		goto bad;
	}
	/*
	 * Allocate a new rf_resource_t now so that the following won't
	 * sleep.
	 */
	if ((newrp = allocrsc()) == NULL) {
		error = ENOMEM;
		goto bad;
	}
	if (ISRFSVP(vp)) {
		error = EREMOTE;
		goto bad;
	}

	if (vp->v_type != VDIR)	 {
		/* the specified path name was not a directory */
		error = ENOTDIR;
		goto bad;
	}

	/*
	 * if the file system containing the specified directory was mounted
	 * read only, the advertisement must be read only.
	 */
	if ((vp->v_vfsp->vfs_flag & VFS_RDONLY) && !(advp->flag & A_RDONLY)) {
		error = EROFS;
		goto bad;
	}
	if ((rp = vp_to_rsc(vp)) != NULL) {
		if (rp->r_flags & R_FUMOUNT || !(rp->r_flags & R_UNADV)) {
			/* already advertised */
			error = EADV;
			goto bad;
		}
		if (strcmp(rp->r_name, advp->resource)) {
			/* Different name, same node. */
			error = EEXIST;
			goto bad;
		}
		/* this is a readvertisement
		 *
		 * read-write permissions on the readvertise must be >=
		 * those already in the table.
		 */
		if ((advp->flag & A_RDONLY) && !(rp->r_flags & R_RDONLY)) {
			error = EACCES;
			goto bad;
		}
		/* add the authorization list, if any, make sure that any
		 * clients with this resource still mounted are in the new
		 * clist
		 */
		if (newclp) {
			for (srp = rp->r_mountp; srp; srp = srp->srm_nextp) {
				/* search gdp to find name for this sysid */
				for (gdpp = gdp; gdpp->sysid != srp->srm_sysid;
				  gdpp++) {
					;
				}
				if (!rf_checkalist(newclp,
				  gdpp->token.t_uname)) {
					error = ESRMNT;
					goto bad;
				}
			}
			rp->r_clistp = newclp;
		}

		/* specify advertisement as read-only if requested */
		if (advp->flag & A_RDONLY) {
			rp->r_flags |= R_RDONLY;
		}
		/*
		 * It is assumed that at this point, the current rp
		 * represents a resource that had been unadvertised,
		 * but that was still remotely mounted.  Then, the
		 * attached rcvd is still defined.  Its refcnt is
		 * increased to account for its use as an advertised
		 * rf_rsrc.
		 */
		rp->r_queuep->rd_refcnt++;
		rp->r_flags &= ~R_UNADV;
		VN_RELE(vp);		/* there was already a reference */
		freersc(&newrp);	/* unused since this was a re-adv. */
		nadvertise++;
		return 0;
	}

	/*
	 * Don't allow reuse of a name for a new advertisement.
	 */
	if (name_to_rsc(adv_name)) {
		error = EBUSY;
		goto bad;
	}

	/*
	 * allocate a general receive descriptor for the resource
	 */
	if ((giftrd = vtord(vp)) == NULL) {
		if ((error = rcvd_create(TRUE, RDGENERAL, &giftrd)) != 0) {
			goto bad;
		}
		giftrd->rd_vp = vp;
		VN_HOLD(vp);		/* this will be released by rcvd_free */
	} else {
		ASSERT(giftrd->rd_vp == vp);
		giftrd->rd_refcnt++;
	}
	/*
	 * insertrsc finds the next free resource index value, assigning
	 * that into newrp->r_mntid.  Newrp is also linked into the
	 * rf_resource_head list.
	 */
	if ((error = insertrsc(newrp)) != 0) {
		goto bad;
	}
	newrp->r_clistp = newclp;
	newrp->r_flags = advp->flag & A_RDONLY;	/* 0 or A_RDONLY */
	newrp->r_rootvp = vp;
	strcpy(newrp->r_name, adv_name);
	newrp->r_queuep = giftrd;
	DUPRINT2(DB_MNT_ADV, "exit adv: error is %d\n", error);
	nadvertise++;
	return 0;
bad:
	rf_remalist(newclp);
	if (vp) {
		VN_RELE(vp);
	}
	if (newrp) {
		freersc(&newrp);
	}
	if (giftrd) {
		rcvd_delete(&giftrd, 0, (long)-1, 1, u.u_cred);
	}
	return error;
}

/*
 * Unadvertise the named resource, return 0 for success, a non-zero errno
 * for failure.
 */

typedef struct rfsys_unadvfsa {
	int	opcode;
	char	*resource;
} rfsys_unadvfsa_t;

/* ARGSUSED */
STATIC int
rfsys_unadvfs(uap, rvp)
	caddr_t			uap;
	rval_t			*rvp;
{
	rfsys_unadvfsa_t	*unap = (rfsys_unadvfsa_t *)uap;
	register rf_resource_t	*rp;
	char			adv_name[RFS_NMSZ];
	char			*clistp;
	vnode_t			*vp;
	uint			c_count;	/* copyinstr out parameter */
	int			error;

	if (rf_state != RF_UP)  {
		return ENONET;
	}
	if (!suser(u.u_cred)) {
		return EPERM;
	}
	/* get the name to be unadvertised */
	if (error = copyinstr(unap->resource, adv_name, RFS_NMSZ, &c_count)) {
		return error;
	}
	if (c_count == 1) {
		return EINVAL;		/* don't allow a NULL resource name */
	}
	if ((rp = name_to_rsc(adv_name)) == NULL) {     /* not advertised */
		return ENODEV;
	}
	if (rp->r_flags & R_FUMOUNT || rp->r_flags & R_UNADV) { 
		/*
		 * has already been unadvertised
		 */
		return ENODEV;
	}
	rp->r_flags |= R_UNADV;         /* mark this as unadvertised */
	rcvd_delete(&rp->r_queuep, (sysid_t)0, (long)-1, 1, u.u_cred);
	clistp = rp->r_clistp;
	rp->r_clistp = NULL;
	if (!rp->r_mountp) {	/* not currently mounted, remove */
		rf_resource_t *arp = rp;	/* addressable */

		vp = rp->r_rootvp;
		freersc(&arp);
		VN_RELE(vp);		/* NOTE:  maintain this ordering */
	} else {
		/* EMPTY */
		ASSERT(rp->r_queuep);
	}

	/*
	 * Get rid of auth list regardless of whether entry has gone away.
	 *
	 * NOTE:  it is necessary that rf_remalist() follows freersc() since it
	 * may sleep.
	 */
	if (clistp) {
		rf_remalist(clistp);
	}
	--nadvertise;
	return 0;
}
