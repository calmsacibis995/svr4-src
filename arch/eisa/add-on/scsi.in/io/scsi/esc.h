/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	Copyright (C) Ing. C. Olivetti & C. S.p.a., 1989.*/
/*	All Rights Reserved	*/

#ident	"@(#)eisa:add-on/scsi.in/io/scsi/esc.h	1.3.1.1"

/*
 * the host adapter minor device number is interpreted as follows:
 *
 *           MAJOR           MINOR      
 *      -------------------------------
 *      |  mmmmmmmm  |  ccc  ttt ll   |
 *      -------------------------------
 *      
 *         m = major number assigned by idinstall
 *	   c = Host Adapter Card number (0-7)
 *         t = target controller ID (0-7)
 *         l = logical unit number (0-3)
 *
 */
#if	(_SYSTEMENV == 4)
#define SC_HAN(dev)	((getminor(dev) >> 5) & 0x07)
#define SC_TCN(dev)	((getminor(dev) >> 2) & 0x07)
#define SC_LUN(dev)	((getminor(dev) & 0x03))
#else
#define SC_HAN(dev)	((minor(dev) >> 5) & 0x07)
#define SC_TCN(dev)	((minor(dev) >> 2) & 0x07)
#define SC_LUN(dev)	((minor(dev) & 0x03))
#endif


/*  parameters of this implementation  */

#define MAX_EQ	 MAX_TCS * MAX_LUS	/* Max equipage per controller	*/
#define	MAX_JOBSZ   (128 * 1024)	/* Max size of data transfer	*/
#define NDMA		20		/* Number of DMA lists		*/
#define NCCB		50		/* number of controller CBs	*/
#define SCSI_ID		7		/* Default ID of controllers	*/

#define	ONE_MSEC	1
#define	ONE_SEC		1000
#define	ONE_MIN		60000

#ifndef TRUE
#define	TRUE		1
#define	FALSE		0
#endif

struct scsi_cfg {
	unsigned int	ha_id;		/* SCSI identifier 		*/
	unsigned int	ivect;		/* Interrupt vector number	*/
	unsigned long	io_base;	/* I/O base address		*/
};

/* The null ID numbers. These are used to address the host adapter 	*/
/* or indicate that a special minor device is being addressed.		*/
/* Both relate directly to the SC_HAN and SC_TCN macros defined above.	*/

#define	NULL_HAN	0x07		/* null host adapter number 	*/
#define	NULL_TCN	0x07		/* null target controller number*/


/* semaphores, doorbells and masks offset */

#define	SEMINC		0x0C8A	/* semaphore for the incoming mailbox */
#define	BELLINC		0x0C8D	/* local doorbell register */
#define	MASKINC		0x0C8C	/* mask for the local doorbell */

#define	SEMOUT		0x0C8B	/* semaphore for the outgoing mailbox */
#define	BELLOUT		0x0C8F	/* system doorbell register */
#define	MASKOUT		0x0C8E	/* mask for the system doorbell */
#define	INTMASK		0x0C89	/* system interrupt enable register */

#define	ELCREG		0x04D0	/* edge/level control register */

#define	SYSID		0x0C80	/* system identifier offset */
#define	OLI1		0x3D	/* first byte manufacturer code */
#define	OLI2		0x89	/* second byte manufacturer code */
#define	BOARDID		0x10	/* first byte board identifier #1 */
#define	BOARDID2	0x21	/* second byte board identifier #2 */


struct mbi {
	unsigned char	m_taskid;	/* task identifier */
	unsigned char	m_cmd;		/* command */
	unsigned short	m_cmdlen;	/* command length */
	paddr_t		m_addr;		/* physical data ptr */
};

/* incoming mailbox offset */
#define	MBI_ADDR	0x0C90

/* host adapter commands */
#define	GETCCB		0x01
#define	SNDCFG		0x02
#define	XDIAGN		0x03
#define	RESET		0xFF


struct mbo {
	unsigned char	m_taskid;	/* task identifier */
	unsigned char	m_pad;		/* undefined */
	unsigned char	m_tstat;	/* target status */
	unsigned char	m_hstat;	/* host adapter status */
	paddr_t		m_addr;		/* physical data ptr */
};

/* outgoing mailbox offset */
#define	MBO_ADDR	0x0C98

/* host adapter status */
#define	NOERROR		0x00	/* no adapter detected error */
#define	INVALCMD	0x01	/* invalid command */
#define	LCMDCOMP	0x0B	/* linked command complete with flag */
#define	NOSELECT	0x11	/* selection timeout expired */
#define	LOSTDATA	0x12	/* data overrun/underrun */
#define	SBUSERR		0x13	/* unexpected bus free phase detected */
#define	TCPROTO		0x14	/* scsi phase sequence error */
#define	HAQFULL		0x1F	/* internal queue full */


#if	defined(i860)
#define	CCB_OFF		2
#else
#pragma pack(2)
#endif	/* i860 */

