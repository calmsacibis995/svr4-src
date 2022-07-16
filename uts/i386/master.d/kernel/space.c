/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */

#ident	"@(#)master:kernel/space.c	1.3.3.2"

#define BUFDEFINE
#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/immu.h"
#include "sys/map.h"
#include "sys/proc.h"
#include "sys/vnode.h"
#include "sys/fs/s5inode.h"
#include "sys/file.h"
#include "sys/cred.h"
#include "sys/callo.h"
#include "sys/var.h"
#include "sys/mount.h"
#include "sys/swap.h"
#include "sys/class.h"
#include "sys/tuneable.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/sysinfo.h"
#include "sys/tty.h"
#include "sys/conf.h"
#include "sys/utsname.h"
#include "sys/sema.h"
#include "sys/acct.h"
#include "sys/pfdat.h"
#include "sys/weitek.h"
#include "sys/stream.h"
#include "sys/sysmacros.h"
#include "sys/resource.h"
#include "sys/x.out.h"		/* XENIX Support */
#include "sys/bootinfo.h"
#include "vm/page.h"
#include "sys/rt.h"
#include "sys/ts.h"

#include "sys/emap.h"		/* channel mapping include file from XENIX */


#define SYS	"UNIX_System_V"
#define XSYS	"UNIX_Sys"	/* For XENIX; must be <= 8 chars */
#define NODE	"unix"
#define REL	"4.0"
#define VER	"3.0"
#define MACH	"i386"
#undef ARCHITECTURE
#ifdef MB1
#define ARCHITECTURE	"386/MB1"
#endif
#ifdef MB2
#define ARCHITECTURE	"386/MB2"
#endif
#ifndef ARCHITECTURE
#define ARCHITECTURE	"386/AT"
#endif
#define HW_SERIAL	"0"
#define HW_PROVIDER	"AT&T"
#define SRPC_DOMAIN	""

/* Streams */

#define RESRVD	""
#define ORIGIN	1
#define OEMNUM	0
#define SERIAL	0

#include "config.h"	/* to collect all other tunable parameters */

/*
 * Tables and initializations
 */

extern	int	oeminit(),
		cinit(),
		vfsinit(),
		aioinit(),
		seminit(),
		msginit(),
		strinit(),
		xsdinit(),
		xseminit();	
extern void	binit();
extern void	finit();
int	(*init_tbl[])() = {
		oeminit,
	  	cinit,
	  	(int (*) ()) binit,
	  	vfsinit,
	  	(int (*) ()) finit,
		seminit,
		msginit,
#ifndef NOSTREAMS
	  	strinit,
#endif
		xsdinit,
		xseminit,
	  	0,
};




struct	buf	pbuf[NPBUF];
struct	hbuf	hbuf[NHBUF];
time_t	inoblkltm[NINODE];		/* coming out ? */
struct	callo	callout[NCALL];
struct	map	sptmap[SPTMAP];
struct	map	piomap[PIOMAP];
int	piomapsz = PIOMAP;
int piomaxsz = PIOMAXSZ;
char		putbuf[PUTBUFSZ];
int		putbufsz = PUTBUFSZ;
int		sanity_clk = SANITYCLK;
u_int		pages_pp_maximum=PAGES_UNLOCK;
struct	var	v = {
		NBUF,
		NCALL,
		NPROC,
		0,
		0,
		MAXCLSYSPRI,
		NCLIST,
		MAXUP,
		NHBUF,
		NHBUF-1,
		NPBUF,
		SPTMAP,
		MAXPMEM,
		NAUTOUP,
		BUFHWM,
		NSCRN,
		NEMAP,
		NUMSXT,
		XSDSEGS,
		XSDSLOTS
};

struct	rlimit	rlimits[] = {
		SCPULIM,
		HCPULIM,
		SFSZLIM,
		HFSZLIM,
		SDATLIM,
		HDATLIM,
		SSTKLIM,
		HSTKLIM,
		SCORLIM,
		HCORLIM,
		SFNOLIM,
		HFNOLIM,
		SVMMLIM,
		HVMMLIM};

/* add overflow lists for sched and pageout */
struct buf pgoutbuf[PGOVERFLOW];
struct buf notpgoutbuf[NOTPGOVERFLOW];
int npgoutbuf = PGOVERFLOW;
int nnotpgoutbuf = NOTPGOVERFLOW;

/*
 *	Data structures for dma_pageio_breakup().
*/
#define DMA_IO_SLOTS 10
struct buf *dma_iobp[DMA_IO_SLOTS];		/* Buffer headers to use for I/O */
struct page *dma_iofp[DMA_IO_SLOTS];	/* Next page ptr for I/O */
struct page *dma_iopp[DMA_IO_SLOTS];	/* Next page ptr for I/O */
struct page *dma_iodma[DMA_IO_SLOTS];	/* dmaable pages for I/O */
int dma_ioreq[DMA_IO_SLOTS];			/* Next page no. for I/O */
int dma_io_slots = DMA_IO_SLOTS; 

