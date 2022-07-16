/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:session.c	1.3.2.1"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/proc.h"
#include "sys/session.h"
#include "sys/kmem.h"
#include "vm/faultcatch.h"

sess_t session0 = {
	1,	/* s_ref   */
	0555,	/* s_mode  */
	0,	/* s_uid   */
	0,	/* s_gid   */
	0,	/* s_ctime */
	NODEV,	/* s_dev   */
	NULL,	/* s_vp    */
	&pid0,	/* s_sidp  */
	NULL	/* s_cred  */
};

int
sess_rele(sp)
	register sess_t *sp;
{
	ASSERT(sp != &session0);
	PID_RELE(sp->s_sidp);
	kmem_free(sp,sizeof(sess_t));
	return 1;
}

void
sess_create()
{
	register proc_t *pp;
	register sess_t *sp;

	pp = u.u_procp;

	pgexit(pp);
	SESS_RELE(pp->p_sessp);

	sp = (sess_t *)kmem_zalloc(sizeof (sess_t), KM_SLEEP);
	sp->s_sidp = pp->p_pidp;
	sp->s_ref = 1;
	sp->s_dev = NODEV;
	pp->p_sessp = sp;
	u.u_ttyp = NULL; /* compatibility */

	pgjoin(pp, pp->p_pidp);

	PID_HOLD(sp->s_sidp);
}

void
freectty(sp)
	register sess_t *sp;
{
	register vnode_t *vp;
	int i;

	vp = sp->s_vp;
	ASSERT(vp != NULL);

	if (vp->v_stream != NULL)
		strfreectty(vp->v_stream);
	else {	/* may be clist driver */
		ASSERT(u.u_ttyp != NULL);
		signal((pid_t)(*u.u_ttyp), SIGHUP);
		*u.u_ttyp = 0;
	}

	for (i = vp->v_count; i; i--) {
		VOP_CLOSE(vp, 0, 1, (off_t)0, sp->s_cred);
		VN_RELE(vp);
	}

	crfree(sp->s_cred);
	sp->s_vp = NULL;

}

dev_t
cttydev(pp)
proc_t *pp;
{
	register sess_t *sp = pp->p_sessp;
	if (sp->s_vp == NULL)
		return NODEV;
	return sp->s_dev;
}

void
alloctty(pp, vp)
register proc_t *pp;
vnode_t *vp;
{
	register sess_t *sp = pp->p_sessp;
	register cred_t *crp = pp->p_cred;

	sp->s_vp = vp;
	sp->s_dev = vp->v_rdev;

	crhold(crp);
	sp->s_cred = crp;
	sp->s_uid = crp->cr_uid;
	sp->s_ctime = hrestime.tv_sec;
	if (session0.s_mode & VSGID)
		sp->s_gid = session0.s_gid;
	else
		sp->s_gid = crp->cr_gid;
	sp->s_mode = 0666;
	CATCH_FAULTS(CATCH_SEGU_FAULT)
		sp->s_mode &= ~(PTOU(pp)->u_cmask);
	END_CATCH();
}

int
hascttyperm(sp, cr, mode)
	register sess_t *sp;
	register cred_t *cr;
	register mode_t mode;
{

	if (cr->cr_uid == 0)
		return 1;

	if (cr->cr_uid != sp->s_uid) {
		mode >>= 3;
		if (!groupmember(sp->s_gid, cr))
			mode >>= 3;
	}

	if ((sp->s_mode & mode) == mode)
		return 1;

	return 0;
}

int
realloctty(frompp, sid)
	proc_t *frompp;
	pid_t sid;
{
	proc_t *topp;
	register sess_t *fromsp;
	register sess_t *tosp;
	cred_t *fromcr;
	vnode_t *fromvp;

	fromsp = frompp->p_sessp;
	fromvp = fromsp->s_vp;
	fromcr = frompp->p_cred;
	
	if (!hascttyperm(&session0, fromcr, VEXEC|VWRITE))
		return EACCES;

	if ((session0.s_mode & VSVTX) 
	  && fromcr->cr_uid != session0.s_uid
	  && (!hascttyperm(fromsp, fromcr, VWRITE)))
		return EACCES;

	if (sid == 0) {
		freectty(fromsp);
		return 0;
	}

	if (fromvp->v_stream == NULL)
		return ENOSYS;

	if ((topp = prfind(sid)) == NULL)
		return ESRCH;

	tosp = topp->p_sessp;

	if (tosp->s_sidp != topp->p_pidp
	  || tosp->s_vp != NULL
	  || !hasprocperm(topp->p_cred, frompp->p_cred))
		return EPERM;

	strfreectty(fromvp->v_stream);
	crfree(fromsp->s_cred);

	alloctty(topp, fromvp);
	stralloctty(tosp, fromvp->v_stream);

	fromsp->s_vp = NULL;

	return 0;
}
