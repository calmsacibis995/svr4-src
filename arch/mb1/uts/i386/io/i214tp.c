/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1985  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ifndef lint
static char itp_copyright[] = "Copyright 1985, 1989 Intel Corp. 462846";
#endif  /* lint */

#ident	"@(#)mb1:uts/i386/io/i214tp.c	1.3"

/*****************************************************************************
 *
 * TITLE:	i214 Device driver Tape Support
 *
 * DATE(S):		April 9, 1985
 *
 *	Streamer Tape Device Driver for UNIX 386.  Supports 214 (and 215G).
 *
 * This driver:
 *      Returns EIO for I/O errors.
 *	Doesn't manage concurrent-seeks on multiple tape drives
 *		(can't on multiple QIC-02 tape drives).
 *      Doesn't compile statistics to take into account the
 *		time involved to load tape. Only read/write statistics are
 *		available.
 *	Doesn't handle multiple start/stop tape drives at all and
 *		will not allow multiple QIC-02 drives to be accessed
 *		unless all drives except the current one have been rewound
 *		(i.e. no tape to tape copies allowed.)
 *
 * Compiliation options:
 *
 *	-DDEBUG		Include debugging support.
 *
 ****************************************************************************/
#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/elog.h"
#include "sys/kmem.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vtoc.h"
#include "sys/alttbl.h"
#include "sys/bbh.h"
#include "sys/tape.h"
#include "sys/i214.h"
#include "sys/ddi.h"
/* broken DDI workaround */
#ifdef wakeup
#undef wakeup
#endif
#include "sys/immu.h" /* phystokv() */
extern caddr_t getcpages();
extern paddr_t vtop(caddr_t, struct proc *);
/* End broken DDI workaround */

int itpdevflag = 0;
#ifdef __STDC__
int itpbufcmd(dev_t, struct buf *, int);
int itpioctl(dev_t, int, ulong, int, struct cred *, int *);
#endif

extern void i214io();
extern void i214start();
extern void i214sweep();
extern char i214checkerr();

/* Values configured in space.c */
extern	int	i214_cnt;			/* Number of boards */
extern	int	i214retry;		/* Number of retries on soft error */
extern 	short	i214maxmin;		/* maximum minor device number */
extern	int	i214tbuf_max;		/* the maximum number of buffers */
					/* that we can allocate.  */

extern	struct	i214cfg	i214cfg[];	/* 214 "configuraton" */
extern	struct	i214dev	i214dev[];	/* per-board device data */
extern	struct	iobuf	i214tab[];	/* buffer headers per board */
extern	struct	iobuf	i214tbuf[];	/* tape buffer headers per board */
extern	struct	i214dev	*i214bdd[];	/* board-index -> "dev" map */
extern	struct	i214minor i214minor[];	/* minor number bit map */
extern	struct	buf i214tmem[];		/* external raw buffers */


struct iobuf itptab[1];			/* device tab for tape */
int	itptape_open = 0;		/* number of open tape devices */
dev_t	itpopen_dev = 0;		/* device of open tape */
int	itp_num_buf = 0;		/* Actual number of buffers	*/
					/* allocated.			*/

#define FALL_BACK_SIZE	0x1000		/* One page, we can always get that */

uint	itpbufsz = i214XBSIZ;		/* Our buffer size.		*/
					/* This is a uint to match the	*/
					/* size field in a buffer.	*/

#define	RTFM_SIZE	0x4000		/* The number of bytes in the	*/
uint	itprtfmsz = RTFM_SIZE;		/* buffer for reading to file	*/
					/* mark.			*/
					/* Note: the number of bytes	*/
					/* specified will be rounded up	*/
					/* to the nearest page in	*/
					/* actual usage.		*/
					/* This is a uint to match the	*/
					/* size field in a buffer.	*/

struct	buf	itprtfm_buf[4];		/* We want to set up our own	*/
					/* external buffer.		*/
					/* Assume MAX of 4 controllers	*/

static int itpbuf_num = 0;		/* number of buffer in current use */

#ifdef DEBUG
#define MTPOPEN		0x0001	/* Bit 0 */
#define MTPCLOSE	0x0002	/* Bit 1 */
#define MTPSTRAT	0x0004	/* Bit 2 */
#define MTPREAD		0x0008	/* Bit 3 */
#define MTPWRITE	0x0010	/* Bit 4 */
#define MTPIOCTL	0x0020	/* Bit 5 */
#define MTPBUF		0x0040	/* Bit 6 */
#define MTPSPEC		0x0080	/* Bit 7 */
ushort	itp_messages = MTPSPEC;

#define BTPOPEN		0x0001	/* Bit 0 */
#define BTPCLOSE	0x0002	/* Bit 1 */
#define BTPBUF1		0x0004	/* Bit 2 */
#define BTPBUF2		0x0008	/* Bit 3 */
#define BTPBUF3		0x0010	/* Bit 4 */
#define BTPBUF4		0x0020	/* Bit 5 */
#define BTPBUF5		0x0040	/* Bit 6 */
#define BTPBUF6		0x0080	/* Bit 7 */
ushort	itp_break = 0x00;
#endif /*DEBUG*/

extern ulong i214dma_limit;		/* Never use direct DMA */
int itp_init = 0;			/* Static buffer init done */

