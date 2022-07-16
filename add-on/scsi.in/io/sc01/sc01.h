/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*      INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied under the terms of a license agreement   */
/*	or nondisclosure agreement with Intel Corporation and may not be   */
/*	copied or disclosed except in accordance with the terms of that    */
/*	agreement.							   */

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*		All Rights Reserved			*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*		All Rights Reserved			*/

#ident	"@(#)scsi.in:io/sc01/sc01.h	1.3"

/*
 * the SCSI CD-ROM minor device number is interpreted as follows:
 *
 *     bits:
 *	 7    4 3  0
 * 	+------+----+
 * 	|      |unit|
 * 	+------+----+
 *
 *     codes:
 *	unit  - unit no. (0 - 7)
 */

#define UNIT(x)		(getminor(x) & 0x07)

#define CDNOTCS		-1		/* No TC's configured in system	*/

#define BLKSIZE  512			/* default block size */
#define BLKMASK  (BLKSIZE-1)

#define JTIME	10000			/* ten sec for a job */
#define LATER   20000

#define MAXPEND   2
#define MAXRETRY  2

#define INQ_SZ		98		/* inquiry data size		*/
#define	RDSTATUS_SZ	10		/* playback status &		*/
					/*  subcode Q address data size	*/

/* Values of cd_state */
#define	CD_INIT		0x01		/* Disk has been initialized  */
#define	CD_WOPEN	0x02		/* Waiting for 1st open	 */
#define	CD_DIR		0x04		/* Elevator direction flag	 */
#define	CD_SUSP		0x08		/* Disk Q suspended by HA	 */
#define	CD_SEND		0x10		/* Send requested timeout	 */
#define	CD_PARMS	0x20		/* Disk parms set and valid	 */

/*
 * CD-ROM specific group 6 commands
 */
#define	SV_AUDIOSEARCH	0xC0		/* Audio track search	*/
#define	SV_PLAYAUDIO	0xC1		/* Play audio		*/
#define	SV_STILL	0xC2		/* still		*/
#define	SV_TRAYOPEN	0xC4		/* Tray open		*/
#define	SV_TRAYCLOSE	0xC5		/* Tray close		*/
#define	SV_RDSTATUS	0xC6		/* Read subcode Q data &*/
					/*  playing status	*/

/*
 * Read Capacity data
 */
struct capacity {
	unsigned long    cd_addr;	   /* Logical block address	*/
	unsigned long    cd_len;	   /* Block length	 	*/
};

#define RDCAP_SZ        8
#define RDCAP_AD(x)     ((char *)(x))

/*  This structure defines group 6 commands.  There is a 16 bit pad
 *  at the beginning of the structure so that the 32 bit address
 *  field is properly aligned. Note the host adapter driver must be
 *  passed the address of the structure plus 2.
 */
struct scv {
	int	sv_pad   : 16;		/* dummy		*/
	int	sv_op    : 8;		/* Opcode		*/
	int	sv_param : 3;		/* parameter bit	*/
	int	sv_res1  : 2;		/* Reserved field	*/
	int	sv_lun   : 3;		/* Logical unit number	*/
	unsigned	sv_addr;	/* Block address	*/
	int	sv_res2  : 24;		/* Reserved field	*/
	int	sv_cont  : 8;		/* Control byte		*/
};

#define SCV_SZ 		10
#define SCV_AD(x) 	((char *) x + 2)

/*
 * Job structure
 */
struct job {
	struct job       *j_next;	   /* Next job on queue		 */
	struct job       *j_prev;	   /* Previous job on queue	 */
	struct job       *j_cont;	   /* Next job block of request  */
	struct sb        *j_sb;		   /* SCSI block for this job	 */
	struct buf       *j_bp;		   /* Pointer to buffer header	 */
	struct cdrom     *j_cdp;	   /* Device to be accessed	 */
	unsigned	  j_errcnt;	   /* Error count (for recovery) */ 
	daddr_t		  j_addr;	   /* Physical block address	 */
	union sc {			   /* SCSI command block	 */
		struct scs  ss;		   /*	group 0 (6 bytes)	*/
		struct scm  sm;		   /*	group 1 (10 bytes)	*/
		struct scv  sv;		   /*	group 6 (10 bytes)	*/
	} j_cmd;
};

/*
 * cdrom information structure
 */
struct cdrom {
	struct job       *cd_first;	    /* Head of job queue	 */
	struct job       *cd_last;	    /* Tail of job queue	 */
	struct job       *cd_next;	    /* Next job to send to HA	 */
	struct job       *cd_batch;	    /* Elevator batch pointer	 */
	struct scsi_ad	  cd_addr;	    /* SCSI address		 */
	unsigned long	  cd_sendid;	    /* Timeout id for send	 */
	unsigned  	  cd_state;	    /* Operational state	 */ 
	unsigned  	  cd_count;	    /* Number of jobs on Q	 */ 
	unsigned 	  cd_npend;	    /* Number of jobs sent 	 */ 
	unsigned	  cd_fltcnt;	    /* Retry cnt for fault jobs	 */
	struct job       *cd_fltjob;	    /* Job associated with fault */
	struct sb        *cd_fltreq;	    /* SB for request sense	 */
	struct sb        *cd_fltres;	    /* SB for resume job	 */
	struct scs	  cd_fltcmd;	    /* Request Sense command	 */
	struct sense	  cd_sense;	    /* Request Sense data	 */
	struct capacity	  cd_capacity;	    /* Read Capacity data	 */
};
