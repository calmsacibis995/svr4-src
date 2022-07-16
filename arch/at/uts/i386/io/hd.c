#ident	"@(#)hd.c	1.2	92/01/16	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/hd.c	1.3.2.6"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/hd.h"
#include "sys/alttbl.h"
#include "sys/vtoc.h"
#include "sys/fdisk.h"
#include "sys/bootinfo.h"
#include "sys/open.h"
#include "sys/debug.h"
#include "sys/file.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/kmem.h"
#include "sys/ddi.h"
#ifdef ASYNCIO
#include "sys/async.h"
#endif


/* Declaration for BBH to be moved hd.h starts here*/


/*
 * The following defines serve two purposes. The first 8 represent the error
 * status's which can be returned by the disk controller. All 17 messages 
 * are used as indexes into the hdb_msg array of messages.
 */
#define DNF	0  /* Data address mark Not Found */
#define TK0     1  /* TracK 0 not found */
#define ACD     2  /* Aborted CommanD error */
#define UNUSE3	3
#define INF     4  /* sector Id Not Found error */
#define UNUSED5 5
#define UNRDBL	6  /* non-correctable ECC error (correct name ECC) */
#define BADBLK	7  /* Bad Block (tag 80) detected */
#define MAPBLK	8
#define BSACRD  9
#define BNOSPR  10
#define	ECCMSG	11
#define	SACRED	12
#define NOSPAR	13
#define BEGVFY	14
#define MRGBLK	15
#define GOODBLK	16


/*
 * messages used by bad block handling.
 */
static char *hdb_msg[] = {
	"Data from Block %d on drive %d is not readable. Data in this block \
has been lost.",
	"Disk is not able to find track 0. Block %d, Drive %d.",
	"Disk controller aborted issued command. Block %d, Drive %d",
	"Error status 3 - NOT USED block %d drive %d.",
	"Requested block %d on drive %d not found. Block will be remapped.",
	"Error status 5 - NOT USED block %d drive %d.",
	"Block %d on drive %d is unreadable. Data of this block has been \
lost.",
	"Block %d on drive %d is physically marked bad on the hard disk",
	"An alternate block has been assigned to block %d on drive %d.",
	"A bad block has been detected (block %d drive %d)\n\
on a sacred area of the hard disk.\nThe system can not recover from this \
failure.\nMust reinstall the UNIX System and restore from previous backups.",
	"The system is out of spare blocks for surrogates. Block %d\n\
on drive %d can not be mapped.",
	"Soft read error corrected by ECC algorithm: block %d on drive %d\n",
	"A potential bad block has been detected (block %d drive %d)\n\
on a sacred area of the hard disk.\nIf this block goes bad, the UNIX System\
will be lost.\nPlease backup your system.",
	"A potential bad block has been detected (block %d drive %d).\n\
The system is out of spare blocks for surrogates.\nIf the block goes bad,\
it can not be mapped.",
	"A potential bad block has been detected (block %d drive %d).\n\
Starting verification to determine if an alternate block needs to be\n\
assigned to this block.",
	"Verification completed. An alternate block will be assigned\n\
to block %d on drive %d.",
	"Verification completed, Block %d on drive %d is a good block.",

};


/* Information about disk blocks used while testing for bad block */

#define MAXXFER		256		/* controller limitation (blocks) */
#define MAXNBLKS	256		/* Max disk blks. in a buffer to xmt */
#define TBUFFLEN	SECSIZE		/* Length in bytes of temp. buffer   */

struct hdbadhnd{
	daddr_t hdb_b2vfy[MAXNBLKS]; /* potential bad disk blks in buffer 
				      * that need verification.           */
	paddr_t	hdb_pmaddr[MAXNBLKS];/* Physical memory address for transfer
				      * of blocks that need verification. */
	daddr_t hdb_bad2map[2];	     /* bad block that needs to be mapped */
	short   hdb_nbadblk;	     /* no. of bad blks to handle         */
	short   hdb_nb2vfy;	     /* no. of blks to verify in this buf.*/
	short	hdb_vfyflg;	     /* indicates verification is needed. */
	short   hdb_eccknt;	     /* times ECC used during vfy. of blk.*/
	short   hdb_rtycnt;	     /* times blk retied during verf.     */
	short   hdb_ndx;	     /* index of hdb_b2vfy[], indicates
				      * which disk blk is being verified. */
	ushort	hdb_mapflg;	     /* indicates block is being mapped.  */
	ushort	hdb_mblktyp;	     /* type of mrg. block to be mapped.  */
	ushort	hdb_bblktyp;	     /* type of bad block to be mapped.   */
	int	hdb_bpflags;	     /* place to save value of b_flags.   */
	ushort  hdb_badflg;	     /* indicates type of bad blk detected*/
	ushort	hdb_bpresid;	     /* place to save value of bp->b_resid.     */
	daddr_t	hdb_physblk;	     /* place to save value of hdcst.hd_physblk.*/
	paddr_t	hdb_addr;	     /* place to save value of hdcst.hd_addr.   */
	char    hdb_tmpbuf[TBUFFLEN];/* buf for io during the vfy. and/or
				      *  mapping of blk.                  */
}hdbad;

/*  values of hdb_mapflg */
#define	HDB_WSEC	0x01	/* operation is write sector to disk */
#define	HDB_WTBL	0x02	/* operation is write table to disk */

/* Types of block to be mapped */
#define	REGBLK		0
#define	UNASSIG		1
#define ASSIGND		2

/* Each of the following two bit maps represent all tracks on the UNIX partition.
 * A bit set means that at least one sector within the track has been
 * detected as bad.
 */
char *bbh_trkmap[NUMDRV];/* Bits are for tracks with at least one bad block.
			  * Initialized with info obtained from the "alternate
			  * sector table". bit map is later updated by BBH
			  * each time a sector is detected as bad.
			  */

char *bth_trkmap[NUMDRV];/* Bits are set for all tracks recorded in the "bad
			  * track table" and those recorded in the bbh_trkmap[].
			  */
unsigned prev_mapsize;	 /* mapsize to free if reallocating bbh maps */

/* Declaration for BBH to be move to hd.h ends here*/

/*
 * Values for calling hderror.
 */
#define RETRY 	1
#define NORETRY	0

#define MAXRETRY 10
#define NORMRETRIES  4

struct hdcstat {
	unchar  hd_active;	/* indicates a transfer in progress */
	unchar  hd_errcnt;	/* number of errors on this transfer */
	ushort  hd_curdrv;	/* unit doing the transfer */
	ushort  hd_nblks;	/* number of blocks being transferred */
	ulong   hd_nbytes;	/* number of bytes being transferred */
	daddr_t hd_physblk;	/* physical disk block */
	daddr_t hd_badblk;	/* bad physical block being mapped */
	daddr_t hd_afterbad;	/* first block after replacement track */
	daddr_t hd_resume;	/* blk no. to resume io after bad track */
	paddr_t hd_addr;	/* virtual memory address for transfer */
	int	hd_idto;	/* id returned by timeout function */
} hdcst;

/* information about each drive */
struct hddrvinfo {
	ushort	hd_state;
	ushort	hd_nparts;
	struct hdparams hd_geom;
#define hd_ncyls	hd_geom.hdp_ncyl
#define hd_nhds		hd_geom.hdp_nhead
#define hd_nsecs	hd_geom.hdp_nsect
#define hd_precomp	hd_geom.hdp_precomp
#define hd_lz		hd_geom.hdp_lz
	daddr_t	hd_unixstart;	/* first absolute sector of active partition */
	int	hd_begtrk;	/* first track of active partition */
	daddr_t hd_last_sacred;	/* last absolute sector of sacred area */
	daddr_t	hd_alts_loc;	/* first absolute sector of alternate table */
	ushort	hd_alts_len;	/* length of alt table in bytes */
	struct buf	*hd_latest;	/* latest buffer added to queue */
	int	hd_otyp[OTYPCNT]; /* open/close accounting information */
} hddrvinfo[NUMDRV];

/* information about each partition on each drive */
struct partition hdpartinfo[NUMDRV][V_NUMPAR];
struct partition hdwholedisk[NUMDRV];

/* array of flags indicating which partitions are being closed */
char hd_closing[NUMDRV][V_NUMPAR];

/* alternate sector information */
struct alt_info hdaltinfo[NUMDRV];

/* I/O statistics */
struct iotime hdstat[NUMDRV];

/* I/O buffers */
struct iobuf hdutab[NUMDRV];
/*
 * error messages 
 */
static char *ATerrmsg[] = {
	"Data address mark not found",
	"Track 0 not found",
	"Command aborted",
	"(bad 3)",
	"Sector not found",
	"(bad 5)",
	"Uncorrectable data read error",
	"Bad block flag detected",
};

/* Internal device for bypassing partitions */
#define ABSDEV(dev)	(BASEDEV(dev)|0x80)
#define ISABSDEV(dev)	((dev) & 0x80)

/*
 * Interleave table for V_FORMAT ioctl.  This is global so that it isn't on
 * the stack, and so we can sleep on it.
 */
static char itable[SECSIZE];
/*
 * Format or Verify requested:  The format and the verify ioctl set this 
 * to cause all other I/O
 * to drain; then format or verify get the controller and all other I/O blocks.
 */
static int fmtvfyreq = 0;
static char fmtvfywait;		/* value irrelevant; just for sleeping */

static time_t vfytime;		/* time that the verify command completed */
static ushort vfystatus;	/* status of error register after verify */

#define NO_RETRY	0x01	/* Controller flag to disable retries */

#ifdef	DEBUG
int hddebug = 0;
#endif

/* this is for rdagetblocks() to work with a buf header not from the pool */
static struct	buf	rdalts_buf[NUMDRV];

/* writefault will be used to signal write fault occured and caused retries */
/* if retries fail writefault causes panic rather than just failing operation*/
int writefault = 0;

/* defines for writefault situation */
#define ALTSTATUS 0x3f6
#define RESET 0x04
#define CLEAR 0x00

/* 4.0 style driver flag */
int hddevflag = 0; 

/* Declare hdtimeout to use in call to timeout in hdinit */
static void hdtimeout();

/*
 * The following are coded as inline assembler functions for speed.
 * The C equivalents are:
 *
 *	int
 *	table_scan(table, value, length)
 *		int	*table, value, length;
 *	{
 *		int	count;
 *		for (count = length; count-- > 0;) {
 *			if (*table++ == value)
 *				return length - count - 1;
 *		}
 *		return -1;
 *	}
 *
 *	int
 *	bit_test(bitmap, bitno)
 *		char	*bitmap;
 *		int	bitno;
 *	{
 *		return bitmap[bitno >> 3] & (1 << (bitno & 7));
 *	}
 *
 *	void
 *	bit_set(bitmap, bitno)
 *		char	*bitmap;
 *		int	bitno;
 *	{
 *		bitmap[bitno >> 3] |= (1 << (bitno & 7));
 *	}
 */

asm int table_scan(table, value, length)
{
%mem table,value,length; lab l;
	pushl	%edi
	movl	length, %ecx
	movl	%ecx, %edx
	movl	value, %eax
	movl	table, %edi
	repnz
	scasl
	movl	$-1, %eax
	jne	l
	movl	%edx, %eax
	subl	%ecx, %eax
	decl	%eax
l:
	popl	%edi
}

asm int bit_test(bitmap, bitno)
{
%mem bitmap,bitno;
	xorl	%eax, %eax
	movl	bitno, %ecx
	movl	bitmap, %edx
	btl	%ecx, (%edx)
	setc	%al
}

asm void bit_set(bitmap, bitno)
{
%mem bitmap,bitno;
	movl	bitno, %eax
	movl	bitmap, %edx
	btsl	%eax, (%edx)
%mem bitmap; reg bitno;
	movl	bitmap, %edx
	btsl	bitno, (%edx)
%reg bitmap,bitno;
	btsl	bitno, (bitmap)
}

struct cur_req {
	daddr_t physblk;
	paddr_t addr;
	unsigned int resid;
}cur_req;

int Hd_timeout = 0;


/*
 * Init routine to determine number of hard disk drives and types
 * and to set up the parameters accordingly.
 */
