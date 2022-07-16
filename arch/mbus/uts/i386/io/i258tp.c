/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/i258tp.c	1.4.3.3"

#ifndef lint
static char i258tpcopyright[] = "Copyright 1988, 1989, 1990 Intel Corporation 462851";
#endif /* lint */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/fdisk.h"
#include "sys/vtoc.h"
#include "sys/ivlab.h"
#include "sys/alttbl.h"
#include "sys/bbh.h"
#include "sys/tape.h"

#include "sys/tuneable.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/kmem.h"

#define b_dev b_edev		/* SVR4.0 thing */

#include "sys/i258.h" 			/* order is important !!! */
#include "sys/ddi.h"
#include "sys/file.h"

#ifdef wakeup
#undef wakeup
#endif

extern unsigned char	ics_myslotid();	/* From ics.c */

/* Values configured in space.c */

extern	int		i258_tapesize;		/* Size of tape device in 512 byte blks */
extern	int		N_i258;				/* Number of boards */
extern	int		i258retry;			/* Number of retries on soft error */
extern	int		i258_max_req;		/* Max outstanding requests per brd */
extern	struct	iobuf	i258tab[];	/* Queue headers, one per board */
extern	int		i25t_elements;		/* Size of i25tbuf */
extern	struct	i258tp_buf i25tbuf[];/* tape buffering - one per unit */

extern 	minor_t	i258maxmin;			/* maximum minor device number */
extern	minor_t	i258bases[];		/* Base minor #s for winis. */

extern	struct i258cfg	i258cfg[];		/* 258 "configuraton" */
extern	struct i258dev	i258dev[];		/* per-board device data */
extern	struct i258minor i258minor[];	/* minor number bit map */
extern	struct alt_info i258_alts[];	/* Alternate table space */

#define EOK			0x0L

#ifdef DEBUG
#define	DEB_OPEN	0x0001	/* i25topen */
#define	DEB_STRAT	0x0002	/* i25tstrategy */
#define DEB_IOCTL	0x0004	/* i25tioctl */
#define DEB_BUF		0x0008	/* i25tbufcmd */
#define DEB_BIOCTL	0x0010	/* i25tioctl break into monitor*/
#define DEB_MIOCTL	0x0020	/* i25tioctl show message*/
#define DEB_PERF	0x0040	/* i25tperform */
#define DEB_MPERF	0x0080	/* i25tperform show message */
#define DEB_BPERF	0x0100	/* i25tperform break */
#define DEB_SET		0x0200	/* i25tsetup */
#define DEB_BUFB	0x0400	/* i25tbufcmd: breakpoint */
#define DEB_CLOSE	0x0800	/* i25tclose */

unsigned long	i25t_debug = 0;
#define	DEBPR(x,y)	if(i25t_debug & (x)) cmn_err y
#else
#define	DEBPR(x,y)
#endif

#define SETERR(x)	error_code = x
#define ERRRET(x)	return(x)

extern unchar i258checkerr();
extern void i258timeout();
extern int i258istart();
extern int i258getreq();
extern struct i258drtab *i258drtab_struct();

int
i25tperform();

/*****************************************************************************
 *
 * 	Open a unit.  Sets a given partition (i.e. special file) open.
 *
 *	Due to the limitations of the QIC2 interface, only one tape drive
 *	per controller can be open at once.  We also use this feature to
 *	make sure that only one user has the device open at once.
 *
 ****************************************************************************/
i25topen(devp, flag, otyp, cred_p)
dev_t	*devp;
int		flag;
int		otyp;			/* not used */
struct	cred 	*cred_p;
{
	struct 		i258dev *dd;
	unsigned 	brd;
	unsigned 	unit;
	unsigned 	x;
	struct		i258cdrt  *cdr; 
	int 		error_code;
	int     	i, j, k;
	dev_t 		dev;

	dev = *devp;
	error_code = EOK;

	DEBPR(DEB_OPEN,(CE_CONT,"i25topen: dev=%x\n",dev));

	/*
	 * Make sure board and minor number exist.
	 */

	if (getminor(dev) > i258maxmin)
		return(ENXIO);

	if ((brd = BOARD(dev)) >= N_i258)
		return(ENXIO);

	if (flag & (FAPPEND | FTRUNC))
		return(EINVAL);

	unit = UNIT(dev);
	cdr = &i258cfg[brd].c_drtab[unit&TID_MASK][DRTAB(dev)];

	x = SPL();

	/* initializations */

	dd = &i258dev[brd];
	
 	DEBPR(DEB_OPEN,(CE_CONT,"i25topen: dev=%x, unit=%d, flags =%x\n",
 			dev,unit, dd->d_sflags[unit]));

	while(dd->d_flags & I258_INITTING)
		(void) sleep((caddr_t)&dd->d_flags, PZERO);

	if ( !(dd->d_flags & I258_ALIVE) ) {
		dd->d_flags |= I258_INITTING;
		i258istart();
		dd->d_flags &= ~I258_INITTING;
	}

	if ( !(dd->d_flags & I258_ALIVE) )
		goto endopen;

