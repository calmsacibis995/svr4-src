/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sa:sys/pdi.h	1.3.3.1"

#ifndef NULL
#define NULL 0
#endif

#define	pdi_procp_t	caddr_t

/*
 *  The portable buffer header structure
 */
typedef struct pdi_buf
{
	long	b_flags;	/* status of I/O */
	long	unavail1;	/* unavailable */
	long	unavail2;	/* unavailable */
 	struct pdi_buf *b_forw;	/* driver queue pntrs */
	struct pdi_buf *b_back;
	short	unavail3;	/* unavailable */
	unsigned long	b_bcount;  /* transfer count */
	caddr_t b_addr;		/* buffer virtual address */
	daddr_t	b_blkno;	/* block # on device (in 512 bytes)*/
	char	b_error;	/* returned after I/O */
	unsigned long	b_resid;  /* words not transferred after error */
	time_t	b_start;	/* request start time */
	pdi_procp_t	b_proc;	/* Pointer to process */
} pdi_buf_t;

/*
 *	These values are kept in b_flags.
 */
#define PDI_B_WRITE   0x0000	/* non-read pseudo-flag */
#define PDI_B_READ    0x0001	/* read when I/O occurs */
#define PDI_B_DONE    0x0002	/* transaction finished */
#define PDI_B_ERROR   0x0004	/* transaction aborted  */
#define PDI_B_BUSY    0x0008	/* buffer busy          */
#define PDI_B_PHYS    0x0010	/* physical I/O         */

/*
 * System accounting logging structures
 */

typedef struct pdi_iostat {
	long	io_ops;		/* number of read/writes */
	long	io_misc;	/* number of "other" operations */
	long	io_qcnt;	/* number of jobs assigned to drive */
	ushort io_unlog;	/* number of unlogged errors */
} pdi_iostat_t;

#define	NTRACK		10	/* # of entries in drive's perf. queue */

/* performance data gathering queue */

typedef struct	pdi_df_ptrack
{
	long		b_blkno;	/* start disk block address */
	pdi_buf_t	*bp;		/* pointer to user buffer header */
} pdi_ptrk_t;

/*
 * structures for system accounting
 */
typedef struct pdi_iotime {
	pdi_iostat_t 	ios; 	/* iostat sub structure */
	long	io_bcnt;	/* total blocks transferred */
	time_t	io_resp;	/* total block response time in ms */
	time_t	io_act;		/* cumulative use in ms */
	long	maj;		/* major number of device */
	long	min;		/* minor number of device */
	long	tnrreq;		/* total number of read requests */
	long	tnwreq;		/* total number of write requests */
	long	cumqlen;	/* cumulative queue length */
	long	maxqlen;	/* max queue length */
	long	minqlen;	/* minimum queue length */
	long	cumseekd;	/* cumulative seek distance */
	time_t	io_liact;	/* time drive active for last interval in ms */
	time_t	io_intv;	/* drive perf reporting interval in ms */
	pdi_ptrk_t	*pttrack;	/* start of queue */
	pdi_ptrk_t	*endptrack;	/* last entry in queue */
	pdi_ptrk_t	ptrackq[NTRACK];	/* queue */
} pdi_iotime_t;

typedef struct	pdi_dskinfo
{
	pdi_iotime_t	di_perf;	/* driver activity accnting */
	long		di_cyls;	/* # of cylinders on drive  */
	long		di_tracks;	/* # of tracks per cylinder */
	long		di_sectors;	/* # of sectors per track   */
	long		di_bytes;	/* # of bytes per sector    */
	long		di_logicalst;	/* logical start of the disk*/
} pdi_dskinfo_t;

/*
 *  dma function tuple structure 
 */
typedef struct pdi_dma_tuple {
	paddr_t physaddr;
	long	count;
} pdi_dma_t;

/*
 *  portable device number structure 
 */
typedef struct pdi_dev {
	long	maj;
	long	min;
} pdi_dev_t;

/*
 *  mode flag values in driver call
 */
#define	PDI_FOPEN	0xffffffff
#define	PDI_FREAD	0x01
#define	PDI_FWRITE	0x02
#define	PDI_FNDELAY	0x04
#define	PDI_FAPPEND	0x08
#define PDI_FSYNC	0x10

#define	io_cnt	ios.io_ops
#define io_qc	ios.io_qcnt


/*
 * PDI Functions
 */
extern caddr_t		pdi_base();
extern void		pdi_bdevset();
extern long		pdi_bemajor();
extern long		pdi_beminor();
extern long		pdi_brelse();
extern time_t		pdi_btime();
extern void		pdi_clrbuf();
extern void		pdi_cmn_err();
#define	PDI_CE_CONT	0
#define	PDI_CE_NOTE	1
#define	PDI_CE_WARN	2
#define	PDI_CE_PANIC	3

extern long		pdi_copyin();
extern long		pdi_copyout();
extern uint		pdi_count();
extern void		pdi_dmafreelist();
extern pdi_dma_t	*pdi_dmamakelist();
extern long		pdi_emajor();
extern long		pdi_eminor();
extern void		pdi_error();
extern long		pdi_euid();
extern long		pdi_freemem();
#define	PDI_ROOTID	0

extern pdi_buf_t	*pdi_getrbuf();
extern void		pdi_freerbuf();
extern pdi_buf_t	*pdi_geteblk();
extern caddr_t		pdi_getmem();
extern void		pdi_hdelog();
#define PDI_SERLEN		12	/* length of serial number */
#define PDI_HDECRC		1	/* for CRC data checking */
#define PDI_HDEECC		2	/* for ECC data processing */
#define PDI_HDEMARG		1	/* for marginal blocks */
#define PDI_HDEUNRD		2	/* for unreadable blocks */

extern void		pdi_hdeeqd();
#define	PDI_EQD_ID	0
#define	PDI_EQD_IF	1
#define	PDI_EQD_TAPE	2
#define	PDI_EQD_EHDC	3
#define	PDI_EQD_EFC	4
#define	PDI_EQD_OROM	5
#define	PDI_EQD_ORW	6
#define	PDI_EQD_OWORM	7

extern long		pdi_imajor();
extern long		pdi_iminor();
extern void		pdi_iodone();
extern void		pdi_iowait();
extern long		pdi_itoemaj();
extern void		pdi_logberr();
extern void		pdi_lognberr();
extern off_t		pdi_offset();
extern long		pdi_physck();
extern void		pdi_physio();
extern pdi_procp_t	pdi_procp();
extern long		pdi_sleep();
#define PDI_PCATCH	0400
#define PDI_PRIBIO	20
#define PDI_PZERO	25

extern long		pdi_splblk();
extern long		pdi_splchar();
extern long		pdi_splhi();
extern long		pdi_splx();
extern long		pdi_subyte();
extern long		pdi_suword();
extern time_t		pdi_time();
extern long		pdi_timeout();
extern void		pdi_ubase();
extern void		pdi_ucount();
extern void		pdi_untimeout();
extern void		pdi_uoffset();
extern void		pdi_ureturn();
extern paddr_t		pdi_vtop();
extern long		pdi_wakeup();
