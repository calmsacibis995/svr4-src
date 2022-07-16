/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_I258_H
#define _SYS_I258_H

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/i258.h	1.3.3.3"

/*
 * NOTE: For some structures, we keep NUMSPINDLE instances instead of NUMUNITS
 *  	 because otherwise we will be wasting lot of memory. 
 * 		 Modify NUMUNIT to reflect the maximum number of a any type
 * 		 of device.
 */
#define NUMTYPE            3
#define NUMUNIT            4
#define NUMSPINDLE         NUMTYPE * NUMUNIT

#define MAXLUNS            8					/* Max. Logical units on SCSI */
#define MAXTIDS            8					/* Max. Targets on SCSI */
#define NUMUNITS		   MAXLUNS * MAXTIDS	/* Max unit number */
#define MAXREQ             64
#define MAXTAPEPAGES       64        /* 64 pages = 256k */
#define I258_BOARD_PORT    0x5491    /* Port addr to talk to boards */
#define	PCI_PORT_ID	   0x520     /* Port id of the PCI server */
#define WATCH_TIME         10        /* Check timeout every 10 seconds. */
#define I258_NUMWINI    NUMUNIT
#define I258_NUMTAPE    2

#define WINI_TYPE		1
#define FLOPPY_TYPE		2
#define TAPE_TYPE		3

/* #define B_TAPE        2               Not in iobuf.h */
#define splbuf        spl6
#define TAPE_DEV_GRAN 0x200
 
/*
 * TID_MASK is used to get TID, given the unit number.
 */
#define	TID_MASK	0x7

#define    SPL        spl6           /* for driver mutex */
#define    ERROR      -1             /* error state, error return */

#define I258_SET_CHAR_SIZE    			256
#define I258_QUERY_CONTROLLER_SIZE    	256
/*
 * Minor table gives the index into the selected partition table.
 *
 * "Board" defines the board number, and corresponding entry in i258cfg.
 */
struct    i258minor {
    unsigned char partition;    /* index into partition table */
    unsigned char drtab;    	/* index into drtab */
    unsigned char unit;    	/* unit number */
    unsigned char board;    	/* board number */
};

struct	i258tp_buf	{
	int				bufpages;	/* for read-ahead/write-behind tape buffering */
	unsigned char	unit;		/* unit number of tape device */
};

#ifdef V_3
/*
 * the following defines are for migration to EFT in V.4
 */
#define getmajor	major
#define getminor	minor
#endif

#define UNIT(dev)    (i258minor[getminor(dev)].unit) /* dev->unit# map */
#define DRTAB(dev)    (i258minor[getminor(dev)].drtab)/* dev->drtab-index map*/
#define PARTITION(dev)    (i258minor[getminor(dev)].partition)/* dev->prt-idx map */
#define BOARD(dev)    (i258minor[getminor(dev)].board)    /* board number */
#define i258MINOR(bnum,unum,drnum,panum) { panum, drnum, unum, bnum }
#define BASEDEV(dev)   (makedevice(getmajor(dev),  \
							((int)(getminor(dev) / V_NUMPAR) * V_NUMPAR)) )
#define ISWINI(dev)    (int) ((i258dev[BOARD(dev)].w_units[UNIT(dev)]) & 0x1)
#define ISFLOP(dev)    (int) ((i258dev[BOARD(dev)].f_units[UNIT(dev)]) & 0x1)
#define ISTAPE(dev)    (int) ((i258dev[BOARD(dev)].t_units[UNIT(dev)]) & 0x1)
#define I258FLP(tr,sp,t,type) ((type<<8)|(t<<4)|(sp<<2)|tr) /* floppy charc */


/*
 * Partition structure.  One per drtab[] entry.
 */
struct    i258part {
    ushort		p_tag;			/* ID tag (from vtoc.h) */
    ushort		p_flag;			/* permision flags (from vtoc.h) */
    daddr_t		p_fsec;			/* first sector */
    daddr_t		p_nsec;			/* number sectors */
};

/*
 * Per-board configuration.  One of these per 258 board.
 * c_devcod indicates what kind of device/floppies/tape are there.
 *
 * The c_drtab field is a pointer to a list of drtab entries per-unit.  A zero
 * value implies non-existent unit.
 */