	if (!ISTAPE(dev)) {
		/*
		 * Figure out the actual units connected on each Target Id.
		 * Do it for all device types because logic at other
		 * places can possibly decide that it is a TAPE if ISWINI and ISFLOP 
		 * macro fail.
		 */
		if ( i258unitmap (dev,WINI_TYPE) || i258unitmap (dev,FLOPPY_TYPE) || 
				i258unitmap (dev,TAPE_TYPE) ) { 
			cmn_err(CE_PANIC, "i258tp: Cannot query the controller\n");
		}

		/*
		 * Assign unit numbers to the i258winidata and i258rdvfy structures. 
		 */
		i258assignstructs (brd, dd)	;

		for (j = 0; j < I258_QUERY_CONTROLLER_SIZE; j++) {
			if ( (dd->t_units[j]) & 0x1) {
				/* There is a tape here. Scan list of configured tape units */ 
				for (k = 0; k < i25t_elements; k++) {
					if (i25tbuf[k].unit == j) {
						/* configured tape unit present */
						i = 0;
						do {
							/* now assign it in the i258dev structure */
							if (dd->tbuf[i].unit == j) 
								break;		/* already accounted */
							if (dd->tbuf[i].unit == 0xff) {
								/* found an unused one */
								dd->tbuf[i].unit = j;
								dd->tbuf[i].bufpages = 
										i25tbuf[k].bufpages;
								break;
							}
							++i;
						}
						while (i < I258_NUMTAPE);
					}
				}
			}
		}	
	}
	/*
	 * Make sure that the device exists.
	 */
	if (!ISTAPE(dev))
		goto endopen;

	/*
	 * Make sure no one else is opening this unit.
	 */
	while (dd->d_sflags[unit] & SF_OPENCLOSE)
		(void) sleep ((caddr_t) &dd->d_sflags[unit], PZERO);

	/*
	 * Only one process can open a tape on a controller at a time.
	 * t_units contains info regarding the tapes connected to the system.
	 */
	if (dd->d_sflags[unit] & SF_OPEN) {
		SETERR(EBUSY);
		goto endopen;
	}
	dd->d_sflags[unit] |= SF_OPENCLOSE;

	/*
	 * If we aren't watching for timeout already, do so.
	 */
	if(!(dd->d_flags & I258_TIMEOUT)) {
		dd->d_flags |= I258_TIMEOUT;
		(void) timeout(i258timeout, (caddr_t)dd, (WATCH_TIME*HZ));
	}
	i258dinit (BOARD(dev), dev, cdr);
	/*
	 * Before we go any farther, reserve the device if needed.
	 */
	if(!(dd->d_sflags[unit] & SF_RESERVED)) {
		if (i258resrel(dev,1)) {
			cmn_err(CE_WARN,"i25topen: Cannot reserve device 0x%x\n",dev);
			SETERR(EBUSY);
			goto endopen;
		}
		dd->d_sflags[unit] |= SF_RESERVED;
	}
	/*
	 * Allocate external buffers
	 * and open the device.
	 */
	i25tbufcmd(dev, (struct buf *)0, TP_GETBUF);

	dd->d_sflags[unit] |= (SF_OPEN|SF_READY);
	if ((cdr->cdr_flags & DR_RETENSION) == DR_RETENSION) 
		i25tioctl(dev, T_RETENSION, 0, 0, 0, 0);

endopen:
	dd->d_sflags[unit] &= ~SF_OPENCLOSE;
	(void) wakeup((caddr_t)&dd->d_sflags[unit]);
	splx(x);
 	DEBPR(DEB_OPEN,(CE_CONT,"i25topen: flags=%x\n",dd->d_sflags[unit]));
	return (error_code);
}


/******************************************************************************
 *
 * Close a unit.
 *
 *	Called on last close of a partition; thus, "close" the
 *	partition.  If this was last partition, mark the unit
 *	closed and not-ready.  In this case, next open will
 *	re-initialize.
 *
 *****************************************************************************/
i25tclose(dev, flag, otyp, cred_p)
register	dev_t	dev;	/* major, minor numbers */
int					flag;		/* not used */
int					otyp;		/* not used */
struct 	cred 		*cred_p;
{
	struct i258dev	*dd = &i258dev[BOARD(dev)];
	unsigned	unit = UNIT(dev);
	unsigned	s;
	struct 		i258cfg *cf = &i258cfg[BOARD(dev)];
	struct		i258cdrt  *cdr = &cf->c_drtab[unit&TID_MASK][DRTAB(dev)];
	int			index;

 	DEBPR(DEB_CLOSE,(CE_CONT,"i25tclose: dev=%x, unit %d, flags =%x \n",
 		dev, index,dd->d_sflags[unit]));
	s = SPL();

	for (index = 0; index < I258_NUMTAPE; index++) {
		if (dd->tbuf[index].unit == unit) 
			break;
	}
	/*
	 * Make sure no one else is opening or closing this unit.
	 */
	while (dd->d_sflags[unit] & SF_OPENCLOSE) {
		(void) sleep ((caddr_t) &dd->d_sflags[unit], PZERO);
	}

