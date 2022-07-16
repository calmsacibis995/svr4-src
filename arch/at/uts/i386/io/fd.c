/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/fd.c	1.3.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/kmem.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/errno.h"

/** New headers for DDI support **/
#include "sys/open.h"
#include "sys/uio.h"
#include "sys/cred.h"

#include "sys/systm.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/file.h"
#include "sys/ioctl.h"
#include "sys/sema.h"


#include "sys/cmn_err.h"
#include "sys/dma.h"
#include "sys/i8237A.h"
#include "sys/cram.h"
#include "sys/fd.h"				
#include "sys/vtoc.h"

#include "sys/inline.h"
#include "sys/conf.h"

/** New header for DDI support **/
#include "sys/ddi.h"


#ifdef DEBUG
int	fddebug = 0;
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* Double Density Drive Parameters
 *
 *	Capacity unformatted	500Kb
 *	Capacity formatted	368.6Kb
 *	Recording density	5876 bpi
 *	Track density		48 tpi
 *	Cylinders		40
 *	Tracks			80
 *	Encoding method		MFM
 *	Rotational speed	300 rpm
 *	Transfer rate		250 kbps
 *	Latency (average)	100 ms
 *	Access time
 *		Average		81 ms
 *		Track to track	5 ms
 *		Settling time	20 ms
 *	Head load time		50 ms
 *	Motor start time	750 ms
 *
 * Quad Density Drive Parameters
 *
 *	Capacity unformatted	1604 Kb
 *	Capacity formatted	1.2 Mb
 *	Recording density	9646 bpi
 *	Track density		96 tpi
 *	Cylinders		80
 *	Tracks			160
 *	Encoding method		MFM
 *	Rotational speed	360 rpm
 *	Transfer rate		500 kbps
 *	Latency (average)	83 ms
 *	Access time
 *		Average		91 ms
 *		Track to track	3 ms
 *		Settling time	18 ms
 *	Head load time		50 ms
 *	Motor start time	750 ms
 */

/* Types of floppy drive as defined in the CMOS RAM */
#define DRV_NONE  0x00
#define DRV_5D	  0x01 
#define DRV_5H	  0x02
#define DRV_3H	  0x04

/* mask for various types of floppy drives */
#define FD_5D	  0x01
#define FD_5H	  0x02
#define FD_3H	  0x08

/* Track format supported */
#define FMT_5H	  0
#define FMT_5D9	  1
#define FMT_5D8	  2
#define FMT_5D4	  3
#define FMT_5D16  4
#define FMT_5Q	  5
#define FMT_3D	  6
#define FMT_3H	  7
#define FMT_AUTO  8
#define FMT_MAX	  8
#define FMT_UNKNOWN 8

/* Transfer rate encoding */
#define FD500KBPS 0
#define FD300KBPS 1
#define FD250KBPS 2

/* Bytes per sector encoding */
#define FD256BPS 1
#define FD512BPS 2
#define FD1024BPS 3

struct fdcstat
{
	char    fd_etimer;
	char    fd_curdrv;
	char    fd_state;
	char    fd_cstat;
	char    fd_savtrk;
	ushort  fd_blkno;
	ushort  fd_bpart;
	ushort  fd_btot;
	long    fd_addr;
} fdcst;

char    fd_mtimer[NUMDRV];
ushort  fd_curmotors;
ushort  fdt_running;

#define DOOR_OPEN   0
#define DOOR_RST    1
#define CHK_RECAL   2
#define RECAL_DONE  3
#define CHK_SEEK    4
#define SEEK_DONE   5
#define XFER_BEG    6
#define XFER_DONE   7
#define	INTR_WAIT   8

struct f0 {
	unsigned char fd_st0;
	unsigned char fd_st1;
	unsigned char fd_st2;
	unsigned char fd_strack;
	unsigned char fd_shead;
	unsigned char fd_ssector;
	unsigned char fd_ssecsiz;
} fdstatus;

struct f1 {
	unsigned char fd_sst0;
	unsigned char fd_scyl;
} fdsnst;

struct fdcmdseq {
	unsigned char fd_cmdb1;
	unsigned char fd_cmdb2;
	unsigned char fd_track;
	unsigned char fd_head;
	unsigned char fd_sector;
	unsigned char fd_ssiz;
	unsigned char fd_lstsec;
	unsigned char fd_gpl;
	unsigned char fd_dtl;
} fdcs;

struct fdcmdseq fdrcs =
{
	SPECIFY, 0xCF, 0x32, 0, 0, 0, 0, 0, 0
};

#define fd_bps  fd_track
#define fd_spt  fd_head
#define fd_gap  fd_sector
#define fd_fil  fd_ssiz

#define b_fdcmd b_error

int fddevflag=D_DMA;
char v86procflag=0;
struct	fdraw	fdraw[NUMDRV];
struct	buf	fdrawbuf[NUMDRV];

struct  fdbufstruct     fdbufstruct;

struct fdraw fdmtn[NUMDRV];
struct buf fdmtnbuf[NUMDRV];
char	fdmtnbusy;
#define MTNCMD	0x7E
#define SENSE_DSKCHG 0x7D

#define FE_CMD		-1
#define FE_ITOUT	-2
#define FE_RSLT 	-3
#define FE_REZERO 	-4
#define FE_SEEK		-5
#define FE_SNSDRV	-6
#define FE_READID	-7
#define FE_READ		-8
#define FE_BARG		-9

#define NOFLOP_EMAX	4


struct  fdstate fd[NUMDRV];
struct  fdparam fdparam[NUMDRV];

/*
 * floppy disk partition tables
 */
#define H_NCYL 80
struct fdpartab	highpart[FDNPART] = {  /* for format 5H, 3H, 3D */
	{0, 80},
	{0,  1},
	{1, 79}
};

#define Q_NCYL 79
struct fdpartab	quadpart[FDNPART] = {	/* for format 5Q */
	{0, 79},			/* dedicated for AT&T 3B2 */
	{0,  1},
	{1, 78}
};

#define D_NCYL 40
struct fdpartab	dblpart[] = { 	/* for format 5D9, 5D8, 5D4, 5D16 */
	{0, 40},
	{0,  1},
	{1, 39},
	{0, 40}		/* dedicate for AT&T unix pc */
};			/* partition started at track 1 of cylinder 0 */
int fd_secskp[NUMDRV];

/*
 * floppy disk format table
 */
struct fdsectab	fdsectab[FDNSECT] = {
	{  512,  9, 15,       FD_5H, 0x1b, 0x54 }, /* FMT_5H   */
	{  512,  9,  9, FD_5D|FD_5H, 0x2a, 0x50 }, /* FMT_5D9  */
	{  512,  9,  8, FD_5D|FD_5H, 0x2a, 0x50 }, /* FMT_5D8  */
	{ 1024, 10,  4, FD_5D|FD_5H, 0x80, 0xf0 }, /* FMT_5D4  */
	{  256,  8, 16, FD_5D|FD_5H, 0x20, 0x32 }, /* FMT_5D16 */
	{  512,  9,  9,       FD_5H, 0x1b, 0x54 }, /* FMT_5Q   */
	{  512,  9,  9,       FD_3H, 0x1b, 0x54 }, /* FMT_3D   */
	{  512,  9, 18,       FD_3H, 0x1b, 0x54 }, /* FMT_3H   */
};
	
struct  iotime  fdstat[NUMDRV];
struct  iotime *fdstptr[NUMDRV] = {
	(struct iotime *)&fdstat[0],
	(struct iotime *)&fdstat[1],
};

struct  iobuf   fdtab = {0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0};

struct  iobuf   fdutab[NUMDRV] = {
	{0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0}
};

/**
struct  iobuf   fdtab = tabinit(FD0, NULL);
struct  iobuf   fdutab[NUMDRV] = {
	tabinit(FD0, &fdstat[0].ios),
	tabinit(FD0, &fdstat[1].ios),
};
**/
struct  iobuf  *fdutptr[NUMDRV] = {
	(struct iobuf *)&fdutab[0],
	(struct iobuf *)&fdutab[1],
};

static char fdmsg[] = " diskette not present - ";
static char nomesg[] = "";

static char *fd_ctrerr[] =
{
	"command timeout",
	"status timeout",
	"busy",
};

static char *fd_drverr[] =
{
	"Missing data address mark",
	"Cylinder marked bad",
	nomesg,
	nomesg,
	"Seek error (wrong cylinder)",
	"Uncorrectable data read error",
	"Sector marked bad",
	"nomesg",
	"Missing header address mark",
	"Write protected",
	"Sector not found",
	nomesg,
	"Data overrun",
	"Header read error",
	nomesg,
	nomesg,
	"Illegal sector specified",
};

struct dma_cb	*fdcb;
struct dma_buf	*dbuf;

#ifdef MERGE386