hdinit()
{
	register unsigned long	temp;
	register unsigned char	*pptr;

/*
 * If we have no hard disk drive, can't run unix.
 */
	if (bootinfo.hdparams[0].hdp_ncyl == 0) {
		cmn_err(CE_WARN, "No hard disk drive 1\n");
		hddrvinfo[0].hd_ncyls = 0;
		return(ENXIO);
	}

	hdutab[0].b_edev = makedevice(0,0);
	hdutab[0].io_stp = &hdstat[0].ios;
	hdutab[1].b_edev = makedevice(1,0);
	hdutab[1].io_stp = &hdstat[1].ios;
/*
 * Set drive 1 parameters
 */
	hdcst.hd_curdrv = 0;
	hddrvinfo[0].hd_geom = bootinfo.hdparams[0];
	hdsetcont(&hddrvinfo[0], 0);

	hdcst.hd_idto = timeout( hdtimeout, 0, (30 * HZ) );

/*
 * Get parameters for second hard disk if there is one.
 */
	if (bootinfo.hdparams[1].hdp_ncyl == 0)
		return(0);
	hdcst.hd_curdrv = 1;
	hddrvinfo[1].hd_geom = bootinfo.hdparams[1];

#if 0	/* Should we really do this? */
/*
 * check drive 2 parameters, this shouldnt be necessary but some
 * ROMs have a bug and the pointer we got was bad.  If the values are
 * bad then set some reasonable default to allow us to access the
 * pdinfo block with the real values for this drive.
 */
	if (hddrvinfo[1].hd_nhds < 2)
		hddrvinfo[1].hd_nhds = 2;
	if (hddrvinfo[1].hd_nsecs < 17)
		hddrvinfo[1].hd_nsecs = 17;
	if (hddrvinfo[1].hd_ncyls < 1)
		hddrvinfo[1].hd_ncyls = 1;
#endif
/*
 * Set drive 2 parameters
 */
	hdsetcont(&hddrvinfo[1], 1);
	return(0);
}


/*
 * program controller for given drive and characteristics.
 */
hdsetcont(hdip, drive)
register struct hddrvinfo	*hdip;
int				drive;
{
	register unsigned char	nheads;
	register int		oldpri;

	/*
	 * Wait until no I/O is happening, to do the set-parameters.
	 * hdstart will wake us up when there are no I/O requests
	 * in any drive's queue.
	 */
	fmtvfyreq = 1;
	oldpri = spl7();
	while (hdcst.hd_active)
		sleep(&fmtvfywait, PRIBIO);

	nheads = HD_DHFIXED | (hdip->hd_nhds-1);
	nheads |= drive ? HD_DRIVE1 : HD_DRIVE0;
	outb(HD0+HD_DRV, nheads);
	outb(HD0+HD_NSECT, hdip->hd_nsecs);
	outb(HD0+HD_CMD, HD_SETPARAM);
	ATwait(); /* wait for command to complete */

	fmtvfyreq = 0;
	wakeup((char *)&fmtvfyreq);
	splx(oldpri);
	return(0);
}

/*
 * Get a block from the given device.
 *	This is used to read the pieces of the vtoc, etc.
 */
hdgetblock(bp, dev, blkno, count)
struct buf *bp;
dev_t dev;
daddr_t blkno;
unsigned count;
{
	bp->b_flags = B_READ;
	bp->b_blkno = blkno;
	bp->b_edev = dev;
	bp->b_bcount = count;
	hdstrategy(bp);
	iowait(bp);
	return(0);
}

/*
 * Write a block to the given device.
 */
hdputblock(bp, dev, blkno, count)
struct buf *bp;
dev_t dev;
daddr_t blkno;
unsigned count;
{
	bp->b_flags = 0;
	bp->b_blkno = blkno;
	bp->b_edev = dev;
	bp->b_bcount = count;
	hdstrategy(bp);
	iowait(bp);
	return(0);
}


/* Get 'n' blocks of data into the specified buffer from the given device.
 * Both the buf header and the body are NOT from the system buffer pool.
 *	This is used to read the alternate sector table,
 *	which may be 1-4 sectors long, determined by pdinfo (alt_len).
*/
rdagetblocks(rda, dev, blkno, count, buffer)
struct buf	*rda;
dev_t	dev;
daddr_t	blkno;
unsigned	count;
char	*buffer;
{
	int x = spl5();

	while(rda->b_flags & B_BUSY) {
		rda->b_flags |= B_WANTED;
		sleep((caddr_t)rda, PRIBIO);
	}
	rda->b_flags = B_READ|B_BUSY;
	rda->b_blkno = blkno;
	rda->b_edev = dev;
	rda->b_bcount = count;
	rda->b_un.b_addr = (caddr_t)buffer;

	splx(x);

	hdstrategy(rda);
	iowait(rda);
	return(0);
}


/* Release buf header used by rdagetblocks().
 *	Just resets flags and gives someone a wakeup call, if needed.
*/
rdarelease(rda)
struct buf	*rda;
{
	rda->b_flags &= ~(B_BUSY | B_READ);
	if (rda->b_flags & B_WANTED)
	{
		wakeup((caddr_t)rda);
	}
	return(0);
}



/*
 * Open a unit.  First open reads physical description, vtoc, and alternate
 * sector table.
 */
hdopen(devp,flags,otyp,cred_p)
dev_t *devp;
int flags;
int otyp;
struct cred *cred_p;
{
	register struct buf *bp1 = NULL;	/* buffers for reading vtoc */
	register struct buf *bp2 = NULL;
	register daddr_t block;		/* logical block to read */
	register int n;			/* loop counter */
	struct hddrvinfo *hdi;		/* drive information for this unit */
	struct ipart *ipart;		/* fdisk table entry */
	struct pdinfo *pdptr;		/* physical description struct */
	struct vtoc *vptr;		/* pointer to VTOC from pdinfo */
	struct partition *hdp;		/* partition tables for this unit */
	struct alt_info *aptr;		/* alternate sectors/tracks tables */
	daddr_t pdsect;			/* sector containing pdinfo */
	unsigned unit;
	unsigned partition;
	unsigned offset;
	unsigned mapsize;
	daddr_t trk; 	/* changed for > 1024 cyl */
	char errflg = 0;
	daddr_t lsttrk;
	int rc=0;
	dev_t dev;

	dev = *devp;
	unit = UNIT(dev);
	partition = PARTITION(dev);
	hdi = &hddrvinfo[unit];
	hdp = hdpartinfo[unit];

	/* Check for non-existent drive/partition */
	if ((unit >= NUMDRV) || (hdi->hd_ncyls == 0) || (partition >= V_NUMPAR)) 
		return(ENXIO);

	/* Don't allow absolute partition to be used externally */
	if (ISABSDEV(dev)) 
		return(ENXIO);

	/* Sleep if someone else already opening */
	while (hdi->hd_state & HD_OPENING)
		sleep((char *)&hdi->hd_state, PRIBIO);

	if (hdi->hd_state & HD_OPEN) {
		/* If partition is not valid, fail the open. */
		if (!(hdp[partition].p_flag & V_VALID))
			errflg++;
	} else {
		hdi->hd_state |= HD_OPENING | HD_DO_RST;
		/*
		 * Set up an absolute partition, used internally to refer
		 * to the whole disk.
		 */
		hdwholedisk[unit].p_tag = V_BACKUP;
		hdwholedisk[unit].p_flag = V_UNMNT | V_VALID;
		hdwholedisk[unit].p_start = 0;
		hdwholedisk[unit].p_size = (long)hdi->hd_ncyls *
						hdi->hd_nhds * hdi->hd_nsecs;
		/*
		 * Temporarily set up partition 0 to be the whole disk,
		 * in case we can't read the FDISK table.
		 */
		hdp[0] = hdwholedisk[unit];
		hdi->hd_nparts = 1;

		/*
		 * Get the FDISK table.
		 *	It is at absolute sector 0 on the disk.
		 */
		bp1 = geteblk();
		hdgetblock(bp1, ABSDEV(dev), (daddr_t)0, SECSIZE);
		if (bp1->b_flags & B_ERROR) {
			cmn_err(CE_WARN, "Cannot read sector 0 on device 0x%x\n", dev);
			errflg++;
			goto done;
		}
		if (((struct mboot *)bp1->b_un.b_addr)->signature != MBB_MAGIC) {
			cmn_err(CE_WARN,"!Invalid sector 0 on device 0x%x\n",dev);
			errflg++;
			goto done;
		}
		ipart = (struct ipart *)((struct mboot *)bp1->b_un.b_addr)->parts;
		/*
		 * Find active FDISK partition.
		 */
		for (n = FD_NUMPART; ipart->bootid != ACTIVE; ipart++) {
			if (--n == 0) {
				cmn_err(CE_WARN, "!Can't find active partition on device 0x%x\n", dev);
				errflg++;
				goto done;
			}
		}
		if (ipart->systid != UNIXOS) {
			cmn_err(CE_WARN, "!Active partition on device 0x%x is not a UNIX System partition\n", dev);
			errflg++;
			goto done;
		}
		hdi->hd_unixstart = ipart->relsect;
		hdi->hd_begtrk = hdi->hd_unixstart / (daddr_t)hdi->hd_nsecs;
		/*
		 * Make partition 0 the whole UNIX partition.
		 */
		hdp[0].p_start = hdi->hd_unixstart;
		hdp[0].p_size = ipart->numsect;

		/*
		 * Get the PDINFO.
		 * 	It is in a known location on the disk.
		 */
		pdsect = hdi->hd_unixstart + HDPDLOC;
		hdgetblock(bp1, ABSDEV(dev), pdsect, SECSIZE);
		if (bp1->b_flags & B_ERROR) {
			cmn_err(CE_WARN, "Cannot read pdinfo on device 0x%x\n", dev);
			errflg++;
			goto done;
		}

		pdptr = (struct pdinfo *)bp1->b_un.b_addr;

		if (pdptr->sanity != VALID_PD) {
			cmn_err(CE_WARN,"!Invalid pdinfo on device 0x%x\n",dev);
			errflg++;
			goto done;
		}
		if ((pdptr->cyls != hdi->hd_ncyls) ||
		    (pdptr->tracks != hdi->hd_nhds) ||
		    (pdptr->sectors != hdi->hd_nsecs) ||
		    (pdptr->bytes != SECSIZE)) {
#ifdef DEBUG
			if (hddebug)
				cmn_err(CE_NOTE, "Pdinfo doesn't match parameters for device 0x%x", dev);
#endif
		    	if (pdptr->bytes != SECSIZE) {
				cmn_err(CE_WARN, "Unable to change hard disk sector size.");
				errflg++;
				goto done;
			}
			hdi->hd_ncyls = pdptr->cyls;
			hdi->hd_nhds = pdptr->tracks;
			hdi->hd_nsecs = pdptr->sectors;
			hdsetcont(hdi, unit);
		}
		/*
		 * Get the VTOC.
		 */
		block = hdi->hd_unixstart + (pdptr->vtoc_ptr >> SECSHFT);
		offset = pdptr->vtoc_ptr & SECMASK;
		if (block != pdsect) {
			/* vtoc is in a different sector. */
			bp2 = geteblk();
			hdgetblock(bp2, ABSDEV(dev), block, SECSIZE);
			if (bp2->b_flags & B_ERROR) {
				cmn_err(CE_WARN, "Cannot read vtoc on device 0x%x\n", dev);
				errflg++;
				goto done;
			}
			vptr = (struct vtoc *)(bp2->b_un.b_addr + offset);
		} else
			vptr = (struct vtoc *)(bp1->b_un.b_addr + offset);

		if (vptr->v_sanity != VTOC_SANE) {
			cmn_err(CE_WARN, "Vtoc invalid on device 0x%x\n", dev);
			errflg++;
			goto done;
		}
		/*
		 * Make sure partition 0 is correct.
		 */
		if (vptr->v_part[0].p_start != hdp[0].p_start
				|| vptr->v_part[0].p_size != hdp[0].p_size) {
			cmn_err(CE_WARN, "!Incorrect partition 0 on device 0x%x\n", dev);
			errflg++;
			goto done;
		}
		/*
		 * VTOC is OK, so copy partition information to hdpartinfo.
		 */
		hdi->hd_nparts = vptr->v_nparts;

		/* Copy partition information. */
		for (n = 1; n < (int)vptr->v_nparts; n++) {
			hdp[n] = vptr->v_part[n];  /* structure copy */
			hdp[n].p_flag &= ~V_OPEN;
		}

		/* Clear remaining partitions. */
		for ( ; n < V_NUMPAR; n++) {
			hdp[n].p_flag = 0;
			hdp[n].p_size = 0;
		}
		/*
		 * Get alternates information.
		 */
		block = hdi->hd_unixstart + (pdptr->alt_ptr >> SECSHFT);
		offset = pdptr->alt_ptr & SECMASK;
		/* n <= # sectors needed for alt table */
		n = (offset + pdptr->alt_len + SECMASK) >> SECSHFT;
		if (block != pdsect || n > 1) {
			/* Alternates table is in a different sector. */
			register struct buf	*rda = &rdalts_buf[unit];

			/* read alternate sector/track tables directly into its
			 *	structure for this device.
			*/
			rdagetblocks(rda, ABSDEV(dev), block,
				     n << SECSHFT, (caddr_t)&hdaltinfo[unit]);
		
			if (rda->b_flags & B_ERROR) {
				cmn_err(CE_WARN, "Cannot read alternates table on device 0x%x\n", dev);
				rdarelease(rda);
				errflg++;
				goto done;
			}

			aptr = (struct alt_info *)rda->b_un.b_addr;
			rdarelease(rda);

		} else {	/* alts table is remainder of vtoc block */
			aptr = (struct alt_info *)(bp1->b_un.b_addr + offset);
			hdaltinfo[unit] = *aptr;  /* structure copy */
		}

		if (aptr->alt_sanity != ALT_SANITY   ||
		    aptr->alt_version != ALT_VERSION ||
		    aptr->alt_trk.alt_used > aptr->alt_trk.alt_reserved ||
		    aptr->alt_sec.alt_used > aptr->alt_sec.alt_reserved    )
		{
			cmn_err(CE_WARN, "Alternates table invalid on device 0x%x\n", dev);
			errflg++;
			goto done;
		}

		hdi->hd_alts_loc = block;
		hdi->hd_last_sacred = hdi->hd_alts_loc + n - 1;
		hdi->hd_alts_len = pdptr->alt_len;

#ifdef BBH_DEBUG
cmn_err(CE_WARN, "\nhdopen(): altloc= :%d: last_sacred= :%d: alts_len= :%d: \
alt_reserved= :%d: alt_base= :%d:", hdi->hd_alts_loc, hdi->hd_last_sacred, 
hdi->hd_alts_len, aptr->alt_sec.alt_reserved, aptr->alt_sec.alt_base);
#endif

		mapsize = ((long)(hdp[0].p_size + hdi->hd_nsecs - 1) / (long)hdi->hd_nsecs + 7) / 8;
		/* MUST allocate space dynamically for bit maps. */
		/* if reopening disk free up current BBH maps */
		if (bbh_trkmap[unit] != NULL) {
			kmem_free(bbh_trkmap[unit],prev_mapsize);
			bbh_trkmap[unit] = NULL;
		}
		if (bth_trkmap[unit] != NULL) {
			kmem_free(bth_trkmap[unit],prev_mapsize);
			bth_trkmap[unit] = NULL;
		}

		if ( (bth_trkmap[unit] = (char *)kmem_zalloc(mapsize,KM_SLEEP)) == NULL) {
			cmn_err(CE_WARN, "Failed to allocate memory for device 0x%x\n",dev);
			errflg++;
		}
		if ( (bbh_trkmap[unit] = (char *)kmem_zalloc(mapsize,KM_SLEEP)) == NULL) {
			cmn_err(CE_WARN, "Failed to allocate memory for device 0x%x\n",dev);
			errflg++;
		}
		prev_mapsize = mapsize; /* saving size of bbh maps */
		lsttrk = (long)(hdp[0].p_start + hdp[0].p_size -1)/(long)hdi->hd_nsecs;

		/* Initialize bit maps */
		for (n=0; n < (int)aptr->alt_trk.alt_used; n++)
		{
			if( aptr->alt_trk.alt_bad[n] != -1)
			{
				/* determine if track is within legal range */
				if( aptr->alt_trk.alt_bad[n] > lsttrk ||
				    aptr->alt_trk.alt_bad[n] < hdi->hd_begtrk )
				{
					errflg++;
					if (bbh_trkmap[unit] != NULL) {
						kmem_free(bbh_trkmap[unit],mapsize);
						bbh_trkmap[unit] = NULL;
					}
					if (bth_trkmap[unit] != NULL) {
						kmem_free(bth_trkmap[unit],mapsize);
						bth_trkmap[unit] = NULL;
					}
					goto done;
				}
				bit_set(bth_trkmap[unit],
					aptr->alt_trk.alt_bad[n]-hdi->hd_begtrk);
			}

		}

		for (n=0; n < (int)aptr->alt_sec.alt_used; n++)
		{
			if( aptr->alt_sec.alt_bad[n] != -1)
			{
				trk = aptr->alt_sec.alt_bad[n] / (daddr_t)hdi->hd_nsecs;
				/* determine if track is within legal range */
				if( trk > lsttrk || trk < hdi->hd_begtrk )
				{
					errflg++;
					if (bbh_trkmap[unit] != NULL) {
						kmem_free(bbh_trkmap[unit],mapsize);
						bbh_trkmap[unit] = NULL;
					}
					if (bth_trkmap[unit] != NULL) {
						kmem_free(bth_trkmap[unit],mapsize);
						bth_trkmap[unit] = NULL;
					}
					goto done;
				}
				bit_set(bbh_trkmap[unit], trk - hdi->hd_begtrk);
				bit_set(bth_trkmap[unit], trk - hdi->hd_begtrk);
			}
		}

		hdi->hd_state &= ~HD_OPENING;
		hdi->hd_state |= HD_OPEN;

		/* 
		 * Make sure partition is valid. 
		 */
		if (!(hdp[partition].p_flag & V_VALID)) {
			errflg++;
			goto done;
		}
		/*
		 * If opening root device, see if swap device is on this unit.
		 * If so, get the swapdev partition size and set nswap.
		 */
		if ((dev == rootdev) && (swapdev != NODEV) && (unit == UNIT(swapdev)))
			nswap = hdp[PARTITION(swapdev)].p_size;
	}
done:
	if (errflg) {
		if (hdi->hd_state & HD_OPENING) {
			hdi->hd_state &= ~HD_OPENING;
			for (n = 0; n < V_NUMPAR; n++)
				hdp[n].p_flag &= ~V_VALID;
		}
		hdi->hd_state = 0;

		/* Allow open on partition 0 to fail so that format works. */
		if (partition == 0)
			rc = 0;
		else
			rc = ENXIO;
	} else {
                if ((hdp[partition].p_flag & V_RONLY) && (flags & FWRITE)
                     && (otyp == OTYP_MNT))
                        rc = EROFS;
	}
 
