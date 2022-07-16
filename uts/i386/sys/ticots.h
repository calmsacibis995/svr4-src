/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ticots.h	1.6.3.1"
/*
 *	ticots provider-dependent info
 *	(provider-independent applications must not include this header file)
 */

/*
 *	disconnect reason codes	 (see t_rcvdis())
 */
#define TCO_NOPEER	 	ECONNREFUSED	/* no listener on dest addr */
#define TCO_PEERNOROOMONQ	ECONNREFUSED	/* peer has no room on incoming queue */
#define TCO_PEERBADSTATE	ECONNREFUSED	/* peer in wrong state */
#define TCO_PEERINITIATED 	ECONNRESET	/* peer-initiated disconnect */
#define TCO_PROVIDERINITIATED 	ECONNABORTED	/* provider-initiated disconnect */

/*
 *	options (flattened linked-list of flattened C-structures)
 */
#define TCO_OPT_NOHDR		0		/* invalid nexthdr offset (end of list) */

#define TCO_OPT_NOOP		1		/* no-op opt -- default */
#define TCO_OPT_SETID		2		/* set ident */
#define TCO_OPT_GETID		3		/* get ident */
#define TCO_OPT_UID		4		/* uid info */
#define TCO_OPT_GID		5		/* gid info */
#define TCO_OPT_RUID		6		/* ruid info */
#define TCO_OPT_RGID		7		/* rgid info */

#define TCO_IDFLG_UID		0x1		/* uid flag */
#define TCO_IDFLG_GID		0x2		/* gid flag */
#define TCO_IDFLG_RUID		0x4		/* ruid flag */
#define TCO_IDFLG_RGID		0x8		/* rgid flag */

/* header for maintaining list of opts; one for each opt in list;
   offsets are measured from beginning of options buffer;
   headers must occur in increasing order, to avoid loops */
struct tco_opt_hdr {
	long			hdr_thisopt_off;	/* offset to current opt */
	long			hdr_nexthdr_off;	/* offset to next hdr */
};

/* no-op opt -- the default */
struct tco_opt_noop {
	long			noop_type;	/* TCO_OPT_NOOP; must be first */
};

/* set ident opt -- subsequent t_rcvudata()'s
   (i.e., T_UNITDATA_IND) will contain peer's ident info */
struct tco_opt_setid {
	long			setid_type;	/* TCO_OPT_SETID; must be first */
	long			setid_flg;	/* which id opts to set */
};

/* get ident opt */
struct tco_opt_getid {
	long			getid_type;	/* TCO_OPT_GETID; must be first */
	long			getid_flg;	/* which id opts are set */
};

/* uid info opt */
struct tco_opt_uid {
	long			uid_type;	/* TCO_OPT_UID; must be first */
	uid_t			uid_val;	/* effective user id */
};

/* gid info opt */
struct tco_opt_gid {
	long			gid_type;	/* TCO_OPT_GID; must be first */
	gid_t			gid_val;	/* effective group id */
};

/* ruid info opt */
struct tco_opt_ruid {
	long			ruid_type;	/* TCO_OPT_RUID; must be first */
	uid_t			ruid_val;	/* real user id */
};

/* rgid info opt */
struct tco_opt_rgid {
	long			rgid_type;	/* TCO_OPT_RGID; must be first */
	gid_t			rgid_val;	/* real group id */
};

/* union of all the opts */
union tco_opt {
	long			opt_type;	/* opt type; must be first */
	struct tco_opt_noop	opt_noop;	/* noop opt */
	struct tco_opt_setid	opt_setid;	/* set ident opt */
	struct tco_opt_getid	opt_getid;	/* get ident opt */
	struct tco_opt_uid	opt_uid;	/* uid info opt */
	struct tco_opt_gid	opt_gid;	/* gid info opt */
	struct tco_opt_ruid	opt_ruid;	/* ruid info opt */
	struct tco_opt_rgid	opt_rgid;	/* rgid info opt */
};