extern	int	floppy_free();
extern	int	flp_for_dos();
extern	int	merge386enable;

#endif /* MERGE386 */
/*
 * The floppy controller can read/write  consecutive sectors,
 * but only on a single track (and head).
 */
fdinit()
{
	unsigned char	ddtb;


	/** The following initialization was added because makedev used in **/
	/** tabinit was changed from a macro to a function. Therefore	   **/
	/** tabinit could not be used for initialization.		   **/

	fdtab.b_edev = makedevice(FD0,0);
	fdtab.io_stp = NULL;
	fdutab[0].b_edev = makedevice(FD0,0);
	fdutab[0].io_stp = &fdstat[0].ios;
	fdutab[1].b_edev = makedevice(FD0,0);
	fdutab[1].io_stp = &fdstat[1].ios;
	/** END DDI initialization changes **/

	ddtb = CMOSread(DDTB);
	fd[0].fd_drvtype = ((ddtb >> 4) & 0x0F);
	fd[1].fd_drvtype = ddtb & 0x0F;
	fd[0].fd_hst = fd[1].fd_hst = T50MS;
	fd[0].fd_mst = fd[1].fd_mst = T750MS;
	fdbufgrow(FDMEMSIZE, 1);
	fdreset();
	dma_init();
	fdcb = dma_get_cb(DMA_NOSLEEP);
	dbuf = dma_get_buf(DMA_NOSLEEP);
	fdcb->targbufs = dbuf;
	fdcb->targ_step = DMA_STEP_INC;
	fdcb->targ_path = DMA_PATH_8;
	fdcb->trans_type = DMA_TRANS_SNGL;
	fdcb->targ_type = DMA_TYPE_IO;
	fdcb->bufprocess = DMA_BUF_SNGL;
	return(0);
}

fdopen(devp, flags, otyp, cred_p)
dev_t	*devp;
int	flags;
int	otyp;
struct cred *cred_p;
{
	register struct fdparam *ff;
	register struct fdstate *f;
	register struct buf *bp;
	int	fmt, part, unit;
	int  	i, rtn;
	extern fdint();
	int	oldpri;
	long fdproc_t;
	dev_t dev;

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_NOTE,"fdopen");
#endif

	dev = *devp;
	unit = UNIT(dev);
	part = PARTITION(dev);
	fmt = FRMT(dev);
	if (unit >= NUMDRV || fmt > FMT_MAX ||
	    (part >= FDNPART && fmt != FMT_5D8)) {
		return(ENXIO);
	}
	f = &fd[unit];
	if (f->fd_drvtype == DRV_NONE) {
		return(ENXIO);
	}
	if (fmt != FMT_AUTO) {
		if (((0x01 << (f->fd_drvtype - 1)) &
				 fdsectab[fmt].fd_drvs) == 0) {
			return(ENXIO);
		}
	}
	oldpri = splhi();
	while (f->fd_status & (OPENING | CLOSING))
			sleep(&f->fd_status, PRIBIO);
	f->fd_status |= OPENING;
	splx(oldpri);

#ifdef MERGE386

	if (merge386enable) {
		if (!floppy_free()) {   
			f->fd_status &= ~OPENING;
			wakeup(&f->fd_status);
			return(EBUSY);
		}
	}

#endif /* MERGE386 */
	if(drv_getparm(UPROCP,&fdproc_t) == -1 )
	{
		f->fd_status &= ~OPENING;
		wakeup(&f->fd_status);
		return(EINVAL);
	}
	if (f->fd_status & OPENED) {
		if ( dev != f->fd_device ) {
			f->fd_status &= ~OPENING;
			wakeup(&f->fd_status);
			return(EBUSY);
		}
		if ( v86procflag ) {
			if (f->fd_proc != (struct proc *)fdproc_t) {
				f->fd_status &= ~OPENING;
				wakeup(&f->fd_status);
				return(EBUSY);
			}
		}
		else if ((f->fd_status & EXCLUSV) || (flags & FEXCL) ) {
			f->fd_status &= ~OPENING;
			wakeup(&f->fd_status);
			return(EBUSY);
		}
	}
	else if (v86procflag)
		f->fd_proc = (struct proc *)fdproc_t;

	if ((flags & FEXCL) || v86procflag) 
		f->fd_status |= EXCLUSV;

	if (!v86procflag) {
		/* check for no diskette or open door */
		fdm_seek(unit, 0, 10);
		fdm_rezero(unit);
		if (!(f->fd_drvtype == DRV_5D))
			if (fdm_snsdskchg(unit)) { 
				f->fd_status &= ~(OPENING | EXCLUSV);
				wakeup(&f->fd_status);
				return(EIO);
			}
		if (fdm_readid(unit, 0) == FE_ITOUT) {
			f->fd_status &= ~(OPENING | EXCLUSV);
			wakeup(&f->fd_status);
			return(EIO);
		}

		/* check for write protection */
		rtn = fdm_snsdrv(unit, 0);
		if ((rtn == FE_SNSDRV) ||
		     ((rtn & WPROT) && ((flags & FWRITE) == FWRITE))) {
			f->fd_status &= ~(OPENING | EXCLUSV);
			wakeup(&f->fd_status);
			return(otyp == OTYP_MNT ? EROFS : ENODEV);
		}

		/* now we are sure there is diskeete in the drive. let's take
	 	* a look at the format recorded, if the format wasn't
		* given explicitly.
	 	*/
		if (fmt == FMT_AUTO) {
			fmt = fdm_dfmt(unit);
			if (fmt == FMT_UNKNOWN) {
				f->fd_status &= ~(OPENING | EXCLUSV);
				wakeup(&f->fd_status);
				return(ENXIO);
			}
		}
	}

	f->fd_device = dev;
	f->fd_nsides = SIDES(dev);
	f->fd_maxerr = NORM_EMAX;
	fdm_setup(unit, fmt, part);

	f->fd_status &= ~OPENING;
	f->fd_status |= (OPENED | RSTDOPN);
	wakeup(&f->fd_status);
	return(0);
}

fdclose(dev, flags, otyp, cred_p)
dev_t	dev;
int	flags;
int	otyp;
struct cred *cred_p;
{
	register struct fdstate *f;
	register struct buf *bp;
	int oldpri;
	int	unit;

	unit = UNIT(dev);
	f = &fd[unit];
	f->fd_status |= CLOSING;
	oldpri = splhi();
	bp = fdtab.b_actf;
	while (bp != NULL) {
		if (bp->b_edev == dev) {
			sleep(&fdtab.b_actf,PRIBIO);
			bp = fdtab.b_actf;
		} else
			bp = bp->av_forw;
	}
	fd_mtimer[unit] = 0;
	fd_curmotors &= ~(ENABMOTOR << unit);
	outb(FDCTRL, fd_curmotors|NORESET|ENABINT|fdcst.fd_curdrv);
	splx(oldpri);

	f->fd_status &= ~EXCLUSV;
	f->fd_proc = NULL;

	f->fd_status &= ~(OPENED|CLOSING);
	wakeup(&f->fd_status);
	return(0);
}


fdbreakup(bp)
struct buf	*bp;
{
	int	fdstrategy();

	dma_pageio(fdstrategy, bp);
	return(0);
}

fdread(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;

{
	register struct fdstate *f;

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_CONT,"<fdread>");
#endif
	f = &fd[UNIT(dev)];
	return(physiock(fdbreakup, NULL, dev, B_READ, (daddr_t)(f->fd_n512b),uio_p));
}

fdwrite(dev,uio_p,cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;

{
	register struct fdstate *f;

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_CONT,"<fdwrite>");
#endif
	f = &fd[UNIT(dev)];
	return(physiock(fdbreakup, NULL, dev, B_WRITE, (daddr_t)(f->fd_n512b),uio_p));
}

/*
 * table for encoded number of bytes per sector.
 */
int sectab[4] = {128, 256, 512, 1024};

