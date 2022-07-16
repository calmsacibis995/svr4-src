/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/chanmux.c	1.3.1.1"

/*
 * IWE Channel Multiplexor
 * Multiplexes N secondary input devices (lower streams) across
 * M primary input/output channels (referred to as principal streams)
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/buf.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/open.h"
#include "sys/termios.h"
#include "sys/file.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strtty.h"
#include "sys/kmem.h"
#include "sys/ws/chan.h"
#include "sys/chanmux.h"
#include "sys/cred.h"
#include "sys/cmn_err.h"
#include "sys/proc.h"	
#include "sys/ddi.h"


#define INHI	4096
#define INLO	512
#define OUTHI	512
#define OUTLO	128

int nulldev();
int cmuxopen(), cmuxclose(), cmux_up_rsrv(), cmux_up_wsrv();
int cmux_up_rput(), cmux_up_wput(), cmux_mux_wsrv(), cmux_mux_rsrv();
int cmux_mux_rput(), cmux_mux_wput();

struct module_info cmux_iinfo = {
	0, "cmux", 0, INFPSZ, INHI, INLO };


struct module_info cmux_oinfo = {
	0, "cmux", 0, CMUXPSZ, OUTHI, OUTLO };


struct qinit cmux_up_rinit = {
	cmux_up_rput, cmux_up_rsrv, cmuxopen, cmuxclose, NULL, &cmux_iinfo, NULL };


struct qinit cmux_up_winit = {
	cmux_up_wput, cmux_up_wsrv, cmuxopen, cmuxclose, NULL, &cmux_oinfo, NULL };

struct qinit cmux_mux_rinit = {
	cmux_mux_rput, cmux_mux_rsrv, nulldev, nulldev, NULL, &cmux_iinfo, NULL };


struct qinit cmux_mux_winit = {
	cmux_mux_wput, cmux_mux_wsrv, nulldev, nulldev, NULL, &cmux_oinfo, NULL };


struct streamtab cmuxinfo = {
	&cmux_up_rinit, &cmux_up_winit, &cmux_mux_rinit, &cmux_mux_winit };

int cmuxdevflag = 0;

cmux_ws_t **wsbase;
unsigned long numwsbase = 0;	/* number of workstations allocated */

int ws_getws();
int ws_getchanno();
void ws_clrcompatflgs();

int
cmuxstart()
{
#ifdef DEBUG
	cmn_err(CE_NOTE,"in cmuxstart!");
#endif
	wsbase = (cmux_ws_t **) kmem_alloc(CMUX_WSALLOC*sizeof(cmux_ws_t *),KM_NOSLEEP);
	if (wsbase == (cmux_ws_t **) NULL)
		cmn_err (CE_PANIC,"cannot allocate workstation space");

	bzero(wsbase,CMUX_WSALLOC * sizeof(cmux_ws_t *));
	numwsbase = CMUX_WSALLOC;
	ws_initcompatflgs();
}


/* cmux_realloc: allocate to the power of two greater than the
 * argument passed in: copy over old ptrs and NULL new ones out.
 * Return ENOMEM if kmem fails, 0 otherwise
 */

int
cmux_realloc(wsno)
unsigned long wsno;
{
	cmux_ws_t **nwsbase,**owsbase;
	int i;

	if (wsno < numwsbase) return (0);
	wsno = (wsno >> 1) << 2; /* round up to next power of 2 */

	owsbase = wsbase;

	if ((nwsbase=(cmux_ws_t **)kmem_alloc(wsno * sizeof(cmux_ws_t *),KM_SLEEP)) == (cmux_ws_t **) NULL)
		return (ENOMEM);

	bcopy(wsbase, nwsbase, numwsbase*sizeof(cmux_ws_t *));

	for (i=numwsbase; i<wsno; i++)
		nwsbase[i] = (cmux_ws_t *) NULL;

	wsbase = nwsbase;
	kmem_free(owsbase, numwsbase*sizeof(cmux_ws_t *));
	numwsbase = wsno;

	return(0);
}

int
cmux_allocstrms(wsp,numstreams)
cmux_ws_t *wsp;
unsigned long numstreams;
{
	unsigned long size, onumlstrms;
	cmux_link_t *olstrmsp, *nlstrmsp;

	onumlstrms = wsp->w_numlstrms;
	numstreams = max(CMUX_STRMALLOC,(numstreams>>1) << 2);
	if (numstreams <= onumlstrms)
		return (0);	/* enough lower streams allocated */

	olstrmsp = wsp->w_lstrmsp;

	size = numstreams*sizeof(cmux_link_t);
	nlstrmsp = (cmux_link_t *) kmem_alloc(size,KM_NOSLEEP);
	if (nlstrmsp == (cmux_link_t *) NULL)
		return (ENOMEM);

	bzero(nlstrmsp, size);

	if (olstrmsp) {
		bcopy(olstrmsp, nlstrmsp, onumlstrms*sizeof(cmux_link_t));
		wsp->w_lstrmsp = nlstrmsp;
		kmem_free(olstrmsp, onumlstrms*sizeof(cmux_link_t));
	}
	else
		wsp->w_lstrmsp = nlstrmsp;

	wsp->w_numlstrms = numstreams;

	return(0);
}


/*
 * allocate cmux_t pointers and cmux_link_t structs for numchan channels
 */

int
cmux_allocchan(wsp,numchan)
cmux_ws_t *wsp;
unsigned long numchan;
{
	unsigned long cmuxsz, princsz, onumchan;
	cmux_t **ocmuxpp, **ncmuxpp;
	cmux_link_t *oprincp, *nprincp;

	onumchan = wsp->w_numchan;
	numchan = max(CMUX_CHANALLOC, (numchan>>1) << 2);
	if (numchan <= onumchan)
		return (0);	/* already allocated enough space */

	ocmuxpp = wsp->w_cmuxpp;
	oprincp = wsp->w_princp;

	cmuxsz = numchan*sizeof(cmux_t *);
	ncmuxpp = (cmux_t **) kmem_alloc(cmuxsz,KM_SLEEP);
	if (ncmuxpp == (cmux_t **) NULL)
		return (ENOMEM);

	princsz = numchan*sizeof(cmux_link_t);
	nprincp = (cmux_link_t *)kmem_alloc(princsz,KM_SLEEP);
	if (nprincp == (cmux_link_t *) NULL)
	{
		kmem_free(ncmuxpp,cmuxsz);
		return (ENOMEM);
	}

	bzero(ncmuxpp,cmuxsz);
	bzero(nprincp,princsz);

#ifdef DEBUG1
	cmn_err(CE_NOTE,"ncmuxpp %x ocmuxpp %x size %x onumchan %x",ncmuxpp,ocmuxpp,cmuxsz,onumchan);
#endif
	if (ocmuxpp) {
		bcopy(ocmuxpp, ncmuxpp, onumchan*sizeof(cmux_t *));
		wsp->w_cmuxpp = ncmuxpp;
		kmem_free(ocmuxpp, onumchan*sizeof(cmux_t *));
	}
	else
		wsp->w_cmuxpp = ncmuxpp;

	if (oprincp) {
		bcopy(oprincp, nprincp, onumchan*sizeof(cmux_link_t));
		wsp->w_princp = nprincp;
		kmem_free(oprincp, onumchan*sizeof(cmux_link_t));
	}
	else
		wsp->w_princp = nprincp;

	wsp->w_numchan = numchan;
	return (0);
}