/* the Command Control Block structure */
struct ccb {
#if	defined(i860)
	char		pad[CCB_OFF];		/* alignment pad */
#endif	/* i860 */
	unsigned char	c_lun  :3;	/* logical unit number */
	unsigned char	c_tcn  :3;	/* target controller number */
	unsigned char	c_xfer :2;	/* transfer direction */
	unsigned char	c_cmdsz;	/* Size of command		*/
	unsigned int	c_datasz;	/* Size of data transfer	*/
	paddr_t 	c_datapt;	/* Physical src or dst		*/
	unsigned int	c_addlen;	/* addition block length */
	paddr_t		c_linkpt;	/* link pointer (not supported) */
	unsigned char	c_cdb[12];	/* Command Descriptor Block	*/

     /* from here down does not get sent to the controller */

	paddr_t		c_addr;		/* CB physical address		*/
	unsigned short	c_index;	/* CB array index		*/
	unsigned short 	c_active;	/* Command sent to controller	*/
	time_t		c_start;	/* Timestamp for start of cmd	*/
	time_t 		c_time;		/* Timeout count (msecs)	*/
	struct sb      *c_bind;		/* Associated SCSI block	*/
	struct ccb     *c_next;		/* Pointer to next free CB	*/
};

#if	!defined(i860)
#pragma	pack()
#endif

#define MAX_CMDSZ	12
#define FIXED_LEN	18

#define msbyte(x)	(((x) >> 16) & 0xff);
#define mdbyte(x)	(((x) >>  8) & 0xff);
#define lsbyte(x)	((x) & 0xff);


/*
 * DMA vector structure
 */
struct dma_vect {
	unsigned int	d_cnt;		/* Size of data transfer	*/
	paddr_t 	d_addr;		/* Physical src or dst		*/
};

/*
 * DMA list structure
 */
struct dma_list {
	unsigned int	 d_size;	/* List size (in bytes)   	*/
	struct dma_list *d_next;	/* Points to next free list	*/
	struct dma_vect  d_list[50];	/* DMA scatter/gather list	*/
};

typedef struct dma_list dma_t;

#define	MAX_DMASZ	((MAX_JOBSZ / NBPP) + 1)
#define pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))


/*
 * SCSI Request Block structure
 */
struct srb {
	struct sb	sb;		/* Target drv definition of SB	*/
	struct srb     *s_next;		/* Next block on LU queue	*/
	struct srb     *s_prev;		/* Previous block on LU queue	*/
	dma_t	       *s_dmap;		/* DMA scatter/gather list	*/
	paddr_t		s_addr;		/* Physical data pointer	*/
};

typedef struct srb sblk_t;


/*
 * Logical Unit Queue structure
 */
struct scsi_lu {
	struct srb     *q_first;	/* First block on LU queue	*/
	struct srb     *q_last;		/* Last block on LU queue	*/
	int		q_flag;		/* LU queue state flags		*/
	int     	q_count;	/* Outstanding job counter	*/
	void	      (*q_func)();	/* Target driver event handler	*/
	long		q_param;	/* Target driver event param	*/
};

#define	QBUSY		0x01
#define	QFULL		0x02
#define	QSUSP		0x04
#define	QPTHRU		0x10

#define queclass(x)	((x)->sb.sb_type)
#define	QNORM		SCB_TYPE


/*
 * Host Adapter structure
 */
struct scsi_ha {
	unsigned short	ha_state;	  /* Operational state 		*/
	unsigned short	ha_id;		  /* Host adapter SCSI id	*/
	int		ha_vect;	  /* Interrupt vector number	*/
	unsigned long   ha_base;	  /* Base I/O address		*/
	int		ha_npend;	/* # of jobs sent to HA	*/
	struct mbo	ha_mbo;	  	/* outgoing mail box */
	struct mbi	ha_mbi;	 	/* incoming mail box */
	struct ccb	ha_ccb[NCCB];	/* command block pool */
	struct ccb     *ha_cblist;	/* command block free list */
	struct scsi_lu	ha_dev[MAX_EQ];	/* logical unit queues	*/
};


/*	
**	Macros to help code, maintain, etc.
*/

#define SUBDEV(t,l)	((t << 3) | l)
#define LU_Q(c,t,l)	sc_ha[c].ha_dev[SUBDEV(t,l)]

#define	SC_ILLEGAL(c,t)	((c >= sdi_hacnt) || \
			(sc_edt[c][t].tc_equip == NULL))
				

#define	SDI_HAN(x)	(((x)->sa_fill >> 3) & 0x07)
#define	SDI_TCN(x)	((x)->sa_fill & 0x07)

#if	(_SYSTEMENV == 4)
#define	SDI_ILLEGAL(c,t,m)    ((c >= sdi_hacnt) || \
		(sc_edt[c][t].tc_equip == NULL) )
#else
#define	SDI_ILLEGAL(c,t,m)    ((c >= sdi_hacnt) || \
		(sc_edt[c][t].tc_equip == NULL) || \
		((sc_edt[c][t].c_maj != m) && (sc_edt[c][t].b_maj != m)))
#endif