fdioctl(dev, cmd, arg, mode, cred_p, rval_p)
dev_t dev;
int cmd;
caddr_t arg;
int mode;
struct cred *cred_p;
int *rval_p;
{
	register struct buf *bp;
	register secno, entry;
	register ushort i;
	register ushort cylinder;
	register caddr_t bptr;
	struct fdstate	*f;
	struct fdpartab	*fp;
	struct disk_parms disk_parms;
	union io_arg karg;
	ushort head, secsiz;
	unsigned char fmt, mda;
	int ecode;

	ecode=0;

	f = &fd[UNIT(dev)];

	switch(cmd) {
	case V_GETPARMS: /* get floppy parameters */
		disk_parms.dp_type = DPT_FLOPPY;
		disk_parms.dp_heads = f->fd_nsides;
		disk_parms.dp_cyls = f->fd_ncyls;
		disk_parms.dp_sectors = f->fd_nsects;
		disk_parms.dp_secsiz = f->fd_secsiz;
		disk_parms.dp_ptag = 0; /* not implemented */
		disk_parms.dp_pflag = 0; /* not implemented */
		disk_parms.dp_pstartsec = f->fd_cylskp * f->fd_nsides * f->fd_nsects + fd_secskp[UNIT(dev)];
		disk_parms.dp_pnumsec = ((long) f->fd_n512b << SCTRSHFT) >> (long)f->fd_secsft;
		if (copyout((caddr_t)&disk_parms, arg, sizeof(disk_parms))) {
			return(EFAULT);
		}
		break;

	case V_FORMAT: /* format tracks */
		/*
		 * Get the user's argument.
		 */
		if ((FRMT(f->fd_device) == FMT_AUTO) ||
		     copyin(arg, (caddr_t)&karg, sizeof(karg))) {
			return(ENXIO);
		}
		/*
		 * Calculate starting head and cylinder numbers.
		 */
		head = karg.ia_fmt.start_trk % f->fd_nsides;
		cylinder = karg.ia_fmt.start_trk / f->fd_nsides + f->fd_cylskp;
		/*
		 * Get encoded bytes/sector from table.
		 */
		for (secsiz = 0; secsiz < sizeof(sectab)/sizeof(int); secsiz++)
			if (sectab[secsiz] == f->fd_secsiz)
				break;
		bp = geteblk();
		/*
		 * Format all the requested tracks.
		 */
		for (i = 0; i < karg.ia_fmt.num_trks; i++){ 
			if ((cylinder >= f->fd_ncyls) || (head >= f->fd_nsides)) {
				brelse(bp);
				return(EINVAL);
			}
			/*
			 * Initialize the buffer.
			 */
			bp->b_flags &= ~B_DONE;
			bp->b_bcount = f->fd_nsects * sizeof(struct fdformid);
			bzero(bp->b_un.b_addr, bp->b_bcount);
			/*
			 * Build the format data.  For each sector, we have to
			 * have 4 bytes: cylinder, head, sector, and encoded 
			 * sector size.
			 */
			entry = 0;
			secno = 1;  /* 1-based for DOS */
			do {
				bptr = &bp->b_un.b_addr[entry];
				if (bptr[2] == '\0') {
					*bptr++ = cylinder;
					*bptr++ = head;
					*bptr++ = secno++;
					*bptr = secsiz;
					entry = (entry + karg.ia_fmt.intlv * sizeof(struct fdformid))
								% bp->b_bcount;
				} else
					entry += sizeof(struct fdformid);
			} while (secno <= (int)f->fd_nsects);

			f->fd_maxerr = FORM_EMAX;
			bp->b_edev = dev;
			bp->b_fdcmd = FORMAT;
			fdxfer(bp);
			iowait(bp);
			if (++head >= f->fd_nsides) {
				head = 0;
				cylinder++;
			}
		}
		brelse(bp);
		f->fd_maxerr = NORM_EMAX;
		break;

	case F_RAW:
	{
		int n = UNIT(dev);
		int oldpri;
		int rw = B_READ;
		struct fdraw *fdrawptr = &fdraw[n];
		struct buf *rbp = &fdrawbuf[n];
		int status;

		if (!v86procflag || !(f->fd_status & EXCLUSV) ||
			copyin(arg, (caddr_t)fdrawptr, sizeof(*fdrawptr))) {
			return(ENXIO);
		}

		switch(fdrawptr -> fr_cmd[0] & 0xF) {
		case SEEK:
		case SENSE_INT:
		case READID:
		case REZERO:
		case SENSE_DRV:
		case SPECIFY:
			fdrawptr -> fr_nbytes = 0;
		case FORMAT:
		case WRCMD:
		case WRITEDEL:
			rw = 0;
		case RDCMD & 0xF:
		case READDEL:
		case READTRACK:

			/*
			 * What F_RAW's caller thinks is the unit #
			 * doesn't necessarily have anything
			 * to do with the driver's idea of the
			 * unit #.  Better fix it up now.
			 * NOTE: SPECIFY is the only command which
			 * uses fr_cmd[1] for anything other
			 * than head/unit info.
			 */
			if ( fdrawptr -> fr_cmd[0] != SPECIFY ) {
				fdrawptr -> fr_cmd[1] &= ~0x03;
				fdrawptr -> fr_cmd[1] |= n;
			}
			else
				/*
				 * SPECIFY better not turn off DMA --
				 * this driver is unequipped to handle it
				 */
				fdrawptr -> fr_cmd[2] &= ~0x01;

			rbp->b_flags = rw;
			rbp->b_resid = 0;        /* used for error codes --
						   b_error is same as b_fdcmd */
			rbp->b_fdcmd = RAWCMD;   /* so fdstart,fdintr can tell */
			rbp->b_edev = dev;
			rbp->b_bcount = fdrawptr -> fr_nbytes;      /* for fdxfer */

			/* If we should attempt to  use  the  buffer */
			/* address,  when  there  is  none,  make it */
			/* fault.                                    */

			rbp -> b_un.b_addr = (caddr_t) 0xdf000000;

			/* Are we reading or writing any bytes.   If */
			/* so, then we need exclusive use of the dma */
			/* memory that fdbufstruct is set up for.    */

			if (rbp -> b_bcount != 0) {
				while (fdbufstruct.fbs_flags & B_BUSY) {
					fdbufstruct.fbs_flags |= B_WANTED;
					sleep (&fdbufstruct, PRIBIO);
				}
				fdbufstruct.fbs_flags |= B_BUSY;

				/* If the dma  buffer  is  currently */
				/* not    big   enough   then   call */
				/* fdbufgrow to attempt to grow  it. */
				/* The    buffer   currently   never */
				/* shrinks.                          */

				if (fdrawptr -> fr_nbytes > fdbufstruct.fbs_size) {
					status = fdbufgrow (fdrawptr -> fr_nbytes, 0);
					if (status) {
						ecode = status;
						goto rawdone;
					}
				}

				rbp -> b_un.b_addr = fdbufstruct.fbs_addr;

				/* If it's a write,  copy  the  data */
				/* from the user area.               */

				if (!(rbp->b_flags & B_READ)) {
					if (copyin(fdrawptr -> fr_addr, rbp->b_un.b_addr,
							fdrawptr -> fr_nbytes)) {
						ecode = EFAULT;
						goto rawdone;
					}
				}
			}

			fdxfer(rbp);
			oldpri = splhi();
			if (!(rbp->b_flags & B_DONE))
				sleep((caddr_t)rbp, PRIBIO);
			splx(oldpri);
			if (rbp->b_resid)
				ecode = rbp->b_resid;

			/*
			 * if it was a read, must copy out from kernel buffer
			 * to user
			 */
			if (rbp->b_flags & B_READ && fdrawptr -> fr_nbytes) {
				if (copyout(rbp->b_un.b_addr, fdrawptr -> fr_addr,
						fdrawptr -> fr_nbytes))
					ecode = EFAULT;
			}

		rawdone:

			/* If we  had  used  the  dma  buffer,  must */
			/* release it.                               */

			if (rbp -> b_bcount) {
				fdbufstruct.fbs_flags &= ~B_BUSY;
				if (fdbufstruct.fbs_flags & B_WANTED) {
					fdbufstruct.fbs_flags &= ~B_WANTED;
					wakeup (&fdbufstruct);
				}
			}
			break;
		default:
			ecode = EINVAL;
		}
		if (copyout((caddr_t)fdrawptr, arg, sizeof(*fdrawptr))) {
			return(EFAULT);
		}
		break;
	}
	case F_DOR:
		/*
		 * The only reasonable thing for a user to request is a reset,
		 * so just check that condition.
		 */
		if ( !((*(char*)&arg) & NORESET) )
			fdreset();
		break;

	case F_FCR:
		/* Here only the transfer rate is pertinent */
		f->fd_trnsfr = (*(char*)&arg) & 0x03;
		break;

	case F_DTYP:
		if (copyout( &(f->fd_drvtype), arg, sizeof(f->fd_drvtype))) {
			return(EFAULT);
		}
		break;

	default:
		ecode = EINVAL;
	}
	return(ecode);
}


fdstrategy(bp)
register struct buf *bp;
{
	register struct fdstate *f;

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_CONT,"fdstrategy(0x%lx)",bp->b_blkno);
#endif
	if (bp->b_bcount == 0) {
		iodone(bp);
		return(0);
	}
	f = &fd[UNIT(bp->b_edev)];

	if (bp->b_blkno >= (long)f->fd_n512b) {
		if (bp->b_blkno > (long)f->fd_n512b || (bp->b_flags&B_READ) == 0) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}
	bp->b_fdcmd = (bp->b_flags&B_READ) ? RDCMD : WRCMD;
	fdxfer(bp);