/* cmux_initws: allocate space for per-channel struct; clear flags;
 * Return ENOMEM if kmem fails.
 */

int
cmux_initws(wsp, numchan)
cmux_ws_t *wsp;
unsigned long numchan;
{
	int error = 0;
	struct cmux_swtch *switchp;

	if (error = cmux_allocchan(wsp,numchan))
		return error;

	if (error = cmux_allocstrms(wsp,CMUX_STRMALLOC))
		return error;

	wsp->w_numswitch = 1;
	switchp = &wsp->w_swtchtimes[0];
	drv_getparm(LBOLT,&switchp->sw_time);
	switchp->sw_chan = numchan-1; 
#ifdef DEBUG1
	cmn_err(CE_NOTE,"switch time is %x, switch chan is %x",
	   wsp->w_swtchtimes[0].sw_time,wsp->w_swtchtimes[0].sw_chan);
#endif
	return (0);
}

/* cmux_openchan: alloocate space for the new channel structure.
 * Make sure that pointers to ws struct and queues get set up.
 * Return an error number based on the success of allocation.
 * Send a ch_proto message indicating that a channel is opening
 * up.
 */

int wakeup();
static int openflg = 0;

int
cmux_openchan(qp, wsp, chan, dev, flag)
queue_t *qp;
cmux_ws_t *wsp;
unsigned long chan;
dev_t dev;
int flag;
{
	int error, oldpri;
	cmux_t *cmuxp;
	mblk_t *mp;
	ch_proto_t *protop;
	struct proc *procp;
	cmux_link_t *linkp;

	if (error = cmux_allocchan(wsp, chan+1))
		return error;

	if (qp->q_ptr) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid open state! Open fails.");
#endif
		return (ENXIO);
	}


	cmuxp = (cmux_t *) kmem_alloc(sizeof(cmux_t), KM_SLEEP);
	if (cmuxp == (cmux_t *) NULL) return ENOMEM;

	cmuxp->cmux_dev = dev;
	cmuxp->cmux_num = chan;
	cmuxp->cmux_wsp = wsp;
	cmuxp->cmux_rqp = qp;
	cmuxp->cmux_wqp = WR(qp);
	cmuxp->cmux_flg = CMUX_OPEN;
	wsp->w_cmuxpp[chan] = cmuxp;

	qp->q_ptr = (caddr_t) cmuxp;
	WR(qp)->q_ptr = (caddr_t) cmuxp;

	while ( (mp=allocb(sizeof(ch_proto_t), BPRI_HI)) == (mblk_t *) NULL)
	{
		oldpri = splstr();
		(void) bufcall(sizeof(ch_proto_t), BPRI_HI, wakeup,
			(caddr_t) &qp->q_ptr);
		(void) sleep( (caddr_t) &qp->q_ptr, STIPRI);
		splx(oldpri);
	}

	mp->b_wptr += sizeof(ch_proto_t);
	mp->b_datap->db_type = M_PCPROTO;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type = CH_CTL;
	protop->chp_stype = CH_CHAN;
	protop->chp_stype_cmd = CH_CHANOPEN;
	drv_getparm(UPROCP,&procp);
	protop->chp_stype_arg = procp->p_ppid;	
	protop->chp_chan = chan;

	putq(cmuxp->cmux_wqp, mp); /* put it on our write queue to ship to
		 	       * principal stream when it is opened
			       */
	linkp = wsp->w_princp + chan;

	openflg --;
	wakeup (&openflg); /* at this point in open, safe to allow another in */

	if (!linkp->cmlb_flg) return (0); /* don't sleep on open until
					   * mux is initialized */
	linkp->cmlb_flg |= CMUX_PRINCSLEEP;

 	if (sleep(&cmuxp->cmux_flg, STOPRI|PCATCH)) {
		linkp->cmlb_flg &= ~CMUX_PRINCSLEEP;
		wsp->w_cmuxpp[chan] = (cmux_t *) NULL;
		qp->q_ptr = (caddr_t) NULL;
		WR(qp)->q_ptr = (caddr_t) NULL;
		kmem_free(cmuxp,sizeof(cmux_t));
 		return (EINTR);
 	}
	linkp->cmlb_flg &= ~CMUX_PRINCSLEEP;
	qenable( RD(linkp->cmlb_lblk.l_qbot) );
	if (linkp->cmlb_err) {
		wsp->w_cmuxpp[chan] = (cmux_t *) NULL;
		qp->q_ptr = (caddr_t) NULL;
		WR(qp)->q_ptr = (caddr_t) NULL;
		kmem_free(cmuxp,sizeof(cmux_t));
	}
	return (linkp->cmlb_err);
}