/******************************************************************************/

#ifdef _KERNEL

/*
 *	transport endpoint structure
 */
struct tco_endpt {
	struct tco_endpt	*te_folist;	/* forw ptr, list of open endpts */
	struct tco_endpt	*te_bolist;	/* back ptr, list of open endpts */
	struct tco_endpt	*te_frqlist;	/* forw ptr, te_rq list */
	struct tco_endpt	*te_brqlist;	/* back ptr, te_rq list */
	struct tco_endpt	*te_fblist;	/* forw ptr, list of endpts bound to addr */
	struct tco_endpt	*te_bblist;	/* back ptr, list of endpts bound to addr */
	queue_t			*te_rq;		/* stream read queue */
	struct tco_addr		*te_addr;	/* addr bound to this endpt */
	minor_t			te_min;		/* minor number */
	unsigned short		te_rqhash;	/* te_rq hash bucket */
	char 			te_state;	/* state of interface */
	char			te_flg;		/* internal flags */
	long			te_idflg;	/* ident flags */
	unsigned char		te_qlen;	/* max incoming connect reqs pending */
	unsigned char		te_nicon;	/* num of incoming connect reqs pending */
#define TCO_MAXQLEN		8		/* must be <= (1 << (NBBY*sizeof(te_nicon))) */
	struct tco_endpt	*te_icon[TCO_MAXQLEN];	/* incoming connect requests pending */
	struct tco_endpt	*te_ocon;	/* outgoing connect request pending */
	struct tco_endpt	*te_con;	/* connected endpt */
	uid_t			te_uid;		/* uid */
	gid_t			te_gid;		/* gid */
	uid_t			te_ruid;	/* ruid */
	gid_t			te_rgid;	/* rgid */
};
typedef struct tco_endpt	tco_endpt_t;

/*
 *	transport addr structure
 */
struct tco_addr {
	struct tco_addr		*ta_falist;	/* forw ptr, list of bound addrs */
	struct tco_addr		*ta_balist;	/* back ptr, list of bound addrs */
	struct tco_endpt	*ta_hblist;	/* head ptr, list of endpts bound to this addr */
	struct tco_endpt	*ta_tblist;	/* tail ptr, list of endpts bound to this addr */
	unsigned short 		ta_ahash;	/* addr hash bucket */
	long			ta_alen;	/* length of abuf */
	char			*ta_abuf;	/* the addr itself */
};
typedef struct tco_addr		tco_addr_t;

/*
 *	registered id
 */
#ifdef TICOTS
#define TCO_ID			10002
#endif
#ifdef TICOTSORD
#define	TCO_ID			10003
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
#define TCO_NENDPT		(OMAXMIN+1)
#ifdef TICOTS
#define TCO_SERVTYPE		T_COTS
#endif
#ifdef TICOTSORD
#define TCO_SERVTYPE		T_COTS_ORD
#endif
#define TCO_TIDUSZ		(4*1024)		/* max packet size */
#define TCO_DEFAULTADDRSZ	4			/* default addr sz */
/* can't make the following 4 sizes -1 (unlimited), because of bug in TLI/TPI specs:
   unlimited data can be sent but cannot received in a well-specified way
   (receiver doesn't know how big to make buffer, and T_MORE flag can't be used) */
#define TCO_ADDRSZ		(256-16)		/* 16 = sizeof(struct T_bind_req)
							      = sizeof(struct T_bind_ack) */
#define TCO_OPTSZ		(TCO_TIDUSZ-16)		/* 16 = sizeof(struct T_optmgmt_req)
							      = sizeof(struct T_optmgmt_ack) */
#define TCO_CDATASZ		(TCO_TIDUSZ-24)		/* 24 = sizeof(struct T_conn_req) + 4
							      = sizeof(struct T_conn_ind)
							      = sizeof(struct T_conn_res) + 4
							      = sizeof(struct T_conn_con) + 4 */