	if (rc == 0) {
		if (otyp == OTYP_LYR)
			++hdi->hd_otyp[OTYP_LYR];
		else if (otyp < OTYPCNT)
			hdi->hd_otyp[otyp] |= (1 << partition);
	}

	wakeup((char *)&hdi->hd_state);
	if (bp1 != NULL)
		brelse(bp1);
	if (bp2 != NULL)
		brelse(bp2);

	/* Let concurrent close drain any pending I/O before reopening. */
	while (hd_closing[unit][partition] != 0)
		sleep(&hd_closing[unit][partition], PRIBIO);

	return(rc);
}

/*
 * Close a device, allowing pending I/O to complete.
 */
hdclose(dev,flag,otyp,cred_p)
dev_t dev;
int flag;
int otyp;
struct cred *cred_p;
{
	register struct buf *bp;
	struct iobuf *hdu;
	struct hddrvinfo *hdi;		/* drive information for this unit */
	unsigned unit;
	unsigned partition;
	int oldpri;
	struct partition *hdp;		/* partition tables for this unit */
	unsigned mapsize;

	unit = UNIT(dev);
	partition = PARTITION(dev);
	hdu = &hdutab[unit];
	hdi = &hddrvinfo[unit];
	hdp = hdpartinfo[unit];

	/* Enforce mutual exclusion with hdopen. */
	hd_closing[unit][partition] = 1;

	/*
	 * Raise priority and then clear out all pending I/O.
	 */
	oldpri = spl5();
	bp = hdu->b_actf;
	while (bp != NULL)
		if (bp->b_edev == dev) {
			sleep((char *)&hdu->b_actf, PRIBIO);
			bp = hdu->b_actf;
		} else
			bp = bp->av_forw;

	splx(oldpri);
	hd_closing[unit][partition] = 0;
	wakeup(&hd_closing[unit][partition]);
	if (otyp == OTYP_LYR)
		--hdi->hd_otyp[OTYP_LYR];
	else if (otyp < OTYPCNT)
		hdi->hd_otyp[otyp] &= ~(1 << partition);

	for (otyp = 0; otyp < OTYPCNT; otyp++) {
		if (hdi->hd_otyp[otyp] != 0)
			break;
	}
	if (otyp == OTYPCNT) {	/* Last close for this unit */
		mapsize = ((long)(hdp[0].p_size + hdi->hd_nsecs - 1) / (long)hdi->hd_nsecs + 7) / 8;
		hdi->hd_state = 0;
		if (bbh_trkmap[unit] != NULL) {
			kmem_free(bbh_trkmap[unit],mapsize);
			bbh_trkmap[unit] = NULL;
		}
		if (bth_trkmap[unit] != NULL) {
			kmem_free(bth_trkmap[unit],mapsize);
			bth_trkmap[unit] = NULL;
		}
	}
	return(0);
}


/*
 * Queue an I/O request, and start if not busy.
 */
hdstrategy(bp)
register struct buf *bp;
{
	register struct partition *hdp;
	register struct hddrvinfo *hdi;
	register struct iotime *hdit;
	register struct buf *curbp, *nextbp;
	struct iobuf *hdu;
	int oldpri;
	unsigned unit;

#ifdef DEBUG
	if (hddebug)
		cmn_err(CE_NOTE,"hdstrategy(0x%lx)\n", bp->b_blkno);
#endif
	/* If a format is waiting, block all other I/O until it's done */
	while (fmtvfyreq)
		sleep((char *)&fmtvfyreq, PRIBIO);
	/* If the requested count to be transferred is zero, we're done. */
	if (bp->b_bcount == 0) {
		iodone(bp);
		return(0);
	}

	unit = UNIT(bp->b_edev);
	hdi = &hddrvinfo[unit];
	if (ISABSDEV(bp->b_edev))
		hdp = &hdwholedisk[unit];
	else
		hdp = &hdpartinfo[unit][PARTITION(bp->b_edev)];

	/*
	 * Check if partition is valid, and for trying to write to a 
	 * read-only partition if not root.
	 */
	if (!(hdp->p_flag & V_VALID) ||
	    !(bp->b_flags & B_READ) && (hdp->p_flag & V_RONLY) && drv_priv(u.u_cred)) { 
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return(0);
	}
	
	if (bp->b_blkno < 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return(0);
	}

	if (bp->b_blkno >= hdp->p_size) {
		if (bp->b_blkno > hdp->p_size || !(bp->b_flags & B_READ)) {
			/* if request is off the end or we're not reading */
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}

		/* reading last block is allowed: it indicates EOF */
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return(0);
	}

	/* must not exceed maximum transfer allowed by the controller */
	if (bp->b_bcount > MAXXFER << SCTRSHFT ) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return(0);
	}

	/* Calculate the physical cylinder number of the request. */
	*((ushort *)&bp->cylin) = (bp->b_blkno + hdp->p_start) /
	    (daddr_t)(hdi->hd_nsecs * hdi->hd_nhds);
	bp->av_forw = NULL;
	drv_getparm(LBOLT, &bp->b_start);  /* time in 1/60 sec. since boot */

	/* Update I/O count statistics */
	hdit = &hdstat[unit];
	hdit->io_cnt++;
	hdit->io_bcnt += (bp->b_bcount + NBPSCTR - 1) >> SCTRSHFT;

	/* Put the buffer onto the queue using an elevator algorithm. */
	hdu = &hdutab[unit];
	oldpri = spl5();
	if (hdu->b_actf == NULL)
		hdu->b_actf = bp;
	else if (hdi->hd_latest &&
			(ushort)hdi->hd_latest->cylin == (ushort)bp->cylin) {
		bp->av_forw = hdi->hd_latest->av_forw;
		hdi->hd_latest->av_forw = bp;
	} else {
		int	s1, s2;

		for (curbp = hdu->b_actf; nextbp = curbp->av_forw; curbp = nextbp) {
			if ((s1 = (ushort)curbp->cylin - (ushort)bp->cylin)<0)
				s1 = -s1;
			if ((s2 = (ushort)curbp->cylin - (ushort)nextbp->cylin)<0)
				s2 = -s2;
			if (s1 < s2)
				break;
		}
		bp->av_forw = nextbp;
		curbp->av_forw = bp;
	}
	hdi->hd_latest = bp;

	/* If no requests are in progress, start this one. */
	if (hdcst.hd_active == 0)
		hdstart();
	splx(oldpri);
	return(0);
}


/*
 * Use pio_breakup to do the I/O in large (256 sector) chunks.
 */
void
hdbreakup(bp)
struct buf *bp;
{
#ifdef ASYNCIO
	if ( bp->b_flags & B_RAIO )
		raio_breakup(hdstrategy, bp, MAXNBLKS);
	else
#endif
		pio_breakup(hdstrategy, bp, MAXNBLKS);
}

/*
 * Raw device read.
 */
hdread(dev, uio_p, cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	int error;
	error=physiock(hdbreakup, NULL, dev, B_READ,(daddr_t)(hdpartinfo[UNIT(dev)][PARTITION(dev)].p_size), uio_p);
	return(error);
}


/*
 * Raw device write.
 */
hdwrite(dev, uio_p, cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	int error;
	error=physiock(hdbreakup, NULL, dev, B_WRITE,(daddr_t)(hdpartinfo[UNIT(dev)][PARTITION(dev)].p_size), uio_p);
	return(error);
}


/*
 * Start an I/O request from the queue.
 */