#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_CONT,"<fdstrategy(0x%lx) dopne>",bp->b_blkno);
#endif
	return(0);
}

fdxfer(bp)
register struct buf *bp;
{
	register int oldpri;
	long lbolt_val;

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_NOTE,"fdxfer(0x%lx)",bp->b_blkno);
#endif
	if(drv_getparm(LBOLT,&lbolt_val) == -1)
	{
		return(EINVAL);
	}
	bp->av_forw = NULL;
	bp->b_start = lbolt_val;

	{
		register struct iotime *fit;

		fit = fdstptr[UNIT(bp->b_edev)];
		fit->io_cnt++;
		fit->io_bcnt += btopr(bp->b_bcount);
	}

	{

		oldpri = splhi();
		if (fdtab.b_actf == NULL)
			fdtab.b_actf = bp;
		else
			fdtab.b_actl->av_forw = bp;
		fdtab.b_actl = bp;
		splx(oldpri);
	}
	if (fdtab.b_active == 0) {
		fdtab.b_errcnt = 0;
		fdstart();
	}
	return(0);
}

fdstart()
{
	register struct buf *bp;
	struct fdstate *f;
	long lbolt_val;

	if ((bp = fdtab.b_actf) == NULL)
		return(0);
#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_NOTE,"fdstart(0x%lx)",bp->b_blkno);
#endif
	fdtab.b_active = 1;
	f = &fd[UNIT(bp->b_edev)];

	if (bp->b_fdcmd == RAWCMD) {	/* non-standard processing */
		outb(FDCSR1, f->fd_trnsfr);
		fdrawio(bp);
		return(0);
	}

	if (bp->b_fdcmd == MTNCMD) {	/* maintenance channel */
		outb(FDCSR1, f->fd_trnsfr);
		fdmtnio(bp);
		return(0);
	}

	if (bp->b_fdcmd == FORMAT)
	{
		register struct fdformid *fh;

		fdcs.fd_track = ((struct fdformid *)paddr(bp))->fdf_track;
		fdcs.fd_head = ((struct fdformid *)paddr(bp))->fdf_head;
		fdcst.fd_btot = bp->b_bcount;
	} else {
		register int i;

		i = bp->b_blkno;
		if(i + (bp->b_bcount >> SCTRSHFT) > f->fd_n512b)
			fdcst.fd_btot = (f->fd_n512b - i) << SCTRSHFT;
		else
			fdcst.fd_btot = bp->b_bcount;
		fdcst.fd_blkno = ((long)(unsigned)i << SCTRSHFT) >> (long)f->fd_secsft;
		fdcst.fd_blkno += fd_secskp[UNIT(bp->b_edev)];
		i = f->fd_cylsiz;
		fdcs.fd_track = fdcst.fd_blkno / (ushort)i;
		i = fdcst.fd_blkno % (ushort)i;
		fdcs.fd_head = i / (int)f->fd_nsects;
		fdcs.fd_sector = (i % (int)f->fd_nsects) + 1; /* sectors start at 1 */
		fdcs.fd_track += f->fd_cylskp;
	}
	fdcst.fd_addr = vtop(paddr(bp), bp->b_proc);
	bp->b_resid = fdcst.fd_btot;

	{
		register struct iobuf *fdu;

		fdu = fdutptr[UNIT(bp->b_edev)];
		if (fdu->b_active == 0) {
			fdu->b_active++;
			if(drv_getparm(LBOLT,&lbolt_val) == -1)
			{
				return(EINVAL);
			}
			fdu->io_start = lbolt_val;
		}
	}
		outb(FDCSR1, f->fd_trnsfr);
	{
		register int n;

		do {
			if (fdcst.fd_state != DOOR_OPEN)
				fdcst.fd_state = CHK_RECAL;
		} while ((n = fdio(bp)) && fderror(bp, n) == 0);
	}
	return(0);
}

fdio(bp)
register struct buf *bp;
{
	register int n;
	struct fdstate *f;
	struct fdparam  *ff;
	int	oldpri;
	int	cmdsiz;
	int	retval;
	int	fdwait(), fdtimer();

#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_NOTE,"fdio(0x%lx)",bp->b_blkno);
#endif

	n = UNIT(bp->b_edev);
	f = &fd[n];
	oldpri = splhi();
	if (fd_mtimer[n]) /* keep motor running */
		fd_mtimer[n] = RUNTIM;
	splx(oldpri);

	if (fd_mtimer[n] == 0) { /* start floppy motor first */
		fd_mtimer[n] = RUNTIM;
		fd_curmotors |= (ENABMOTOR << n);
		fdcst.fd_curdrv = n;
		outb(FDCTRL, fd_curmotors|ENABINT|NORESET|n);
		timeout(fdwait, bp, f->fd_mst);
		if (fdt_running == 0)
			timeout(fdtimer, (caddr_t)0, MTIME);
		retval = 0;
		goto done;
	}
	if (fdcst.fd_curdrv != n) {
		fdcst.fd_curdrv = n;
		outb(FDCTRL, fd_curmotors|ENABINT|NORESET|n);
	}
	fdcs.fd_cmdb2 = (fdcs.fd_head<<2)|n;
	if (f->fd_status&RSTDOPN) {
		f->fd_status &= ~RSTDOPN;
		goto dooropen;
	}

	if (f->fd_drvtype != DRV_5D && fdcst.fd_state >= CHK_RECAL) {
		if (inb(FDCSR1) & DOOROPEN) {
			fdcst.fd_state = DOOR_OPEN;
			f->fd_status |= RECAL;
			fd_mtimer[n] = 0;
			fd_curmotors &= ~(ENABMOTOR << n);
			outb(FDCTRL, fd_curmotors|ENABINT|NORESET|n);
			fddooropen(f);
			retval = 0;
			goto done;
		}
	}

	switch(fdcst.fd_state) {
dooropen:
	case DOOR_OPEN:
		fdcst.fd_savtrk = fdcs.fd_track;
		/* force a real seek */
		fdcs.fd_track++;
		if (fdcs.fd_track == f->fd_ncyls)
			fdcs.fd_track -= 2;
		retval = fd_doseek(f);
		goto done;
	case DOOR_RST:
		if (retval = fdsense(fdcs.fd_track)) {
			fdcst.fd_state = DOOR_OPEN;
			goto done;
		}
		fdcst.fd_state = CHK_RECAL;
		fdcs.fd_track = fdcst.fd_savtrk;
	case CHK_RECAL: /* check for rezero */
		if (f->fd_status & RECAL) {
			fdcs.fd_cmdb1 = REZERO;
			fdcst.fd_cstat |= WINTR;
			retval = fdcmd(&fdcs, 2);
			goto done;
		}
		goto seek;
	case RECAL_DONE: /* come here for REZERO completion */
		if (retval = fdsense(0))
			goto done;
		f->fd_curcyl = 0;
		f->fd_status &= ~RECAL;
seek:
		fdcst.fd_state = CHK_SEEK;
	case CHK_SEEK:
		if (f->fd_curcyl != fdcs.fd_track) {
			retval = fd_doseek(f);
			goto done;
		}
		fdcst.fd_state = XFER_BEG;
	case XFER_BEG: /* come here for r/w operation */
		ff = &fdparam[fdcst.fd_curdrv];
		if ((fdcs.fd_cmdb1 = bp->b_fdcmd) == FORMAT) {
			fdcs.fd_bps = ff->fdf_bps;
			fdcs.fd_spt = ff->fdf_spt;
			fdcs.fd_fil = ff->fdf_fil;
			fdcs.fd_gap = ff->fdf_gplf;
			n = bp->b_bcount;
			cmdsiz = 6;
		} else {
			if ((n = ((f->fd_nsects + 1) -
			    fdcs.fd_sector) << f->fd_secsft) > bp->b_resid)
				n = bp->b_resid;
			fdcs.fd_lstsec = f->fd_nsects;
			fdcs.fd_gpl    = ff->fdf_gpln;
			fdcs.fd_ssiz   = ff->fdf_bps;
			fdcs.fd_dtl    = ff->fdf_dtl;
			cmdsiz = 9;
		}
		fdcs.fd_cmdb1 |= ff->fdf_den;
		fdcst.fd_bpart = n;
		if (n){
			fdcb->targbufs->address = fdcst.fd_addr;
			fdcb->targbufs->count = n;
			if( bp->b_flags & B_READ)
				fdcb->command = DMA_CMD_READ;
			else
				fdcb->command = DMA_CMD_WRITE;
			if(dma_prog(fdcb, DMA_CH2, DMA_SLEEP) == FALSE){
				retval = ENXIO;
				goto done;
			}
			dma_enable(DMA_CH2);
		}
		fdcst.fd_cstat |= WINTR;
		retval = fdcmd(&fdcs, cmdsiz);
		goto done;
	case SEEK_DONE: /* come here for SEEK completion */
		if (retval = fdsense(fdcs.fd_track))
			goto done;
		f->fd_curcyl = fdcs.fd_track;
		fdcst.fd_state = XFER_BEG;
		timeout(fdwait, bp, f->fd_hst);
		retval = 0;
		goto done;
	}
	retval = 1;
done:
	return(retval);
}