struct    i258cfg    {
    char    **name_ptr;    /* Name search array */
    struct i258cdrt *c_drtab[NUMSPINDLE];
};

/*
 * Per-board driver "dynamic" data.
 */
struct    i258state {
    unsigned long flags;                /* Board state. */
    char          s_state;              /* what just finished (for intr) */
    ushort        t_flags;              /* non-buffered flags */
    char          t_state;              /* tape state */
    char          s_opunit;             /* current unit being programmed */
    ushort        s_board;              /* board number */
    char          s_flags[NUMUNITS];  	/* flags per spindle */
    char          s_devcod[NUMUNITS]; 	/* device-code */
    char          s_unit[NUMUNITS];   	/* "unit" code */
    char          s_error[NUMUNITS];  	/* status from nonbuffered op */
    struct iobuf  *s_bufh;              /* pointer to regular buffer queue */
    struct iobuf  *t_bufh;              /* pointer to tape buffer queue */
    struct buf    *rtfm_buf;            /* pointer to blk for RTFM state */
};

/*
 * Format Structure.  1 per "board"
 * i258ftk is the argument structure to the format ioctl.
 */
struct    i258format {
    char    f_trtype;            /* format track-type code */
    char    f_interleave;        /* interleave-factor */
    char    f_pattern[4];        /* pattern; depends on f_trtype */
    char    f_reserved[10];      /* Reserved for future use */
};

struct    i258ftk    {
    ushort    f_track;       /* track # */
    ushort    f_num_tk;      /* Number of tracks */
    ushort    f_intl;        /* interleave factor */
    ushort    f_skew;        /* track skew -- ignored by 258 */
    char      f_type;        /* format type-code */
    char      f_pat[4];      /* pattern data */
    daddr_t   f_secno;       /* for calculating absolute sector number */
};

/*
 * 258 device parameter structure.
 */
struct    i258dp {        /* Partition description (16 bytes) */
    ushort      ncyl;     /* # cylinders */
    char        nfhead;   /* # fixed heads (Winchester) */
    char        nrhead;   /* # removable heads (floppy) */
    char        secptk;   /* # sectors per track */
    char        nalt;     /* # alternate cylinders */
    ushort      secsiz;   /* sector-size */
    daddr_t     fstsec;   /* First sector */
    daddr_t     numsec;   /* Number of sectors */
};

/*
 * Board state flags.
 */
#define    I258_ALIVE       0x0001    /* Board has been found */
#define	   I258_NEED_INIT   0x0002    /* Board has not been initialized yet */
#define	   I258_INITTING    0x0004    /* Board is being initialized */
#define    I258_PAUSE       0x0008    /* Sleep until flags is cleared */
#define    I258_NEED_REQ    0x0010    /* A request slot is needed */
#define    I258_TIMEOUT     0x0020    /* Restart fail/safe timer */
#define    I258_FORMAT      0x0040    /* Format data area in use */

/*
 * Timeout values
 */

#define WAIT_1MINUTE	61
#define WAIT_5MINUTES	301
#define WAIT_10MINUTES	601
#define WAIT_FOREVER	9999

/*
 * Per-Unit State Flags.
 */
#define SF_OPEN         0x01    /* unit is open */
#define SF_READY        0x02    /* unit is ready; reset by media-change */
#define SF_VTOC_OK      0x04    /* a valid VTOC has been read for this unit */
#define SF_OPENCLOSE    0x08    /* open or close in progress */
#define SF_CLOSEWAIT    0x10    /* Waiting for all requests for "unit" */
                                /* to complete so device can be closed */
#define SF_EOF_MARK     0x20    /* Tape EOF mark encountered */
#define SF_NEED_EOF     0x40    /* Buffered tape needs to handle EOF mark */
#define SF_RESERVED     0x80    /* Device has been reserved */
#define SF_HOLD_RSRV    0x100   /* Don't release device on close */
#define SF_OEXCL	0x200	/* Device opened for exclusive open */

/*
 * Tape State values.
 */
#define TS_NOTHING    0
#define TS_READING    1
#define TS_WRITING    2

/*
 * I002
 * Buffered tape I/O flags.
 */