hdstart()
{
	register struct buf *bp;
	register struct partition *hdp;
	register loopcnt;
	register ushort curdrv;

	/* Check all drives for some queued I/O. */
	curdrv = hdcst.hd_curdrv;
#if NUMDRV != 2			/* general case */
	loopcnt = NUMDRV;
	while (loopcnt--) {
		if (++hdcst.hd_curdrv >= NUMDRV)
			hdcst.hd_curdrv = 0;
		if ((bp = hdutab[hdcst.hd_curdrv].b_actf) != NULL)
			break;
	}
#else			/* optimization for 2-drive case */
	if ((bp = hdutab[1 - curdrv].b_actf) != NULL)
		hdcst.hd_curdrv = 1 - curdrv;
	else
		bp = hdutab[curdrv].b_actf;
#endif
	if (bp == NULL) {
		hdcst.hd_active = 0;
		/* Nothing is queued; check for format */
		if (fmtvfyreq)
			wakeup(&fmtvfywait);
		return(0);
	}
	hdcst.hd_active = 1;

	/* If changing drives, set flag in previous drive. */
	if (hdcst.hd_curdrv != curdrv)
		hddrvinfo[curdrv].hd_state |= HD_FMT_RST;
		
	if (ISABSDEV(bp->b_edev))
		hdp = &hdwholedisk[UNIT(bp->b_edev)];
	else
		hdp = &hdpartinfo[UNIT(bp->b_edev)][PARTITION(bp->b_edev)];

	/* b_resid gets the number of full sectors being transferred. */
	hdcst.hd_nbytes = bp->b_bcount;
	bp->b_resid = ((unsigned)hdcst.hd_nbytes + SECMASK) >> SECSHFT;

	/* Transfer as much as will fit in the partition. */
	if (bp->b_blkno + bp->b_resid > hdp->p_size) {
		bp->b_resid = hdp->p_size - bp->b_blkno;
		hdcst.hd_nbytes = bp->b_resid << SECSHFT;
	}

	hdcst.hd_addr = paddr(bp);
	hdcst.hd_physblk = hdp->p_start + bp->b_blkno;
	hdio(bp);
	return(0);
}


/*
 * Do bad block mapping and start the transfer(s).
 */
hdio(bp)
register struct buf *bp;
{
	register struct  alt_table *secptr;
	register struct  alt_table *trkptr;
	register daddr_t firstsec;
	register daddr_t sec, lastsec;
	register short trk, lastrk;
	register short ndx;
	unsigned unit;
	struct hddrvinfo *hdi;

	unit = UNIT(bp->b_edev);
	hdi = &hddrvinfo[unit];
	/* Set number of blocks to transfer. */
	if (hdi->hd_state & HD_BADTRK)
		hdcst.hd_nblks = hdcst.hd_afterbad - hdcst.hd_physblk;
	else
		hdcst.hd_nblks = bp->b_resid;
restart:
	firstsec = hdcst.hd_physblk;
	sec = firstsec;
	trk = firstsec / (daddr_t)hdi->hd_nsecs;
	lastrk = (daddr_t)(firstsec + hdcst.hd_nblks -1) / (daddr_t)hdi->hd_nsecs;

	/* Map bad blocks except on partition 0, which is the whole UNIX partition. */
	if ((PARTITION(bp->b_edev) != 0) && (hdb_inunix(bp)))
	{
		secptr = &hdaltinfo[unit].alt_sec;
		trkptr = &hdaltinfo[unit].alt_trk;
		while (trk <= lastrk)
		{
			lastsec = ( lastrk == trk ? firstsec + hdcst.hd_nblks -1
				  : (trk +1) * hdi->hd_nsecs -1 );

			/* determine if sector is potentially bad */
			if ( bit_test(bth_trkmap[unit], trk - hdi->hd_begtrk) )
			{
				/* sector is potentially bad, find out if it is
				 * in the "alternate sector table".
				 */
				if ( bit_test(bbh_trkmap[unit], trk - hdi->hd_begtrk) )
				{
					while (sec <= lastsec)
					{
						ndx = table_scan(secptr->alt_bad,sec,
								 secptr->alt_used);
						if (ndx != -1)
						{
						    if (firstsec == sec)
						    {
							hdi->hd_state |= HD_BADBLK;
							hdcst.hd_badblk=firstsec;
							hdcst.hd_physblk=ndx +
								  secptr->alt_base;
							hdcst.hd_nblks = 1;
							/*xfer rmapped bad blk alone*/
#ifdef BBH_DEBUG
	cmn_err(CE_NOTE, "HD: drive :%d: mapping block :%ld: --> :%ld:",
		unit, firstsec, hdcst.hd_physblk);
#endif
							goto altblk_xfer;
						    }
						    else
						    {
							hdcst.hd_nblks = sec-firstsec;
							/*break io req up to bad blk*/
#ifdef BBH_DEBUG
    cmn_err(CE_NOTE, "HD: drive :%d: found badblk= :%ld:  brkng IO req. first= :%ld:  nblocks= :%d:",
		unit, sec, hdcst.hd_physblk, hdcst.hd_nblks);
#endif
							goto altblk_xfer;
						    }
						}
						else
						   ++sec;
					}
				}
				/* bit not set in bbh_trkmap, block must be in
				 * a track recorded in the "alternate track table".
				 */ 
				else 
				{
					if (firstsec == sec)
					{
						hdi->hd_state |= HD_BADTRK;
						ndx = table_scan(trkptr->alt_bad,trk,
								 trkptr->alt_used);
						ASSERT(ndx!=-1);
						hdcst.hd_physblk= trkptr->alt_base +
							     (ndx * hdi->hd_nsecs) +
							     (sec % (daddr_t)hdi->hd_nsecs);
						hdcst.hd_nblks = lastsec - sec +1;
						hdcst.hd_afterbad = hdcst.hd_physblk+
								    hdcst.hd_nblks;
						hdcst.hd_resume = firstsec +
								  hdcst.hd_nblks;
#ifdef BBH_DEBUG
    cmn_err(CE_NOTE, "HD: drive :%d: Blk on badtrk= :%d:  first= :%ld: last= :%ld:",
		unit, trk, firstsec, lastsec);
#endif

						goto restart;
					}
					else
					{
						hdcst.hd_nblks = sec - firstsec;
#ifdef BBH_DEBUG
    cmn_err(CE_NOTE, "HD: drive :%d: found badblk= :%ld: on badtrk= :%d:  brkng IO req. first= :%ld: nblks= :%d:",
		unit, sec, trk, hdcst.hd_physblk, hdcst.hd_nblks);
#endif
						break;
					}
				}
			}
			sec = lastsec +1;
			++trk;
		}
	}
altblk_xfer:
	hdxfer(bp);
	return(0);
}


/* 
 * Called when starting initial transfer, and after an error.
 */
hdxfer(bp)
register struct buf *bp;
{
	register struct iobuf *hdu;
	register unit;
	struct hddrvinfo *hdi;

	unit = hdcst.hd_curdrv;
	hdu = &hdutab[unit];

	/* If this iobuf isn't active, mark it so and record start time. */
	if (hdu->b_active == 0) {
		hdu->b_active++;
		drv_getparm(LBOLT, &hdu->io_start);
	}

	hdi = &hddrvinfo[unit];

	/*
	 * If we want to do a restore (after some errors have occurred),
	 * give the controller the command and return.  hdintr will 
	 * start up the real transfer when the interrupt indicating
	 * restore complete comes in.
	 */
	if (hdi->hd_state & HD_DO_RST) {
		ATxcmd(unit);
		/*
		 * Clear the flag that says we have switched drives since
		 * this drive was last restored.
		 */
		hdi->hd_state &= ~HD_FMT_RST;
		return(0);
	}

	/***
	** Save job information so that we can restart the job
	** if the controller/drive get hung during this job.
	** This can happen in the WD1007 Controllers due to a
	** bug in the firmware.
	***/
	cur_req.physblk = hdcst.hd_physblk;
	cur_req.addr = hdcst.hd_addr;
	cur_req.resid = bp->b_resid;

	ATiocmd(unit, bp->b_flags&B_READ? HD_RDSEC : HD_WRSEC);
	return(0);
}


/*
 * Process interrupts.  There are three cases:
 *	1) Restore has completed.  Retry if there was an error, otherwise do
 *	   the transfer.
 *	2) Format has completed.  No error reporting is supported.
 *	3) Read/write has completed.  Check for errors and retry command if
 *	   error wasn't write fault.  Issue a notice if an error was corrected
 *	   by the ECC algorithm, which may indicate that that sector should be
 *	   mapped out.  If no errors, continue a multi-sector command or a 
 *	   command involving bad block mapping, or if we're finished call 
 *	   hddone.
 */
