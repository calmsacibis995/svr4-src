/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1983, 1984, 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ifndef lint
static char i214_copyright[] = "Copyright ,1983, 1984, 1986, 1987, 1988 Intel Corp. 462654";
#endif  /* lint */

#ident	"@(#)mb1:uts/i386/io/i214.c	1.3"

/*****************************************************************************
 *
 * TITLE:	i214 Device driver
 *
 *	iSBC 214/215/217/218 Device Driver for UNIX System V.3.
 *	Supports iSBC 214 (also supports iSBC 215G/217/218).
 *	Most of code originated from the iRMX 86 215 driver.
 *
 * This driver:
 *
 *      Supports PDINFO/VTOC to find drive characteristics and partitions
 *	Handles multiple 214, 215/217/218 boards.
 *	Handles iSBC 214, iSBC 215G
 *	Handles 5.25" or 8" floppies on a 215/218.
 *	Handles 5.25" floppies on a 214
 *	Has configurable device-characteristics.
 *	Has configurable partition tables.
 *	Handles non-BSIZE sector sizes.
 *	Handles format of 214/215/218 (no automatic alternate-tracking).
 *	Handles media-change as follows:
 *		Unit becomes "ready" on 1st successful open.
 *		Media-change (units 4-7) and controller "not-ready" signal
 *			reset ready bit.
 *		Strategy insists on "ready" status.
 *		Close resets open & ready flags.  Thus, media change==>must
 *			close all files before can use unit again.
 *	Doesn't handle multiple start/stop tape drives at all and
 *		will not allow multiple QIC-02 drives to be accessed
 *		unless all drives except the current one have been rewound
 *		(i.e. no tape to tape copies allowed.)
 *	Handles Bad Blocks
 *
 * TODO:
 *
 *	Consider advantages to internal disk-sort: take winnie before floppy,
 *		tape.  (One queue per board mixes winnie, floppy and tape
 *		requests).
 *	Add a call that will execute the controller's transfer status function
 *		with the interrupt disabled so we can get longterm status.
 *	Add transfer status state while INITIALIZING, so we can get bad status
 *		from the specified disk unit.
 *	Fix the situation where a read or write near the end of the partition
 *		(where the count specifies more bytes than is left) returns
 *		zero bytes, rather than the amount possible.
 *
 * Compilation options:
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
#include "sys/uio.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/elog.h"
#include "sys/alttbl.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vtoc.h"
#include "sys/inline.h"
#include "sys/bbh.h"
#include "sys/cred.h"
#include "sys/kmem.h"
#include "sys/i214.h"
#include "sys/ddi.h"
/* Kludges for incomplete DDI */
#ifdef wakeup
#undef wakeup	/* B10 */
#endif
#include "sys/immu.h"
extern char *getcpages();
extern paddr_t vtop();
/* End DDI kludges */

#define i214_LOCKED	1U

#ifdef DEBUG
#define BIODONE(bp) if (bp->b_error) \
		cmn_err(CE_CONT,"%d line %d\n",bp->b_blkno,__LINE__); \
		biodone(bp)
#define M214OPEN	0x0001	/* Bit 0 */
#define M214INTR	0x0002	/* Bit 1 */
#define M214BINIT	0x0004	/* Bit 2 */
#define M214RW		0x0008	/* Bit 3 */
#define M214STRAT	0x0010	/* Bit 4 */
#define M214START	0x0020	/* Bit 5 */
#define M214IO		0x0040	/* Bit 6 */
#define M214CHECK	0x0080	/* Bit 7 */
#define M214ZTRAP	0x0100	/* Bit 8 */
#define M214IOC		0x0200	/* Bit 9 */
#define M214BBH		0x0400	/* Bit 10 */
#define M214FMT		0x0800	/* Bit 11 */

short   i214messages = 0;
#else
#define BIODONE(bp) biodone(bp)
#endif  /* DEBUG */

#ifdef DEBUGY
struct {
	ulong   iocnt;
	ulong   intcnt;
	struct i214iopb iob;
	unsigned status;
	ulong   intlast;
	} debdata = {0,0};
ulong   iocounter = 0;
#endif /* DEBUGY */

struct	i214dev	*i214dev;	/* per-board device data */

/* Values configured in space.c */
extern	int	i214_cnt;			/* Number of boards */
extern	int	i214retry;		/* Number of retries on soft error */
extern 	short	i214maxmin;		/* maximum minor device number */

extern	struct	i214cfg	i214cfg[];	/* 214 "configuraton" */
extern	struct	iobuf	i214tab[];	/* buffer headers per board */
extern	struct	iobuf	i214tbuf[];	/* tape buffer headers per board */
extern	struct	i214dev	*i214bdd[];	/* board-index -> "dev" map */
extern	struct	i214minor i214minor[];	/* minor number bit map */
extern  struct  alt_info i214_alts[]; 	/* Alternates info space  */
extern	struct	i214winidata i214winidata[][FIRSTFLOPPY]; /* Misc data per wini */
extern  int	i214bases[][FIRSTFLOPPY]; /* Minor # of unit's BASE device */
extern	ulong 	i214dma_limit;		/* use direct DMA if end < limit */

#ifdef BBFAKE
/**********************************************************************
	DEBUGGING!
*/
extern	struct	i214_bb_fake	i214fake[];    /* structure which holds the
						definitions for the blocks
						which the driver is supposed
						to fake as 'bad'.
						*/
extern 	int	i214_bb_max;		       /* number of fake bad blocks */
/*
	end of DEBUGGING
***********************************************************************/
#endif /* BBFAKE */

static struct	i214dev	*i214ldd[72];		/* int-level -> "dev" map */

#define base86(x)	((short)(x>>4))	/* 8086 segment from address */

int i214devflag = 0;

char i214checkerr();

/* Puts 'a' in higher byte and 'b' in lower byte. */
#define MERGE_NUM(a,b)	(unsigned short)((((a) << 8) & 0xff00) | ((b) & 0xff))

/*
 * Board type strings .
 * The support level resolves
 * to string.
 */
static char *i214b_type[] = {
	"215B",			/* Unsupported */
	"215B",			/* Unsupported */
	"215B/220",		/* Unsupported */
	"215g",			/* Unsupported */
	"214/215g"		/* Only 214 and 221 are supported */
};

extern  void    lmainit();
extern	caddr_t lma_alloc();	/* low memory allocator */

/*****************************************************************************
 *
 * 	Init a board.
 *
 *	Perform init sequence, copy parameters to board-local
 *	data-structure and init.  Does not configure devices;
 *	that's done by i214open.
 *
 *	Only called ONCE per board, during boot "probe".  Caller
 *	insures valid board number for configuration.
 *
 ****************************************************************************/
static void
i214binit(board)
unsigned board;		/* board no. */
{
	register struct i214cfg *cf;
	register struct i214dev *dd;
	struct i214wub	*wub;	/* Address for the wake-up-block */
	int		i;
	long		p_phys;	/* scratch variable for physical address */
	char	phys_add_mode;	/* physical address instead of seg:offset */


	cf = &i214cfg[board];	/* part of configuration */
	dd = &i214dev[board];

	/*
	 * Set up wake-up block for the board.
	 */

	wub = (struct i214wub *)phystokv(cf->c_wua);
#ifdef DEBUG
	if (i214messages & M214BINIT)
		cmn_err(CE_CONT, "i214binit: Set up WUB (at %x)\n",wub);
#endif /* DEBUG */
	wub->w_sysop = 1;
	p_phys = (long)kvtophys(&dd->d_ccb);    /* ccb physical address */
	if (p_phys > 0xfffff) {
		phys_add_mode = 1;
		wub->w_rsvd = 1;
		wub->w_ccb = (ushort)p_phys;		  /*  "low 16bits" */
		wub->w_ccb_b = (ushort)(p_phys >> 16); /* "high 16bits" */
	}
	else {
		phys_add_mode = 0;
		wub->w_rsvd = 0;
		wub->w_ccb = (ushort)p_phys;		  /*  "offset" */
		wub->w_ccb_b = (ushort)((p_phys & 0xffff0000UL) >> 4); /* "base" */
	}

#ifdef iAPX286
	/* Make a segment for the wub */
	mmudescr(I214WORK, cf->c_wua, sizeof(wub)-1, RW);

	/* Put it where the 214 can find it */
	copyseg(FADDR(KDS_SEL,&wub), FADDR(I214WORK,0), (unsigned)sizeof(wub));
#endif

	/*
	 * Reset the board.
	 * Make sure that there is time for the board to see the wakeup.
	 */
	outb(base86(cf->c_wua), WAKEUP_RESET);
	drv_usecwait(10);
	outb(base86(cf->c_wua), WAKEUP_CLEAR_INT);

#ifdef DEBUG
	if (i214messages & M214BINIT)
		cmn_err(CE_CONT, "i214binit: Board has been reset\n");
#endif /* DEBUG */

	/*
	 * Make a copy of i214cfg in the i214dev.i214state structure,
 	 * so the values will be accessible through a register pointer.
 	 * Code will be faster and smaller than having a pointer to a
	 * separate i214cfg structure.
	 */
	dd->d_state.s_wua = cf->c_wua;

	/* queue header points at array of buffers for this board */
	dd->d_state.s_bufh = &i214tab[board];

	/* Init queue header for tape request storage */
	dd->d_state.t_bufh = &i214tbuf[board];

	i214ldd[cf->c_level] = dd;		/* set intr->"dev" map */
	i214bdd[board] = dd;			/* set board->"dev" map */
	dd->d_state.s_board = board;

	/* set up channel control block */
	dd->d_ccb.c_ccw1 = 1;
	dd->d_ccb.c_busy1 = 0xFF;
	p_phys = (long)kvtophys(&dd->d_cib.c_csa[0]);
	if (phys_add_mode) {
		dd->d_ccb.c_cib = (ushort)p_phys;
		dd->d_ccb.c_cib_b = (ushort)(p_phys >> 16);
	}
	else {
		dd->d_ccb.c_cib = (ushort)p_phys;		  /*  "offset" */
		dd->d_ccb.c_cib_b = (ushort)((p_phys & 0xffff0000UL) >> 4); /* "base" */
	}
	dd->d_ccb.c_ccw2 = 1;
	p_phys = (long)kvtophys(&dd->d_ccb.c_cp);
	if (phys_add_mode) {
		dd->d_ccb.c_cpp = (ushort)p_phys;
		dd->d_ccb.c_cpp_b =  (ushort)(p_phys >> 16);
	}
	else {
		dd->d_ccb.c_cpp = (ushort)p_phys;
		dd->d_ccb.c_cpp_b =  (ushort)((p_phys & 0xffff0000UL) >> 4); /* "base" */
	}
	dd->d_ccb.c_cp = 0x4;

	/* now for the controller invocation block */
	dd->d_cib.c_statsem = 0;
	dd->d_cib.c_csa[0] = 0;
	dd->d_cib.c_csa[1] = 0;
	p_phys = (long)kvtophys(&dd->d_iopb);
	if (phys_add_mode) {
		dd->d_cib.c_iopb =  (ushort)p_phys;
		dd->d_cib.c_iopb_b =  (ushort)(p_phys >> 16);
	}
	else {
		dd->d_cib.c_iopb =  (ushort)p_phys;
		dd->d_cib.c_iopb_b =  (ushort)((p_phys & 0xffff0000UL) >> 4); /* "base" */
	}

	/* Set flag so that first open will result in an init sweep */
	dd->d_state.s_1st_init = 1;

	/* Don't know what firmware support is yet, so start at bottom */
	dd->d_state.s_support = 0;

	/* Try to init board.  If there, set "exists" in state. */
	outb(base86(dd->d_state.s_wua), WAKEUP_START);

#ifdef DEBUG
	if (i214messages & M214BINIT)
		cmn_err(CE_CONT, "i214binit: Board has been woken up\n");
#endif /* DEBUG */
	drv_usecwait(50000);			/* give it time to happen */
	if (dd->d_ccb.c_busy1 == 0)
		++dd->d_state.s_exists;	/* It exists! */

	cmn_err(CE_CONT, "iSBC %s @ WUA %x level %d %s\n",
			((cf->c_devcod[0] == DEV220) ? "220" :
			((cf->c_devcod[0] == DEVWINIG) ? "214/215G" : "215B")),
			(short)dd->d_state.s_wua >> 4, cf->c_level,
			(dd->d_state.s_exists ? "Found" : "Not Found"));

	if (!dd->d_state.s_exists)
		return;
	/*
	 * Allocate a 1 page buffer through which all disk I/O to addresses
	 * > i214dma_limit will be done.
	*/
	dd->d_state.d_buf = getcpages(1, KM_NOSLEEP);
	if (dd->d_state.d_buf == NULL)
		cmn_err(CE_PANIC, "cannot allocate 214 disk buffer");
	/*
	 * Set up "unit" and "device-code" value (per unit), for iopb
	 * programming.  Set up here to avoid calculating on EACH I/O.
	 */
	for (i = 0; i < FIRSTFLOPPY; i++) {		/* for winnie's */
		dd->d_state.s_devcod[i] = cf->c_devcod[0];
		dd->d_state.s_unit[i] = i;
	}

	for (; i < NUMSPINDLE; i++) {			/* and for the rest */
 		/* c_devcod calculation */
		dd->d_state.s_devcod[i] = cf->c_devcod[i / FIRSTFLOPPY];

		/* UNIT_REMOVABLE not needed for 214 */
		dd->d_state.s_unit[i] = (i & FIXEDMASK) |
					(IS214(dd) ? 0 : UNIT_REMOVABLE);
	}
}


/*****************************************************************************
 *
 * 	Called at boot time to "probe" the boards.
 *
 *	Sees if they exist and resets them.
 *
 ****************************************************************************/
void
i214init()
{
	register unsigned i;

	lmainit();
	i214dev = (struct i214dev *)lma_alloc(sizeof(struct i214dev) * i214_cnt);
	if (i214dev == NULL) 
		cmn_err (CE_PANIC, "i214init: Not enough low memory available\n");
	for (i = 0; i < i214_cnt; i++)
		i214binit(i);
}




/*
 *	Error message, called from Unix 5.3 kernel via bdevsw
*/
void
i214print (dev,str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE,
		"%s on disk unit %d, partition %d\n",
		str, UNIT(dev), PARTITION(dev));
}



/*******************************************************************************
 *
 * 	Mechanics of starting an I/O operation.  Parameters are
 *	already stored in "dd".
 *
 *	Starts the 214 by setting up the iopb and sending a wakeup signal
 *	to the controller.
 *
 ****************************************************************************/
void
i214io(dd, bp, op, unit)
struct i214dev *dd;		/* device parameters */
struct	buf	*bp;		/* buffer header */
int	op;			/* command */
int	unit;			/* unit number */
{
	register struct i214iopb *iopb;
	register struct i214state *st;
	unsigned int	i;
	long	p_addr;
	ushort  resid;
	ulong sector, endsec;

	iopb = &dd->d_iopb;
	st = &dd->d_state;

	/* If the controller is busy, i214io should not have been called */
	if ((dd->d_state.s_bufh)->b_active & IO_BUSY)
		cmn_err(CE_PANIC, "i214io: controller still busy!\n");

	/*	TF_NO_BUFFER is set here but cleared in i214intr.  As a result,
	 *	TF_NO_BUFFER set implies a no-buffer operation has another request
	 *	for the controller and bp should still be NULL.
	*/
	if ((st->t_flags & TF_NO_BUFFER) && bp)
		cmn_err(CE_PANIC,"i214io: have bp but TF_NO_BUFFER still set!\n");
	if (!bp)
		st->t_flags |= TF_NO_BUFFER;

	/*
	 * Set up IOPB.  If we are using a 214 and have
	 * the proper support level, turn on 24-bit mode.
	 */
	iopb->i_modifier = MOD_24_BIT;

	switch(op) {
	case STATUS_OP:
		p_addr = (long)kvtophys(&dd->d_error);
		break;

	case INIT_OP:
		if (IS214(dd) && !st->s_1st_init && (unit == 0) && NOSWEEP_SPT(dd))
			iopb->i_modifier |= MOD_NO_CLEAR;

		p_addr = (long)kvtophys(&dd->d_drtab[unit]);
		break;

	case FORMAT_OP:
		/* fill in iopb */
		iopb->i_cylinder = dd->d_format.f_secno / dd->d_drtab[unit].dr_spc;
		iopb->i_head = (dd->d_format.f_secno % dd->d_drtab[unit].dr_spc) / dd->d_drtab[unit].dr_nsec;
		iopb->i_sector = 1;

		/* fill in data buffer for controller */
		dd->d_format.f_trtype = dd->d_ftk.f_type;
		for (i = 0; i < sizeof(dd->d_ftk.f_pat); i++)
			dd->d_format.f_pattern[i] = dd->d_ftk.f_pat[i];
		dd->d_format.f_interleave = dd->d_ftk.f_intl;

		p_addr = (long)kvtophys(&dd->d_format);
		break;

	case READ_VENDLIST_OP:
		p_addr = (long)kvtophys(iopb->i_addr)	;
		break	;

	case DIAGNOSTIC_OP:
		iopb->i_modifier = MOD_RECAL;
		break;
	case ERASETAPE_OP:
	case LOADTAPE_OP:
	case SFFM_OP:
	case REW_OP:
	case RETTAPE_OP:
		st->t_flags |= TF_LONG_TERM;
		break;
	case WRFM_OP:
	case R_W_TERMINATE:
	case TAPEINIT_OP:
	case TAPERESET_OP:
		break;
	default:	/* Regular reads and writes */
		if (ISTAPE(dd, unit)) {
			/* get 24-bit physical buffer address */
			p_addr = vtop(bp->b_un.b_addr, bp->b_proc);
			iopb->i_xfrcnt = bp->b_bcount;
			if (iopb->i_xfrcnt == 0)
				cmn_err(CE_PANIC,
						"i214io: zero length tape operation (0x%x)\n", op);
		} else {
			/*
			 * If the buffer is not addressible to the controller,
			 * Use the per-board buffer instead
			*/
			p_addr = dd->d_ext.xfer_addr;
			if ((p_addr+dd->d_ext.xfer_count) >= i214dma_limit) {
				/*
				 * If this is a write, copy the user's data
				 * into the per-board buffer.
				*/
				if (op == WRITE_OP)
					bcopy((caddr_t)phystokv(p_addr), st->d_buf,
							dd->d_ext.xfer_count);
				p_addr = kvtophys(st->d_buf);
			}

			/* fill in IOPB */
			sector = bp->b_sector;
			iopb->i_xfrcnt = 0;
			/* do alternate sectoring only if vtoc is valid and
			   not on partition 0 */
			if ((dd->d_state.s_flags[unit] & SF_VTOC_OK) &&
			    (PARTITION(bp->b_edev) != 0))
			    {
			    struct alt_info *altptr;
			    ulong req_cnt = dd->d_ext.xfer_count;
			    ushort secsiz = dd->d_drtab[unit].dr_secsiz;
			    if (req_cnt <= secsiz) endsec = sector;
			    else endsec = sector + req_cnt / secsiz -
					  ((req_cnt % secsiz) == 0 ? 1 : 0);
			    altptr = dd->d_drtab[unit].dr_altptr;
			    /*
			     * Fix section to handle new altinfo layout
			     */
			    for (i=0; i < altptr->alt_sec.alt_used; i++) {
				if (sector == altptr->alt_sec.alt_bad[i])
				    {
				    sector = altptr->alt_sec.alt_base + i;
				    iopb->i_xfrcnt = min(req_cnt,secsiz);
				    break;
				    }
				if ((altptr->alt_sec.alt_bad[i] > sector) &&
				    (altptr->alt_sec.alt_bad[i] <= endsec))
				    {
				    endsec = altptr->alt_sec.alt_bad[i] - 1;
				    iopb->i_xfrcnt = (endsec-sector+1) * secsiz;
				    }
				}
			    }

			if (iopb->i_xfrcnt == 0) iopb->i_xfrcnt = dd->d_ext.xfer_count;
			if (iopb->i_xfrcnt % dd->d_drtab[unit].dr_secsiz)
				cmn_err(CE_PANIC, "i214io: i_xfrcnt == %d, secsiz == %d\n",
						iopb->i_xfrcnt, dd->d_drtab[unit].dr_secsiz);
			iopb->i_cylinder = sector / dd->d_drtab[unit].dr_spc;
			resid = sector % dd->d_drtab[unit].dr_spc;
			iopb->i_head = resid / dd->d_drtab[unit].dr_nsec;
			iopb->i_sector = resid % dd->d_drtab[unit].dr_nsec;

			if (st->s_devcod[unit] == DEV8FLPY ||
				st->s_devcod[unit] == DEV5FLPY)
				/* floppy starts at sector 1 */
				++iopb->i_sector;
				if (iopb->i_xfrcnt == 0)
					cmn_err(CE_PANIC,
							"i214io: zero length disk operation (0x%x)\n", op);
		}
		break;
	}

	iopb->i_funct = op;
	iopb->i_device = st->s_devcod[unit] & DEVMASK;
	iopb->i_unit = st->s_unit[unit];

	iopb->i_addr = p_addr;	/*  Set buffer address in IOPB */

	st->s_opunit = unit;
	if((op != STATUS_OP) && (op != DIAGNOSTIC_OP))
		st->s_error[unit] = 0;	/* clear error status */

	/* Fire up the controller */
	dd->d_state.s_bufh->b_active |= IO_BUSY;
	outb(base86(st->s_wua), WAKEUP_START);
}



