/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_I214_H
#define _SYS_I214_H

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/sys/i214.h	1.3"

/****************************************************************************
 *
 * i214.h
 *	214/215/217/218 Driver declarations. Support for 214/215G only.
 *
 * Details of the 215/218 hardware may be found in Intel manual order
 * number 144780-002 (215)
 *
 ****************************************************************************/

/*******  Values needed but not defined in Unix 5.3 standard header files */
#define EBBHARD		128	/* Not in errno.h */
#define EBBSOFT		129

#define	SPL		 spl6		/* for driver mutex */
#define	NUMSPINDLE	 12		/* # spindles per board */
#define	FIRSTFLOPPY	 4		/* first removable unit-number */
#define NEXT_REMOVE_UNIT 4		/* increment to next removable unit */
#define FIRSTTAPE	 8 		/* first tape unit */
#define	FIXEDMASK  (FIRSTFLOPPY - 1)	/* mask for fixed-unit given unit */
#define	LOC_BUF		 1		/* local "sa" buffer from getablk */
#define	EXP_BUF		 0		/* exported buffer from getablk */
#define	ERROR		-1		/* error state, error return */


/*
 * Mapping of each minor device number to the following:
 *
 * Units 
 *	0-3	Winchester disk
 *	4-7	Floppy disk
 *	8-11	Tape drive
 *
 * Drtab's are selected per unit.  See i214cfg, below.
 *
 * Partitions are selected per drtab entry.  See i214cdrt, below.
 * Minor table gives the index into the selected partition table.
 *
 * "Board" defines the board number, and corresponding entry in i214cfg.
 */
struct	i214minor {
	unsigned partition:	8;	/* index into partition table */
	unsigned drtab:		4;	/* index into drtab */
	unsigned unit:		4;	/* unit number */
	unsigned board:		4;	/* board number */
};

#define	UNIT(dev)	(i214minor[getminor(dev)].unit)	/* dev->unit# map */
#define	DRTAB(dev)	(i214minor[getminor(dev)].drtab)	/* dev->drtab map */
#define	PARTITION(dev)	(i214minor[getminor(dev)].partition)/* dev->partition */
#define BOARD(dev)	(i214minor[getminor(dev)].board)	/* board number */
#define i214MINOR(bnum,unum,drnum,panum) ((bnum<<16)|(unum<<12)|(drnum<<8)|panum)	/* used in space.c */

#define BASEDEV(dev)	(makedevice(getmajor(dev), \
				i214bases[BOARD(dev)][UNIT(dev)]))

#define	LOW(x)		((x)&0xFF)		/* "low" byte */
#define	HIGH(x)		(((x)>>8)&0xFF)		/* "high" byte */


/*
 * Partition structure.  One per drtab[] entry.
 */
struct	i214part {
	ushort p_tag;                   /* ID tag (from vtoc.h) */
	ushort p_flag;                  /* permision flags (from vtoc.h) */
	ulong	p_fsec;			/* first sector */
	ulong	p_nsec;			/* number sectors */
};

/*
 * Per-board configuration.  One of these per 215/218/217 .
 * c_devcod indicates what kind of device/floppies/tape are there and
 * what type of board this is (215[AB], 215G ).
 *
 * The c_drtab field is a pointer to a list of drtab entries per-unit.  A zero
 * value implies non-existent unit.
 */
struct	i214cfg	{
	long		c_wua;			/* Physical Wake-Up Address */
	char		c_devcod[(NUMSPINDLE/FIRSTFLOPPY)];
	char		c_level;		/* what interrupt level */
	struct i214cdrt	*c_drtab[NUMSPINDLE];	/* per-spindle pointer to */
};						/* drive-characteristic table */

/*
 * Per-board driver "dynamic" data.
 */