fd_doseek(f)
register struct fdstate *f;
{
	register int n;

	fdcs.fd_cmdb1 = SEEK;
	fdcst.fd_cstat |= WINTR;
	if (f->fd_dstep)
		fdcs.fd_track <<= 1;
	n = fdcmd(&fdcs, 3);
	if (f->fd_dstep)
		fdcs.fd_track >>= 1;
	return(n);
}


fdintr(ivect)
int	ivect;	/* interrupt vector */
{
	register struct buf *bp;
	register int n;
	struct fdstate *f;
	struct fdparam *ff;
#ifdef DEBUG
	if (fddebug)
		cmn_err(CE_NOTE,"fdintr");
#endif

#ifdef MERGE386
	if(merge386enable) {

		if (flp_for_dos()) /* Test if floppy is assigned to a DOS process */
			return(0);
	}

#endif /* MERGE386 */

	dma_disable(DMA_CH2);	 /* disable channel 2 of the DMA chip */
	if (((bp = fdtab.b_actf) == NULL) || !(fdcst.fd_cstat & WINTR))
		return(0);
	fdcst.fd_cstat &= ~WINTR;



	if (bp->b_fdcmd == RAWCMD) {	/* non-standard processing */
		if (fdraw[UNIT(bp->b_edev)].fr_cmd[0] != SEEK &&
		    fdraw[UNIT(bp->b_edev)].fr_cmd[0] != REZERO &&
		    fdresult(fdraw[UNIT(bp->b_edev)].fr_result, 7)) {
				bp->b_resid = EIO;
		}
		fdtab.b_active = 0;
		fdtab.b_actf = bp->av_forw;
		bp->b_flags |= B_DONE;
		wakeup((caddr_t)bp);	/* wake up fdioctl */
		fdstart();
		return(0);
	}

	if (bp->b_fdcmd == MTNCMD) {	/* maintenance channel */
		if (fdmtn[UNIT(bp->b_edev)].fr_cmd[0] == SEEK || 
		    fdmtn[UNIT(bp->b_edev)].fr_cmd[0] == REZERO) {
			fdcs.fd_cmdb1 = SENSE_INT;
			if (fdcmd(&fdcs, 1))
				bp->b_resid = FE_CMD;
			if (fdresult(fdmtn[UNIT(bp->b_edev)].fr_result, 2))
				bp->b_resid = FE_RSLT;
		} else {	
		    if (fdresult(fdmtn[UNIT(bp->b_edev)].fr_result, 7))
				bp->b_resid = FE_RSLT;
		}
		fdtab.b_active = 0;
		fdtab.b_actf = bp->av_forw;
		bp->b_flags |= B_DONE;
		wakeup((caddr_t)bp);
		fdstart();
		return(0);
	}

	if (++fdcst.fd_state >= XFER_DONE) {
		if (n = fdresult(&fdstatus, 7)) {
			fderror(bp, n);
			fdstart();
			return(0);
		}
		if ((fdstatus.fd_st0 & (ABNRMTERM|INVALID|EQCHK|NOTRDY))) {
			fderror(bp, 1);
			fdstart();
			return(0);
		}
		f = &fd[fdcst.fd_curdrv];
		if ((bp->b_resid -= fdcst.fd_bpart) == 0) {
			fddone(bp);
			fdstart();
			return(0);
		}
		if (++fdcs.fd_head >= f->fd_nsides) {
			fdcs.fd_head = 0;
			fdcs.fd_track++;
			fdcst.fd_state = CHK_SEEK;  /* go back to seek state */
		} else
			fdcst.fd_state = XFER_BEG; /* stay in data xfer state */
		fdcs.fd_sector = 1;
		fdcst.fd_addr += fdcst.fd_bpart;
		fdcst.fd_blkno += (fdcst.fd_bpart >> f->fd_secsft);
	}	
	if (n = fdio(bp)) {
		fderror(bp,n);
		fdstart();
	}
	return(0);
}


fddone(bp)
register struct buf *bp;
{
	int oldpri;
	register struct iotime *fit;
	struct iobuf *fdu;
	long lbolt_val;

	oldpri = splhi();
	if (fd_mtimer[fdcst.fd_curdrv])
		fd_mtimer[fdcst.fd_curdrv] = WAITTIM;
	splx(oldpri);
	fdu = fdutptr[UNIT(bp->b_edev)];
	fit = fdstptr[UNIT(bp->b_edev)];

	if(drv_getparm(LBOLT,&lbolt_val) == -1)
	{
		return(EINVAL);
	}

	fit->io_resp += lbolt_val - bp->b_start;
	fit->io_act += lbolt_val - fdu->io_start;
	fdu->b_active = 0;
	fdtab.b_active = 0;
	fdtab.b_errcnt = 0;
	fdtab.b_actf = bp->av_forw;
	wakeup(&fdtab.b_actf);
	if ((bp->b_resid += (bp->b_bcount - fdcst.fd_btot)) != 0){
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
	}
	else
		bp->b_error = 0;
	iodone(bp);
	return(0);
}

fdsense(cylnum)
{
	register int n;

	fdcs.fd_cmdb1 = SENSE_INT;
	if (n = fdcmd(&fdcs, 1))
		goto done;
	if (n = fdresult(&fdsnst, 2))
		goto done;
	if (fd[fdcst.fd_curdrv].fd_dstep)
		cylnum <<= 1;
	if ((fdsnst.fd_sst0 & (INVALID|ABNRMTERM|SEEKEND|EQCHK|NOTRDY)) !=
	    SEEKEND || fdsnst.fd_scyl != cylnum)
		n = 1;
done:
	return(n);
}

fderror(bp,type)
struct buf *bp;
int type;
{
	register struct fdstate *f;
	register int i;
	unsigned char ltrack;
	int error;

	f = &fd[fdcst.fd_curdrv];

	if (type == 1) {
		f->fd_status |= RECAL;
		if (fdcst.fd_state >= XFER_BEG && bp->b_fdcmd != FORMAT) {
			ltrack = fdstatus.fd_strack;
			if (f->fd_dstep)
				ltrack >>= 1;
			i = fdstatus.fd_ssector - fdcs.fd_sector;
			fdcs.fd_track = ltrack;
			fdcs.fd_sector = fdstatus.fd_ssector;
			fdcst.fd_blkno += i;
			i <<= f->fd_secsft;
			fdcst.fd_addr += i;
			bp->b_resid -= i;
		}
	} else
		fdreset();

	if (++fdtab.b_errcnt > f->fd_maxerr) {
		if (type == 1) {
			if (fdcst.fd_state >= XFER_BEG) {
				f->fd_lsterr = fdstatus.fd_st1;
				error = (fdstatus.fd_st1<<8)|fdstatus.fd_st2;
				for (i = 0; i < 16 && ((error&01) == 0); i++)
					error >>= 1;
			} else
				i = 4; /* fake a seek error */
			if (bp->b_fdcmd == FORMAT) {
				cmn_err(CE_WARN,"FD  drv %d, trk %d: %s",fdcst.fd_curdrv,
				    f->fd_curcyl, fd_drverr[i]);
			} else {
				cmn_err(CE_WARN,"FD  drv %d, blk %d: %s\n",fdcst.fd_curdrv,
				    fdcst.fd_blkno, fd_drverr[i]);
			}
			fdreset();
		} else
			cmn_err(CE_WARN,"FD controller %s",fd_ctrerr[type-2]);
		bp->b_flags |= B_ERROR;
		fddone(bp);
		return(1);
	}
	if (fdtab.b_errcnt == TRYRESET)
		fdreset();
	fdcst.fd_state = CHK_RECAL;
	return(0);
}

/*
 * entered after a timeout expires when waiting for
 * floppy drive to perform something that takes too
 * long to busy wait for.
 */
fdwait(bp)
register struct buf *bp;
{
	register int n;

	if (n = fdio(bp)) {
		fderror(bp,n);
		fdstart();
	}
}

fdnoflop(f)
register struct fdstate *f;
{
	register struct buf *bp;

	bp = fdtab.b_actf;
	if(bp == NULL)
		return;
	if (fdcst.fd_etimer == 0)
		return;
	if (--fdcst.fd_etimer == 0) {
		if (++fdtab.b_errcnt >= NOFLOP_EMAX) {
			bp->b_flags |= B_ERROR;
			fddone(bp);
		}
		fdstart();
	} else
		timeout(fdnoflop, f, ETIMOUT);
}

