/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ticotsord.h	1.5.3.1"
/*
 *	ticotsord provider-dependent info
 *	(provider-independent applications must not include this header file)
 */

/*
 *	disconnect reason codes	 (see t_rcvdis())
 */
#define TCOO_NOPEER	 	1		/* no listener on dest addr */
#define TCOO_PEERNOROOMONQ	2		/* peer has no room on incoming queue */
#define TCOO_PEERBADSTATE	3		/* peer in wrong state */
#define TCOO_PEERINITIATED 	4		/* peer-initiated disconnect */
#define TCOO_PROVIDERINITIATED 	5		/* provider-initiated disconnect */

/*
 *	options (flattened linked-list of flattened C-structures)
 */
#define TCOO_OPT_NOHDR		0		/* invalid nexthdr offset (end of list) */

#define TCOO_OPT_NOOP		1		/* no-op opt -- default */
#define TCOO_OPT_SETID		2		/* set ident */
#define TCOO_OPT_GETID		3		/* get ident */
#define TCOO_OPT_UID		4		/* uid info */
#define TCOO_OPT_GID		5		/* gid info */
#define TCOO_OPT_RUID		6		/* ruid info */
#define TCOO_OPT_RGID		7		/* rgid info */

#define TCOO_IDFLG_UID		0x1		/* uid flag */
#define TCOO_IDFLG_GID		0x2		/* gid flag */
#define TCOO_IDFLG_RUID		0x4		/* ruid flag */
#define TCOO_IDFLG_RGID		0x8		/* rgid flag */

/* header for maintaining list of opts; one for each opt in list;
   offsets are measured from beginning of options buffer;
   headers must occur in increasing order, to avoid loops */
struct tcoo_opt_hdr {
	long			hdr_thisopt_off;	/* offset to current opt */
	long			hdr_nexthdr_off;	/* offset to next hdr */
};

/* no-op opt -- the default */
struct tcoo_opt_noop {
	long			noop_type;	/* TCOO_OPT_NOOP; must be first */
};

/* set ident opt -- subsequent t_rcvudata()'s
   (i.e., T_UNITDATA_IND) will contain peer's ident info */
struct tcoo_opt_setid {
	long			setid_type;	/* TCOO_OPT_SETID; must be first */
	long			setid_flg;	/* which id opts to set */
};

/* get ident opt */
struct tcoo_opt_getid {
	long			getid_type;	/* TCOO_OPT_GETID; must be first */
	long			getid_flg;	/* which id opts are set */
};

/* uid info opt */
struct tcoo_opt_uid {
	long			uid_type;	/* TCOO_OPT_UID; must be first */
	uid_t			uid_val;	/* effective user id */
};

/* gid info opt */
struct tcoo_opt_gid {
	long			gid_type;	/* TCOO_OPT_GID; must be first */
	gid_t			gid_val;	/* effective group id */
};

/* ruid info opt */
struct tcoo_opt_ruid {
	long			ruid_type;	/* TCOO_OPT_RUID; must be first */
	uid_t			ruid_val;	/* real user id */
};

/* rgid info opt */
struct tcoo_opt_rgid {
	long			rgid_type;	/* TCOO_OPT_RGID; must be first */
	gid_t			rgid_val;	/* real group id */
};

/* union of all the opts */
union tcoo_opt {
	long			opt_type;	/* opt type; must be first */
	struct tcoo_opt_noop	opt_noop;	/* noop opt */
	struct tcoo_opt_setid	opt_setid;	/* set ident opt */
	struct tcoo_opt_getid	opt_getid;	/* get ident opt */
	struct tcoo_opt_uid	opt_uid;	/* uid info opt */
	struct tcoo_opt_gid	opt_gid;	/* gid info opt */
	struct tcoo_opt_ruid	opt_ruid;	/* ruid info opt */
	struct tcoo_opt_rgid	opt_rgid;	/* rgid info opt */
};

/******************************************************************************/

#ifdef _KERNEL

/*
 *	transport endpoint structure
 */