/*****************************************************************************
 *
 * 	Start up an I/O request if there is one on the queue.
 *
 *	Called from i214strategy to start executing a request.  Called
 *	from i214intr to execute the next request on the queue, if any.
 *
 *	Called from i214[open|close|ioctl] in case a request was held up while
 *	waiting to finish one of the above, to restart the delayed request.
 *
 *	Note on sleeps:  different data areas are used for different kinds
 *		of sleeps, to avoid waking everybody up when only one has KP.
 *		Here is a list of the current wakeup variables:
 *
 *		   waiting for controller idle:  "dd->d_state.s_state"
 *		waiting for operation complete:  "dd"
 *		  waiting for second interrupt:  "dd->d_state.t_flags"
 *		   waiting till tape available:  "dd->d_state.s_opunit"
 *
 ****************************************************************************/
void
i214start(dd)
register struct i214dev *dd;
{
#ifdef DEBUG
	register char *bf,*bf2;
#endif /* DEBUG */

	register struct iobuf *bufh;
	struct buf *bp;

	bufh = dd->d_state.s_bufh;

#ifdef DEBUG
	if (i214messages & M214START)
		cmn_err(CE_CONT, "i214start: dd is: %x (bufh is: %x)\n",dd,bufh);
#endif /* DEBUG */

	/* If the controller is busy, i214intr will call start again later */
	if ((dd->d_state.s_bufh)->b_active & IO_BUSY)
		return;
	/*
	 * Grab the buffer at the head of the queue.  If the queue is
	 * empty, wake up anybody waiting for the controller.
	 * Note that this should probably be changed to give priority
	 * to anybody waiting instead of the next buffer on the queue.
	 */
	if ((bp = bufh->b_actf) == NULL) {
		bufh->b_active &= ~IO_WAIT;
		wakeup((caddr_t)&dd->d_state.s_state);
		return;
	}


	/*
	 * Set b_active flag to indicate controller is busy.  No one
	 * should make requests to the controller while this is set.
	 * It gets cleared right above here, usually when i214intr
	 * calls i214start to process next request and the queue is empty.
	 */
	bufh->b_errcnt = 0;			/* clear retry count */

	/*
	 * If this is a disk request, set the i214ext values properly.
	 * The i214ext values are used when a multiple sector I/O
	 * the request must be broken up because of alternate sectors.
	 */
	if (!ISTAPE(dd,UNIT(bp->b_edev)))
		{
		dd->d_ext.xfer_count = bp->b_bcount - bp->b_resid;
		if ((long)dd->d_ext.xfer_count < 0)
			cmn_err(CE_PANIC, "i214start: bad count\n");
		bp->b_resid = bp->b_bcount;  /* reset for return value */
		dd->d_ext.xfer_addr = vtop(bp->b_un.b_addr, bp->b_proc);
		}

	/* fire up the controller */
	i214io(dd, bp, IO_OP(bp), (int)UNIT(bp->b_edev));
}






/*******************************************************************************
 *
 * 	Perform an initialization sweep.  Called on first open
 *	of a unit, and only called from i214open.
 *
 * 	The 214 controllers run a diagnostic and clear their RAM whenever
 *	winchester unit 0 is initialized.  This can be inhibited on the
 *	214 with a modifier bit.
 *
 * 	On the 215A/B, re-initializing a floppy unit to go from double-sided
 *	to single-sided doesn't clear the controller's count of the number of
 *	heads on a drive.  The only way to do this is to run the RAM test,
 *	which clears memory to zero, corresponding to a single-sided floppy.
 *
 * 	The upshot is that on the 214, using the modifier bit, only one
 *	initialization sweep ever need be done, and then only winchester
 *	unit zero and the unit being opened need to be initialized.
 *
 *	Assumes called with interrupts disabled, for mutual exclusion on data.
 *
 *	When initializing a floppy disk on a 215A/B, calls i214io with the
 *	unit set to zero, otherwise, the current unit number is passed.
 *	i214intr loops on the unit number, so all units that need it get
 *	initialized.
 *
 ****************************************************************************/
void
i214sweep(dd, dev, cdr)
register struct i214dev *dd;	/* device parameters */
dev_t	dev;			/* major, minor numbers */
struct i214cdrt  *cdr;		/* device initialization data */
{
	register struct i214drtab *dr;
	unsigned	unit;


	/* Figure unit & drtab entry. */
	unit = UNIT(dev);
	dr = &dd->d_drtab[unit];

	/*
	 * Set up drtab entry for initialization of the device.  By
	 * building a copy of the drtab in the i214dev, we make all these
	 * values accessible thru a register pointer to the i214dev.
	 */
	dr->dr_ncyl = cdr->cdr_ncyl;

	if (ISTAPE(dd, unit)) {
		dr->dr_part = cdr->cdr_part;
		dr->dr_flags = cdr->cdr_flags;
	} else {
		dr->dr_nfhead = cdr->cdr_nfhead;
		dr->dr_nrhead = cdr->cdr_nrhead;
		dr->dr_nsec = cdr->cdr_nsec;
		dr->dr_lsecsiz = LOW(cdr->cdr_secsiz);
		dr->dr_hsecsiz = HIGH(cdr->cdr_secsiz);
		dr->dr_nalt = cdr->cdr_nalt;

		/*
		 * Compute spc (sectors/cylinder), lbps (logical blx/sec), secsiz.
		 * Also, copy partition-table pointer for i214strategy().
		 */
		dr->dr_spc = ((unit < FIRSTFLOPPY) ?
			dr->dr_nfhead : dr->dr_nrhead) * dr->dr_nsec;

		dr->dr_secsiz = cdr->cdr_secsiz;
		dr->dr_lbps = dr->dr_secsiz / NBPSCTR;
		dr->dr_part = cdr->cdr_part;
		dr->dr_pnum = cdr->cdr_pnum;
		if (unit < FIRSTFLOPPY)
			/* set up a wini to be first 2-head cylinder.
			   (open will take care of rest if VTOC ok). */
			{
			dr->dr_part->p_tag = V_BACKUP;
			dr->dr_part->p_flag = V_RONLY | V_OPEN | V_VALID;
			dr->dr_part->p_fsec = 0L;
			dr->dr_part->p_nsec = (long)dr->dr_spc * dr->dr_ncyl;
			dr->dr_altptr = &i214_alts[BOARD(dev)*4+unit];
			}
	}

	/* If we are NOT initializing the tape, we must clear the flags */
	/* that itpioctl uses to determine if it should sleep.  We do NOT */
	/* want itpioctl to sleep since it will never get woken up. */
	if (ISTAPE(dd, unit)){
		dd->d_state.t_state = NOTHING;
		if(dd->d_state.s_flags[unit] & SF_READY) {
			dd->d_state.t_flags &= ~(TF_WAIT_SECOND|TF_LT_DONE);
			dd->d_state.s_state = NOTHING;
			return;
		}
	}

	/* Do the init-sweep. */
	dd->d_state.s_state = INITIALIZING;

	/* Save the unit that started the sweep for i214intr */
	dd->d_state.s_sstart = unit;

	/*
	 * Do an init sweep on the very first open on any controller.
	 * Also when initializing winchester unit zero on 220, 215A/B or
	 * 214 with OMO V1.n firmware.  Sweep when opening a floppy on
	 * a 215A/B, or a 214 with OMO V1.n firmware.
	 * i214intr knows when to quit.
	 */
	i214io(dd, (struct buf *)NULL, INIT_OP,
		(int)((dd->d_state.s_1st_init ||
		(!IS214(dd) && (unit >= FIRSTFLOPPY)) ||
		(IS214(dd) && (unit >= FIRSTFLOPPY) &&
			!dd->d_state.s_1st_init &&
			!NOSWEEP_SPT(dd))) ? 0 : unit));
}


/*
 * Configure the controller according to the specified parameters.
 * Note - All checking and SPL locks are automatically set.
 */
static int
i214configbrd(dev, cdr)
dev_t		dev;
struct i214cdrt	*cdr;
{
	struct i214dev		*dd;
	struct iobuf		*bufh;
	unsigned int		x;
	unsigned short		rtnstat = 0;

	dd= i214bdd[BOARD(dev)];
	bufh= dd->d_state.s_bufh;

	x= SPL();
	/*
	 * Gain possesion of the controller by waiting till
	 * it is idle and then setting the busy flag.
	 */
	while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
		bufh->b_active |= IO_WAIT;
		(void) sleep((caddr_t) &dd->d_state.s_state, PRIBIO+1);
	}

#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		cmn_err(CE_CONT, "Reconfiguring controller to:\n          ");
		cmn_err(CE_CONT, "%d cyls, %d heads, and %d secs/trk\n",
			cdr->cdr_ncyl, cdr->cdr_nfhead, cdr->cdr_nsec);
	}
#endif /* DEBUG */

	/*
	 * Send the new parameters to the controller, wait for the
	 * response, and check the error status.
	 */
	i214sweep(dd, dev, cdr);
	while (dd->d_state.t_flags & TF_NO_BUFFER)
		(void) sleep((caddr_t) dd, PRIBIO+1);

	if (dd->d_state.s_error[UNIT(dev)] & ST_HARD_ERR) {
		i214print(dev, "ERROR SENDING NEW PARAMETERS TO CONTROLLER");
		rtnstat= EIO;
	}

	/*
	 * Restart normal I/O, enable interrupts and return status.
	 */
	i214start(dd);
	splx(x);
	return(rtnstat);
}



/*
 * Set the driver's internal partition table according to
 * the specified partition information.
 */
static int
i214setparts(dev, vtoc_pt, ptcnt)
dev_t			dev;
struct partition	*vtoc_pt;	/* VTOC partition table */
unsigned short		ptcnt;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;
	struct i214part		*pt;
	unsigned short		i;
	unsigned short		x;

	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];

#ifdef DEBUG
	if (i214messages & M214BBH) {
		cmn_err(CE_CONT, "Found %d new partitons:\n", ptcnt);
	}
#endif /* DEBUG */

	x=SPL();
	/*
	 * Copy partition info to driver's interal partition table.
	 * Also, zero-out any unspecified partitions.
	 */
	dr->dr_pnum= ptcnt;
	pt= &dr->dr_part[0];
	for (i=0; i < ptcnt; i++) {
		pt->p_tag= vtoc_pt->p_tag;
		pt->p_flag= vtoc_pt->p_flag;
		pt->p_fsec= vtoc_pt->p_start;
		pt->p_nsec= vtoc_pt->p_size;
		pt++;
		vtoc_pt++;
	}
	for (i= ptcnt; i < V_NUMPAR; i++) {
		pt->p_tag= 0;
		pt->p_flag= 0;
		pt->p_fsec= 0;
		pt->p_nsec= 0;
		pt++;
	}

#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		pt= dr->dr_part;
		for (i=0; i < ptcnt; i++) {
			cmn_err(CE_CONT, "    Part %d: tag %x flag %x fsec %lx nsec %lx\n",
				i, pt->p_tag, pt->p_flag, pt->p_fsec,
				pt->p_nsec);
			pt++;
		}
	}
#endif /* DEBUG*/

	splx(x);
	return(0);
}


/*
 *	Two-way elevator algorithm, based on sectors.
 */
#define	INWARD	1		/* Arm movement toward higher sectors	*/
#define	OUTWARD	0		/* Arm movement toward smaller sectors	*/
#define ADVANCE	bp1=bp2,bp2=bp2->av_forw
static void
i214disksort(dp, bp)
struct iobuf *dp;		/* Pointer to head of active queue	*/
struct buf *bp;			/* Pointer to buffer to be inserted	*/
{
	register struct buf *bp2; /* Pointer to next buffer in queue	*/
	register struct buf *bp1; /* Pointer where to insert buffer	*/
	int	 direct, this_d;  /* Direction of arm movement		*/
	daddr_t  sector;	  /* Disk address of target buffer	*/

	(void) drv_getparm(LBOLT, &bp->b_start);		/* probably a no-op */
	bp2 = dp->b_actf;
	/*
	 * trivial case: nothing active
	 */
	if (bp2 == NULL) {
		dp->b_actf = dp->b_actl = bp;
		bp->av_back = (struct buf *) dp;
		bp->av_forw = NULL;
		return;
	}
	/*
	 * advance thru all i/o queued
	 * with the same sector number
	 */
	do
		ADVANCE;
	while (bp2 && bp1->b_sector == bp2->b_sector);
	/*
	 * if all queued i/o has the same sector number,
	 * or if this request has the same sector number,
	 * simply queue at the tail of these i/o's, and exit.
	 */
	if (bp2 == NULL || (sector = bp->b_sector) == bp1->b_sector) {
		bp1->av_forw = bp;
		bp->av_back = bp1;		/* probably a no-op */
		bp->av_forw = bp2;
		if (bp2 == NULL)
			dp->b_actl = bp;
		else
			bp2->av_back = bp;	/* probably a no-op */
		return;
	}
	direct = (bp1->b_sector <= bp2->b_sector) ? INWARD : OUTWARD;
	this_d = (bp1->b_sector <= sector) ? INWARD : OUTWARD;
	/*
	 * find the correct sector position
	 */
	if (this_d == INWARD) {
		/*
		 * if direction is inconvenient, skip outward list
		 */
		if (direct == OUTWARD)
			while (bp2 &&
				bp1->b_sector >= bp2->b_sector)
				ADVANCE;
		/*
		 * find position in the inward list
		 */
		while (bp2 &&
			bp1->b_sector <= bp2->b_sector &&
			bp2->b_sector <= sector)
			ADVANCE;
	}
	else /* this_d is outward */ {
		/*
		 * if direction is inconvenient, skip inward list
		 */
		if (direct == INWARD)
			while (bp2 &&
				bp1->b_sector <= bp2->b_sector)
				ADVANCE;
		/*
		 * find position in the outward list
		 */
		while (bp2 &&
			bp1->b_sector >= bp2->b_sector &&
			bp2->b_sector >= sector)
			ADVANCE;
	}
	/*
	 * Queue the request between bp1 and bp2.
	 */
	bp1->av_forw = bp;
	bp->av_back = bp1;		/* probably a no-op */
	bp->av_forw = bp2;
	if (bp2 == NULL)
		dp->b_actl = bp;
	else
		bp2->av_back = bp;	/* probably a no-op */
}


/*
 *	i214rawsread:	i214 Raw Sector Read
 *
 *	Reads data from the specified device beginning with the specified
 *	absolute sector #.  One attempt is made to read 'count' bytes into
 *	the specified buffer.  The buffer MUST be large enough to hold all
 *	of the 'count' bytes.  The number of bytes actually read are
 *	returned to the caller.
 */
static int
i214rawsread(dev, sec, count, bp)
dev_t		dev;		/* Disk device to read from */
daddr_t		sec;		/* Absolute sector number to begin reading */
unsigned int	count;		/* # of bytes to read */
struct buf	*bp;		/* Buffer to hold the data */
{
	int		x;	/* Current interrupt level */
	struct i214dev		*dd;	/* Pntr to the device's dev structure */
	struct i214drtab	*dr;	/* Pntr to dev's dr_tab data struct */
	unsigned int		totsec;	/* Total # of sec of physical device */

	/*
	 * Find the proper dev structure.
	 */
	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];
	totsec= dr->dr_ncyl * dr->dr_spc;

	/*
	 * Make sure the device is ready to be used.
	 */
	if ((dd->d_state.s_flags[UNIT(dev)] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error= EBUSY;
		BIODONE(bp);
		return(0);
	}

	/*
	 * Make sure we're not starting beyond the end of the physical disk.
	 */
	if (sec >= totsec) {
		bp->b_flags |= B_ERROR;
		bp->b_error= ENXIO;
		BIODONE(bp);
		return(0);
	}

	/*
	 * Make sure we don't read past the end of the physical media.
	 */
	count= min(count, ((totsec - sec) * dr->dr_secsiz));

	bp->b_flags |= B_READ;
	bp->b_flags &= ~(B_DONE | B_ERROR);
	bp->b_error= 0;
	bp->b_blkno= sec * dd->d_drtab[UNIT(dev)].dr_lbps;
	bp->b_sector= sec;
	bp->b_edev= BASEDEV(dev);
	bp->b_proc= 0x00;
	bp->b_bcount= count;
	bp->b_resid= 0;

	/*
	 * Queue the request and wait for a response.
	 * Note - We don't use the strategy routine
	 * because we may want to read outside of the
	 * partition boundries.
	 */
	x= SPL();
	i214disksort(dd->d_state.s_bufh, bp);
	i214start(dd);
	splx(x);
	(void) biowait(bp);

	/*
	 * Check for errors.
	 */
	if (bp->b_flags & B_ERROR) {
		return(0);
	}

	return(count - bp->b_resid);
}



/*
 *	i214rawbread:	i214 Raw Byte Read
 *
 *	Read data from the specified device beginning with the specified
 *	absolute byte offset (from the beginning of the disk).  Data is
 *	repeatly read from the device until 'count' bytes have been read
 *	or until an error occures.  The data is placed in the specified
 *	destination address.  Optionally, the callers own buffer 'bp' can be
 *	used for reading.  If 'bp' has a value of '0' then a tempory buffer
 *	will be allocated.  The actual number of bytes read is returned to
 *	the user.
 */
static int
i214rawbread(dev, start, count, destaddr)
dev_t		dev;		/* Device (maj/min) to read from */
unsigned long	start;		/* Absolute byte offset to begin reading */
unsigned long	count;		/* Number of bytes to read */
char		*destaddr; 	/* Destination address to place the data */
{
	struct i214dev	*dd;		/* Ptr to the device's dev structure */
	char		*dest;		/* Destination addr of next data byte */
	char		*src;		/* Source addr of next data byte */
	struct buf *bp;
	daddr_t		sec;		/* Sector # of next byte to be read */
	unsigned int	secsize;	/* # of bytes in a disk sector */

	unsigned int	readlen;	/* # of bytes actually read from disk */
	unsigned int	offset;		/* Offset into sec to begin reading */
	unsigned int	copylen;	/* # of bytes to copy on this loop */
	ushort		i;		/* Loop counter */

	/*
	 * Setup the pointers and counters.
	 */
	dd= i214bdd[ BOARD(dev) ];
	secsize= dd->d_drtab[UNIT(dev)].dr_secsiz;

	bp= geteblk();
	if (bp == (struct buf *) 0) {
		i214print(dev, "Unable to get temporary buffer");
		return(0);
	}

	/*
	 * While there is still more data to be read, keep
	 * reading sectors from the disk and copy the data
	 * to it final destination.
	 */
	dest= destaddr;
	while (count != 0) {
		sec= start / secsize;
		offset= start % secsize;
		readlen= i214rawsread(dev, sec, BSIZE, bp);
		if (readlen <= offset) {
			break;
		}
		src= (char *) (bp->b_un.b_addr + offset);
		copylen= min(readlen-offset, count);
		for (i=0; i < copylen; i++)
			*dest++= *src++;
		start += copylen;
		count -= copylen;
	}

	brelse(bp);
	return(dest-destaddr);
}



/*
 * Read Intel Volume Lable (IVLAB) from disk and copy it to
 * to the specified location.
 */