int
cmuxopen(qp, devp, flag, sflag, credp)
queue_t *qp;
dev_t *devp;
int flag, sflag;
cred_t *credp;
{
	register unsigned long chan, wsno;
	register int oldpri, dev;
	cmux_ws_t *wsp;
	mblk_t *mp;
	struct stroptions *sop;
	int error = 0;

	if (sflag)
		return (EINVAL); /* can't be opened as module */

	dev = getminor(*devp);
	wsno = ws_getws(dev);
	chan = ws_getchanno(dev);

	if (qp->q_ptr) {
		cmux_t *cmuxp = (cmux_t *) qp->q_ptr;

		if (cmuxp->cmux_num != chan) {
#ifdef DEBUG1
			cmn_err(CE_WARN, "chanmux: found q_ptr != chan in open; open fails");
#endif
			return (EINVAL);
		}

		if (cmuxp->cmux_flg & CMUX_CLOSE)
			return (EAGAIN); /* prevent open during close */
		else
			return (flag & FEXCL) ? EBUSY : 0;
	}

	oldpri = splhi(); /* only permit one open at a time for sanity's sake */

	/* check to see if FNONBLOCK or FNDELAY set;
	 * return EBUSY if so 
	 */

	if (openflg && (flag & FNONBLOCK ||  flag & FNDELAY)) {
		splx(oldpri);
		return (EAGAIN);
	}

	while (openflg > 0)
		sleep (&openflg, TTIPRI);

	openflg++;
	splx(oldpri);

	if (wsno >= numwsbase)
		error = cmux_realloc(wsno);
	if (error) goto openexit;

	wsp = wsbase[wsno];

	if (wsp == (cmux_ws_t *) NULL) {
		unsigned long size;
		size = sizeof(cmux_ws_t);
		if ( (wsp=(cmux_ws_t *)kmem_alloc(size,KM_SLEEP)) == (cmux_ws_t *) NULL) {
			error = ENOMEM;
			goto openexit;
		}

		bzero(wsp, size);
		wsbase[wsno] = wsp;

		if (error = cmux_initws(wsp, chan+1))
			goto openexit;
	} /* wsp == NULL */

	/* open channel */


	/* openflg decrement and wakeup done in cmux_openchan if
	 * we reach here
	 */
	error = cmux_openchan(qp, wsp, chan, *devp, flag);
	if (error)
		return (error);

	/* allocate stroptions struct and indicate that stream is TTY */

	while ( (mp=allocb(sizeof(struct stroptions), BPRI_HI)) == (mblk_t *) NULL)
	{
		register struct stroptions *sop;

		oldpri = splstr();
		(void) bufcall(sizeof(ch_proto_t), BPRI_HI, wakeup,
			(caddr_t) &qp->q_ptr);
		(void) sleep( (caddr_t) &qp->q_ptr, STIPRI);
		splx(oldpri);
	}

	mp->b_datap->db_type = M_SETOPTS;
	mp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) mp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
	sop->so_hiwat = INHI;
	sop->so_lowat = INLO;
	putnext(qp,mp);

	return (0);

openexit:
	openflg --;
	wakeup (&openflg);
	return (error);
}


int
cmuxclose(qp)
queue_t *qp;
{
	cmux_t *cmuxp;
	cmux_ws_t *wsp;
	mblk_t *mp;
	ch_proto_t *protop;
	int oldpri;
	cmux_link_t *linkp;


	cmuxp = (cmux_t *) qp->q_ptr;

	if (cmuxp == (cmux_t *) NULL)
	{
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: finding invalid q_ptr in cmuxclose");
#endif
		return (ENXIO);	
	}

	oldpri=splstr();
	cmuxp->cmux_flg |= CMUX_CLOSE;
	splx(oldpri);

	/* check to see if principal stream linked underneath.
	 * If not, flush the queues and return
	 */

	wsp = cmuxp->cmux_wsp;
	linkp = wsp->w_princp + cmuxp->cmux_num;
	if (!linkp->cmlb_flg) {
		flushq(qp,FLUSHALL);
		flushq(WR(qp),FLUSHALL);
		return (0);
	}

	/* principal stream linked underneath. Allocate "channel closing"
	 * message and ship it to principal stream. It should respond
	 * with a "channel closed" message. */

	while ( (mp=allocb(sizeof(ch_proto_t), BPRI_HI)) == (mblk_t *) NULL)
	{
		oldpri = splstr();
		(void) bufcall(sizeof(ch_proto_t), BPRI_HI, wakeup,
			(caddr_t) &qp->q_ptr);
		(void) sleep( (caddr_t) &qp->q_ptr, STIPRI);
		splx(oldpri);
	}

	/* want this to be last message received on the channel,
	   so make it normal priority so that it doesn't get ahead of
	   user data */

	mp->b_wptr += sizeof(ch_proto_t);
	mp->b_datap->db_type = M_PROTO;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type = CH_CTL;
	protop->chp_stype = CH_CHAN;
	protop->chp_stype_cmd = CH_CHANCLOSE;
	protop->chp_chan = cmuxp->cmux_num;

	putq(cmuxp->cmux_wqp, mp); /* put it on our write queue to ship to
		 	       * principal stream 
			       */
	oldpri=splstr();
	cmuxp->cmux_flg |= CMUX_WCLOSE; /* waiting for close */
	while (cmuxp->cmux_flg & CMUX_WCLOSE)
		sleep(cmuxp,PZERO+1);
	splx(oldpri);

#ifdef DEBUG1
	cmn_err(CE_WARN,"chanmux: woken up from close on channel %d",cmuxp->cmux_num);
#endif
	qp->q_ptr = NULL;
	WR(qp)->q_ptr = NULL;
	ws_clrcompatflgs(cmuxp->cmux_dev);

	/* 
	 * release the channel.
	 */

	wsp->w_cmuxpp[cmuxp->cmux_num] = (cmux_t *) NULL;

	kmem_free(cmuxp, sizeof(cmux_t));
	return (0);
}


/* STATIC */ int
cmux_unlink(mp, cmuxp, iocp)
mblk_t *mp;
cmux_t *cmuxp;
struct iocblk *iocp;
{
	register cmux_ws_t *wsp;
	cmux_lstrm_t *lstrmp;
	cmux_link_t *linkp;
	struct linkblk *ulinkbp;
	int i, oldpri;

	wsp = cmuxp->cmux_wsp;
	ulinkbp = (struct linkblk *) mp->b_cont->b_rptr;

	linkp = wsp->w_princp + cmuxp->cmux_num;
#ifdef DEBUG1
	cmn_err(CE_NOTE,"In cmux_unlink. ");
#endif
	if (linkp->cmlb_lblk.l_index == ulinkbp->l_index)
	{
		oldpri = splstr();
		kmem_free(linkp->cmlb_lblk.l_qbot->q_ptr,sizeof(cmux_lstrm_t));
		linkp->cmlb_lblk.l_qbot->q_ptr = NULL;
		RD(linkp->cmlb_lblk.l_qbot)->q_ptr = NULL;
		bzero(linkp,sizeof(cmux_link_t));
		splx(oldpri);
#ifdef DEBUG1
	cmn_err(CE_NOTE,"unlinked principal stream. ");
#endif
		return (1);
	}

	linkp = wsp->w_lstrmsp;
	for (i=0; i<wsp->w_numlstrms; i++,linkp++) 
	   if ( (linkp->cmlb_flg) && (linkp->cmlb_lblk.l_index == ulinkbp->l_index))
	   {
		oldpri = splstr();
		kmem_free(linkp->cmlb_lblk.l_qbot->q_ptr,sizeof(cmux_lstrm_t));
		linkp->cmlb_lblk.l_qbot->q_ptr = NULL;
		RD(linkp->cmlb_lblk.l_qbot)->q_ptr = NULL;
		bzero(linkp,sizeof(cmux_link_t));
		wsp->w_lstrms--;
		splx(oldpri);
#ifdef DEBUG1
	cmn_err(CE_NOTE,"unlinked secondary stream. ");
#endif
		return (1);
	   }

	return (0);
}