struct	i214state {
	char		s_1st_init;	/* Do init sweep for ANY open */
	char		s_sstart;	/*  unit starting init sweep */
	char		s_exists;	/* flag that board exists */
	char		s_support;	/* level of support for 214 */
	char		s_state;	/* what just finished (for intr) */
	ushort		t_flags;	/* non-buffered flags */
	char		t_state;	/* tape state */
	char		s_opunit;	/* current unit being programmed */
	ushort		s_board;	/* board number */
	long		s_wua;		/* copy of i214cfg.c_wua */
	char		s_flags[NUMSPINDLE];	/* flags per spindle */
	char		s_devcod[NUMSPINDLE];	/* device-code for iopb */
	char		s_unit[NUMSPINDLE];	/* "unit" code for iopb */
	char		s_error[NUMSPINDLE];	/* status from nonbuffered op */
	struct iobuf	*s_bufh;	/* pointer to regular buffer queue */
	struct iobuf	*t_bufh;	/* pointer to tape buffer queue */
	struct buf	*rtfm_buf;	/* pointer to blk for RTFM state */
	char		*d_buf;		/* pointer to data buffer */
};

#define	i214XBSIZ	0x8000		/* size of segment */

#define	TP_GETBUF	1		/* allocate raw external buffers */
#define	TP_FREEBUF	2		/* deallocate raw external buffers */
#define	TP_RDBUF	3		/* write to raw external buffer */
#define	TP_WRBUF	4		/* write to raw external buffer */
#define	TP_FLUSH	5		/* flush raw external buffers */
#define	TP_INITBUF	6		/* init raw external buffers */

#define	BP_ENQUE(q, b) { \
	int qmk = splbuf(); \
	(b)->av_forw = 0; \
	if((q)->b_actf == 0) \
		(q)->b_actf = b; \
	else \
		((q)->b_actl)->av_forw = b; \
	(q)->b_actl = b; \
	splx(qmk); \
};

#define	BP_DEQUE(q, b) { \
	int qmk = splbuf(); \
	if((b = (q)->b_actf) != 0) { \
		if(((q)->b_actf = (b)->av_forw) == 0) \
			(q)->b_actl = 0; \
		(b)->av_forw = 0; \
	} \
	splx(qmk); \
};

/*
 * Per-Unit State Flags.
 */
#define	SF_OPEN		0x01	/* unit is open */
#define	SF_READY	0x02	/* unit is ready; reset by media-change */
#define SF_VTOC_OK      0x04    /* a valid VTOC has been read for this unit */
#define SF_OPENING      0x08    /* open (VTOC reading) in progress */

/*
 * Per-board tape Flags
 */
#define TF_NO_BUFFER	0x01	/* means that no buffer is present */
#define TF_LONG_TERM	0x02	/* operation just invoked is long-term */
#define TF_WAIT_SECOND	0x04	/* waiting for 2nd int from long-term op */
				/* tape operations had better wait */
#define TF_IM_WAITING	0x08	/* has left sleep for first interrupt */
#define TF_LT_DONE	0x10	/* don't bother sleeping; second interrupt */
				/* has already happened */
#define TF_READING_TO_FM	0x20	/* Reading to filemark */
#define TF_WANTED	0x80	/* non-buffered request is waiting for a */
				/* tape to finish a long-term operation */
#define TF_FOUND_FM	0x100	/* File mark encountered */
#define TF_FM_ALWAYS	0x200	/* Always set TF_FOUND_FM when a */
								/* is encountered.  This is used by */
								/* the internal buffering. */
#define TF_AT_LEOT	0x400	/* we are at the logical end-of-tape */


/*
 * Macros to make things easier to read/code/maintain/etc...
 */
#define	IS220(dd)	((dd)->d_state.s_devcod[0] == DEV220)
#define	IS214(dd)	((dd)->d_state.s_devcod[0] == DEVWINIG)
#define	IO_OP(bp)	(((bp)->b_flags & B_READ) ? READ_OP : WRITE_OP)
#define ISTAPE(dd,unit) ((dd)->d_state.s_devcod[unit] == STREAMER)
#define NOT_BTAPE(dev)	(UNIT((dev)) < FIRSTTAPE)