static int
i214rdivlab(dev, ivlab)
dev_t		dev;
struct ivlab	*ivlab;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned int	ivlabloc;
	unsigned int	readcnt;
	struct buf		*bp;
	struct btblk	*btblk;

	board= BOARD(dev);
	unit= UNIT(dev);
	ivlabloc= i214winidata[board][unit].ivlabloc;

	if (ivlabloc == 0) {
		i214print(dev, "Unable to read IVLAB - Invalid location");
		return(ENXIO);
	}
	bp= geteblk();
	if (bp == (struct buf *) 0) {
		i214print(dev, "Read IVLAB: No tmp buffers");
		return(ENOMEM);
	}
	bp->b_flags |= (B_STALE | B_AGE);
	readcnt= i214rawsread(dev, BTBLK_LOC, sizeof(*btblk), bp);
	if (readcnt != sizeof(*btblk)) {
		i214print(dev, "Unable to read IVLAB sector - Disk I/O Error");
		brelse(bp);
		return(EIO);
	}
	btblk = (struct btblk *) bp->b_un.b_addr;
	if (btblk->signature != BTBLK_MAGIC) {
		i214print(dev, "Invalid VOLUME LABEL sector found");
		brelse(bp);
		return(ENXIO);
	}
	*ivlab = btblk->ivlab;
	brelse(bp);

#ifdef DEBUG
	if(i214messages & (M214OPEN | M214BBH)) {
		struct i214drtab	*dr;
		dr = (struct i214drtab *) &ivlab->v_dspecial[0];
		cmn_err(CE_CONT, "i214open: Volume label says disk has:\n");
		cmn_err(CE_CONT, " %d cylinders, %d heads, and %d sectors/track\n",
			dr->dr_ncyl, dr->dr_nfhead, dr->dr_nsec);
	}
#endif /* DEBUG */

	return(0);
}



/*****************************************************************************
 *
 * 		Queue an I/O Request, and start it if the controller
 *		is not busy already.
 *
 *		Check legality, and adjust for partitions.  Reject request if
 *		unit is not-ready.
 *
 *	Note:	check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.  214 gives
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
i214strategy(bp)
register struct buf *bp;	/* buffer header */
{
	register struct i214dev	 *dd;
	struct i214drtab *dr;
	struct i214part *p;
	daddr_t		secno;
	int	x;
	unsigned	unit;
	unsigned        bytes_left;

#ifdef DEBUG
	if (i214messages & M214STRAT)
		cmn_err(CE_CONT, "i214strat: bp->bdev is: %x\n",bp->b_edev);
#endif /* DEBUG */
	/* initializations */
	dd = i214bdd[BOARD(bp->b_edev)];
	unit = UNIT(bp->b_edev);
	dr = &dd->d_drtab[unit];
	p = &dr->dr_part[PARTITION(bp->b_edev)];

	/* set b_resid to b_bcount because we haven't done anything yet */
	bp->b_resid = bp->b_bcount;
	if (bp->b_resid % dd->d_drtab[unit].dr_secsiz) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		BIODONE(bp);
		return;
	}
	if ((dd->d_state.s_flags[unit] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;		/* not ready */
		bp->b_error = ENXIO;
		BIODONE(bp);
		return;
	}

	/*
	 *	See if partition is valid.  If not, error.
	*/
	if(!(p->p_flag & V_VALID)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		BIODONE(bp);
		return;
	}

	/*
	 * Figure "secno" from b_blkno. Adjust sector # for partition.
	 * Check if ready, and see if fits in partition.
	 *
	 * If reading just past the end of the device, it's
	 * End of File.  If not reading, or if read starts further in
	 * than the first sector after the partition, it's an error.
	 */
	/* secno is logical blockno / # of logical blocks per sector */
	/* UNLESS IT'S A 128-byte sectored FLOPPY!!! */
	if (dr->dr_lbps)
		secno = (unsigned)bp->b_blkno / dr->dr_lbps;
	else
		secno = bp->b_blkno * (NBPSCTR/dr->dr_secsiz);
#ifdef DEBUG
	if (i214messages & M214STRAT)
		cmn_err(CE_CONT, "i214strat: secno is: %x\n",secno);
#endif /* DEBUG */

	if (secno >= p->p_nsec) {
		if (!((bp->b_flags & B_READ) && (secno == p->p_nsec)))
		{
			/* off the deep end */
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		BIODONE(bp);                            /* finish this */
		return;
	}

	/*	At this point, it is no longer possible to directly return from
	 *	strategy.  We now set b_resid to the number of bytes we cannot
	 *	transfer because they lie beyond the end of the request's partition.
	 *	This value is 0 if the entire request is within the partition.
	*/
	x = SPL();
	bytes_left = (p->p_nsec - secno) * dr->dr_secsiz;
	bp->b_resid = ((bp->b_bcount <= bytes_left)
			? 0 : (bp->b_bcount - bytes_left));
	if (bp->b_bcount == bp->b_resid)
		cmn_err(CE_PANIC, "i214strategy: zero length xfer 0x%x\n", bp);

	secno += p->p_fsec;
	bp->b_sector = secno;


	 i214disksort(dd->d_state.s_bufh, bp);	/* queue the request */

	/*
	 * If no requests are in progress, start this one up.  Else
	 * leave it on the queue, and i214intr will call i214start later.
	 */
	i214start(dd);

	splx(x);
}



/*******************************************************************************
 *
 * 	Determines firmware version support level.
 *
 *	Support level determined as follows:
 *
 *	level 0 - I believe this is OMO V1.n; treat it the same as a 215B.
 *	level 1 - OMO V2.0, V2.1, V2.2; doesn't require init sweep when
 *		initializing floppies, adds modifier bit to disable RAM
 *		test when initializing winchester unit zero.
 *	level 2 - OMO V2.3, ISO-N V1.1; adds full Kennedy (start/stop)
 *		tape support.
 *	level 3 - The current version from ISO-S, V2.4; Kennedy support has
 *		been removed, 24-bit addressing added, QIC-2 (Archive)
 *		tape support added, firmware ID changed from OMO to ISO-S.
 *	level 4 - The current version from ISO-N, V1.2.  A timeout
 *		problem with 5-1/4 inch tape drives has been fixed.  The
 *		problem can crash winchesters, so it is necessary that
 *		tapes not be supported until level 4.
 *
 *	According to the experts, the next version done by OMO will be
 *	ISO-S V2.5.  The next version done by ISO-N will be ISO-N V1.3.
 *	Therefore, we will assume that OMO V2.4 and on are invalid version
 *	numbers.  There was no ISO-N V1.0, but ISO-N V1.3 and on should be
 *	valid.  ISO-S ID wasn't used before V2.4, so we will consider
 *	ISO-S V1.0 through V2.3 invalid also.
 *
 *	This code assumes that OMO will incorporate ISO-N changes in its
 *	future firmware versions.  When new releases come out, this should
 *	be checked carefully.
 *
 ******************************************************************************/
static void
i214version(dd)
register struct i214dev *dd;	/* driver data area */
{
	unsigned division;
	unsigned version;
	register unsigned rev_level;
	extern	char *i214b_type[];


	/* find originating division */
	division = dd->d_iopb.i_actual & 0x3;

	/* find version number */
	version = ((dd->d_iopb.i_actual >> 6) & 0x3) + 1;

	/* find revision level */
	rev_level = (dd->d_iopb.i_actual >> 2) & 0xF;

	switch (division) {
	case OMO:
		if (version == 1)
			/* This is really old stuff - treat it as a 215B. */
			dd->d_state.s_support = 0;

		else if (version == 2) {	/* check revision level */
			if (rev_level <= 2)	/* rev 0, 1 or 2 */
				dd->d_state.s_support = 1;

			else if (rev_level == 3)	/* Kennedy support */
				dd->d_state.s_support = 2;

			else {
				/*
				 * Illegal - division changed
				 * from OMO to ISO-S at V2.4.
				 */
				dd->d_state.s_support = 0;
				break;
			}
		}
		else {
			/*
			 * Version 3 or 4.  Illegal firmware version -
			 * division changed from OMO to ISO-S at V2.4.
			 */
			dd->d_state.s_support = 0;
			break;
		}
		break;
	case ISO_N:
		if (version == 1) {	/* check revision level */
			if (rev_level == 0) {
				/* Illegal - ISO-N versions started at V1.1. */
				dd->d_state.s_support = 0;
				break;
			}
			else if (rev_level == 1)	/* Approx. equal to OMO V2.3. */
				dd->d_state.s_support = 2;

			else if (rev_level >= 2)
				/*
				 * V1.2 is the next one out of ISO-N;
				 * Versions 1.3 and up don't exist yet,
				 * but will be valid.  Assume level 4.
				 */
				dd->d_state.s_support = 4;
		}
		else
			dd->d_state.s_support = 4;
		break;

	case ISO_S:
		if (version == 1) {
			/* ISO-S V1.n doesn't exist */
			dd->d_state.s_support = 0;
			break;
		}
		else if (version == 2) {	/* check revision level */
			if (rev_level <= 3) {
				/*
				 * Illegal - ISO-S ID not
				 * used until after OMO V2.3.
				 */
				dd->d_state.s_support = 0;
				break;
			}
			else if (rev_level == 4)
				/* V2.4 is latest from OMO/ISO-S */
				dd->d_state.s_support = 3;
			else
				/* V2.5 and up will be good.  */
				dd->d_state.s_support = 4;
		} else	/* V3.? and 4.? will be good */
			dd->d_state.s_support = 4;

		break;
	default:	/* Nobody uses this ID */
		dd->d_state.s_support = 0;
		break;
	}

	/*
	 * Print out the firmware version.
	 */
	cmn_err(CE_CONT, "iSBC %s controller %d Firmware: ",
		i214b_type[dd->d_state.s_support], dd->d_state.s_board);
	if(dd->d_state.s_support != 0) {
		switch (division) {
		case OMO: cmn_err(CE_CONT, "OMO");
			break;
		case ISO_N: cmn_err(CE_CONT, "ISO-N");
			break;
		case ISO_S: cmn_err(CE_CONT, "ISO-S");
			break;
		}
		cmn_err(CE_CONT, " V%d.%d\n", version, rev_level);
	} else {
		cmn_err(CE_CONT, "Unknown Version. Default 215B Support\n");
	}

}



/*******************************************************************************
 *
 * 	Handle interrupt.
 *
 *	Interrupt procedure for 214 driver.  Gets status of last
 *	operation and performs service function according to the
 *	type of interrupt.  If it was an operation complete interrupt,
 *	switches on the current driver state and either declares the
 *	operation done, or starts the next operation and sets
 *	dd->d_state.s_state to the next state.
 *
 *	We can actually get three kinds
 *
 ****************************************************************************/
void
i214intr(level)
int	level;			/* interrupt level */
{
	register struct	i214dev		*dd;
	register struct	i214state	*st;
	struct	buf	*bp;
	struct	iobuf	*bufh;
	unsigned	spindle;
	unsigned	status;
	unsigned	istape;
	char		err_stat;
	dev_t		t_buf_dev;
	struct	buf	*t_bp;
#ifdef BBFAKE
	int		i;		/* DEBUGGING */
#endif /* BBFAKE */
	extern char	*i214b_type[];

	dd = i214ldd[level];		/* map level -> board */
	st = &dd->d_state;		/* dereference d_state */

#ifdef DEBUGY
	debdata.intcnt++;
	debdata.status = dd->d_cib.c_stat;
	debdata.intlast = iocounter;
#endif /* DEBUGY */

#ifdef DEBUG
	if(i214messages & M214INTR)
		cmn_err(CE_CONT, "i214intr: level %x\n",level);
#endif /* DEBUG */

	/* Clear the interrupt and make sure that the interrupt is real. */
	outb(base86(st->s_wua), WAKEUP_CLEAR_INT);

	if ((!st->s_exists) || (dd->d_cib.c_statsem == 0)) {
		cmn_err(CE_CONT, "iSBC %s: spurious interrupt, level %d\n",
			i214b_type[st->s_support], level);
		return;
	}

	status = dd->d_cib.c_stat;	/* Get controller status */
	dd->d_cib.c_statsem = 0;	/* Clear status semaphore */

#ifdef DEBUG
	if(i214messages & M214INTR)
		cmn_err(CE_CONT, "i214intr: status=%x, s_state=%x, t_flags=%x\n",
			status, st->s_state, st->t_flags);
#endif /* DEBUG */

	/*
	 * Set spindle to the spindle number of the interrupting
	 * device: 0-3 hard disk / 4-7 floppies / 8-11 tape
	 */
	spindle = (status & ST_UNIT) >> 4;

	/*  set up the spindle number for floppy.  */
	if (status & ST_FLOPPY)
		spindle += FIRSTFLOPPY;		/* normal floppy mode */

	/* Here we check to see if it is actually a tape */
	if (((status & ST_LONG_COMPL) == ST_LONG_COMPL)
		|| ((status & ST_LONG_COMPL) == ST_TAPE_MEDIA)
		|| (st->s_opunit >= FIRSTTAPE))
		spindle += NEXT_REMOVE_UNIT;	/* tape mode */

	istape = ISTAPE(dd, spindle);

	/*
	 * See what kind of operation is done.  In each case, this function
	 * will either start the controller doing something and return from
	 * interrupt handling until another interrupt, or it will drop thru
	 * to the bottom of the function and start processing the next I/O
	 * request.
	 * Note:  Only interrupt possible on unopened unit is media-change.
	 *
	 * Note:  There are three parts to the following if statement:
	 *		if short term complete
	 *			A particularly large and nasty case statement
	 *		else if media change
	 *		else long term complete
	 */
	if ((status & ST_COMPL_MASK) == ST_OP_COMPL) { /* Short Term */

		bufh = st->s_bufh;		/* Get buf header */
		if (st->t_flags & TF_NO_BUFFER)
			bp = NULL;				/* Preserve "request not from actf" */
		else
			bp = bufh->b_actf;		/* Current active buffer  */

		dd->d_state.s_bufh->b_active &= ~IO_BUSY;

#ifdef BBFAKE
		/*************************************************************
			DEBUGGING: code added to allow the simulation
			of a bad block
		 */
		for (i=0; i < i214_bb_max; i++) {
			if ((bp->b_blkno == i214fake[i].f_block_num) &&
				(bp->b_edev == i214fake[i].f_device)) {
#ifdef DEBUGBB
				cmn_err(CE_CONT, "i214intr: found a fake bad block.\n");
				monitor();
#endif /* DEBUGBB */
				dd->d_state.s_error[spindle] = i214fake[i].f_serror;
				if (i214fake[i].f_serror == ST_HARD_ERR) {
					dd->d_error.e_hard = i214fake[i].f_hard_soft;
					cmn_err(CE_CONT, "e_hard: %d\n", dd->d_error.e_hard);
				}
				else {
					dd->d_error.e_soft = (char)i214fake[i].f_hard_soft;
					cmn_err(CE_CONT, "e_soft: %d\n", dd->d_error.e_soft);
				}
				err_stat = i214checkerr (dd, spindle);
				bp->b_flags |= B_ERROR;
				bp->b_error = err_stat;
				goto finish_fake;   /* arg, i can't stand this!!! */
			}
		}
		/*
			END of i025 DEBUGGING code
		 *************************************************************/
#endif /* BBFAKE */
		/*
		 * st->s_state was set to indicate what kind of operation was
		 * started.  Note that each case must check for errors for itself.
		 */
		switch (st->s_state) {

		case RVENDLIST:
			/*
			 * Return from request to read vendor defect list.
			 */
			dd->d_state.s_error[spindle] = status	;
			st->s_state = NOTHING;	
			break						;
			 
		case WRITEFM:
			/*
			 * We tried to write a filemark to the tape.
			 * Note that this operation will change the tape drive
			 * from WRITE state to NOTHING state.
			 */
			if (status & ST_HARD_ERR) {
				st->s_error[spindle] = status;
				if (((st->t_flags & TF_NO_BUFFER) == 0) && istape)
					bp->b_resid -= dd->d_iopb.i_actual;
				st->t_state = NOTHING;
				err_stat = i214checkerr(dd,spindle);
				bp->b_flags |= B_ERROR;
				bp->b_error = err_stat;
			}
			st->s_state = NOTHING;
			break;

		/*
		 * Normal read/write operation complete interrupt.
		 * Check status for error; on error get status
		 * information from controller.  Otherwise declare
		 * operation done.
		 * For disks, this is a simple operation.
		 * For tapes, however, we must check to see if we hit a filemark.
		 * If we are in the process of reading to a file mark, we need
		 * to requeue the buffer (or if there is nothing on the queue,
		 * just restart the I/O).
		 */
		case NOTHING:
			if (status & ST_HARD_ERR) {
				/*
				 * Ask the controller what's wrong and
				 * return, pending another interrupt.
				 */
				st->s_error[spindle] = status;
				if (((st->t_flags & TF_NO_BUFFER) == 0) && istape)
					bp->b_resid -= dd->d_iopb.i_actual;
				st->s_state = GET_BAD_STATUS;
				i214io(dd, bp, STATUS_OP, (int)spindle);
				return;
			}
			/* See if we got a file mark while reading a tape */
			if ((status & ST_ERROR) && istape &&
				(st->t_state == TS_READING)) {
				/* update count */
				if ((st->t_flags & TF_NO_BUFFER) == 0)
					bp->b_resid -= dd->d_iopb.i_actual;
				if(st->rtfm_buf == bp)
					st->s_state = T_RTFM_STATUS;
				else
					st->s_state = T_SOFT_STATUS;
				i214io(dd, bp, STATUS_OP, (int)spindle);
				return;
			}

			/* Successful operation complete; drop out of switch */
			if (!(st->t_flags & TF_NO_BUFFER)) {
				if (istape) {
					if(st->rtfm_buf != bp)	/* Normal tape I/O?? */
						bp->b_resid -= dd->d_iopb.i_actual;
					else {		/* Read to filemark buffer */
						if(bp->av_forw == (struct buf *)0) {	/* Queue empty?? */
							i214io(dd, bp, READ_OP, (int)spindle);
							return;
						} else {	/* Queue not empty, put rtfm buf on the end */
							bufh->b_actf = bp->av_forw;
							t_bp = bufh->b_actl;
							if ((bp->av_forw = t_bp->av_forw) == (struct buf *)(0))
								bufh->b_actl = bp;
							t_bp->av_forw = bp;
							bp->av_back = t_bp;
						}
					}
				}
			}
			/*
			 * Disk I/O done.  Decrement b_resid by number of bytes
			 * transferred.  Note that d_ext.xfer_count is
			 * incremented so that we can keep track of where
			 * we are in the user's buffer.
			 * Check to see if we've fulfilled our request
			 * (for transfers that have bad sectors in a portion
			 * of the request).  If we still need to transfer more,
			 * increment b_sector by the amount actually
			 * transferred and fire up controller for next chunk.
			 * This is only done for disk I/O.
			 */
			if (!istape && !(st->t_flags & TF_NO_BUFFER)) {
			    ulong xfrcnt = dd->d_iopb.i_actual;
			    register ulong phys = dd->d_ext.xfer_addr;
			    /*
			     * If this is a read, copy the data from the
			     * controller's buffer to the user's buffer.
			     * (Whether we used the buffer is independent
			     * of xfrcnt.)
			    */
			    if (IO_OP(bp) == READ_OP) {
			    	if ((phys + dd->d_ext.xfer_count) >= i214dma_limit) {

						bcopy(st->d_buf, (caddr_t)phystokv(phys), xfrcnt);
					}
				}
			    bp->b_resid -= xfrcnt;
			    dd->d_ext.xfer_count -= xfrcnt;
				if ((long)dd->d_ext.xfer_count < 0) {
					cmn_err(CE_CONT, "xfrcnt == %d, i_actual == %d\n",
								xfrcnt, dd->d_iopb.i_actual);
					cmn_err(CE_PANIC, "i214intr: count < 0\n");
			    }
			    if (dd->d_ext.xfer_count > 0) {
					bp->b_sector += xfrcnt / dd->d_drtab[spindle].dr_secsiz;
					dd->d_ext.xfer_addr += xfrcnt;
					i214io(dd, bp, IO_OP(bp), (int)spindle);
					return;
				}
			}
			break;

		/*
		 * Have read status information from controller.  If it was
		 * a hard error, print message and quit.  If there was a
		 * soft error, start a diagnostic recal function.
		 * This keeps us from having to handle a long-term interrupt
		 * for disks, as well as being more accurate.
		 * If the device was a tape, we need to dequeue the read ahead/
		 * write behind buffers so that the tape doesn't thrash.
		 *
		 * Write out a file mark if the error was 'logical end
		 * of tape' while writing to the tape.  It turns out that
		 * the Tandberg drives don't automatically write out a file
		 * mark at end of tape, while the Archive drives do.  The
		 * extra file mark at the end of the Archive tape won't
		 * hurt anything, but NOT having one on a tape written by
		 * a Tandberg drive screws up the Archive drive.
		 */
		case GET_BAD_STATUS:
			if (istape) {
				/*
				 * Dequeue any buffers associated with the device.
				 */
				if(bp) {
				    t_buf_dev = bp->b_edev;
				    while(((t_bp=bp->av_forw) != NULL) &&
						    (t_bp->b_edev == t_buf_dev)) {
					    bufh->b_actf = bp->av_forw;
					    BIODONE(bp);
					    bp = t_bp;
				    }
				}
#ifdef DEBUG
				else cmn_err(CE_CONT, "i214intr: GET_BAD_STATUS with no bp\n");
#endif /* DEBUG */

				st->t_flags &= ~(TF_LONG_TERM|TF_READING_TO_FM);
				/*
				 * See if our friendly user removed
				 * the tape cartridge.
				 */
				if (dd->d_error.e_hard &
					 (HARD_NOT_READY|HARD_NO_CARTRIDGE)) {
					st->s_flags[spindle] &= ~SF_READY;
				} else {
					if ((st->t_state == TS_WRITING) &&
						dd->d_error.e_leot) {
						st->t_flags |= TF_AT_LEOT;
						st->t_state = NOTHING;
					/*
					 * Send the write file mark command -
					 * we'll end up back here to finish up
					 */
						i214io(dd, bp, WRFM_OP,(int)spindle);
						st->s_state = WRITEFM;
						return;
					}
				}

				st->t_state = NOTHING;
			}
			st->s_state = NOTHING;

			if (((st->t_flags & TF_NO_BUFFER) == 0) &&
				!(istape && (dd->d_error.e_no_data))) {
				err_stat = i214checkerr(dd, spindle);
				if (((err_stat == EIO)/* ||
					(err_stat == (char)EBBHARD) ||
					(err_stat == (char)EBBSOFT)*/) && !istape) {
					/* restore and retry */
					st->s_state = RESTORING;
					i214io(dd, bp, DIAGNOSTIC_OP, (int)spindle);
					return;	/* wait for interrupt */
				} else {
					bp->b_flags |= B_ERROR;
					bp->b_error = err_stat;
				}
			}
			break;

		/*
		 * Was reading a tape and got a soft error.  Check for
		 * a file mark and reset tape state to NOTHING if found.
		 * If a file mark is found, dequeue all buffers owned
		 * by the same device that are queued adjacently to
		 * this buffer.
		 */
		case T_SOFT_STATUS:
			st->s_state = NOTHING;
			if (dd->d_error.e_fm_found) {
				if((st->t_flags & TF_FM_ALWAYS) || (bp->b_resid == 0))
					st->t_flags |= TF_FOUND_FM;
				t_buf_dev = bp->b_edev;
				while(((t_bp=bp->av_forw) != NULL) &&
						(t_bp->b_edev == t_buf_dev)) {
					bufh->b_actf = bp->av_forw;
					BIODONE(bp);
					bp = t_bp;
				}
				st->t_state = NOTHING;
			}
			break;

		/*
		 * Have just completed a diagnostic recalibration to track 0.
		 * If haven't exhausted retry-count, then retry;
		 * Else, print diagnostic & terminate.
		 *
		 */
		case RESTORING:
			st->s_state = NOTHING;
			if (++bufh->b_errcnt < i214retry) {
				i214io(dd, bp, IO_OP(bp), (int)spindle); /* retry */
				return;
			}
			bp->b_flags |= B_ERROR;
			break;

		/*
		 * Have just initialized a unit.  Step to the next unit
		 * and start initializing it.  Return status after the
		 * last unit is initialized.
		 *
		 * If there is an error while initializing a unit, record it,
		 * but keep going so that all units which were attached are
		 * initialized.
		 *
		 */
		 case INITIALIZING:
			/*
			 * If we just did the first initialization of
			 * winchester unit zero on a 214, set the support
			 * level appropriately.
			 */
			if ((st->s_opunit == 0) && st->s_1st_init && IS214(dd))
				i214version(dd);

			st->s_error[st->s_opunit] = status;

			/* Increment to next unit */
			if (!IS214(dd) ||
				(IS214(dd) && !st->s_1st_init &&
				!NOSWEEP_SPT(dd)))
				st->s_opunit++;
			else if (st->s_opunit != st->s_sstart)
				st->s_opunit = st->s_sstart;
			else
				st->s_sstart = 0;

			/* Determine when to quit. */
			if ((IS220(dd) && (st->s_opunit >= FIRSTFLOPPY)) ||	/* 220 and unit is floppy */
				(!IS214(dd) && (st->s_opunit >= FIRSTTAPE)) ||	/* Not 214 and unit is a tape */
				(IS214(dd) && !st->s_1st_init &&		/* 214 and first init has */
					NOSWEEP_SPT(dd)) ||			/* already been done and support */
										/* level is nonzero */
				(!st->s_1st_init && (st->s_sstart != 0) &&	/* first init done and the unit */
					(st->s_sstart < FIRSTFLOPPY)) ||	/* that started the sweep was */
										/* winchester unit 1, 2 or 3 */
				(IS214(dd) && st->s_1st_init &&		/* 214 in init sweep; the unit */
					(st->s_sstart == 0) &&			/* that started it was winnie */
					NOSWEEP_SPT(dd)) ||			/* 0 and support level is nonzero */
				(st->s_opunit >= NUMSPINDLE)) {			/* unit is out of range */

				/* Reset first init flag */
				st->s_1st_init = 0;
				/*
				 * Just got back from the first command in
				 * the tape initialization sequence.  Check
				 * for errors; if none, send next command in
				 * the sequence, TAPEINIT_OP.
				 */
				if (istape) {
					if (!TAPE_SPT(dd))
						st->s_error[spindle] = ST_HARD_ERR;
					else if ((status & ST_HARD_ERR) == 0) {
						st->s_state = T_INIT;
						i214io(dd, bp, TAPEINIT_OP, (int)spindle);
						return;
					} else {
						st->s_state = GET_BAD_STATUS;
						i214io(dd, bp, STATUS_OP, (int)st->s_opunit);
						return;
					}
				}
				st->s_state = NOTHING;
			} else {
				i214io(dd, bp, INIT_OP, (int)st->s_opunit);
				return;
			}
			break;

		/*
		 * Here we are after sending the second command in the
		 * tape init sequence.  Next command is TAPERESET_OP if
		 * there were no errors.
		 */
		case T_INIT:
			if (status & ST_HARD_ERR) {
				/* mark error for sweep to find */
				st->s_error[st->s_opunit] = status;
				st->s_state = GET_BAD_STATUS;
				i214io(dd, bp, STATUS_OP, (int)spindle);
			} else {
				st->s_state = T_RESET;
				i214io(dd, bp, TAPERESET_OP, (int)spindle);
			}
			return;

		/*
		 * OK, tape reset (third command in sequence) is done,
		 * now it's time to send the load tape command to the
		 * 214 (assuming no errors).
		 */
		case T_RESET:
			if (status & ST_HARD_ERR) {
				st->s_error[st->s_opunit] = status;
				st->s_state = GET_BAD_STATUS;
				i214io(dd, bp, STATUS_OP, (int)spindle);
			} else {
				st->s_state = NOTHING;
				i214io(dd, bp, LOADTAPE_OP, (int)spindle);
			}
			return;

		/*
		 * Check returned status bytes to see if we found a
		 * file mark so we can quit reading.
		 */
		case T_RTFM_STATUS:
			if (dd->d_error.e_fm_found) {	/* Yay! */
				/* Clear the filemark found bit. */
				/* The user told us to go by it. */
				st->t_flags &= ~(TF_READING_TO_FM|TF_FOUND_FM);
				st->s_state = NOTHING;
				st->t_state = NOTHING;
				wakeup((caddr_t)bp);	/* Wake the rtfm sleeper */
			} else {	/* Keep looking */
				st->s_state = NOTHING;
				if(bp->av_forw == (struct buf *)0) {
					i214io(dd, bp , READ_OP, (int)spindle);
					return;
				} else {	/* There is other stuff waiting */
					bufh->b_actf = bp->av_forw;
					t_bp = bufh->b_actl;
					if ((bp->av_forw = t_bp->av_forw) == (struct buf *)(0))
						bufh->b_actl = bp;
					t_bp->av_forw = bp;
					bp->av_back = t_bp;
				}
			}
			break;
		}	/* end of s_state switch */
#ifdef BBFAKE
finish_fake:
#endif /* BBFAKE */

		/*
		 * I/O done; notify next of kin
		 * If the TF_NO_BUFFER is set, somebody is waiting
		 * for the completion of this I/O.  Wake them up.
		 */
		if (st->t_flags & TF_NO_BUFFER) {
			st->t_flags &= ~TF_NO_BUFFER;
			if (st->t_flags & TF_LONG_TERM) {
				st->t_flags &= ~TF_LONG_TERM;
				st->t_flags |= TF_WAIT_SECOND;
			}
			wakeup((caddr_t)dd);
		} else {
			/*
			 * If this is not the reading to file mark buffer,
			 * Pop the current buffer off of the queue and
			 * call BIODONE.
			 */
			if(!(st->t_flags & TF_READING_TO_FM) || (st->rtfm_buf != bp)) {
				bufh->b_actf = bp->av_forw;
				BIODONE(bp);
			}
		}
	}	/* end of short-term (immediate) operation complete */

	/* Check for media change interrupt and set not ready if so.  */
	/* We basically ignore media change on floppies.  On tapes */
	/* however, we will keep careful track of whether the tape */
	/* has been initialized or not. */
	else if((status & ST_MCHANGE_MASK) == ST_MEDIA_CHANGE) {
		if((st->s_flags[spindle]&(SF_OPEN|SF_READY)) == (SF_OPEN|SF_READY)) {
			i214checkerr(dd, spindle);
		}
		st->s_flags[spindle] &= ~SF_READY;
		return;
	}

	/*
	 * Long-term interrupt - shouldn't get here for disks;
	 * driver does no explicit seeks.
	 */

	else {
		/*
		 * Handle a long-term tape operation.  This operation was
		 * invoked from open, close or ioctl and has no buffer
		 * associated with it; means the call is sleeping and
		 * needs a wakeup.
		 */
		if (ISTAPE(dd, spindle)) {
			st->s_error[spindle] = status;

			/* reset long-term flag */
			st->t_flags &= ~TF_WAIT_SECOND;

			if (st->t_flags & TF_IM_WAITING) {
				st->t_flags &= ~TF_IM_WAITING;
				wakeup((caddr_t)&st->t_flags);
			} else
				st->t_flags |= TF_LT_DONE;
		} else
			cmn_err(CE_CONT, "iSBC %s level %d  (invalid long-term interrupt)\n",
				i214b_type[st->s_support], level);
		return;
	}

	i214start(dd);		/* start up next I/O request on queue */
}


static long
i214blksize(dev)
register	dev_t	dev;
{
	long 				nblks;
	struct	i214dev		*dd;
	struct	i214drtab	*dr;
	unsigned short 		secsiz;
	unsigned			unit;


	dd = &i214dev[BOARD(dev)];
	unit = UNIT(dev);
	dr = &dd->d_drtab[unit];
	nblks = dr->dr_part[PARTITION(dev)].p_nsec;
	secsiz = dd->d_drtab[unit].dr_secsiz;
	/* convert to 512 byte blocks */
	if (secsiz < 512)
		nblks = (long)(nblks*secsiz) / 512;
	else
		nblks = nblks * (secsiz/512);

	return(nblks);
}




/*
 *	itpbreakup - break buffers into contiguous chunks.
*/
void
i214breakup(bp)
struct buf *bp;
{
	dma_pageio(i214strategy, bp);
}


/*****************************************************************************
 *
 * 	"Raw" read.  Use physio().
 *
 ****************************************************************************/
/*ARGSUSED*/
i214read(dev, uio_p, cred_p)
register	dev_t    dev;
struct		uio		*uio_p;
struct		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = i214blksize(dev);
	error_code = physiock(i214breakup, NULL, dev, B_READ,num_blks,uio_p);
	return(error_code);
}