/*****************************************************************************
 *
 * 	Init the buffers, if sufficient RAM is available.
 *
 *	If sufficient RAM is available that we cannot DMA it all,
 *	we allocate the buffers here.  This is because at init time
 *	we are guaranteed that our buffers will get allocated in the
 *	area of RAM to which we CAN use DMA.
 *
 ****************************************************************************/
void
itpinit()
{
	register unsigned board;

	if (itp_init)
		return;
	for (board = 0; board < i214_cnt; board++) {
		itprtfm_buf[board].b_un.b_addr = getcpages(btop(itprtfmsz), KM_NOSLEEP);
		if (itprtfm_buf[board].b_un.b_addr == NULL)
			cmn_err(CE_PANIC, "i214tp: No read-to-file-mark buffer");
		itprtfm_buf[board].b_bcount = itprtfmsz;
	}
	/*
	 * Allocate external buffers
	 * and init the device.
	 */
	if (i214tbuf_max > 0)
		(void) itpbufcmd(0, (struct buf *)0, TP_GETBUF);
	if (itp_num_buf != i214tbuf_max)
		cmn_err(CE_PANIC, "i214tp: Can't get external tape buffers\n");
	itp_init = 1;
}


/*
 * 	Open a unit. Sets a given partition (i.e., special file) open.
 *
 *	If this function is opening the first partition on a physical
 *	device, it calls i214sweep to configure the device.  Ideally,
 *	i214binit would configure all devices once and for all, but
 *	this is impossible because users can insert different density
 *	floppy disks into the same drive.  Therefore, configuration
 *	is done here.  To further complicate things, the 214 manual
 *	says that when you configure a device you should configure
 *	them all, starting with the winchesters.+  So, i214sweep causes
 *	all devices to be configured.++  It does this by calling for
 *	device 0 to be configured, and i214intr contains a loop that
 *	configures all devices in turn whenever it senses that device 0
 *	is being configured.
 *
 *	Open, close, ioctl and strategy must not be concurrent.
 *	bufh->b_active and a sleep loop accomplish this.  Open must
 *	lock each other out in case the second open gets through
 *	and tries to do I/O before sweep finishes.
 *
 * 	For opening a tape drive, we need to allocate a buffer in case
 *	we need one for executing the T_READING_TO_FM state.  This is
 *	truly a botch, as the tape drive itself IS capable of seeking
 *	to the next file mark when reading; but the iSBX 217 didn't see
 *	fit to implement it properly (it will rewind on you if you try
 *	it as is).  After calling getablk(), itprtfm_buf->b_paddr will point
 *	to a 1K (BSIZE) buffer that we'll read into for throwing away data.
 *	That buffer has to be allocated here in open, as we can't take the
 *	chance that start will try to allocate one during interrupt time.
*/
/*ARGSUSED*/
itpopen(devp, flag, otyp, cred_p)
dev_t	*devp;
int	flag;	/* not used */
int otyp;	/* not used */
struct cred	*cred_p;
{
	register struct i214dev *dd;
	unsigned board, unit;
	int i;
	int errcod = 0;

#ifdef DEBUG
	if (itp_messages&MTPOPEN)
		printf("itpopen: dev(%x)\n", *devp);
#endif
	/*
	 * Check for errors in 'dev' parameters:
	 * If it's addressing a nonexistent board, addressing floppy
	 * or tape on a 220, or a tape unit on a 215A/B, or a tape unit
	 * on a 215G whose firmware doesn't support it, return ENXIO.
	 */
	if (getminor(*devp) > i214maxmin)
		return(ENXIO);
	board = BOARD(*devp);
	if (board >= i214_cnt)
		return(ENXIO);
	dd = i214bdd[board];
	if (!dd->d_state.s_exists)
		return(ENXIO);
	unit = UNIT(*devp);
	if (!ISTAPE(dd, unit))
		return(ENXIO);
	if (!IS214(dd))
		return(ENXIO);	/* No tape support prior to 214 */
	if (!dd->d_state.s_1st_init && !TAPE_SPT(dd)) {
		return(ENXIO);	/* 214 but no tape support */
	}

	/*
	 * Only one tape on a controller can
	 * be open at a time.
	 */
	for (i = FIRSTTAPE; i < NUMSPINDLE; i++) {
		if (dd->d_state.s_flags[i] & SF_OPEN) {
			return(EBUSY);
		}
	}
	/*
	 * Allocate the buffer for T_READING_TO_FM
	 * and initialize tape drive.
	 */
	itprtfm_buf[board].b_flags = (B_BUSY|B_DONE|B_READ);
	itprtfm_buf[board].b_edev = *devp;
	itprtfm_buf[board].b_resid = 0;
	itprtfm_buf[board].b_proc = 0;
	dd->d_state.rtfm_buf = &itprtfm_buf[board];

	/* Initalize the unit, i214sweep knows whether or */
	/* not to do the physical initialization. */
	errcod = itpioctl(*devp, T_TINIT, 0, 0, (struct cred *)0, (int *)0);
	if (errcod == 0) {
		/*
		 * Init external buffers and open the device.
		 */
		if (itpopen_dev == 0) {
			if (i214tbuf_max > 0) {
				errcod = itpbufcmd(*devp, (struct buf *)0, TP_INITBUF);
			}
		}
	}

