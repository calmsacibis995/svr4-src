#ident	"@(#)e503li.c	1.1	92/09/30	JPB"
static char SysVr3TCPID[] = "@(#)e503li.c	3.3 Lachman System V STREAMS TCP source";
/*
 *	System V STREAMS TCP - Release 3.0
 *
 *	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strlog.h"
#include "sys/log.h"
#include "sys/lihdr.h"

#include "sys/signal.h"		/* needed for sys/user.h */
#include "sys/dir.h"		/* needed for sys/user.h */
#include "sys/page.h"		/* needed for sys/user.h */
#include "sys/seg.h"		/* needed for sys/user.h */

#include "sys/user.h"
#include "sys/errno.h"
#include "sys/conf.h"
#ifdef M_XENIX
#include "sys/assert.h"
#else
#include "sys/debug.h"
#endif
#include "sys/socket.h"
#ifdef M_XENIX
#include "sys/machdep.h"	/* for MAXVEC */
#endif
#include "net/if.h"
#include "sys/e503.h"

/* stream data structure definitions */

int e503open(), e503close(), e503uwput(), e503uwsrv(), e503ursrv();

extern nulldev();
extern u_short htons();

extern e503hwput(), e503hwinit();

struct module_info e503_minfo = {
	0, "e503", E503ETHERMIN, E503ETHERMTU, 16*E503ETHERMTU, 12*E503ETHERMTU
};

struct qinit e503urinit = {
	NULL,  e503ursrv, e503open, e503close, nulldev, &e503_minfo, NULL
};

struct qinit e503uwinit = {
	e503uwput, e503uwsrv, e503open, e503close, nulldev, &e503_minfo, NULL
};

struct streamtab e503info = { &e503urinit, &e503uwinit, NULL, NULL };

/*
 * The following six routines are the normal streams driver
 * access routines
 */

e503open(q, dev, flag, sflag)
register queue_t *q; 
{
	register struct en_minor *em;
	unsigned int maj;
	unsigned int bd;
	int x;

	maj = major(dev);
	dev = minor(dev);
	bd = maj_to_board(maj);

	if (!e503inited[bd] && (e503INIT(bd), !e503inited[bd])) {
		u.u_error = ENXIO;
		return(OPENFAIL);
	}

	em = &e503_em[bd*n3c503min];

	if (sflag == CLONEOPEN)
		for(dev = 0; dev < n3c503min; dev++)
			if (em[dev].em_top == NULL) 
				break;

	if ((dev < 0) || (dev >= n3c503min)) {
		u.u_error = ENXIO;
		return(OPENFAIL);
	}
	em = &em[dev];
	x = splstr();
	if (em->em_top == NULL) {
		/* setup streams pointer */
		q->q_ptr = WR(q)->q_ptr = (char *)em;
		((struct en_minor *)q->q_ptr)->em_min = dev;
		((struct en_minor *)q->q_ptr)->em_unit = bd;
		em->em_top = q;
		em->em_sap = 0;
		noenable(WR(q));	/* enabled after tx completion */
	}
	splx(x);

	/* only one stream at a time */

	if (q != em->em_top) {
		u.u_error = EBUSY;
		return(OPENFAIL);
	}

	e503nopens[bd]++;
	return(dev);
}

e503close(q)
register queue_t *q; 
{
	register struct en_minor *em;
	register bd;
	int x;

	em = (struct en_minor *)q->q_ptr;
	bd = em->em_unit;

	x = splstr();

	em->em_top = NULL;
	em->em_sap = 0;

	flushq(WR(q), 1);
	q->q_ptr = NULL;

	e503nopens[bd]--;

	if (!e503nopens[bd]) {
		e503hwclose(bd);
		e503inited[bd] = 0;
	}

	splx(x);
}

e503uwput(q, mp)
register queue_t *q;
register mblk_t *mp; 
{
	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503uwput: type=0x%x", mp->b_datap->db_type);
	switch (mp->b_datap->db_type) {
	    case M_PROTO:
	    case M_PCPROTO:
		dlproto(q, mp);
		return;

	    case M_IOCTL:
		e503ioctl(q, mp);
		return;

	    case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHALL);
			*mp->b_rptr &= ~FLUSHW;
		}
		if (*mp->b_rptr & FLUSHR) {
			flushq(RD(q), FLUSHALL);
			qreply(q, mp);
		} 
		else
			freemsg(mp);
		return;
		
	    default:
		printf("e503uwput: unknown STR msg type %x received mp = %x\n",
			mp->b_datap->db_type, mp);
		freemsg(mp);
		return;
	}
}