/*****************************************************************************
 *
 * 	"Raw" write.  Use physio().
 *
 ****************************************************************************/
/*ARGSUSED*/
i214write(dev, uio_p, cred_p)
register	dev_t	dev;
struct 		uio 	*uio_p;
struct 		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = i214blksize(dev);
	error_code = physiock(i214breakup, NULL, dev, B_WRITE,num_blks,uio_p);
	return(error_code);
}



/*****************************************************************************
 *
 * 	Return the most appropriate error.
 *
 *	Called from i214intr to determine hard error or retry exhaustion.
 *	This is a separate function for debugging and diagnostic purposes.
 *	It is easy to trap to and see what's going on.
 *
 * 	Assumes that the controller's transfer status function has been
 *	executed.  It decodes the bits and bytes in the error status
 *	buffer to decide what the most appropriate error to return is.
 *
 ****************************************************************************/
static char *i214er_msg[] = {
	"?",
	"Invalid Function",
	"ROM Checksum Error",
	"RAM Self-test Error",
	"Long Term Operation In Progress",
	"Drive Not Present",
	"End Of Media",
	"Length Error",
	"Command Timed Out",
	"Media Not Present",
	"Invalid Data Address",
	"Drive Not Ready",
	"Media Write Protected",
	"Recoverable Soft Error",
	"Drive Interface Error",
	"Drive Faulted",
	"Buffer Over/Under Run",
	"Seek Error",
	"Format Error",
	"Illegal Sector Size",
	"Diagnostic Failure",
	"Missing Index Pulse",
	"Sector Not Found",
	"CRC Error",
	"Cylinder Address Miscompare",
	"Drive Configuration Error",
	"Invalid Operation"
};

struct er_tbl {
	unsigned er_type;		/* error status from 214/214 */
	unchar	er_num;			/* Unix error number */
	unchar	er_bmsg;		/* block device error message */
	unchar	er_tmsg;		/* tape device error message */
};

static struct er_tbl i214er_hard[] = {
/*	Error Type,		Error Number,	Block,		Tape		*/
	HARD_214_REJECT,	EIO,		1,		1,
	HARD_217_REJECT,	EIO,		1,		1,
	HARD_DRIVE_REJECT,	EIO,		1,		1,
	HARD_214_RAM_ERR,	EIO,		3,		3,
	HARD_214_ROM_ERR,	EIO,		2,		2,
	HARD_LT_IN_PROGRESS,	EIO,		4,		4,
	HARD_CONFIGURATION,	EIO,		18,		25,
	HARD_END_OF_MEDIA,	ENOSPC,		6,		6,
	HARD_DIAG_FAULT,	EIO,		20,		20,
	HARD_TIME_OUT,		EIO,		21,		8,
	HARD_INVALID_FUNC,	EIO,		1,		1,
	HARD_NO_SECTOR,		EBBHARD,	22,		9,
	HARD_INVALID_ADDR,	EIO,		10,		10,
	HARD_NOT_READY,		ENXIO,		11,		11,
	HARD_WRITE_PROT,	ENODEV,		12,		12,
	HARD_LENGTH_ERR,	EBBHARD,	19,		7,
};
#define	ER_HARD_MAX	16

static struct er_tbl i214er_soft[] = {
/*	Error Type,		Error Number,	Block,		Tape		*/
	SOFT_TAPE_ERROR,	EIO,		0,		1,
	SOFT_CABLE_CHECK,	EIO,		14,		14,
	SOFT_DATA_ERROR,	EBBSOFT,	23,		23,
	SOFT_ID_CRC,		EBBSOFT,	23,		0,
	SOFT_DRIVE_FAULT,	EIO,		15,		15,
	SOFT_OVER_UNDER_RUN,	EBBSOFT,	24,		16,
	SOFT_SEEK_ERR,		EBBSOFT,	17,		0,
};
#define	ER_SOFT_MAX	7


char
i214checkerr(dd, spindle)
struct i214dev *dd;			/* device data area */
unsigned spindle;			/* unit number */
{
	char	err_status = NULL;	/* error number returned */
	int	i;			/* loop control variable */
	int	index = 0;		/* error message table index */
	int	hard_err = 0;		/* hard error flag */
	extern	char *i214b_type[];

#ifdef DEBUG
	if (i214messages & M214CHECK)
		cmn_err(CE_CONT, "i214checkerr: hard=%x  soft=%x\n",
			dd->d_error.e_hard, dd->d_error.e_soft);
#endif /* DEBUG */
	/*
	 * First check for hard errors
	 * if no hard errors then check
	 * for soft errors.
	 */
	for(i = 0; i < ER_HARD_MAX; i++) {
		if(dd->d_error.e_hard & i214er_hard[i].er_type) {
			hard_err++;
			err_status = i214er_hard[i].er_num;
			if(ISTAPE(dd, spindle))
				index = i214er_hard[i].er_tmsg;
			else
				index = i214er_hard[i].er_bmsg;
			break;
		}
	}

	/*
	 * If we didn't find the error yet, keep looking.
	 */
	if(!hard_err) {
		for(i = 0; i < ER_SOFT_MAX; i++) {
			if(dd->d_error.e_soft & i214er_soft[i].er_type) {
				err_status = i214er_soft[i].er_num;
				if(ISTAPE(dd, spindle))
					index = i214er_soft[i].er_tmsg;
				else
					index = i214er_soft[i].er_bmsg;
				break;
			}
		}
	}

	dd->d_error.e_hard = 0;
	dd->d_error.e_soft = 0;
	if(err_status && index) {
		cmn_err(CE_CONT, "iSBC %s controller %d drive %d  (%s)\n",
			i214b_type[dd->d_state.s_support],
			dd->d_state.s_board, spindle, i214er_msg[index]);
		/*
		 * Now print out the cylinder, head and sector info
		 * for block devices.
		 */
		if(!ISTAPE(dd, spindle)) {
			struct i214drtab *dr = &dd->d_drtab[spindle];

			i = ((unsigned) dd->d_error.e_req_cyl_h << 8) & 0xff00;
			i |= (unsigned) dd->d_error.e_req_cyl_l & 0x00ff;
			cmn_err(CE_CONT, "     cylinder    %u\n", i);
			cmn_err(CE_CONT, "     head        %d\n", (dd->d_error.e_req_head & 0x0f));
			cmn_err(CE_CONT, "     sector      %d\n", dd->d_error.e_req_sec);
			if (dr->dr_nfhead) {
				/* On wini, give absolute sector # */
				daddr_t absno;
				ulong cylsecs = (long)(dr->dr_nfhead) * dr->dr_nsec;
				absno = (long)(i) * cylsecs +
					(long)(dd->d_error.e_req_head & 0x0f) *
					      dr->dr_nsec + dd->d_error.e_req_sec;
				cmn_err(CE_CONT, "*** Absolute sector number: %d\n",absno);
			}
		/*
		 * As the code below is somewhat non-intuitive, an explanation
		 * is in order.  Bits 4 and 5 of the actual cylinder high byte
		 * indicate the sector size found.  Values are:  00 = 128 bytes;
		 * 01 = 256 bytes; 10 = 512 bytes; 11 = 1024 bytes.  We shift
		 * and mask to get these bits into the low bit positions, where
		 * they will end up as a value from 0 to 3.  This value is used
		 * to shift left (multiply by 2^value) the number i, which
		 * starts out as 128.
		 *
		 * Example:  actual count returns a bit pattern of xx10xxxx;
		 * shift gives 0000xx10; & gives 00000010 (or 2); i is 128,
		 * so i << 2 = 512.
		 *
		 * Note:  If a shift count of zero causes problems, then make
		 * i = 64 instead and add one to the shift count.  (i is 64;
		 * so i << (2 + 1) = 512.)
		 */
			i = 128;
			cmn_err(CE_CONT, "Media: %u Byte/Sector ",
				(i << ((dd->d_error.e_act_cyl_h >> 4) & 0x3)));

			/* print out indicated track type */
			switch (dd->d_error.e_act_cyl_h & 0xc0) {
			case 0:
				cmn_err(CE_CONT, "Data Track\n");
				break;
			default:
				cmn_err(CE_CONT, "Non-Data Track\n");
				break;
			}
		}
		if (dd->d_state.s_bufh == NULL)
			cmn_err(CE_CONT, "%s Error\n", (hard_err ? "Hard" : "Soft"));
		else
			cmn_err(CE_CONT, "%s Error: %d Retries\n", (hard_err ? "Hard" : "Soft"),
				(dd->d_state.s_bufh)->b_errcnt);

	}

	if ((err_status == (char)EBBHARD) || (err_status == (char)EBBSOFT))
		err_status = EIO;

	return(err_status);
}


/*
 *	i214rawswrite:	i214 Raw Sector Write
 *
 *	Writes data to the specified device beginning with the specified
 *	absolute sector #.  One attempt is made to write 'count' bytes from
 *	the specified buffer.  The number of bytes actually write are
 *	returned to the caller.
 */