fdtimer(dummy)
{
	register int i;
	register struct fdstate *f;
	struct buf *bp;

	for (i = 0; i < NUMDRV; i++) {
		if (fd_mtimer[i] == 0)
			continue;
		if (--fd_mtimer[i] == 0) {
			fd_curmotors &= ~(ENABMOTOR << i);
			outb(FDCTRL, fd_curmotors|NORESET|ENABINT|fdcst.fd_curdrv);
			if (i == fdcst.fd_curdrv && (fdcst.fd_cstat&WINTR)) {
				fdcst.fd_cstat &= ~WINTR;
				bp = fdtab.b_actf;
				f = &fd[fdcst.fd_curdrv];
				fdreset();

				if ((f->fd_status & EXCLUSV)) {
					fdtab.b_active = 0;
					fdtab.b_actf = bp->av_forw;
					bp->b_resid = EBUSY;
					bp->b_flags |= B_DONE;
					wakeup((caddr_t)bp);
					fdstart();
				} else if (bp->b_fdcmd == MTNCMD) {
					bp->b_resid = FE_ITOUT;
					fdtab.b_active = 0;
					fdtab.b_actf = bp->av_forw;
					bp->b_flags |= B_DONE;
					wakeup((caddr_t)bp);
					fdstart();
				}
				else
					fddooropen(f);
			}
		}
	}
	if (fd_curmotors) {
		timeout(fdtimer, (caddr_t)0, MTIME);
		fdt_running = 1;
	} else
		fdt_running = 0;
}

fddooropen(f)
register struct fdstate *f;
{
	cmn_err(CE_WARN,"FD(%d): No diskette present - Please insert",fdcst.fd_curdrv);
	fdcst.fd_etimer = LOADTIM;
	timeout(fdnoflop, f, ETIMOUT);
}



fdreset()
{
	register int i;
	register int oldpri;

	for (i = 0; i < NUMDRV; i++)
		fd[i].fd_status |= RECAL;
	fdcst.fd_cstat &= ~WINTR;
	oldpri = splhi();
	outb(FDCTRL, ENABINT);        /* reset floppy controller */
	for (i = 0; i < 5; i++)
		tenmicrosec(); /* wait 30-45 usec for NEC */
	outb(FDCTRL, fd_curmotors|ENABINT|NORESET|fdcst.fd_curdrv);
	splx(oldpri);
	fdsense(0);
	fdcmd(&fdrcs, 3);
}


/*
 * Error message, called from Unix 5.3 kernel via bdevsw
 */
fdprint (dev, str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE, "%s on floppy disk unit %d, partition %d",
		str, UNIT(dev), PARTITION(dev));
}

/*
 * Routine to return diskette drive status information
 * The diskette controller data register is read the 
 * requested number of times and the results placed in
 * consecutive memory locations starting at the passed
 * address.
 */
fdresult(addr, cnt)
caddr_t	addr;
int	cnt;
{
	unsigned char	msr;
	register int	i, j;
	int		ntries;

	for (i = cnt; i > 0; i--) {
/*
 * read main status register to see if OK to read data.
 */
		ntries = FCRETRY;
		while (TRUE) {
			msr = inb(FDSTAT);
			if ((msr & (IODIR|IORDY)) == (IODIR|IORDY))
				break;
			if (--ntries <= 0)
				return RTIMOUT;
			else
				tenmicrosec();
		}
		for (j = 0; j < 5; j++)
			tenmicrosec(); /* wait 30-45 usec for NEC */
		*addr++ = inb(FDDATA);
	}
	for (j = 0; j < 5; j++)
		tenmicrosec(); /* wait 30-45 usec for NEC */
	msr = inb(FDSTAT);
	if (msr & FCBUSY)
		return NECERR;
	return 0;
}

/*
 * routine to program a command into the floppy disk controller.
 * the requested number of bytes is copied from the given address
 * into the floppy disk data register.
 */
fdcmd(cmdp, size)
caddr_t	cmdp;
int	size;
{
	unsigned char	msr;
	register int	i, j;
	int		ntries;

	for (i = size; i > 0; i--) {
		ntries = FCRETRY;
		while (TRUE) {
			msr = inb(FDSTAT);
			if ((msr & (IODIR|IORDY)) == IORDY)
				break;
			if (--ntries <= 0)
				return CTIMOUT;
			else
				tenmicrosec();
		}
		for (j = 0; j < 5; j++)
			tenmicrosec(); /* wait 30-45 usec for NEC */
		outb(FDDATA, *cmdp++);
	}
	return 0;
}


fdrawio(bp)
register struct buf *bp;
{
	int rnum;			/* number of result bytes */
	register int n = UNIT(bp->b_edev);
	int oldpri;
	int fdtimer();

	/*
	 * This function is called directly from fdstart() when a bp
	 * specifying a RAWCMD is found.  It is responsible for completely
	 * processing the raw request.
	 */

	oldpri = splhi();
	if (fd_mtimer[n]) /* if motor's running, keep it going */
		fd_mtimer[n] = RUNTIM;
	splx(oldpri);

	/* first turn on the specified drive's motor if it isn't running */
	if (fd_mtimer[n] == 0) {
		fd_mtimer[n] = RUNTIM;
		fdcst.fd_curdrv = n;
		fd_curmotors |= (ENABMOTOR << n);
		outb(FDCTRL, fd_curmotors | ENABINT | NORESET | n);
		timeout(fdrawio, bp, fd[n].fd_mst);	/* wait for motor */
		if (fdt_running == 0)
			timeout(fdtimer, (caddr_t)0, MTIME);
		return(0);
	}
	if (fdcst.fd_curdrv != n) {
		fdcst.fd_curdrv = n;
		outb(FDCTRL, fd_curmotors|ENABINT|NORESET|n);
	}

	/* if data-transfer or SENSE_DRV command, check for media change */
	if ((bp->b_bcount || (fdraw[n].fr_cmd[0] == SENSE_DRV)) &&
						(inb(FDCSR1) & DOOROPEN))
	{
		bp->b_resid = EBUSY;
		goto finish;
	}

	if (bp->b_bcount){
		fdcb->targbufs->address = kvtophys(bp->b_un.b_addr);
		fdcb->targbufs->count = bp->b_bcount;
		if( bp->b_flags & B_READ)
			fdcb->command = DMA_CMD_READ;
		else
			fdcb->command = DMA_CMD_WRITE;
		if(dma_prog(fdcb, DMA_CH2, DMA_SLEEP) == FALSE){
			bp->b_resid = ENXIO;
			fdreset();
			goto finish;
		}
		dma_enable(DMA_CH2);
	}
	fdcst.fd_cstat |= WINTR;
	if (fdcmd(fdraw[n].fr_cmd, fdraw[n].fr_cnum)) { /* controller error */
		bp->b_resid = EIO;
		fdreset();
		goto finish;
	}

	switch (fdraw[n].fr_cmd[0]) {
	/* no interrupt will be received in these cases */
	case SENSE_INT:
		rnum = 2;
		goto a;
	case SPECIFY:
		rnum = 0;
		goto a;
	case SENSE_DRV:
		rnum = 1;
a:
		fdcst.fd_cstat &= ~WINTR;
		if (fdresult(fdraw[n].fr_result, rnum)) {
			bp->b_error = EIO;
			fdreset();
		}
		goto finish;
	default:
		return(0);
	}
finish:
	fdtab.b_active = 0;
	fdtab.b_actf = bp->av_forw;
	bp->b_flags |= B_DONE;
	wakeup((caddr_t)bp);		/* wake up fdioctl */
	fdstart();
	return(0);
}

