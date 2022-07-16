/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ansi.c	1.3"

/* 
 * IWE ANSI module; parse for character input string for
 * ANSI X3.64 escape sequences and the like. Translate into
 * TCL (terminal control language) comands to send to the
 * principal stream (display) linked under CHANMUX
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <sys/kmem.h>
#include <sys/at_ansi.h>
#include <sys/vt.h>
#include <sys/ascii.h>
#include <sys/kd.h>
#include <sys/ws/chan.h>
#include <sys/ws/tcl.h>
#include <sys/ansi.h>
#include <sys/ddi.h>

int ansi_open(/*queue_t *q, dev_t * devp, int oflag, int sflag*/);
int ansi_close(/*queue_t *q*/);
int ansi_read_queue_put(/*queue_t *q, mblk_t *mp*/);
int ansi_read_queue_serv(/*queue_t *q*/);
int ansi_write_queue_put(/*queue_t *q, mblk_t *mp*/);
int ansi_write_queue_serv(/*queue_t *q, mblk_t *mp*/);

int ansi_debug = 0;

#define	DEBUG1(a)	if (ansi_debug == 1) printf a
#define	DEBUG2(a)	if (ansi_debug >= 2) printf a /* allocations */
#define	DEBUG3(a)	if (ansi_debug >= 3) printf a /* M_CTL Stuff */
#define	DEBUG4(a)	if (ansi_debug >= 4) printf a /* M_READ Stuff */
#define	DEBUG5(a)	if (ansi_debug >= 5) printf a
#define	DEBUG6(a)	if (ansi_debug >= 6) printf a

/*
 * Since most of the buffering occurs either at the stream head or in
 * the "message currently being assembled" buffer, we have no
 * write side queue. Ditto for the read side queue since we have no
 * read side responsibilities.
 */

static struct module_info ansi_iinfo = {
	0,
	"ansi",
	0,
	ANSIPSZ,
	1000,
	100
};

static struct qinit ansi_rinit = {
	ansi_read_queue_put,
	NULL,
	ansi_open,
	ansi_close,
	NULL,
	&ansi_iinfo
};

static struct module_info ansi_oinfo = {
	0,
	"ansi",
	0,
	ANSIPSZ,
	1000,
	100
};

static struct qinit ansi_winit = {
	ansi_write_queue_put,
	ansi_write_queue_serv,
	ansi_open,
	ansi_close,
	NULL,
	&ansi_oinfo
};

struct streamtab ansiinfo = {
	&ansi_rinit,
	&ansi_winit,
	NULL,
	NULL
};

int ansidevflag = 0; /* We are new-style SVR4.0 driver */

void ansi_send_1ctl();

/* ansi_stat_init initializes the queue pair associated with qp --
 * we assume that qp is a read queue ptr -- and the state structure
 * pointed to by ap. We set back ptrs from ap to the read and write
 * queues and set the queues' private data structure ptr to ap.
 * Also, the assorted fields of ap are initialized.
 */

/* STATIC */
 void
ansi_stat_init(qp, ap)
queue_t *qp;
register ansistat_t	*ap;
{
	register int j, oldpri;

	oldpri = splhi(); /* protect against multiple opens */

	if (qp->q_ptr != (caddr_t) NULL)
	{
		/* already allocated state structure */
		splx(oldpri);
		kmem_free ((caddr_t) ap, sizeof(ansistat_t));
		return;
	}

	/* set q_ptr for read and write queue */

	qp->q_ptr = (caddr_t) ap;
	WR(qp)->q_ptr = (caddr_t) ap;

	splx(oldpri);

	/* set queue ptrs in state structure to read/write queue */

	ap->a_rqp = qp;
	ap->a_wqp = WR(qp);
	ap->a_wmsg = (mblk_t *) NULL;

	/* initialize state fields */

	ap->a_state = 0;
	ap->a_gotparam = 0;
	ap->a_curparam = 0;
	ap->a_paramval = 0;
	ap->a_font = ANSI_FONT0;
	ap->a_flags = 0;
	for (j = 0; j < ANSI_MAXPARAMS; j++)
		ap->a_params[j] = 0;

}


/*
 * Character module open. Allocate per-stream state structure. Return an error
 * number for failure, 0 otherwise.
 */

/*ARGSUSED*/

/* STATIC */
 int