void
cmux_close_chan(wsp,lstrmp,protop)
cmux_ws_t *wsp;
cmux_lstrm_t *lstrmp;
ch_proto_t *protop;
{
	unsigned long chan;
	cmux_t *cmuxp;
	int oldpri;

	chan = lstrmp->lstrm_id;
	cmuxp = wsp->w_cmuxpp[chan];
	if (!cmuxp) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"Found null cmuxp; do not wakeup");
#endif
		return;
	}

	oldpri = splstr();
	cmuxp->cmux_flg &= ~CMUX_WCLOSE;
#ifdef DEBUG1
	cmn_err(CE_NOTE,"Close on channel %d",cmuxp->cmux_num);
#endif
	wakeup(cmuxp);
#ifdef DEBUG1
	cmn_err(CE_NOTE,"Called wakeup channel %d",cmuxp->cmux_num);
#endif
	splx(oldpri);
	return;
}


clock_t
cmux_striptime(mp)
register mblk_t *mp;
{
	register ch_proto_t *protop;

	protop = (ch_proto_t *) mp->b_rptr;
	return (protop->chp_tstmp);
}

int
cmux_foundit(mintime, maxtime, timeval)
clock_t mintime, maxtime, timeval;
{
	if (maxtime >= mintime) /* no wrap */
		return (timeval <= maxtime && timeval >= mintime);
	else
		return !(timeval < mintime && timeval > maxtime);
}

/*
 * 	100  <-- most recent switch 
 *	 30
 *	930
 *	700  <-- last switch
 */

cmux_t *
cmux_findchan(wsp, timestamp)
cmux_ws_t *wsp;
clock_t timestamp;
{
	struct cmux_swtch *switchp;
	clock_t curtime, mintime;
	int found,cnt;
	
	drv_getparm(LBOLT,&curtime);
	switchp = &wsp->w_swtchtimes[wsp->w_numswitch - 1];

	if (!cmux_foundit(switchp->sw_time, curtime, timestamp))
		return (cmux_t *) NULL; /* chanmux will drop the message */

	switchp = &wsp->w_swtchtimes[0];
	cnt = 0;
	mintime = switchp->sw_time;
	found = cmux_foundit(mintime, curtime, timestamp);
	while ( (cnt < wsp->w_numswitch - 1) && !found) {
		switchp++;
		cnt++;
		curtime = mintime;
		mintime = switchp->sw_time;
		found = cmux_foundit(mintime, curtime, timestamp);
	}

	if (!found)
		return (cmux_t *) NULL;

	return (wsp->w_cmuxpp[switchp->sw_chan]);
}


void
cmux_clr_ioc(wsp)
register cmux_ws_t *wsp;
{
	register cmux_link_t *linkp;
	register cmux_t *cmuxp;
	int oldpri, i;
	
#ifdef DEBUG1
	cmn_err(CE_WARN,"In cmux_clrioc:");
#endif
	if (wsp->w_iocmsg) freemsg(wsp->w_iocmsg);
	wsp->w_iocmsg = (mblk_t *) NULL;

	linkp = wsp->w_princp + wsp->w_ioctlchan;
	linkp->cmlb_iocresp = 0;
	if (linkp->cmlb_iocmsg) freemsg (linkp->cmlb_iocmsg);

	for (i=0,linkp = wsp->w_lstrmsp; i<wsp->w_numlstrms; i++,linkp++) {
		linkp->cmlb_iocresp = 0;
		if (linkp->cmlb_iocmsg) freemsg (linkp->cmlb_iocmsg);
	}

	oldpri = splstr();
	wsp->w_ioctlcnt = wsp->w_ioctllstrm = 0;
	wsp->w_ioctlchan = 0;
	wsp->w_state &= ~CMUX_IOCTL;
	splx(oldpri);

	for (i=0; i<wsp->w_numchan; i++) {
	   cmuxp = wsp->w_cmuxpp[i];
	   if (cmuxp && cmuxp->cmux_wqp)
		qenable(cmuxp->cmux_wqp);
	}
}

void
cmux_switch_chan(wsp,protop)
register cmux_ws_t *wsp;
register ch_proto_t *protop;
{
	int oldpri,i;
	struct cmux_swtch *switchp;
	unsigned long chan;

	wsp->w_numswitch = min(wsp->w_numswitch + 1, CMUX_NUMSWTCH);
	chan = protop->chp_chan;

	if (!wsp->w_cmuxpp[chan]) { /* invalid channel request? */
		cmn_err(CE_PANIC,"invalid channel switch request %x",chan);
	}

#ifdef DEBUG1
	cmn_err(CE_NOTE,"switch_chan: switching to channel %x",chan);
#endif
	/* must be a good channel. Update switchtime list */
	oldpri = splstr();
	for (i=CMUX_NUMSWTCH-1; i>0; i--) 
	{
#ifdef DEBUG1
	   cmn_err(CE_WARN,"wsp %x, i %x, i-1 %x",wsp,&wsp->w_swtchtimes[i],&wsp->w_swtchtimes[i-1]);
#endif
	   bcopy(&wsp->w_swtchtimes[i-1],&wsp->w_swtchtimes[i],sizeof(struct cmux_swtch));
	}
	switchp = &wsp->w_swtchtimes[0];
#ifdef DEBUG1
	   cmn_err(CE_WARN,"wsp %x, switchp %x",wsp,switchp);
#endif
	switchp->sw_chan = protop->chp_chan;
	switchp->sw_time = protop->chp_tstmp;
	splx(oldpri);
	return;
}


/* STATIC */ void
cmux_iocack(qp, mp, iocp, rval)
queue_t *qp;
mblk_t *mp;
struct iocblk *iocp;
int rval;
{
	mblk_t	*tmp;

	mp->b_datap->db_type = M_IOCACK;
	iocp->ioc_rval = rval;
	iocp->ioc_count = iocp->ioc_error = 0;
	if ((tmp = unlinkb(mp)) != (mblk_t *)NULL)
		freeb(tmp);
	qreply(qp,mp);
}

/* STATIC */ void
cmux_iocnak(qp, mp, iocp, error, rval)
queue_t *qp;
mblk_t *mp;
struct iocblk *iocp;
int error;
int rval;
{
	mp->b_datap->db_type = M_IOCACK;
	iocp->ioc_rval = rval;
	iocp->ioc_error = error;
	qreply(qp,mp);
}