#define ISWINI(dev)	(UNIT(dev) < FIRSTFLOPPY)
#define ISFLOP(dev)	((!ISWINI(dev)) && (NOT_BTAPE(dev)))


/*
 * 214 Wake-Up Block.  Lives at wakeup-address, points at CCB.
 */
struct	i214wub {
	char		w_sysop;	/* Must == 0x01 */
	char		w_rsvd;		/* reserved */
	ushort 		w_ccb;		/* "offset" of CCB pointer */
	ushort		w_ccb_b;	/* "base" == Kernel DS >> 4 */
};

/*
 * CCB (Channel Control Block).  See 214 manual.
 */
struct	i214ccb {
	char		c_ccw1;		/* 1 ==> Use 214 Firmware */
	unchar		c_busy1;	/* 0x00 ==> Idle, 0xFF ==> busy */
	ushort 		c_cib;		/* "offset" of CIB pointer */
	ushort		c_cib_b;	/* "base" == Kernel DS >> 4 */
	ushort		c_rsvd0;	/* reserved */
	char		c_ccw2;		/* Must == 0x01 */
	char		c_busy2;	/* Not useful to Host */
	ushort		c_cpp;		/* -> i214ccb.c_cp (offset)  */
	ushort		c_cpp_b;	/* "base" == Kernel DS >> 4 */
	ushort		c_cp;		/* Control Pointer == 0x04 */
};

/*
 * CIB (Controller Invocation Block).  See 214 manual.
 */
struct	i214cib {
	char		c_cmd;		/* reserved */
	char		c_stat;		/* Operation Status (see below) */
	char		c_cmdsem;	/* Not used by 214 */
	char		c_statsem;	/* 0xFF ==> new status avail */
	ushort		c_csa[2];	/* 214 Firmware; MUST == 0 */
	ushort	 	c_iopb;		/* IOPB pointer offset */
	ushort		c_iopb_b;	/* "base" == Kernel DS >> 4 */
	ushort		c_rsvd1[2];	/* reserved */
};

/*
 * IOPB (I/O Parameter Block).  See 214 manual.
 */
#ifndef lint
#pragma pack(1)
#endif
struct	i214iopb {
	ushort		i_rsvd[2];	/* reserved */
	ulong		i_actual;	/* actual transfer count */
	ushort		i_device;	/* Device Code (see below) */
	char		i_unit;		/* Unit: <4> == fixed/rem, <1,0> == unit # */
	char		i_funct;	/* Function Code (see below) */
	ushort		i_modifier;	/* Modifier.  0 ==> normal, interrupt */
	ushort		i_cylinder;	/* starting cylinder # */
	char		i_head;		/* starting head # */
	char		i_sector;	/* starting sector # */
	ulong		i_addr;		/* buffer address */
	ulong		i_xfrcnt;	/* Requested Transfer Count */
	ulong		i_gaddr_ptr;	/* general address ptr (not used) */
};
#ifndef lint
#pragma pack()
#endif

/*
 * Drive-Data Table (used to initialize drives).  See 214 manual.
 * Because of C alignment problem on secsiz, it must be entered bytewise.
 * Fields through dr_nalt are programmed into controller for an init (disk);
 * tapes only use first byte.  Other fields are for internal driver use.
 * The i214cdrt structure is for static initialization of data.  It has
 * to be moved into the drtab so it will be aligned the way the controller
 * wants it.
 */
struct	i214drtab {
	ushort		dr_ncyl;	/* # cylinders */
	unchar		dr_nfhead;	/* # fixed heads (Winchester) */
	unchar		dr_nrhead;	/* # removable heads (floppy) */
	unchar		dr_nsec;	/* # sectors per track */
	unchar		dr_lsecsiz;	/* "low" of sector-size */
	unchar		dr_hsecsiz;	/* "high" of sector-size */
	unchar		dr_nalt;	/* # alternate cylinders */
					/* if floppy, 0==FM, 1==MFM */
	ushort		dr_spc;		/* actual sectors/cylinder */
	ushort          dr_lbps;        /* logical blocks (512b) / sector */
	ushort		dr_secsiz;	/* sector-size (bytes) */
	struct i214part	*dr_part;	/* partition table pointer */
	char		dr_pnum;	/* number of partitions */
	struct alt_info *dr_altptr;    /* points at alternates table */
};