static int
i214rawswrite(dev, sec, count, bp)
dev_t		dev;		/* Disk device to write to */
daddr_t		sec;		/* Absolute sector number to begin writing */
unsigned int	count;		/* # of bytes to write */
struct buf	*bp;		/* Buffer to hold the data */
{
	int		x;	/* Current interrupt level */
	struct i214dev		*dd;	/* Pntr to the device's dev structure */
	struct i214drtab	*dr;	/* Pntr to dev's dr_tab data struct */
	unsigned int		totsec;	/* Total # of sec of physical device */

	/*
	 * Find the proper dev structure.
	 */
	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];
	totsec= dr->dr_ncyl * dr->dr_spc;

	/*
	 * Make sure the device is ready to be used.
	 */
	if ((dd->d_state.s_flags[UNIT(dev)] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error= EBUSY;
		BIODONE(bp);
		return(0);
	}

	/*
	 * Make sure we're not starting beyond the end of the physical disk.
	 */
	if (sec >= totsec) {
		bp->b_flags |= B_ERROR;
		bp->b_error= ENXIO;
		BIODONE(bp);
		return(0);
	}

	/*
	 * Make sure we don't write past the end of the physical media.
	 */
	count= min(count, ((totsec - sec) * dr->dr_secsiz));

	bp->b_flags &= ~B_READ;
	bp->b_flags &= ~(B_DONE | B_ERROR);
	bp->b_error= 0;
	bp->b_blkno= sec * dd->d_drtab[UNIT(dev)].dr_lbps;
	bp->b_sector= sec;
	bp->b_edev= BASEDEV(dev);
	bp->b_proc= 0x00;
	bp->b_bcount= count;
	bp->b_resid= 0;

	/*
	 * Queue the request and wait for a response.
	 * Note - We don't use the strategy routine
	 * because we may want to write outside of the
	 * partition boundries.
	 */
	x= SPL();
	i214disksort(dd->d_state.s_bufh, bp);
	i214start(dd);
	splx(x);
	(void) biowait(bp);

	/*
	 * Check for errors.
	 */
	if (bp->b_flags & B_ERROR) {
		return(0);
	}

	return(count - bp->b_resid);
}



/*
 *	i214rawbwrite:	i214 Raw Byte Write
 *
 *	Write data to the specified device beginning with the specified
 *	absolute byte offset (from the beginning of the disk).  Data is
 *	repeatly written to the device until 'count' bytes have been written
 *	or until an error occures.  Optionally, the callers own buffer 'bp'
 *	can be used for writing.  If 'bp' has a value of '0' then a temporary
 *	buffer will be allocated.  The number of bytes actually written is
 *	returned to the user.
 */
static int
i214rawbwrite(dev, start, count, srcaddr)
dev_t		dev;		/* Device (maj/min) to write to */
unsigned long	start;		/* Absolute byte offset to begin writing */
unsigned long	count;		/* Number of bytes to write */
char		*srcaddr; 	/* Source address of the data to write */
{
	struct i214dev	*dd;		/* Ptr to the device's dev structure */
	char		*src;		/* Source addr of next data byte */
	char		*dest;		/* Destination addr of next data byte */
	struct buf *bp;
	daddr_t		sec;		/* Sector # of next byte to write */
	unsigned int	secsize;	/* # of bytes in a disk sector */

	unsigned int	readlen;	/* # of bytes actually read from dsk */
	unsigned int	writelen;	/* # of bytes actually written to dsk */
	unsigned int	offset;		/* Offset into sec to begin writing */
	unsigned int	copylen;	/* # of bytes to copy on this loop */
	ushort		i;		/* Loop counter */

	/*
	 * Setup the pointers and counters.
	 */
	dd= i214bdd[ BOARD(dev) ];
	secsize= dd->d_drtab[UNIT(dev)].dr_secsiz;

	bp= geteblk();
	if (bp == (struct buf *) 0) {
		i214print(dev, "Unable to get temporary buffer");
		return(0);
	}

	/*
	 * While there is still more data to be written, keep
	 * writing sectors to disk.
	 *
	 * Note - 'secsize' should probably be used for the read/write
	 *	granularity but we're not sure if non-BSIZE sizes are
	 *	readable and writable.
	 */
	src= srcaddr;
	while (count != 0) {
		sec= start / secsize;
		offset= start % secsize;
		readlen= i214rawsread(dev, sec, BSIZE, bp);
		if (readlen != BSIZE) {
			break;
		}
		dest= (char *) (bp->b_un.b_addr + offset);
		copylen= min(readlen-offset, count);
		for (i=0; i < copylen; i++)
			*dest++= *src++;
		writelen= i214rawswrite(dev, sec, BSIZE, bp);
		if (writelen != BSIZE) {
			src -= copylen;
			break;
		}
		start += copylen;
		count -= copylen;
	}

	brelse(bp);
	return(src-srcaddr);
}


/*
 * Write the Intel Volume Label (IVLAB) to disk.
 */
static int
i214wrtivlab(dev, ivlab)
dev_t		dev;
struct ivlab	*ivlab;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	ivlabloc;
	unsigned long	ivlablen;
	unsigned int	writecnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	ivlabloc= i214winidata[board][unit].ivlabloc;
	ivlablen= i214winidata[board][unit].ivlablen;

	if (ivlabloc == 0) {
		i214print(dev, "Unable to write IVLAB - Invalid location");
		return(ENXIO);
	}
	writecnt= i214rawbwrite(dev, ivlabloc, ivlablen, (caddr_t)ivlab);
	if (writecnt != ivlablen) {
		i214print(dev, "Unable to write VOLUME LABEL");
		return(EIO);
	}

#ifdef DEBUG
	if(i214messages & (M214OPEN | M214BBH)) {
		struct i214drtab	*dr;
		dr= (struct i214drtab *) &ivlab->v_dspecial[0];
		cmn_err(CE_CONT, "i214open: Volume label written to disk has:\n          ");
		cmn_err(CE_CONT, " %d cylinders, %d heads, and %d sectors/track\n",
			dr->dr_ncyl, dr->dr_nfhead, dr->dr_nsec);
	}
#endif /* DEBUG */

	return(0);
}


/*
 * Read Physical Device Information (PDINFO) from disk
 */
static int
i214rdpdinfo(dev, pdinfo)
dev_t		dev;
struct pdinfo	*pdinfo;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	pdinfoloc;
	unsigned long	pdinfolen;
	unsigned int	readcnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	pdinfoloc= i214winidata[board][unit].pdinfoloc;
	pdinfolen= i214winidata[board][unit].pdinfolen;

	if (pdinfoloc == 0) {
		i214print(dev, "Unable to read PDINFO - Invalid location");
		return(ENXIO);
	}
#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		cmn_err(CE_CONT, "i214open: Read PDINFO at absolute byte %d.\n", pdinfoloc);
	}
#endif /* DEBUG */
	readcnt= i214rawbread(dev, pdinfoloc, pdinfolen, (caddr_t)pdinfo);
	if (readcnt != pdinfolen) {
		i214print(dev, "Unable to read PDINFO - Disk I/O error");
		return(EIO);
	}

#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		cmn_err(CE_CONT, "Got %lx as pdinfo sanity word\n", pdinfo->sanity);
	}
#endif	/* DEBUG */

	if (pdinfo->sanity != VALID_PD) {
		i214print(dev, "Invalid PDINFO found");
		return(ENXIO);
	}
	return(0);
}



/*
 * Write Physical Device Information (PDINFO) to disk.
 */
static int
i214wrtpdinfo(dev, pdinfo)
dev_t		dev;
struct pdinfo	*pdinfo;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	pdinfoloc;
	unsigned long	pdinfolen;
	unsigned int	writecnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	pdinfoloc= i214winidata[board][unit].pdinfoloc;
	pdinfolen= i214winidata[board][unit].pdinfolen;

	if (pdinfo->sanity != VALID_PD) {
		i214print(dev, "Unable to write PDINFO - Invalid SANITY.");
		return(ENXIO);
	}
	if (pdinfoloc == 0) {
		i214print(dev, "Unable to write PDINFO - Invalid location.");
		return(ENXIO);
	}
#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		cmn_err(CE_CONT, "PDINFO written to location %x:\n", pdinfoloc);
	}
#endif /* DEBUG */
	writecnt= i214rawbwrite(dev, pdinfoloc, pdinfolen, (caddr_t)pdinfo);
	if (writecnt != pdinfolen) {
		i214print(dev, "Unable to write PDINFO - Disk I/O error");
		return(EIO);
	}
	return(0);
}


/*
 * Read Volume Table Of Contents (VTOC) from disk.
 */
static int
i214rdvtoc(dev, vtoc)
dev_t		dev;
struct vtoc	*vtoc;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	vtocloc;
	unsigned long	vtoclen;
	unsigned int	readcnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	vtocloc= i214winidata[board][unit].vtocloc;
	vtoclen= i214winidata[board][unit].vtoclen;

	if (vtocloc == 0) {
		i214print(dev, "Unable to read VTOC - Invalid location");
		return(ENXIO);
	}
	readcnt= i214rawbread(dev, vtocloc, vtoclen, (caddr_t)vtoc);
	if (readcnt != vtoclen) {
		i214print(dev, "Unable to read VTOC - Disk I/O error");
		return(EIO);
	}

#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH))
		cmn_err(CE_CONT, "*** Partitions found: %d\n", vtoc->v_nparts);
#endif /* DEBUG */

	if (vtoc->v_sanity != VTOC_SANE) {
		i214print(dev, "Invalid VTOC found");
		return(ENXIO);
	}
	return(0);
}



/*
 * Write Volume Table of Contents (VTOC) to disk.
 */
static int
i214wrtvtoc(dev, vtoc)
dev_t		dev;
struct vtoc	*vtoc;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	vtocloc;
	unsigned long	vtoclen;
	unsigned int	writecnt;

	board = BOARD(dev);
	unit = UNIT(dev);
	vtocloc= i214winidata[board][unit].vtocloc;
	vtoclen= i214winidata[board][unit].vtoclen;

	if (vtoc->v_sanity != VTOC_SANE) {
		i214print(dev, "Unable to write VTOC - Invalid SANITY");
		return(ENXIO);
	}
	if (vtocloc == 0) {
		i214print(dev, "Unable to write VTOC - Ivalid location");
		return(ENXIO);
	}
	writecnt= i214rawbwrite(dev, vtocloc, vtoclen, (caddr_t)vtoc);
	if (writecnt != vtoclen) {
		i214print(dev, "Unable to write VTOC - Disk I/O error");
		return(EIO);
	}

#ifdef DEBUG
	cmn_err(CE_CONT, "VTOC written to location %x\n", vtocloc);
#endif /* DEBUG */

	return(0);
}


/*
 * Read Software Alternate Information (ALT INFO) from disk.
 */
static int
i214rdaltinfo(dev, altinfo)
dev_t		dev;
struct alt_info	*altinfo;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	altinfoloc;
	unsigned long	altinfolen;
	unsigned int	readcnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	altinfoloc= i214winidata[board][unit].altinfoloc;
	altinfolen= i214winidata[board][unit].altinfolen;

	if (altinfoloc == 0) {
		i214print(dev, "Unable to read ALT INFO - Invalid location");
		return(ENXIO);
	}
	if (altinfolen != sizeof(*altinfo)) {
		i214print(dev, "Inconsistent ALT INFO size");
		return(ENXIO);
	}
	readcnt= i214rawbread(dev, altinfoloc, altinfolen, (caddr_t)altinfo);
	if (readcnt != altinfolen) {
		i214print(dev, "Unable to read ALT INFO - Disk I/O error");
		return(EIO);
	}
	if (altinfo->alt_sanity != ALT_SANITY) {
		i214print(dev, "Invalid ALT INFO found");
		return(ENXIO);
	}

#ifdef DEBUG
	if (i214messages & (M214OPEN | M214BBH)) {
		cmn_err(CE_CONT, "** SW Alt secs used: %d, SW Alt sec present: %d **\n",
			altinfo->alt_sec.alt_used, altinfo->alt_sec.alt_reserved);
		cmn_err(CE_CONT, "** SW Alt trks used: %d, SW Alt trk present: %d **\n",
			altinfo->alt_trk.alt_used, altinfo->alt_trk.alt_reserved);
	}
#endif /* DEBUG */

	return(0);
}



/*
 * Write Software Alternate Information (ALT INFO) to disk.
 */
static int
i214wrtaltinfo(dev, altinfo)
dev_t		dev;
struct alt_info	*altinfo;
{
	unsigned int	board;
	unsigned int	unit;
	unsigned long	altinfoloc;
	unsigned long	altinfolen;
	unsigned int	writecnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	altinfoloc= i214winidata[board][unit].altinfoloc;
	altinfolen= i214winidata[board][unit].altinfolen;

	if (altinfo->alt_sanity != ALT_SANITY) {
		i214print(dev, "Unable to write ALT INFO - Invalid SANITY");
		return(ENXIO);
	}
	if (altinfoloc == 0) {
		i214print(dev, "Unable to write ALT INFO - Invalid location");
		return(ENXIO);
	}
	writecnt= i214rawbwrite(dev, altinfoloc, altinfolen, (caddr_t)altinfo);
	if (writecnt != altinfolen) {
		i214print(dev, "Unable to write ALT INFO - Disk I/O error");
		return(EIO);
	}

#ifdef DEBUG
	if (i214messages & M214BBH) {
		cmn_err(CE_CONT, "ALT INFO written to location: %x:\n", altinfoloc);
	}
#endif /* DEBUG */

	return(0);
}


/*
 * Calculate the disk location (absolute byte address) of the
 * 1st copy of the Manufacturer's Defect List corresponding to
 * the current sector size.
 *
 *
 * The ST506 info is stored in the 2nd cyl of the MDL partition.
 * This info is written on 4 different surfaces (heads) using a
 * different sector size for each surface as follows:
 *
 *	128 bytes/sec written on last surface
 *	256 bytes/sec written on last surface - 1
 *	512 bytes/sec written on last surface - 2
 *	1024 bytes/sec written on last surface - 3
 *
 * Each surface contains 4 copies of the defect info written with
 * the appropriate bytes/sector configuration.  Each copy is written
 * on every other 1K boundry starting at the beginning of the track.
 *
 * For example, the 128 bytes/sec defect info is written using the
 * last head in sectors 0, 16, 32, and 48.
 */
static unsigned int
i214ST506loc(dev, mdlstart)
dev_t		dev;
daddr_t		mdlstart;
{
	unsigned int		board;
	unsigned int		unit;
	struct i214drtab	*dr;
	struct i214dev		*dd;

	unsigned int		trkskips;
	unsigned int		mdlloc;

	board= BOARD(dev);
	unit= UNIT(dev);
	dd= i214bdd[board];
	dr= &dd->d_drtab[unit];

	/*
	 * Caluculate the # of tracks to skip from the beginning of
	 * the MDL partitions in order to get proper track.
	 *
	 * Skip the 1st cylinder of the MDL partition since the ST506
	 * defect lists are contained in the 2nd cylinder of the partiton.
	 *
	 * Also, skip to first few tracks of the 2nd cylinder according
	 * to the current sector size.
	 */
	trkskips= dr->dr_nfhead;
	switch (dr->dr_secsiz) {
		case 1024:	trkskips += dr->dr_nfhead - 4;
				break;
		case 512:	trkskips += dr->dr_nfhead - 3;
				break;
		case 256:	trkskips += dr->dr_nfhead - 2;
				break;
		case 128:	trkskips += dr->dr_nfhead - 1;
				break;
		default:
			cmn_err(CE_CONT, "Unable to calculate MDL location - ");
			i214print(dev, "Invalid sector size");
			return(0);
		}

	/*
	 * Convert location of the 1st copy to an absolute byte address.
	 */
	mdlloc= (mdlstart + (trkskips*dr->dr_nsec)) * dr->dr_secsiz;
	return(mdlloc);
}

static unsigned int
i214ESDIloc(dev, mdlstart)
dev_t		dev;
daddr_t		mdlstart;
{
	unsigned int		board;
	unsigned int		unit;
	struct i214drtab	*dr;
	struct i214dev		*dd;
	unsigned int		mdlloc;

	board= BOARD(dev);
	unit= UNIT(dev);
	dd= i214bdd[board];
	dr= &dd->d_drtab[unit];

	/*
	 * Convert location of the 1st copy to an absolute byte address.
	 */
	mdlloc= mdlstart *  dr->dr_secsiz;
	return(mdlloc);
}


/*
 * Read Manufacturer's Defect List (MDL) from disk.
 */
static int
i214rdmdl(dev, mdl, mdltype)
dev_t		dev;
union	esdi506mdl	*mdl;
unsigned int	mdltype;
{
	unsigned int	board;
	unsigned int	unit;
	struct i214drtab	*dr;
	struct i214dev		*dd;
	unsigned int	st506mdlloc;
	unsigned int	st506mdllen;
	unsigned int	esdimdlloc;
	unsigned int	esdimdllen;
	unsigned int	readcnt;

	unsigned int	copysz;
	unsigned short	copycnt;
	unsigned short	surface;

	board= BOARD(dev);
	unit= UNIT(dev);
	dd= i214bdd[board];
	dr= &dd->d_drtab[unit];
	st506mdlloc= i214winidata[board][unit].st506mdlloc; 
	st506mdllen= i214winidata[board][unit].st506mdllen;
	esdimdlloc=  i214winidata[board][unit].esdimdlloc; 
	esdimdllen=  i214winidata[board][unit].esdimdllen;

	if ((esdimdlloc == 0) || (st506mdlloc == 0)) {
		cmn_err(CE_CONT, "Unable to read MFG's DEFECT LIST - ");
		i214print(dev, "Invalid location");
		return(ENXIO);
	}

	if (mdltype == I_ESDI) {
		copysz= BBHESDICOPYSZ;
		for (surface=0; surface < dr->dr_nfhead; surface++) {
			for (copycnt=0; copycnt < BBHESDICOPYCNT; copycnt++, 
													esdimdlloc += copysz) {
				readcnt= i214rawbread( dev, esdimdlloc, esdimdllen, 
												&(mdl->esdimdl[surface]), 0 );
				if (readcnt != esdimdllen) {
					/*
					 * Don't put these messages as application will always try
					 * to read ESDI mdl first before trying ST506 mdl. And this 
					 * generates unnecessary mesages.
					 * dri_printf("Unable to read ESDI MFG's DEFECT LIST (copy %d)",
					 *		copycnt );
					 * i214print( dev, " - Disk I/O error" );
					 */
					continue;
				}
				if (mdl->esdimdl[surface].header.magic != BBHESDIMDLVALID) {
					/*
					 * Don't put these messages as application will always try
					 * to read ESDI mdl first before trying ST506 mdl. And this 
					 * generates unnecessary mesages.
					 * dri_printf( "Invalid MFG's Defect List (copy %d)",copycnt);
					 * i214print( dev, " found" ); 
					 */
					continue;
				}
				break;
			}

			if (copycnt >= BBHESDICOPYCNT) {
				return (EIO)	;
			}
	
		/* Calculate esdimdlloc for next surface */
		esdimdlloc= i214winidata[board][unit].esdimdlloc  + 
							(surface +1) * dr->dr_nsec * dr->dr_secsiz;

		}
	}

	if (mdltype == I_ST506) {
		copysz= BBH506COPYSZ;
		for (copycnt=0; copycnt < BBH506COPYCNT; copycnt++, 
												st506mdlloc += copysz) {
			readcnt= i214rawbread( dev, st506mdlloc, st506mdllen, 
													&(mdl->st506mdl), 0 );
			if (readcnt != st506mdllen) {
				dri_printf("Unable to read ST506 MFG's DEFECT LIST (copy %d)",
					copycnt );
				i214print( dev, " - Disk I/O error" );
				continue;
			}
			if (mdl->st506mdl.header.bb_valid != BBH506MDLVALID) {
				dri_printf( "Invalid MFG's Defect List (copy %d)",copycnt);
				i214print( dev, " found" ); 
				continue;
			}
	
			break;
		}

		if (copycnt >= BBH506COPYCNT) {
			return(STATBAD);
		}
	}

	return(0);
}



/*
 * Write Manufacturer's Defect List (MDL) to disk.
 */
static int
i214wrtmdl(dev, mdl)
dev_t		dev;
union	esdi506mdl	*mdl;
{
	unsigned int	board;
	unsigned int	unit;
	struct i214drtab	*dr;
	struct i214dev		*dd;
	unsigned int	st506mdlloc;
	unsigned int	st506mdllen;
	unsigned int	esdimdlloc;
	unsigned int	esdimdllen;
	unsigned int	writecnt;

