/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/char.h	1.1.2.1"

#ifndef	_SYS_CHAR_H
#define	_SYS_CHAR_H

#define	IBSIZE	16		/* "standard" input data block size */
#define	OBSIZE	64		/* "standard" output data block size */
#define	EBSIZE	16		/* "standard" echo data block size */

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define	MAXCHARPSZ	1024
#define	CHARPSZ	64

typedef struct copystate {
	ulong cpy_arg;
	ulong cpy_state;
} copy_state_t;

#define CHR_IN_0	0x0
#define CHR_IN_1	0x1
#define CHR_OUT_0	0xF000
#define CHR_OUT_1	0xF001


struct char_stat {
	queue_t *c_rqp;		/* saved pointer to read queue */
	queue_t *c_wqp;		/* saved pointer to write queue */
	unsigned long c_state;	/* internal state of tty module */
	mblk_t *c_rmsg;		/* ptr to read-side message being built */
	mblk_t *c_wmsg;		/* ptr to write-side message being built */
	charmap_t *c_map_p;	/* pointer to shared charmap_t with principal
				   stream */
	scrn_t *c_scrmap_p;
	xqInfo *c_xqinfo;
	struct v86dat *c_stashed_p_v86;
	proc_t *c_rawprocp;
	pid_t c_rawpid;
	kbstate_t c_kbstat;	/* pointer to keyboard state struct */
	copy_state_t c_copystate;
	xqEvent c_xevent;
	struct mouseinfo c_mouseinfo; /* the next 3 for mouse processing */
	mblk_t *c_heldmseread;
	int c_oldbutton;
#ifdef MERGE386
	void (*c_merge_kbd_ppi)(); /* Merge keyboard ppi function pointer */
	void (*c_merge_mse_ppi)(); /* Merge mouse ppi function pointer */
	struct mcon *c_merge_mcon; /* pointer to merge console structure */
#endif /* MERGE386 */
};

typedef struct char_stat charstat_t;

#ifdef MERGE386
struct chr_merge {
	void (*merge_kbd_ppi)();
	void (*merge_mse_ppi)();
	struct mcon *merge_mcon;
};

typedef struct chr_merge chr_merge_t;
#endif /* MERGE386 */
	

/*
 * Internal state bits.
 */

#define	C_RAWMODE	0x00000001
#define C_XQUEMDE	0x00000002
#define C_FLOWON	0x00000004
#define	C_MSEBLK	0x00000008
#define C_MSEINPUT	0x00000010

#endif /* _SYS_CHAR_H */