#define B_RAMRD		0x0001	/* Received response to buf tape read cmd. */
#define B_RAMWT		0x0002	/* Received response to buf tape write cmd. */

/*
 * Buffered tape handler commands
 */
#define TP_GETBUF   0
#define TP_FREEBUF  1
#define TP_RDBUF    2
#define TP_WRBUF    3
#define TP_FLUSH    4


/*
 * Drive-Data Table (used to initialize drives).  See 258 manual.
 * Fields through dr_resv are programmed into controller for an init (disk);
 * Other fields are for internal driver use.
 * The i258cdrt structure is for static initialization of data.  It has
 * to be moved into the drtab so it will be aligned the way the controller
 * wants it.
 */
struct i258drtab {
    union    {
	struct {
	    unsigned long      dr_ncyl;       /* # cylinders                  */
	    ushort             dr_nhead;      /* # heads (# of surfaces)      */
	    ushort             dr_nsec;       /* # sectors per track          */
	    ushort             dr_secsiz;     /* # bytes/sector (sector size) */
	    ushort             dr_floppy;     /* floppy disk format           */
	    unsigned long      dr_rwcc;       /* Reduced write current cyl    */
	    unsigned long      dr_wpc;        /* Write precomp cylinder       */
	    ushort             dr_step_rate;  /* step rate                    */
	    ushort             dr_alt_sect_z; /* Alternate sectors per Zone   */
	    ushort             dr_alt_trk_z;  /* Alternate tracks per Zone    */
	    ushort             dr_alt_trk_v;  /* Alternate tracks per Volume  */
	    unsigned long      dr_trk_z;      /* Tracks per Zone              */
	    ushort             dr_trk_skew;   /* Track Skew                   */
	    ushort             dr_cyl_skew;   /* Cylinder Skew                */
	    ushort             dr_interleave; /* Interleave                   */
	    unsigned char      dr_resv[218];  /* Reserved                     */
	} w_f;  /* Set Unit Characteristics buffer for wini and floppy        */
        struct {
	    ushort             dr_bytes_block;/* data bytes per tape block    */
	    unsigned char      dr_resv[254];  /* Reserved */
        } tape; /* Set Unit Characteristics buffer for tape */
    } suc_buf;  /* Set Unit Characteristics buffer */
	/* from here down does not get sent to the 258 controller */
    ushort             dr_spc;        /* actual sectors/cylinder */
    ushort             dr_lbps;       /* logical blocks(512b) per sector */
    struct   i258part  *dr_part;      /* partition table pointer */
    char               dr_pnum;       /* number of partitions */
    struct   alt_info  *dr_altptr;    /* points at alternates table */
	ushort		 	   unitnum;		  /* Unit number for this structure */
};

struct    i258cdrt {
    ushort          cdr_ncyl;     /* # cylinders */
    char            cdr_nhead;    /* # heads */
    char            cdr_nsec;     /* # sectors per track */
    ushort          cdr_secsiz;   /* sector-size */
    ushort          cdr_flags;    /* Drive flags */
    char            cdr_pnum;     /* number of partitions */
	int				cdr_size;	  /* size of the device in 512 byte blocks */
    struct i258part *cdr_part;    /* partition table pointer */
};

#define DR_NO_REWIND		0x1
#define DR_RETENSION		0x2
#define DR_READ_TO_FM		0x4

/*
 * Drive-Data Table as found in the volume label.
 * Because of C alignment problem on secsiz, it must be entered bytewise.
 * Fields through dr_nalt are programmed into controller for an init (disk);
 * tapes only use first byte.  Other fields are for internal driver use.
 * The i214cdrt structure is for static initialization of data.  It has
 * to be moved into the drtab so it will be aligned the way the controller
 * wants it.
 */
struct    ilabdrtab {
    ushort      dr_ncyl;       /* # cylinders */
    char        dr_nfhead;     /* # fixed heads (Winchester) */
    char        dr_nrhead;     /* # removable heads (floppy) */
    char        dr_nsec;       /* # sectors per track */
    char        dr_lsecsiz;    /* "low" of sector-size */
    char        dr_hsecsiz;    /* "high" of sector-size */
    char        dr_nalt;       /* # alternate cylinders */
                               /* if floppy, 0==FM, 1==MFM */
};