struct tcoo_endpt {
	struct tcoo_endpt	*te_folist;	/* forw ptr, list of open endpts */
	struct tcoo_endpt	*te_bolist;	/* back ptr, list of open endpts */
	struct tcoo_endpt	*te_frqlist;	/* forw ptr, te_rq list */
	struct tcoo_endpt	*te_brqlist;	/* back ptr, te_rq list */
	struct tcoo_endpt	*te_fblist;	/* forw ptr, list of endpts bound to addr */
	struct tcoo_endpt	*te_bblist;	/* back ptr, list of endpts bound to addr */
	queue_t			*te_rq;		/* stream read queue */
	struct tcoo_addr		*te_addr;	/* addr bound to this endpt */
	minor_t			te_min;		/* minor number */
	unsigned short		te_rqhash;	/* te_rq hash bucket */
	char 			te_state;	/* state of interface */
	char			te_flg;		/* internal flags */
	long			te_idflg;	/* ident flags */
	unsigned char		te_qlen;	/* max incoming connect reqs pending */
	unsigned char		te_nicon;	/* num of incoming connect reqs pending */
#define TCOO_MAXQLEN		8		/* must be <= (1 << (NBBY*sizeof(te_nicon))) */
	struct tcoo_endpt	*te_icon[TCOO_MAXQLEN];	/* incoming connect requests pending */
	struct tcoo_endpt	*te_ocon;	/* outgoing connect request pending */
	struct tcoo_endpt	*te_con;	/* connected endpt */
	uid_t			te_uid;		/* uid */
	gid_t			te_gid;		/* gid */
	uid_t			te_ruid;	/* ruid */
	gid_t			te_rgid;	/* rgid */
};
typedef struct tcoo_endpt	tcoo_endpt_t;

/*
 *	transport addr structure
 */
struct tcoo_addr {
	struct tcoo_addr		*ta_falist;	/* forw ptr, list of bound addrs */
	struct tcoo_addr		*ta_balist;	/* back ptr, list of bound addrs */
	struct tcoo_endpt	*ta_hblist;	/* head ptr, list of endpts bound to this addr */
	struct tcoo_endpt	*ta_tblist;	/* tail ptr, list of endpts bound to this addr */
	unsigned short 		ta_ahash;	/* addr hash bucket */
	long			ta_alen;	/* length of abuf */
	char			*ta_abuf;	/* the addr itself */
};
typedef struct tcoo_addr		tcoo_addr_t;

/*
 *	registered id
 */
#ifdef TICOTS
#define TCOO_ID			10002
#endif
#ifdef TICOTSORD
#define	TCOO_ID			10003
#endif

/*
 *	macro to change state
 *	NEXTSTATE(event,current state)
 */
#define NEXTSTATE(X,Y)		ti_statetbl[X][Y]	/* should be standardized */
#define NR 			127	/* unreachable state */	/* should be standardized */

/*
 *	basic constants
 */
#define TCOO_NENDPT		(OMAXMIN+1)
#ifdef TICOTS
#define TCOO_SERVTYPE		T_COTS
#endif
#ifdef TICOTSORD
#define TCOO_SERVTYPE		T_COTS_ORD
#endif
#define TCOO_TIDUSZ		(4*1024)		/* max packet size */
#define TCOO_DEFAULTADDRSZ	4			/* default addr sz */
/* can't make the following 4 sizes -1 (unlimited), because of bug in TLI/TPI specs:
   unlimited data can be sent but cannot received in a well-specified way
   (receiver doesn't know how big to make buffer, and T_MORE flag can't be used) */
#define TCOO_ADDRSZ		(256-16)			/* 16 = sizeof(struct T_bind_req)
							      = sizeof(struct T_bind_ack) */
#define TCOO_OPTSZ		(TCOO_TIDUSZ-16)		/* 16 = sizeof(struct T_optmgmt_req)
							      = sizeof(struct T_optmgmt_ack) */
#define TCOO_CDATASZ		(TCOO_TIDUSZ-24)		/* 24 = sizeof(struct T_conn_req) + 4
							      = sizeof(struct T_conn_ind)
							      = sizeof(struct T_conn_res) + 4
							      = sizeof(struct T_conn_con) + 4 */
#define TCOO_DDATASZ		(TCOO_TIDUSZ-12)		/* 8 = sizeof(struct T_discon_req) + 4
							     = sizeof(struct T_discon_ind) */