	dd->d_sflags[unit] |= SF_OPENCLOSE;

	/*
	 * First flush out the buffers, then deallocate them.
	 * Rewind tape on close.
	 */
	if(dd->tbuf[index].bufpages != 0) {
		i25tbufcmd(dev, (struct buf *)0, TP_FLUSH);
		i25tbufcmd(dev, (struct buf *)0, TP_FREEBUF);
	}

	if ((cdr->cdr_flags & DR_NO_REWIND) == DR_NO_REWIND) {
		if(dd->d_tstate[unit] == TS_WRITING)
			i25tioctl(dev, T_WRFILEM, 0, 0, 0, 0);
		else if(dd->d_tstate[unit] == TS_READING)
			i25tioctl(dev, T_SFF, 1, 0, 0, 0);
		else	/* t_state == TS_NOTHING */
			dd->d_sflags[unit] &= ~(SF_EOF_MARK|SF_NEED_EOF);
	} else
		i25tioctl(dev, T_RWD, 0, 0, 0, 0);

	while (i258checkreq(dd,unit)) {
		dd->d_sflags[unit] |= SF_CLOSEWAIT;
		(void) sleep((caddr_t)&dd->d_pdev[unit],PRIBIO);
	}

	if(!(dd->d_sflags[unit] & SF_HOLD_RSRV)) {
		if (i258resrel(dev,0))
			cmn_err(CE_WARN,"i25tclose: Cannot release device 0x%x\n",dev);
		dd->d_sflags[unit] &= ~SF_RESERVED;
	}

	dd->d_sflags[unit] &= ~(SF_OPEN|SF_OPENCLOSE|SF_CLOSEWAIT);
	(void) wakeup((caddr_t)&dd->d_sflags[unit]);
	splx(s);
 	DEBPR(DEB_CLOSE,(CE_CONT,"i25tclose: flags=%x\n",dd->d_sflags[unit]));
	return (EOK);
}


/*****************************************************************************
 *
 * 		Try to start an I/O request.  Queue it if there are no
 *		available request slots.
 *
 *	Note:	check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.  258 gives
 *		not-ready status to those requests that are attempted
 *		before a new volume is inserted.  Once a new volume is
 *		inserted, would get good I/O's to wrong volume.
 *
 *	Note:	The partition-check algorithm insists that requests must
 *		not cross a sector boundary.  If partition size is not a
 *		multiple of BSIZE, the last few sectors in the partition
 *		are not accessible.
 *
 ****************************************************************************/
