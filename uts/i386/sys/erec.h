/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EREC_H
#define _SYS_EREC_H

#ident	"@(#)head.sys:sys/erec.h	11.1.8.1"
/*
 * Every error record has a header as follows.
 */



struct errhdr {
	short	e_type;		/* record type */
	short	e_len;		/* bytes in record (with header) */
	time_t	e_time;		/* time of day */
};

/*
 * Error record types
 */

#define E_GOTS	010		/* Start for UNIX/TS */
#define E_GORT	011		/* Start for UNIX/RT */
#define E_STOP	012		/* Stop */
#define E_TCHG	013		/* Time change */
#define E_CCHG	014		/* Configuration change */
#define E_BLK	020		/* Block device error */
#define E_STRAY	030		/* Stray interrupt */
#define E_MEM	031		/* Memory error */
#define E_CNTL	041		/* IO Controller error */
#define E_SYS	042		/* System error */
#define E_CAC	043		/* Cache error */
#define E_PF	044		/* Prefetch error MAC 32 only */


/* Device descriptors for logging start up record. */

struct ddesc {
	char d_name[9];		/* Device name */
	long lb_addr;		/* Device address */
	long su_equip;		/* Device equippage */
};


/*
 * Error logging startup record. One of these is
 * sent to the logging daemon when logging is
 * first activated.
 */

struct estart {
	short	e_cpu;		/* cpu type */
	struct	utsname e_name;	/* system names */
	int	e_mmcnt;	/* Memory size */
	struct ddesc e_conf[NBEDT]; /* Configured hardware */
	long	e_dcnt;		/* # devices found in EDT */
};

/*
 * Error logging termination record that is sent to the daemon
 * when it stops error logging.
 */

#define eend errhdr

/*
 * A time change record is sent to the daemon whenever
 * the system's time of day is changed.
 */

struct etimchg {
	time_t	e_ntime;	/* new time */
};


/*
 * A configuration change message is sent to
 * the error logging daemon whenever a block device driver
 * is attached or detached (MERT only).
 */

struct econfchg {
	char	e_trudev;	/* "true" major device number */
	char	e_cflag;	/* driver attached or detached */
};

#define E_ATCH	1
#define E_DTCH	0

/*
 * Template for the error record that is logged by block device
 * drivers.
 */

struct eblock {
	o_dev_t	e_num;		/* device number (major + minor) */
	ushort	e_bytes;	/* number of bytes to transfer */
	short	e_bflags;	/* read/write, error, etc */
	daddr_t	e_bnum;		/* logical block number */
	struct iostat e_stats;	/* unit I/O statistics */
	paddr_t e_badd;		/* physical buffer address */
	unsigned long	e_stat1;	/* job completion status */
	unsigned long	e_stat2;	/* extended err status 1 */
	unsigned long	e_stat3;	/* extended err status 2 */
};

/*
 * Flags (selected subset of flags in buffer header)
 */

#define E_WRITE	0
#define E_READ	1
#define E_NOIO	02
#define E_PHYS	04
#define E_MAP	010
#define E_ERROR	020

/*
 * Template for the stray interrupt record that is logged
 * every time an unexpected interrupt occurs.
 */

struct estray {
	unsigned int	e_saddr;	/* stray loc or device addr */
};

/*
 * Memory error record that is logged whenever one
 * of those things occurs
 */

struct emem {
	 long	e_mcsr[2];	/* registers: MASC CSR0, CSR1 */
};

/*
 * Template for the IO controller error reported via SYSERR 
 */

struct ecntl {
	char	dev_name[9];	/* device name */
	long	lb_addr;	/* bus address */
	long	unit_equip;	/* device equippage */
	long	d_csr;		/* device csr content */
	long	d_errcode;	/* device error code */
};

/*
 * Template for local bus SYSERR
 */

struct esys {
	char	md_name[9];	/* master device name */
	long	mlb_addr;	/* master local bus address */
	long	e_mcsr;		/* CSR content of local bus master */
	char	sd_name[9];	/* slave device name */
	long	slb_addr;	/* slave local bus address */
	long	e_scsr;		/* CSR content of local bus slave */
};


/* This structure will be used to log a prefetch error reported via
 * the dedicated PFERR interrupt.
 */

struct	epf {
	long	e_pc;		/* PC address of error */
	long	e_psw;		/* PSW on error */
	long	e_cccsr;	/* CC CSR on error */
	char	e_dname[9];	/* Slave device name */
	long	e_lbaddr;	/* Slave local bus address */
	long	e_scsr1;	/* Slave CSR content */
	long	e_scsr2;	/* Slave CSR content */
};

/* This structure will be used to log a cache error */

struct ecache {
	long	e_cccsr;	/* CC CSR on error */
	char	e_dname[9];	/* Slave device name */
	long	e_lbaddr;	/* Slave local bus address */
	long	e_scsr1;	/* Slave CSR content */
	long	e_scsr2;	/* Slave CSR content */
};


/*
 * The following structure is used by geru() which finds and clears
 * error data in device control and status registers.
 */

struct eunit {
	struct edt *edtp;	/* edt pointer to error device */
	long csr1;		/* device control and status register */
	long csr2;		/* address trap register if MASC is err dev */
};

#endif	/* _SYS_EREC_H */