hdintr()
{
	register struct buf *bp;
	register struct hddrvinfo *hdi;
	int  statlow, stathigh, status;
	int  errval, altstatus, errcnt;
	int  track, i;
	daddr_t block;
	ushort  curdrv, blktyp;
	unsigned char	nheads;

	bp = hdutab[hdcst.hd_curdrv].b_actf;
	hdi = &hddrvinfo[hdcst.hd_curdrv];
	status = ATstatus(hdcst.hd_curdrv);


	if (!(hdi->hd_state & (HD_DO_RST|HD_DO_FMT|HD_DO_VFY))) {
		/* Read or write command has completed */
		if (bp == NULL) {
			cmn_err(CE_NOTE, "HD: interrupt with no request queued");
			return(0);
		}
		if (status & WRFAULT) {
			cmn_err(CE_NOTE, "!HD controller: write fault\n");
			writefault = 1;
			errval = inb(HD0 + HD_ERROR);
			errcnt = inb(HD0 + HD_NSECT);
			altstatus = inb(ALTSTATUS);
			if (hdi->hd_nsecs > 17) { /*do passthru for ESDI ONLY*/
				outb(HD0 + HD_LCYL, 0x00);
				outb(HD0 + HD_HCYL, 0x20);
				outb(HD0 + HD_CMD, 0xe0);
				ATwait();
				statlow = inb(HD0 + HD_LCYL);
				stathigh = inb(HD0 + HD_HCYL);
				cmn_err(CE_NOTE,"!LOW STAT=0x%x HIGH STAT=0x%x \n",statlow, stathigh);
			}
			/* Attempt to reset controller */
			outb(FDR, RESET);
			tenmicrosec();
			outb(FDR, CLEAR);
			/*
			 * Program controller for drive characteristics.  Cannot
			 * call hdsetcont() that sleeps for a quiescent state.
			 */
			nheads = HD_DHFIXED | (hdi->hd_nhds-1);
			nheads |= hdcst.hd_curdrv ? HD_DRIVE1 : HD_DRIVE0;
			outb(HD0+HD_DRV, nheads);
			outb(HD0+HD_NSECT, hdi->hd_nsecs);
			outb(HD0+HD_CMD, HD_SETPARAM);
			ATwait();

			/* Now start retries */
			hderror(bp, RETRY);
			return(0);
		}

		if (status & ERROR)
		{
			status = inb(HD0 + HD_ERROR);
			for (i = 0; (status & 0x01) == 0; status >>= 1, i++)
				; 
			errcnt = inb(HD0 + HD_NSECT);
			if (errcnt != hdcst.hd_nblks) {
				hdcst.hd_addr -= (errcnt - hdcst.hd_nblks) * SECSIZE;
				hdcst.hd_physblk -= (errcnt - hdcst.hd_nblks);
				bp->b_resid = hdcst.hd_nblks = errcnt;
			}

			/* if block not in UNIX space, BBH will
			 * not be done. */
			if (!hdb_inunix(bp)) {
				track = hdcst.hd_physblk / (daddr_t)hdi->hd_nsecs;
                                cmn_err(CE_NOTE,
                                "!HD DOS slice error: drive %d, cyl %d, head %d, sector %d: %s",
                		hdcst.hd_curdrv,track/(int)hdi->hd_nhds,track%(int)hdi->hd_nhds,
                                hdcst.hd_physblk%(daddr_t)hdi->hd_nsecs, ATerrmsg[i]);
				hderror(bp, RETRY);
				return(0);
			}
			/* I/O to slice for entire UNIX partition - no BBH */
			if ( PARTITION(bp->b_edev)==0 ) {
				hderrmsg(hdcst.hd_physblk, ATerrmsg[i]);
				hderror(bp, RETRY);
			}
			else if ( i == BADBLK ||
				((hdcst.hd_errcnt >= NORMRETRIES) &&
				( i == UNRDBL || i == INF || i == DNF )))
			{
				cmn_err(CE_WARN,hdb_msg[i],
 					hdcst.hd_physblk, hdcst.hd_curdrv);

				if( hdb_sacred(hdcst.hd_physblk) )
				{
				   if( hdcst.hd_errcnt >= MAXRETRY )
					cmn_err(CE_WARN,hdb_msg[BSACRD],
 					hdcst.hd_physblk, hdcst.hd_curdrv);
				   hderror(bp, RETRY);
				}
				else if ( hdb_nospar() )
				{
				   if( hdcst.hd_errcnt >= MAXRETRY )
					cmn_err(CE_WARN,hdb_msg[BNOSPR],
 					hdcst.hd_physblk, hdcst.hd_curdrv);
				   hderror(bp, RETRY);
				}
				else
				{
					hdbad.hdb_badflg = 1;
					hdbad.hdb_bad2map[hdbad.hdb_nbadblk++]=
								hdcst.hd_physblk;
					hdb_mapbad(bp);
				}
			}
			else
			{
				hderrmsg(hdcst.hd_physblk, ATerrmsg[i]);
				hderror(bp, RETRY);
			}
		}
		else /*if the controller cmd did not returned hard error */
		{
			if (status & ECC) /*if correctable error occurred*/
			{
			   /* if verification running to detect a marginal
			    * blk. or if handling a bad blk,
			    * do not need to prepare for verification.
			    */
			   if(!(hdi->hd_state & HD_BBH_VFY)&& !hdbad.hdb_badflg)
				{
			/* Warning message displayed ONLY if block is remapped
					cmn_err(CE_NOTE, hdb_msg[ECCMSG],
					hdcst.hd_physblk, hdcst.hd_curdrv); 
			*/

					/* if block not in UNIX space, BBH will
					 * not be done. */
					if ( ! hdb_inunix(bp) ||
					    PARTITION(bp->b_edev) == 0 )
						;
					else if( hdb_sacred(hdcst.hd_physblk) )
					   cmn_err(CE_WARN,hdb_msg[SACRED],
 					   hdcst.hd_physblk, hdcst.hd_curdrv);
					else if ( hdb_nospar() )
					   cmn_err(CE_WARN,hdb_msg[NOSPAR],
 					   hdcst.hd_physblk, hdcst.hd_curdrv);
					else
					{
					   /* indicate verification is
					    * needed after all disk blks
					    * of the buffer are processed
					    */
					   hdbad.hdb_vfyflg++;
					   /* remember which blocks need
					    * to be verified.
					    */
					   hdbad.hdb_b2vfy[hdbad.hdb_nb2vfy]=
								     hdcst.hd_physblk;
					   hdbad.hdb_pmaddr[hdbad.hdb_nb2vfy++]=
								        hdcst.hd_addr;
					}
				}
				else 
				{
					if ( hdb_inunix(bp) &&
					     PARTITION(bp->b_edev) != 0 )
						hdbad.hdb_eccknt++;
				}
			}

			if (bp->b_flags & B_READ)
				ATin(hdcst.hd_addr); /* fetch data */

			/* If resid == 0 we're finished with this buffer. */
			if (--bp->b_resid == 0)
			{
				if ( hdbad.hdb_vfyflg > 0 )
					hdi->hd_state |= HD_BBH_VFY;

				if (hdi->hd_state & HD_BBH_MAP )
				{
					if (hdbad.hdb_mapflg & HDB_WSEC)
					{
					   hdb_cleanup(bp);
					   if(hdbad.hdb_badflg != 0 )
					   {
					       block = 
					       hdbad.hdb_bad2map[hdbad.hdb_nbadblk-1];
					       blktyp = hdbad.hdb_bblktyp;
					   }
					   else
					   {
					   	block=hdbad.hdb_b2vfy[hdbad.hdb_ndx];
					   	blktyp = hdbad.hdb_mblktyp;
					   }
					   curdrv=hdcst.hd_curdrv;
					   hdb_updtbl(block,curdrv,blktyp);
					   hdb_wrttbl(bp);
					}
					else if (hdbad.hdb_mapflg & HDB_WTBL)
					{
					   hdb_cleanup(bp);
					   if( hdbad.hdb_badflg != 0)
					   {
						if(hdbad.hdb_nbadblk <= 0)
						{
							hdbad.hdb_badflg = 0;
							hdbad.hdb_nbadblk= 0;
							if (!(bp->b_flags & B_READ))
								hdio(bp);
							else
								hddone(bp);
						}
						else if ( hdb_nospar() ) {
							cmn_err(CE_WARN,hdb_msg[BNOSPR],hdcst.hd_physblk,hdcst.hd_curdrv);
							hderror(bp,NORETRY);
						}
						else
							hdb_mapbad(bp);
					   }
					   else
					   	hdb_contvfy(bp);
					}
				}
				else if ( hdi->hd_state & HD_BBH_VFY )
				{
					hdb_verify(bp);
				}

				else
					hddone(bp);
			}

			else {
				hdcst.hd_physblk++;
				hdcst.hd_addr += SECSIZE;
				/*
				 * Check if we have finished a portion of a
				 * command involving a bad block.  If so, start
				 * the I/O for the rest of the transfer, and
				 * restore the old value of physblk which was
				 * changed when the bad block was mapped.
				 */
				if (--hdcst.hd_nblks == 0  && ~Hd_timeout)
				{
					if (hdi->hd_state & HD_BADBLK)
					{
						hdcst.hd_physblk = hdcst.hd_badblk + 1;
						hdi->hd_state &= ~HD_BADBLK;
					}
					if ((hdi->hd_state & HD_BADTRK) && 
					    hdcst.hd_physblk == hdcst.hd_afterbad)
					{
						hdcst.hd_physblk = hdcst.hd_resume;
						hdi->hd_state &= ~HD_BADTRK;
					}
					hdio(bp);
				}
				else if (!(bp->b_flags & B_READ))
					ATout(hdcst.hd_addr); /* write data */
			}
		}
	}
	else if (hdi->hd_state & HD_DO_VFY)  /* Verify command completed */
	{
		drv_getparm(LBOLT, &vfytime);
		if (status & ERROR)
			vfystatus = inb(HD0+HD_ERROR);
		hdi->hd_state &= ~HD_DO_VFY;
		wakeup(itable);
	}
	else if (hdi->hd_state & HD_DO_RST)  /* Restore command has completed */
	{
		if (status & ERROR)
			hderror(bp, RETRY);
		else
		{
			hdi->hd_state &= ~HD_DO_RST;
			hdxfer(bp);
		}
	}
	else /* hdi->hd_state & HD_DO_FMT */  /* Format command completed */
	{
		hdi->hd_state &= ~HD_DO_FMT;
		wakeup(itable);
	}
	return(0);
}

hdioctl(dev, cmd, arg, mode, cred_p, rval_p)
dev_t dev;
register int cmd;
caddr_t arg;
int mode;
struct cred *cred_p;
int *rval_p;
{
	register struct hddrvinfo	*hdi;	/* info for this unit */
	register struct partition	*hdp;	/* info for current partition */
	unsigned int	unit;
	unsigned int	partition;
	union io_arg	karg;		/* kernel copy of arg */
	struct absio	absio;
	struct alt_info *ap;
	struct buf	*bp;
	daddr_t badblk, goodblk;
	short blktyp;

	unit = UNIT(dev);
	partition = PARTITION(dev);
	hdi = &hddrvinfo[unit];

	switch (cmd) {

	case GETALTTBL:
		ap = &hdaltinfo[unit];
		if (copyout((caddr_t)ap, arg, sizeof(struct alt_info))!=0)
			return(EFAULT);
		break;

	case V_CONFIG:		/* Change drive configuration parameters. */
		if (copyin(arg, (caddr_t)&karg, sizeof(karg)) != 0) {
			return(EFAULT);
		}
		if (karg.ia_cd.secsiz != SECSIZE) {
			/*
			 * Don't allow them to change sector size.
			 */
			return(EINVAL);
		}
		if (karg.ia_cd.ncyl < 1) {
			return(EINVAL);
		}

		hdi->hd_ncyls = karg.ia_cd.ncyl;
		hdi->hd_nhds = karg.ia_cd.nhead;
		hdi->hd_nsecs = karg.ia_cd.nsec;
		hdsetcont(hdi, unit);
		break;

	case V_REMOUNT: {	/* Force hdopen to reread vtoc. */
		register int	otyp;

		/* Make sure no partitions other than 0 are open. */
		for (otyp = 0; otyp < OTYPCNT; otyp++) {
			if (otyp != OTYP_CHR) {
				if (hdi->hd_otyp[otyp] != 0)
					break;
			} else if (hdi->hd_otyp[otyp] & ~1)
				break;
		}
		hdi->hd_state = 0;
		if (otyp != OTYPCNT) {	/* Something else open on this unit */
			return(ENXIO);
		}
		break;
		}
	case V_ADDBAD: 	/* Add a bad sector. */

			return(ENXIO);

	case V_GETPARMS: {	/* Get drive and partition parameters. */
		struct disk_parms	disk_parms;

		hdp = &hdpartinfo[unit][partition];

		disk_parms.dp_type = DPT_WINI;
		disk_parms.dp_heads = hdi->hd_nhds;
		disk_parms.dp_cyls = hdi->hd_ncyls;
		disk_parms.dp_sectors = hdi->hd_nsecs;
		disk_parms.dp_secsiz = SECSIZE;
		disk_parms.dp_ptag = hdp->p_tag;
		disk_parms.dp_pflag = hdp->p_flag;
		disk_parms.dp_pstartsec = hdp->p_start;
		disk_parms.dp_pnumsec = hdp->p_size;
		if (copyout((caddr_t)&disk_parms, arg, sizeof(disk_parms)) != 0)
				return(EFAULT);
		break;
		}

	case V_FORMAT:		/* Format tracks */
	case V_XFORMAT:
	case FMTBAD:		/* Format tracks as bad */
	     {
		register int	i, secno, track, num_trks, oldpri;

		if (copyin(arg, (caddr_t)&karg, sizeof(karg)) != 0) {
			return(EFAULT);
		}
		if (fmtvfyreq != 0) { /* Another format is already requested. */
			return(EBUSY);
		}
		fmtvfyreq = 1;
		oldpri = spl5();
		/*
		 * Wait until no I/O is happening to do the format.
		 * hdstart will wake us up when there are no I/O requests
		 * in any drive's queue.
		 */
		while (hdcst.hd_active)
			sleep(&fmtvfywait, PRIBIO);
		splx(oldpri);
		if (hdcst.hd_curdrv != unit) {
			hddrvinfo[hdcst.hd_curdrv].hd_state |= HD_FMT_RST;
			hdcst.hd_curdrv = unit;
		}
		if ( cmd == V_XFORMAT )
		{
			if (copyin(karg.ia_xfmt.intlv_tbl, itable, sizeof(itable)))
			{
				fmtvfyreq = 0;
				wakeup((char *)&fmtvfyreq);
				return(EFAULT);
			}
			track = karg.ia_xfmt.start_trk;
			num_trks = 1;
		}
		else    /* if cmd == V_FORMAT || cmd == FMTBAD */
		{
			/*
			 * Make interleave table.
			 * Even-numbered bytes are either 0x00 or 0x80 indicating
			 * good or bad sectors.  Since we aren't using this
			 * mechanism for bad block handling, set them all to zero.
			 * Odd-numbered bytes are the actual sector numbers.
			 * Note that since DOS expects sectors to be 1-based, we
			 * format the disk this way.
			 */
			bzero(itable, sizeof(itable));
			i = 1;
			secno = 1;
			do {
				if (itable[i] == '\0') { /* an unused slot */
					itable[i] = secno++;
					i = (int)(i + karg.ia_fmt.intlv * 2) % 
					    (int)(hdi->hd_nsecs * 2);
				} else
					i += 2; /* skip to next empty slot. */
			} while (secno <= (int)hdi->hd_nsecs);

			track = karg.ia_fmt.start_trk;
			num_trks = karg.ia_fmt.num_trks;
		}

		if ( cmd == FMTBAD )
		{
			for (i=0; i<SECSIZE; i+=2)
				itable[i] = 0x80;
		}

		/*
		 * Check if restore is necessary.
		 */
		if (hdi->hd_state & HD_FMT_RST) {
			hdi->hd_state |= HD_DO_FMT;
			ATxcmd(unit);
			oldpri = spl5();
			while (hdi->hd_state & HD_DO_FMT)
				sleep(itable, PRIBIO);
			splx(oldpri);
			hdi->hd_state &= ~HD_FMT_RST;
		}

		/*
		 * Now format all the requested tracks.
		 */

		for (i = 0; i < num_trks; i++, track++) {
			hdi->hd_state |= HD_DO_FMT;
			ATfmtcmd(unit, track, itable);
			oldpri = spl5();
			while (hdi->hd_state & HD_DO_FMT)
				sleep(itable, PRIBIO);
			splx(oldpri);
		}
		fmtvfyreq = 0;
		wakeup((char *)&fmtvfyreq);
		break;
	     }

	case V_VERIFY:		/* Verify sectors command */
	     {
		register int	i, secno, oldpri;
		union vfy_io	kvfyio;

		if (copyin(arg, (caddr_t)&kvfyio, sizeof(kvfyio)) != 0) {
			return(EFAULT);
		}
		hdp = &hdwholedisk[unit];
		if( kvfyio.vfy_in.abs_sec >= hdp->p_size ||
		    (daddr_t)(kvfyio.vfy_in.abs_sec + kvfyio.vfy_in.num_sec) > (daddr_t)hdp->p_size  ||
		    kvfyio.vfy_in.num_sec > MAXXFER)
		{
			return(EINVAL);
		}

		if (fmtvfyreq != 0) { /* Another verify is already requested. */
			return(EBUSY);
		}
		fmtvfyreq = 1;
		oldpri = spl5();
		/*
		 * Wait until no I/O is happening to do the verify.
		 * hdstart will wake us up when there are no I/O requests
		 * in any drive's queue.
		 */
		while (hdcst.hd_active)
			sleep(&fmtvfywait, PRIBIO);
		splx(oldpri);
		if (hdcst.hd_curdrv != unit) {
			hddrvinfo[hdcst.hd_curdrv].hd_state |= HD_FMT_RST;
			hdcst.hd_curdrv = unit;
		}
		vfystatus = 0;
		/*
		 * Check if restore is necessary.
		 */
		if (hdi->hd_state & HD_FMT_RST) {
			hdi->hd_state |= HD_DO_VFY;
			ATxcmd(unit);
			oldpri = spl5();
			while (hdi->hd_state & HD_DO_VFY)
				sleep(itable, PRIBIO);
			splx(oldpri);
			hdi->hd_state &= ~HD_FMT_RST;
			if (vfystatus) {
				fmtvfyreq = 0;
				wakeup((char *)&fmtvfyreq);
				return(ENXIO);
			}
		}

		hdcst.hd_physblk = kvfyio.vfy_in.abs_sec;
		hdcst.hd_nblks = kvfyio.vfy_in.num_sec;

		if (kvfyio.vfy_in.time_flg) {
			hdi->hd_state |= HD_DO_VFY;
			ATiocmd(unit, HD_SEEK);
			oldpri = spl5();
			while (hdi->hd_state & HD_DO_VFY)
				sleep(itable, PRIBIO);
			splx(oldpri);
			if (vfystatus) {
				fmtvfyreq = 0;
				wakeup((char *)&fmtvfyreq);
				return(ENXIO);
			}
		}

		hdi->hd_state |= HD_DO_VFY;
		drv_getparm(LBOLT, &kvfyio.vfy_out.deltatime);
		ATiocmd(unit, HD_RDVER|NO_RETRY);
		oldpri = spl5();
		while (hdi->hd_state & HD_DO_VFY)
			sleep(itable, PRIBIO);
		splx(oldpri);
		kvfyio.vfy_out.deltatime = vfytime - kvfyio.vfy_out.deltatime;
		kvfyio.vfy_out.err_code = vfystatus;
	
		fmtvfyreq = 0;
		wakeup((char *)&fmtvfyreq);

		if (copyout((caddr_t)&kvfyio, arg, sizeof(kvfyio)) != 0) {
			return(EFAULT);
		}
		break;
	     }

	case V_PDLOC: {	/* Tell user where pdinfo is on disk */
		unsigned long	vtocloc = HDPDLOC;

		if (copyout((caddr_t)&vtocloc, arg, sizeof(vtocloc)) != 0) {
			return(EFAULT);
		}
		break;
		}

	case V_RDABS:  
		/* need to be root, or requesting res slice, or part. table */

		if (copyin(arg, (caddr_t)&absio, sizeof(absio)) != 0) {
			return(EFAULT);
		}
		if (drv_priv(cred_p)) {   /* returns non-zero if not privileged */
		register int	i;
			if (absio.abs_sec != 0) { /* Not read of partition table */
				for (i = 0; ((i < V_NUMPAR) && 
			    	  (hdpartinfo[unit][i].p_tag != V_BOOT));
				   i++)
					;
				if (i == V_NUMPAR) {  /* No Res/V_BOOT slice */
					return(EACCES);
				}
				if ((absio.abs_sec < 
				     hdpartinfo[unit][i].p_start) ||
				    (absio.abs_sec >= 
				    (hdpartinfo[unit][i].p_start + 
				     hdpartinfo[unit][i].p_size))) {
					return(EACCES);
				}
			}
		}
		bp = geteblk();
		hdgetblock(bp, ABSDEV(dev), absio.abs_sec, SECSIZE);
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return(ENXIO);
		}
		else if (copyout(bp->b_un.b_addr, absio.abs_buf, SECSIZE) != 0){
			brelse(bp);
			return(EFAULT);
		}
		brelse(bp);
		break;
	case V_WRABS: {
		int error;

		if (error = drv_priv(cred_p)) {  /* need to be privileged to do wrabs */
			return(error);
		}
		if (copyin(arg, (caddr_t)&absio, sizeof(absio)) != 0) {
			return(EFAULT);
		}
		bp = geteblk();
		if (copyin(absio.abs_buf, bp->b_un.b_addr, SECSIZE) != 0){
			brelse(bp);
			return(EFAULT);
		}
		else {
			hdputblock(bp, ABSDEV(dev), absio.abs_sec, SECSIZE);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return(ENXIO);
			}
		}
		brelse(bp);
		break;
	   }