fdmtnio(bp)
register struct buf *bp;
{
	int rnum;			/* number of result bytes */
	register int n = UNIT(bp->b_edev);
	int oldpri;
	int fdtimer();

	oldpri = splhi();
	if (fd_mtimer[n]) /* if motor's running, keep it going */
		fd_mtimer[n] = RUNTIM;
	splx(oldpri);

	/* first turn on the specified drive's motor if it isn't running */
	if (fd_mtimer[n] == 0) {
		fd_mtimer[n] = RUNTIM;
		fdcst.fd_curdrv = n;
		fd_curmotors |= (ENABMOTOR << n);
		outb(FDCTRL, fd_curmotors | ENABINT | NORESET | n);
		timeout(fdmtnio, bp, fd[n].fd_mst);	/* wait for motor */
		if (fdt_running == 0)
			timeout(fdtimer, (caddr_t)0, MTIME);
		return;
	}
	if (fdcst.fd_curdrv != n) {
		fdcst.fd_curdrv = n;
		outb(FDCTRL, fd_curmotors|ENABINT|NORESET|n);
	}

	if (fdmtn[n].fr_cmd[0] == SENSE_DSKCHG) {
		fdmtn[n].fr_result[0] = (inb(FDCSR1) & DOOROPEN);
		goto finish;
	}

	if (bp->b_bcount){
		fdcb->targbufs->address = kvtophys(bp->b_un.b_addr);
		fdcb->targbufs->count = bp->b_bcount;
		if( bp->b_flags & B_READ)
			fdcb->command = DMA_CMD_READ;
		else
			fdcb->command = DMA_CMD_WRITE;
		if(dma_prog(fdcb, DMA_CH2, DMA_SLEEP) == FALSE){
			bp->b_resid = ENXIO;
			fdreset();
			goto finish;
		}
		dma_enable(DMA_CH2);
	}
	fdcst.fd_cstat |= WINTR;
	if (fdcmd(fdmtn[n].fr_cmd, fdmtn[n].fr_cnum)) { /* controller error */
		bp->b_resid = FE_CMD;
		fdreset();
		goto finish;
	}

	switch (fdmtn[n].fr_cmd[0]) {
	/* no interrupt will be received in these cases */
	case SENSE_INT:
		rnum = 2;
		goto a;
	case SENSE_DRV:
		rnum = 1;
a:
		fdcst.fd_cstat &= ~WINTR;
		if (fdresult(fdmtn[n].fr_result, rnum)) {
			bp->b_resid = FE_RSLT;	/* controller error */
			fdreset();
		}
		goto finish;
	default:
		return;
	}
finish:
	fdtab.b_active = 0;
	fdtab.b_actf = bp->av_forw;
	bp->b_flags |= B_DONE;
	wakeup((caddr_t)bp);
	fdstart();
}

fdm_seek(drv, head, cyl)
unsigned char drv, head, cyl;
{
	int oldpri, rtn;
	struct buf *bp;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = SEEK; 
	fdmtn[drv].fr_cmd[1] = (head << 2) | drv;
	fdmtn[drv].fr_cmd[2] = cyl;
	fdmtn[drv].fr_cnum = 3;

	bp = &fdmtnbuf[drv];	
	bp->b_flags = 0;
	bp->b_resid = 0;
	bp->b_bcount = 0;
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	fdxfer(bp);
	oldpri = splhi();
	while (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);
	if (bp->b_resid || (fdmtn[drv].fr_result[0] & 0xC0) ||
			   (fdmtn[drv].fr_result[1] != cyl)) {
		rtn = FE_SEEK;	
	} else
		rtn = cyl;


	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}

fdm_rezero(drv)
unsigned char drv;
{
	int oldpri, rtn;
	struct buf *bp;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = REZERO;
	fdmtn[drv].fr_cmd[1] = drv;
	fdmtn[drv].fr_cnum = 2;

	bp = &fdmtnbuf[drv];
	bp->b_flags = 0;
	bp->b_resid = 0;
	bp->b_bcount = 0;
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	fdxfer(bp);
	oldpri = splhi();
	while (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);
	if (bp->b_resid || (fdmtn[drv].fr_result[0] & 0xC0) ||
			   (fdmtn[drv].fr_result[1] != 0)) {
		rtn = FE_REZERO;	
	}else
		rtn = 0;

	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}

fdm_snsdrv(drv, head)
unsigned char drv, head;
{
	int oldpri, rtn;
	struct buf *bp;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = SENSE_DRV;
	fdmtn[drv].fr_cmd[1] = (head << 2) | drv;
	fdmtn[drv].fr_cnum = 2;

	bp = &fdmtnbuf[drv];
	bp->b_flags = 0;
	bp->b_resid = 0;
	bp->b_bcount = 0;
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	fdxfer(bp);
	oldpri = splhi();
	while (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);
	if (bp->b_resid) {
		rtn = FE_SNSDRV;
	} else	
		rtn = fdmtn[drv].fr_result[0];

	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}

fdm_setup(drv, fmt, part)
char drv, fmt, part;
{
	struct fdstate *f;
	struct fdparam *ff;
	char dt;
	int nsectors;

	if ((fmt < 0) || (fmt > FMT_MAX)) {
		return(FE_BARG);
	}

	f = &fd[drv];
	dt = f->fd_drvtype;

	f->fd_secsiz = fdsectab[fmt].fd_ssize;
	f->fd_secsft = fdsectab[fmt].fd_sshift;
	f->fd_secmsk = fdsectab[fmt].fd_ssize - 1;
	f->fd_nsects = fdsectab[fmt].fd_nsect;
	f->fd_cylsiz = f->fd_nsides * f->fd_nsects;
	
	if (fmt == FMT_5D8 && part == 3)
		fd_secskp[drv] = 8;
	else
		fd_secskp[drv] = 0;

	if (fmt >= FMT_5D9 && fmt <= FMT_5D16) {
		f->fd_cylskp = dblpart[part].startcyl;
		f->fd_ncyls = D_NCYL;
		nsectors = dblpart[part].numcyls * f->fd_cylsiz - fd_secskp[drv];
	} else if (fmt == FMT_5Q) {
		f->fd_cylskp = quadpart[part].startcyl;
		f->fd_ncyls = Q_NCYL;
		nsectors = quadpart[part].numcyls * f->fd_cylsiz;
	} else {
		f->fd_cylskp = highpart[part].startcyl;
		f->fd_ncyls = H_NCYL;
		nsectors = highpart[part].numcyls * f->fd_cylsiz;
	}
	f->fd_n512b = (int)((long) nsectors << f->fd_secsft) >> SCTRSHFT;

	if ((fmt >= FMT_5D9 && fmt <= FMT_5D16) && f->fd_drvtype == DRV_5H)
		f->fd_dstep = 1;
	else
		f->fd_dstep = 0;
	
	if (dt == DRV_5D || fmt == FMT_3D)
		f->fd_trnsfr = FD250KBPS;
	else if ((dt == DRV_5H && fmt == FMT_5H) ||
		 (dt == DRV_3H && fmt == FMT_3H))
		f->fd_trnsfr = FD500KBPS;
	else
		f->fd_trnsfr = FD300KBPS;

	ff = &fdparam[drv];
	ff->fdf_bps = f->fd_secsft - 7;
	ff->fdf_dtl = 0xFF;
	ff->fdf_fil = 0xF6;
	ff->fdf_den = DEN_MFM;
	ff->fdf_gpln = fdsectab[fmt].fd_gpln;
	ff->fdf_gplf = fdsectab[fmt].fd_gplf;
	ff->fdf_spt = f->fd_nsects;
	ff->fdf_nhd = f->fd_nsides;
	ff->fdf_ncyl = f->fd_ncyls;
	return(0);
}

/* Return byte 0: encorded byte per sector
	       1: recorded cylinder number
 */
fdm_readid(drv, head)
unsigned char 	drv, head;
{
	int oldpri, rtn;
	struct buf *bp;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = DEN_MFM | READID; 
	fdmtn[drv].fr_cmd[1] = (head << 2) | drv;
	fdmtn[drv].fr_cnum = 2;

	bp = &fdmtnbuf[drv];	
	bp->b_flags = 0;
	bp->b_resid = 0;
	bp->b_bcount = 0;
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	fdxfer(bp);
	oldpri = splhi();
	while (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);
	if (bp->b_resid == FE_ITOUT)
		rtn = FE_ITOUT;
	else if (bp->b_resid || (fdmtn[drv].fr_result[0] & 0xC0))
		rtn = FE_READID;	
	else
		rtn = (fdmtn[drv].fr_result[3] << 8) | fdmtn[drv].fr_result[6];

	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}


fdm_snsdskchg(drv)
unsigned char drv;
{
	int oldpri, rtn;
	struct buf *bp;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = SENSE_DSKCHG; 
	fdmtn[drv].fr_cmd[1] = drv;
	fdmtn[drv].fr_cnum = 2;

	bp = &fdmtnbuf[drv];	
	bp->b_flags = 0;
	bp->b_resid = 0;
	bp->b_bcount = 0;
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	fdxfer(bp);
	oldpri = splhi();
	while (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);

	rtn = fdmtn[drv].fr_result[0];

	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}