ansi_open(qp, devp, oflag, sflag)
	queue_t *qp;
	dev_t *devp;  /* meaningless for modules! */
	int oflag, sflag;
{
	ansistat_t *ap;
	mblk_t *mp;
	ch_proto_t *protop;
	int oldpri;


	if (qp->q_ptr != NULL)
		return (0);		/* already attached */


	/* allocate and initialize state structure */
	if ((ap = (ansistat_t *) kmem_alloc( sizeof(ansistat_t), KM_SLEEP)) == NULL) {
		cmn_err(CE_WARN, "ansi_open: open fails, can't allocate state structure\n");
		return (ENOMEM);
	}

	ansi_stat_init(qp,ap);

	return(0);
}



/* close routine. Deallocate any stashed messages and state structure for
 * that queue pair.
 */

/* STATIC */
 int
ansi_close(qp)
	register queue_t *qp;
{
	ansistat_t *ap = (ansistat_t *)qp->q_ptr;
	register mblk_t *bp;
	int oldpri;

	oldpri=splstr();

	if (ap->a_wmsg != NULL)
	{
		freemsg(ap->a_wmsg);
		ap->a_wmsg = NULL;
	}

	/* Dump the state structure, then unlink it */
	kmem_free(ap, sizeof(ansistat_t));
	qp->q_ptr = NULL;

	splx(oldpri);
}


/*
 * Put procedure for input from driver end of stream (read queue). We don't
 * do any readside processing, so simply call putnext() to forward the
 * data.
 */

ansi_read_queue_put(qp, mp)
	queue_t *qp;
	mblk_t *mp;
{
	switch(mp->b_datap->db_type) {

	default: 
		putnext(qp,mp);
		return;

	case M_PROTO:
	case M_PCPROTO:
		if ((mp->b_wptr - mp->b_rptr) != sizeof (ch_proto_t)) 
			putnext(qp,mp);
		else
			freemsg(mp);
	}
}



/*
 * write side message processing.  Call the appropriate handler for
 * each message type.
 */

/* STATIC */
 void ansi_proc_w_data();

/* STATIC */
 int
ansi_write_queue_put(qp, mp)
	queue_t *qp;
	register mblk_t *mp;
{
	int type;

	type = mp->b_datap->db_type;

	if (type > QPCTL)
	   switch (type) {

	   default: 
		putnext(qp,mp);
		return;

	   case M_START:
	   case M_STOP:
	   case M_FLUSH:
		putq(qp,mp);
		return;
	   }
	else
	   putq(qp,mp);
}


/* STATIC */
 int
ansi_write_queue_serv(qp)
	queue_t *qp;
{

	register mblk_t *mp;
	ansistat_t *ap;

	ap = (ansistat_t *) qp->q_ptr;

	while ((mp = getq(qp)) != NULL) {
	   if (mp->b_datap->db_type <= QPCTL && 
	      ( (ap->a_flags & ANSI_BLKOUT) || !canput(qp->q_next)))
	   {
		putbq(qp, mp);
		return;	/* read side is blocked */
	   }

	   switch(mp->b_datap->db_type) {

	   default:
		putnext(qp, mp);	/* pass it on */
		break;

	   case M_DATA:
		ansi_proc_w_data(qp,mp,ap);
		break;

	   case M_FLUSH:
		/*
	 	 * Flush everything we haven't looked at yet.
		 */
		flushq(qp, FLUSHDATA);
		putnext(qp, mp); /* pass it on */
		break;

	   case M_START:
		ap->a_flags &= ~ANSI_BLKOUT;
		ansi_send_1ctl(ap, TCL_FLOWCTL, TCL_FLOWOFF, M_PCPROTO);
		continue;

	   case M_STOP:
		ap->a_flags |= ANSI_BLKOUT;
		ansi_send_1ctl(ap, TCL_FLOWCTL, TCL_FLOWON, M_PCPROTO);
		continue;

	   } /* switch */
	}
}


/* For each data message coming downstream, ANSI assumes that it is composed
 * of ASCII characters, which are treated as a byte-stream input to the
 * parsing state machine. All data is parsed immediately -- there is
 * no enqueing. Data and Terminal Control Language commands obtained from
 * parsing are sent in the same order in which they occur in the data.
 */

/* STATIC */
 void