struct	i214cdrt {
	ushort		cdr_ncyl;	/* # cylinders */
	char		cdr_nfhead;	/* # fixed heads (Winchester) */
	char		cdr_nrhead;	/* # removable heads (floppy) */
	char		cdr_nsec;	/* # sectors per track */
	ushort		cdr_secsiz;	/* sector-size */
	char		cdr_nalt;	/* # alternate cylinders */
	struct i214part	*cdr_part;	/* partition table pointer */
	char		cdr_pnum;	/* number of partitions */
};

#define	dr_flags	dr_nalt
#define	cdr_flags	cdr_nalt
#define	DR_NO_REWIND	1
#define DR_READ_TO_FM	2

/*
 * Error Status-Structure, Returned on status inquiry.  See 214 manual.
 * Note another alignment problem
 */
struct	i214err {
	ushort		e_hard;		/* Hard Error Status (see below) */
	char		e_soft;		/* soft error status */
	char		e_req_cyl_l;	/* desired cylinder - low byte */
	char		e_req_cyl_h;	/* desired cylinder - high byte */
	char		e_req_head;	/* desired head and volume */
	char		e_req_sec;	/* desired sector */
	char		e_act_cyl_l;	/* act cylinder - low byte */
	char		e_act_cyl_h;	/* act cylinder - high byte, flags */
	char		e_act_head;	/* actual head & volume */
	char		e_act_sec;	/* actual sector */
	char		e_retries;	/* # retries attempted */
};

/***********************************************************************
	i025 DEBUGGING: this is the structure which is used
			to fake bad blocks for testing.
 */
struct i214_bb_fake {
	daddr_t		f_block_num;	/* block number for fake */
	dev_t		f_device;	/* device num (maj/min) for fake */
	char		f_serror;	/* to be put in s_error field */
	ushort		f_hard_soft;	/* to be put in e_hard or e_soft,
						depending on what is in
						serror.
					 */
};

/*
	end of i025 DEBUGGING
************************************************************************/

/*
 * Tape status access defines.
 */
#define e_no_data	e_act_cyl_h	/* no data detected */
#define e_leot		e_req_sec	/* logical end of tape */
#define e_fm_found	e_req_head	/* file mark detected */
#define e_llp		e_req_cyl_h	/* at logical load point */
#define e_bot		e_req_cyl_l	/* at beginning of tape */

/*
 * Format Structure.  1 per "board"
 * i214ftk is the argument structure to the format ioctl.
 */
struct	i214format {
	char	f_trtype;		/* format track-type code */
	char	f_pattern[4];		/* pattern; depends on f_trtype */
	char	f_interleave;		/* interleave-factor */
	ulong	f_secno;		/* absolute sector # */
};

struct	i214ftk	{
	ushort	f_track;		/* Absolute track # */
	ushort	f_intl;			/* interleave factor */
	ushort	f_skew;			/* track skew -- ignored by 214 */
	unchar	f_type;			/* format type-code */
	unchar	f_pat[4];		/* pattern data */
};

/*
 * 214 device parameter structure.
 */
struct	i214dp {		/* Partition description (16 bytes) */
	ushort		ncyl;		/* # cylinders */
	char		nfhead;		/* # fixed heads (Winchester) */
	char		nrhead;		/* # removable heads (floppy) */
	char		secptk;		/* # sectors per track */
	char		nalt;		/* # alternate cylinders */
	ushort		secsiz;		/* sector-size */
	daddr_t		fstsec;		/* First sector */
	daddr_t		numsec;		/* Number of sectors */
};

/*
 * 214 Disk Driver Extensions.  One per board.  Contains information
 * about the transfer in progress for disks only.
 */
struct  i214ext {
	ulong   xfer_addr;      /* current transfer address */
	ulong   xfer_count;     /* current transfer count */
};