e503uwsrv(q)
register queue_t *q; 
{
	register mblk_t *mp;
	register unit = ((struct en_minor *)(q->q_ptr))->em_unit;
	int 	 s;
	int	 ok = 1;

	while (ok && (mp = getq(q))) {
		s = splstr();
		if (e503hwput(unit, mp) == OK) 
			freemsg(mp);
		else {
			putbq(q, mp);
			ok = 0;
		}
		splx(s);
	}
}

e503ursrv(q)
register queue_t *q;
{
	register mblk_t *mp;

	while (mp = getq(q))
		dlput(q, mp);
}

e503unitcnt()
{
	int i, j;

	for (i=0, j=0; i < cdevcnt && j < n3c503unit; i++)
		if (cdevsw[i].d_str == &e503info)
			e503major[j++] = i;
		
	n3c503unit = j;     /* in case more boards declared than installed */
}

e503init()
{
	static done = 0;
	extern int (*vecintsw[])();
	extern e503intr();
	extern char *eaddrstr();
	unchar e[6];
	int i;

	if (done)
		return;
	done = 1;

#ifndef M_XENIX
	printf("3c503 Ethernet Driver V3.3 \n");
	printf("Copyright 1988, 1989 Lachman Associates, Inc.  All Rights Reserved.\n");
#endif
	e503unitcnt();
	for (i = 0; i < n3c503unit; i++) { 
		if (e503present(i,e)) {
#ifdef M_XENIX
		    printcfg("3comB",e503io_base(i),EDEVSIZ,e503intl[i],-1, 
			"type=3c503 addr=%s",eaddrstr(e));
#else
		    printf("3c503 board %d present, ethernet address %s\n",
			i, eaddrstr(e));
#endif
		}
		else
		    printf("3comB (3c503) board %d not present\n", i);
	}
}

/*
 * real init
 */
e503INIT(unit)
register unit;
{
	register struct en_minor *em;
	register int i;
	int x;

	x = splstr();

	em = &e503_em[unit*n3c503min];

	for (i = 0; i < n3c503min; i++, em++) {
		em->em_unit = unit;
		em->em_sap = 0;
		em->em_top = NULL;
	}

	if (!e503hwinit(unit, e503eaddr[unit].octet)) {
		printf("No board for unit %d\n", unit);
		splx(x);
		return;
	}

	e503inited[unit]++;

	splx(x);
}

/*
 * The following three routines implement LLI protocol 
 */