ansi_proc_w_data(qp, mp,ap)
	register queue_t *qp;
	register mblk_t *mp;
	register ansistat_t *ap;
{

	register mblk_t *bp;

	bp = mp;

	/* Parse each data block in the message. Assume b_rptr through b_wptr
	 * point to ASCII characters. Release the data message when each
	 * block has been parsed.
	 */

	while (bp) {
	   while ( (unsigned) bp->b_rptr < (unsigned) bp->b_wptr)
		ansi_parse( ap, *bp->b_rptr++);
	   bp = bp->b_cont;
	} 

	/* free the data message we created while parsing characters */
	if (ap->a_wmsg != (mblk_t *) NULL)
	{
		putnext(qp, ap->a_wmsg);
		ap->a_wmsg = (mblk_t *) NULL;
	}

	freemsg(mp); /* release the data message passed down to us */
}


/* sends a TCL command downstream for the principal stream to interpret */

/* STATIC */
 void
ansi_send_ctl(ap, cmd, type)
ansistat_t *ap;
unsigned long cmd;
unsigned long type;
{
	register queue_t *qp;
	register mblk_t *mp;
	register ch_proto_t *protop;

	qp = ap->a_wqp;

	if (ap->a_wmsg) /* transmit current data message */
		putnext(qp, ap->a_wmsg);

	ap->a_wmsg = (mblk_t *) NULL; /* no write data message alloc'd */

	if ((mp = allocb(sizeof(ch_proto_t), BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping ctl msg; cannot alloc msg block");
		return;
	}

	/* set up CHANNEL protocol message and identify it as coming from
	 * a TCL parsing module
	 */

	mp->b_wptr += sizeof (ch_proto_t);
	mp->b_datap->db_type = type;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type  = CH_CTL; 
	protop->chp_stype = CH_TCL;
	protop->chp_stype_cmd = cmd; /* set the command to be performed */

	putnext(qp, mp); /* ship it down */
}



/* 
 * Allocate a 1-argument TCL command to send downstream.
 */

/* STATIC */
 void
ansi_send_1ctl(ap, cmd, data, type)
ansistat_t *ap;
unsigned long cmd;
unsigned long data;
unsigned long type;
{
	register queue_t *qp;
	register mblk_t *mp;
	register ch_proto_t *protop;

	qp = ap->a_wqp; /* write queue ptr */

	if (ap->a_wmsg)
	{
		/* transmit current data message */
		putnext(qp, ap->a_wmsg);
		ap->a_wmsg = (mblk_t *) NULL; /* no write data message
					       * alloc'd now */
	}

	if ((mp = allocb(sizeof(ch_proto_t), BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping ctl msg; cannot alloc msg block");
		return;
	}


	mp->b_wptr += sizeof (ch_proto_t);
	mp->b_datap->db_type = type;

	/* set up CHANNEL protocol message and identify it as coming from
	 * a TCL parsing module
	 */
	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type  = CH_CTL;
	protop->chp_stype = CH_TCL;
	protop->chp_stype_cmd = cmd; /* cmd to perform */
	protop->chp_stype_arg = data; /* cmd to perform */

	putnext(qp, mp);
}




/*
 * send the appropriate control message or set state based on the
 * value of the control character ch
 */

ansi_control(ap, ch)
register ansistat_t *ap;
unsigned char ch;
{
	ap->a_state = 0;
	switch (ch) {
	case A_BEL:
		ansi_send_ctl(ap, TCL_BELL, M_PROTO);
		break;

	case A_BS:
		ansi_send_ctl(ap, TCL_BACK_SPCE, M_PROTO);
		break;

	case A_HT:
		ansi_send_ctl(ap, TCL_H_TAB, M_PROTO);
		break;

	case A_NL:
		ansi_send_ctl(ap, TCL_NEWLINE, M_PROTO);
		break;

	case A_VT:
		ansi_send_ctl(ap, TCL_V_TAB, M_PROTO);
		break;

	case A_FF:
		ansi_send_ctl(ap, TCL_DISP_RST, M_PROTO);
		break;

	case A_CR:
		ansi_send_ctl(ap, TCL_CRRGE_RETN, M_PROTO);
		break;

	case A_SO:
		ansi_send_ctl(ap, TCL_SHFT_FT_OU, M_PROTO);
		break;

	case A_SI:
		ansi_send_ctl(ap, TCL_SHFT_FT_IN, M_PROTO);
		break;

	case A_ESC:
	case A_CSI:
		ap->a_state = 1;
		break;

	case A_GS:
		ansi_send_ctl(ap, TCL_BACK_H_TAB, M_PROTO);
		break;

	default:
		break;
	}
}
		

/* 
 * if parameters [0..count - 1] are not set, set them to the value of newparam.
 */

ansi_setparam(ap, count, newparam)
register ansistat_t *ap;
register int count;
register ushort newparam;
{
	register int i;

	for (i = 0; i < count; i++) {
		if (ap->a_params[i] == -1)
			ap->a_params[i] = newparam;
	}
}


/*
 * select graphics mode based on the param vals stored in a_params
 */
ansi_selgraph(ap) 
register ansistat_t *ap;
{
	register short curparam;
	register int count = 0;
	short param;

	curparam = ap->a_curparam;
	do {
		param = ap->a_params[count];

		/* param == -1 means no parameters, same as 0 */
		if (param == -1) 
			param = 0;

		if (param == 10) {	/* Select primary font */
			ap->a_font = ANSI_FONT0;
			ansi_send_ctl(ap, TCL_SHFT_FT_OU, M_PROTO); /* Shift font in*/
		}
		else if (param == 11) 	/* First alternate font */
			ap->a_font = ANSI_FONT1;

		else if (param == 12) 	/* Second alternate font */
			ap->a_font = ANSI_FONT2;

		else
			ansi_send_1ctl(ap, TCL_SET_ATTR, param, M_PROTO);
		count++;
		curparam--;

	} while (curparam > 0);


	ap->a_state = 0;
}



/* send multi-byte move cursor TCL message to principal stream */

/* STATIC */
 void
ansi_mvcursor(ap, x_coord, x_type, y_coord, y_type)
ansistat_t *ap;
short x_coord, y_coord; /* either absolute coordinates or deltas */
unchar x_type, y_type;  /* either TCL_POSABS or TCL_POSREL */
{
	queue_t *qp;
	register mblk_t *mp;
	register tcl_data_t *tp;
	register ch_proto_t *protop;

	qp = ap->a_wqp; /* write queue pointer */

	if (ap->a_wmsg) /* transmit current data message */
		putnext(qp, ap->a_wmsg);

	ap->a_wmsg = (mblk_t *) NULL; /* no write data message alloc'd */

	if ((mp = allocb(sizeof(ch_proto_t), BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping cursor move msg; cannot alloc msg block");
		return;
	}

	/* set up CHANNEL protocol message; identify it as TCL command 
	 * TCL_POS_CURS.
	 */

	mp->b_wptr += sizeof (ch_proto_t);
	mp->b_datap->db_type = M_PROTO;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type  = CH_CTL;
	protop->chp_stype = CH_TCL;
	protop->chp_stype_cmd = TCL_POS_CURS;

	/* allocate data block for move cursor cmd data */
	if ((mp->b_cont = allocb(sizeof(tcl_data_t), BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping cursor move msg; cannot alloc msg block");
		freemsg(mp);
		return;
	}

	/* tcl_data_t is a union including a data structure used
	 * to send down the x,y change to the principal stream
	 */
	mp->b_cont->b_wptr += sizeof (tcl_data_t);
	mp->b_cont->b_datap->db_type = M_PROTO;
	tp = (tcl_data_t *) mp->b_cont->b_rptr;

	tp->mv_curs.delta_x = x_coord;
	tp->mv_curs.x_type = x_type;
	tp->mv_curs.delta_y = y_coord;
	tp->mv_curs.y_type = y_type;

	putnext(qp,mp);
}


/*
 * perform the appropriate action for the escape sequence
 */
ansi_chkparam(ap, ch) 
register ansistat_t *ap;
unchar ch;
{
	int i;

	switch (ch) {

	case 'k': /* select key click. NOT ANSI X3.64!! */
		if (ap->a_params[0] == 0)
			ansi_send_ctl(ap, TCL_KEYCLK_OFF, M_PROTO);  
		else if (ap->a_params[0] == 1)
			ansi_send_ctl(ap, TCL_KEYCLK_ON, M_PROTO);  
		break;

	case 'c': { /* select cursor type. NOT ANSI X3.64!! */
		ushort i;
		ansi_setparam(ap, 1, 0);
		i = ap->a_params[0];
		if (i > 2) /* only 0, 1 and 2 are valid */
			break;
		ansi_send_1ctl(ap, TCL_CURS_TYP, i, M_PROTO);
		break;
	   }

	case 'm': /* select terminal graphics mode */
		ansi_selgraph(ap);
		break;

	case '@':		/* insert char */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_INSRT_CHR, ap->a_params[0], M_PROTO);
		break;

	case 'A':		/* cursor up */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, 0, TCL_POSREL, -ap->a_params[0], TCL_POSREL);
		break;

	case 'd':		/* VPA - vertical position absolute */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, 0, TCL_POSREL, ap->a_params[0], TCL_POSABS);
		break;

	case 'e':		/* VPR - vertical position relative */
	case 'B':		/* cursor down */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, 0, TCL_POSREL, ap->a_params[0], TCL_POSREL);
		break;

	case 'a':		/* HPR - horizontal position relative */
	case 'C':		/* cursor right */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, ap->a_params[0], TCL_POSREL, 0, TCL_POSREL);
		break;

	case '`':		/* HPA - horizontal position absolute */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, ap->a_params[0], TCL_POSABS, 0, TCL_POSREL);
		break;

	case 'D':		/* cursor left */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, -ap->a_params[0], TCL_POSREL, 0, TCL_POSREL);
		break;

	case 'E':		/* cursor next line */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, 0, TCL_POSABS, ap->a_params[0], TCL_POSREL);
		break;

	case 'F':		/* cursor previous line */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, 0, TCL_POSABS, -ap->a_params[0], TCL_POSREL);
		break;

	case 'G':		/* cursor horizontal position */
		ansi_setparam(ap, 1, 1);
		ansi_mvcursor(ap, ap->a_params[0] - 1, TCL_POSABS, 0, TCL_POSREL);
		break;

	case 'g':		/* clear tabs */
		ansi_setparam(ap, 1, 0);
		if (ap->a_params[0] == 3)  /* clear all tabs */
			ansi_send_ctl(ap,TCL_CLR_TABS, M_PROTO);
		if (ap->a_params[0] == 0)
			ansi_send_ctl(ap,TCL_CLR_TAB, M_PROTO); /* clr tab at cursor */
		break;

	case 'f':		/* HVP */
	case 'H':		/* position cursor */
		ansi_setparam(ap, 2, 1);
		ansi_mvcursor(ap, ap->a_params[1] - 1, TCL_POSABS,
			       ap->a_params[0] -1, TCL_POSABS);
		break;

	case 'i':		/* MC - Send screen to host */
		if (ap->a_params[0] == 2)
			ansi_send_ctl(ap, TCL_SEND_SCR, M_PROTO);
		break;

	case 'I':		/* NO_OP entry */
		break;

	case 'h':		/* SM - Lock keyboard */
	case 'l':		/* RM - Unlock keyboard */
		if (ap->a_params[0] == 2)
		{
			if (ch == 'h') /* lock */
				ansi_send_ctl(ap, TCL_LCK_KB, M_PROTO);
			else
				ansi_send_ctl(ap, TCL_UNLCK_KB, M_PROTO);
		}
		break;

	case 'J':		/* erase screen */
		ansi_setparam(ap, 1, 0);
		if (ap->a_params[0] == 0)
			/* erase cursor to end of screen */
			ansi_send_ctl(ap, TCL_ERASCR_CUR2END, M_PROTO);

		else if (ap->a_params[0] == 1)
			/* erase beginning of screen to cursor */
			ansi_send_ctl(ap, TCL_ERASCR_BEG2CUR, M_PROTO);

		else
			/* erase whole screen */
			ansi_send_ctl(ap, TCL_ERASCR_BEG2END, M_PROTO);

		break;

	case 'K':		/* erase line */
		ansi_setparam(ap, 1, 0);
		if (ap->a_params[0] == 0)
			/* erase cursor to end of line*/
			ansi_send_ctl(ap, TCL_ERALIN_CUR2END, M_PROTO);

		else if (ap->a_params[0] == 1)
			/* erase beginning of line to cursor */
			ansi_send_ctl(ap, TCL_ERALIN_BEG2CUR, M_PROTO);

		else
			/* erase whole line */
			ansi_send_ctl(ap, TCL_ERALIN_BEG2END, M_PROTO);

		break;

	case 'L':		/* insert line */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_INSRT_LIN, ap->a_params[0], M_PROTO);
		break;

	case 'M':		/* delete line */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_DELET_LIN, ap->a_params[0], M_PROTO);
		break;

	case 'P':		/* delete char */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_DELET_CHR, ap->a_params[0], M_PROTO);
		break;

	case 'S':		/* scroll up */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_SCRL_UP, ap->a_params[0], M_PROTO);
		break;

	case 'T':		/* scroll down */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_SCRL_DWN, ap->a_params[0], M_PROTO);
		break;

	case 'X':		/* erase char */
		ansi_setparam(ap, 1, 1);
		ansi_send_1ctl(ap, TCL_DISP_CLR, ap->a_params[0], M_PROTO);
		break;

	case 'Z':		/* cursor backward tabulation */
		ansi_setparam(ap, 1, 1);
		for (i=0; i < ap->a_params[0]; i++)
			ansi_send_ctl(ap, TCL_BACK_H_TAB, M_PROTO);
		break;

	}
	ap->a_state = 0; 
}