/*
 * 214 Per-Board Device-Data.  One per board.
 */
struct	i214dev {
	struct  i214ext         d_ext;
	struct	i214state	d_state;
	struct	i214ccb		d_ccb;
	struct	i214cib		d_cib;
	struct	i214iopb	d_iopb;
	struct	i214drtab	d_drtab[NUMSPINDLE];
	struct	i214err		d_error;
	struct	i214format	d_format;
	struct	i214ftk		d_ftk;
	struct	iotime		d_iot; /* system accounting information */
};

/*
 * Misc wini data. One entry per wini.
 */
struct i214winidata {
	unsigned int	basedev;		/* Minor # of Part 0 for this wini */
	unsigned int	ivlabloc;		/* Disk location (Abs byte) of IVLAB */
	unsigned int	ivlablen;		/* Length of IVLAB */
	unsigned int	pdinfoloc;
	unsigned int	pdinfolen;		/* PDINFO disk location and length. */
	unsigned int	vtocloc;	
	unsigned int	vtoclen;		/* VTOC disk location and length. */
	unsigned int	altinfoloc;	
	unsigned int	altinfolen;		/* ALT INFO disk location and length. */
	unsigned int	st506mdlloc;	/* ST506 MDL location	*/
	unsigned int	st506mdllen;	/* and length.			*/
	unsigned int	esdimdlloc;		/* ESDI MDL location	*/
	unsigned int	esdimdllen;		/* and length.			*/
	unsigned int	isesdi;			/* If this is ESDI drive or not. */
	/*
	 * Pointers to tempoarary copies of the data structures.
	 * Normally, these pointers are NULL, but are filled in 
	 * when temporary copies of these structures are nedded.
	 * Before the pointers are used, the lock must be set 
	 * and some memory must be allocated.
	 */
	unsigned int	lock;		/* Semaphore */
	unsigned int	pgcnt;		/* Number of pages allocated. */
	caddr_t	pgaddr;				/* Address of allocated pages. */
	struct ivlab	*ivlab;
	struct pdinfo	*pdinfo;
	struct vtoc	*vtoc;
	struct alt_info	*altinfo;
	union	esdi506mdl	*mdl;
	};

/*
 * Values of buffer-header b_active, used for mutual-exclusion of
 * opens and other I/O requests.
 */
#define	IO_IDLE		0		/* idle -- anything goes */
#define	IO_BUSY		1		/* something going on */
#define	IO_WAIT		2		/* waiting for controller to be idle */

/*
 * Values of i214state.s_state, internal driver state.
 */

#define	NOTHING		0	/* normal situation */
#define	GET_BAD_STATUS	1	/* retrieving status on hard error */
#define	RESTORING	2	/* recalibrating to track 0 for retry */
#define	INITIALIZING	3	/* going through init-sweep */
#define	READING_LABEL	4	/* reading device label - unused */
#define FORMAT0		5	/* unused */
#define FORMAT1		6       /* unused */
#define FORMAT2		7       /* unused */
#define FORMAT3		8       /* unused */
#define T_INIT		9	/* initializing 217 controller */
#define T_RESET		10	/* resetting tape drive */
#define T_SOFT_STATUS	11	/* soft status check (for TS_READING) */
#define T_READING_TO_FM	12	/* dumping data to get out of read mode */
#define T_RTFM_STATUS	13	/* status check in T_READING_TO_FM */
#define WRITEFM		14	/* handle end of media		*/
#define RVENDLIST		15	/* Read vendor defect list */


/*
 * tape state variables for state.t_state.
 */
#define TS_READING	1	/* tape device engaged in a read op */
#define TS_WRITING	2	/* tape device engaged in a write op */


/*
 * IOPB fields/flags definitions.
 */
#define	UNIT_REMOVABLE		0x10	/* ==> removable unit */

/*
 * 214 Wake-up command codes.  These get output to the wakeup-address-port.
 */