dlproto(q, mp)
queue_t *q;
mblk_t *mp; 
{
	register struct en_minor *em = (struct en_minor *)q->q_ptr;
	union DL_primitives *prim;
	mblk_t *infomp, *bindmp, *respmp;
	struct DL_info_ack *info_ack;
	struct DL_bind_ack *bind_ack;
	struct DL_ok_ack *ok_ack;
	e_frame *enethdr;
	register min = em->em_min;
	register bd = em->em_unit;
	int	 s;

	prim = (union DL_primitives *) mp->b_rptr;

	if (em->em_top == NULL) {
		dlerror(q, prim->prim_type, DLSYSERR, ENXIO);
		freemsg(mp);
		return;
	}

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "dlproto: prim=0x%x", prim->prim_type);

	switch(prim->prim_type) {
	case DL_INFO_REQ:
		if ((infomp = allocb(sizeof(struct DL_info_ack), BPRI_HI)) == NULL) {
			dlerror(q, prim->prim_type, DLSYSERR, ENOSR);
			freemsg(mp);
			return;
		}
		info_ack = (struct DL_info_ack *) infomp->b_rptr;
		infomp->b_wptr += sizeof(struct DL_info_ack);
		infomp->b_datap->db_type = M_PCPROTO;
		info_ack->PRIM_type = DL_INFO_ACK;
		info_ack->SDU_max = E503ETHERMTU;
		info_ack->SDU_min = E503ETHERMIN;
		info_ack->ADDR_length = sizeof(e_addr);
		info_ack->SUBNET_type = DL_ETHER;
		info_ack->SERV_class = DL_NOSERV;
		info_ack->CURRENT_state = (em->em_sap == 0)?DL_UNBND:DL_IDLE;
		freemsg(mp);
		qreply(q, infomp);
		break;

	case DL_UNITDATA_REQ:
		ASSERT(mp->b_rptr >= mp->b_datap->db_base);
		enethdr = (e_frame *) mp->b_rptr;
		if (em->em_sap == 0) {
			prim->prim_type = DL_UDERROR_IND;
			prim->error_ind.ERROR_type = EPROTO;
			qreply(q, mp);
			return;
		}
		if (prim->data_req.RA_length != sizeof(enethdr->ether_dhost)) {
			prim->prim_type = DL_UDERROR_IND;
			prim->error_ind.ERROR_type = EINVAL;
			qreply(q, mp);
			return;
		}
		bcopy((caddr_t) mp->b_rptr+prim->data_req.RA_offset,
			(caddr_t) enethdr->ether_dhost,
			(unsigned) prim->data_req.RA_length);
		bcopy((caddr_t) e503eaddr[bd].octet, 
		      (caddr_t) enethdr->ether_shost, sizeof(e_addr));

#if defined(M_I386) || defined(i386)
		enethdr->e_type = em->em_sap;
#else
		enethdr->e_type[0] = ((unchar *) &(em->em_sap))[sizeof(long)-2];
		enethdr->e_type[1] = ((unchar *) &(em->em_sap))[sizeof(long)-1];
#endif
		ASSERT(mp->b_rptr >= mp->b_datap->db_base);
		mp->b_wptr = mp->b_rptr + sizeof(struct ether_header);
		s = splstr();
		if ((q->q_first || e503hwput(bd, mp) != OK) && canput(q))
			putq(q, mp);
		else
			freemsg(mp);
		splx(s);
		break;

	case DL_BIND_REQ:
		if (em->em_sap) {
			dlerror(q, prim->prim_type, DLOUTSTATE, 0);
			freemsg(mp);
			return;
		}
		em->em_sap =  htons((short) prim->bind_req.LLC_sap);
		if ((bindmp = allocb(sizeof(struct DL_bind_ack)+sizeof(e_addr),
					BPRI_HI)) == NULL) {
			dlerror(q, prim->prim_type, DLSYSERR, ENOSR);
			freemsg(mp);
			return;
		}
		bind_ack = (struct DL_bind_ack *) bindmp->b_rptr;
		bindmp->b_wptr += (sizeof(struct DL_bind_ack) 
				+ sizeof(e_addr));
		bindmp->b_datap->db_type = M_PCPROTO;
		bind_ack->PRIM_type = DL_BIND_ACK;
		bind_ack->ADDR_offset = sizeof(struct DL_bind_ack);
		bind_ack->ADDR_length = sizeof(e_addr);
		bcopy((caddr_t) e503eaddr[bd].octet,
			(caddr_t) bindmp->b_rptr + sizeof(struct DL_bind_ack),
			sizeof(e_addr));
		bind_ack->LLC_sap = prim->bind_req.LLC_sap;
		freemsg(mp);
		qreply(q, bindmp);
		break;

	case DL_UNBIND_REQ:
		if (em->em_sap == 0) {
			dlerror(q, prim->prim_type, DLSYSERR, EINVAL);
			freemsg(mp);
			return;
		}
		em->em_sap = 0;
		if (respmp = allocb(sizeof(struct DL_ok_ack), BPRI_HI)) {
			ok_ack = (struct DL_ok_ack *) respmp->b_rptr;
			respmp->b_wptr += sizeof(struct DL_ok_ack);
			respmp->b_datap->db_type = M_PCPROTO;
			ok_ack->PRIM_type = DL_OK_ACK;
			ok_ack->CORRECT_prim = prim->prim_type;
			qreply(q, respmp);
		}
		freemsg(mp);
		break;

	default:
		dlerror(q, prim->prim_type, DLSYSERR, EINVAL);
		break;
	}
}

#ifdef E503_DEBUG
static char* 
eaddr(a)
    char* a;
{
    static char addr[2][7];
    static use = 0;
    register i;

    for (i=0; i<6; ++i)
	addr[use][i] = '0' + *a++;
    i = use;
    use = use ? 1 : 0;
    return addr[i];
}
#endif