#ifdef ASYNCIO
	/* The following three ioctls provide support for ASYNC I/O */

	case DKIOCMLOCK: {
		return(raioctl(dev, cmd, arg, mode, &hdbreakup, MAXXFER << SCTRSHFT));
	}
	case DKIOCASTRT: {
		return(raioctl(dev, cmd, arg, mode, &hdbreakup, MAXXFER << SCTRSHFT));
		
	}
	case DKIOCASTAT: {
		return(raioctl(dev, cmd, arg, mode, &hdbreakup, MAXXFER << SCTRSHFT));
	}
	case DKIOCAIOVERS: {
		return(raioctl(dev, cmd, arg, mode, &hdbreakup, MAXXFER << SCTRSHFT));
	}
#endif /* ASYNCIO */
	default:
		return(EINVAL);
	}
	return(0);
}


hddone(bp)
register struct buf *bp;
{
	register struct iotime *hdit;
	register struct hddrvinfo *hdi;
	struct iobuf *hdu;
	long curtime;

	hdi = &hddrvinfo[hdcst.hd_curdrv];
	hdit = &hdstat[hdcst.hd_curdrv];
	hdu = &hdutab[hdcst.hd_curdrv];

	/* Update the statistics. */
	drv_getparm(LBOLT, &curtime);
	hdit->io_resp += curtime - bp->b_start;
	hdit->io_act += curtime - hdu->io_start;
	hdu->b_active = 0;
	hdcst.hd_errcnt = 0;
	if (hdi->hd_state & HD_OPEN)
		hdi->hd_state = HD_OPEN; /* ensure state cleared for next op */

	/* When driver reaches this point and writefault is set, retries have*/
	/* handled a stray writefault signal, now turn off writefault switch */
	if (writefault)
		writefault = 0;

	/* If we're done with this iobuf, wake up sleep in hdclose. */
	if ((hdu->b_actf = bp->av_forw) == NULL)
		wakeup((char *)&hdu->b_actf);

	if (bp == hdi->hd_latest)
		hdi->hd_latest = NULL;

	if ((bp->b_resid += (bp->b_bcount - (unsigned)hdcst.hd_nbytes)) != 0)
		bp->b_flags |= B_ERROR;

#ifdef ASYNCIO
	if (bp->b_flags & B_RAIO)
		raio_bkdone(bp);
#endif
	iodone(bp);
	hdstart();
	return(0);
}


hderror(bp, retry)
register struct buf *bp;
{
	/* If retry not requested or more than MAXRETRY errors, give up. */
	if (!retry || ++hdcst.hd_errcnt > MAXRETRY) {
		if (writefault)
			cmn_err(CE_PANIC, "HD controller: write fault\n");
		else {
			/* make sure that related variables to BBH are cleared
		 	* before giving up.
		 	*/
			hdb_err(bp);
			bp->b_resid <<= SECSHFT; /* hddone expects b_resid in bytes. */
			hddone(bp);
			}
	} else {
		/* If more than 5 errors, try restoring the heads. */
		if (hdcst.hd_errcnt > 5)
			hddrvinfo[hdcst.hd_curdrv].hd_state |= HD_DO_RST;
		hdxfer(bp);
	}
	return(0);
}


hderrmsg(blockno, msg)
daddr_t blockno;
char    *msg;
{
	register int track;
	register struct hddrvinfo *hdi;

	if (hdcst.hd_errcnt < MAXRETRY)
		return(0);

	hdi = &hddrvinfo[hdcst.hd_curdrv];
	track = blockno / (daddr_t)hdi->hd_nsecs;

    cmn_err(CE_NOTE,"HD error: drive %d, cyl 0x%x, head 0x%x, sector 0x%lx: %s",
	hdcst.hd_curdrv, track/(int)hdi->hd_nhds, track%(int)hdi->hd_nhds,
	blockno%(daddr_t)hdi->hd_nsecs, msg);
	return(0);
}


/*
 * Error message, called from Unix 5.3 kernel via bdevsw
 */
hdprint (dev, str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE, "%s on hard disk unit %d, partition %d\n",
		str, UNIT(dev), PARTITION(dev));
	return(0);
}


static  struct  AT_cmd AT_cmd;

ATdocmd(drive)
{
	AT_cmd.nhd_drv |= (HD_DHFIXED | (drive << 4));
	if (ATcmd(&AT_cmd) != 0)
		cmn_err(CE_PANIC, "HD controller: command aborted\n");

	if (AT_cmd.nhd_cmd == HD_WRSEC)
		ATout(hdcst.hd_addr);
	return(0);
}


ATiocmd(drv, cmd)
int drv;
int cmd;
{
	register unsigned track;
	register struct hddrvinfo *hdi;
	unsigned char	ehd;

	hdi = &hddrvinfo[drv];
	AT_cmd.nhd_cmd = cmd;
	AT_cmd.nhd_precomp = hdi->hd_precomp >> 2;
	if (hdi->hd_nhds > 8)
		ehd = HD_EXTRAHDS;
	else
		ehd = HD_NOEXTRAHDS;
	outb(FDR, ehd);

#ifdef DEBUG
	if (hddebug) {
		cmn_err(CE_NOTE,"ATiocmd: cmd 0x%x sector 0x%x.\n",
			cmd, hdcst.hd_physblk);
	}
#endif
	track = hdcst.hd_physblk / (daddr_t)hdi->hd_nsecs;
	AT_cmd.nhd_drv = track % hdi->hd_nhds;
	AT_cmd.nhd_cyl = track / hdi->hd_nhds;
	AT_cmd.nhd_sect = (hdcst.hd_physblk % (daddr_t)hdi->hd_nsecs);
	AT_cmd.nhd_nsect = hdcst.hd_nblks;
	ATdocmd(drv);
	return(0);
}


ATxcmd(drv)
int drv;
{
	AT_cmd.nhd_cmd = HD_RESTORE;
	AT_cmd.nhd_drv = 0;
	ATdocmd(drv);
	return(0);
}


ATfmtcmd(drv, track, itable)
unsigned drv;
unsigned track;
char *itable;	/* interleave table */
{
	register struct hddrvinfo *hdi;
	unsigned char	ehd;

	hdi = &hddrvinfo[drv];

	AT_cmd.nhd_cmd = HD_FORMAT;
	AT_cmd.nhd_precomp = hdi->hd_precomp >> 2;
	if (hdi->hd_nhds > 8)
		ehd = HD_EXTRAHDS;
	else
		ehd = HD_NOEXTRAHDS;
	outb(FDR, ehd);
	AT_cmd.nhd_drv = track % hdi->hd_nhds;
	AT_cmd.nhd_cyl = track / hdi->hd_nhds;
        /* The following is per the Western Digital 1010-05 manual... */
        AT_cmd.nhd_sect = 35;   /* Make GAP 1 & 3 38 bytes */
	AT_cmd.nhd_nsect = hdi->hd_nsecs;
	ATdocmd(drv);
	ATout(itable);
	return(0);
}


/*
 * Routine to write a command to the controller registers.
 */
ATcmd(cmdp)
struct AT_cmd *cmdp;
{
	register unsigned char	status;

#ifdef DEBUG
	if (hddebug) {
		cmn_err(CE_NOTE,"AT_cmd: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
			cmdp->nhd_precomp, cmdp->nhd_nsect, cmdp->nhd_sect+1,
			cmdp->nhd_cyl&0xFF, (cmdp->nhd_cyl>>8)&0xFF,
			cmdp->nhd_drv, cmdp->nhd_cmd);
	}
#endif
	
	ATwait(); /* wait for controller ready */
	status = inb(HD0 + HD_STATUS);
	if ((status & WRFAULT) || !(status & READY))
		return (-1);