	unsigned int	copysz;
	unsigned short	copycnt;
	unsigned short	surface;
	unsigned short	errcnt;

	board= BOARD(dev);
	unit= UNIT(dev);
	dd= i214bdd[board];
	dr= &dd->d_drtab[unit];
	st506mdlloc= i214winidata[board][unit].st506mdlloc;
	st506mdllen= i214winidata[board][unit].st506mdllen;
	esdimdlloc = i214winidata[board][unit].esdimdlloc;
	esdimdllen = i214winidata[board][unit].esdimdllen;

	if ((mdl->esdimdl[0].header.magic != BBHESDIMDLVALID) && 
	    (mdl->st506mdl.header.bb_valid != BBH506MDLVALID)) {
		cmn_err(CE_CONT, "Unable to write MFG's DEFECT LIST");
		i214print(dev, " - Invalid SANITY");
		return(ENXIO);
	}
	if ((st506mdlloc == 0) || (esdimdlloc == 0))  {
		cmn_err(CE_CONT, "Unable to write MFG's Defect List");
		i214print(dev, " - Invalid location");
		return(ENXIO);
	}
	/*
	 * Write all copies of the Manufacture's Defect List to disk.
	*/

	errcnt= 0;
	
	if (mdl->st506mdl.header.bb_valid == BBH506MDLVALID) {
		copysz= BBH506COPYSZ;
		for (copycnt=0; copycnt < BBH506COPYCNT; copycnt++, 
											st506mdlloc += copysz) {
			writecnt= i214rawbwrite( dev, st506mdlloc, st506mdllen, 
													&(mdl->st506mdl), 0 );
			if (writecnt != st506mdllen) {
				dri_printf("Unable to write ST506 MFG's Defect List (copy %d)"
					, copycnt );
				i214print( dev, " - Disk I/O error" );
				errcnt++;
				continue;
				}

#ifdef DEBUG
if (i214messages & M214BBH) {
	dri_printf( "MFG's DEFECT LIST (copy %d) written to location: %x:\n",
		copycnt, st506mdlloc );
}
#endif /* DEBUG */

		}

	}

	else { /* ESDI mdl */

		copysz = BBHESDICOPYSZ	;
		for (surface=0; surface < dr->dr_nfhead; surface++) {
			for (copycnt=0; copycnt < BBHESDICOPYCNT; copycnt++, 
												esdimdlloc += copysz) {
				writecnt= i214rawbwrite( dev, esdimdlloc, esdimdllen, 
												&(mdl->esdimdl[surface]), 0 );
				if (writecnt != esdimdllen) {
					dri_printf("Unable to write ESDI MFG's Defect List (copy %d)"
						, copycnt );
					i214print( dev, " - Disk I/O error" );
					errcnt++;
					continue;
					}
	
#ifdef DEBUG
if (i214messages & M214BBH) {
	dri_printf( "MFG's DEFECT LIST (copy %d) written to location: %x:\n",
		copycnt, esdimdlloc );
}
#endif /* DEBUG */
			}

			if (errcnt != 0)
				break	;

			/* Calculate esdimdlloc for next surface */
			esdimdlloc= i214winidata[board][unit].esdimdlloc +
								(surface+1) * dr->dr_nsec * dr->dr_secsiz;
		}
	}

	if (errcnt != 0)
		return(EIO);

	return(0);
}


/*
 * Unlock the device's wini data structures.
 */
static void
i214unlockwini(dev, wd, flags)
dev_t			dev;
struct i214winidata	*wd;
unsigned int		flags;
{
	unsigned short	errcnt;
	unsigned int	x;

	if (wd->lock != i214_LOCKED) {
		i214print(dev, "Wini data not locked");
		return;
	}

	/*
	 * Update the disk copies of the each data structure
	 * ONLY if the caller asked for it.
	 */
	errcnt= 0;
	if (flags == STRUCTIO) {
		if (i214wrtivlab(dev, wd->ivlab) != 0)
			errcnt++;
		if (i214wrtpdinfo(dev, wd->pdinfo) != 0)
			errcnt++;
		if (i214wrtvtoc(dev, wd->vtoc) != 0)
			errcnt++;
		if (i214wrtaltinfo(dev, wd->altinfo) != 0)
			errcnt++;
		if (i214wrtmdl(dev, wd->mdl) != 0)
			errcnt++;
	}

	x= SPL();
	wd->ivlab= (struct ivlab *) 0;
	wd->pdinfo= (struct pdinfo *) 0;
	wd->vtoc= (struct vtoc *) 0;
	wd->altinfo= (struct alt_info *) 0;
	wd->mdl= (union esdi506mdl *) 0;

	kmem_free(wd->pgaddr, ptob(wd->pgcnt));
	wd->pgcnt= 0;
	wd->pgaddr= 0;

	wd->lock = ~i214_LOCKED;
	splx(x);

	if (errcnt != 0) {
		i214print(dev, "Unable to update wini data structures");
		return;
	}
}



/*
 * Locks the device's wini data structures, allocates memory to hold
 * temporary copies of each wini data structure, and reads
 * in a current copy of each structure.
 */
static int
i214lockwini(dev, wd, flags, mdltype)
dev_t			dev;
struct i214winidata	*wd;
unsigned int		flags;
unsigned int		mdltype;
{
	unsigned int	bytecnt;
	caddr_t			byteaddr;
	unsigned int	x;

	/*
	 * Lock wini data structures so format can update structure
	 * if need without worry of a collision.
	 */
	x= SPL();
	if (wd->lock == i214_LOCKED) {
		i214print(dev, "Wini data structures already locked");
		splx(x);
		return(EBUSY);
	}
	wd->lock= i214_LOCKED;

	/*
	 * Allocate enough memory to hold all the wini data structures
	 * for this device.  Then read in a current copy of each structure.
	 */
	bytecnt= sizeof(struct ivlab) + sizeof(struct pdinfo) +
		sizeof(struct vtoc) + sizeof(struct alt_info) +
		sizeof(union esdi506mdl);

	wd->pgcnt= bytecnt / ptob(1) + ((bytecnt % ptob(1)) == 0 ? 0 : 1);
	wd->pgaddr= kmem_alloc(ptob(wd->pgcnt), KM_SLEEP);

	byteaddr= wd->pgaddr;
	wd->ivlab= (struct ivlab *) byteaddr;
	byteaddr += sizeof(struct ivlab);

	wd->pdinfo= (struct pdinfo *) byteaddr;
	byteaddr += sizeof(struct pdinfo);

	wd->vtoc= (struct vtoc *) byteaddr;
	byteaddr += sizeof(struct vtoc);

	wd->altinfo= (struct alt_info *) byteaddr;
	byteaddr += sizeof(struct alt_info);

	wd->mdl= (union esdi506mdl *) byteaddr;
	byteaddr += sizeof(union esdi506mdl);

	/*
	 * Read in a current copy of each data structure from
	 * the disk ONLY if the caller asked for it.
	 */
	if (flags == STRUCTIO) {
		if ((i214rdivlab(dev, wd->ivlab) != 0) ||
			(i214rdpdinfo(dev, wd->pdinfo) != 0) ||
			(i214rdvtoc(dev, wd->vtoc) != 0) ||
			(i214rdaltinfo(dev, wd->altinfo) != 0)	||
			(i214rdmdl(dev, wd->mdl, mdltype) != 0)	) {

			/*
			 * If all structures are not successfully read,
			 * then unlock everything.
			 */
			i214print(dev,
				"Unable to read all wini data structures");
			i214unlockwini(dev, wd, ~STRUCTIO);
			return(EIO);
		}
	}
	splx(x);

	return(0);
}



/*
 * Send a format request to the controller
 */
static int
i214fmtsend(dev)
dev_t dev;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;
	struct iobuf		*bufh;

	unsigned int	unit;
	unsigned int	x;
	unsigned short	rtncode;

	unit= UNIT(dev);
	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[unit];
	bufh= dd->d_state.s_bufh;

	/*
	 * Calc absolute sector # of 1st sector in track.
	 */
	dd->d_format.f_secno= (daddr_t) (dd->d_ftk.f_track * dr->dr_nsec);

#ifdef DEBUG
	if (i214messages & M214FMT) {
		cmn_err(CE_CONT, "i214fmtsend: format (%x) (%x) (%x) (%c%c%c%c)\n",
			dd->d_ftk.f_track, dd->d_ftk.f_type,
			dd->d_ftk.f_intl,
			dd->d_ftk.f_pat[0],dd->d_ftk.f_pat[1],
			dd->d_ftk.f_pat[2],dd->d_ftk.f_pat[3]);
	}
#endif /* DEBUG */

	x= SPL();

	/*
	 * Wait for a chance to get at the device and then go for it.
	 */
	while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
		bufh->b_active |= IO_WAIT;
		(void) sleep((caddr_t) &dd->d_state.s_state, PRIBIO+1);
	}

	/*
	 * Send format request to the contoller.
	 * Note: We do NOT do retries.
	 */
	i214io(dd, (struct buf *) NULL, FORMAT_OP, (int) unit);

	/*
	 * Wait for the response.
	 */
	while (dd->d_state.t_flags & TF_NO_BUFFER)
		(void) sleep((caddr_t) dd, PRIBIO+1);

	/*
	 * Check format status.
	 */
	if (dd->d_state.s_error[unit] & ST_HARD_ERR) {

#ifdef DEBUG
		if (i214messages & M214FMT) {
			cmn_err(CE_CONT, "FMT ERROR: trk %d, type %d\n",
				dd->d_ftk.f_track, dd->d_ftk.f_type);
		}
#endif /* DEBUG */

		rtncode = i214checkerr(dd, unit);
		if ((rtncode == (char) EBBHARD) ||
		   (rtncode == (char) EBBSOFT)) {
			rtncode = EIO;
		}
	} else {
		rtncode= 0;
	}
	i214start(dd);
	splx(x);
	return(rtncode);
}


/*
 * Scan the MDL for any defects in a specific track.
 */
static int
i214trkok(dev, mdl, trk)
dev_t		dev;
union esdi506mdl *mdl;
ushort		trk;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;
	struct st506defect	*dfct;
	struct esdidefect	*esdidfct;

	unsigned short		cyl;
	unsigned char		head;
	unsigned short		cnt;
	unsigned short		surface;

	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];

	/*
	 * Make sure current MDL is valid.
	 */
	if ((mdl->esdimdl[0].header.magic != BBHESDIMDLVALID) && 
	    (mdl->st506mdl.header.bb_valid != BBH506MDLVALID) ) {
		return(ENXIO);
	}

	/*
	 * Check MDL to see if any entires match specified track.
	 */
	cyl= trk / dr->dr_nfhead;
	head= trk % dr->dr_nfhead;

	if (mdl->st506mdl.header.bb_valid == BBH506MDLVALID) {
		dfct= &(mdl->st506mdl.defects[0]);
		for (cnt=1; cnt <= mdl->st506mdl.header.bb_num; cnt++, dfct++) {
			if ((dfct->be_cyl == cyl) && (dfct->be_surface == head)) {
					return(EIO);
			}
		}
	}
	else { /* It is Esdi MDL */
		esdidfct = &(mdl->esdimdl[head].defects[0])	;
		for (cnt=0; cnt < BBHESDIMAXDFCTS; cnt++, esdidfct++) {
			/*
			 * End of defects for this head is signified by 0xFFFF
			 * as the cylinder number.
			 */
			if (MERGE_NUM(esdidfct->hi_cyl, esdidfct->lo_cyl) == 0xFFFF) {
				break	;
			}

			if (MERGE_NUM(esdidfct->hi_cyl, esdidfct->lo_cyl) == cyl) {
#ifdef DEBUG
	if (i214messages & M214FMT) {
		dri_printf( "MDL MATCH - Offset (%d) = cyl %d, head %d",
			cnt-1, cyl, head); 
	}
#endif /* DEBUG */
				return (STATBAD)	;
			}
		}
	}

	return(0);
}



/*
 * Format the specified track as a normal data track.
 */
static int
i214fmtnorm(dev, mdl, trk, intlv)
dev_t		dev;
union esdi506mdl	*mdl;
ushort		trk;			/* Absolute track # */
ushort		intlv;
{
	struct i214dev	*dd;

	dd= i214bdd[BOARD(dev)];

	/*
	 * Make sure MDL is valid.
	 */
	if ((mdl->esdimdl[0].header.magic != BBHESDIMDLVALID) && 
	    (mdl->st506mdl.header.bb_valid != BBH506MDLVALID)) {
		return(ENXIO);
	}

	/*
	 * Scan MDL for any defects in this track.
	 */
	if (i214trkok(dev, mdl, trk) != 0) {
		return(EIO);
	}

	/*
	 * Everything looks ok.  Try to format as a normal track.
	 */
	dd->d_ftk.f_track= trk;
	dd->d_ftk.f_intl= intlv;
	dd->d_ftk.f_skew= 0;
	dd->d_ftk.f_type= FORMAT_DATA;
	dd->d_ftk.f_pat[0]= 0xE5;
	dd->d_ftk.f_pat[1]= 0xE5;
	dd->d_ftk.f_pat[2]= 0xE5;
	dd->d_ftk.f_pat[3]= 0xE5;

	return(i214fmtsend(dev));
}


/*
 * Format a defective track using HW Alternate Tracking
 */
static int
i214fmtalttrk(dev, pdinfo, mdl, trk, intlv)
dev_t		dev;
struct pdinfo	*pdinfo;
union  esdi506mdl *mdl;
ushort		trk;
ushort		intlv;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;

	unsigned int	altsec;
	ushort		alttrk;
	ushort		althead;
	ushort		altcyl;

	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];

	/*
	 * Make sure PDINFO data structure is valid.
	 */
	if (pdinfo->sanity != VALID_PD) {
		cmn_err(CE_CONT, "Unable to format using alternate tracks");
		i214print(dev, " - Invalid PDINFO");
		return(ENXIO);
	}

	/*
	 * Make sure MDL is valid.
	 */
	if ((mdl->esdimdl[0].header.magic != BBHESDIMDLVALID) && 
	    (mdl->st506mdl.header.bb_valid != BBH506MDLVALID)) {
		cmn_err(CE_CONT, "Unable to format track using normal method");
		i214print(dev, " - Invalid MDL");
		return(ENXIO);
	}

	/*
	 * Search the unallocated tracks of the alternate track
	 * partition until a track is found that can be successfully
	 * formatted as an alternate.
	 */
	for (;;) {
		/*
		 * Make sure the next alternate track actually starts
		 * on a track boundry.
		 */
		altsec= pdinfo->relnext;
		if ((altsec % dr->dr_nsec) != 0) {
			altsec= altsec - (altsec % dr->dr_nsec) + dr->dr_nsec;
		}

		/*
		 * Make sure the entire track is contained withing
		 * the alternate track partition.
		 */
		if ((altsec + dr->dr_nsec - 1) >
				    (pdinfo->relst + pdinfo->relsz - 1)) {
			i214print(dev, "Out of HW ALTERNATE TRACKS");
			return(ENOSPC);
		}

		/*
		 * Allocate the track and make sure that it does
		 * not contain a defect, according to the MDL.
		 */
		pdinfo->relnext= altsec + dr->dr_nsec;
		alttrk= altsec / dr->dr_nsec;

		if (i214trkok(dev, mdl, alttrk) != 0) {
			continue;
		}

#ifdef DEBUG
		if (i214messages & M214FMT) {
			cmn_err(CE_CONT, "Fmt HW Alt Trk %d for bad trk %d\n", alttrk, trk);
		}
#endif /* DEBUG */

		/*
		 * Format the alternate track as a surrogate.
		 */
		dd->d_ftk.f_track= alttrk;
		dd->d_ftk.f_intl= intlv;
		dd->d_ftk.f_skew= 0;
		dd->d_ftk.f_type= FORMAT_ALTERNATE;
		dd->d_ftk.f_pat[0]= 0xE5;
		dd->d_ftk.f_pat[1]= 0xE5;
		dd->d_ftk.f_pat[2]= 0xE5;
		dd->d_ftk.f_pat[3]= 0xE5;

		if (i214fmtsend(dev) == 0)
			break;
	}

	/*
	 * WARNING - A usable alternate track now been allocated.
	 * If, for some reason, it is not actually used then
	 * it needs to be properly deallocated.
	 */

#ifdef DEBUG
	if (i214messages & M214FMT) {
		cmn_err(CE_CONT, "Fmt trk %d as BAD\n", trk);
	}
#endif /* DEBUG */

	/*
	 * Now that a usable alternate track has been located
	 * and formatted, format the defective track.
	 */
	altcyl= alttrk / dr->dr_nfhead;
	althead= alttrk % dr->dr_nfhead;

	dd->d_ftk.f_track= trk;
	dd->d_ftk.f_intl= intlv;
	dd->d_ftk.f_skew= 0;
	dd->d_ftk.f_type= FORMAT_BAD;
	dd->d_ftk.f_pat[0]= altcyl & 0xFF;
	dd->d_ftk.f_pat[1]= (altcyl >> 8) & 0xFF;
	dd->d_ftk.f_pat[2]= althead;
	dd->d_ftk.f_pat[3]= 0xB5;

	/*
	 * If the defective track fails to format, then the
	 * assigned alternate is not actually used so it must
	 * be deallocated.  Otherwise, it is wasted.
	 */
	if (i214fmtsend(dev) != 0) {
		pdinfo->relst -= dr->dr_nsec;
		return(EIO);
	}

	return(0);
}


/*
 * Format a defective track using SW Alternates
 */
static int
i214fmtswalt(dev, altinfo, trk)
dev_t		dev;
struct alt_info	*altinfo;
ushort		trk;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;

	unsigned int 		secnum;
	unsigned short		altinx;
	unsigned short		cnt;

	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];

	/*
	 * Make sure the alternate information is valid.
	 */
	if (altinfo->alt_sanity != ALT_SANITY) {
		i214print(dev, "Invalid SW ALT INFO");
		return(ENXIO);
	}

	/*
	 * Make sure there are enough alternate sectors left to
	 * remap the entire track.
	 */
	if ((unsigned)(altinfo->alt_sec.alt_used + dr->dr_nsec)
					> altinfo->alt_sec.alt_reserved) {
		i214print(dev, "Not enough SW ALT SECTORS to remap the trk");
		return(ENOSPC);
	}

	/*
	 * Remap each of the sectors in the track.
	 */
	secnum= trk * dr->dr_nsec;
	altinx= altinfo->alt_sec.alt_used;

	for (cnt= 1; cnt <= dr->dr_nsec; cnt++) {
		altinfo->alt_sec.alt_bad[altinx++]= secnum++;
		altinfo->alt_sec.alt_used++;
	}

#ifdef DEBUG
	if (i214messages & M214FMT) {
		cmn_err(CE_CONT, "Fmt trk %d using SW ALTS SECS %d - %d\n",
			trk, altinfo->alt_sec.alt_base + altinx - dr->dr_nsec,
			altinfo->alt_sec.alt_base + altinx - 1);
	}
#endif /* DEBUG */

	return(0);
}


/*
 * Format a single track using one of the specified methods.
 */
static int
i214fmttrk(dev, wd, trk, intlv, method)
dev_t			dev;
struct i214winidata	*wd;
unsigned int		trk;
unsigned short		intlv;
unsigned short		method;
{
	ushort	cnt;
	ushort	fmtstat;
	ushort	methodlist[FMTMETHODS];

	/*
	 * Make sure the wini data structures are locked.
	 */
	if (wd->lock != i214_LOCKED) {
		cmn_err(CE_CONT, "Unable to format partition");
		i214print(dev, " - wini data not locked");
		return(ENXIO);
	}

	/*
	 * Decide which order to try the various formatting methods.
	 */
	if (method == FMTANY) {
		methodlist[0]= FMTNORM;
		methodlist[1]= FMTALTTRK;
		methodlist[2]= FMTSWALTS;
		methodlist[3]= FMTFAIL;
	} else {
		methodlist[0]= method;
		methodlist[1]= FMTFAIL;
	}

	for (cnt=0; methodlist[cnt] != FMTFAIL; cnt++) {
		switch (methodlist[cnt]) {
		case FMTNORM:
			fmtstat= i214fmtnorm(dev, wd->mdl, trk, intlv);
			break;
		case FMTALTTRK:
			fmtstat= i214fmtalttrk(dev, wd->pdinfo, wd->mdl,
								trk, intlv);
			break;
		case FMTSWALTS:
			fmtstat= i214fmtswalt(dev, wd->altinfo, trk);
			break;
		default:
			fmtstat= EINVAL;

		}
		if (fmtstat == 0)
			break;
	}

	if (fmtstat != 0) {
		i214print(dev, "Unable to format track");
		return(fmtstat);
	}
	return(0);
}