cmux_do_iocresp(wsp)
cmux_ws_t *wsp;
{
	cmux_link_t *linkp;
	unsigned long ackcnt, nakcnt, acknum, naknum, i;
	struct iocblk *iocp;
	struct copyresp *resp;
	cmux_t *cmuxp;
	mblk_t *mp;

	ackcnt = nakcnt = 0;
	acknum = naknum = 0;
	cmuxp = wsp->w_cmuxpp[wsp->w_ioctlchan];

	linkp = wsp->w_princp + wsp->w_ioctlchan;
	iocp = (struct iocblk *) linkp->cmlb_iocmsg->b_rptr;

	if (linkp->cmlb_iocresp == M_IOCACK) 
		ackcnt++;
	else if (linkp->cmlb_iocresp == M_IOCNAK) 
		nakcnt++;

	linkp = wsp->w_lstrmsp;
	for (i=1; i<= wsp->w_numlstrms; i++,linkp++)
	   if (linkp->cmlb_iocresp == M_IOCACK) {
		ackcnt++;
		acknum = i;
	   }
	   else if (linkp->cmlb_iocresp == M_IOCNAK) {
		nakcnt++;
		iocp = (struct iocblk *) linkp->cmlb_iocmsg->b_rptr;
		if (iocp->ioc_error != 0)
			naknum = i;
	   }

	if (ackcnt == 0) { /* failure all around */
#ifdef DEBUG1
	   cmn_err(CE_NOTE,"cmux_do_iocresp: everyone nackd");
#endif
	   if (naknum == 0) {
		linkp = wsp->w_princp + wsp->w_ioctlchan;
		putnext(cmuxp->cmux_rqp, linkp->cmlb_iocmsg);
		linkp->cmlb_iocmsg = (mblk_t *) NULL;
	   }
	   else {
		linkp = wsp->w_lstrmsp + naknum -1;
		putnext(cmuxp->cmux_rqp, linkp->cmlb_iocmsg);
		linkp->cmlb_iocmsg = (mblk_t *) NULL;
	   }
	   cmux_clr_ioc(wsp);
	}
	else if (ackcnt == 1) { /* success! */
#ifdef DEBUG1
	   cmn_err(CE_NOTE,"cmux_do_iocresp: only one ack");
#endif
	   if (acknum == 0) {
		linkp = wsp->w_princp + wsp->w_ioctlchan;
	   }
	   else {
		linkp = wsp->w_lstrmsp + acknum -1;
	   }
#ifdef DEBUG1
	   cmn_err(CE_NOTE,"cmux_do_iocresp: about to call putnext");
#endif
	   putnext(cmuxp->cmux_rqp, linkp->cmlb_iocmsg);
	   if (linkp->cmlb_iocmsg->b_datap->db_type != M_IOCACK) {
	        linkp->cmlb_iocmsg = (mblk_t *) NULL;
	   	wsp->w_ioctllstrm = acknum;
	   }
	   else {
	        linkp->cmlb_iocmsg = (mblk_t *) NULL;
		cmux_clr_ioc(wsp);
	   }
	}
	else { /* multiple acks. Send up an M_IOCNAK with
		* errno set to EACCES, and fail each
		* M_COPYIN/M_COPYOUT that was passed up
		* by sending down an M_IOCDATA message
		* with ioc_rval set to EACCES.
	   	*/

#ifdef DEBUG1
	   cmn_err(CE_NOTE,"cmux_do_iocresp: multiple acks");
#endif
	   linkp = wsp->w_princp + wsp->w_ioctlchan;
	   mp = linkp->cmlb_iocmsg;

	   if (mp->b_datap->db_type == M_COPYIN ||
	       mp->b_datap->db_type == M_COPYOUT) {
		resp = (struct copyresp *) mp->b_rptr;
		resp->cp_rval = (caddr_t) EACCES;
		mp->b_datap->db_type = M_IOCDATA;
		putnext(linkp->cmlb_lblk.l_qbot,mp);
		linkp->cmlb_iocmsg = (mblk_t *) NULL;
	   }

	   linkp = wsp->w_lstrmsp;
	   for (i=1; i<= wsp->w_numlstrms; i++,linkp++) {
	      if (!linkp->cmlb_flg) continue;
	      mp = linkp->cmlb_iocmsg;
	      if (mp->b_datap->db_type == M_COPYIN ||
	   	  mp->b_datap->db_type == M_COPYOUT) {
			resp = (struct copyresp *) mp->b_rptr;
			resp->cp_rval = (caddr_t) EACCES;
			mp->b_datap->db_type = M_IOCDATA;
			putnext(linkp->cmlb_lblk.l_qbot,mp);
			linkp->cmlb_iocmsg = (mblk_t *) NULL;
	      }
	   } /* for */

	   mp = wsp->w_iocmsg;
	   mp->b_datap->db_type = M_IOCNAK;
	   iocp = (struct iocblk *) mp->b_rptr;
	   iocp->ioc_error = EACCES;
	   putnext(cmuxp->cmux_rqp,mp);
	   wsp->w_iocmsg = (mblk_t *) NULL;
	   cmux_clr_ioc(wsp);
	} /* multiple acks */
}

/* this routine will only get called when a canput from a lower
 * stream to the q_next of this queue failed.
 */
int
cmux_up_rsrv(qp)
queue_t *qp;
{
	cmux_t *cmuxp;
	cmux_ws_t *wsp;
	unsigned long i;
	cmux_link_t *linkp;

	cmuxp = (cmux_t *) qp->q_ptr;
	if (cmuxp == (cmux_t *) NULL) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in up_rsrv");
#endif
		return;
	}

	wsp = cmuxp->cmux_wsp;

	/* qenable all secondary lower streams and the principal
	 * stream associated with channel
	 */

	linkp = wsp->w_lstrmsp; /* do secondary streams first */

#ifdef DEBUG1
	if (linkp == (cmux_link_t *) NULL) {
		cmn_err(CE_WARN,"chanmux: NUll lstrms ptr in up_rsrv");
		return;
	}
#endif

	for (i=0; i<wsp->w_numlstrms; i++,linkp++) {
		if (!linkp->cmlb_flg) continue;
		if (linkp->cmlb_lblk.l_qbot)
		   qenable( RD(linkp->cmlb_lblk.l_qbot) );
	}
	
	/* now enable principal stream */
#ifdef DEBUG1
	if (linkp == (cmux_link_t *) NULL) {
		cmn_err(CE_WARN,"chanmux: NUll princstrms ptr in up_rsrv");
		return;
	}
#endif
	linkp = wsp->w_princp + cmuxp->cmux_num;
	if (linkp->cmlb_lblk.l_qbot)
		qenable( RD(linkp->cmlb_lblk.l_qbot) );
}


/* service routine for all messages going downstream. Do not service
 * any messages until a principal stream is linked below the channel,
 * nor while an ioctl is being processed on a channel different from
 * ours.
 * When the service routine is activated and we find an M_IOCTL
 * message, mark that this channel is servicing an ioctl.
 * Perform a copymsg of the message for each secondary stream
 * and ship the messages to the principal stream for the channel as well
 * as all secondary streams. 
 * For M_FLUSH handling, only flush the principal stream.
 */