/*
 * 258 request structure.
 */
struct i258req {
    unsigned char  r_tid;        /* Transaction id */
    unsigned char  r_dev;        /* Device */
    unsigned short r_flags;      /* Control flags for this request */
    unsigned short r_retries;    /* retry count. */
    unsigned short r_timeout;    /* timeout count */
    struct   buf   *r_bp;        /* associated buffer */
    struct dma_buf       *r_dbp;       /* data buffer descriptors */
    mps_msgbuf_t       *r_mbp;       /* Message buffer pointer */
    unsigned long  r_xcount;     /* remaining transfer count */
    time_t         r_tqueued;    /* Time request was put on driver's queue */
    time_t         r_tstart;     /* Timestamp for start of request */
    unsigned char  r_op;         /* Remember the op */
};

/*
 * The flags that affect a given request.
 */
#define i258RQ_BUSY        0x0001    /* This slot is in use */
#define i258RQ_WATCH_ME    0x0002    /* Watch this request for a timeout */
#define i258RQ_TIMED_OUT   0x0004    /* this request has timed out */
#define i258RQ_WATCH_MASK  0x0007    /* Important bits for checking timeout */
#define i258RQ_IM_WAITING  0x0008    /* wakeup(this request) */
#define i258RQ_DONT_REUSE  0x0010    /* Don't reuse this slot (in bmsg) */
#define i258RQ_BUF_TAPE    0x0040    /* Request is buffered tape I/O */

/*
 * 258 Per-Board Device-Data.  One per board.
 */
struct    i258dev {
    unsigned long		d_flags;            /* Board state flags */
    unsigned short		unit_def_rec;       /* ICS addr of Unit Def Rec */
    unsigned short		host_id_rec;        /* ICS offset of host id rec */
    unsigned short		slot;               /* Slot number of board. */
    unsigned short		host_id;            /* Host ID of board. */
    unsigned short		port_id;            /* Port ID of board. */
    unsigned short		req_use;            /* Number of slots used */
    unsigned short		av_max;             /* Max outstanding requests */
    struct   i258req	reqinfo[MAXREQ];    /* per request info */
    unsigned char		dunit[NUMTYPE];     /* Wini/floppy/tape unit #'s */
    unsigned char		d_pdev[NUMUNITS]; 	/* Device #, board style */
    unsigned short		d_sflags[NUMUNITS];
    struct   iobuf		*d_bufh;            /* pointer regular buffer queue */
    struct   iobuf		*d_tbufh;           /* pointer to tape buffer queue */
    unsigned short		d_tstate[NUMUNITS];	/* tape state */
    unsigned short		d_tflags[NUMUNITS];	/* non-buffered flags */
    struct   i258drtab	d_drtab[NUMSPINDLE];
    struct   i258format	d_format;           /* Format data sent to board */
    struct   i258ftk	d_ftk;				/* Driver data for formatting */
    struct	 i258tp_buf	tbuf[I258_NUMTAPE];	/* tape buffer information */
    struct   buf		d_tbp[I258_NUMTAPE];/* Buffer ptr handling buffer */
	unsigned char		w_units[I258_QUERY_CONTROLLER_SIZE]; /* Wini Units */
	unsigned char		f_units[I258_QUERY_CONTROLLER_SIZE]; /* Floppy Units */
	unsigned char		t_units[I258_QUERY_CONTROLLER_SIZE]; /* Tape Units */
};

/*
 * Misc wini data. One entry per wini.
 */