/*  Scheduler initialization */
#define INITCLASS 	"TS"

char	intcls[]= {INITCLASS};
char	*initclass=intcls;

extern void sys_nullclass();

extern struct classfuncs sys_classfuncs;


struct	tune	tune = {
		GPGSLO,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		FDFLUSHR,
		MINARMEM,
		MINASMEM,
		MAXDMAPAGE,
		FLCKREC,
		MINAKMEM
};

int	minpagefree = MINPAGEFREE;
int	Dstflag = DSTFLAG;	/* tuneable daylight time flag for XENIX */
int	Timezone = TIMEZONE;	/* tuneable time zone for XENIX ftime() */
int	dma_single = DMAEXCL;
int	Hz = HZ;

int 	exec_ncargs = {ARG_MAX};
struct	flckinfo flckinfo = {0, 0};
struct	shlbinfo shlbinfo = {SHLBMAX, 0, 0};
struct	utsname	utsname = {
		SYS,
		NODE,
		REL,
		VER,
		MACH
};

struct	xutsname xutsname = {
		XSYS,
		NODE,
		REL,
		VER,
		MACH,
		RESRVD,
		ORIGIN,
		OEMNUM,
		SERIAL
};
/* sysinfo for 4.0 */
char architecture[] = ARCHITECTURE;
char hw_serial[] = HW_SERIAL;
char hw_provider[] = HW_PROVIDER;
char srpc_domain[] = SRPC_DOMAIN;

/* Line Discipline Switch Table
 * order: open close read write ioctl rxint txint modemint
 */
extern int 	ttopen(),
		ttclose(),
		ttread(),
		ttwrite(),
		ttioctl(),
		ttin(),
		ttout(),
		nulldev();

extern	int	xtin(),
		xtout(),
		sxtin(),
		sxtout();

struct	linesw	linesw[] = {
/* 	tty ------------- */
			 ttopen,
			 ttclose,
			 ttread,
			 ttwrite,
			 ttioctl,
			 ttin,
			 ttout,
			 nulldev,
/*              xt ------------- */
			 nulldev,
			 nulldev,
			 nulldev,
			 nulldev,
			 nulldev,
			 xtin,
			 xtout,
			 nulldev,
/*		sxt -------------*/
			 nulldev,
			 nulldev,
			 nulldev,
			 nulldev,
			 nulldev,
			 sxtin,
			 sxtout,
			 nulldev,
 };
int     linecnt = {3};


/* STREAMS */

int		nstrpush = {NSTRPUSH};
int		strmsgsz = {STRMSGSZ};
int		strctlsz = {STRCTLSZ};
int		strthresh = {STRTHRESH};

struct	acct	acctbuf;
struct	vnode	*acctp;
struct	buf	bfreelist;	/* Head of the free list of buffers */
struct	pfree	pfreelist;	/* Head of physio buffer headers    */
int		pfreecnt;	/* Count of free physio buffers.    */
struct vnode	*rootdir;	/* Inode for root directory. */

int	nptalloced;	/* Total number of page tables allocated.	*/
int	nptfree;	/* Number of free page tables.			*/

proc_t	*runq;		/* Head of linked list of running procs. */
proc_t	*curproc;	/* The currently running proc.		 */
proc_t	*old_curproc;	/* Previous curproc */
proc_t	*oldproc;	/* The previous proc, if it exited.	 */
int	curpri;		/* Priority of currently running proc.	 */

struct pfdat	phead;		/* Head of free page list. */
struct pfdat	*pfdat;		/* Page frame database. */
struct pfdat	**phash;	/* Page hash access to pfdat. */
struct pfdat	ptfree;		/* Head of page table free list. */
struct pfdat	dbdfree;	/* Head of DBD table free list. */
int		phashmask;	/* Page hash mask. */

struct sysinfo sysinfo;
struct	syswait	syswait;
struct	syserr	syserr;
struct	vminfo	vminfo;		/* VM stats	*/
struct	rtminfo	rtminfo;	/* Real Time stats */
struct	kmeminfo kmeminfo;	/* KMA stats */
struct  minfo minfo;

#include	"sys/fs/s5macros.h"

vnodeops_t *rf_vnopsp;	/* RFS vnode op vector (initialized by rf_init) */

struct vnode *rootvp = (struct vnode *) NULL;

/* Default fstype for root is "s5".  This can be overridden with
   the "rootfstype" parameter in /stand/boot. */
#define	ROOTFSTYPE	"s5"
char rootfstype[ROOTFS_NAMESZ+1] = {ROOTFSTYPE};

/* parameters for remote (RFS) network access (which uses buffers from 
 * local buffer pool) */