int
dlput(q, mp)
queue_t *q;
register mblk_t *mp;
{
	register struct DL_unitdata_ind *ind;
	register mblk_t *indmp;
	register e_frame *eh;
	register i;

	eh = (e_frame *) mp->b_rptr;

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "dlput: type=0x%x", eh->e_type);
	mp->b_rptr += sizeof(e_frame);

	indmp = allocb(sizeof(struct DL_unitdata_ind)
		       + 2*sizeof(eh->ether_shost), BPRI_MED);
	if (indmp == NULL) {
		freemsg(mp);
		printf("dlput: no streams buffer\n");
		return(NOT_OK);
	}
	indmp->b_wptr += sizeof(struct DL_unitdata_ind)
		+ 2*sizeof(eh->ether_shost);
	indmp->b_datap->db_type = M_PROTO;
	indmp->b_cont = mp;
	ind = (struct DL_unitdata_ind *) indmp->b_rptr;
	ind->PRIM_type = DL_UNITDATA_IND;
	ind->RA_length = sizeof(eh->ether_shost);
	ind->RA_offset = sizeof(struct DL_unitdata_ind);

	bcopy((caddr_t) eh->ether_shost,
	      (caddr_t) indmp->b_rptr 
		+ sizeof(struct DL_unitdata_ind),
		sizeof(eh->ether_shost));

	ind->LA_length = sizeof(eh->ether_dhost);
	ind->LA_offset = sizeof(struct DL_unitdata_ind)
		+ sizeof(eh->ether_shost);

	bcopy((caddr_t) eh->ether_dhost,
	      (caddr_t) indmp->b_rptr 
		+ sizeof(struct DL_unitdata_ind)
		+ sizeof(eh->ether_shost),
		sizeof(eh->ether_dhost));

	ind->SERV_class = DL_NOSERV;

	/* pass this message up */
	if (canput(q->q_next)) {
		putnext(q, indmp);
		return(OK);
	}
	else {
		freemsg(indmp);
		return(NOT_OK);
	}
}

static
dlerror(q,prim,dl_err,sys_err)
queue_t *q;
long prim;
int dl_err, sys_err;
{
	mblk_t *errmp;
        struct DL_error_ack *error_ack;
        
        if(errmp = allocb(sizeof(struct DL_error_ack), BPRI_HI)) {
                error_ack = (struct DL_error_ack *) errmp->b_rptr;
                errmp->b_wptr += sizeof(struct DL_error_ack);
                errmp->b_datap->db_type = M_PCPROTO;
                error_ack->PRIM_type = DL_ERROR_ACK;
                error_ack->ERROR_prim = prim;
                error_ack->LLC_error = dl_err;
                error_ack->UNIX_error = sys_err;
                qreply(q, errmp);
        }
}

/* The following are routines to manipulate software structures */

e503ioctl(q, mp)
queue_t *q;
mblk_t *mp; 
{
	register struct iocblk *iocp;
	long cmd;
	unsigned char *data;

	iocp = (struct iocblk *) mp->b_rptr;

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503ioctl: cmd=0x%x", iocp->ioc_cmd);

	switch (iocp->ioc_cmd) {
	default:
		cmd  = iocp->ioc_cmd;
		data = (unsigned char *) mp->b_cont->b_rptr;
		break;
	}

	switch (cmd) {
#ifdef E503_DEBUG
#define  INB  ('I'<<16|'N'<<8|'B')
	    case INB:
		printf("inb(%x) => 0x%x\n", *(int *)data, inb(*(int *)data));
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		break;

#define  OUTB  ('O'<<24|'U'<<16|'T'<<8|'B')
	    case OUTB:
		outb(*(int *)data, *(int *)(data+sizeof(int)));
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		break;

#define  STAT  ('S'<<24|'T'<<16|'A'<<8|'T')
	    case STAT:
		e503dumpstats(*(int *)data);
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		break;
#endif
	    default:
		mp->b_datap->db_type = M_IOCNAK;
		iocp->ioc_count = 0;
		break;
	}
	qreply(q, mp);
}

static
maj_to_board(m)
{
	register int i;

	for (i=0; i<n3c503unit; i++)
		if (e503major[i] == m)
			break;
	return(i);
}

static char*
eaddrstr(ea)
    unsigned char ea[6];
{  
    static char buf[18];
    char* s = "0123456789abcdef";
    int i;

    for (i=0; i<6; ++i) {
	buf[i*3] = s[(ea[i] >> 4) & 0xf];
	buf[i*3 + 1] = s[ea[i] & 0xf];
	buf[i*3 + 2] = ':';
    }
    buf[17] = 0;
    return buf;
}

u_short
htons(hs)
	u_short         hs;
{
	char            ns[2];

	ns[0] = ((char *) &hs)[1];
	ns[1] = ((char *) &hs)[0];
	return (*(u_short *) ns);
}