int
cmux_up_wsrv(qp)
queue_t *qp;
{
	mblk_t *mp;
	cmux_t *cmuxp;
	cmux_ws_t *wsp;
	cmux_link_t *linkp;
	unsigned long chan;
	int oldpri,i;

	cmuxp = (cmux_t *) qp->q_ptr;
	if (cmuxp == (cmux_t *) NULL) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in up_wsrv");
#endif
		return;
	}

	wsp = cmuxp->cmux_wsp;
	chan = cmuxp->cmux_num;
	linkp = wsp->w_princp + chan;
	if (!linkp->cmlb_flg) return;

	if ( (wsp->w_state & CMUX_IOCTL) && (wsp->w_ioctlchan!=chan)) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: blocked on ioctl");
#endif
		return;
	}
	/* keep getting messages until none left or we honor
	 * flow control and see that the stream above us is blocked
	 * or are set to enqueue messages while an ioctl is processed
	 */

	while ((mp = getq(qp)) != NULL) {
	   switch (mp->b_datap->db_type) {

	   case M_FLUSH:
		/*
	 	 * Flush everything we haven't looked at yet.
		 * Turn the message around if FLUSHR was set
	 	 */
		if (*mp->b_rptr & FLUSHW) {
			flushq(qp, FLUSHDATA);
			*mp->b_rptr &= ~FLUSHW;
		}
		if (*mp->b_rptr & FLUSHR) 
			putnext(RD(qp),mp);
		else 
			freemsg(mp);
		continue;

	   case M_IOCDATA:
		if (!wsp->w_state&CMUX_IOCTL || wsp->w_ioctlchan != chan) {
#ifdef DEBUG1
			cmn_err(CE_WARN,"unexpected M_IOCDATA msg; freeing it");
#endif
			freemsg(mp);
			continue;
		}

		if (wsp->w_ioctllstrm == 0) 
			putnext(linkp->cmlb_lblk.l_qbot, mp);
		else {
			cmux_link_t *nlinkp;
			nlinkp = wsp->w_lstrmsp + wsp->w_ioctllstrm - 1;
			putnext(nlinkp->cmlb_lblk.l_qbot, mp);
		}

		continue;

	   case M_IOCTL: {

	   	/* we could not have gotten in here if
		 * an ioctl was in process, on this
		 * stream or any other (STREAMS protects
		 * against multiple ioctls on the same
		 * stream, and we protect against multiple
		 * ioctls on different streams
		 */

		struct iocblk *iocp;
		cmux_link_t *nlinkp;

		iocp = (struct iocblk *) mp->b_rptr;
		if (iocp->ioc_cmd == I_PUNLINK || iocp->ioc_cmd == I_UNLINK)
		   if (cmux_unlink(mp, cmuxp, iocp)) {
#ifdef DEBUG1
			cmn_err(CE_NOTE,"unlinking cmux");
#endif
			cmux_iocack(qp,mp,iocp,0);

			/* explicitly enable queue before returning so
			 * message processing can continue. We return
			 * rather than continue because we need to
			 * reset state
			 */
			qenable(qp);
			return;
		   }
#ifdef DEBUG1
		cmn_err(CE_WARN,"cmuxioctl: ioctl %x starting",iocp->ioc_cmd);
#endif
		oldpri = splstr();
		wsp->w_state |= CMUX_IOCTL;
		wsp->w_ioctlchan = chan;
		wsp->w_ioctlcnt = 1 + wsp->w_lstrms;
		splx(oldpri);

		/* ship copies of message to secondary streams
		 * adjust ioctlcnt so that if message copy
		 * fails, we aren't waiting for a response
		 * that will never come
		 */
		nlinkp = wsp->w_lstrmsp;
		for (i = 0; i < wsp->w_numlstrms; i++,nlinkp++) {
			if (!nlinkp->cmlb_flg) continue;
			nlinkp->cmlb_iocresp = 0;
			nlinkp->cmlb_iocmsg = copymsg(mp);
			if (nlinkp->cmlb_iocmsg)
		 		putnext(nlinkp->cmlb_lblk.l_qbot,nlinkp->cmlb_iocmsg);
			else
				wsp->w_ioctlcnt -= 1;
		}

		/* ship message to principal stream */
		nlinkp = wsp->w_princp + chan;
		nlinkp->cmlb_iocresp = 0;
		nlinkp->cmlb_iocmsg = copymsg(mp);
		if (nlinkp->cmlb_iocmsg)
			putnext(nlinkp->cmlb_lblk.l_qbot,nlinkp->cmlb_iocmsg);
		else
			wsp->w_ioctlcnt -= 1;
		wsp->w_iocmsg = mp;
		continue;
	   } /* M_IOCTL */			
			
	   default:
		if (mp->b_datap->db_type <= QPCTL && 
		    !canput(linkp->cmlb_lblk.l_qbot->q_next))
		{
			putbq(qp, mp);
			return;	/* read side is blocked */
		}

		putnext(linkp->cmlb_lblk.l_qbot, mp);
		continue;

	   } /* switch */
	} /* while */
}



int
cmux_up_rput(qp,mp)
queue_t *qp;
mblk_t *mp;
{
	/* should not be called */

	freemsg(mp);
#ifdef DEBUG1
	cmn_err(CE_WARN,"chanmux: up_rput called");
#endif
}


/*
 * if non-priority messages are put before a principal stream has 
 * been linked under, free them. For non-ioctl priority messages,
 * enqueue them, and for non-I_LINK/I_PLINK ioctls, NACK them.
 */