unsigned long lbuf_ct;
unsigned long rbuf_ct;
unsigned long nrbuf;
unsigned long nlbuf;
unsigned long maxbufage = 0;
int	rcache_enable = 0;
int	rcacheinit = 0;		/* RFS client caching initialized*/
unsigned long rfs_vcode = 1;	/* version code for RFS caching */

int	rf_state;


int	nextswap;
daddr_t swplo = 0;
daddr_t nswap;  /* set by disk driver when mounting root fs to size of swap */


/*	Each process has 3 pregions (text, data, and stack) plus
 *	enough for the maximum number of shared memory segments.
 *	We also need one extra null pregion to indicate the end
 *	of the list.  The maximum number of shared memory segments
 *	will be added in reginit.
 */

int	pregpp = 3 + 1;


/*	The following describe the physical memory configuration.
**
**		maxclick - The largest physical click number.
**			   ctob(maxclick) is the largest physical
**			   address configured plus 1.
**
**		physmem	 - The amount of physical memory configured
**			   in clicks.  ctob(maxclick) is the amount
**			   of physical memory in bytes.
**		kpbase	 - The physical address of the start of
**			   the allocatable memory area.  That is,
**			   after all kernel tables are allocated.
**			   Pfdat table entries are allocated for
**			   physical memory starting at this address.
**			   It is always on a page boundary.
*/

int	maxclick;
int	physmem;


/*	The following are concerned with the kernel virtual
**	address space.
**
**		kptbl	  - The address of the kernel page table.
**			    It is dynamically allocated in startup.c
**		piownptbl - Points to the page table for the pio windows.
**			    It is dynamically allocated in startup.c
**		usertable - The address of the page table used to map
**			    the current u block.
*/

pte_t	*kptbl;
pte_t	*piownptbl;
pte_t	*usertable;

/*	The following space is concerned with the configuration of
**	NOFILES.  We have to do it this hard way because the code
**	which actually does this is in the assembly language file
**	misc.s and can't include header files or conveniently
**	print things.
**
**	The variable nofiles_cfg is assigned a value here to force
**	it to be in the data region rather than bss.  The value
**	will be overwritten by the code at vstart_s in misc.s.
**	However, this occurs before bss is cleared so that if
**	this variable were in bss, the value written by vstart_s
**	would be cleared in mlsetup when bss is cleared.
*/

/*  The following are no longer supported in SVR 4.0
int	nofiles_min = NOFILES_MIN;
int	nofiles_max = NOFILES_MAX;
 */

int	nofiles_cfg = 1;		/* Originally configured value. */

int	ngroups_max = NGROUPS_MAX;

int	maxminor = MAXMINOR;

int	eua_lim_ma = 2;
char	fp_kind;

/*
**	Switch to turn on/off 386 B1 stepping workarounds.
**	The variables do386b1, do386b1_387, and do386b1_x87 are all controlled
**	by the DO386B1 tuneable.
**	The do386b1_387 variable will be turned off if there is no 80387 present,
**	in order to disable workarounds which are only needed with a 387.
**	Similarly, do386b1_x87 will be turned off if there is neither an 80287
**	nor an 80387.
**	The DO386B1 tuneable takes 3 values:
**		0: disable workarounds
**		1: always enable workarounds
**		2: auto-detect B1 stepping; enable workarounds if needed
*/
int	do386b1 = DO386B1;
int	do386b1_387;	/* copied from do386b1 if 387 present */
int	do386b1_x87;	/* copied from do386b1 if 287 or 387 present */
/*
**	DO387CR3 enables the workaround for the 386 B1 stepping errata #21.
**	Like do386b1_x87, it will be turned off if do386b1 is off or there is
**	no math chip.  It needs to be separate since some hardware can't
**	support this workaround.
*/
int	do387cr3 = DO387CR3;

/*	The following describe the minimum conditions required
**	for putting the stack below data and starting it in
**	the data region's page table.
*/
int minhidustk = ctob(MINHIDUSTK);
int minustkgap = ctob(stoc(MINUSTKGAP));

/* The mapping table for discontiguous chunks of user free physical memory.
*/
struct pageac pageac_table[B_MAXARGS];
struct page *pages = (struct page *) NULL;      /* 1st page structure */
struct page *epages = (struct page *) NULL;     /* Last page structure */

int	ncsize=NPROC+100;

int	rstchown= RSTCHOWN;

struct	emap	emap[NEMAP];		/* channel mapping data struct table */


/* Another stub */
ssinvalidate(){}


/* spl invalidation */
int	splvalid_flag=0;


unsigned int	dmaable_pages = DMAABLEBUF;	/* Pages reserved in DMA page pool */
unsigned int	dmaable_free = DMAABLEBUF;	/* Current free pages in this pool */
int		ifstats;			/* head of inet statistics list */
int piosegsz = PIOSEGSZ;		/* map sizes in pages for pio   */
int syssegsz = SYSSEGSZ;	/*	sysseg			*/
int segmapsz = SEGMAPSZ;	/*	segmax			*/