void
i25tstrategy(bp)
register struct buf *bp;	/* buffer header */
{
	register struct i258dev	 *dd;
	unsigned	 brd;
	unsigned	unit;
	int			index;

	/* initializations */

	if (getminor(bp->b_dev) > i258maxmin) {
		bp->b_error = ENXIO;
		return;
	}

	if (((brd = BOARD(bp->b_dev)) >= N_i258) || (!ISTAPE(bp->b_dev))) {
		bp->b_error = ENXIO;
		return;
	}

	/* set b_resid to b_bcount because we haven't done anything yet */

	unit = UNIT(bp->b_dev);
	dd = &i258dev[brd];

	for (index = 0; index < I258_NUMTAPE; index++) {
		if (dd->tbuf[index].unit == unit) 
			break;
	}
	if ((dd->d_sflags[unit] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;		/* not ready */
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	/* Check that the request is of 512 byte granularity.	*/
	/* Courtesy of the tape drive characteristics.		*/

	if ((bp->b_bcount % TAPE_DEV_GRAN) != 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		biodone(bp);
		return;
	}

	bp->b_resid = bp->b_bcount;

	/*
	 * Use read ahead/write behind buffers if they were
	 * allocated. If tape state changes from reading to writing
	 * or vice versa then buffers must first be flushed before
	 * tape state changes.
	 */
	if(dd->tbuf[index].bufpages) {
		if(bp->b_flags&B_READ) {
			if(dd->d_tstate[unit] == TS_WRITING) {
				i25tbufcmd(bp->b_dev, bp, TP_FLUSH);
				i25tioctl(bp->b_dev, T_WRFILEM, 0, 0, 0, 0);
			}
			i25tbufcmd(bp->b_dev, bp, TP_RDBUF);
		} else {
			if(dd->d_tstate[unit] == TS_READING) {
				i25tbufcmd(bp->b_dev, bp, TP_FLUSH);
				i25tioctl(bp->b_dev, T_SFF, 1, 0, 0, 0);
			}
			i25tbufcmd(bp->b_dev, bp, TP_WRBUF);
		}
	} else {
		/*
		 * If we are going to switch directions, make sure that
		 * it is handled here.
		 */
		if(bp->b_flags&B_READ) {
			if(dd->d_sflags[unit] & SF_EOF_MARK) {
				dd->d_sflags[unit] &= ~SF_EOF_MARK;
				bp->b_flags &= ~B_ERROR;
				biodone(bp);
				return;
			}
			if (dd->d_tstate[unit] == TS_WRITING)
				i25tioctl(bp->b_dev, T_WRFILEM, 0, 0, 0, 0);
		} else {
			if (dd->d_tstate[unit] == TS_READING)
				i25tioctl(bp->b_dev, T_SFF, 1, 0, 0, 0);
		}

		i25tsetup(bp,0);
	}
	return;
}

/*****************************************************************************
 *
 * 		"Raw" read.  Uses dma_breakup (via i25tbreakup)
 *		to transform raw requests into requests that contain
 *		only contiguous memory.  Sometime this will be changed
 *		to use the scatter/gather feature of the DMA on the
 *		386 MB2 boards.
 *
 ****************************************************************************/
i25tread(dev, uio_p, cred_p)
register 	dev_t	dev;
struct		uio	*uio_p;
struct		cred	*cred_p;
{ 
	int	error_code;
	int	num_blks;

	num_blks = i25tsize(dev);
	error_code = physiock(i25tstrategy, NULL, dev, B_READ, num_blks, uio_p);
	return (error_code);
}

/*****************************************************************************
 *
 * 		"Raw" write.  Uses dma_breakup (via i25tbreakup)
 *		to transform raw requests into requests that contain
 *		only contiguous memory.  Sometime this will be changed
 *		to use the scatter/gather feature of the DMA on the
 *		386 MB2 boards.
 *
 *
 ****************************************************************************/
i25twrite(dev, uio_p, cred_p)
register 	dev_t	dev;
struct		uio		*uio_p;
struct		cred	*cred_p;
{ 
	int	error_code;
	int	num_blks;

	num_blks = i25tsize(dev);
	error_code = physiock(i25tstrategy, NULL, dev, B_WRITE, num_blks, uio_p);
	return (error_code);
}

/******************************************************************************
 *
 * 	Print an error message when called from the kernel via bdevsw.
 *
 ****************************************************************************/
i25tprint (dev,str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE, "%s on disk unit %d, partition %d\n",
		str, UNIT(dev), PARTITION(dev));
	return (EOK);
}

/*****************************************************************************
 *
 *	i258 tape driver special functions.
 *
 *	T_ERASE:  Erases a tape; this also retensions the tape.  If
 *		given while t_state == TS_READING, the driver will dump
 *		data until a file mark is found.  If TS_WRITING, a file
 *		mark will be written to get us out of TS_WRITING.
 *
 *	T_LOAD, T_UNLOAD:  Implemented as T_REWIND.  Except for
 *		ejection of media on some devices, end result is same.
 *
 *	T_RETENSION:  Retensions the tape.  Runs the tape to the end of
 *		the first track, then back to the LLP to ensure that the
 *		tape is stacked evenly on the cartridge.  See T_ERASE
 *		regarding t_state.
 *
 *	T_RWD:  Rewinds the tape to the logical load point.  See
 *		T_ERASE regarding t_state.
 *
 *	T_SFF:  Seek forward a file mark.  If t_state is TS_READING,
 *		will be translated to just dump data until a file mark
 *		is found, and the TS_READING state will be terminated.
 *		If TS_WRITING, command will simply be rejected.
 *
 *	T_SBF, T_SFB, T_SBB:  Rejected, since these are not implemented.
 *
 *	T_SFREC, T_SBREC:  Seek forward/backward a record (Kennedy drives
 *		only).  Rejected, since Kennedy drives are not supported.
 *
 *	T_WRFILEM:  Writes a file mark at the current position on the tape.
 *
 ****************************************************************************/
i25tioctl(dev, cmd, cmdarg, flag, cred_p, rval_p)
dev_t		dev;		/* major, minor numbers */
int			cmd;		/* command code */
ulong		cmdarg;		/* user structure with parameters */
int			flag;		/* not used */
struct		cred 	*cred_p;
int			*rval_p;
{
	register struct 	i258dev	*dd;
	register struct 	i258req	*rq = (struct i258req *)0;
	char				do_it = 1;
	unsigned			x, op;
	int					rn;
	int					error_code;
	register unsigned	unit, brd;
	struct i258drtab	*dr;
	struct i258part		*p;
	struct disk_parms	i258dp;

	if (getminor(dev) > i258maxmin)
		return(ENXIO);

	if (((brd = BOARD(dev)) >= N_i258) || (!ISTAPE(dev)))
		return(ENXIO);

	unit = UNIT(dev);
	dd = &i258dev[brd];
	dr = i258drtab_struct(brd, unit);
	p = &dr->dr_part[PARTITION(dev)];
	error_code = EOK;

	DEBPR(DEB_IOCTL,(CE_CONT,"i25tioctl: brd=%d, unit=%d, cmd=0x%x\n",
			brd,unit,cmd));
	x = SPL();
	/*
	 * Get a request slot for doing the operation.
	 */
	while((rn=i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}
	DEBPR(DEB_IOCTL,(CE_CONT,"i25tioctl: rn=%x, tstate=%x\n",rn,dd->d_tstate[unit]));
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	rq->r_mbp = (mps_msgbuf_t *)0;

	switch (cmd) {
	case T_ERASE:
		op = ERASE_CMD;
		break;
	case T_RWD:
	case T_RST:
	case T_LOAD:
	case T_UNLOAD:
		op = SEEK_BOT_CMD;
		break;
	case T_SFF:
		do_it = (char)cmdarg; 	/* XXX */
		op = SFFM_CMD;
		break;
	case T_RETENSION:
		op = RETEN_TAPE_CMD;
		break;
	case T_WRFILEM:
		op = WRFM_CMD;
		break;
	case V_GETPARMS:	/* Caller wants device parameters */
		i258dp.dp_type = DPT_NOTDISK;
		i258dp.dp_heads = 0;
		i258dp.dp_sectors = 0;
		i258dp.dp_secsiz = TAPE_DEV_GRAN;
		i258dp.dp_ptag = p->p_tag;
		i258dp.dp_pflag = p->p_flag;
		i258dp.dp_pstartsec = p->p_fsec;
		i258dp.dp_pnumsec = p->p_nsec;

		/* Put parameters into user segment */
		if (copyout((caddr_t)&i258dp, (caddr_t) cmdarg, 
		    (unsigned)sizeof(struct disk_parms))) {
			error_code = EFAULT;
		}
		do_it = 0;
		break;
	case I258_RESERVE:
		dd->d_sflags[unit] |= SF_HOLD_RSRV;
		do_it = 0;
		break;
	case I258_RELEASE:
		dd->d_sflags[unit] &= ~SF_HOLD_RSRV;
		do_it = 0;
		break;
	case T_RDSTAT:
	case T_SBF:
	default:
		SETERR(EINVAL);			/* bad command */
		do_it = 0;				/* don't execute */
		break;
	}

	DEBPR(DEB_IOCTL,(CE_CONT,"i25tioctl: do_it=%x, op=%x, d_tstate=%x\n", 
				do_it,op,dd->d_tstate[unit]));
	if (do_it) {
		switch (dd->d_tstate[unit]) {
		case TS_NOTHING:
				/*
				 * If our file mark flag is set, we don't really
				 * need to seek, the tape is at a file mark, but
				 * the user just doesn't know it yet.
				 */
			if ((op == SFFM_CMD) && 
				(dd->d_sflags[unit] & SF_EOF_MARK)) {
				dd->d_sflags[unit] &= ~(SF_EOF_MARK|SF_NEED_EOF);
				do_it = 0;
			}
			break;

		case TS_READING:
			i25tbufcmd(dev, (struct buf *)0, TP_FLUSH);
			if(op == RESET_CMD)
				break;
			if (op == WRFM_CMD) {
				SETERR(ENODEV);
				do_it = 0;	/* don't execute */
				break;
			}
			/* First thing for any operation (that this driver currently
			 * supports) is to seek forward to the filemark. Since the
			 * read/write terminate command causes a rewind, we use the
			 * seek instead.
			 */
			if (op == SFFM_CMD)
				do_it-- ;
			do_it *= i25tperform(dev,brd,rn,SFFM_CMD);
			/* Now, in some cases we must explicitly rewind the tape. */
			if ((do_it) && ((op == ERASE_CMD) || (op == SEEK_BOT_CMD)
								|| (op == RETEN_TAPE_CMD))) {
				if (op == SEEK_BOT_CMD)
					do_it = 0;
				do_it *= i25tperform(dev,brd,rn,SEEK_BOT_CMD);
			}
			break;

		case TS_WRITING:
			i25tbufcmd(dev, (struct buf *)0, TP_FLUSH);
			if(op == RESET_CMD)
				break;
			if (op == SFFM_CMD) {
				SETERR(ENODEV);
				do_it = 0;	/* don't execute */
				break;
			}
			if (op == WRFM_CMD)
				do_it = 0;
			do_it *= i25tperform(dev,brd,rn,WRFM_CMD);
		}

		DEBPR(DEB_IOCTL,(CE_CONT,"i25tioctl: do_it=%d, op=%x, d_tstate=%x\n", 
				do_it, op, dd->d_tstate[unit]));
		while (do_it--) {
			(void)i25tperform(dev,brd,rn,op);
		}
	}

	/*
	 * If there was a file mark, we don't care now.
	 */
	dd->d_sflags[unit] &= ~(SF_EOF_MARK|SF_NEED_EOF);
	i258setup(brd,rn);
	splx(x);
	DEBPR(DEB_IOCTL,(CE_CONT,"i25tioctl: completed, error_code = %x\n", 
		error_code));
	return (error_code);
}

#define VERY_UGLY_KLUDGE

i25tbufcmd(dev, bp, opt)
dev_t			dev;		/* major, minor numbers */
register struct	buf *bp;
int				opt;		/* command option  */
{
	register struct buf *tp = NULL;
	register struct i258dev *dd = &i258dev[BOARD(dev)];
	caddr_t	 		bp_addr;
	unsigned 		s, b_amount, unit;
	unsigned 		tapepages;
	int				i, index;
#ifdef VERY_UGLY_KLUDGE
	extern ramdactive;
#endif

	unit = UNIT(dev);
	for (index = 0; index < I258_NUMTAPE; index++) {
		if (dd->tbuf[index].unit == unit) {
			tp = &(dd->d_tbp[index]);
			break;
		}
	}
	DEBPR(DEB_BUF,(CE_CONT,"bufcmd: dev=%x, opt=%x, tstate=%x, unit=%x\n",
				dev,opt,dd->d_tstate[unit], unit));

	switch(opt) {
	/*
	 * Allocate external raw tape buffers
	 *
	 * WARNING - If more than one tape buffer is allocated
	 * per controller, then B_RAMRD and B_RAMWT must be 
	 * reexamined for proper usage.
	 */
	case TP_GETBUF:
		s = splbuf();	/* splhi ????? */
		dd->tbuf[index].bufpages = 0;
	/*	tapepages = min(availrmem, i258tbuf_pages); */
		tapepages = 0;
		for (i=0; i < i25t_elements; i++) {
			if (i25tbuf[i].unit == unit)
				tapepages = i25tbuf[i].bufpages;
		}
		if(tapepages > (2*tune.t_minarmem))
			tapepages = min(tapepages,tune.t_minarmem);
		if(tapepages == 0) {
			splx(s);
			break;	/* tape buffering will not be done */
		}
		DEBPR(DEB_BUF,(CE_CONT,"i25tbufcmd: tapepages=0x%x\n",tapepages));

		while ((tp->b_un.b_addr = (caddr_t) kmem_zalloc( tapepages * ptob(1),
			KM_NOSLEEP )) == 0) {
			if (tapepages > 2)
				tapepages  = tapepages / 2;
			else {
				cmn_err(CE_NOTE,
					"i258tp: cannot get memory for tape buffering\n");
				splx(s);
				break;
			}
		}
		dd->tbuf[index].bufpages = tapepages;
		DEBPR(DEB_BUF,(CE_CONT,
			"i25tbufcmd: b_addr=%x, %d pages for tape buffers\n",
				(ulong)tp->b_un.b_addr,tapepages));
		tp->b_flags = B_BUSY|B_DONE;
		if (bp != NULL)
			tp->b_proc = bp->b_proc;
		tp->b_bcount = dd->tbuf[index].bufpages * ptob(1);
		tp->b_resid = tp->b_bcount;
		tp->b_dev = dev;
		tp->b_error = 0;			/* No errors */
		splx(s);
		break;

	/*
	 * Deallocate external raw tape buffers
	 */
	case TP_FREEBUF:
		s = splbuf();
		if(dd->tbuf[index].bufpages != 0) {
			kmem_free(dd->d_tbp[index].b_un.b_addr,
					dd->tbuf[index].bufpages * ptob(1));
			dd->tbuf[index].bufpages = 0;
			dd->d_tbp[index].b_un.b_addr = 0;
		}
		splx(s);
		break;

	/*
	 * Read buffer from tape and dole it out
	 * to the user as required.
	 */
	case TP_RDBUF:
		bp->b_resid = bp->b_bcount;
		if(dd->d_sflags[unit] & SF_NEED_EOF) {
			dd->d_sflags[unit] &= ~SF_NEED_EOF;
			biodone(bp);
			break;
		}

		bp_addr = (caddr_t)paddr(bp);
		
		while(bp->b_resid) {
			s = splbuf();
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: TP_RDBUF before biowait, tp->b_flags=%x\n",
				tp->b_flags));
			/* Make sure that tape buffer is not doing I/O */
			biowait(tp);	
			splx(s);
			if(tp->b_error) {
				bp->b_error = tp->b_error;
				bp->b_flags |= B_ERROR;
				tp->b_error = 0;			/* Error has been reported */
				tp->b_resid = tp->b_bcount;	/* Rig to do I/O next time */
				break;
			}

			/* Changed location of B_RAMRD flag */
			if(dd->d_tflags[unit] & B_RAMRD) { 
					/* Buffer just got filled */
				DEBPR(DEB_BUF,(CE_CONT,
					"i25tbufcmd: READBUF - RAMRD set,count=%x resid=%x\n",
					tp->b_bcount,tp->b_resid));
				tp->b_bcount -= tp->b_resid;
				tp->b_resid = 0;
				dd->d_tflags[unit] &= ~B_RAMRD;
			}

			b_amount = tp->b_bcount - tp->b_resid;
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: READBUF b_amount=%x\n",b_amount));

			b_amount = min(bp->b_resid,b_amount);
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: READBUF b_amount=%x\n",b_amount));

			if (b_amount != 0) {
				if(copyout((tp->b_un.b_addr + tp->b_resid), bp_addr,b_amount)) {
					/* copyout failed, if the destination was a kernel
					/* address, then use bcopy */
#ifdef VERY_UGLY_KLUDGE
					/* use (b_proc == NULL) to indicate that the caller */
					/* has specified a kernel address */
					if (!bp->b_proc) {
						/* kernel address's use bcopy to move the data */
						bcopy((tp->b_un.b_addr+tp->b_resid), bp_addr,b_amount);
					}
#else /* NOT THAT UGLY */
					/* determine if the address is writable in the */
					/* kernel */
					if (kernacc(bp_addr,b_amount,B_WRITE)) {
						/* kernel address's use bcopy to move the data */
						bcopy((tp->b_un.b_addr+tp->b_resid), bp_addr,b_amount);
					}
#endif /* VERY_UGLY */
					else {
						/* no hope for this transfer */
						bp->b_flags |= B_ERROR;
						bp->b_error = EFAULT;
						break;
					}
				}
				tp->b_resid += b_amount;
				bp_addr += b_amount;
				bp->b_resid -= b_amount;
			}
			/*
			 * If the index = count, we need to read more
			 * data. If the end of filemark is set, we stop .
			 * reading here. If the read request is entirely 
			 * filled, we need to remember to send back a 0 
			 * request on the next read request.
			 */
			if(tp->b_resid >= tp->b_bcount) {
				DEBPR(DEB_BUF,(CE_CONT,
					"i25tbufcmd: READBUF - need to read. d_tflags=%x\n",
					dd->d_tflags));
#ifdef DEBUG
				if(i25t_debug & DEB_BUFB)
					monitor();
#endif
				if(dd->d_sflags[unit] & SF_EOF_MARK) {
					dd->d_sflags[unit] |= SF_NEED_EOF;
					dd->d_sflags[unit] &= ~SF_EOF_MARK;
					break;
				} else {
					tp->b_bcount = dd->tbuf[index].bufpages * ptob(1);
					tp->b_flags |= B_READ;
					i25tsetup(tp,1);
				}
			}
		}
		biodone(bp);
		break;

	/*
	 * Accept data from the user until the buffer is full.
	 * Then write it out and accept more data.
	 */
	case TP_WRBUF:
		bp_addr = (caddr_t)paddr(bp);
		bp->b_resid = bp->b_bcount;

		while(bp->b_resid) {
			s = splbuf();
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: WRITEBUF - before biowait, tp->b_flags=%x\n",
					tp->b_flags));
			/* Make sure tape buf isn't doing I/O */
			biowait(tp);	
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: WRITEBUF - before biowait, tp->b_flags=%x\n",
					tp->b_flags));
			splx(s);
			if(tp->b_error) {
				DEBPR(DEB_BUF,(CE_CONT,
					"i25tbufcmd: WRITEBUF - tp->error set\n"));
				bp->b_error = tp->b_error;
				bp->b_flags |= B_ERROR;
				tp->b_error = 0;  /* Error has been reported */
				tp->b_resid = tp->b_bcount;	/* Rig to do I/O next time */
				break;
			}

			/* Changed location of B_RAMWT flags */
			if(!(dd->d_tflags[unit] & B_RAMWT)) { /* Setup tape buffer */
				tp->b_bcount = dd->tbuf[index].bufpages * ptob(1);
				tp->b_resid = 0;
				DEBPR(DEB_BUF,(CE_CONT,
					"i25tbufcmd: WRITEBUF - RAMWT not set, count=%x \
					resid=%x\n", tp->b_bcount,tp->b_resid));
				tp->b_flags &= ~B_READ;
				dd->d_tflags[unit] |= B_RAMWT;
			}

			b_amount = tp->b_bcount - tp->b_resid;

			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: WRITEBUF - b_amount=%x\n",b_amount));

			b_amount = min(bp->b_resid,b_amount);

			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: WRITEBUF - b_amount=%x\n",b_amount));

			if(b_amount != 0) {
				if(copyin(bp_addr,(tp->b_un.b_addr + tp->b_resid), b_amount)) {
					/* copyin failed, if the source was a kernel
					/* address, then use bcopy */
#ifdef VERY_UGLY_KLUDGE
					/* use (b_proc == NULL) to indicate that the caller */
					/* has specified a kernel address */
					if (!bp->b_proc) {
						/* kernel address's use bcopy to move the data */
						bcopy(bp_addr, (tp->b_un.b_addr+tp->b_resid) ,b_amount);
					}
#else /* NOT THAT UGLY */
					/* determine if the address is readable in the */
					/* kernel */
					if (kernacc(bp_addr,b_amount,B_READ)) {
						/* kernel address's use bcopy to move the data */
						bcopy(bp_addr, (tp->b_un.b_addr+tp->b_resid) ,b_amount);
					}
#endif /* VERY_UGLY */
					else {
						/* no hope for this transfer */
						bp->b_flags |= B_ERROR;
						bp->b_error = EFAULT;
						break;
					}
				}
				tp->b_resid += b_amount;
				bp_addr += b_amount;
				bp->b_resid -= b_amount;
			}
			/*
			 * If the index = count, we need to write the data.
			 */
			if(tp->b_resid >= tp->b_bcount) {
				DEBPR(DEB_BUF,(CE_CONT,
					"i25tbufcmd: WRITEBUF - need to write. d_tflags=%x\n",
						dd->d_tflags));
				i25tsetup(tp,1);
			}
		}
		biodone(bp);
		break;

		/*
		 * Flush the buffers in tape queue.
		 * Start I/O on partial buffer
		 * at device close.
		 */
	case TP_FLUSH:
		/* Changed location of B_RAMWT flag. */
		if((dd->d_tflags[unit] & B_RAMWT) && (tp->b_resid != 0)) {
			tp->b_bcount = tp->b_resid;
			DEBPR(DEB_BUF,(CE_CONT,
				"i25tbufcmd: FLUSHBUF - RAMWT  set, count=%x resid=%x\n",
				tp->b_bcount,tp->b_resid));
			i25tsetup(tp,1);
			s = splbuf();
			/* Make sure tape buf isn't doing I/O */
			biowait(tp);	
			splx(s);
		}
		break;
	}
}