struct i258wini {
	unsigned int	unitnum;	/* Unit # for this wini */
	unsigned int	basedev;	/* Minor # of Part 0 for this wini */
	unsigned int	ivlabloc;	/* Disk location (Abs byte) of IVLAB */
	unsigned int	ivlablen;	/* Length of IVLAB */
	unsigned int	pdinfoloc;
	unsigned int	pdinfolen;	/* PDINFO disk location and length. */
	unsigned int	vtocloc;	
	unsigned int	vtoclen;	/* VTOC disk location and length. */
	unsigned int	altinfoloc;	
	unsigned int	altinfolen;	/* ALT INFO disk location and length. */
	unsigned int	mdlloc;	
	unsigned int	mdllen;		/* MDL disk location and length. */
	/*
	 * Pointers to temporary copies of the data structures.
	 * Normally, these pointers are NULL, but are filled in 
	 * when temporary copies of these structures are needed.
	 * Before the pointers are used, the lock must be set 
	 * and some memory must be allocated.
	 */
	unsigned int	lock;		/* Semaphore */
	unsigned int	pgcnt;		/* Number of pages allocated. */
	unsigned int	pgaddr; 	/* Address of allocated pages. */
	struct ivlab	*ivlab;
	struct pdinfo	*pdinfo;
	struct vtoc		*vtoc;
	struct alt_info	altinfo;
	struct st506mdl	*mdl;
};
/* 
 * variables for verify ioctl
 */
struct i258rdvfy {
	unsigned int	unitnum;	/* Unit # for this wini */
	union	vfy_io	vfy_io;
	ushort			err_code;
	time_t			vfytime;
	ushort			vfyreq;			
};

/*
 * Interconnect Space Record Type definitions.
 */

#define UNIT_DEF_TYPE   0xfe
#define FW_COMM_TYPE    0x0f

/*
 * i258PCM -- Peripheral Command Message
 */
struct i258PCM {
    unsigned char    P_cmd_type;       /* Command being issued */
    unsigned char    P_resv;           /* Reserved */
    unsigned char    P_device_type;    /* Logical code for type of device */
    unsigned char    P_unit_number;    /* Unit Number */
    union {
        unsigned long          P_device_addr; /* Logical Block Number 32 bits */
        struct {
                 unsigned char P_flags;       /* flags for set char command */
                 unsigned char P_mode;        /* mode for set options commnad */
                 unsigned char P_no_care;     /* Reserved */
                 unsigned char P_retries;
        } P_unit_char;
		/*
		 * the following structure supports control only not control and data
		 * modifications will have to be made to entire PCM structure if 
		 * control and data has to be included.
		 */
		struct {
			unsigned char P_flags;	/* flags for format */
			unsigned char P_fmtresv[3];
		} P_format;
		/* stuff for get server information */
		struct	{
			unsigned char	cmd;
			unsigned char	resv;
			unsigned short	arg1;
		} P_server;
    } P_union1;
    union {
        unsigned long    P_byte_count;        /* bytes to transfer 32 bits */
        struct {
                 unsigned char P_cmd_ord;     /* Command Ordering */
                 unsigned char P_cache_size;  /* Cache Size       */
                 unsigned char P_read_ahead;  /* Read Ahead       */
                 unsigned char P_nocare;      /* Reserved         */
        } P_unit_opt;
    } P_union2;
    unsigned char    P_reserved[8];    /* Reserved */
};

#define	PCMCLEN		20	/* Length of Control Only Requests */
#define	PCMCDLEN	16	/* Length of Control+Data Requests */

/*
 * i258PSM -- Peripheral Status Message
 */
struct i258PSM {
    unsigned char    P_cmd_type;       /* Command being issued */
    unsigned char    P_comp_stat;      /* Completion status */
    unsigned char    P_device_type;    /* Logical code for type of device */
    unsigned char    P_unit_number;    /* Unit Number */
    unsigned long    P_device_addr;    /* Logical Block Number 32 bits */
    unsigned long    P_byte_count;     /* Number bytes transfered 32 bits */
    unsigned char    P_retries;        /* Number of retries */
    unsigned char    P_device_err[3];  /* Device Specific Error */
    unsigned char    P_reserved[4];
};

/*
 * Values of buffer-header b_active, used for mutual-exclusion of
 * opens and other I/O requests.
 */
#define IO_IDLE    0        /* idle -- anything goes */
#define IO_BUSY    1        /* something going on */
#define IO_WAIT    2        /* waiting for controller to be idle */

/*
 * i258 driver command codes used to communicate with i258io.
 */