	outb(HD0+HD_PRECOMP, cmdp->nhd_precomp);
	outb(HD0+HD_NSECT, cmdp->nhd_nsect);
	/*
	 * DOS expects sectors to be 1-based, so the disk is formatted that 
	 * way.  Everywhere else we keep sectors 0-based, and convert here
	 * right before we talk to the controller.
	 */
	outb(HD0+HD_SECT, cmdp->nhd_sect+1);

	outb(HD0+HD_LCYL, cmdp->nhd_cyl&0xFF);
	outb(HD0+HD_HCYL, (cmdp->nhd_cyl>>8)&0xFF);
	outb(HD0+HD_DRV, cmdp->nhd_drv);
	outb(HD0+HD_CMD, cmdp->nhd_cmd);
	return (0);
}


/*
 * Return the value of the controller status register
 */
ATstatus()
{
	ATwait(); /* wait for controller ready */
	return inb(HD0 + HD_STATUS);
}


/*
 * Wait for controller to be not busy.  Will wait
 * approx. 1/4 second if controller still busy
 * then hdtimeout shall attempt to reset the controller
 * and re-issue the current request,
 */
ATwait()
{
	register int		i;

	for (i = HDTIMOUT; i > 0; i--) {
		if (inb(HD0 + HD_STATUS) & BUSY) {
			tenmicrosec();
			continue;
		}
		return (0);
	}
	hdtimeout( 1 );
	return(0);
}


/*
 * Routine to write data to the sector buffer.
 */
ATout(buf)
caddr_t	buf;
{

	ATwait(); /* wait for controller ready */
	if ((inb(HD0 + HD_STATUS) & DATARQ) == 0)
		return(-1);
	loutw(HD0 + HD_DATA, buf, SECSIZE/2);
	return(0);
}


/*
 * Routine to read data from the sector buffer.
 */
ATin(buf)
caddr_t	buf;
{

	ATwait(); /* wait for controller ready */
	if ((inb(HD0 + HD_STATUS) & DATARQ) == 0)
		return(-1);
	linw(HD0 + HD_DATA, buf, SECSIZE/2);
	return(0);
}

/* Find out if block is in UNIX space. */
hdb_inunix(bp)
register struct buf *bp;
{
	unsigned partition;

	if (ISABSDEV(bp->b_edev))
		return(0);
	partition = PARTITION(bp->b_edev);
	return ( hdpartinfo[hdcst.hd_curdrv][partition].p_tag != V_OTHER );
}

/* Determine if block is in the sacred area of the disk */
hdb_sacred(block)
daddr_t block;
{
	return (block <= hddrvinfo[hdcst.hd_curdrv].hd_last_sacred);
}

/* Determine if there are no more spare blocks to be used as surrogates */
hdb_nospar()
{
	register struct alt_table *altptr;

	altptr = &hdaltinfo[hdcst.hd_curdrv].alt_sec;
	return ( altptr->alt_used >= altptr->alt_reserved );
}

/* Main routine for verification of Marginal blocks */
hdb_verify(bp)
register struct buf *bp;
{
	daddr_t block;

	/* if retry have not started yet, find out which disk block
	 * needs to be retried.
	 */
	if (hdbad.hdb_rtycnt == 0)
		hdbad.hdb_ndx = hdb_b2vfy();
	block = hdbad.hdb_b2vfy[hdbad.hdb_ndx];

	if (hdbad.hdb_rtycnt <= 5 && hdbad.hdb_eccknt < 3)
		hdb_retry(bp);
	else
	{
		if (hdbad.hdb_eccknt >= 3)
		{
			cmn_err(CE_NOTE, hdb_msg[BEGVFY], block, hdcst.hd_curdrv); 
			cmn_err(CE_NOTE,hdb_msg[MRGBLK],block, hdcst.hd_curdrv);
			hdb_mapblk(bp,block);
			return(0);
		}
/* NO Longer display message is block is good (no remap) */
/*		else 
			cmn_err(CE_NOTE,hdb_msg[GOODBLK],block,hdcst.hd_curdrv);
		*/

		hdb_contvfy(bp);
	}
	return(0);
	
}


/* Determine which block if any need to be verified */
hdb_b2vfy()
{
	short i;
register struct hddrvinfo *hdi;
hdi = &hddrvinfo[hdcst.hd_curdrv];

	/* determine which disk block needs to be verified */
	for (i=hdbad.hdb_ndx; i<MAXNBLKS; i++)
	{
		if (hdbad.hdb_b2vfy[i] != 0 )
		{
/*	Removing message, printed only if bad
			cmn_err(CE_NOTE,hdb_msg[BEGVFY],
				hdbad.hdb_b2vfy[i], hdcst.hd_curdrv);
*/
#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: on hdb_b2vfy() retning index :%d: hd_state=0x%x: mapflg=:%d:",
		i, hdi->hd_state, hdbad.hdb_mapflg);
#endif
			return(i);
		}
	}
	return(-1);
}


hdb_retry(bp)
register struct buf *bp;
{
	register struct hddrvinfo *hdi;
	daddr_t block;

		hdi = &hddrvinfo[hdcst.hd_curdrv];
		hdbad.hdb_rtycnt++;
		block = hdbad.hdb_b2vfy[hdbad.hdb_ndx];
		hdcst.hd_physblk = block;
		hdcst.hd_nblks = 1;
		hdi->hd_state |= HD_DO_RST;
		bp->b_resid++;
		hdcst.hd_addr = (long)hdbad.hdb_tmpbuf;
#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: hdb_retry() rdy to call hdxfer() for blk %d rtycnt=%d eccknt=%d",
	block, hdbad.hdb_rtycnt, hdbad.hdb_eccknt);
#endif

		hdxfer(bp);
	return(0);
}


/* Continue with the verification of potential marginal blocks */
hdb_contvfy(bp)
register struct buf *bp;
{
	register struct hddrvinfo *hdi;
	daddr_t block;
	short i;

	hdi = &hddrvinfo[hdcst.hd_curdrv];
	/* clear info related to the disk block just verified */
	hdbad.hdb_b2vfy[hdbad.hdb_ndx] = 0;
	hdbad.hdb_pmaddr[hdbad.hdb_ndx] = 0;
	hdbad.hdb_eccknt=0;
	hdbad.hdb_rtycnt=0;
	hdbad.hdb_vfyflg--;
	hdbad.hdb_nb2vfy--;

	/* if all disk blocks are verified, release buffer */
	if ( hdbad.hdb_nb2vfy == 0 )
	{
		hdbad.hdb_ndx=0;
		hdi->hd_state &= ~HD_BBH_VFY;
		for (i=0; i<SECSIZE; ++i)
			hdbad.hdb_tmpbuf[i] = 0;
#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: in hdb_contvfy() hd_state=:0x%x: mapflg=:%d: rel buf. calling hddone()",hdi->hd_state, hdbad.hdb_mapflg);
#endif
		hddone(bp);
	}
	else
	{
		/* more disk blocks need to be verified, determine
		 * which block and start the retry procedure.
		 */
		hdbad.hdb_ndx = hdb_b2vfy();
#ifdef BBH_DEBUG
	block = hdbad.hdb_b2vfy[hdbad.hdb_ndx];
	cmn_err(CE_NOTE,"HD: on hdb_contvfy(bp) star retry for block :%d: ",block);
	cmn_err(CE_NOTE,"hd_state=:0x%x: mapflg=:%d:",hdi->hd_state,hdbad.hdb_mapflg);
#endif
		hdb_retry(bp);
	}
	return(0);
}


/* mapping of marginal blocks is done here */
hdb_mapblk(bp,block)
register struct buf *bp;
daddr_t block;
{

#ifdef BBH_DEBUG
	cmn_err(CE_NOTE,"HD: in hdb_mapblk() routine  block :%d:",block);
#endif
	if ( hdb_nospar() )
	{
		cmn_err(CE_WARN,hdb_msg[NOSPAR],
 			hdcst.hd_physblk, hdcst.hd_curdrv);
		return(0);
	}
	
	hdbad.hdb_mblktyp = hdb_blktyp(block,hdcst.hd_curdrv);

#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: in hdb_mapblk() block type is :%d:",hdbad.hdb_mblktyp);
#endif

	if( hdbad.hdb_mblktyp == UNASSIG )
	{
		hdb_updtbl(block, hdcst.hd_curdrv, hdbad.hdb_mblktyp);
		hdb_wrttbl(bp);
	}
	else /*if hdbad.hdb_mblktyp is == ASSIGND or == REGBLK) */
	{
		hdb_wrtsec(bp);
	}
}

/* Determine type of block to be mapped */
hdb_blktyp(block, curdrv)
daddr_t block;
ushort curdrv;
{
	register struct alt_table *altptr;
	daddr_t  lastalt;

	altptr = &hdaltinfo[curdrv].alt_sec;
	lastalt = altptr->alt_base +  altptr->alt_reserved -1;

	/* is block is in the reserved area */
	if( block >= altptr->alt_base && block <= lastalt )
	{
		/* is block an assigned alternate */
		if( block < (daddr_t)(altptr->alt_base + altptr->alt_used) )
			return(ASSIGND);
		else
			return(UNASSIG);
	}
	else
		/* block is a regular block */
		return(REGBLK);
}

/* Update the bad block mapping table */
hdb_updtbl(block, curdrv, blktyp)
daddr_t block;
ushort curdrv;
ushort blktyp;
{

	register struct alt_table *ap;
	short trk;

	ap = &hdaltinfo[curdrv].alt_sec;

	if ( blktyp == UNASSIG )
	{
		/* bad block is an unassigned alt., make it unavailable in table */
		ap->alt_bad[block - ap->alt_base] = -1;
		if (block - ap->alt_base != ap->alt_used)
			return(0);
	}

	else if ( blktyp == ASSIGND )
	{
		/* bad block is an assigned alt. block. Reassign the next
		 * available alt. block from the table to it.
		 * Also, make the bad alternate block unavailable in table.
		 */
		ap->alt_bad[ap->alt_used] = ap->alt_bad[block - ap->alt_base];
		ap->alt_bad[block - ap->alt_base] = -1;
		/* Switch in initially requested block for alt which has gone bad */
		hdbad.hdb_physblk = hdcst.hd_badblk;
		hdcst.hd_physblk = hdcst.hd_badblk;
	}

	else /* if blktyp is == REGBLK  */
	{
		/* bad block is regular block. Enter it in the map table */
		ap->alt_bad[ap->alt_used] = block;
		trk = block / (daddr_t)hddrvinfo[curdrv].hd_nsecs
				- hddrvinfo[curdrv].hd_begtrk;
		bit_set(bbh_trkmap[curdrv], trk); 
		bit_set(bth_trkmap[curdrv], trk); 
	}
	while ( ap->alt_used < ap->alt_reserved )
	{
		if( ap->alt_bad[++ap->alt_used] != -1 )
			break;
	}
	return(0);
}


/* write the bad block mapping table to disk */
hdb_wrttbl(bp)
register struct buf *bp;
{
	short nblks;
	ushort curdrv;
	register struct hddrvinfo *hdi;

	hdi = &hddrvinfo[curdrv = hdcst.hd_curdrv];

	/* calculate # of disk blocks of the table
	 * and # of bytes (factor of 512)
	 */
	nblks =  (ushort)(hdi->hd_alts_len + SECMASK) >> (ushort)SECSHFT;

	/* save variables needed to continue regular IO after mapping is done */
	/* done only when good alt had been found, nbadblk will > 1 if the    */
	/* next the UNASSIGN alts are bad.                                    */
	if (hdbad.hdb_nbadblk < 2) {
		hdbad.hdb_addr = hdcst.hd_addr;
		hdbad.hdb_physblk = hdcst.hd_physblk;
		hdbad.hdb_bpresid = bp->b_resid;
		hdbad.hdb_bpflags = bp->b_flags;
	}

	hdcst.hd_addr = (paddr_t)&hdaltinfo[curdrv];
	hdcst.hd_physblk = hdi->hd_alts_loc;
	hdcst.hd_nblks = nblks;
	hdbad.hdb_mapflg |= HDB_WTBL;
	bp->b_flags &= ~B_READ;
	bp->b_resid = hdcst.hd_nblks;
	hdi->hd_state |= HD_BBH_MAP;

#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: on hdb_wrttbl() wrtng beg sec :%d: for :%d: blks call hdxfer()",
	hdcst.hd_physblk, hdcst.hd_nblks);
#endif

	hdxfer(bp);
	return(0);
}