	if (errcod == 0) {
		itpopen_dev = getminor(*devp);
		if (itp_num_buf != 0)
			dd->d_state.t_flags |= TF_FM_ALWAYS;
		else
			dd->d_state.t_flags &= ~TF_FM_ALWAYS;
		dd->d_state.s_flags[unit] |= (SF_OPEN|SF_READY);
		itptab[0].b_flags = B_TAPE;
		itptape_open++;
	} else {
		dd->d_state.s_flags[unit] &= ~SF_READY;
	}

	/*  Start up any requests on the queue */
	if (dd->d_state.t_flags & TF_WANTED) {
		dd->d_state.t_flags &= ~TF_WANTED;
		wakeup((caddr_t)&dd->d_state.s_opunit);
	}
	return(errcod);
}




/*
 * 	Close a tape unit.
*/
/*ARGSUSED*/
itpclose(dev, flag, otyp, cred_p)
register dev_t	dev;	/* major, minor numbers */
int	flag;	/* not used */
int otyp;	/* not used */
struct cred	*cred_p;
{
	register struct i214dev	*dd = i214bdd[BOARD(dev)];
	unsigned	unit = UNIT(dev);
	struct		i214drtab *dr = &dd->d_drtab[unit];
	int errcod = 0;

#ifdef DEBUG
	if (itp_messages&MTPCLOSE)
		printf("itpclose: dev(%x)  itpopen_dev(%x)\n", dev, itpopen_dev);
#endif
	/*
	 * First flush out the buffers, then deallocate them.
	 * Rewind tape on close.
	 */
	if (itpopen_dev == getminor(dev)) {
#ifdef DEBUG
		if (itp_break & BTPCLOSE)
			monitor();
#endif
		if (itp_num_buf != 0) {
			errcod = itpbufcmd(dev, (struct buf *)0, TP_FLUSH);
			itpopen_dev = 0;
		}
	}
	itptape_open--;

	if (dr->dr_flags & DR_NO_REWIND) {
		if (dd->d_state.t_state == TS_WRITING)
			errcod = itpioctl(dev, T_WRFILEM, 0, 0, (struct cred *)0, (int *)0);
		else if (dd->d_state.t_state == TS_READING)
			errcod = itpioctl(dev, T_SFF, 1, 0, (struct cred *)0, (int *)0);
		else	/* t_state == TS_NOTHING */
			dd->d_state.t_flags &= ~TF_FOUND_FM;
	} else
		errcod = itpioctl(dev, T_RWD, 0, 0, (struct cred *)0, (int *)0);

	/* If we were at the logical end of tape, clear the flag */
	dd->d_state.t_flags &= ~TF_AT_LEOT;

	/* All partitions are closed, mark entire unit closed */
	dd->d_state.s_flags[unit] &= ~SF_OPEN;
	return(errcod);
}


/*****************************************************************************
 *
 * 	Queue an I/O Request, and start it if not busy already.
 *
 *	Check legality, and adjust for partitions.  Reject request if
 *	unit is not-ready.
 *
 *	Note:	check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.  214 gives
 *		not-ready status to those requests that are attempted
 *		before a new volume is inserted.
 *
 ****************************************************************************/
void
itpstrategy(bp)
register struct buf *bp;	/* buffer header */
{
	register struct i214dev	 *dd = i214bdd[BOARD(bp->b_edev)];
	struct i214state *sp;
	int	x;
	unsigned	unit;
	int errcod = 0;


	/* initializations */
	unit = UNIT(bp->b_edev);
	sp = &dd->d_state;

	if ((sp->s_flags[unit] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;		/* not ready */
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	/* Check that the request is a 512 byte request.	*/
	/* Courtesy of the tape drive characteristics.		*/
	if (bp->b_bcount & 0x1FF) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		biodone(bp);
		return;
	}

	bp->b_resid = bp->b_bcount;

	/* Don't allow writes beyond the logical end of tape */
	if((sp->t_flags & TF_AT_LEOT) && !(bp->b_flags&B_READ)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENOSPC;
		iodone(bp);
		return;
	}
	/*
	 * Use read ahead/write behind buffers if they were
	 * allocated. Only one tape device can have access to
	 * buffers. If tape state changes from reading to writing
	 * or vice versa then buffers must first be flushed before
	 * tape state changes.
	 */
#ifdef DEBUG
	if (itp_messages&MTPSTRAT)
		printf("itpstrategy: b_edev(%x)\n", bp->b_edev);
#endif
	if ((itp_num_buf != 0) && (itpopen_dev == getminor(bp->b_edev))) {
		if (bp->b_flags&B_READ) {
			if (sp->t_state == TS_WRITING) {
				(void) itpbufcmd(bp->b_edev, bp, TP_FLUSH);
				(void) itpioctl(bp->b_edev, T_WRFILEM, 0, 0, (struct cred *)0, (int *)0);
			}
			errcod = itpbufcmd(bp->b_edev, bp, TP_RDBUF);
		} else {
			if (sp->t_state == TS_READING) {
				(void) itpbufcmd(bp->b_edev, bp, TP_FLUSH);
				(void) itpioctl(bp->b_edev, T_SFF, 1, 0, (struct cred *)0, (int *)0);
			}
			errcod = itpbufcmd(bp->b_edev, bp, TP_WRBUF);
		}
	} else {
		/*
		 * If we are going to switch directions, make sure that
		 * it is handled here.
		 */
		if (bp->b_flags&B_READ) {
			if (sp->t_state == TS_WRITING)
				errcod = itpioctl(bp->b_edev, T_WRFILEM, 0, 0, (struct cred *)0, (int *)0);
			sp->t_state = TS_READING;
		} else {
			if (sp->t_state == TS_READING)
				errcod = itpioctl(bp->b_edev, T_SFF, 1, 0, (struct cred *)0, (int *)0);
			sp->t_state = TS_WRITING;
		}
		if (errcod) {
			bp->b_flags |= B_ERROR;
			bp->b_error = errcod;
			biodone(bp);
			return;
		}
		/*
		 * Disksort does not guarantee the order it puts
		 * the requests on the queue so tape requests are
		 * put in FIFO order.
		 */
		if ((sp->t_state == TS_READING) && (sp->t_flags & TF_FOUND_FM)) {
			sp->t_flags &= ~TF_FOUND_FM;
			bp->b_flags &= ~B_ERROR;
			biodone(bp);	/* Return EOF now */
			return;
		} else {
			BP_ENQUE(sp->s_bufh, bp);
			/*
			 * If no requests are in progress, start this one up.  Else
			 * leave it on the queue, and i214intr will call i214start later.
			 */
			x = splbuf();
			i214start(dd);
			splx(x);
		}
	}
}

/*
 *	itpsize - return the size of the device
*/
/*ARGSUSED*/
itpsize(dev)
dev_t dev;
{
	return(409600);
}

/*
 *	itpbreakup - break buffers into contiguous chunks.
*/
void
itpbreakup(bp)
struct buf *bp;
{
	dma_pageio(itpstrategy, bp);
}

/*****************************************************************************
 *
 * 	"Raw" read.  Use physio().
 *
 ****************************************************************************/
/*ARGSUSED*/
itpread(dev, uio_p, cred_p)
register	dev_t    dev;
struct		uio		*uio_p;
struct		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = itpsize(dev);
	error_code = physiock(itpbreakup, NULL, dev, B_READ,num_blks,uio_p);
	return(error_code);
}

