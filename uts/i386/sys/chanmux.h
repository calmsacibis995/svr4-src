/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/chanmux.h	1.1.3.1"

#ifndef	_SYS_CHANMUX_H
#define	_SYS_CHANMUX_H

#define	CMUX_NUMSWTCH	10	/* keep history of last 10 switches */
#define	CMUXPSZ	32		/* maximun output packet size */

typedef struct cmux_stat {
	unsigned long cmux_num;
	unsigned long cmux_flg;
	queue_t *cmux_rqp;		/* saved pointer to read queue */
	queue_t *cmux_wqp;		/* saved pointer to write queue */
	struct ws_stat *cmux_wsp;
	unsigned long cmux_enqueue;	/* should messages be enqueued? */
	dev_t cmux_dev;			/* device number for this channel */
} cmux_t;

struct cmux_swtch {
	clock_t sw_time;
	unsigned long sw_chan; /* channel number made active */
};


typedef struct cmux_linkblk {
	unsigned long cmlb_iocresp; /* ACK or NACK? */
	unsigned long cmlb_flg; /* in use? */
	mblk_t *cmlb_iocmsg;
	unsigned long cmlb_err;
	struct linkblk cmlb_lblk;
} cmux_link_t;


typedef struct ws_stat {
	unsigned long w_ioctlchan;
	unsigned long w_ioctllstrm;
	unsigned long w_ioctlcnt;
	mblk_t *w_iocmsg;
	unsigned long w_state;
	cmux_t **w_cmuxpp;
	unsigned long w_numchan;
	cmux_link_t *w_princp; /* numchan allocated */
	cmux_link_t *w_lstrmsp;
	unsigned long w_numlstrms;
	unsigned long w_lstrms;
	unsigned long w_numswitch;
	struct cmux_swtch w_swtchtimes[CMUX_NUMSWTCH];
} cmux_ws_t;


typedef struct cmux_lstrm {
	cmux_ws_t *lstrm_wsp;
	unsigned long lstrm_flg;
	unsigned long lstrm_id;
	unsigned long lstrm_err;
} cmux_lstrm_t;


#define	MAXCMUXPSZ	1024
#define	CMUX_STRMALLOC	2
#define	CMUX_CHANALLOC	8
#define CMUX_WSALLOC	4


/*
 * Internal state bits.
 */

#define	CMUX_OPEN	0x1
#define	CMUX_CLOSE	0x2
#define	CMUX_WCLOSE	0x4
#define	CMUX_IOCTL	0x8

/*
 * cmux_lstrm_t flag bits
 */

#define	CMUX_SECSTRM	0x1
#define	CMUX_PRINCSTRM	0x2
#define	CMUX_PRINCSLEEP	0x4
#endif /* _SYS_CHANMUX_H */
