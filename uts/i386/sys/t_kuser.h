/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_T_KUSER_H
#define	_T_KUSER_H

#ident	"@(#)head.sys:sys/t_kuser.h	1.4.2.1"

/*	@(#)t_kuser.h 1.1 88/12/12 SMI	*/

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/* Note this structure will need to be expanded to handle data
 * related to connection orientated transports. 
 */
typedef struct tiuser {
	struct	file *fp;
	struct	t_info tp_info;	/* Transport provider Info. */
	int	flags;
} TIUSER;
#define		TIUSERSZ	sizeof(TIUSER)

struct knetbuf {
	mblk_t   *udata_mp;	/* current receive streams block */
	unsigned int maxlen;
	unsigned int len;
	char     *buf;
};

struct t_kunitdata {
	struct netbuf addr;
	struct netbuf opt;
	struct knetbuf udata;
};

#ifdef DEBUG
extern int	ktli_log();
extern int	ktlilog;

#define		KTLILOG(A, B, C) ((void)((ktlilog) && ktli_log((A), (B), (C))))
#else
#define		KTLILOG(A, B, C)
#endif

/* flags
 */
#define		TIME_UP		0x01

extern int	t_kalloc();
extern int	t_kbind();
extern int	t_kclose();
extern int	t_kconnect();
extern int	t_kfree();
extern int	t_kgetstate();
extern int	t_kopen();
extern int	t_krcvudata();
extern int	t_ksndudata();
extern int	t_kspoll();
extern int	t_kunbind();
extern int	tli_send();
extern int	tli_recv();
extern int	get_ok_ack();

/* these make life a lot easier
 */
#define		TCONNREQSZ	sizeof(struct T_conn_req)
#define		TCONNRESSZ	sizeof(struct T_conn_res)
#define		TDISCONREQSZ	sizeof(struct T_discon_req)
#define		TDATAREQSZ	sizeof(struct T_data_req)
#define		TEXDATAREQSZ	sizeof(struct T_exdata_req)
#define		TINFOREQSZ	sizeof(struct T_info_req)
#define		TBINDREQSZ	sizeof(struct T_bind_req)
#define		TUNBINDREQSZ	sizeof(struct T_unbind_req)
#define		TUNITDATAREQSZ	sizeof(struct T_unitdata_req)
#define		TOPTMGMTREQSZ	sizeof(struct T_optmgmt_req)
#define		TORDRELREQSZ	sizeof(struct T_ordrel_req)
#define		TCONNINDSZ	sizeof(struct T_conn_ind)
#define		TCONNCONSZ	sizeof(struct T_conn_con)
#define		TDISCONINDSZ	sizeof(struct T_discon_ind)
#define		TDATAINDSZ	sizeof(struct T_data_ind)
#define		TEXDATAINDSZ	sizeof(struct T_exdata_ind)
#define		TINFOACKSZ	sizeof(struct T_info_ack)
#define		TBINDACKSZ	sizeof(struct T_bind_ack)
#define		TERRORACKSZ	sizeof(struct T_error_ack)
#define		TOKACKSZ	sizeof(struct T_ok_ack)
#define		TUNITDATAINDSZ	sizeof(struct T_unitdata_ind)
#define		TUDERRORINDSZ	sizeof(struct T_uderror_ind)
#define		TOPTMGMTACKSZ	sizeof(struct T_optmgmt_ack)
#define		TORDRELINDSZ	sizeof(struct T_ordrel_ind)
#define		TPRIMITIVES	sizeof(struct T_primitives)

/******************************************************************************/

#endif	/* _T_KUSER_ */