/*****************************************************************************
 *
 * 	"Raw" write.  Use physio().
 *
 ****************************************************************************/
/*ARGSUSED*/
itpwrite(dev, uio_p, cred_p)
register	dev_t	dev;
struct 		uio 	*uio_p;
struct 		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = itpsize(dev);
	error_code = physiock(itpbreakup, NULL, dev, B_WRITE,num_blks,uio_p);
	return(error_code);
}



/*
 * 	Tape driver special functions.
 *
 *	I214_IOC_FMT: Will execute exactly the the same as if
 *		the T_ERASE ioctl had been executed.
 *
 *	T_ERASE:  Erases a tape; this also retensions the tape.  If
 *		given while t_state == TS_READING, the driver will dump
 *		data until a file mark is found.  If TS_WRITING, a file
 *		mark will be written to get us out of TS_WRITING.
 *
 *	T_RWD:  Rewinds the tape to the logical load point.  See
 *		T_ERASE regarding t_state.
 *
 *	T_SFF:  Seek forward a file mark.  If t_state is TS_READING,
 *		will be translated to just dump data until a file mark
 *		is found, and the TS_READING state will be terminated.
 *		If TS_WRITING, command will simply be rejected.
 *
 *	T_RETENSION:  Retensions the tape.  Runs the tape to the end of
 *		the first track, then back to the LLP to ensure that the
 *		tape is stacked evenly on the cartridge.  See T_ERASE
 *		regarding t_state.
 *
 *	T_WRFILEM:  Writes a file mark at the current position on the tape.
 *		This will get you out of TS_WRITING by being translated to
 *		a R_W_TERMINATE command.  Otherwise, it doesn't seem to have
 *		much use.  Will be rejected if TS_READING.
 *
 *	T_SFREC:  Seek forward a record (Kennedy drives only).
 *		Rejected, as driver doesn't support Kennedy tape drives.
 *
 *	T_SBREC:  Seek backward a record (Kennedy drives only).
 *		Rejected as above.
 *
 *	T_SBF:  Seek backward a file mark (Kennedy drives only).
 *		Rejected as above.
*/
/*ARGSUSED*/
itpioctl(dev, cmd, cmdarg, flag, cred_p, rval_p)
dev_t	dev;		/* major, minor numbers */
int	cmd;		/* command code */
ulong	cmdarg;		/* user structure with parameters */
int	flag;
struct cred	*cred_p;
int		*rval_p;
{
	register struct i214dev *dd;
	struct iobuf *bufh;
	int do_it = 1;
	unsigned op;
	int x;
	register unsigned unit;
	struct i214drtab *dr;
	struct i214part *p;
	struct i214cfg *cf;
	struct disk_parms i214dp;
	struct	buf	*bp,*t_bp;
	int errcod = 0;

	dd = i214bdd[BOARD(dev)];
	bufh = dd->d_state.s_bufh;
	unit = UNIT(dev);
	dr = &dd->d_drtab[unit];
	p = &dr->dr_part[PARTITION(dev)];
	cf = &i214cfg[BOARD(dev)];

#ifdef DEBUG
	if (itp_messages&MTPIOCTL)
		printf("itpioctl: unit(%d) cmd(%x) flags(%x)\n", unit, (unsigned)(cmd&0xffff),dd->d_state.t_flags);
#endif

/* we need to flush all outstanding write buffers first */

	if (itp_num_buf != 0) {
		errcod = itpbufcmd(dev, (struct buf *)0, TP_FLUSH);
	}


	/*
	 * Wait for a chance to get at the device and then go for it.
	 */
	x = SPL();
	/* wait till controller not busy */
	while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
#ifdef DEBUG
		if (itp_messages&MTPIOCTL)
			printf("itpioctl: IO_WAIT\n");
#endif
		bufh->b_active |= IO_WAIT;
		sleep((caddr_t)&dd->d_state.s_state, PRIBIO +1);
	}
	/* we own the controller now; see if tape finished */
	if (dd->d_state.t_flags & TF_WAIT_SECOND) {
#ifdef DEBUG
		if (itp_messages&MTPIOCTL)
			printf("itpioctl: TF_WANTED\n");
#endif
		dd->d_state.t_flags |= TF_WANTED;
		i214start(dd);
		sleep((caddr_t)&dd->d_state.s_opunit, PRIBIO + 1);
	}

	/* Get parameters from user segment */
	if ((cmd != T_SFF) && (cmd != T_SBF) && (cmdarg != NULL)) {
		copyin((caddr_t)cmdarg, (caddr_t)&dd->d_ftk,
					(unsigned)sizeof(struct i214ftk));
		if (dd->d_ftk.f_track != 0) {
			splx(x);
			return(ENOSPC);
		}
	}

	switch (cmd) {
	case I214_IOC_FMT:
	case T_ERASE:
		op = ERASETAPE_OP;
		break;
	case T_RWD:
		op = REW_OP;
		break;
	case T_SFF:
		op = SFFM_OP;
		do_it = cmdarg;
		break;
	case T_RETENSION:
		op = RETTAPE_OP;
		break;
	case T_WRFILEM:
		op = WRFM_OP;
		break;
	case T_TINIT:
		op = INIT_OP;
		break;
	case V_GETPARMS:	/* Caller wants device parameters */
		i214dp.dp_type = DPT_NOTDISK;
		i214dp.dp_heads = 0;
		i214dp.dp_cyls = dr->dr_ncyl;
		i214dp.dp_sectors = dr->dr_nsec;
		i214dp.dp_secsiz = ((ushort)dr->dr_hsecsiz << 8) |
				 (ushort)dr->dr_lsecsiz;
		i214dp.dp_ptag = p->p_tag;
		i214dp.dp_pflag = p->p_flag;
		i214dp.dp_pstartsec = p->p_fsec;
		i214dp.dp_pnumsec = p->p_nsec;

		/* Put parameters into user segment */
		copyout((caddr_t)&i214dp, (caddr_t)cmdarg, (unsigned)sizeof(struct disk_parms));
		do_it = 0;
		break;
	case T_SFREC:	/* FALL THROUGH */
	case T_SBREC:	/* FALL THROUGH */
	case T_SBF:		/* FALL THROUGH */
	default:
		errcod = EINVAL;	/* bad command */
		do_it = 0;		/* don't execute */
		break;
	}