#define	WAKEUP_CLEAR_INT	0x00
#define	WAKEUP_START		0x01
#define	WAKEUP_RESET		0x02

/*
 * 214 IOPB Command Codes.
 */
#define INIT_OP                 0x0
#define	STATUS_OP		0x1
#define	FORMAT_OP		0x2
#define	READ_ID_OP		0x3	/* not used */
#define	READ_OP			0x4
#define	VERIFY_OP		0x5	/* not used */
#define	WRITE_OP		0x6
#define	WRITE_BUFFER_OP		0x7	/* not used */
#define	SEEK_OP			0x8	/* not used */
#define	READ_VENDLIST_OP	0x9	/* Read Vendor defect list */
#define DIAGNOSTIC_OP		0xF	/* for RESTORING state */

/*
 * IOPB commands for tape only. These commands work only
 * on the iSBC 214 series of controller boards which supports
 * the iSBX 217 tape controller.
 *
 * l.t. (long term command)
 * s.t. (short term command)
 */
#define TAPEINIT_OP		0x10	/* s.t. initialize 217 firmware */
#define REW_OP			0x11	/* l.t. tape rewind */
#define SFFM_OP			0x12	/* l.t. forward a file mark */
#define SBFM_OP			0x13	/* l.t. backward a file mark not used */
#define WRFM_OP			0x14	/* s.t. write filemark */
#define ERASETAPE_OP		0x17	/* l.t. erase tape (format command) */
#define LOADTAPE_OP		0x18	/* l.t. tape to logical load point */
#define UNLOADTAPE_OP		0x19	/* l.t. tape to physical end of tape not used */
#define SFREC_OP		0x1A	/* s.t. forward a record not used */
#define SBREC_OP		0x1B	/* s.t. backward a record not used */
#define TAPERESET_OP		0x1C	/* s.t. reset tape drive */
#define RETTAPE_OP		0x1D	/* l.t. retension tape */
#define TAPE_STATUS_OP		0x1E	/* s.t. get long term status info not used */
#define R_W_TERMINATE		0x1F	/* terminate read/write command */


/*
 * 214 IOPB Modifier Bits.
 */
#define	MOD_NO_INT		0x0001	/* no interrupt */
#define	MOD_NO_RETRY		0x0002	/* no retry attempts */
#define	MOD_DELETED_DATA	0x0004	/* 218 deleted-data RW - not used */
#define	MOD_24_BIT		0x0010	/* 214 24-bit address mode */
#define MOD_NO_CLEAR		0x0020	/* 214 no clear the ram on init bit */
#define MOD_LT_STATUS		0x0040	/* 214 tape status for long command */
#define MOD_RECAL		0xFF00	/* to use diagnostic recalibrate cmd */

/*
 * Device Codes (for iopb.i_device).
 */
#define	DEVWINI		0		/* Wini */
#define	DEV8FLPY	1		/* 8" 218 Floppy */
#define	DEV220		2		/* 220 */
#define	DEV5FLPY	3		/* 5.25" 218 Floppy */
#define STREAMER	4		/* streamer tape (QIC-2)  */
#define STARTSTOP	5		/* start/stop tape (Kennedy) not used */
#define DEVWINIG	8		/* 214 type, DEVWINI + 8 */
#define DEVMASK		7		/* mask to get actual device */
#ifdef INVALID
#undef INVALID
#endif
#define INVALID		0xF		/* invalid device code */


/*
 * Floppy FM/MFM codes for drtab[*].nalt.
 */
#define	FLPY_FM		0		/* FM -- single density */
#define	FLPY_MFM	1		/* MFM -- double density */
#define	FLPY_HD		7		/* HD -- high density */

/*
 * Operation Status Bits.  Returned by controller in i214cib.c_stat.
 *
 * Note: the 214 controller has two additional bit patterns
 * for tape identification. 0x?f for tape long term command complete
 * and 0x?e for media change. Since we will need to treat long term
 * command completion diffently any way it has been defined as a byte mask
 * instead of a bit mask.
 */		