#define TCO_DDATASZ		(TCO_TIDUSZ-12)		/* 8 = sizeof(struct T_discon_req) + 4
							     = sizeof(struct T_discon_ind) */
#define TCO_TSDUSZ		-1			/* unlimited */
#define TCO_ETSDUSZ		-1			/* unlimited */
#define TCO_MINPSZ		0
#define TCO_MAXPSZ		TCO_TIDUSZ
#define TCO_LOWAT		(TCO_TIDUSZ/4)
#define TCO_HIWAT		(4*TCO_TIDUSZ)

/*
 *	te_flg
 */
#define TCO_ZOMBIE		0x1			/* fatal error on endpoint */

/*
 *	pass/fail indicators
 */
#define TCO_PASS		0
#define TCO_FAIL		(!TCO_PASS)
#define TCO_REALOPT		0x01			/* for tco_ckopt() */
#define TCO_NOOPOPT		0x02			/* for tco_ckopt() */
#define TCO_BADFORMAT		0x04			/* for tco_ckopt() */
#define TCO_BADTYPE		0x08			/* for tco_ckopt() */
#define TCO_BADVALUE		0x10			/* for tco_ckopt() */
#define UNIX_PASS		0			/* should be standardized */
#define UNIX_FAIL		(!UNIX_PASS)		/* should be standardized */
#define BADSEQNUM		((long)(-1))		/* should be standardized */

/*
 *	internal defines
 */
#define TCO_BIND		1
#define TCO_CONN		2
#define TCO_OPEN		3
#define TCO_RQ			4
#define TCO_IDFLG_ALL		(TCO_IDFLG_UID | TCO_IDFLG_GID | TCO_IDFLG_RUID | TCO_IDFLG_RGID)
#define TCO_MHASH		5
#define TCO_NMHASH		(1 << TCO_MHASH)	/* num of hash buckets in open endpt table */
#define TCO_MMASK		(TCO_NMHASH - 1)
#define TCO_RQHASH		5			/* must be <= NBBY*sizeof(te_rqhash) */
#define TCO_NRQHASH		(1 << TCO_RQHASH)	/* num of hash buckets in te_rq table */
#define TCO_RQMASK		(TCO_NRQHASH - 1)
/* following magic number and shift factor for fibonacci hash function */
#define TCO_RQMAGIC		0x9ce14b36
#define TCO_RQSHIFT		(NBBY*sizeof(int) - TCO_RQHASH)
#define TCO_AHASH		5			/* must be <= NBBY*sizeof(ta_ahash) */
#define TCO_NAHASH		(1 << TCO_AHASH)	/* num of hash buckets in bound addr table */
#define TCO_AMASK		(TCO_NAHASH - 1)

/*
 *	some useful macros
 */
#define tco_min(TE)		((TE)->te_min)
#define tco_mkmhash(TE)		((unsigned)(tco_min(TE)) & TCO_MMASK)
#define tco_mhash(TE)		tco_mkmhash(TE)
#define tco_mkrqhash(TE)	(((((unsigned)((TE)->te_rq))*TCO_RQMAGIC) >> TCO_RQSHIFT) & TCO_RQMASK)
#define tco_rqhash(TE)		((unsigned)(TE)->te_rqhash)
#define tco_alen(TA)		((TA)->ta_alen)
#define tco_abuf(TA)		((TA)->ta_abuf)
#define tco_ahash(TA)		((unsigned)(TA)->ta_ahash)
#define tco_mkahash(TA)		((unsigned)(tco_sumbytes(tco_abuf(TA),tco_alen(TA)) & TCO_AMASK))
#define tco_eqabuf(TA,TB)	((tco_alen(TA) == tco_alen(TB)) \
				 && tco_bequal(tco_abuf(TA),tco_abuf(TB),tco_alen(TA)))

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