#ifdef DEBUG
	if (itp_messages&MTPIOCTL)
		printf("itpioctl: t_state(%d)  op(%x)\n", (ushort)(dd->d_state.t_state), op);
#endif

	if (do_it) {
		switch (dd->d_state.t_state) {
		case NOTHING:
				/*
				 * If our file mark flag is set, we don't really
				 * need to seek.
				 */
			if ((op == SFFM_OP) && (dd->d_state.t_flags & TF_FOUND_FM))
				do_it--;

			dd->d_state.s_error[unit] = 0;	/* clear error status */
			break;

		case TS_READING:
			if (op == WRFM_OP) {
				errcod = ENODEV;
				do_it = 0;	/* don't execute */
				/* clear error status */
				dd->d_state.s_error[unit] = 0;
			} else {
				/* First thing for any operation (that
				 * this driver currently supports) is
				 * to seek forward to the filemark. Since
				 * the read/write terminate command causes
				 * a rewind, we must read to the filemark.
				 * Note that since we are in read state,
				 * we have not hit a file mark.
				 */
				if (op == SFFM_OP)
					do_it--;
				dd->d_state.t_flags |= TF_READING_TO_FM;
				dd->d_state.s_state = NOTHING;
				bp = dd->d_state.rtfm_buf;
				bp->av_forw = (struct buf *)(0);
				if (bufh->b_actf == (struct buf *)0) {
					bufh->b_actf = bufh->b_actl = bp;
				} else {
					t_bp = bufh->b_actl;
					if ((bp->av_forw = t_bp->av_forw) == (struct buf *)(0))
						bufh->b_actl = bp;
					t_bp->av_forw = bp;
					bp->av_back = t_bp;
				}
				i214io(dd, bp, READ_OP, (int)unit);

				/* wait for this to finish */
				while (dd->d_state.t_flags & TF_READING_TO_FM)
					sleep((caddr_t)bp, PRIBIO + 1);

				/* Note that i214intr will have seen	*/
				/* any hard errors.			*/

				/* Now, in some cases we must explicitly
				 * rewind the tape.
				 */
				if ((do_it) && ((op == ERASETAPE_OP)
					|| (op == REW_OP)
					||(op == RETTAPE_OP))) {
					if (op == REW_OP)
						do_it = 0;

					/* wait till controller not busy */
					while ((bufh->b_actf != NULL)
							|| (bufh->b_active & IO_BUSY)) {
						bufh->b_active |= IO_WAIT;
						sleep((caddr_t)&dd->d_state.s_state, PRIBIO +1);
					}
					i214io(dd, (struct buf *)NULL, REW_OP, (int)unit);

					/* wait for the interrupt */
					while (dd->d_state.t_flags & TF_NO_BUFFER)
						sleep((caddr_t)dd, PRIBIO + 1);

					/* t_flags will indicate if we need to wait for a second interrupt */
					if (dd->d_state.t_flags & (TF_WAIT_SECOND | TF_LT_DONE)) {
						if (dd->d_state.t_flags & TF_WAIT_SECOND) {
							dd->d_state.t_flags |= TF_IM_WAITING;
							sleep((caddr_t)&dd->d_state.t_flags, PRIBIO + 1);
						} else
							dd->d_state.t_flags &= ~TF_LT_DONE;
					}

					/* check for errors; lt doesn't xfer status */
					if (dd->d_state.s_error[unit] & ST_HARD_ERR) {
						do_it = 0;
						errcod = EIO;
					}
				}
			}
			break;

		case TS_WRITING:
			if (op == SFFM_OP) {
				errcod = ENODEV;
				do_it = 0;	/* don't execute */
				/* clear error status */
				dd->d_state.s_error[unit] = 0;
			} else {
				if (op == WRFM_OP)
					do_it--;	/* convert first one to TERM */
				dd->d_state.t_state = NOTHING;
				i214io(dd, (struct buf *)NULL, R_W_TERMINATE, (int)unit);

				/* wait for the interrupt */
				while (dd->d_state.t_flags & TF_NO_BUFFER)
					sleep((caddr_t)dd, PRIBIO + 1);
				/* Note that i214intr will have seen	*/
				/* any hard errors.			*/
			}
			break;
		}

		while (!errcod && do_it--) {
			/* fire up the controller */
			/* wait till controller not busy */
			while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
				bufh->b_active |= IO_WAIT;
				sleep((caddr_t)&dd->d_state.s_state, PRIBIO +1);
			}

			if (op == INIT_OP)
				i214sweep(dd, dev, &cf->c_drtab[UNIT(dev)][DRTAB(dev)]);
			else
				i214io(dd, (struct buf *)NULL, (int)op, (int)unit);

			/* wait for the interrupt */
			while (dd->d_state.t_flags & TF_NO_BUFFER)
				sleep((caddr_t)dd, PRIBIO + 1);

			/* t_flags will indicate if we need to wait for a second interrupt */
			if (dd->d_state.t_flags & (TF_WAIT_SECOND | TF_LT_DONE)) {
				/* It really is a longterm op,
				 */
				if (dd->d_state.t_flags & TF_WAIT_SECOND) {
					dd->d_state.t_flags |= TF_IM_WAITING;
					sleep((caddr_t)&dd->d_state.t_flags, PRIBIO + 1);
				} else
					dd->d_state.t_flags &= ~TF_LT_DONE;

				/* check for errors; lt doesn't xfer status */
				if (dd->d_state.s_error[unit] & ST_HARD_ERR)
					errcod = EIO;

				/*
				 * Wake anybody waiting for us -
				 * intr has already called start.
				 */
				if (dd->d_state.t_flags & TF_WANTED) {
					dd->d_state.t_flags &= ~TF_WANTED;
					wakeup((caddr_t)&dd->d_state.s_opunit);
				}
			} else {	/* any errors? */
				if (dd->d_state.s_error[unit] & ST_HARD_ERR) {
					errcod = i214checkerr(dd,unit);
					if ((errcod == EBBHARD) || (errcod == EBBSOFT))
						errcod = EIO;
				}
			}
		}
	}

	/*
	 * If there was a file mark, we don't care now.
	 */
	dd->d_state.t_flags &= ~TF_FOUND_FM;

	i214start(dd);

	splx(x);