/*****************************************************************************
 *
 * 	Open a unit.  Sets a given partition (i.e. special file) open.
 *
 *	If this function is opening the first partition on a physical
 *	device, it calls i214sweep to configure the device.  Ideally,
 *	i214binit would configure all devices once and for all, but
 *	this is impossible because users can insert different density
 *	floppy disks into the same drive.  Therefore, configuration
 *	is done here.  To further complicate things, according to current
 *	lore, the FIRST unit initialized on a given controller must be
 *	winchester 0.  This, however, is handled by i214sweep.
 *
 *	Open, close, ioctl and strategy must not be concurrent.  bufh->b_active
 *	and a sleep loop accomplish this (see i214README).  Open(s) must
 *	lock each other out in case the second open gets through and tries to
 *	do I/O before sweep finishes.
 *
 *	To support VTOC, when opening winchesters, the following steps are
 *	done:
 *	1) The drive is initialized as a 1 cylinder, 2 track device.
 *	2) The Volume label (see ivlab.h) is read from it.
 *	3) The drive is re-initialized with the information in the
 *	   volume label.
 *	4) The pdinfo (see vtoc.h) is read from the device.  The pdinfo
 *	   is found using a pointer in the volume label.
 *	5) The VTOC is located by a pointer in the pdinfo and read in.
 *	6) The Alternate info (see alttbl.h) is also located from
 *	   pdinfo, read in, and the drivers internal alternate information is
 *	   initialized.
 *
 ****************************************************************************/
/*ARGSUSED*/
int
i214open(devp, flag, otyp, cred_p)
dev_t		*devp;
int			flag;            /* not used */
int			otyp;            /* not used */
struct cred	*cred_p;
{
	register struct i214dev	*dd;
	struct i214drtab	*dr;
	struct i214drtab	*ivlab_dr;
	struct i214cdrt 	*cdr;

	struct i214winidata	*wd;
	struct ivlab		*ivlab;
	struct pdinfo		*pdinfo;
	struct vtoc		*vtoc;
	struct alt_info		*altinfo;

	unsigned int		board;
	unsigned int 		unit;
	unsigned int 		part;
	unsigned int 		x;
	int					errcod = 0;
	struct buf     		*scrbuf1 = (struct buf *) NULL;
	unsigned int		i214isitesdi();
	extern		dev_t	rootdev;
	extern		dev_t	pipedev;
	extern		dev_t	swapdev;
	extern		int		nswap;

#ifdef DEBUG
	ushort  msgsave=i214messages;
	if (i214messages & M214OPEN) {
		cmn_err(CE_CONT, "i214open: dev=%x\n",*devp);
		i214messages = 0xfeff;
	}
#endif /* DEBUG */
	if (getminor(*devp) > i214maxmin)
		return(ENXIO);
	board= BOARD(*devp);
	if (board >= i214_cnt)
		return(ENXIO);
	dd= i214bdd[board];
	if (!dd->d_state.s_exists) {
		return(ENXIO);
	}
	unit= UNIT(*devp);
	if (unit >= FIRSTTAPE) {
		return(ENXIO);
	}
	part= PARTITION(*devp);
	dr= &dd->d_drtab[unit];
	cdr= &i214cfg[board].c_drtab[unit][DRTAB(*devp)];
	wd= &i214winidata[board][unit];
	x = SPL();
	/*
	 * Ensure that no one else tries to open a device at the same time.
	 */
	while (dd->d_state.s_flags[unit] & SF_OPENING) {
		(void) sleep (&dd->d_state.s_flags[unit], PZERO);
	}
	dd->d_state.s_flags[unit] |= SF_OPENING;

	/*
	 * If the device is already open and the partition is valid
	 * then there is nothing more to do.
	 */
	if (dd->d_state.s_flags[unit] & SF_OPEN) {
		if (!(dr->dr_part[part].p_flag & V_VALID))
			goto badopen;  /* invalid partition */
		dr->dr_part[part].p_flag |= V_OPEN;
		goto unitopen;
	}

	/*
	 * For floppy drives, set up hi/lo density device type
	 */
	if (unit >= FIRSTFLOPPY && unit < FIRSTTAPE)
		dd->d_state.s_devcod[unit] =
			((DRTAB(*devp) == FLPY_HD) ? DEV8FLPY : DEV5FLPY);

	/*
	 * The device has not been opened yet, so configure the drive
	 * to an initial state so the IVLAB can be read.
	 */
	if (i214configbrd(*devp, cdr) != 0) {
		goto badopen;
	}
	dd->d_state.s_flags[unit] = SF_OPEN | SF_READY;

	/*
	 * If unit is not a wini, then we're done.
	 */
	if (unit >= FIRSTFLOPPY)
		goto unitopen;

	scrbuf1= geteblk();
	scrbuf1->b_flags |= (B_STALE | B_AGE);

	/*
	 * TEMPORARY CODE: Until 'mkpart' is fixed to use the
	 * 'FMTLOCK' and 'FMTUNLOCK' ioctl calls.
	 *
	 * Lock the device's wini data structures
	 *
	 */
	if (wd->lock != i214_LOCKED) {
		if (i214lockwini(*devp, wd, ~STRUCTIO) != 0) {
			i214print(*devp, "Unable to lock wini structures");
			goto badopen;
		}
	}
	/*
	 * End temporary code.
	 */


	/*
	 * Read and validate the Intel Volume Label.
	 */
	wd->ivlabloc= VLAB_SECT * dr->dr_secsiz + VLAB_START;
	wd->ivlablen= sizeof(struct ivlab);

	ivlab= (struct ivlab *) scrbuf1->b_un.b_addr;
	if (i214rdivlab(*devp, ivlab) != 0) {
		goto badopen;
	}

	/*
	 * Ivlab contains the 1st part of a DRTAB.
	 */
	ivlab_dr= (struct i214drtab *) &ivlab->v_dspecial[0];
	cdr->cdr_ncyl= ivlab_dr->dr_ncyl;
	cdr->cdr_nfhead= ivlab_dr->dr_nfhead;
	cdr->cdr_nsec= ivlab_dr->dr_nsec;

	if (i214configbrd(*devp, cdr) != 0) {
		goto badopen;
	}

	/*
	 * Read and validate PDINFO.
	 */
	wd->pdinfoloc= VTOC_SEC * dr->dr_secsiz;
	wd->pdinfolen= sizeof(struct pdinfo);

	pdinfo= (struct pdinfo *) scrbuf1->b_un.b_addr;
	if (i214rdpdinfo(*devp, pdinfo) != 0) {
		goto badopen;
	}
	if ((pdinfo->cyls != dr->dr_ncyl) ||
	    (pdinfo->tracks != dr->dr_nfhead) ||
	    (pdinfo->sectors != dr->dr_nsec)) {
		cmn_err(CE_WARN,
			"PDINFO DOES NOT MATCH LABEL ON DEVICE %x\n",*devp);
		goto badopen;
	}
	wd->vtocloc= pdinfo->vtoc_ptr;
	wd->vtoclen= pdinfo->vtoc_len;
	wd->altinfoloc= pdinfo->alt_ptr;
	wd->altinfolen= pdinfo->alt_len;
	wd->st506mdlloc= i214ST506loc( *devp, pdinfo->mfgst );
	wd->st506mdllen= sizeof(struct st506mdl);
	wd->esdimdlloc= i214ESDIloc( *devp, pdinfo->mfgst );
	wd->esdimdllen= sizeof(struct esdiheadmdl);

	/*
	 * Figure out if I am dealing with ESDI drive.
	 * Set the proper flag in winidata structure.
	 */
	wd->isesdi = ( i214isitesdi(*devp, dr->dr_ncyl - 1)  ||
			       i214isitesdi(*devp, dr->dr_ncyl) )  ;

	/*
	 * Read and validate VTOC.  If it's a valid VTOC then
	 * set the driver's internal partition table with the
	 * new partiton info.
	 */
	vtoc= (struct vtoc *) scrbuf1->b_un.b_addr;
	if (i214rdvtoc(*devp, vtoc) != 0) {
		goto badopen;
	}
	if(i214setparts(*devp, &vtoc->v_part[0], vtoc->v_nparts) != 0) {
		goto badopen;
	}

	/*
	 * Read and validate Alternate Info.
	 */
	altinfo= &i214_alts[4*board+unit];
	if (i214rdaltinfo(*devp, altinfo) != 0) {
		goto badopen;
	}

	/*
	 * Device successfully opened.
	 */
	dd->d_state.s_flags[unit] |= SF_VTOC_OK;
	if (!(dr->dr_part[part].p_flag & V_VALID))
		goto badopen;

	/*
	 * If opening the root device, see if swap device is on the same
	 * unit.  If so, get the partition size for swapdev and set
	 * nswap accordingly.
	 */
	if((*devp == rootdev) && (swapdev != NODEV) &&
	   (board == BOARD(swapdev)) && (unit == UNIT(swapdev))) {
		nswap = dr->dr_part[PARTITION(swapdev)].p_nsec * dr->dr_lbps;
	}
	dr->dr_part[part].p_flag |= V_OPEN;

	/*
	 * The following KLUDGE cause partition 0 to be marked open.
	 * This nullifies the errornious calls to close by preventing
	 * close from actually shutting down the unit.
	 */
	dr->dr_part[0].p_flag |= V_OPEN;

	goto unitopen;

badopen:
	/*
	 * If this is partition 0, then ignore the error so
	 * that a totally hosed disk can be formatted.
	 */
#ifdef DEBUG
	if(i214messages & M214OPEN)
		cmn_err(CE_CONT, "at *badopen* for dev %x\n",*devp);
#endif
	if (part == 0)
		errcod = 0;
	else
		errcod = ENXIO;

unitopen:
	/*
	 * The device is now open.  Wake up any processes waiting to
	 * do an open.  Also, release the temporary buffer.
	 */
#ifdef DEBUG
	if(i214messages & M214OPEN) {
		cmn_err(CE_CONT, "at *unitopen* for dev %x\n",*devp);
		i214messages = msgsave;
	}
#endif
	dd->d_state.s_flags[unit] &= ~SF_OPENING;
	wakeup(&dd->d_state.s_flags[unit]);
	if (scrbuf1 != 0)
		brelse(scrbuf1);
	splx(x);
	return(errcod);
}



/*******************************************************************************
 *
 * 	Close a unit.
 *
 *	Called on last close of a partition; thus, "close" the
 *	partition.  If this was last partition, mark the unit
 *	closed and not-ready.  In this case, next open will
 *	re-initialize.
 *
 ******************************************************************************/
/*ARGSUSED*/
int
i214close(dev, flag, otyp, cred_p)
register dev_t	dev;		/* major, minor numbers */
int				flag;		/* not used */
int				otyp;		/* not used */
struct cred		*cred_p;
{
	register struct i214dev	*dd = i214bdd[BOARD(dev)];
	unsigned	unit = UNIT(dev);
	unsigned	s;
	extern		dev_t	rootdev;
	extern		dev_t	pipedev;
	extern		dev_t	swapdev;
	int             openflg = 0;
	int             i;
	int				errcod = 0;

#ifdef DEBUG
	if (i214messages & M214OPEN)
		cmn_err(CE_CONT, "i214close -- dev %x\n",dev);
#endif /* DEBUG */
	if ((dev == rootdev) || (dev == swapdev) || (dev == pipedev)) {
		return(errcod);		/* never close these */
	}

	/*
	 * Clear the bit that said the partition was open.
	 * If last close of drive insure drtab queue is
	 * empty before returning.
	 */
	/*
	 * Given that it is quite possible to configure a floppy
	 * for multiple partitions, this next "if" is probably an error.
	 */
	if (unit < FIRSTFLOPPY) { /* only reset partitions on hard disk */
	    dd->d_drtab[unit].dr_part[PARTITION(dev)].p_flag &= ~V_OPEN;
	    for (i=0; i<dd->d_drtab[unit].dr_pnum; i++) {
			if (dd->d_drtab[unit].dr_part[i].p_flag & V_OPEN) {
#ifdef DEBUG
				if (i214messages & M214OPEN)
					cmn_err(CE_CONT, "i214close -- other partitions open\n");
#endif /* DEBUG */
				openflg++;
				break;
			}
		}
	}
	if(!openflg) {
		struct iobuf		*bufh;

		s = SPL();
		bufh= dd->d_state.s_bufh;
		while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
			(dd->d_state.s_bufh)->b_active |= IO_WAIT;
			(void) sleep((caddr_t)&dd->d_state.s_state, PRIBIO);
		}
		splx(s);
		dd->d_state.s_flags[unit] = 0;
		/*
		 * The following code is required to work-around a
		 * bug in the UNIX kernel that allows the driver's
		 * close routine to be called even though the partition
		 * is still opened by other processes.
		 */
		/* Start Kludge: */
		dd->d_state.s_flags[unit] |= SF_READY;
		/* End Kludge */

	}
	return(errcod);
}



/*
 * Format the opened partition using the specified method(s).
 */
static int
i214fmtpart(dev, intlv, method)
dev_t	dev;
ushort	intlv;
ushort	method;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;
	struct i214part		*part;
	struct i214winidata	*wd;

	unsigned short 	trk;
	unsigned short	begtrk;
	unsigned short	endtrk;

	dd= i214bdd[BOARD(dev)];
	dr= &dd->d_drtab[UNIT(dev)];
	part= &dr->dr_part[PARTITION(dev)];
	wd= &i214winidata[BOARD(dev)][UNIT(dev)];

	/*
	 * Since the controller can only format at a track granularity,
	 * make sure the partition starts and ends on a track boundry.
	 */
	if (((part->p_fsec % dr->dr_nsec) != 0) ||
	   ((part->p_nsec % dr->dr_nsec) != 0)) {
		cmn_err(CE_CONT, "Unable to format partition");
		i214print(dev, " - Not defined on track boundry");
		return(ENXIO);
	}

	/*
	 * Calculate the 1st and last track within the partition.
	 */
	begtrk= part->p_fsec / dr->dr_nsec;
	endtrk= begtrk + (part->p_nsec / dr->dr_nsec) - 1;

#ifdef DEBUG
	if (i214messages & M214FMT) {
		cmn_err(CE_CONT, "FMT Partition: trk %d - %d\n", begtrk, endtrk);
	}
#endif /* DEBUG */

	/*
	 * Format the partition track by track until either 1) all the tracks
	 * are formatted or 2) one of the tracks can not be formatted.
	 */
	for (trk= begtrk; trk <= endtrk; trk++) {
		if (i214fmttrk(dev, wd, trk, intlv, method) != 0) {
			i214print(dev, "Unable to format partition");
			return(EIO);
		}
	}
	return(0);
}

/******************************************************************************
 * Sends a request to the controller to read vendor defect list. If it is
 * successful in reading it, it has to be ESDI drive.
 *****************************************************************************/

static unsigned int
i214isitesdi (dev, cylnum)
dev_t	dev	;
unsigned short	cylnum	;
{
		struct	 iobuf	*bufh;
		struct	 i214dev *dd;
		unsigned int	x	;
		unsigned int	unit;
		unsigned int	board;
		unsigned int	bytecnt;
		unsigned int	pgcnt;
		unsigned int	pgaddr;

		board = BOARD(dev)	;
		unit = UNIT(dev)	;
		dd = i214bdd[board]	;
		bufh = dd->d_state.s_bufh;

		x = SPL()	;

		/*
		 * Wait for a chance to get at the device and then go for it.
		 */

		while (bufh->b_active != IO_IDLE) {
			bufh->b_active |= IO_WAIT;
			sleep( (caddr_t) &dd->d_state.s_state, PRIBIO+1);
		}
	
		/*
		 * Send 'Read Vendor List' request to the contoller.
		 * Note: We do NOT do retries. 
		 */

		bytecnt= BBHVENDDFCTSZ	;
		pgcnt = bytecnt / ptob(1) + ((bytecnt % ptob(1)) == 0 ? 0 : 1);
	    pgaddr= (unsigned int)kmem_alloc( ptob(pgcnt), KM_SLEEP);

		dd->d_iopb.i_addr		= pgaddr;
		dd->d_state.s_state		= RVENDLIST				;
		dd->d_iopb.i_cylinder	= cylnum				;
		dd->d_iopb.i_head 		= dd->d_drtab[unit].dr_nfhead - 1;
		dd->d_iopb.i_sector 	= 0;
		dd->d_iopb.i_xfrcnt		= BBHVENDDFCTSZ;

		i214io( dd, NULL, READ_VENDLIST_OP, (int) unit );
	
		/*
		 * Wait for the response.
		 */
		while (dd->d_state.t_flags & TF_NO_BUFFER) {
			sleep( (caddr_t) dd, PRIBIO+1);
		}
	
		/* 
		 * We tried to read Vendor defect list. This command is 
		 * supported only on ESDI drives. So if we get a hard error,
		 * we will assume that we have found a ST506 drive. This may 
		 * not be true, if it is ESDI drive and the track containing 
		 * Vendor defect list is corrupted. Though this is highly 
		 * unlikely, we may end up in that situation. The consequences
		 * are that user will not be able to specify more than 255
		 * defects for the drive. Too bad !!!
		 */
		if (dd->d_state.s_error[unit] & ST_HARD_ERR) {
			kmem_free((caddr_t)pgaddr, ptob(pgcnt));
			splx (x);
			return (0)								;
		}
		else { /* It is ESDI drive */
			kmem_free((caddr_t)pgaddr, ptob(pgcnt));
			splx (x);
			return (1)								;
		}
	
}

/*****************************************************************************
 *
 * 		214 driver special functions.
 *
 *      V_FORMAT:  Formats a single data track on a disk.
 *              This code has only the intelligence to format
 *		the track; no testing, etc of the track is done.  If
 *		the unit specified is a tape, will execute exactly the
 *		same as if the I214_ERASE ioctl had been executed.
 *
 ****************************************************************************/
