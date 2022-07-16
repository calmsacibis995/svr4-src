/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:char.c	1.3.3.2"

/*
 * IWE CHAR module; scan-code translation and screen-mapping
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/termios.h>
#include <sys/strtty.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <sys/kmem.h>
#include <sys/ascii.h>
#include <sys/vt.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include <sys/ws/tcl.h>
#include <sys/proc.h>
#include <sys/tss.h>
#include <sys/xque.h>
#include <sys/ws/ws.h>
#include <sys/ws/chan.h>
#include <sys/mouse.h>
#include <sys/char.h>
#include <sys/v86.h>
#include <sys/ddi.h>

int chardevflag = 0;

int chr_open(/*queue_t *q, dev_t * devp, int oflag, int sflag*/);
int chr_close(/*queue_t *q*/);
int chr_read_queue_put(/*queue_t *q, mblk_t *mp*/);
int chr_read_queue_serv(/*queue_t *q*/);
int chr_write_queue_put(/*queue_t *q, mblk_t *mp*/);

void chr_r_shipup();
void chr_do_iocdata();
void chr_do_ioctl();
void chr_scan();

int chr_maxmouse;

int chr_debug = 0;

#define	DEBUG1(a)	if (chr_debug == 1) printf a
#define	DEBUG2(a)	if (chr_debug >= 2) printf a /* allocations */
#define	DEBUG3(a)	if (chr_debug >= 3) printf a /* M_CTL Stuff */
#define	DEBUG4(a)	if (chr_debug >= 4) printf a /* M_READ Stuff */
#define	DEBUG5(a)	if (chr_debug >= 5) printf a
#define	DEBUG6(a)	if (chr_debug >= 6) printf a

static struct module_info chr_iinfo = {
	0,
	"char",
	0,
	MAXCHARPSZ,
	1000,
	100
};

static struct qinit chr_rinit = {
	chr_read_queue_put, 
	chr_read_queue_serv, 
	chr_open,
	chr_close,
	NULL,
	&chr_iinfo
};

static struct module_info chr_oinfo = {
	0,
	"char",
	0,
	MAXCHARPSZ,
	1000,
	100
};

static struct qinit chr_winit = {
	chr_write_queue_put,
	NULL,
	chr_open,
	chr_close,
	NULL,
	&chr_oinfo
};

struct streamtab charinfo = {
	&chr_rinit,
	&chr_winit,
	NULL,
	NULL
};


extern int ws_newkeymap();

/* initialize per-stream state structure for CHAR. Return 0 for
 * success, error number for failure
 */

int
chr_stat_init(qp,cp)
queue_t *qp;
register charstat_t *cp;
{
	register int oldpri;
	struct mouseinfo *minfop; /* just used for sizing */

	chr_maxmouse = (1 << (8*sizeof(minfop->xmotion)-1) ) - 1;
	oldpri = splhi(); /* protect against multiple opens */

	/* if q_ptr already set, we've allocated a struct already */
	if (qp->q_ptr != (caddr_t) NULL)
	{
		splx(oldpri);
		kmem_free ((caddr_t) cp, sizeof(charstat_t));
		return (0); /* not failure -- just simultaneous open */
	}

	/* set ptrs in queues to point to state structure */
	qp->q_ptr = (caddr_t) cp; 
	WR(qp)->q_ptr = (caddr_t) cp;
	splx(oldpri);


	cp->c_rqp = qp;
	cp->c_wqp = WR(qp);
	cp->c_wmsg = (mblk_t *) NULL;
	cp->c_rmsg = (mblk_t *) NULL;
	cp->c_state = 0;
	cp->c_map_p = (charmap_t *) NULL;
	cp->c_scrmap_p = NULL;
	cp->c_oldbutton = 0x07;		/* Initially all buttons up */
	return (0);
}


/* release cp and associated allocated data structures */

void
chr_free_stat(cp)
register charstat_t *cp;
{
	if (cp->c_rmsg != (mblk_t *) NULL) {
		freemsg(cp->c_rmsg);
		cp->c_rmsg = (mblk_t *) NULL;
	}

	if (cp->c_wmsg != (mblk_t *) NULL) {
		freemsg(cp->c_wmsg);
		cp->c_wmsg = (mblk_t *) NULL;
	}
	if (cp->c_heldmseread != (mblk_t *) NULL) {
		freemsg(cp->c_heldmseread);
		cp->c_heldmseread = (mblk_t *) NULL;
	}

	kmem_free(cp, sizeof(charstat_t));
}


/*
 * Character module open. Allocate state structure and
 * send pointer to pointer to character map structure
 * to principal stream below in the form of a M_PCPROTO
 * message. Return 0 for sucess, an errno for failure.
 */

int wakeup();

/*ARGSUSED*/
int
chr_open(qp, devp, oflag, sflag)
	queue_t *qp;
	dev_t *devp;
	int oflag, sflag;
{
	charstat_t *cp;
	mblk_t *mp;
	struct ch_protocol *protop;
	int oldpri;
	int error = 0;


	if (qp->q_ptr != NULL)
		return (0);		/* already attached */

	/* allocate and initialize state structure */
	if ((cp = (charstat_t *) kmem_alloc( sizeof(charstat_t), KM_SLEEP)) == NULL) {
		cmn_err(CE_WARN, "CHAR: open fails, can't allocate state structure\n");
		return (ENOMEM);
	}

	bzero(cp, sizeof(charstat_t));

	error = chr_stat_init(qp,cp); /* initialize state structure */
	return (error);
}


/* Release state structure associated with stream */

int 
chr_close(qp)
	register queue_t *qp;
{
	charstat_t *cp = (charstat_t *)qp->q_ptr;
	register mblk_t *bp;
	int oldpri;

	flushq(qp,FLUSHDATA);
	oldpri=splstr();

	/* Dump the associated state structure */
	chr_free_stat(cp);
	qp->q_ptr = NULL;

	splx(oldpri);
}

/*
 * Put procedure for input from driver end of stream (read queue).
 */


void chr_proc_r_data();
void chr_proc_r_proto();
void chr_proc_r_pcproto();
void chr_proc_w_data();

int
chr_read_queue_put(qp, mp)
	queue_t *qp;
	mblk_t *mp;
{
	putq(qp,mp); /* enqueue this pup */
}

/*
 * Read side queued message processing.  
 */

int
chr_read_queue_serv(qp)
	register queue_t *qp;
{
	register charstat_t *cp;
	mblk_t *mp;
	int oldpri;

	cp = (charstat_t *)qp->q_ptr;

	/* keep getting messages until none left or we honor
	 * flow control and see that the stream above us is blocked
	 * or are set to enqueue messages while an ioctl is processed
	 */

	while ((mp = getq(qp)) != NULL) {
	   if (mp->b_datap->db_type <= QPCTL && !canput(qp->q_next))
	   {
		putbq(qp, mp);
		return;	/* read side is blocked */
	   }

	   if (cp->c_state & C_XQUEMDE) {
		if (!cp->c_xqinfo ||
		    !validproc(cp->c_xqinfo->xq_proc,cp->c_xqinfo->xq_pid)) {
			cp->c_state &= ~C_XQUEMDE;
			cp->c_xqinfo = NULL;
	   	}
	   }

	   if (cp->c_state & C_RAWMODE) {
	   	if (!validproc(cp->c_rawprocp,cp->c_rawpid)) {
			cp->c_state &= ~C_RAWMODE;
			cp->c_rawprocp = NULL;
			cp->c_rawpid = 0;
	   	}
	   }

	   switch(mp->b_datap->db_type) {

		default:
			putnext(qp, mp);	/* pass it on */
			continue;

		case M_FLUSH:
			/*
		 	* Flush everything we haven't looked at yet.
		 	*/
			flushq(qp, FLUSHDATA);
			putnext(qp, mp); /* pass it on */
			continue;

		case M_DATA:
			chr_proc_r_data(qp,mp,cp);
			continue;

		case M_PCPROTO:
		case M_PROTO:
			chr_proc_r_proto(qp,mp,cp);
			continue;

	   } /* switch */
	} /* while */
}