#ifdef DEBUG
	if (itp_messages&MTPIOCTL)
		printf("itpioctl: exit\n");
#endif
	return(errcod);
}


#define	B_INUSE		B_DELWRI	/* good data in buffer flag */
itpbufcmd(dev, bp, opt)
dev_t	dev;		/* major, minor numbers */
struct	buf *bp;
int	opt;		/* command option  */
{
	register struct buf *tp;
	struct i214dev *dd = i214bdd[BOARD(dev)];
	struct i214state *sp;
	caddr_t	 to_addr, from_addr;
	unsigned s, b_rem;
	int errcod = 0;

	sp = &dd->d_state;

	switch(opt) {
	/*
	 * Initialize external raw tape buffers
	 */
	case TP_INITBUF:
		s = splbuf();
		for (tp = &i214tmem[0]; tp < &i214tmem[i214tbuf_max]; tp++) {
			if (tp->b_un.b_addr != NULL) {
				tp->b_flags = (B_BUSY|B_DONE);
				tp->b_bcount = 0;
				tp->b_resid = 0;
			} else
				break;
		}
		splx(s);
		break;
	/*
	 * Allocate external raw tape buffers
	 */
	case TP_GETBUF:
		if (itp_num_buf != 0)
			break;
		itpbuf_num = 0;
		itpbufsz = i214XBSIZ;
		s = splbuf();
		for (tp = &i214tmem[0]; tp < &i214tmem[i214tbuf_max];) {
			tp->b_un.b_addr = getcpages(btop(itpbufsz), KM_NOSLEEP);
			if (tp->b_un.b_addr == NULL) {
				if (itp_num_buf == 0) {
					itpbufsz = ptob(btop(itpbufsz/2));
					if (itpbufsz < FALL_BACK_SIZE) {
						errcod = ENOMEM;
						itpbufsz = i214XBSIZ;
						break;
					}
				} else
					break;
			} else {
				tp->b_bcount = itpbufsz;
				itp_num_buf++;
				tp++;
			}
		}
		splx(s);
		break;

	/*
	 * Write entire user buffer.
	 * Transfer data to external raw
	 * buffers, writing buffers when full.
	 */
	case TP_RDBUF:
	case TP_WRBUF:
		to_addr = (caddr_t)phystokv(vtop(bp->b_un.b_addr,bp->b_proc));
		tp = &i214tmem[itpbuf_num];
		if (opt == TP_WRBUF) {
			from_addr = to_addr;
			to_addr = tp->b_un.b_addr + tp->b_resid;
		}
		else	{
			from_addr = tp->b_un.b_addr + tp->b_resid;
		}

#ifdef DEBUG
		if (itp_messages&MTPBUF)
			printf("Buf(%d) resid(%x)\n",itpbuf_num,tp->b_resid);
		if (itp_break & BTPBUF1)
			monitor();
#endif
		bp->b_resid = bp->b_bcount;
		while (bp->b_resid) {
			/*
			 * We must make sure that we are pointing to the
			 * correct buffer each time through this loop.
			 * If you decide to optimize this, make sure that
			 * you understand every place where 'tp' is changed.
			 * (meaning the BP_ENQUEs and BP_DEQUEs).
			 */
			tp = &i214tmem[itpbuf_num];
			/*
			 * B_DONE flag indicates buffer is not on tape queue,
			 * B_INUSE flag indicates if buffer has good data
			 * in it, or is empty. Since buffers are used in a
			 * round robin, first ~B_DONE is found we assume all
			 * buffers are full/empty, so start I/O.
			 */
			if (tp->b_flags&B_DONE) {
				if (tp->b_flags&B_ERROR) {
#ifdef DEBUG
					if (itp_messages&MTPBUF)
						printf("tbuf(%d) error(%d)\n",itpbuf_num,tp->b_error);
#endif
					bp->b_error = tp->b_error;
					bp->b_flags |= B_ERROR;
					tp->b_flags &= ~B_ERROR;
					break;
				}
				/*
				 * Initialize new buffer on first use.
				 */
				if (!(tp->b_flags&B_INUSE)) {
					if (!(bp->b_flags&B_READ)) {
						tp->b_bcount = ptob(btop(itpbufsz));
						tp->b_flags &= ~(B_READ);
						to_addr = tp->b_un.b_addr;
#ifdef DEBUG
						if (itp_messages&MTPBUF)
							printf("TP_WRBUF tbuf(%d)\n",itpbuf_num);
#endif
					} else {
						tp->b_flags |= B_READ;
						from_addr = tp->b_un.b_addr;
						/* See if the read */
						/* succeeded.  Note that we */
						/* determine if this is the */
						/* first time here by the */
						/* value of b_bcount. */
						if (tp->b_bcount != 0) {
							tp->b_bcount -= tp->b_resid;
							if (tp->b_bcount == 0)
								break;
						}
#ifdef DEBUG
						if (itp_messages&MTPBUF)
							printf("TP_RDBUF tbuf(%d)\n",itpbuf_num);
#endif
					}
					tp->b_resid = 0;
					tp->b_flags |= B_INUSE;
				}
				/*
				 * Copy data to/from buffer.
				 */
#ifdef DEBUG
				if (itp_break & BTPBUF2)
					monitor();
#endif
				if (bp->b_resid >= (b_rem = tp->b_bcount - tp->b_resid)) {
					if (b_rem != 0) {
						bcopy(from_addr,to_addr,b_rem);
						bp->b_resid -= b_rem;
						to_addr += b_rem;
						from_addr += b_rem;
						/* This statement may seem a waste of time since we
						 * are done with this buffer until it has been written
						 * or read.  Well, that is correct EXCEPT for the case
						 * where you have exactly filled (or emptied) all of
						 * the buffers.  In that case you must have b_resid
						 * set to zero so the the from_far/to_far pointers
						 * are set correctly.
						*/
						tp->b_resid = 0;
#ifdef DEBUG
						if (itp_messages&MTPBUF)
							printf("copyseg: tbuf(%d) from(%x) to(%x) count(%d)\n",
								itpbuf_num,from_addr,to_addr, b_rem);
#endif	/* DEBUG */
					}
					tp->b_flags &= ~(B_DONE|B_ERROR|B_INUSE);
					tp->b_blkno = itpbuf_num;
					tp->b_bcount = ptob(btop(itpbufsz));
					tp->b_edev = dev;
					tp->b_error = 0;
					BP_ENQUE(sp->t_bufh, tp);
					/* NOTE:	It looks like we are switching buffers	*/
					/*		here without switching the buffer	*/
					/*		pointers, selector, and the "to_far"	*/
					/*		and "from_far" variables.		*/
					/*	1)	The buffer pointers and selector are	*/
					/*		set at the beginning of this loop.	*/
					/*	2)	"to_far" and "from_far" are set when	*/
					/*		the buffer is initialized as a new	*/
					/*		buffer.					*/
					/* CONCLUSION:	It is hereby assumed that after a	*/
					/*		buffer has been written (or read) it	*/
					/*		is considered to be a "new" buffer.	*/
					itpbuf_num = ((++itpbuf_num) % itp_num_buf);
				} else {
					bcopy(from_addr,to_addr,bp->b_resid);
					tp->b_resid += bp->b_resid;
					from_addr += bp->b_resid;
					to_addr += bp->b_resid;
#ifdef DEBUG
					if (itp_messages&MTPBUF)
						printf("copyseg: tbuf(%d) from(%x) to(%x) count(%d)\n",
							itpbuf_num,from_addr,to_addr, bp->b_resid);
#endif
					bp->b_resid = 0;
				}
			} else {
				if ((bp->b_flags&B_READ) && (dd->d_state.t_flags & TF_FOUND_FM)) {
					dd->d_state.t_flags &= ~TF_FOUND_FM;
					bp->b_flags &= ~B_ERROR;
					break;
				}
				/*	HERE is where it looks at TF_FOUND_FM */
				/*
				 * If no free buffers found, start I/O
				 * on all buffers queued.
				 * And if data left to write wait for
				 * buffers.
				 */
#ifdef DEBUG
				if (itp_break & BTPBUF3)
					monitor();
#endif
				while (dd->d_state.t_bufh->b_actf != NULL) {
					BP_DEQUE(sp->t_bufh, tp);
#ifdef DEBUG
					if (itp_messages&MTPBUF)
						printf("posting tbuf(%d) count(%d)\n",tp->b_blkno, tp->b_bcount);
#endif
					/* We have to set b_resid */
					/* to the size of the requested */
					/* transfer so that we can tell */
					/* if the controller actually did */
					/* the transfer. */
					tp->b_resid = tp->b_bcount;
					BP_ENQUE(sp->s_bufh, tp);
				}
#ifdef DEBUG
				if (itp_break & BTPBUF4)
					monitor();
#endif
				s = splbuf();
				i214start(dd);
				/* We must set the tape state here because here is
				 * basically where we are going to cause it to change state.
				 */
				if (bp->b_flags&B_READ)
					sp->t_state = TS_READING;
				else
					sp->t_state = TS_WRITING;
				splx(s);
				if (bp->b_resid) {
					biowait(tp);
				}
			}
		}
		biodone(bp);
		break;

		/*
		 * Flush all buffers in tape queue.
		 * Start I/O on partial buffer
		 * at device close.
		 */
		case TP_FLUSH:
#ifdef DEBUG
			if (itp_messages&MTPBUF)
				printf("TP_FLUSH\n");
#endif
			tp = &i214tmem[itpbuf_num];
			/*
			 * If partial buffer with valid data
			 * and buffer is for writing then
			 * queue it to start I/O.
			 */
#ifdef DEBUG
			if (itp_break & BTPBUF5)
				monitor();
#endif
			/* clear all buffers that are not on tape i/o queue */
 			for (tp = &i214tmem[itpbuf_num]; (tp->b_flags & B_DONE) && (tp->b_flags & B_INUSE); tp = &i214tmem[itpbuf_num]) {
				if (!(tp->b_flags&B_READ)) {
					if (tp->b_resid && (tp->b_flags&B_INUSE)) {
						tp->b_flags &= ~(B_READ|B_DONE|B_ERROR|B_INUSE);
						tp->b_bcount = tp->b_resid;
						tp->b_blkno = itpbuf_num;
						tp->b_edev = dev;
						BP_ENQUE(sp->t_bufh, tp);
						itpbuf_num = ((++itpbuf_num) % itp_num_buf);
 					} else {
 						tp->b_bcount = 0;
 						tp->b_flags &= ~(B_ERROR|B_INUSE);
					}
 				} else {
 					tp->b_bcount = 0;
 					tp->b_flags &= ~(B_ERROR|B_INUSE);
 				}
			}
			/*
			 * Start I/O on all buffers on tape queue.
			 * And return after waiting for completion.
			 */
			while ((sp->t_bufh)->b_actf != NULL) {
				BP_DEQUE(sp->t_bufh, tp);
				if (!(tp->b_flags&B_READ)) {
#ifdef DEBUG
				if (itp_messages&MTPBUF)
					printf("Posting tbuf(%d) count(%d)\n",
							tp->b_blkno, tp->b_bcount);
#endif
					BP_ENQUE(sp->s_bufh, tp);
					/* Set the state to writing here.
					 * We know that we are going to write, but,
					 * since in the read case we do nothing, this
					 * is the best place to set it.
					 */
					sp->t_state = TS_WRITING;
				} else {
					tp->b_flags |= B_DONE;
					tp->b_bcount = 0;
				}
				tp->b_resid=0;
			}
#ifdef DEBUG
			if (itp_break & BTPBUF6)
				monitor();
#endif
			s = splbuf();
			i214start(dd);
			splx(s);
			if (((sp->s_bufh)->b_active&IO_BUSY) && !(tp->b_flags&B_DONE))
				biowait(tp);
		break;
	}
	return(errcod);
}


/*
 *	Error message, called from Unix 5.3.1 kernel via bdevsw
*/
void
itpprint (dev,str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE, "%s on tape unit %d, partition %d\n",
						str, UNIT(dev), PARTITION(dev));
}