/* send a CHANNEL proto message to change the function key string
 * This message will actually be handled by CHAR or its equivalent,
 * which must be pushed below ANSI.
 */

/* STATIC */
 void
ansi_addstring(ap, keynum, buf, len)
ansistat_t *ap;
ushort keynum, len;
char *buf;
{
	queue_t *qp;
	register mblk_t *mp;
	register tcl_data_t *tp;
	register ch_proto_t *protop;

	/* return automatically if buf or len not set */
	if (!len || buf == (char *) NULL) return;

	qp = ap->a_wqp;

	if (ap->a_wmsg) /* xmit current data message */
		putnext(qp, ap->a_wmsg);

	ap->a_wmsg = (mblk_t *) NULL; /* no write data message alloc'd */

	if ((mp = allocb(sizeof(ch_proto_t), BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping cursor move msg; cannot alloc msg block");
		return;
	}


	/* make CHANNEL control message of type TCL_ADD_STR */
	mp->b_wptr += sizeof (ch_proto_t);
	mp->b_datap->db_type = M_PROTO;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_type  = CH_CTL;
	protop->chp_stype = CH_TCL;
	protop->chp_stype_cmd = TCL_ADD_STR;

	/* allocate data block for len, key arguments and string. 
	 * which will be stored at end of data
	 */

	if ((mp->b_cont = allocb(sizeof(tcl_data_t)+len, BPRI_MED)) == NULL) {
		cmn_err(CE_WARN,"ansi: dropping addstr msg; cannot alloc msg block");
		freemsg(mp);
		return;
	}

	mp->b_cont->b_wptr += sizeof(tcl_data_t) + len;
	tp = (tcl_data_t *)mp->b_cont->b_rptr;
	tp->add_str.len = len;
	tp->add_str.keynum = keynum;

	/*
	 * Now we copy the string to the end of the data block
	 */

	bcopy (buf, ++tp, len); /* ++tp should point at space allocated past
				 * past the tcl_data_t structure
				 */

	putnext (qp,mp);
}


/*
 * Gather the parameters of an ANSI escape sequence
 */
ansi_getparams(ap, ch) 
register ansistat_t	*ap;
unchar				ch;
{
	if ((ch >= '0' && ch <= '9') && ap->a_state != 21)
	{	/* Number? */
		ap->a_paramval = ((ap->a_paramval * 10) + (ch - '0'));	
		ap->a_gotparam++;	/* Remember got parameter */
		return;			/* Return immediately */
	}
	switch (ap->a_state) {		/* Handle letter based on state */

	case 20:			/* <ESC>Q<num> ? */
		ap->a_params[1] = ch;	/* Save string delimiter */
		ap->a_params[2] = 0;	/* String length 0 to start */
		ap->a_state = 21;		/* Read string next */
		break;

	case 21:			/* <ESC>Q<num><delm> ? */
		if (ch == ap->a_params[1])
		{	/* End of string? */
			ansi_addstring(ap, ap->a_paramval, ap->a_fkey, ap->a_params[2]);
			ap->a_state = 0;
			/* End of <ESC> sequence */
		}
		else if (ch == '^')
			/* Control char escaped with '^'? */
			ap->a_state = 22;
			/* Read control character next */

		else if (ch != '\0')
		{	/* Not a null? Add to string */
			ap->a_fkey[ap->a_params[2]++] = ch;
			if (ap->a_params[2] >= ANSI_MAXFKEY)	/* Full? */
				ap->a_state = 0;
				/* End of <ESC> sequence */
		}
		break;

	case 22:			/* Contrl character escaped with '^' */
		ap->a_state = 21;		/* Read more of string later */
		ch -= ' ';		/* Convert to control character */
		if (ch != '\0')
		{	/* Not a null? Add to string */
			ap->a_fkey[ap->a_params[2]++] = ch;
			if (ap->a_params[2] >= ANSI_MAXFKEY)	/* Full? */
				ap->a_state = 0;
				/* End of <ESC> sequence */
		}
		break;

	default:			/* All other states */
		if (ap->a_gotparam)
		{	/* Previous number parameter? Save and
			 * point to next free parameter.
			 */
			ap->a_params[ap->a_curparam] = ap->a_paramval;
			ap->a_curparam++;
		}

		if (ch == ';')
		{	/* Multiple param separator? */
			ap->a_gotparam = 0;	/* Restart parameter search */
			ap->a_paramval = 0;	/* No parameter value yet */
		}
		else 	/* Regular letter */
			ansi_chkparam(ap, ch);	/* Handle escape sequence */
		break;
	}
}


/* add character to data message block being formed to send downstream
 * when received data message is through being parsed
 */

/* STATIC */
 void
ansi_outch(ap, ch)
ansistat_t *ap;
unchar ch;
{
	register mblk_t *mp;
	queue_t *qp;

	qp = ap->a_wqp;
	mp = ap->a_wmsg;

	if (ap->a_font == ANSI_FONT2)
		ch |= 0x80;
	if ( mp == (mblk_t *) NULL || mp->b_datap->db_lim == mp->b_wptr)
	{
		if (mp) putnext(qp,mp);
		if ( (mp = allocb(ANSIPSZ, BPRI_MED)) == (mblk_t *) NULL)
		{
			cmn_err(CE_WARN,"ansi: cannot allocate data in ansi_outch");
			return;
		}
	}

	*mp->b_wptr++ = ch;
	ap->a_wmsg = mp;
}


/* state machine parser based on the current state and character input 
 * major transitions are to control character or normal character
 */

/* STATIC */

ansi_parse(ap, ch)
register ansistat_t *ap;
unchar ch;
{
	mblk_t *mp;
	int cnt;
	char *strp;
	int i;

	if (ap->a_state == 0) {	/* Normal state? */ 
		if (ch == A_ESC ||
		    ch == A_CSI ||
		    (ch < ' ' && ap->a_font == ANSI_FONT0))	/* Control? */
			ansi_control(ap, ch);	/* Handle control */
		else 
			ansi_outch(ap, ch);	/* Display */
	}
	else {			/* In <ESC> sequence */
		if (ap->a_state != 1)	/* Need to get parameters? */
			ansi_getparams(ap, ch);
		else {			/* Previous char was <ESC> */	
			if (ch == '[') {
				ap->a_curparam = 0;
				ap->a_paramval = 0;
				ap->a_gotparam = 0;
				/* clear the parameters */
				for (i = 0; i < ANSI_MAXPARAMS; i++)
					ap->a_params[i] = -1;
				ap->a_state = 2;
			}
			else if (ch == 'Q') {	/* <ESC>Q ? */
				ap->a_curparam = 0;
				ap->a_paramval = 0;
				ap->a_gotparam = 0;
				for (i = 0; i < ANSI_MAXPARAMS; i++)
					ap->a_params[i] = -1;	/* Clear */
				ap->a_state = 20;	/* Next get params */
			}
			else if (ch == 'C') {	/* <ESC>C ? */
				ap->a_curparam = 0;
				ap->a_paramval = 0;
				ap->a_gotparam = 0;
				for (i = 0; i < ANSI_MAXPARAMS; i++)
					ap->a_params[i] = -1;	/* Clear */
				ap->a_state = 30;	/* Next get params */
			}
			else {
				ap->a_state = 0;
				if (ch == 'c')
					/* ESC c resets display */
					ansi_send_ctl(ap, TCL_DISP_RST, M_PROTO);
				else if (ch == 'H')
					/* ESC H sets a tab */
					ansi_send_ctl(ap, TCL_SET_TAB, M_PROTO);
				 /* check for control chars */
				else if (ch < ' ')
					ansi_control(ap, ch);
				else
					ansi_outch(ap, ch);
			}
		}
	}
}