hdb_cleanup(bp)
register struct buf *bp;
{
	short i;
	register struct hddrvinfo *hdi;
	daddr_t block;

	hdi = &hddrvinfo[hdcst.hd_curdrv];

	if ( hdbad.hdb_mapflg & HDB_WTBL)
	{
		if ( hdbad.hdb_badflg != 0 )
		{
			i = --hdbad.hdb_nbadblk;
			block = hdbad.hdb_bad2map[i];
			hdbad.hdb_bad2map[i] = 0;
		}
		else
			block = hdbad.hdb_b2vfy[hdbad.hdb_ndx];

		hdi->hd_state &= ~HD_BBH_MAP;
		hdbad.hdb_mapflg &= ~HDB_WTBL;
		cmn_err(CE_NOTE,hdb_msg[MAPBLK], block, hdcst.hd_curdrv);
	}
	else /* if hdbad.hdb_mapflg & HDB_WSEC  */
		hdbad.hdb_mapflg &= ~HDB_WSEC;

	/* restore variables needed to continue regular IO after mapping is done */
	hdcst.hd_addr =  hdbad.hdb_addr;
	hdcst.hd_physblk = hdbad.hdb_physblk;
	bp->b_resid = hdbad.hdb_bpresid;
	bp->b_flags = hdbad.hdb_bpflags;

	hdbad.hdb_addr = 0;
	hdbad.hdb_physblk = 0;
	hdbad.hdb_bpresid = 0;
	for (i=0; i<TBUFFLEN; i++)
		hdbad.hdb_tmpbuf[i] = 0;
	return(0);
}


/* write a sector to disk */
hdb_wrtsec(bp)
register struct buf *bp;
{
	register struct alt_table *ap;
	register struct hddrvinfo *hdi;
	short i;

#ifdef BBH_DEBUG
	daddr_t block;
#endif

	ap = &hdaltinfo[hdcst.hd_curdrv].alt_sec;
	hdi = &hddrvinfo[hdcst.hd_curdrv];

	/* save variables needed to continue regular IO after mapping is done */
	hdbad.hdb_addr = hdcst.hd_addr;
	hdbad.hdb_physblk = hdcst.hd_physblk;
	hdbad.hdb_bpresid = bp->b_resid;
	hdbad.hdb_bpflags = bp->b_flags;

	/* if mapping a bad block */
	if( hdbad.hdb_badflg != 0 )
	{
		/* initialize alternate block to zeros */
		for (i=0; i<SECSIZE; ++i)
			hdbad.hdb_tmpbuf[i] = 0;
		hdcst.hd_addr = (paddr_t)hdbad.hdb_tmpbuf;
#ifdef BBH_DEBUG
		block = hdbad.hdb_bad2map[hdbad.hdb_nbadblk-1];
#endif
	}
	else
	{
		hdcst.hd_addr = hdbad.hdb_pmaddr[hdbad.hdb_ndx];
#ifdef BBH_DEBUG
		block = hdbad.hdb_b2vfy[hdbad.hdb_ndx];
#endif
	}
	hdcst.hd_physblk = ap->alt_base + ap->alt_used;
	hdcst.hd_nblks = 1;
	hdbad.hdb_mapflg |= HDB_WSEC;
	bp->b_flags &= ~B_READ;
	bp->b_resid = hdcst.hd_nblks;
	hdi->hd_state |= HD_BBH_MAP;

#ifdef BBH_DEBUG
cmn_err(CE_NOTE,"HD: on hdb_wrtsec() copying sec :%d: to sec :%d: ",
	block, hdcst.hd_physblk);
cmn_err(CE_NOTE,"hd_state=:0x%x: hdb_mapflg=:%d:", hdi->hd_state,hdbad.hdb_mapflg);
#endif

	hdxfer(bp);
	return(0);
}

/* mapping of bad blocks is done here*/
hdb_mapbad(bp)
register struct buf *bp;
{
	daddr_t block;
	short i;
	unsigned partition;
	register struct hddrvinfo *hdi;

	hdi = &hddrvinfo[hdcst.hd_curdrv];

	
	/* If bad block occurred while mapping a marginal block (ie. the
	 * alt. sec. being assigned to the marginal block is detected bad),
	 * take care of the bad block now!!! forget the marginal for now.
	 * Marginal block can be handled next time around.
	 */
	if( hdbad.hdb_vfyflg > 0 && 
	    (hdi->hd_state & HD_BBH_VFY) &&
	    (hdbad.hdb_mapflg & HDB_WSEC)   )
	{
		/* restore values of bp */
		bp->b_resid = 0;
		bp->b_flags = hdbad.hdb_bpflags;

		/* Zero out variables set for marginal blocks */
		hdb_0mrgflg(bp);
	}

	/* If a bad block occurred after a block of current buffer has
	 * been detected potential marginal and/or marginal,
	 * forget the marginal block. Take care of the bad block now.
	 */
	else if ( hdbad.hdb_vfyflg > 0 )
	{
		/* Zero out variables set for marginal blocks */
		hdb_0mrgflg(bp);
	}

	i = hdbad.hdb_nbadblk-1;
	block = hdbad.hdb_bad2map[i];

#ifdef BBH_DEBUG
	cmn_err(CE_NOTE,"HD: in hdb_mapbad() routine  block :%d:",block);
#endif

	if ( hdb_nospar() )
	{
		cmn_err(CE_WARN,hdb_msg[BNOSPR],
 			block,hdcst.hd_curdrv);
		hdb_err(bp);
		hderror(bp,RETRY);
	}
	hdbad.hdb_bblktyp = hdb_blktyp(block,hdcst.hd_curdrv);
	if( hdbad.hdb_bblktyp == UNASSIG )
	{
		hdbad.hdb_mapflg &= ~HDB_WSEC;
		hdb_updtbl(block, hdcst.hd_curdrv, hdbad.hdb_bblktyp);
		hdb_wrttbl(bp);
	}
	else /*if hdbad.hdb_bblktyp is == ASSIGND or == REGULAR */
	{
		if (bp->b_flags & B_READ)
		{
			bp->b_flags |= B_ERROR;		/* fail I/O */
			partition = PARTITION(bp->b_edev);
			if (hdpartinfo[hdcst.hd_curdrv][partition].p_tag == V_ROOT ||
			    hdpartinfo[hdcst.hd_curdrv][partition].p_tag == V_USR    )
			{
#ifdef BBH_DEBUG
		cmn_err(CE_NOTE, "Calling fshadbad()to mark FS dirty");
#endif
				fshadbad(bp->b_edev,
				    bp->b_blkno+(bp->b_bcount/SECSIZE - bp->b_resid));
#ifdef BBH_DEBUG
		cmn_err(CE_NOTE, "Returned from fshadbad()");
#endif
			}
		}
		hdb_wrtsec(bp);
	}
	return(0);
}


hdb_err(bp)
register struct buf *bp;
{
	register struct hddrvinfo *hdi;

	hdi = &hddrvinfo[hdcst.hd_curdrv];

	if( hdbad.hdb_vfyflg > 0 || hdi->hd_state & HD_BBH_VFY )
	{
		/* Restore values of bp */
		bp->b_resid = 0;
		if( hdi->hd_state & HD_BBH_MAP )
			bp->b_flags = hdbad.hdb_bpflags;

		/* zero out variables set for marginal block */
		hdb_0mrgflg();
	}

	if( hdbad.hdb_badflg != 0 )
	{
		/* Restore values of bp */
		bp->b_resid = 0;
		if( hdi->hd_state & HD_BBH_MAP )
			bp->b_flags = hdbad.hdb_bpflags;

		/* zero out variables set for bad block */
		hdb_0badflg();
	}
	return(0);
}


/* Zero out all variables set for marginal blocks */
hdb_0mrgflg()
{
	register struct hddrvinfo *hdi;
	short i;

	hdi = &hddrvinfo[hdcst.hd_curdrv];
	hdi->hd_state &= ~HD_BBH_VFY;
	hdi->hd_state &= ~HD_BBH_MAP;

	for ( i=0; i<MAXNBLKS; i++)
	{
		hdbad.hdb_b2vfy[i] = 0;
		hdbad.hdb_pmaddr[i]= 0;
	}
	for ( i=0; i<TBUFFLEN; i++)
		hdbad.hdb_tmpbuf[i] = 0;

	hdbad.hdb_nb2vfy = 0;
	hdbad.hdb_vfyflg = 0;
	hdbad.hdb_eccknt = 0;
	hdbad.hdb_rtycnt = 0;
	hdbad.hdb_ndx    = 0;
	hdbad.hdb_mapflg = 0;
	hdbad.hdb_mblktyp= 0;
	hdbad.hdb_bpflags= 0;
	return(0);
}


/* Zero out all variables set for bad blocks */
hdb_0badflg()
{
	register struct hddrvinfo *hdi;
	short i;

	hdi = &hddrvinfo[hdcst.hd_curdrv];
	hdi->hd_state &= ~HD_BBH_MAP;

	for ( i=0; i<2; i++)
		hdbad.hdb_bad2map[i] = 0;
	hdbad.hdb_nbadblk = 0;
	hdbad.hdb_bblktyp = 0;
	hdbad.hdb_badflg  = 0;
	return(0);
}

int
hdsize(dev)
dev_t dev;
{
        unsigned unit;
        register long nblks;
	struct hddrvinfo *hdi;
	struct partition *hdp;

        unit = UNIT(dev);
	hdi = &hddrvinfo[unit];
        if (!(hdi->hd_state & HD_OPEN)) {
                /*
                 * Initialize.  (The VTOC must be read in order to get the
                 * partition table.)
                 */
                if (hdopen(&dev, 0, OTYP_LYR) == 0)
                	(void) hdclose(dev, 0, OTYP_LYR);
        }
	hdp = hdpartinfo[unit];
        if (hdp[PARTITION(dev)].p_flag == ~V_VALID)
                nblks = -1;
        else
                nblks = hdp[PARTITION(dev)].p_size;
        return nblks;
}

static void
hdtimeout( hdbusy )
        int hdbusy;
{
	register int nblks;
	register struct buf *bp;
	register struct iobuf *hdu;
	register unit;
        register unsigned char nheads;
	long lbolt_val;
	struct hddrvinfo *hdi;

	if(drv_getparm(LBOLT,&lbolt_val) == -1) {
		hdcst.hd_idto = timeout( hdtimeout, 0, (30 * HZ) );
		return;
	}

	bp = hdutab[hdcst.hd_curdrv].b_actf;

	if((( bp == NULL ) || ((bp->b_start + (30 * HZ)) > lbolt_val)) && hdbusy == 0) {
		hdcst.hd_idto = timeout( hdtimeout, 0, (30 * HZ) );
		return;
	}

	if( hdbusy == 1 ) {
		/***
		** Called via ATwait(), remove ourself from the
		** callout table.
		***/
		untimeout( hdcst.hd_idto );
	}

	Hd_timeout = 1;

	cmn_err(CE_WARN,"!Hard Disk Request Timed Out, Resetting Controller");

	outb( FDR, 0x4 );
	tenmicrosec();
	tenmicrosec();

	outb( FDR, 0x0 );
	tenmicrosec();

	unit = hdcst.hd_curdrv;
	hdu = &hdutab[unit];

	if (hdu->b_active == 0) {
		hdu->b_active++;
		hdu->io_start = lbolt_val;
	}

	hdi = &hddrvinfo[unit];

	hddrvinfo[unit].hd_state |= HD_DO_RST;
	outb(HD0+HD_CMD, HD_RESTORE );
	hdi->hd_state &= ~HD_DO_RST;
	tenmicrosec();

	ATwait();

        /***
        ** Set drive 1 parameters.
        ***/
        hdcst.hd_curdrv = 0;
        hddrvinfo[0].hd_geom = bootinfo.hdparams[0];
        nheads = HD_DHFIXED | (hddrvinfo[0].hd_nhds-1);
        nheads |= HD_DRIVE0;
        outb(HD0+HD_DRV, nheads);
        outb(HD0+HD_NSECT, hddrvinfo[0].hd_nsecs);
        outb(HD0+HD_CMD, HD_SETPARAM);
        ATwait();

        if (bootinfo.hdparams[1].hdp_ncyl != 0) {
                /***
                ** Set drive 2 parameters.
                ***/
                hdcst.hd_curdrv = 1;
                hddrvinfo[1].hd_geom = bootinfo.hdparams[1];
                nheads = HD_DHFIXED | (hddrvinfo[1].hd_nhds-1);
                nheads |= HD_DRIVE1;
                outb(HD0+HD_DRV, nheads);
                outb(HD0+HD_NSECT, hddrvinfo[1].hd_nsecs);
                outb(HD0+HD_CMD, HD_SETPARAM);
                ATwait();
        }

        hdcst.hd_curdrv = unit;

	tenmicrosec();

	hdcst.hd_idto = timeout( hdtimeout, 0, (30 * HZ) );

	nblks = hdcst.hd_nblks;
	hdcst.hd_physblk = cur_req.physblk;
	hdcst.hd_addr = cur_req.addr;
	bp->b_resid = cur_req.resid;
	hdcst.hd_nblks = 1;

	for( ; nblks; --nblks ) {
		ATwait();
		ATiocmd(unit, bp->b_flags&B_READ? HD_RDSEC : HD_WRSEC);
	}

	Hd_timeout = 0;

	return;
}