#define CP_DATA_CMD			0x01	/* not used */
#define ERASE_CMD			0x02	
#define FORMAT_CMD			0x03
#define LOC_SERVER_CMD		0x04	/* not used */
#define LOAD_CMD			0x05
#define QUERY_CONTLR_CMD	0x06	/* not used */
#define QUERY_UC_CMD		0x07	/* not used */
#define QUERY_UO_CMD		0x08	/* not used */
#define RW_CMD   			0x09
#define RD_VERIFY_CMD   	0x0A	/* not used */
#define READ_DEFLIST_CMD	0x0B	/* not used */
#define REASGN_BLOCKS_CMD	0x0C	/* not used */
#define RECAL_CMD			0x0D
#define RELEASE_CMD			0x0E
#define REQ_UA_CMD			0x0F	/* not used */
#define RESERVE_CMD			0x11
#define RESET_CMD			0x12	
#define RETEN_TAPE_CMD		0x13
#define SEEK_BOT_CMD		0x14
#define SEOD_CMD			0x15
#define SFFM_CMD			0x16
#define SUC_CMD				0x17
#define SIO_CMD				0x18
#define START_UNIT_CMD		0x19	/* not used */
#define STOP_UNIT_CMD		0x1A	/* not used */
#define TEST_IO_CMD			0x1B	/* not used */
#define UNIT_NOTIFY_CMD		0x1C	/* not used */
#define UNLOAD_CMD			0x1D
#define WRFM_CMD			0x1E

/*
 * 258 Command Codes.
 */
#define COPY_DATA			0x20	/* not used */
#define ERASE_UNIT			0x29	
#define FORMAT_UNIT			0x08
#define GET_SERVER_INFO		0x18
#define LOAD				0x2C
#define LOCATE_PCI			0x01	
#define QUERY_CONTROLLER	0x05	/* not used */
#define QUERY_UNIT_CHAR		0x17	/* not used */
#define QUERY_UNIT_OPTS		0x07	/* not used */
#define READ_DATA			0x11
#define READ_DATA_VERIFY	0x22
#define READ_DEF_LIST		0x0A	/* not used */
#define REASSIGN_BLOCKS		0x09	/* not used */
#define RECALIBERATE		0x23
#define RELEASE				0x04
#define REQUEST_UNIT_ATTN	0x30	/* not used */
#define RESERVE				0x03
#define RESET_DEVICE		0x02	
#define RETENSION_TAPE		0x26
#define SEEK_BEG_OF_TAPE	0x2A
#define SEEK_END_OF_DATA	0x2B
#define SEEK_FILEMARK		0x27
#define SET_UNIT_CHAR		0x10
#define SET_UNIT_OPTIONS	0x06
#define START_UNIT			0x0F	/* not used */
#define STOP_UNIT			0x0E	/* not used */
#define TEST_IO				0x24	/* not used */
#define UNIT_ATTN_NOTIFY	0x2F	/* not used */
#define UNLOAD				0x25
#define WRITE_DATA			0x14
#define WRITE_FILEMARK		0x28

/*
 * Floppy FM/MFM codes for drtab[*].nalt.
 */
#define FLPY_FM     0        /* FM -- single density */
#define FLPY_MFM    1        /* MFM -- double density */

/*
 * Misc Format definitions, for i258ftk.f_type.
 */
#define FORMAT_DATA         0x00    /* format data track */
#define FORMAT_BAD          0x80    /* format bad track */
#define FORMAT_ALTERNATE    0x40    /* format alternate track */
#define FMTDONE             0       /* Return status if successful format */

/*
 * iSBC 258 ioctl mnemonics.
 */
#define I258_IOC_RDC        (('W'<<8)|32)
#define I258_RESERVE        (('W'<<8)|33)
#define I258_RELEASE        (('W'<<8)|34)
#define I258_GET_SPECS      (('W'<<8)|35)
#define I258_SET_SPECS      (('W'<<8)|36)

/*
 * I004
 * Hardwre Specific Tape ioctl mnemonics
 */
#define I258_RESET			(('W'<<8)|10)
#define I258_XINIT			(('W'<<8)|11)
#define I258_TESTIO			(('W'<<8)|12)
#define I258_BRD_STATS		(('W'<<8)|0x90)

/*
 *    BIST defines - some handy values.
 */
#define BIST_SLAVE_STAT    0x00    /* Expected Slave status bits */
#define BIST_MSTR_STAT     0x20    /* Expected Master status bits */

/*
 * the following describes function codes for formatting
 */
