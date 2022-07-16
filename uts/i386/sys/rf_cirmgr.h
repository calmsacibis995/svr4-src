/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RF_CIRMGR_H
#define _SYS_RF_CIRMGR_H

#ident	"@(#)head.sys:sys/rf_cirmgr.h	1.12.3.1"

#define MAXTOKLEN sizeof(rf_token_t)       /* maximum token length in bytes */

struct gdpmisc {
        int	hetero;
        int	version;
        int	ngroups_max;
};
typedef struct gdpmisc gdpmisc_t;

struct rf_token {
        int     t_id;    /* token id for differentiating multiple ckts  */
        char    t_uname[MAXDNAME]; /* full domain name of machine */
};
typedef struct rf_token rf_token_t;

struct gdp {
	struct queue	*queue;		/* pointer to associated stream head */
	struct file	*file;		/* to stream head we stole */
	short		mntcnt;		/* number of mounts on this stream */
	sysid_t		sysid;
	short		constate;	/* connection info */
	struct {
		char	istate;		/* input state machine */
		int	oneshot : 1;	/* 1 if expect to find entire incoming
					 * message in queue at same time */
		int	mcdecan	: 1;	/* messsage and common headers already
					 * decanonized */
		int	rhdecan : 1;	/* request or response header already
					 * decanonized */
	} input;
	int		hetero;		/* need to canonicalize messages */
	int		version;	/* RFS version at the other end */
	long		timeskew_sec;	/* time skew in sec */
	rf_token_t	token;		/* circuit identification */
	char		*idmap[2];	/* 0=uid=UID_DEV, 1=gid=GID_DEV */
	ushort		timeout;
	struct msgb	*hdr;		/* message header collected so far */
	struct msgb	*idata;		/* request/response collected so far */
	size_t		hlen;		/* header length yet to be collected */
	size_t		rhlen;		/* saved request/response header len */
	size_t		dlen;		/* data length yet to be collected */
	size_t		maxpsz;		/* max TIDU size of the provider */
	int		ngroups_max;	/* max # suppl. groups per user */
	size_t		datasz;		/* max data in RFS message */
    ulong       outseq;
    ulong       inseq;

};
typedef struct gdp gdp_t;

/*
 * Sysids are unique only among pairs of machines and are constructed
 * as follows:
 *      hibyte:  index on other machine of GDP on that end of virtual
 *               circuit
 *      lobyte:  index of GDP on this end of virtual circuit
 * TO DO:  network-unique sysids
 */
#define SDTOSYSID(sdp)	((gdp_t *)(sdp)->sd_queue->q_ptr)->sysid
#define QPTOGP(qp)	((gdp_t *)((queue_t *)(qp))->q_ptr)
extern int maxgdp;
extern gdp_t *gdp;


/* GDP circuit states */
#define GDPFREE		0
#define GDPCONNECT	1	/* sound circuit */
#define GDPDISCONN	2	/* not yet in recovery */
#define GDPRECOVER	4

/* istate */
#define GDPSTGMC	0 /* gathering rf_message_t and rf_common_t headers */
#define GDPSTPMC	1 /* pulling up rf_message_t and rf_common_t headers */
#define GDPSTGR		2 /* gathering rf_request_t or rf_response_t header */
#define GDPSTPR		3 /* pulling up rf_request_t or rf_response_t header */
#define GDPSTGD		4 /* gathering data */

/* RFS version numbers */
#define RFS1DOT0	1
#define RFS2DOT0	2

#ifdef _KERNEL

extern queue_t	*gdp_sysidtoq();
extern int	gdp_get_circuit();
extern void	gdp_put_circuit();
extern int	gdp_init();
extern void	gdp_discon();
extern void	gdp_j_accuse();
extern void	gdp_kill();
extern void	gdp_put_discon();

#endif /* _KERNEL */

#endif /* _SYS_RF_CIRMGR_H */
