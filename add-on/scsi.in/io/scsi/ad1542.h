#ident	"@(#)ad1542.h	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*      INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied under the terms of a license agreement   */
/*	or nondisclosure agreement with Intel Corporation and may not be   */
/*	copied or disclosed except in accordance with the terms of that    */
/*	agreement.							   */

#ident	"@(#)scsi.in:io/scsi/ad1542.h	1.4.2.1"
#ident "$Header: ad1542.h 2.2 91/01/09 $"


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

#define MAX_NCTL	2		   /* number of adapters */
#define MAX_EQ	 	MAX_TCS * MAX_LUS  /* Max equipage per controller */
#define NDMA		20		   /* Number of DMA list */
#define NMBX		50		   /* Number of mailbox pairs */
#define SCSI_ID		7		   /* Default ID of controllers	*/

#define	ONE_MSEC	1
#define	ONE_SEC		1000
#define	ONE_MIN		60000
#define R_LIMIT		20000L		/* retry count */

#ifndef TRUE
#define	TRUE		1
#define	FALSE		0
#endif

struct scsi_cfg {
	unsigned int	ha_id;		/* SCSI identifier 		*/
	unsigned int	ivect;		/* Interrupt vector number	*/
	unsigned long	io_base;	/* I/O base address		*/
};

/* offsets from base */
#define HA_CNTL		0x00		/* Control register		*/
#define HA_CMD		0x01		/* Command /data (out) register	*/
#define HA_ISR		0x02		/* Interrupt status register	*/
#define HA_STAT		0x00		/* Hardware status register	*/
#define HA_DATA		0x01		/* Data (in) register		*/

/* ctl reg bits */
#define HA_SBR		0x10		/* Reset the SCSI bus		*/
#define	HA_IACK		0x20		/* Acknowledge interrupt	*/
#define HA_RST		0x40		/* Host adapter (soft) reset	*/
#define HA_HRST		0x80		/* host adapter hard reset */

/* cmd reg codes */
#define	HA_INIT		0x01		/* Mail box initialization	*/
#define	HA_CKMAIL	0x02		/* Check outgoing mbx		*/
#define HA_BONT         0x07            /* Set Bus-On Time              */
#define HA_BOFFT        0x08            /* Set Bus-Off Time             */
#define HA_XFERS        0x09            /* Set Transfer Speed           */

/* intr reg bits */
#define HA_CMDDONE	0x04		/* command completed */
#define HA_INTR		0x80		/* Interrupt pending service   	*/

/* stat reg bits */
#define HA_DIFULL	0x04		/* adapter write allowed ? */
#define HA_DOFULL	0x08		/* host write allowed ? */
#define HA_READY	0x10		/* Command port ready		*/
#define HA_IREQD	0x20		/* Initialization required 	*/
#define HA_DIAGF	0x40		/* Diagnostic failure		*/
#define HA_STIP		0x80		/* sel test in progress */

#define XFER_5_0	0		/* 5 MB/sec transfer speed */

/*
 * Mail Box structure
 */
struct mbx {
	unsigned int	m_stat : 8;	/* Mail box status or command	*/
	unsigned int	m_addr : 24;	/* CB physical address		*/
};

#define	EMPTY		0x00		/* Mail box is free		*/
#define	SUCCESS		0x01		/* CB successfully completed 	*/
#define	ABORTED		0x02		/* CB aborted by host   	*/
#define	NOT_FND		0x03		/* Aborted CB not found 	*/
#define	FAILURE		0x04		/* CB completed with error    	*/
#define	START		0x01		/* Start the CB	command		*/
#define	ABORT		0x02		/* Abort the CB	command 	*/


/*
 * Controller Command Block 
 */
struct ccb {
	unsigned char	c_opcode;
	unsigned char	c_dev;		/* TC LU command destination	*/
	unsigned char	c_cmdsz;	/* Size of command		*/
	unsigned char	c_sensz;	/* Request Sense alloc size	*/
	unsigned char	c_datasz[3];	/* Size of data transfer	*/
	unsigned char	c_datapt[3];	/* Physical src or dst		*/
	unsigned char	c_linkpt[3];	/* link pointer (not supported) */
	unsigned char	c_linkid;	/* Command link ID		*/
	unsigned char	c_hstat; 	/* Host adapter error status	*/
	unsigned char	c_tstat;	/* Target completion status	*/
	unsigned char	c_res[2];	/* reserved for later use	*/
	unsigned char	c_cdb[12];	/* Command Descriptor Block	*/
	unsigned char	c_sense[sizeof(struct sense)];
					/* Sense data			*/

     /* from here down does not get sent to the controller */

	paddr_t		c_addr;		/* CB physical address		*/
	unsigned short	c_index;	/* CB array index		*/
	unsigned short 	c_active;	/* Command sent to controller	*/
	time_t		c_start;	/* Timestamp for start of cmd	*/
	time_t 		c_time;		/* Timeout count (ticks)	*/
	struct sb      *c_bind;		/* Associated SCSI block	*/
	struct ccb     *c_next;		/* Pointer to next free CB	*/
};

#define MAX_CMDSZ	12

#define	NO_ERROR	0x00		/* No adapter detected error	*/
#define	NO_SELECT	0x11		/* Selection time out		*/
#define	TC_PROTO	0x14		/* TC protocol error		*/

#define SCSI_CMD	0x00
#define SCSI_DMA_CMD	0x02
#define SCSI_TRESET	0x81

#define msbyte(x)	(((x) >> 16) & 0xff);
#define mdbyte(x)	(((x) >>  8) & 0xff);
#define lsbyte(x)	((x) & 0xff);


/*
 * DMA vector structure
 */
struct dma_vect {
	unsigned char	d_cnt[3];	/* Size of data transfer	*/
	unsigned char	d_addr[3];	/* Physical src or dst		*/
};

/*
 * DMA list structure
 */

#define	MAX_DMASZ	16		/* Hard limit in the controller	*/
#define pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))

struct dma_list {
	unsigned int	 d_size;	/* List size (in bytes)   	*/
	struct dma_list *d_next;	/* Points to next free list	*/
	struct dma_vect  d_list[MAX_DMASZ];	/* DMA scatter/gather list	*/
};

typedef struct dma_list dma_t;


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
	struct sense	q_sense;	/* Sense data			*/
	int     	q_count;	/* Outstanding job counter	*/
	void	      (*q_func)();	/* Target driver event handler	*/
	long		q_param;	/* Target driver event param	*/
};

#define	QBUSY		0x01
#define	QFULL		0x02
#define	QSUSP		0x04
#define	QSENSE		0x08		/* Sense data cache valid */
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
	int		ha_npend;	  /* # of jobs sent to HA	*/
	struct mbx     *ha_give;	  /* Points to next mbo		*/
	struct mbx     *ha_take;	  /* Points to next mbi		*/
	struct mbx	ha_mbo[NMBX];	  /* Outgoing mail box area	*/
	struct mbx	ha_mbi[NMBX];	  /* Incoming mail box area	*/
	struct ccb	ha_ccb[NMBX];	  /* Controller command blocks	*/
	struct ccb     *ha_cblist;	  /* Command block free list	*/
	struct scsi_lu	ha_dev[MAX_EQ];	  /* Logical unit queues	*/
};

#define C_SANITY	0x8000


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