fdm_read(drv, head, cyl, sect, nsects, bps, eot, gpl, dtl) 
char	drv, head, cyl, sect, nsects;
char	bps, eot, gpl, dtl;
{
	int oldpri, rtn;
	struct buf *bp;
	caddr_t	va;
	int  i;
	long onepage;

	while (fdmtnbusy & (0x01 << drv))
		sleep(&fdmtn[drv], PRIBIO);
	fdmtnbusy |= (0x01 << drv);

	fdmtn[drv].fr_cmd[0] = DEN_MFM | RDCMD; 
	fdmtn[drv].fr_cmd[1] = (head << 2) | drv;
	fdmtn[drv].fr_cmd[2] = cyl;
	fdmtn[drv].fr_cmd[3] = head;
	fdmtn[drv].fr_cmd[4] = sect;
	fdmtn[drv].fr_cmd[5] = bps;
	fdmtn[drv].fr_cmd[6] = eot;
	fdmtn[drv].fr_cmd[7] = gpl;
	fdmtn[drv].fr_cmd[8] = dtl;
	fdmtn[drv].fr_cnum = 9;

	bp = &fdmtnbuf[drv];	
	bp->b_flags = B_READ;
	bp->b_resid = 0;
	bp->b_bcount = nsects * (0x01 << (bps + 7));
	bp->b_fdcmd = MTNCMD;
	bp->b_edev = drv;

	va = (caddr_t)getcpages(1,1);

	if (va == NULL) {
		cmn_err(CE_NOTE,"Unable to get kernel pages\n");
		fdmtnbusy &= ~(0x01 << drv);
		wakeup(&fdmtn[drv]);
		return(FE_READ);
	}
	bp->b_un.b_addr = va;

	fdxfer(bp);
	oldpri = splhi();
	if (!(bp->b_flags & B_DONE))
		sleep((caddr_t)bp, PRIBIO);
	splx(oldpri);

	freepage(kvtopfn(va));

	if (bp->b_resid || (fdmtn[drv].fr_result[0] & 0xC0))
		rtn = FE_READ;	
	else
		rtn = 0;

	fdmtnbusy &= ~(0x01 << drv);
	wakeup(&fdmtn[drv]);
	return(rtn);
}

fdm_dfmt(dev)
dev_t dev;
{
	struct fdstate *f;
	char dt, part, cyl;
	int rtn, curcyl;
	char drv;

	drv = UNIT(dev);
	f = &fd[drv];
	dt = f->fd_drvtype;
	part = PARTITION(dev);
	
	fdm_seek(drv, 0, 4);	/* Position the head so following */
	fdm_rezero(drv);	/* recalibration will be successful */
	cyl = 0;
	if (part == 2)
		cyl = 2;

	f->fd_fmt = FMT_UNKNOWN;
	switch (dt) {
	case DRV_5H:
		fdm_seek(drv, 0, cyl);
		f->fd_trnsfr = FD500KBPS;
		if (fdm_readid(drv, 0) != FE_READID) {
	 	    f->fd_fmt = FMT_5H;
		    break;
		}
		fdm_seek(drv, 0, 2);
		f->fd_trnsfr = FD300KBPS;
		if ((rtn = fdm_readid(drv,0)) != FE_READID) {
		    curcyl = (rtn >> 8) & 0xFF;
		    if (curcyl == 2) {
			f->fd_fmt = FMT_5Q;
			break;
		    }
		    if (curcyl == 1) {
			switch(rtn & 0xFF) {
			case FD256BPS:		/* 256 bytes per sector */
			    f->fd_fmt = FMT_5D16;
			    break;
			case FD512BPS:		/* 512 bytes per sector */
			    if (fdm_read(drv, 0, 1, 9, 1, 2, 9, 0x2A, 0xFF)
					== FE_READ) 
				f->fd_fmt = FMT_5D8;
			    else
				f->fd_fmt = FMT_5D9;
			    break;
			case FD1024BPS:		/* 1024 bytes per sector */
			    f->fd_fmt = FMT_5D4;
			    break;
			default:
			    break;
			}
		        break;
		    }
		}
		break;
	case DRV_5D:
		fdm_seek(drv, 0, cyl);
		f->fd_trnsfr = FD250KBPS;
		if ((rtn = fdm_readid(drv,0)) != FE_READID &&
		     (((rtn >> 8) & 0xFF) == cyl))
		    switch(rtn & 0xFF) {
		    case FD256BPS:
			f->fd_fmt = FMT_5D16;
			break;
		    case FD512BPS:
			if (fdm_read(drv, 0, cyl, 9, 1, 2, 9, 0x2A, 0xFF)
				== FE_READ) 
				f->fd_fmt = FMT_5D8;
			else
				f->fd_fmt = FMT_5D9;
			break;
		    case FD1024BPS:
			f->fd_fmt = FMT_5D4;
			break;
		    default:
			break;
		    }
		break;
	case DRV_3H:
		fdm_seek(drv, 0, cyl);
		f->fd_trnsfr = FD500KBPS;
		if (fdm_readid(drv, 0) != FE_READID) {
		    f->fd_fmt = FMT_3H;
		    break;
		}
		f->fd_trnsfr = FD250KBPS;
		if (fdm_readid(drv, 0) != FE_READID) {
		    f->fd_fmt = FMT_3D;
		    break;
		}
	default:
		break;
	}
	f->fd_dskchg = 0;
	return(f->fd_fmt);
}

#ifdef MERGE386
int
unix_has_floppy()
{
	register struct fdstate *f;
	int status = FALSE;
	int unit;

	for( unit = 0; unit < NUMDRV ; unit++ ) {
		f = &fd[unit];
		if ( f->fd_status & (OPENED|OPENING|CLOSING) )
			status = TRUE;
	}
		return(status);
}
#endif /* MERGE386 */

fdbufgrow (bytesize, sleepflag)
unsigned    bytesize;
int         sleepflag;
{
	register    unsigned    int     pageon;
	register    unsigned    int     dmapage;
	register    unsigned    int     pageswanted;
	register    unsigned    int     pagesrequested;
	register    unsigned    int     curpages;
	register    caddr_t     newbuf;
	register    unsigned    int     oldpfn;
	register    unsigned    int     newpfn;

	/* Grow or Shrink a buffer to hold  bytesize  bytes  without */
	/* crossing a dma boundary.                                  */

	/* We can't handle more than 64k, since this has to cross  a */
	/* dma boundary.                                             */

	if (bytesize > 0x10000)
	    return (EINVAL);

	/* Compute the number of pages  we  want,  as  well  as  the */
	/* number we already have, and the page frame number of the  */
	/* current buffer.                                           */

	pageswanted = btopr (bytesize);
	curpages = btopr (fdbufstruct.fbs_size);

	/* If we already have enough pages,  then  free  any  excess */
	/* pages, and update the new size.                           */

	if (pageswanted <= curpages) {
	    oldpfn = kvtopfn (fdbufstruct.fbs_addr);
	    for (pageon = pageswanted; pageon < curpages; pageon++)
		freepage (oldpfn + pageon);
	    fdbufstruct.fbs_size = ptob (pageswanted);
	    return (0);
	    }

	/* The number of pages  necessary  to  insure  that  we  get */
	/* pageswanted  pages that do not cross a dma boundary is (2 */
	/* * pageswanted - 1), as the  worst  case  is  if  the  dma */
	/* boundary  is  at  (pagewanted  -  1)  and  then  we  need */
	/* (pagewanted) pages.                                       */

	pagesrequested = 2 * pageswanted - 1;

	/* Get the contiguous pages, if we can't, print as error and */
	/* return.                                                   */

	newbuf = (caddr_t) getcpages (pagesrequested, sleepflag);
	if (newbuf == NULL) {
	    printf ("Unable to allocate memory for floppy raw buffer\n");
	    return (EAGAIN);
	    }

	newpfn = kvtopfn (newbuf);

	/* Find the spot of the dma boundary.  dmapage will  be  set */
	/* at  the  location  where the next dma boundary begins.    */

	dmapage = btopr (0x10000 - (kvtophys (newbuf) & 0xffff));

	/* Check  whether  we  found  a  boundary   in   the   first */
	/* pageswanted pages.                                        */

	if (dmapage < pageswanted) {

	    /* We found a dma  boundary  in  the  first  pageswanted */
	    /* pages.   We  will  use the pageswanted pages starting */
	    /* from the dma boundary.                                */

	    /* Free the pages that were prior to the dma boundary.   */

	    for (pageon = 0; pageon < dmapage; pageon++)
		freepage (newpfn + pageon);

	    /* Free the pages after the area we are reserving.       */

	    for (pageon = dmapage + pageswanted; pageon < pagesrequested;
	      pageon++)
		freepage (newpfn + pageon);

	    /* Point buffer at area that begins at the dma page      */

	    newbuf += ptob (dmapage);
	    }

	else {
	    /* We  did  not  find  a  dma  boundary  in  the   first */
	    /* pageswanted pages, so we will use that area.          */

	    /* Free the subsequent pages.                            */

	    for (pageon = pageswanted; pageon < pagesrequested; pageon++)
		freepage (newpfn + pageon);
	    }

	/* Free the old area that had been used.                     */

	if (curpages != 0) {
	    oldpfn = kvtopfn (fdbufstruct.fbs_addr);
	    for (pageon = 0; pageon < curpages; pageon++)
		freepage (oldpfn + pageon);
	    }

	/* Set the new area that will be used.                       */

	fdbufstruct.fbs_addr = newbuf;
	fdbufstruct.fbs_size = ptob (pageswanted);
	return (0);
}