#define	PRIM_ONLY		0x00	/* map out primary defect list */
#define GROWN_ONLY		0x01	/* map out grown defect list */
#define PRIM_AND_GROWN	0x02	/* map out primary and grown defect list */
#define NO_DEFECT		0x03	/* Neither list is mapped out */

#define CERTIFY_DISABLE	0x00	/* No certification process */		
#define CERTIFY_ENABLE	0x04	/* certification process */

/* 258 command options */
#define FORCE_MODE_ENABLE	0x01		/* enable force mode bit */
#define PHYSICAL_ADDR_MODE	0x01		/* enable phy. addressing mode bit */
#define CACHE_MODE_ENABLE	0x02		/* enable cache mode bit */
#define CMD_REORDER_ENABLE	0x04		/* enable command re-order mode bit */
#define DEF_SEEK_ORDERING	(0xF<<2)	/* enable command re-order mode bit */

/*
 *    Error codes from the 258
 */

#define ER258_CMD_COMP 		0x0
#define ER258_END_MEDIA		0x1
#define ER258_EOR_MEDIA		0x2
#define ER258_FILE_MARK		0x3
#define ER258_COMM_TIME		0x4
#define ER258_RESERVED		0x5
#define ER258_NOT_INITIAL	0x6
#define ER258_ALREADY_INIT	0x7
#define ER258_INVAL_COMM	0x8
#define ER258_INVAL_PARB	0x9
#define ER258_NOT_READY		0xA
#define ER258_WRITE_PRO		0xB
#define ER258_MEDIA_ERR		0xC

/*
 *    The error log structure as used by the transfer error log command
 */
struct i258dtot {
    unsigned char    d_stotal;        /* Total soft errors for the device */
    unsigned char    d_htotal;        /* Total hard errors for the device */
};

struct i258elog {
    unsigned short     e_total;               /* Total errors */
    unsigned char      e_reserve1[4];
    unsigned char      e_stotal;              /* Total soft errors */
    unsigned char      e_htotal;              /* Total hard errors */
    struct   i258dtot  e_dtot[NUMUNITS];      /* error totals per device */
    unsigned long      e_rhcache;             /* cache hit total for reads */
    unsigned long      e_rmcache;             /* cache miss total for reads */
    unsigned long      e_wtotal;              /* total number write requests */
    unsigned char      e_index;               /* index to psb's */
    unsigned char      e_reserve2[19];
    struct   i258PSM   e_psm[30];
};

struct elogarg {
    unsigned short        el_type;        /* TYPE */
    unsigned short        el_count;       /* Number of pointers in struct */
    struct   i258elog    *el_elog;
};

#define EL_258    1        /* Type 258 */
#define SP_258    1        /* Type 258 */

struct sp_get {
    unsigned short    sp_type;        /* TYPE */
    unsigned short    sp_count;       /* Number of pointers in struct */
    unsigned short    sp_rq_slot;     /* Number of requests slots */
    unsigned short    sp_reqs;        /* Number of requests allowed */
};

struct sp_set {
    unsigned short     sp_type;        /* TYPE */
    unsigned short     sp_count;       /* Number of pointers in struct */
    unsigned short     sp_req_val;     /* sp_req valid */
    unsigned short     sp_reqs;        /* Number of requests allowed */
};

union i258specs {
    struct sp_set sp_set;
    struct sp_get sp_get;
};

struct	i258resp	{
unsigned short	hostid;
unsigned short	portid;
unsigned char	nservers; 
unsigned char	stoff; 
};

#ifdef __STDC__
extern	int		i258close(dev_t, int, int, struct cred *);
extern	int		i258halt();
extern	int		i258init();
extern	int		i258ioctl (dev_t, int, caddr_t, int, struct cred *, int *);
extern	int		i258open(dev_t *, int, int, struct cred *);
extern	int		i258print (dev_t, char *);
extern	int		i258read(dev_t, uio_t *, struct cred *);
extern	int		i258size(dev_t);
extern	void	i258strategy(struct buf *);
extern	int		i258write(dev_t,  uio_t *, struct cred *);
extern	int		i258bmsg(mps_msgbuf_t *);
#endif

#endif	/* _SYS_I258_H */