/* Currently, CHAR understands messages from KDSTR, MOUSE and
 * MERGE386.
 */

void chr_do_xmouse();
void chr_do_mouseinfo();

void
chr_proc_r_proto(qp,mp,cp)
	register queue_t *qp;
	register mblk_t *mp;
	charstat_t *cp;
{
	ch_proto_t *protop;
	int i;

	if ( (mp->b_wptr - mp->b_rptr) != sizeof(ch_proto_t)) {
		putnext(qp,mp);
		return;
	}

	protop = (ch_proto_t *) mp->b_rptr;

	switch (protop->chp_type) {
	
	case CH_CTL: {

	    switch (protop->chp_stype) {
	    
	    case CH_CHR:
		switch (protop->chp_stype_cmd) {

		case CH_LEDSTATE:
		   i = cp->c_kbstat.kb_state;
		   cp->c_kbstat.kb_state = (i & NONTOGGLES) | protop->chp_stype_arg;
		   break;

		case CH_CHRMAP:
		   cp->c_map_p = (charmap_t *)protop->chp_stype_arg;
		   break;

		case CH_SCRMAP:
		   cp->c_scrmap_p = (scrn_t *)protop->chp_stype_arg;
		   break;
#ifdef MERGE386
		case CH_SETMVPI: {
		   chr_merge_t *mergep;

		   mergep = (mp->b_cont) ? (chr_merge_t *) mp->b_cont->b_rptr
					 : (chr_merge_t *)NULL;
		   if (mergep == (chr_merge_t *) NULL ||
		      (mp->b_cont->b_wptr - (unchar *)mergep) != sizeof(chr_merge_t)) {
			cmn_err(CE_WARN,"char: Found CH_SETMVPI with invalid arg");
			break;
		   }
		   cp->c_merge_kbd_ppi = mergep->merge_kbd_ppi;
		   cp->c_merge_mse_ppi = mergep->merge_mse_ppi;
		   cp->c_merge_mcon = mergep->merge_mcon;
		   break;
		}

		case CH_DELMVPI: 
		   cp->c_merge_kbd_ppi = NULL;
		   cp->c_merge_mse_ppi = NULL;
		   cp->c_merge_mcon = NULL;
		   break;
#endif /* MERGE386 */
		default:
		   putnext(qp,mp);
		   return;
		}

		freemsg(mp);
		return; /* case CH_CHR */

	    case CH_XQ:
		if (protop->chp_stype_cmd == CH_XQENAB) {
			cp->c_xqinfo = (xqInfo *) protop->chp_stype_arg;
			cp->c_state |= C_XQUEMDE;
			protop->chp_stype_cmd = CH_XQENAB_ACK;
			mp->b_datap->db_type = M_PCPROTO;
			qreply(qp,mp);
			return;
		}

		if (protop->chp_stype_cmd == CH_XQDISAB) {
			cp->c_xqinfo = (xqInfo *) NULL;
			cp->c_state &= ~C_XQUEMDE;
			protop->chp_stype_cmd = CH_XQDISAB_ACK;
			mp->b_datap->db_type = M_PCPROTO;
			qreply(qp,mp);
			return;
		}
		putnext(qp,mp);
		return;

	    default:
		putnext(qp,mp);
	    } 
	    break;
	}

	case CH_DATA:
	   switch (protop->chp_stype) {

	   case CH_MSE:
		if (cp->c_state & C_XQUEMDE)
			chr_do_xmouse(qp,mp,cp);
#ifdef VPIX
		else if ( (cp->c_state & C_RAWMODE) && cp->c_stashed_p_v86) 
			v86setint(cp->c_stashed_p_v86,V86VI_MOUSE);
#endif /* VPIX */
#ifdef MERGE386
		else if (cp->c_merge_mse_ppi)
			(*(cp->c_merge_mse_ppi))(
				(struct mse_event *)mp->b_cont->b_rptr,
				cp->c_merge_mcon);
#endif /* MERGE386 */
		chr_do_mouseinfo(qp,mp,cp);
		freemsg(mp);
		return;

	   case CH_NOSCAN: /* send up b_cont message to LDTERM directly */
		if (mp->b_cont) 
			putnext(qp,mp->b_cont);
		freeb(mp);
		return;

	   default:
#ifdef DEBUG
		cmn_err(CE_NOTE,"char: Found unknown CH_DATA message in input");
#endif
		putnext(qp,mp);
		break;
	   }
	   return;

	default:
		putnext(qp,mp);
		return;
	}
}


/* Treat each byte of the message as a scan code to be translated. */

void
chr_proc_r_data(qp, mp,cp)
	register queue_t *qp;
	register mblk_t *mp;
	register charstat_t *cp;
{

	register mblk_t *bp;
	unsigned char scan;
	int israw;

	if (cp->c_map_p == (charmap_t *) NULL) {
		freemsg(mp);
		return;
	}

	bp = mp;
	israw = cp->c_state & (C_RAWMODE | C_XQUEMDE);

	/* for each data block, take the buffered bytes and pass them
	 * to chr_scan; it will translate them and put them in a
	 * message that we send up when when we're through
	 * with this message
	 */

	while (bp) {
	   while ( (unsigned) bp->b_rptr < (unsigned) bp->b_wptr)
		chr_scan( cp, *bp->b_rptr++, israw );
	   bp = bp->b_cont;
	} 

	freemsg(mp); /* free the scanned message */

	/* send up the message we stored at c_rmsg */
	if (cp->c_rmsg != (mblk_t *) NULL)
	{
		putnext(qp, cp->c_rmsg);
		cp->c_rmsg = (mblk_t *) NULL;
	}
}



void chr_r_charin();

/*
 * Translate the rawscan code to its mapped counterpart,
 * using the provided WS function ws_scanchar().
 */