#define TCOO_TSDUSZ		-1			/* unlimited */
#define TCOO_ETSDUSZ		-1			/* unlimited */
#define TCOO_MINPSZ		0
#define TCOO_MAXPSZ		TCOO_TIDUSZ
#define TCOO_LOWAT		(TCOO_TIDUSZ/4)
#define TCOO_HIWAT		(4*TCOO_TIDUSZ)

/*
 *	te_flg
 */
#define TCOO_ZOMBIE		0x1			/* fatal error on endpoint */

/*
 *	pass/fail indicators
 */
#define TCOO_PASS		0
#define TCOO_FAIL		(!TCOO_PASS)
#define TCOO_REALOPT		0x01			/* for tcoo_ckopt() */
#define TCOO_NOOPOPT		0x02			/* for tcoo_ckopt() */
#define TCOO_BADFORMAT		0x04			/* for tcoo_ckopt() */
#define TCOO_BADTYPE		0x08			/* for tcoo_ckopt() */
#define TCOO_BADVALUE		0x10			/* for tcoo_ckopt() */
#define UNIX_PASS		0			/* should be standardized */
#define UNIX_FAIL		(!UNIX_PASS)		/* should be standardized */
#define BADSEQNUM		((long)(-1))		/* should be standardized */

/*
 *	internal defines
 */
#define TCOO_BIND		1
#define TCOO_CONN		2
#define TCOO_OPEN		3
#define TCOO_RQ			4
#define TCOO_IDFLG_ALL		(TCOO_IDFLG_UID | TCOO_IDFLG_GID | TCOO_IDFLG_RUID | TCOO_IDFLG_RGID)
#define TCOO_MHASH		5
#define TCOO_NMHASH		(1 << TCOO_MHASH)	/* num of hash buckets in open endpt table */
#define TCOO_MMASK		(TCOO_NMHASH - 1)
#define TCOO_RQHASH		5			/* must be <= NBBY*sizeof(te_rqhash) */
#define TCOO_NRQHASH		(1 << TCOO_RQHASH)	/* num of hash buckets in te_rq table */
#define TCOO_RQMASK		(TCOO_NRQHASH - 1)
/* following magic number and shift factor for fibonacci hash function */
#define TCOO_RQMAGIC		0x9ce14b36
#define TCOO_RQSHIFT		(NBBY*sizeof(int) - TCOO_RQHASH)
#define TCOO_AHASH		5			/* must be <= NBBY*sizeof(ta_ahash) */
#define TCOO_NAHASH		(1 << TCOO_AHASH)	/* num of hash buckets in bound addr table */
#define TCOO_AMASK		(TCOO_NAHASH - 1)

/*
 *	some useful macros
 */
#define tcoo_min(TE)		((TE)->te_min)
#define tcoo_mkmhash(TE)		((unsigned)(tcoo_min(TE)) & TCOO_MMASK)
#define tcoo_mhash(TE)		tcoo_mkmhash(TE)
#define tcoo_mkrqhash(TE)	(((((unsigned)((TE)->te_rq))*TCOO_RQMAGIC) >> TCOO_RQSHIFT) & TCOO_RQMASK)
#define tcoo_rqhash(TE)		((unsigned)(TE)->te_rqhash)
#define tcoo_alen(TA)		((TA)->ta_alen)
#define tcoo_abuf(TA)		((TA)->ta_abuf)
#define tcoo_ahash(TA)		((unsigned)(TA)->ta_ahash)
#define tcoo_mkahash(TA)		((unsigned)(tcoo_sumbytes(tcoo_abuf(TA),tcoo_alen(TA)) & TCOO_AMASK))
#define tcoo_eqabuf(TA,TB)	((tcoo_alen(TA) == tcoo_alen(TB)) \
				 && tcoo_bequal(tcoo_abuf(TA),tcoo_abuf(TB),tcoo_alen(TA)))

/*
 *	STRLOG tracing levels:
 *
 *	0 = urgent
 *	1 = fatal
 *	2 = errack
 *	3 = interesting stuff
 *	4 = chit-chat
 */

#endif /* _KERNEL */
