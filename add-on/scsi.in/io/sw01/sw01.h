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

#ident	"@(#)scsi.in:io/sw01/sw01.h	1.3"

/*
 * the SCSI WORM minor device number is interpreted as follows:
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

#define UNIT(x)		(minor(x) & 0x07)

#define WMNOTCS		-1		/* No TC's configured in system	*/

#define BLKSIZE  512	/* default block size */
#define BLKSHFT  9
#define BLKMASK  (BLKSIZE-1)

#define JTIME	10000	/* ten sec for a job */
#define LATER   20000

#define MAXPEND   2
#define MAXRETRY  2

#define INQ_SZ	126	/* inquiry data size */
#define CB_SZ	1024	/* control block data size */

/* Values of dk_state */
#define	WM_INIT		0x01		   /* Disk has been initialized  */
#define	WM_WOPEN	0x02		   /* Waiting for 1st open	 */
#define	WM_DIR		0x08		   /* Elevator direction flag	 */
#define	WM_SUSP		0x10		   /* Disk Q suspended by HA	 */
#define	WM_SEND		0x20		   /* Send requested timeout	 */
#define	WM_PARMS	0x40		   /* Disk parms set and valid	 */

#define	GROUP0		0		/* group 0 command 		*/
#define	GROUP1		1		/* group 1 command 		*/
#define	GROUP6		6		/* group 6 command 		*/
#define	GROUP7		7		/* group 7 command 		*/

/*
 * Read Capacity data
 */
struct capacity {
	unsigned long    wm_addr;	   /* Logical block address	*/
	unsigned long    wm_len;	   /* Block length	 	*/
};

#define RDCAP_SZ        8
#define RDCAP_AD(x)     ((char *)(x))

/*
 * Job structure
 */
struct job {
	struct job       *j_next;	   /* Next job on queue		 */
	struct job       *j_prev;	   /* Previous job on queue	 */
	struct job       *j_cont;	   /* Next job block of request  */
	struct sb        *j_sb;		   /* SCSI block for this job	 */
	struct buf       *j_bp;		   /* Pointer to buffer header	 */
	struct worm      *j_wmp;	   /* Device to be accessed	 */
	unsigned	  j_errcnt;	   /* Error count (for recovery) */ 
	daddr_t		  j_addr;	   /* Physical block address	 */
	union sc {			   /* SCSI command block	 */
		struct scs  ss;		/*	group 0,6 (6 bytes)	*/
		struct scm  sm;		/*	group 1,7 (10 bytes)	*/
	} j_cmd;
};

/*
 * worm information structure
 */
struct worm {
	struct job       *wm_first;	    /* Head of job queue	 */
	struct job       *wm_last;	    /* Tail of job queue	 */
	struct job       *wm_next;	    /* Next job to send to HA	 */
	struct job       *wm_batch;	    /* Elevator batch pointer	 */
	struct scsi_ad	  wm_addr;	    /* SCSI address		 */
	unsigned long	  wm_sendid;	    /* Timeout id for send	 */
	unsigned  	  wm_state;	    /* Operational state	 */ 
	unsigned  	  wm_count;	    /* Number of jobs on Q	 */ 
	unsigned 	  wm_npend;	    /* Number of jobs sent 	 */ 
	unsigned	  wm_fltcnt;	    /* Retry cnt for fault jobs	 */
	struct job       *wm_fltjob;	    /* Job associated with fault */
	struct sb        *wm_fltreq;	    /* SB for request sense	 */
	struct sb        *wm_fltres;	    /* SB for resume job	 */
	struct scs	  wm_fltcmd;	    /* Request Sense command	 */
	struct sense	  wm_sense;	    /* Request Sense data	 */
	struct capacity	  wm_capacity;	    /* Read Capacity data	 */
};
