/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STRPPC_H
#define _SYS_STRPPC_H

#ident	"@(#)head.sys:sys/strppc.h	1.4.7.1"

/*
 * b_state (board state) flags
 *
 *	ISSYSGEN:	board has been sysgen'ed and is operational
 *	CIOTYPE		common IO type command
 *	SYSGFAIL        indicated that SYSGEN has failed
 *	EXPRESS:	REQUEST EXPRESS entry "being used"
 */
#define ISSYSGEN	0x0001
#define CIOTYPE		0x0002
#define SYSGFAIL	0x0004
#define EXPRESS		0x8000


/*
 * t_dstat (special tty struc flags) flag definition
 *	SETOPT		a SET_OPTION command is in progress
 *	SPLITMSG	output message split into 64 byte chunks
 *	WENTRY		wait for xmit queue entry
 *      SUPBUF          have buffers been supplied for receive?
 *      WRTQDRAIN       Wait for write queues to drain to user's terminal
 *	CLDRAIN		wait for output to drain
 *	OPDRAIN		wait for output to drain
 *	WBREAK		wait for break send completion
 */
#define SETOPT		0x0001
#define SPLITMSG	0x0002
#define WENTRY 		0x0004
#define SUPBUF		0x0008
#define WRTQDRAIN       0x0010
#define CLDRAIN		0x0020
#define OPDRAIN		0x0040
#define WBREAK          0x0080

#define MAX_RBUF  17	/* 4 recv bufs/tty + 1 extra */
#define PPBUFSIZ 64	/* MAX. size of in/out buffer */

/*
 * Constants needed for determining the version
 * of the PORTS board, e. g. PORTS or HIPORTS
 *
 *	PPC_VERS	request version number of a ppc board (ioctl)
 *	O_VERS		oflag value for opening a board to read version number
 *	DEFAULTVER	default version number for boards without one
 *	HIPORTS		rom version number of the HIPORT board
 */

#define PPC_VERS (('v'<<8)|1)   
#define O_VERS  0200	       
#define DEFAULTVER 1           
#define HIPORTS  2              

struct pprbuf
{
	mblk_t *bp;	/* message block pointer */
	long sp;	/* PHYSICAL address of bp->b_datap->db_base */
	long ep;	/* PHYSICAL address of bp->b_datap->db_lim */
};

/*
 * PORTS board control block
 */
struct ppcboard {
	unsigned short b_state;	/* board state flags		     */
	short dcb;		/* desired nbr of cblocks for rqueue */
	short qcb;		/* actual nbr of allocated cblocks   */
	short retcode;		/* return code for lla commands	     */
 	short p_nbr[NUM_QUEUES];	/* bnr of entries in queue   */
	RQUEUE rqueue;
	CQUEUE cqueue;
	struct pprbuf rbp[MAX_RBUF];/* allocated recv buffers for PPC */
	};

#define BSTATE(b)	npp_board[b].b_state
#define PNBR(b,p)	npp_board[b].p_nbr[p]



/*
 * Variables allocated by the operating system
 */

extern int npp_cnt;
extern struct strtty npp_tty[];
extern char *npp_addr[];
extern struct ppcboard npp_board[];
extern int ncsbit[];
extern short nppcbid[];
extern short nppcpid[];

/*
 Variables allocated by the driver
 */

extern int npp_bnbr;

#endif	/* _SYS_STRPPC_H */