/**********************************************************************
 *
 * 		Try to start an I/O request.  Queue it if there are no
 *		available request slots.
 *
 *		Check legality, and adjust for partitions.  Reject request if
 *		unit is not-ready.
 *
 ***********************************************************************/
i25tsetup(bp,flag)
register struct buf *bp;	/* buffer header */
int					flag;
{
	register struct i258dev	*dd;
	register struct i258req	*rq;
	register struct buf		*ap;
	struct  iobuf			*bufh;	/* buffer queue header */
	int				 		brd, rn, unit;
	unsigned			 	x;

	/* initializations */
	brd = BOARD(bp->b_dev);
	dd = &i258dev[brd];
	unit = UNIT(bp->b_dev);

	DEBPR(DEB_SET,(CE_CONT,"i25tsetup: \n"));
	x = SPL();

	bp->b_resid = 0;
	bp->b_flags &= ~B_DONE;

	/*
	 * Get a request slot for doing the operation.
	 */
	while((rn=i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}

	/*
	 * Put the buffer on the HEAD of the queue so that
	 * i258setup will be sure to get it in our request slot.
	 * Note that this whole mess must be (and is) protected
	 * by an spl.
	 */
	bufh = dd->d_bufh;
	bp->av_forw = (struct buf *)(0);
	if (bufh->b_actf == (struct buf *)(0)) {
		bufh->b_actf = bp;
		bufh->b_actl = bp;
	} else {
		ap = bufh->b_actf;
		bufh->b_actf = bp;
		ap->av_back = bp;
		bp->av_forw = ap;
	}
	if(bp->b_flags & B_READ)
		dd->d_tstate[unit] = TS_READING;
	else
		dd->d_tstate[unit] = TS_WRITING;
	dd->d_sflags[unit] &= ~(SF_EOF_MARK|SF_NEED_EOF);
	DEBPR(DEB_SET,(CE_CONT,"i25tsetup: buffer queued on brd=%d\n",brd));
	DEBPR(DEB_SET,(CE_CONT,"i25tsetup: got request 0x%x\n",rn));
	rq = &dd->reqinfo[rn];
	if(flag)
		rq->r_flags |= i258RQ_BUF_TAPE;
	rq->r_mbp = (mps_msgbuf_t *)0;
	i258setup(brd,rn);
	splx(x);
}

/***********************************************************************
 *
 * 		Perform an I/O request for i25tioctl.
 *		Wait for completion and check results.
 *
 ************************************************************************/
int
i25tperform(dev,brd, rn, cmd)
dev_t		dev;		/* major, minor numbers */
int			brd;
int			rn;
unsigned	cmd;
{
	register struct i258dev	*dd = &i258dev[brd];
	register struct i258req	*rq = &dd->reqinfo[rn];


	DEBPR(DEB_PERF,(CE_CONT,"i25tperform: brd=%d, rn=%x, cmd=%x\n",brd,rn,cmd));
	rq->r_flags |= i258RQ_IM_WAITING;
	if (rq->r_mbp == (mps_msgbuf_t *)NULL)
		if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == (mps_msgbuf_t *)NULL)
			cmn_err (CE_PANIC, "i25tperform: Cannot get message buffers");
	i258io(brd, rn, cmd);
	while(rq->r_flags & i258RQ_IM_WAITING)
		sleep((caddr_t)rq,PRIBIO+1);
	dd->d_tstate[UNIT(dev)] = TS_NOTHING;
#ifdef DEBUG
	if(i25t_debug & DEB_MPERF)
		mps_msg_showmsg(rq->r_mbp);
	if(i25t_debug & DEB_BPERF)
		monitor();
#endif
	if(mps_msg_iserror(rq->r_mbp)  || 
	   mps_msg_iscancel(rq->r_mbp) ||
	   mps_msg_isreq(rq->r_mbp)    ||	
	   i258checkerr(brd,mps_msg_getudp(rq->r_mbp))) {
		return(0);
	}
	return(1);
}

/*
 * Size function required as part of DDI/DKI for V4.0
 */
i25tsize(dev)
dev_t dev;
{
	struct 		i258cfg *cf = &i258cfg[BOARD(dev)];
	unsigned	unit = UNIT(dev);
	struct		i258cdrt  *cdr = &cf->c_drtab[unit&TID_MASK][DRTAB(dev)];

	return(cdr->cdr_size); 
}