void
chr_scan(cp, rawscan,israw)
register charstat_t *cp;
unsigned char rawscan;
int israw;
{
	charmap_t *cmp;		/* char map pointer */
	kbstate_t *kbstatp;	/* ptr to kboard state */
	queue_t	*qp;
	ushort ch;
	mblk_t *mp;
	int cnt;
	unchar *strp, *sp;
	unchar	lbuf[3],	/* Buffer for escape sequences */
		str[4];		/* string buffer */

	pfxstate_t *pfxstrp;
	strmap_t *strbufp;
	stridx_t *strmap_p;

	cmp = cp->c_map_p;
	kbstatp = &cp->c_kbstat;
	pfxstrp = cmp->cr_pfxstrp;
	strbufp = cmp->cr_strbufp;
	strmap_p = cmp->cr_strmap_p;

	/* translate the character */
	ch = ws_scanchar(cmp, kbstatp, rawscan, israw);

	if (ch & NO_CHAR)
		return;

	if (!israw) 
		switch (ch) {
		case K_SLK:	/* CTRL-F generates K_SLK */
			if ((rawscan == SCROLLLOCK) || (rawscan == 0x45)) {
				ch = (cp->c_state & C_FLOWON) ? CSTART: CSTOP;
			}
			break;

		case K_BRK:
			if (rawscan == 0x46 || ws_specialkey(cmp->cr_keymap_p, kbstatp, rawscan)) {
				chr_r_charin(cp, strp, 0, 1);	/* flush */
				putctl(cp->c_rqp->q_next, M_BREAK);
				return;
			}
			break;

		default:
			break;
		}

	strp = &lbuf[0];

	if (cp->c_state & C_XQUEMDE) {	/* just send the event */
		lbuf[0] = ch;
		chr_r_charin(cp, lbuf, 1, 0);
		return;
	}

	if (ch & GEN_ESCLSB) {		/* Prepend character with "<ESC>["? */
		lbuf[0] = 033;		/* Prepend <ESC> */
		lbuf[1] = '[';		/* Prepend '[' */
		lbuf[2] = ch;		/* Add character */
		cnt = 3;		/* Three characters in buffer */
	}

	else if (ch & GEN_ESCN) {	/* Prepend character with "<ESC>N"? */
		lbuf[0] = 033;		/* Prepend <ESC> */
		lbuf[1] = 'N';		/* Prepend 'N' */
		lbuf[2] = ch;		/* Add character */
		cnt = 3;		/* Three characters in buffer */
	}

	else if (ch & GEN_ZERO) {	/* Prepend character with 0? */
		lbuf[0] = 0;		/* Prepend 0 */
		lbuf[1] = ch;		/* Add character */
		cnt = 2;		/* Two characters in buffer */
	}

	else if (ch & GEN_FUNC) {	/* Function key? */
		if ((int)(ch & 0xff) >= (int)K_PFXF && (ushort)(ch & 0xff) <= (ushort)K_PFXL){
			ushort pfx, val;
			struct pfxstate *pfxp;

			str[0] = '\033';
			pfxp = (struct pfxstate *) pfxstrp;
			pfxp += (ch & 0xff) - K_PFXF; 
			val = pfxp->val;
			switch(pfxp->type){
				case K_ESN:
					str[1] = 'N';
					break;
				case K_ESO:
					str[1] = 'O';
					break;
				case K_ESL:
					str[1] = '[';
					break;
			}
			str[2] = (unchar)val;
			strp = &str[0];
			cnt = 3;
		}
		else {
			/* Start of string */
			ushort idx, *ptr;

			ptr = (ushort *) strmap_p;

			idx = * (ptr + (ch&0xff) - K_FUNF);
			strp = ((unchar *) strbufp) + idx;

			/* Count characters in string */
			for (cnt = 0, sp = strp; *sp != '\0'; cnt++, sp++);
		}
	}

	else {  /* Nothing special about character */
		lbuf[0] = ch;		/* Put character in buffer */
		cnt = 1;		/* Only one character */
	}


	chr_r_charin(cp, strp, cnt, 0);	/* Put characters in data message */
}


/*
 * Stuff the characters pointed to by bufp in the message allocated for
 * shipping upstream if normal operation. VP/ix and MERGE386 hooks will most
 * likely be handled here, too, to some extent, as well as the X-queue.
 */

void
chr_r_charin(cp, bufp, cnt, flush)
charstat_t *cp;
char *bufp;
int cnt, flush;
{
	mblk_t *mp;
	int oldpri;

	if ( flush || (cp->c_rmsg == (mblk_t *) NULL) ||
	        (cp->c_rmsg->b_wptr >= (cp->c_rmsg->b_datap->db_lim - cnt)) )
	{
	      if (cp->c_rmsg) putnext(cp->c_rqp, cp->c_rmsg);
	      cp->c_rmsg = (mblk_t *) NULL;

	      if ((mp = allocb( max(CHARPSZ,cnt), BPRI_MED)) == NULL)
	      {
		   cmn_err(CE_WARN,
		        "char: chr_scan; cannot allocate message, dropping input data");
		   return;
	      }

	      cp->c_rmsg = mp;
	} 

	/*
	 * If we're in queue mode, just send the event.
	 */
	if (cp->c_state & C_XQUEMDE) {
		cp->c_xevent.xq_type = XQ_KEY;
		while (cnt-- != 0) {
			cp->c_xevent.xq_code = *bufp++;
			xq_enqueue(cp->c_xqinfo, &cp->c_xevent); 
		}
		return;
	}
#ifdef VPIX
	else if ( cnt && (cp->c_state & C_RAWMODE) && cp->c_stashed_p_v86) 
			v86setint(cp->c_stashed_p_v86,V86VI_KBD);
#endif /* VPIX */
#ifdef MERGE386
	else if (cp->c_merge_kbd_ppi) {
		while (cnt-- != 0) 
			(*(cp->c_merge_kbd_ppi))(*bufp++, cp->c_merge_mcon);
		return;
	}
#endif /* MERGE386 */
	   /*
	    * Add the characters to end of read message.
	    */
   
	while (cnt-- != 0) 
		*cp->c_rmsg->b_wptr++ = *bufp++;
}



/*
 * Char module output queue put procedure.
 */
chr_write_queue_put(qp, mp)
	register queue_t *qp;
	register mblk_t *mp;
{
	register charstat_t *cp;
	mblk_t *bpt;

	cp = (charstat_t *)qp->q_ptr;

	switch (mp->b_datap->db_type) {

	case M_FLUSH:
		/*
		 * This is coming from above, so we only handle the write
		 * queue here.  If FLUSHR is set, it will get turned around
		 * at the driver, and the read procedure will see it
		 * eventually.
		 */
		if (*mp->b_rptr & FLUSHW)
			flushq(qp, FLUSHDATA);
		putnext(qp, mp);
		break;

	case M_IOCTL:
		chr_do_ioctl(qp, mp, cp);
		break;

	case M_IOCDATA:
		chr_do_iocdata(qp, mp, cp);
		break;

	case M_DATA:
		chr_proc_w_data(qp,mp,cp);
		break;

	case M_PCPROTO:
	case M_PROTO:
		chr_proc_w_proto(qp,mp,cp);
		break;

	default:
		putnext(qp, mp);	/* pass it through unmolested */
		break;
	}
}


void
chr_iocack(qp, mp, iocp, rval)
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
		freemsg(tmp);
	qreply(qp,mp);
}

void
chr_iocnack(qp, mp, iocp, error, rval)
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

void
chr_copyout(qp, mp, nmp, size, state)
queue_t *qp;
register mblk_t *mp, *nmp;
uint size;
unsigned long state;
{
	register struct copyreq *cqp;
	copy_state_t *copyp;
	charstat_t *cp;

	cp = (charstat_t *) qp->q_ptr;
	copyp = &(cp->c_copystate);

	cqp = (struct copyreq *) mp->b_rptr;
	cqp->cq_size = size;
	cqp->cq_addr = (caddr_t) * (long *) mp->b_cont->b_rptr;
	cqp->cq_flag = 0;
	cqp->cq_private = (mblk_t *)copyp;

	copyp->cpy_arg = (unsigned long) cqp->cq_addr;
	copyp->cpy_state = state;

	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	mp->b_datap->db_type = M_COPYOUT;

	if (mp->b_cont) freemsg(mp->b_cont);
	mp->b_cont = nmp;

	qreply(qp, mp);
}


void
chr_copyin(qp, mp, size, state)
queue_t *qp;
register mblk_t *mp;
int size;
unsigned long state;
{
	register struct copyreq *cqp;
	copy_state_t *copyp;
	charstat_t *cp;

	cp = (charstat_t *) qp->q_ptr;
	copyp = &cp->c_copystate;

	cqp = (struct copyreq *) mp->b_rptr;
	cqp->cq_size = size;
	cqp->cq_addr = (caddr_t) * (long *) mp->b_cont->b_rptr;
	cqp->cq_flag = 0;
	cqp->cq_private = (mblk_t *)copyp;

	copyp->cpy_arg = (unsigned long) cqp->cq_addr;
	copyp->cpy_state = state;
	
	if (mp->b_cont) freemsg(mp->b_cont);

	mp->b_datap->db_type = M_COPYIN;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);

	qreply(qp, mp);
}


/*
 * Called when an M_IOCTL message is seen on the write queue; does whatever
 * we're supposed to do with it, and either replies immediately or passes it
 * to the next module down.
 */