int
cmux_up_wput(qp,mp)
queue_t *qp;
mblk_t *mp;
{
	cmux_t *cmuxp;
	struct iocblk *iocp;
	cmux_ws_t *wsp;
	cmux_link_t *linkp,nlinkp;
	cmux_lstrm_t *lstrmp;
	int error,i;

	cmuxp = (cmux_t *) qp->q_ptr;
	if (cmuxp == (cmux_t *) NULL) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in up_wput");
#endif
		freemsg(mp);
		return;
	}

	wsp = cmuxp->cmux_wsp;
	linkp = wsp->w_princp + cmuxp->cmux_num;

	if (mp->b_datap->db_type < QPCTL && mp->b_datap->db_type != M_IOCTL) {
	   if (!linkp->cmlb_flg)
		freemsg(mp);
	   else
		putq(qp,mp);
	   return;
	}

	if (mp->b_datap->db_type != M_IOCTL)
	{
		putq(qp,mp);
		return;
	}

	iocp = (struct iocblk *) mp->b_rptr;

	if (iocp->ioc_cmd != I_LINK && iocp->ioc_cmd != I_PLINK) {
#ifdef DEBUG1
	   cmn_err(CE_NOTE,"up_wput: Have an ioctl");
#endif
	   if (!linkp->cmlb_flg)
		cmux_iocnak(qp,mp,iocp,EAGAIN,-1);
	   else
		putq(qp,mp);
	   return;
	}

	lstrmp = (cmux_lstrm_t *)kmem_alloc(sizeof(cmux_lstrm_t), KM_NOSLEEP);
	if (lstrmp == (cmux_lstrm_t *) NULL) {
		cmux_iocnak(qp,mp,iocp,EAGAIN,-1);
		return;
	}

	if (linkp->cmlb_flg) {
	   /* add secondary stream to set of lower streams */

	   if (error = cmux_allocstrms(wsp,++wsp->w_lstrms)) {
		wsp->w_lstrms--;
		kmem_free(lstrmp, sizeof(cmux_lstrm_t));
		cmux_iocnak(qp,mp,iocp,error,-1);
		return;
	   }

	   
	   linkp = wsp->w_lstrmsp;
	   for (i=0; i<wsp->w_numlstrms; i++,linkp++) 
		if (!linkp->cmlb_flg) break;

	   /* now i is the first free link_t struct found */
	   lstrmp->lstrm_wsp = wsp;
	   lstrmp->lstrm_flg = CMUX_SECSTRM;
	   lstrmp->lstrm_id = i; 

	   bcopy(mp->b_cont->b_rptr, &linkp->cmlb_lblk, sizeof(struct linkblk));
	   linkp->cmlb_flg = CMUX_SECSTRM;
	   linkp->cmlb_lblk.l_qbot->q_ptr = (caddr_t) lstrmp;
	   RD(linkp->cmlb_lblk.l_qbot)->q_ptr = (caddr_t) lstrmp;
	   cmux_iocack(qp,mp,iocp,0);
	   return;
	}

	lstrmp->lstrm_wsp = wsp;
	lstrmp->lstrm_flg = CMUX_PRINCSTRM;
	lstrmp->lstrm_id = cmuxp->cmux_num;
	bcopy(mp->b_cont->b_rptr, &linkp->cmlb_lblk, sizeof(struct linkblk));
	linkp->cmlb_flg = CMUX_PRINCSTRM;
	linkp->cmlb_lblk.l_qbot->q_ptr = (caddr_t ) lstrmp;
	RD(linkp->cmlb_lblk.l_qbot)->q_ptr = (caddr_t) lstrmp;
	cmux_iocack(qp,mp,iocp,0);
	/* enable processing on queue now that there is an active channel */
	qenable(qp); 
}

/*
 * This routine will be invoked by flow control when the queue below
 * it is enabled because a canput() from up_wsrv() on the queue failed
 * This routine, if invoked on a principal stream linked below,
 * will enable only the upper write queue associated with the
 * principal stream. If invoked on a secondary stream, any of the
 * upper streams above could be the culprit, so enable them all.
 */

int
cmux_mux_wsrv(qp)
queue_t *qp;
{
	cmux_lstrm_t *lstrmp;
	cmux_link_t *linkp;
	cmux_ws_t *wsp;
	cmux_t *cmuxp;
	unsigned long i;

	lstrmp = (cmux_lstrm_t *) qp->q_ptr;

	if (lstrmp == (cmux_lstrm_t *) NULL) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in mux_wsrv");
#endif
		return;
	}

	wsp = (cmux_ws_t *)lstrmp->lstrm_wsp;
	if (lstrmp->lstrm_flg & CMUX_PRINCSTRM) {
		cmuxp = wsp->w_cmuxpp[lstrmp->lstrm_id];
		if (cmuxp == NULL) {
#ifdef DEBUG1
		   cmn_err(CE_WARN,"chanmux: Invalid cmuxp in mux_wsrv");
#endif
		   return;
		}
		if (cmuxp->cmux_flg & CMUX_OPEN)
			qenable(cmuxp->cmux_wqp);
	}
	else {
		for (i=0; i<=wsp->w_numchan; i++) {

		   cmuxp = wsp->w_cmuxpp[i];
		   if ( (cmuxp == NULL) || !(cmuxp->cmux_flg & CMUX_OPEN))
			continue;
		   qenable(cmuxp->cmux_wqp);
		}
	}
}

#define IOCTL_TYPE(type)	((type==M_COPYIN)||(type==M_COPYOUT)||(type==M_IOCACK)||(type==M_IOCNAK))
/* 
 * cmux_mux_rsrv is the service routine of all input messages from the lower
 * streams. For normal messages from principal streams, forward
 * directly to the associated upper stream, if it exists, otherwise
 * discard the message. Normal messages from the lower streams should
 * be timestamped with an M_PROTO header for non-priority messages,
 * M_PCPROTO for priority messages. Send the message to the channel
 * that was active in the range given by the timestamp. If the
 * channel was closed, drop the message on the floor.
 *
 * If the message is ioctl-related, make note of the
 * response in the cmux_link_t structure for the lower stream
 * and update the count of waiting responses. When zero, check
 * all STREAMS. If exactly 1 ack (M_IOCACK, M_COPYIN, M_COPYOUT) was
 * sent up, we are in good shape. If more than one ack was sent up,
 * NACK the ioctl, and send M_IOCDATA messages to all lower streams
 * requesting M_COPYINs/M_COPYOUTs.
 *
 * For the switch channel command message from the principal stream, 
 * update the list of most recently active channels and its count.
 * Upon receipt of the "channel close acknowledge" message, 
 * wakeup the process sleeping in the close.
 */