#define	ST_OP_COMPL		0x01	/* immediate operation complete */
#define	ST_SEEK_COMPL		0x02	/* seek complete */
#define	ST_MEDIA_CHANGE		0x04	/* media changed */
#define	ST_MCHANGE_MASK		0x05	/* I015 media change mask */
#define	ST_COMPL_MASK		0x07	/* I015 operation complete mask */
#define	ST_FLOPPY		0x08	/* ==> 218 floppy; possibly 217 tape */
#define	ST_UNIT			0x30	/* unit mask */
#define	ST_HARD_ERR		0x40	/* 0 ==> was soft, recovered error */
#define	ST_ERROR		0x80	/* summary error - can read status */
#define ST_TAPE_MEDIA		0x0E	/* I004 tape media change detected */
#define ST_LONG_COMPL		0x0F	/* I004 tape long term complete */

/*
 * Error Bits.
 *
 * Errors returned to user in b_error (byte).  Error is either soft-status
 * byte, or high-byte of hard-status byte.  b_error needs to be a word,
 * and can be used as:
 *	Bits	Contents
 *	 6-0	EIO
 *	  7	0 ==> Hard, 1 ==> Soft status
 *	15-8	High-order byte of hard status, or soft status byte.
 *	(just like iRMX 86; huh, guys?  puck)
 *
 * I017 - additional bit definitions.
 */
#define HARD_214_REJECT		0x0001
#define HARD_214_RAM_ERR	0x0008
#define HARD_214_ROM_ERR	0x0010
#define HARD_LT_IN_PROGRESS	0x0020
#define HARD_FORMAT_TYPE	0x0040
#define HARD_END_OF_MEDIA	0x0080
#define HARD_ILL_SEC_SIZE	0x0100
#define HARD_DIAG_FAULT		0x0200
#define HARD_NO_INDEX		0x0400
#define HARD_INVALID_FUNC	0x0800
#define	HARD_NO_SECTOR		0x1000
#define HARD_INVALID_ADDR	0x2000
#define	HARD_NOT_READY		0x4000
#define	HARD_WRITE_PROT		0x8000

#define SOFT_DATA_CRC		0x08
#define SOFT_ID_CRC		0x10
#define SOFT_DRIVE_FAULT	0x20
#define SOFT_CYL_ADDR_MISC	0x40
#define SOFT_SEEK_ERR		0x80

/*
 * I017
 * Error bits defined for tapes;
 * see 214 HRM or i214harderr for descriptions.
 */
#define HARD_217_REJECT		0x0002
#define HARD_DRIVE_REJECT	0x0004
#define HARD_217_ROM_ERR	0x0010
#define HARD_CONFIGURATION	0x0040
#define HARD_LENGTH_ERR		0x0100
#define HARD_TIME_OUT		0x0400
#define HARD_NO_CARTRIDGE	0x1000

#define SOFT_TAPE_ERROR		0x02
#define SOFT_CABLE_CHECK	0x04
#define SOFT_DATA_ERROR		0x08
#define SOFT_OVER_UNDER_RUN	0x40

/*
 * Misc Format definitions, for i214ftk.f_type.
 */
#define	FORMAT_DATA		0x00	/* Format data track */
#define	FORMAT_BAD		0x80	/* Format bad track */
#define	FORMAT_ALTERNATE	0x40	/* Format alternate track */

/*
 * iSBC 214 ioctl mnemonics.
 */
#define	I214_IOC_FMT		(('W'<<8)|0)
#define I214_IOC_RDC		(('W'<<8)|32)

/*
 * I012
 * Division ID for firmware version number recognition on 214.
 */
#define ISO_S	0x1
#define OMO	0x2
#define ISO_N	0x3

/*
 * I019
 * Definitions for firmware version usage.
 */
#define NOSWEEP_SPT(dd)		((dd)->d_state.s_support >= 1)
#define A24BIT_SPT(dd)		((dd)->d_state.s_support >= 3)
#define TAPE_SPT(dd)		((dd)->d_state.s_support >= 4)

#endif	/* _SYS_I214_H */