/*ARGSUSED*/
int
i214ioctl (dev, cmd, cmdarg, flag, cred_p, rval_p)
dev_t	dev;        /* major, minor numbers */
int		cmd;        /* command code */
caddr_t	cmdarg;     /* user structure with parameters */
int		flag;		/* not used */
struct	cred *cred_p;
int		*rval_p;
{
	struct i214dev		*dd;
	struct i214drtab	*dr;
	struct i214drtab	*ivlab_dr;
	struct i214part		*pt;
	struct i214cdrt		*cdr;
	struct iobuf		*bufh;
	struct i214winidata	*wd;

	unsigned int	board;
	unsigned int	unit;
	unsigned int	part;
	unsigned int	x;
	unsigned int	i;
	int				errcod = 0;

	struct disk_parms	i214dp;
	union io_arg		varg;
	struct absio		absio;
	struct fmtpart		fmtpart;
	union	esdi506mdl	*mdl;

	struct buf	*tmpbuf;
	struct ivlab	*ivlab;
	struct pdinfo	*pdinfo;
	struct vtoc	*vtoc;
	struct alt_info	*altinfo;

	unsigned int	pgcnt;
	unsigned int	bytecnt;
	caddr_t pgaddr;
	unsigned int		i214isitesdi();

#ifdef DEBUG
	ushort msgsave;
#endif /* DEBUG */

	board= BOARD(dev);
	unit= UNIT(dev);
	part= PARTITION(dev);
	dd= i214bdd[board];
	dr= &dd->d_drtab[unit];
	pt= &dr->dr_part[part];
	cdr= &i214cfg[board].c_drtab[unit][DRTAB(dev)];
	wd= &i214winidata[board][unit];
	bufh= dd->d_state.s_bufh;

	if (!(dd->d_state.s_flags[unit] & SF_READY))
		return(ENXIO);

	switch (cmd) {
	case V_CONFIG:
		/*
		 * Reconfigure the board according to the specified parameters.
		 */
#ifdef DEBUG
		if(i214messages & M214IOC)
			cmn_err(CE_CONT, "IOCTL V_CONFIG on dev %x\n",dev);
#endif /* DEBUG */

		/*
		 * Make sure the user is the 'super user' and that
		 * the device is a wini.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		/*
		 * Get the new disk parameters and reconfigure the controller.
		 */
		(void) copyin(cmdarg, (caddr_t) &varg, sizeof(union io_arg));

		x=SPL();
		/*
		 * Make sure no partitions are open except for partition 0.
		 */
		for (i=1; i < dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				if (part == 0)
					errcod = EBUSY;
				else
					errcod = ENODEV;
				return(errcod);
			}
		}
		cdr->cdr_ncyl= varg.ia_cd.ncyl;
		cdr->cdr_nfhead= varg.ia_cd.nhead;
		cdr->cdr_nsec= varg.ia_cd.nsec;
		i214configbrd(dev, cdr);
		splx(x);
		break;

	case V_REMOUNT:
		/*
		 * Force the next call to open to re-read all of the
		 * devices data strucuters.
		 */

#ifdef DEBUG
		if(i214messages & M214IOC)
		    cmn_err(CE_CONT, "IOCTL V_REMOUNT on dev %x\n",dev);
#endif /* DEBUG */

		/*
		 * Make sure the user is the 'super user' and that
		 * the device is a wini.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}

		x = SPL();
		/*
		 * Make sure no other partitions are open except partition 0.
		 */
		for (i=1; i < dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				if (part == 0)
					errcod = EBUSY;
				else
					errcod = ENODEV;
				return(errcod);
			}
		}

		/*
		 * Wait for the controller to be come idle, set clear
		 * the state flags to force a read during the next open,
		 * then restart normal I/O.
		 */
		while ((bufh->b_actf != NULL) || (bufh->b_active & IO_BUSY)) {
			bufh->b_active |= IO_WAIT;
			(void) sleep((caddr_t) &dd->d_state.s_state, PRIBIO+1);
		}
		dd->d_state.s_flags[unit]= 0;
		i214start(dd);
		splx(x);
		break;

	case V_ADDBAD:
		(void) copyin(cmdarg, (caddr_t) &varg, sizeof(union io_arg));
		return(EINVAL);	/* Not supported */

	case V_FORMAT:
		/*
		 * Format one track of the device.
		 *
		 * If formatting a wini, make sure caller is 'root'.
		 */
		if (ISWINI(dev) && drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}

		/*
		 * Verify the format parameters and setup the
		 * format argument structure.
		 *
		 * Note - Track # is relative to the start of the partition.
		 */
		(void) copyin(cmdarg, (caddr_t) &varg, sizeof(union io_arg));
		if (varg.ia_fmt.num_trks != 1) {
			errcod = EINVAL;
			break;
		}

		x= SPL();
		/*
		 * Set up format information.
		 */
		dd->d_ftk.f_track= varg.ia_fmt.start_trk;
		dd->d_ftk.f_intl= varg.ia_fmt.intlv;
		dd->d_ftk.f_skew= 0;
		dd->d_ftk.f_type= FORMAT_DATA;
		dd->d_ftk.f_pat[0]= 0xE5;
		dd->d_ftk.f_pat[1]= 0xE5;
		dd->d_ftk.f_pat[2]= 0xE5;
		dd->d_ftk.f_pat[3]= 0xE5;

		/*
		 * Make sure the track is within the partition and
		 * convert it to an absolue sector #.
		 */
		dd->d_format.f_secno=
			(daddr_t) (dd->d_ftk.f_track * dr->dr_nsec);
		if (dd->d_format.f_secno >= pt->p_nsec) {
			splx(x);
			errcod = ENXIO;
			break;
		}
		dd->d_format.f_secno += pt->p_fsec;

		/*
		 * Convert track # to absolute so that 'fmtsend' can be used.
		 */
		dd->d_ftk.f_track= dd->d_format.f_secno / dr->dr_nsec;

		i214fmtsend(dev);
		splx(x);
		break;

	case V_GETPARMS:
		/*
		 * Send the device parameters to the user.
		 */

#ifdef DEBUG
		if(i214messages & M214IOC)
		    cmn_err(CE_CONT, "IOCTL V_GETPARMS on dev %x\n",dev);
#endif /* DEBUG */

		if(ISWINI(dev)) {
			if ( i214isitesdi(dev, dd->d_drtab[unit].dr_ncyl - 1)  ||
			     i214isitesdi(dev, dd->d_drtab[unit].dr_ncyl) )  {
				i214dp.dp_type = DPT_ESDI_HD;
			}
			else {
				i214dp.dp_type = DPT_WINI;
			}
			i214dp.dp_heads = dr->dr_nfhead;

		} else if (ISFLOP(dev)) {
			i214dp.dp_type = DPT_FLOPPY;
			i214dp.dp_heads = dr->dr_nrhead;

		} else { /* Must be a tape */
			i214dp.dp_type = DPT_NOTDISK;
			i214dp.dp_heads = 0;
		}

		i214dp.dp_cyls= dr->dr_ncyl;
		i214dp.dp_sectors= dr->dr_nsec;
		i214dp.dp_secsiz= ((ushort) dr->dr_hsecsiz << 8) |
						   (ushort)dr->dr_lsecsiz;
		i214dp.dp_ptag= pt->p_tag;
		i214dp.dp_pflag= pt->p_flag;
		i214dp.dp_pstartsec= pt->p_fsec;
		i214dp.dp_pnumsec= pt->p_nsec;

#ifdef DEBUG
		if(i214messages & M214IOC) {
			cmn_err(CE_CONT,
				"About to return type %x ncyl %x heads %x s/trk %x\n",
				i214dp.dp_type,i214dp.dp_cyls,i214dp.dp_heads,
				i214dp.dp_sectors);
			cmn_err(CE_CONT, "secsiz %x tag %x flag %x fstsec %x numsec %x\n",
				i214dp.dp_secsiz,i214dp.dp_ptag,i214dp.dp_pflag,
				i214dp.dp_pstartsec,i214dp.dp_pnumsec);
		}
#endif /* DEBUG */

		/*
		 * Copy parameters to user segment.
		 */
		(void) copyout((caddr_t) &i214dp, cmdarg, sizeof(struct disk_parms));
		break;

	case V_FMTPART:
		/*
		 * Format the current partition.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		(void) copyin(cmdarg, (caddr_t) &fmtpart, sizeof(struct fmtpart));
		errcod = i214fmtpart(dev, fmtpart.intlv, fmtpart.method);
		break;

	case V_L_VLAB:
		/*
		 * Load IVLAB from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/*
		 * Get and validate the new IVLAB before overlaying the
		 * existing one.
		 */
		tmpbuf= geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		ivlab= (struct ivlab *) tmpbuf->b_un.b_addr;

		(void) copyin(cmdarg, (caddr_t) ivlab, sizeof(struct ivlab));

		x= SPL();
		/*
		 * Reconfigure the board and update the driver's internal
		 * variables with the new IVLAB information.  But first,
		 * make sure no partitions are open, except for partition 0.
		 */
		for (i=1; i < dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				brelse(tmpbuf);
				if (part == 0)
					errcod = EBUSY;
				else
					errcod = ENODEV;
				return(errcod);
			}
		}
		ivlab_dr= (struct i214drtab *) &ivlab->v_dspecial[0];
		cdr->cdr_ncyl= ivlab_dr->dr_ncyl;
		cdr->cdr_nfhead= ivlab_dr->dr_nfhead;
		cdr->cdr_nsec= ivlab_dr->dr_nsec;
		if (i214configbrd(dev, cdr) != 0) {
			splx(x);
			brelse(tmpbuf);
			break;
		}

		*(wd->ivlab)= *ivlab;
		wd->ivlabloc= VLAB_SECT * dr->dr_secsiz + VLAB_START;
		wd->ivlablen= sizeof(struct ivlab);
		splx(x);

		brelse(tmpbuf);
		break;

	case V_U_VLAB:
		/*
		 * Upload IVLAB to the user.
		 *
		 * But first make sure: the device in a wini, the
		 * device's wini data structures are properly locked
		 * and the current ivlab is valid.
		 */
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		(void) copyout((caddr_t) wd->ivlab, cmdarg, sizeof(struct ivlab));
		break;

	case V_R_VLAB:
		/*
		 * Read IVLAB from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214rdivlab(dev, wd->ivlab);
		splx(x);
		break;

	case V_W_VLAB:
		/*
		 * Write IVLAB to disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214wrtivlab(dev, wd->ivlab);
		splx(x);
		break;

	case V_L_PDIN:
		/*
		 * Load PDINFO from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/*
		 * Get and validate the new PDINFO before overlaying
		 * the existing one.
		 */
		tmpbuf= geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		pdinfo= (struct pdinfo *) tmpbuf->b_un.b_addr;

		(void) copyin(cmdarg, (caddr_t) pdinfo, sizeof(struct pdinfo));
		if ((pdinfo->sanity != VALID_PD) ||
		   (pdinfo->cyls != dr->dr_ncyl) ||
		   (pdinfo->tracks != dr->dr_nfhead) ||
		   (pdinfo->sectors != dr->dr_nsec)) {
			i214print(dev, "PDINFO does not match Volume Label");
			errcod = EINVAL;
			brelse(tmpbuf);
			break;
		}
		x=SPL();
		*(wd->pdinfo)= *pdinfo;
		wd->pdinfoloc= VTOC_SEC * dr->dr_secsiz;
		wd->pdinfolen= sizeof(struct pdinfo);
		wd->vtocloc= pdinfo->vtoc_ptr;
		wd->vtoclen= pdinfo->vtoc_len;
		wd->altinfoloc= pdinfo->alt_ptr;
		wd->altinfolen= pdinfo->alt_len;
		wd->st506mdlloc= i214ST506loc( dev, pdinfo->mfgst );
		wd->st506mdllen= sizeof(struct st506mdl);
		wd->esdimdlloc= i214ESDIloc( dev, pdinfo->mfgst );
		wd->esdimdllen= sizeof(struct esdiheadmdl);
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_PDIN:
		/*
		 * Upload PDINFO to the user.
		 *
		 * But first make sure: the device in a wini, and the
		 * wini data structures are properly locked and the
		 * the current PDINFO is valid.
		 */
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		if ((wd->pdinfo)->sanity != VALID_PD) {
			errcod = EIO;
			break;
		}
		(void) copyout((caddr_t) wd->pdinfo, cmdarg, sizeof(struct pdinfo));
		break;

	case V_R_PDIN:
		/*
		 * Read PDINFO from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214rdpdinfo(dev, wd->pdinfo);
		splx(x);
		break;

	case V_W_PDIN:
		/*
		 * Write PDINFO to disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214wrtpdinfo(dev, wd->pdinfo);
		splx(x);
		break;

	case V_L_VTOC:
		/*
		 * Load VTOC from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/*
		 * Get and validate new vtoc before overlaying the
		 * existing one.
		 */
		tmpbuf= geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		vtoc= (struct vtoc *) tmpbuf->b_un.b_addr;

		(void) copyin(cmdarg, (caddr_t) vtoc, sizeof(struct vtoc));
		if (vtoc->v_sanity != VTOC_SANE) {
			errcod = EINVAL;
			brelse(tmpbuf);
			break;
		}
		x=SPL();
		/*
		 * Update the driver's internal partition table with the
		 * partition information in the new VTOC.  But first,
		 * make sure no partitions are open, except for partition 0.
		 * Also, after the partition table has been updated, make
		 * sure that the current partiton is left open.
		 */
		for (i=1; i < dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				brelse(tmpbuf);
				if (part == 0)
					errcod = EBUSY;
				else
					errcod = ENODEV;
				return(errcod);
			}
		}
		*(wd->vtoc)= *vtoc;
		i214setparts(dev, &vtoc->v_part[0], vtoc->v_nparts);
		dr->dr_part[part].p_flag |= V_OPEN;
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_VTOC:
		/*
		 * Upload VTOC to the user.
		 *
		 * But first, make sure the device in a wini, and
		 * the wini data structures are properly locked.
		 */
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		if ((wd->vtoc)->v_sanity != VTOC_SANE) {
			errcod = EIO;
			break;
		}
		(void) copyout((caddr_t) wd->vtoc, cmdarg, sizeof(struct vtoc));
		break;

	case V_R_VTOC:
		/*
		 * Read VTOC from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214rdvtoc(dev, wd->vtoc);
		splx(x);
		break;

	case V_W_VTOC:
		/*
		 * Format the opened partition.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214wrtvtoc(dev, wd->vtoc);
		splx(x);
		break;

	case V_L_SWALT:
		/*
		 * Load SW ALTINFO from user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/*
		 * Get and validate the new SW ALTINFO before overlaying
		 * the existing one.
		 */
		altinfo= (struct alt_info *) kmem_alloc(ptob(1), KM_SLEEP);
		(void) copyin(cmdarg, (caddr_t) altinfo, sizeof(struct alt_info));
		if (altinfo->alt_sanity != ALT_SANITY) {
			errcod = EINVAL;
			kmem_free((caddr_t)altinfo, ptob(1));
			break;
		}
		x=SPL();
		*(wd->altinfo)= *altinfo;
		i214_alts[4*board+unit]= *altinfo;
		splx(x);
		kmem_free((caddr_t)altinfo, ptob(1));
		break;

	case V_U_SWALT:
		/*
		 * Upload SW ALTINFO to user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		if ((wd->altinfo)->alt_sanity != ALT_SANITY) {
			errcod = EIO;
			break;
		}
		(void) copyout((caddr_t)wd->altinfo, cmdarg, sizeof(struct alt_info));
		break;

	case V_R_SWALT:
		/*
		 * Read SW ALTINFO from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214rdaltinfo(dev, wd->altinfo);
		splx(x);
		break;

	case V_W_SWALT:
		/*
		 * Write SW ALTINFO to disk.
		 *
		 * But first, make sure the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214wrtaltinfo(dev, wd->altinfo);
		splx(x);
		break;

	case V_L_MDL:
		/*
		 * Load MDL from user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/*
		 * Get and validate the new MDL before overlaying
		 * the existing one.
		 */
		
		copyin( cmdarg, (caddr_t) &i, sizeof(short))	;
		i &= 0xffff;
		if ((i != BBHESDIMDLVALID) && (i != BBH506MDLVALID)) {
			errcod = EINVAL;
			break;
		}

		if (i == BBHESDIMDLVALID) {
			bytecnt= sizeof(struct esdiheadmdl[BBHESDIMAXHEADS]);
		}
		else if (i == BBH506MDLVALID) {
			bytecnt= sizeof (struct st506mdl)	;
		}
		
		
		pgcnt= bytecnt / ptob(1) + ((bytecnt % ptob(1)) == 0 ? 0 : 1);
		pgaddr= kmem_alloc(ptob(pgcnt), KM_SLEEP);

		mdl= (union esdi506mdl *) pgaddr;

		(void) copyin( cmdarg, (caddr_t) mdl, bytecnt)	;

		x=SPL();
		if (i == BBHESDIMDLVALID) {
			*(wd->mdl)= *mdl;
		}
		else if (i == BBH506MDLVALID) {
			*((struct st506mdl *)(wd->mdl))= *(struct st506mdl *)mdl;
		}
		else {
			errcod = EINVAL;
		}

		splx(x);
		kmem_free((caddr_t)pgaddr, ptob(pgcnt));
		break;

	case V_U_MDL:
		/*
		 * Upload MDL to the user.
		 *
		 * But first make sure: the device in a wini, and
		 * the wini data structures are properly locked.
		 */
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}

		if 	(wd->mdl->esdimdl[0].header.magic == BBHESDIMDLVALID) {
			(void) copyout( (caddr_t) wd->mdl, cmdarg, 
							sizeof(struct esdiheadmdl[BBHESDIMAXHEADS]) );
		}
		else if (wd->mdl->st506mdl.header.bb_valid == BBH506MDLVALID) {
			(void) copyout( (caddr_t) wd->mdl, cmdarg, sizeof(struct st506mdl) );
		}
		else {
			errcod = EIO;
		}

		break;

	case V_R_MDL:
		/*
		 * Read MDL from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		/* 
		 * Find out if the user wants to read ESDI MDL or ST506 MDL.
		 */
		copyin( cmdarg, (caddr_t) &i, sizeof(int) );
		if ((i != I_ESDI) && (i != I_ST506)) {
			errcod = EINVAL;
			break;
		}
				
		x=SPL();
		errcod = i214rdmdl(dev, wd->mdl, i);
		splx(x);
		break;

	case V_W_MDL:
		/*
		 * Write MDL to disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini, and the wini data structures
		 * are properly locked.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		if (wd->lock != i214_LOCKED) {
			errcod = ENOLCK;
			break;
		}
		x=SPL();
		errcod = i214wrtmdl(dev, wd->mdl);
		splx(x);
		break;

	case V_FMTLOCK:
		/*
		 * Lock the device's wini data strucutures.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}

		if (wd->isesdi)
			(void) i214lockwini(dev, wd, (unsigned int) cmdarg, I_ESDI);
		else
			(void) i214lockwini(dev, wd, (unsigned int) cmdarg, I_ST506);
		break;

	case V_FMTUNLOCK:
		/*
		 * Unlock the device's wini data strucutures.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device in a wini.
		 */
		if (drv_priv(cred_p)) {
			errcod = EPERM;
			break;
		}
		if (!ISWINI(dev)) {
			errcod = ENODEV;
			break;
		}
		i214unlockwini(dev, wd, (unsigned int) cmdarg);
		break;

	case V_RDABS:
		/*
		 * Read a sector from disk at the specified absolute sector #.
		 */
		(void) copyin(cmdarg, (caddr_t) &absio, sizeof(struct absio));
		tmpbuf= geteblk();
		if (i214rawsread(dev, absio.abs_sec, dr->dr_secsiz,
						tmpbuf) == dr->dr_secsiz) {
			(void) copyout(tmpbuf->b_un.b_addr,(caddr_t)absio.abs_buf,dr->dr_secsiz);
		}
		brelse(tmpbuf);
		break;

	case V_WRABS:
		/*
		 * Write a sector to disk at the specified absolute sector #.
		 */
		(void) copyin(cmdarg, (caddr_t) &absio, sizeof(struct absio));
		tmpbuf= geteblk();
		(void) copyin(absio.abs_buf, (caddr_t) tmpbuf->b_un.b_addr,
			dr->dr_secsiz);
		i214rawswrite(dev, absio.abs_sec, dr->dr_secsiz, tmpbuf);
		brelse(tmpbuf);
		break;

	default:
		errcod = EINVAL;	/* bad command */
		break;
	}
	return(errcod);
}

int
i214size(dev)
dev_t dev;
{
	register int nblks;
	struct	i214dev		*dd;
	struct	i214drtab	*dr;
	unsigned			unit;
	unsigned char		devopened;

	dd = &i214dev[BOARD(dev)];
	unit = UNIT(dev);
	dr = &dd->d_drtab[unit];

	devopened = 0;
	nblks = -1;

	if (!(dd->d_state.s_flags[unit] & SF_VTOC_OK))  {
		if (i214open(&dev, 0, 0, (struct cred *)0) != 0) { /* XXX - flags? */
				return(nblks);
		}
		devopened++;
	}

	/*
	 *  Note: the following three conditions apply:
	 * 1. Device is a floppy (does not have vtoc)
	 * 2. Device is a formatted wini (has vtoc)
	 * 3. Device is an unformatted wini(does not have vol. label and vtoc)
	 *    and it is for partition 0.
	 */

	if (ISFLOP(dev)
	|| ((dr->dr_part[PARTITION(dev)].p_flag & V_VALID)
		&& (dd->d_state.s_flags[unit] & SF_VTOC_OK))
	|| (!(dd->d_state.s_flags[unit] & SF_VTOC_OK) && (PARTITION(dev) == 0)))  {
		nblks = i214blksize(dev);
	}
	if (devopened)
		(void) i214close(dev, 0, 0, (struct cred *)0);
	return (nblks);
}

#ifdef lint
main()
{	extern int itpdevflag, itpsize();
	extern void itpinit(), itpprint();
	extern int itpopen(), itpclose(), itpread(), itpwrite(), itpioctl();

	i214intr(0);
	i214init();
	i214print((dev_t)0, "");
	(void) i214open((dev_t *)0, 0, 0, (struct cred *)0);
	(void) i214close((dev_t)0, 0, 0, (struct cred *)0);
	(void) i214read((dev_t)0, (struct uio *)0, (struct cred *)0);
	(void) i214write((dev_t)0, (struct uio *)0, (struct cred *)0);
	(void) i214ioctl((dev_t)0, 0, (caddr_t)0, 0, (struct cred *)0, (int *)0);
	(void) i214size((dev_t)0);

	itpinit();
	itpprint((dev_t)0, "");
	(void) itpopen((dev_t *)0, 0, 0, (struct cred *)0);
	(void) itpclose((dev_t)0, 0, 0, (struct cred *)0);
	(void) itpread((dev_t)0, (struct uio *)0, (struct cred *)0);
	(void) itpwrite((dev_t)0, (struct uio *)0, (struct cred *)0);
	(void) itpioctl((dev_t)0, 0, (caddr_t)0, 0, (struct cred *)0, (int *)0);
	(void) itpsize((dev_t)0);
	return(itpdevflag+i214devflag);
}
#endif