int
cmux_mux_rsrv(qp)
queue_t *qp;
{
	cmux_lstrm_t *lstrmp;
	mblk_t *mp, *nmp;
	unsigned long princflg;
	time_t timestamp;
	cmux_t *cmuxp;
	ch_proto_t *protop;
	cmux_link_t *linkp;
	cmux_ws_t *wsp;

	lstrmp = (cmux_lstrm_t *) qp->q_ptr;
	if (lstrmp == (cmux_lstrm_t *) NULL) {
#ifdef DEBUG1
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in mux_rsrv");
#endif
		return;
	}

#ifdef DEBUG1
	if (!lstrmp->lstrm_flg) {
		cmn_err(CE_WARN,"chanmux: Invalid q_ptr in mux_rsrv");
		return;
	}
#endif
	wsp = lstrmp->lstrm_wsp;
	if (lstrmp->lstrm_flg & CMUX_PRINCSTRM) {
		princflg = 1;
		linkp = wsp->w_princp + lstrmp->lstrm_id;
	}
	else {
		princflg = 0;
		linkp = wsp->w_lstrmsp + lstrmp->lstrm_id;
	}
	
	while ((mp = getq(qp)) != NULL) {
	   if (IOCTL_TYPE(mp->b_datap->db_type))
		goto msgproc;

	   if (!princflg) {
		/* message is from lower stream and should have
		 * header indicating timestamp.
		 */
		if ( (mp->b_wptr - mp->b_rptr) != sizeof(ch_proto_t)) {
#ifdef DEBUG1
	cmn_err(CE_WARN,"chanmux: illegal lower stream protocol in mux_rsrv");
#endif
			freemsg(mp);
			continue;
		}
		timestamp = cmux_striptime(mp);
		cmuxp = cmux_findchan(wsp,timestamp);
		if (cmuxp == (cmux_t *) NULL) {
#ifdef DEBUG1
	cmn_err(CE_WARN,"chanmux: illegal cmuxp found in mux_rsrv");
#endif
			freemsg(mp);
			continue;
		}
		if (mp->b_datap->db_type < QPCTL && !canput(cmuxp->cmux_rqp->q_next)) {
			putbq(qp,mp);
			return;
		}


	   }
	   else {
		cmuxp = wsp->w_cmuxpp[lstrmp->lstrm_id];
		if (cmuxp == (cmux_t *) NULL) {
#ifdef DEBUG1
			cmn_err(CE_NOTE,"did not find cmuxp; id %d %x",lstrmp->lstrm_id, wsp->w_cmuxpp);
#endif
			freemsg(mp);

			continue;
	 	}
		if (mp->b_datap->db_type < QPCTL && !canput(cmuxp->cmux_rqp->q_next)) {
			putbq(qp,mp);
			return;
		}
	   }


msgproc:   switch (mp->b_datap->db_type) {

	   case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			flushq(qp, FLUSHDATA);
			*mp->b_rptr &= ~FLUSHR;
		}
		if (*mp->b_rptr & FLUSHW) {
			/* nothing to flush on the lower write side */
			qreply(qp,mp);
		}
		else
			freemsg(mp);
		continue;

/* ioctl handling. differentiate between waiting for ack and
 * received ack. This treats M_COPYIN/M_COPYOUT messages differently
 */
	   case M_IOCACK:
#ifdef DEBUG1
		cmn_err(CE_NOTE,"Found M_IOCACK on queue in rsrv");
#endif
		if (wsp->w_ioctlcnt) {
			linkp->cmlb_iocresp = M_IOCACK;
			linkp->cmlb_iocmsg = mp;
#ifdef DEBUG1
		cmn_err(CE_NOTE,"ioctlcnt > 0");
#endif
			if (--wsp->w_ioctlcnt == 0)
				cmux_do_iocresp(wsp);
			continue;
		}
		/* ioctlcnt == 0 means that this messages
		 * comes after a M_COPYIN/M_COPYOUT.
		 * In this, send the message up to
		 * the next read queue
		 */

		cmuxp = wsp->w_cmuxpp[wsp->w_ioctlchan];
		putnext(cmuxp->cmux_rqp,mp);
		cmux_clr_ioc(wsp);
		continue;
	   
	   case M_IOCNAK:
#ifdef DEBUG1
		cmn_err(CE_NOTE,"Found M_IOCNAK on queue in rsrv");
#endif
		if (wsp->w_ioctlcnt) {
			linkp->cmlb_iocresp = M_IOCNAK;
			linkp->cmlb_iocmsg = mp;
			if (--wsp->w_ioctlcnt == 0)
				cmux_do_iocresp(wsp);
			continue;
		}
		/* ioctlcnt == 0 means that this message
		 * comes after M_COPYIN/M_COPYOUT.
		 * Send the message up to the next read queue
		 */

		cmuxp = wsp->w_cmuxpp[wsp->w_ioctlchan];
		putnext(cmuxp->cmux_rqp,mp);
		cmux_clr_ioc(wsp);
		continue;


	   case M_COPYIN:
		if (wsp->w_ioctlcnt) {
			linkp->cmlb_iocresp = M_IOCACK;
			linkp->cmlb_iocmsg = mp;
			if (--wsp->w_ioctlcnt == 0)
				cmux_do_iocresp(wsp);
			continue;
		}
		/* ioctlcnt == 0 means that this message
		 * comes after another M_COPYIN/M_COPYOUT.
		 * Send the message up to the next read queue
		 */

		cmuxp = wsp->w_cmuxpp[wsp->w_ioctlchan];
		putnext(cmuxp->cmux_rqp,mp);
		continue;

	   case M_COPYOUT:
		if (wsp->w_ioctlcnt) {
			linkp->cmlb_iocresp = M_IOCACK;
			linkp->cmlb_iocmsg = mp;
			if (--wsp->w_ioctlcnt == 0)
				cmux_do_iocresp(wsp);
			continue;
		}
		/* ioctlcnt == 0 means that this message
		 * comes after another M_COPYIN/M_COPYOUT.
		 * Send the message up to the next read queue
		 */

		cmuxp = wsp->w_cmuxpp[wsp->w_ioctlchan];
		putnext(cmuxp->cmux_rqp,mp);
		continue;

	   case M_PCPROTO:
	   case M_PROTO:
		/* close acknowledgement, switch channel */

		if ( (mp->b_wptr - mp->b_rptr) != sizeof(ch_proto_t))
			putnext(cmuxp->cmux_rqp,mp);
		protop = (ch_proto_t *) mp->b_rptr;
		if ( princflg && (protop->chp_type != CH_CTL ||
		     protop->chp_stype != CH_PRINC_STRM))
			putnext(cmuxp->cmux_rqp,mp);

		else if (princflg)
		/* potentially a command for us */
		   switch (protop->chp_stype_cmd) {

		   case CH_CHANGE_CHAN:
			cmux_switch_chan(wsp,protop);
			freemsg(mp); /* free msg */
			continue;

		   case CH_OPEN_RESP:
			linkp->cmlb_err = protop->chp_stype_arg;
			freemsg(mp);
			if (linkp->cmlb_flg & CMUX_PRINCSLEEP) {
				wakeup(&cmuxp->cmux_flg);
				return; /* q will be enabled */
			}
			continue; /* only if we were not sleeping in open */

		   case CH_CLOSE_ACK:
#ifdef DEBUG1
			cmn_err(CE_WARN,"Found close_ack");
#endif
			cmux_close_chan(wsp,lstrmp,protop);
			continue;

		   default:
			putnext(cmuxp->cmux_rqp,mp);
			continue;
		   } /* switch */

		else /* no CH_* protocol with lower streams */
			putnext(cmuxp->cmux_rqp,mp);
		   continue;	

	   default: 
		putnext(cmuxp->cmux_rqp,mp);
		continue;

	   } /* switch */
	} /* while */
}


int
cmux_mux_rput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
	putq(qp,mp);
}


int
cmux_mux_wput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
	/* should not be called */

	freemsg(mp);
#ifdef DEBUG1
	cmn_err(CE_WARN,"chanmux: mux_wput called");
#endif
}