void
chr_do_ioctl(qp, mp,cp)
	queue_t *qp;
	register mblk_t *mp;
	register charstat_t *cp;
{
	register struct iocblk *iocp;
	int transparent;
	struct copyreq *cqp;
	struct copyresp *csp;
	mblk_t *nmp;
	ch_proto_t *protop;


	iocp = (struct iocblk *)mp->b_rptr;

	transparent = (iocp->ioc_count == TRANSPARENT);

	switch (iocp->ioc_cmd) {

	case MOUSEIOCDELAY: {
		int oldpri;
	
		oldpri = splstr();
		cp->c_state |= C_MSEBLK;
		splx(oldpri);
		chr_iocack(qp,mp,iocp,0);
		break;
	}

	case MOUSEIOCNDELAY: {
		int oldpri;
	
		oldpri = splstr();
		cp->c_state &= ~C_MSEBLK;
		splx(oldpri);
		chr_iocack(qp,mp,iocp,0);
		break;
	}

	case MOUSEIOCREAD: {
		mblk_t *bp;
		int oldpri;
		struct mouseinfo *minfop;

		oldpri = splstr();
		if ((!(cp->c_state & C_MSEINPUT)) && (cp->c_state & C_MSEBLK)) {
			cp->c_heldmseread = mp;
			splx(oldpri);
			return;
		}
		splx(oldpri);
		if((bp = allocb(sizeof(struct mouseinfo),BPRI_MED)) == NULL){ 
			chr_iocnack(qp, mp, iocp, EAGAIN, 0);
			break;
		}
		minfop = &cp->c_mouseinfo;
		oldpri = splstr();
		bcopy(minfop,bp->b_rptr,sizeof(struct mouseinfo));
		minfop->xmotion = minfop->ymotion = 0;
		minfop->status &= BUTSTATMASK;
		cp->c_state &= ~C_MSEINPUT;
		splx(oldpri);
		bp->b_wptr += sizeof(struct mouseinfo);
		if(transparent)
			chr_copyout(qp, mp, bp, sizeof(struct mouseinfo), 0);
		else {
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = sizeof(struct mouseinfo);
			if (mp->b_cont) freemsg(mp->b_cont);
			mp->b_cont = bp;
			qreply(qp, mp);
		}
		break;
	}

	case TIOCSTI: { /* Simulate typing of a character at the terminal. */
		register mblk_t *bp;

		/*
		 * The permission checking has already been done at the stream
		 * head, since it has to be done in the context of the process
		 * doing the call. Special processing was done at STREAM head.
		 */

		if ((bp = allocb(1, BPRI_MED)) != NULL) {
			if ((nmp=allocb(sizeof(ch_proto_t),BPRI_MED)) == NULL) {
				freemsg(bp);
				bp = NULL;
			} else {
				*bp->b_wptr++ = *mp->b_cont->b_rptr++;
				nmp->b_datap->db_type = M_PROTO;
				protop = (ch_proto_t *)nmp->b_rptr;
				nmp->b_wptr += sizeof(ch_proto_t);
				protop->chp_type = CH_DATA;
				protop->chp_stype = CH_NOSCAN;
				nmp->b_cont = bp;
				putq(cp->c_rqp,nmp);
			}
		}
		if (bp)
		   chr_iocack(qp,mp,iocp,0);
		else 
		   chr_iocnack(qp, mp, iocp, ENOMEM, 0);
		
		break;
	}

	case KBENABLED: {
		kbstate_t *kbp;

		if (transparent) {
		   kbp = &cp->c_kbstat;
		   chr_iocack(qp,mp,iocp,kbp->kb_extkey);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;
	}

	case TIOCKBOF: {
		kbstate_t *kbp;

		if (transparent) {
		   kbp = &cp->c_kbstat;
		   kbp->kb_extkey = 0;
		   chr_iocack(qp,mp,iocp,0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;
	}

	case TIOCKBON: {
		kbstate_t *kbp;

		if (transparent) {
		   kbp = &cp->c_kbstat;
		   kbp->kb_extkey = 1;
		   chr_iocack(qp,mp,iocp,0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;
	}

	case KDGKBENT:
		if (transparent)
		   chr_copyin(qp,mp,sizeof(struct kbentry),CHR_IN_0);
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;
		
	case KDSKBENT: 
		if (transparent) {
		   chr_copyin(qp,mp,sizeof(struct kbentry),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case KDGKBMODE: {
		unsigned char val;
		mblk_t *nmp;

		if (!transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}


		if ( (nmp = allocb(sizeof(val), BPRI_MED)) == NULL)
		{
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		val = (cp->c_state & C_RAWMODE) ? K_RAW : K_XLATE;
		bcopy(&val, nmp->b_rptr, sizeof(val));
		nmp->b_wptr += sizeof(val);

		chr_copyout(qp,mp,nmp,sizeof(val),CHR_OUT_0);
		break;
	   }
		
	case KDSKBMODE: {
		int arg,oldpri;
		struct v86blk *v86p; 

		if (!transparent) {
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);
		   break;
		}

		arg = * (int *)mp->b_cont->b_cont->b_rptr;
		v86p = (struct v86blk *) mp->b_cont->b_rptr; 

		if ( (arg != K_RAW) && (arg != K_XLATE) ) {
		   chr_iocnack(qp, mp, iocp, ENXIO, 0);
		   break;
		}

		oldpri = splstr();
		if (arg == K_RAW) {
			cp->c_state |= C_RAWMODE;
			cp->c_stashed_p_v86 = v86p->v86_p_v86;
			cp->c_rawprocp = v86p->v86_u_procp;
			cp->c_rawpid = v86p->v86_p_pid;
		}
		else {
			cp->c_state &= ~C_RAWMODE;
			cp->c_stashed_p_v86 = 0;
			cp->c_rawprocp = 0;
			cp->c_rawpid = 0;
		}
		splx(oldpri);
		chr_iocack(qp,mp,iocp,0);
		break;
	}


	case SETFKEY: 
		if (transparent) {
		   chr_copyin(qp,mp,sizeof(struct fkeyarg),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case GETFKEY: 
		if (transparent)
		   chr_copyin(qp,mp,sizeof(struct fkeyarg),CHR_IN_0);
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case KDDFLTKEYMAP:
		if (transparent) {
		   chr_copyin(qp,mp,sizeof(struct key_dflt),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case KDDFLTSTRMAP:
		if (transparent) {
		   chr_copyin(qp,mp,sizeof(struct str_dflt),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case KDDFLTSCRNMAP:
		if (transparent) {
		   chr_copyin(qp,mp,sizeof(struct scrn_dflt),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;


	case GIO_KEYMAP: {
		int size;
		charmap_t *cmp;
		mblk_t *nmp;
		keymap_t *kmp;

		if (!transparent) {
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);
		   break;
		}

		cmp = cp->c_map_p;

		size = cmp->cr_keymap_p->n_keys;
		size *= sizeof(cmp->cr_keymap_p->key[0]);
		size += sizeof(cmp->cr_keymap_p->n_keys);

		if ( (nmp = allocb(size,BPRI_MED)) == (mblk_t *)NULL) {
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		kmp = (keymap_t *) nmp->b_rptr;
		bcopy(cmp->cr_keymap_p, kmp, size);
		nmp->b_wptr += size;
		chr_copyout(qp,mp,nmp,size,CHR_OUT_0);
		break;
	   }	
		
		
	
	case PIO_KEYMAP: {
		int size,error;
		charmap_t *cmp;

		if (!transparent) {
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);
		   break;
		}

		error = drv_priv(iocp->ioc_cr);
		if (error) {
		   chr_iocnack(qp, mp, iocp, error, 0);
		   break;
		}

		cmp = cp->c_map_p;
		size = sizeof (cmp->cr_keymap_p->n_keys);
		chr_copyin(qp,mp,size,CHR_IN_0);
		break;
	   }	

	case PIO_SCRNMAP:

		if (transparent) {
		   chr_copyin(qp,mp,sizeof(scrnmap_t),CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case GIO_SCRNMAP: {
		scrnmap_t *scrp;
		unsigned int i = 0;

		if (!transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}


		if ( (nmp = allocb(sizeof(scrnmap_t), BPRI_MED)) == NULL)
		{
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		scrp = (scrnmap_t *) NULL;
		if (cp->c_scrmap_p) {
		   if ((scrp=cp->c_scrmap_p->scr_map_p) == (scrnmap_t *)NULL)
			scrp = cp->c_scrmap_p->scr_defltp->scr_map_p;
		}
		if (scrp) {
			bcopy(scrp,nmp->b_rptr,sizeof(scrnmap_t));
			nmp->b_wptr += sizeof(scrnmap_t);
		}
		else {
			for (i=0; i<sizeof(scrnmap_t); i++)
				*nmp->b_wptr++ = (unsigned char) i;
		}
		chr_copyout(qp,mp,nmp,sizeof(scrnmap_t),CHR_OUT_0);
		break;
	   }

	case PIO_STRMAP:

		if (transparent) {
		   chr_copyin(qp,mp,STRTABLN,CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case PIO_STRMAP_21:

		if (transparent) {
		   chr_copyin(qp,mp,STRTABLN_21,CHR_IN_0);
		}
		else 
		   chr_iocnack(qp, mp, iocp, EINVAL, 0);

		break;

	case GIO_STRMAP:
		if (!transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}


		if ( (nmp = allocb(STRTABLN, BPRI_MED)) == NULL)
		{
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		bcopy(cp->c_map_p->cr_strbufp, nmp->b_rptr, STRTABLN);
		nmp->b_wptr += STRTABLN;

		chr_copyout(qp,mp,nmp,STRTABLN,CHR_OUT_0);
		break;

	case GIO_STRMAP_21:
		if (!transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}


		if ( (nmp = allocb(STRTABLN_21, BPRI_MED)) == NULL)
		{
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		bcopy(cp->c_map_p->cr_strbufp, nmp->b_rptr, STRTABLN_21);
		nmp->b_wptr += STRTABLN_21;

		chr_copyout(qp,mp,nmp,STRTABLN_21,CHR_OUT_0);
		break;

	case SETLOCKLOCK: 
		if (!transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}

		if (mp->b_cont && ( *(int *)mp->b_cont->b_rptr == 0) ) {
			chr_iocack(qp,mp,iocp,0);
			break;
		}

		chr_iocnack(qp,mp,iocp,EINVAL,0);
		break;

	case KDGKBSTATE: {
		kbstate_t *kbp;
		unchar state = 0;
		mblk_t *nmp;
		if (! transparent) {
		   chr_iocnack(qp,mp,iocp,EINVAL,0);
		   break;
		}

		if ( (nmp = allocb(sizeof(state), BPRI_MED)) == NULL) {
		   chr_iocnack(qp, mp, iocp, EAGAIN, 0);
		   break;
		}

		kbp = &cp->c_kbstat;
		if (kbp->kb_state & SHIFTSET)
			state |= SHIFTED;
		if (kbp->kb_state & CTRLSET)
			state |= CTRLED;
		if (kbp->kb_state & ALTSET)
			state |= ALTED;

		bcopy(&state, nmp->b_rptr, sizeof(state));
		nmp->b_wptr += sizeof(state);

		chr_copyout(qp,mp,nmp,sizeof(state),CHR_OUT_0);
		break;
	   }

	default:
		putnext(qp, mp);

	}
}

int
chr_is_special(kp, idx, table)
register keymap_t *kp;
unchar idx;
unchar table;
{
	return (kp->key[idx].spcl & (0x80 >> table));
}


/* KDGKBENT ioctl translation function for character mapping tables */

ushort
chr_getkbent(cp, table, idx)
charstat_t *cp;
unchar table;
unchar idx;
{
	register keymap_t *kp;
	ushort	val, pfx;
	struct pfxstate *pfxp;

	kp = cp->c_map_p->cr_keymap_p;
	pfxp = (struct pfxstate *) cp->c_map_p->cr_pfxstrp;

	val = kp->key[idx].map[table];
	if(kp->key[idx].flgs & KMF_NLOCK)
		val |= NUMLCK;
	if(kp->key[idx].flgs & KMF_CLOCK)
		val |= CAPLCK;
	if (chr_is_special(kp,idx, table)) {
		if(IS_FUNKEY(val & 0xff) && pfxp[(val & 0xff) - K_PFXF].type != (unchar) 0){
			pfx = pfxp[(val & 0xff) - K_PFXF].type;	
			val = pfxp[(val & 0xff) - K_PFXF].val;
			switch(pfx){
				case K_ESN:
					pfx = SS2PFX;
					break;
				case K_ESO:
					pfx = SS3PFX;
					break;
				case K_ESL:
					pfx = CSIPFX;
					break;
			}
			return (pfx| (unchar)val);
		}
		switch(val & 0xff){
			case K_NOP:
				return NOKEY;
			case K_BRK:
				return BREAKKEY;
			case K_LSH:
			case K_RSH:
			case K_CLK:
			case K_NLK:
			case K_ALT:
			case K_CTL:
			case K_LAL:
			case K_RAL:
			case K_LCT:
			case K_RCT:
				return (val | SHIFTKEY);
			case K_ESN: {
				ushort rv;
				rv = kp->key[idx].map[table ^ ALTED] | SS2PFX | (val & 0xff00);
/*				return (kp->key[idx].map[table ^ ALTED] | SS2PFX | (val & 0xff00));
*/
				return rv;
			}
			case K_ESO:
				return (kp->key[idx].map[table ^ ALTED] | SS3PFX | (val & 0xff00));
			case K_ESL:
				return (kp->key[idx].map[table ^ ALTED] | CSIPFX | (val & 0xff00));
			case K_BTAB:
				return ('Z' | CSIPFX);
			default: {
				return (val | SPECIALKEY);
			}
		}
	}
	if(kp->key[idx].map[table | CTRLED] & 0x1f)
		val |= CTLKEY;
	return val;
}


/* KDSKBENT ioctl translation function for character mapping tables */
chr_setkbent(cp, table, idx, val)
charstat_t *cp;
unchar	table;
unchar	idx;
ushort	val;
{
	int special=0, smask, pfx,oldpri;
	register struct pfxstate *pfxp;
	register keymap_t *kp;

	kp = cp->c_map_p->cr_keymap_p;
	pfxp = (struct pfxstate *) cp->c_map_p->cr_pfxstrp;

#ifdef DEBUG1
	if (idx >= NUM_KEYS )
		cmn_err(CE_PANIC,"char_setkbent -- idx was bad");
#endif
	if((val & TYPEMASK) == SHIFTKEY)
		return;
	if((val & TYPEMASK) != NORMKEY)
		val &= ~CTLKEY;
	if(chr_is_special(kp,idx, table)) {
		int old_val = kp->key[idx].map[table];

		if (IS_FUNKEY(old_val) && (old_val > K_PFXF) && pfxp[old_val - K_PFXF].type != 0) {
			oldpri = splstr();
			pfxp[old_val - K_PFXF].val = 0;
			pfxp[old_val - K_PFXF].type = 0;
			splx(oldpri);
		}
	}
	oldpri = splstr();
	kp->key[idx].flgs = 0;
	if(val & NUMLCK)
		kp->key[idx].flgs |= KMF_NLOCK;
	if(val & CAPLCK)
		kp->key[idx].flgs |= KMF_CLOCK;
	splx(oldpri);
	smask = (0x80 >> table) + (0x80 >> (table | CTRLED));
	switch(val & TYPEMASK){
		case BREAKKEY:
			special = smask;	
			val = K_BRK;
			break;
		case NORMKEY:
			break;
		case SPECIALKEY:
			special = smask;
			break;
		default:
			special = smask;
			val = K_NOP;
			break;
		case SS2PFX:
			pfx = K_ESN;
			goto prefix;
		case SS3PFX:
			pfx = K_ESO;
			goto prefix;
		case CSIPFX:
			if((val & 0xff) == 'Z'){
				special = smask;
				val = K_BTAB;
				break;
			}
			pfx = K_ESL;
prefix:
		special = smask;
		if( (val & 0xff) == kp->key[idx].map[table ^ ALTED])
			val = pfx;
		else{
			int	keynum;

			for(keynum = 0; keynum < (K_PFXL - K_PFXF); keynum++){
				if(pfxp[keynum].type == 0)
					break;
			}
			if(keynum < K_PFXL - K_PFXF){
				oldpri = splstr();
			 	pfxp[keynum].val = val & 0xff;
				pfxp[keynum].type = pfx;
				splx(oldpri);
				val = K_PFXF + keynum;
			}else
				val = K_NOP;
		}
		break;
	}
	oldpri = splstr();
	kp->key[idx].map[table] = (unchar)val;
	kp->key[idx].map[table | CTRLED] = (val & CTLKEY) ? (val & 0x1f) : val;
	kp->key[idx].spcl = (kp->key[idx].spcl & ~smask) | special;
	splx(oldpri);
}


extern int ws_addstring();

void
chr_do_iocdata(qp, mp,cp)
	queue_t *qp;
	register mblk_t *mp;
	register charstat_t *cp;
{
	register struct iocblk *iocp;
	struct copyresp *csp;
	struct copyreq *cqp;
	struct mblk_t *bp;
	copy_state_t *copyp;
	int oldpri,error;

	iocp = (struct iocblk *)mp->b_rptr;

	csp = (struct copyresp *) mp->b_rptr;
	copyp = (copy_state_t *)csp->cp_private;


	switch (iocp->ioc_cmd) {

	default:
		putnext(qp,mp); /* not for us */
		break;

	case GIO_SCRNMAP: /* and other M_COPYOUT ioctl types */
	case GIO_KEYMAP:
	case GIO_STRMAP:
	case GIO_STRMAP_21:
	case KDGKBMODE:	
	case KDGKBSTATE:
	case MOUSEIOCREAD:
		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}

		chr_iocack(qp,mp,iocp,0);
		break;

	case GETFKEY: {

		struct fkeyarg *fp;
		unchar *charp;
		ushort *idxp;

		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}

		if (copyp->cpy_state == CHR_OUT_0) {
			chr_iocack(qp, mp, iocp, 0);
			return;
		}

		/* must be copy in */
		if (pullupmsg(mp->b_cont, sizeof(struct fkeyarg)) == 0) {
			cmn_err(CE_WARN,"char: pull up of GETFKEY ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		fp = (struct fkeyarg *) mp->b_cont->b_rptr;
		if ((int) fp->keynum < 1 || (int) fp->keynum > NSTRKEYS) {
			chr_iocnack(qp,mp,iocp,EINVAL,0);
			return;
		}

		idxp = (ushort *) cp->c_map_p->cr_strmap_p;
		idxp += fp->keynum - 1;
		fp->flen = 0;
		charp = (unchar *) cp->c_map_p->cr_strbufp + *idxp;

		while ( fp->flen < MAXFK && *charp != '\0') 
		   fp->keydef[fp->flen++] = *charp++;

		/* now copyout fkeyarg back up to user */
		cqp = (struct copyreq *) mp->b_rptr;
		cqp->cq_size = sizeof(struct fkeyarg);
		cqp->cq_addr = (caddr_t) copyp->cpy_arg;
		cqp->cq_flag = 0;
		cqp->cq_private = (mblk_t *) copyp;
		copyp->cpy_state = CHR_OUT_0;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		mp->b_datap->db_type = M_COPYOUT;
		qreply(qp, mp);
		return;
	   }

	case SETFKEY: {
		struct fkeyarg *fp;
		ushort idx;

		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}

		if (pullupmsg(mp->b_cont, sizeof(struct fkeyarg)) == 0) {
			cmn_err(CE_WARN,"char: pull up of SETFKEY ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		fp = (struct fkeyarg *) mp->b_cont->b_rptr;
		if ((int)fp->keynum < 1 || (int)fp->keynum > NSTRKEYS) {
			chr_iocnack(qp,mp,iocp,EINVAL,0);
			return;
		}
		fp->keynum -= 1; /* ws_addstring assumes 0..NSTRKEYS-1 range */

		if (!ws_addstring(cp->c_map_p, fp->keynum, fp->keydef, fp->flen))
			chr_iocnack(qp,mp,iocp,EINVAL,0);
		else
			chr_iocack(qp, mp, iocp, 0);

		return;
	   }

	case PIO_STRMAP_21:
	case PIO_STRMAP: {
		strmap_t *newmap;
		unsigned long size;

		size = (iocp->ioc_cmd == PIO_STRMAP) ? STRTABLN : STRTABLN_21;

		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}

		if (pullupmsg(mp->b_cont, size) == 0) {
			cmn_err(CE_WARN,"char: pull up of PIO_STRMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		if (!ws_newstrbuf(cp->c_map_p,KM_NOSLEEP)) {
			chr_iocnack(qp, mp, iocp, ENOMEM, 0);
			return;
		}

		oldpri = splstr();
		bcopy(mp->b_cont->b_rptr, cp->c_map_p->cr_strbufp, size);
		splx(oldpri);
		(void) ws_strreset(cp->c_map_p);
		chr_iocack(qp, mp, iocp,0);
		break;
	   }


	case PIO_SCRNMAP: {
		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}

		if (pullupmsg(mp->b_cont, sizeof(scrnmap_t)) == 0) {
			cmn_err(CE_WARN,"char: pull up of PIO_SCRNMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		if (!ws_newscrmap(cp->c_scrmap_p,KM_NOSLEEP)) {
			chr_iocnack(qp, mp, iocp, ENOMEM, 0);
			return;
		}

		oldpri = splstr();
		bcopy(mp->b_cont->b_rptr, cp->c_scrmap_p->scr_map_p, sizeof(scrnmap_t));
		splx(oldpri);
		chr_iocack(qp, mp, iocp,0);
		break;
	   }

	case PIO_KEYMAP: {
		keymap_t *newmap;
		short numkeys;
		uint size;
		charmap_t *cmp;

		if (csp->cp_rval) {
			freemsg(mp);
	 		return;
		}

		if (copyp->cpy_state == CHR_IN_0)
		{
		   numkeys = *(short *) mp->b_cont->b_rptr;
		   numkeys = min(numkeys, NUM_KEYS);
		   freemsg(mp->b_cont);

		   size = numkeys * sizeof(newmap->key[0]);
		   size += sizeof(newmap->n_keys);

		   cqp = (struct copyreq *) mp->b_rptr;
		   cqp->cq_size = size;
		   cqp->cq_addr = (caddr_t) copyp->cpy_arg;
		   cqp->cq_flag = 0;
		   cqp->cq_private = (mblk_t *)copyp;
		   copyp->cpy_arg = numkeys;
		   copyp->cpy_state = CHR_IN_1;
		   mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		   mp->b_datap->db_type = M_COPYIN;
		   qreply(qp, mp);
		   return;
	   	}

		numkeys = copyp->cpy_arg;
		size = numkeys*sizeof(newmap->key[0]) + sizeof(numkeys);
		if (pullupmsg(mp->b_cont, size) == 0) {
			cmn_err(CE_WARN,"char: pull up of PIO_KEYMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		cmp = cp->c_map_p;
		if (!ws_newkeymap(cmp, numkeys, mp->b_cont->b_rptr,KM_NOSLEEP))
		{
			cmn_err(CE_WARN,"char: PIO_KEYMAP: could not allocate new keymap");
			chr_iocnack(qp, mp, iocp, EAGAIN, 0);
			return;
		}

		chr_iocack(qp, mp, iocp,0);
		break;
	   }

	case KDDFLTSTRMAP: {
		uint size;
		charmap_t *cmp;
		struct str_dflt *kp;

		if (csp->cp_rval) {
			freemsg(mp);
	 		return;
		}

		if (copyp->cpy_state == CHR_OUT_0) {
			chr_iocack(qp, mp, iocp,0);
			return;
		}

		cmp = cp->c_map_p;

		if (pullupmsg(mp->b_cont, sizeof(struct str_dflt)) == 0) {
			cmn_err(CE_WARN,"char: pull up of KDDFLTSTRMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		kp = (struct str_dflt *) mp->b_cont->b_rptr;
		if (kp->str_direction == KD_DFLTGET) {
		   cqp = (struct copyreq *) mp->b_rptr;
		   cqp->cq_size = sizeof(struct str_dflt);
		   cqp->cq_addr = (caddr_t) cp->c_copystate.cpy_arg;
		   cqp->cq_flag = 0;
		   cqp->cq_private = (mblk_t *) copyp;
		   copyp->cpy_state = CHR_OUT_0;
		   mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		   mp->b_datap->db_type = M_COPYOUT;
		   oldpri = splstr();
		   bcopy(cmp->cr_defltp->cr_strbufp,&kp->str_map,sizeof(strmap_t));
		   splx(oldpri);
		   qreply(qp, mp);
		   return;
		}
		if (kp->str_direction != KD_DFLTSET) {
			chr_iocnack(qp, mp, iocp, EINVAL, 0);
			return;
		}
		error = drv_priv(iocp->ioc_cr);
		if (error) {
		   chr_iocnack(qp, mp, iocp, error, 0);
		   break;
		}

		oldpri = splstr();
		bcopy(&kp->str_map,cmp->cr_defltp->cr_strbufp,sizeof(strmap_t));
		splx(oldpri);
		ws_strreset(cmp->cr_defltp);
		chr_iocack(qp,mp,iocp,0);
		break;
	   }

	case KDDFLTKEYMAP: {
		keymap_t *newmap;
		uint size;
		charmap_t *cmp;
		struct key_dflt *kp;

		if (csp->cp_rval) {
			freemsg(mp);
	 		return;
		}

		if (copyp->cpy_state == CHR_OUT_0) {
			chr_iocack(qp, mp, iocp,0);
			return;
		}

		cmp = cp->c_map_p;

		if (pullupmsg(mp->b_cont, sizeof(struct key_dflt)) == 0) {
			cmn_err(CE_WARN,"char: pull up of KDDFLTKEYMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		kp = (struct key_dflt *) mp->b_cont->b_rptr;
		if (kp->key_direction == KD_DFLTGET) {
		   cqp = (struct copyreq *) mp->b_rptr;
		   cqp->cq_size = sizeof(struct key_dflt);
		   cqp->cq_addr = (caddr_t) cp->c_copystate.cpy_arg;
		   cqp->cq_flag = 0;
		   cqp->cq_private = (mblk_t *) copyp;
		   copyp->cpy_state = CHR_OUT_0;
		   mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		   mp->b_datap->db_type = M_COPYOUT;
		   oldpri = splstr();
		   bcopy(cmp->cr_defltp->cr_keymap_p,&kp->key_map,sizeof(keymap_t));
		   splx(oldpri);
		   qreply(qp, mp);
		   return;
		}
		if (kp->key_direction != KD_DFLTSET) {
			chr_iocnack(qp, mp, iocp, EINVAL, 0);
			return;
		}
		error = drv_priv(iocp->ioc_cr);
		if (error) {
		   chr_iocnack(qp, mp, iocp, error, 0);
		   break;
		}

		oldpri = splstr();
		bcopy(&kp->key_map,cmp->cr_defltp->cr_keymap_p,sizeof(keymap_t));
		splx(oldpri);
		chr_iocack(qp,mp,iocp,0);
		break;
	   }

	case KDDFLTSCRNMAP: {
		uint size;
		scrn_t *scrp;
		struct scrn_dflt *kp;
		int i;
		char *c;

		if (csp->cp_rval) {
			freemsg(mp);
	 		return;
		}

		if (copyp->cpy_state == CHR_OUT_0) {
			chr_iocack(qp, mp, iocp,0);
			return;
		}

		if ( (scrp = cp->c_scrmap_p) == (scrn_t *) NULL) {
			chr_iocnack(qp, mp, iocp, EACCES, 0);
			return;
		}

		if (pullupmsg(mp->b_cont, sizeof(struct scrn_dflt)) == 0) {
			cmn_err(CE_WARN,"char: pull up of KDDFLTSCRNMAP ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		kp = (struct scrn_dflt *) mp->b_cont->b_rptr;
		if (kp->scrn_direction == KD_DFLTGET) {
		   cqp = (struct copyreq *) mp->b_rptr;
		   cqp->cq_size = sizeof(struct scrn_dflt);
		   cqp->cq_addr = (caddr_t) cp->c_copystate.cpy_arg;
		   cqp->cq_flag = 0;
		   cqp->cq_private = (mblk_t *) copyp;
		   copyp->cpy_state = CHR_OUT_0;
		   mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		   mp->b_datap->db_type = M_COPYOUT;
		   if (scrp->scr_defltp->scr_map_p) {
		   	oldpri = splstr();
		   	bcopy(scrp->scr_defltp->scr_map_p,&kp->scrn_map,sizeof(scrnmap_t));
		   	splx(oldpri);
		   } else {
			c = (char *) &kp->scrn_map;
			for (i=0; i<sizeof(scrnmap_t); *c++ = i++);
		   }
		   qreply(qp, mp);
		   return;
		}
		if (kp->scrn_direction != KD_DFLTSET) {
			chr_iocnack(qp, mp, iocp, EINVAL, 0);
			return;
		}
		error = drv_priv(iocp->ioc_cr);
		if (error) {
		   chr_iocnack(qp, mp, iocp, error, 0);
		   break;
		}

		if (!scrp->scr_defltp->scr_map_p) {
			if (!ws_newscrmap(scrp->scr_defltp,KM_NOSLEEP)) {
			   chr_iocnack(qp, mp, iocp, ENOMEM, 0);
			   return;
			}
		}
		oldpri = splstr();
		bcopy(&kp->scrn_map,scrp->scr_defltp->scr_map_p,sizeof(scrnmap_t));
		splx(oldpri);
		chr_iocack(qp,mp,iocp,0);
		break;
	   }

	case KDGKBENT: {
		struct kbentry *kbep;
		charmap_t *cmp;

		cmp = cp->c_map_p;

		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}


		if (copyp->cpy_state == CHR_OUT_0)
		{
			chr_iocack(qp, mp, iocp,0);
			return;
		}

		if (pullupmsg(mp->b_cont, sizeof(struct kbentry)) == 0) {
			cmn_err(CE_WARN,"char: pull up of KDGKBENT ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		kbep = (struct kbentry *) mp->b_cont->b_rptr;

		if (kbep->kb_index >= 128) {
			chr_iocnack(qp, mp, iocp, ENXIO, 0);
			return;
		}

		switch (kbep->kb_table) {
		case K_NORMTAB:
		   kbep->kb_value = chr_getkbent(cp,NORMAL,kbep->kb_index);
		   break;
		case K_SHIFTTAB:
		   kbep->kb_value = chr_getkbent(cp,SHIFT,kbep->kb_index);
		   break;
		case K_ALTTAB:
		   kbep->kb_value = chr_getkbent(cp,ALT,kbep->kb_index);
		   break;
		case K_ALTSHIFTTAB:
		   kbep->kb_value = chr_getkbent(cp,ALTSHF,kbep->kb_index);
		   break;
		case K_SRQTAB:
		   kbep->kb_value = *((unchar *)cmp->cr_srqtabp + kbep->kb_index);
		   break;
		default:
		   chr_iocnack(qp, mp, iocp, ENXIO, 0);
		   return;
		}

		cqp = (struct copyreq *) mp->b_rptr;
		cqp->cq_size = sizeof(*kbep);
		cqp->cq_addr = (caddr_t) cp->c_copystate.cpy_arg;
		cqp->cq_flag = 0;
		cqp->cq_private = (mblk_t *) copyp;
		copyp->cpy_state = CHR_OUT_0;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		mp->b_datap->db_type = M_COPYOUT;
		qreply(qp, mp);
		return;
	   }

	case KDSKBENT: {
		struct kbentry *kbep;
		charmap_t *cmp;
		ushort numkeys;

		cmp = cp->c_map_p;

		if (csp->cp_rval) {
			freemsg(mp);
			return;
		}


		if (pullupmsg(mp->b_cont, sizeof(struct kbentry)) == 0) {
			cmn_err(CE_WARN,"char: pull up of KDSKBENT ioctl data failed");
			chr_iocnack(qp, mp, iocp, EFAULT, 0);
			return;
		}

		kbep = (struct kbentry *) mp->b_cont->b_rptr;

		if (kbep->kb_index >= 128) {
			chr_iocnack(qp, mp, iocp, ENXIO, 0);
			return;
		}
		
		numkeys = cmp->cr_keymap_p->n_keys;

		if (!ws_newkeymap(cmp, numkeys, cmp->cr_keymap_p,KM_NOSLEEP))
		{
			cmn_err(CE_WARN,"char: could not allocate new keymap");
			chr_iocnack(qp, mp, iocp, EAGAIN, 0);
			return;
		}

		if (!ws_newpfxstr(cmp,KM_NOSLEEP))
		{
			cmn_err(CE_WARN,"char: could not allocate new pfxstr");
			chr_iocnack(qp, mp, iocp, EAGAIN, 0);
			return;
		}

		switch (kbep->kb_table) {
		case K_NORMTAB:
		   chr_setkbent(cp,NORMAL,kbep->kb_index,kbep->kb_value);
		   break;
		case K_SHIFTTAB:
		   chr_setkbent(cp,SHIFT,kbep->kb_index,kbep->kb_value);
		   break;
		case K_ALTTAB:
		   chr_setkbent(cp,ALT,kbep->kb_index,kbep->kb_value);
		   break;
		case K_ALTSHIFTTAB:
		   chr_setkbent(cp,ALTSHF,kbep->kb_index,kbep->kb_value);
		   break;
		case K_SRQTAB: {
		   unchar *c;
		   c = (unchar *)cmp->cr_srqtabp;
		   oldpri = splstr();
		   *(c + kbep->kb_index) = kbep->kb_value;
		   splx(oldpri);
		   break;
		}
		default:
		   chr_iocnack(qp, mp, iocp, ENXIO, 0);
		   return;
		}

		chr_iocack(qp, mp, iocp,0);
		break;
	   }

	}
}

void
chr_proc_w_data(qp,mp,cp)
queue_t *qp;
mblk_t *mp;
charstat_t *cp;
{
	register unchar *chp;
	register scrnmap_t *scrnp;
	scrn_t *map_p;
	mblk_t *bp;

	if ( (map_p = cp->c_scrmap_p) == (scrn_t *) NULL) {
		putnext(qp,mp);
		return;
	}

	if ( (scrnp = map_p->scr_map_p) == (scrnmap_t *) NULL) {
		if ((scrnp = map_p->scr_defltp->scr_map_p) == (scrnmap_t *) NULL) {
			putnext(qp,mp);
			return;
		}
	}

	for (bp = mp; bp != (mblk_t *)NULL; bp=bp->b_cont)
	{
		chp = (unchar *) bp->b_rptr;
		while (chp != (unchar *) bp->b_wptr)
		{
			*chp = *( (unchar *)scrnp + *chp);
			chp++;
		}
	}

	putnext(qp,mp);
}


void
chr_do_xmouse(qp,mp,cp)
queue_t *qp;
mblk_t *mp;
charstat_t *cp;
{
	register struct mse_event *msep;
	register xqEvent *evp;

	msep = (struct mse_event *) mp->b_cont->b_rptr;
	evp = &cp->c_xevent;

	evp->xq_type = msep->type;
	evp->xq_x = msep->x;
	evp->xq_y = msep->y;
	evp->xq_code = msep->code;

	xq_enqueue(cp->c_xqinfo, evp); 
}

void
chr_do_mouseinfo(qp,mp,cp)
queue_t *qp;
mblk_t *mp;
charstat_t *cp;
{
	register struct mse_event *msep;
	register struct mouseinfo *minfop;
	int oldpri;

	msep = (struct mse_event *) mp->b_cont->b_rptr;
	minfop = &cp->c_mouseinfo;
	minfop->status = (~msep->code & 7) | ((msep->code ^ cp->c_oldbutton) << 3) | (minfop->status & BUTCHNGMASK) | (minfop->status & MOVEMENT);

	if (msep->type == MSE_MOTION) {
		register int sum;

		minfop->status |= MOVEMENT;
		sum = minfop->xmotion + msep->x;
		if (sum >= chr_maxmouse)
			minfop->xmotion = chr_maxmouse;
		else if (sum <= -chr_maxmouse)
			minfop->xmotion = -chr_maxmouse;
		else
			minfop->xmotion = sum;
		sum = minfop->ymotion + msep->y;
		if (sum >= chr_maxmouse)
			minfop->ymotion = chr_maxmouse;
		else if (sum <= -chr_maxmouse)
			minfop->ymotion = -chr_maxmouse;
		else
			minfop->ymotion = sum;
	}
	/* Note the button state */
	cp->c_oldbutton = msep->code;
	cp->c_state |= C_MSEINPUT;
	if (cp->c_heldmseread) {
		mblk_t *tmp;
		tmp = cp->c_heldmseread;
		cp->c_heldmseread = (mblk_t *)NULL;
		chr_write_queue_put(WR(qp),tmp);
	}
	return;
}

int
chr_proc_w_proto(qp,mp,cp)
queue_t *qp;
mblk_t *mp;
charstat_t *cp;
{
	register ch_proto_t *chp;
	mblk_t *bp;

	if ( (mp->b_wptr - mp->b_rptr) != sizeof(ch_proto_t)) {	
		putnext(qp, mp);
		return;
	}

	chp = (ch_proto_t *) mp->b_rptr;

	if (chp->chp_type != CH_CTL) {	
		putnext(qp, mp);
		return;
	}

	switch (chp->chp_stype) {

	default:
		putnext(qp, mp);
		return;

	case CH_TCL:
		break;

	} /* switch */

	/* CH_TCL message handling */
	switch (chp->chp_stype_cmd) {

	default:
		putnext(qp,mp);
		return;

	case TCL_ADD_STR: {
		tcl_data_t *tp;	
		ushort keynum, len;

		if (mp->b_cont == (mblk_t *) NULL)
		{
			putnext(qp,mp);
			return;
		}

		/* assume one data block */
		tp = (tcl_data_t *) mp->b_cont->b_rptr;

		keynum = tp->add_str.keynum;
		len = tp->add_str.len;

		/* put tp past the tcl_data structure in the
		 * data block. Now it points to the string itself
		 */

		tp++;

		/* if ws_addstring fails, beep the driver */
		if (!ws_addstring(cp->c_map_p, keynum, (unchar *) tp, len)) {
			chp->chp_stype_cmd  = TCL_BELL;
			freemsg(mp->b_cont);
			putnext(qp,mp);	/* ship it down */
		} else
			freemsg(mp);

		break;
	   }

	   case TCL_FLOWCTL:
		if (chp->chp_stype_arg == TCL_FLOWON)
			cp->c_state |= C_FLOWON;
		else if (chp->chp_stype_arg == TCL_FLOWOFF)
			cp->c_state &= ~C_FLOWON;
		putnext(qp,mp);
		break;
	} /* switch */
}
