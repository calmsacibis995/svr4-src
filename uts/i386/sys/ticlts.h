/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ticlts.h	1.6.3.1"
/*
 *	ticlts provider-dependent info
 *	(provider-independent applications must not include this header file)
 */

/*
 *	unitdata error codes (see t_rcvuderr())
 */
#define TCL_BADADDR		1		/* bad addr specification */
#define TCL_BADOPT		2		/* bad option specification */
#define TCL_NOPEER		3		/* dest addr is unbound */
#define TCL_PEERBADSTATE	4		/* peer in wrong state */

/*
 *	options (flattened linked-list of flattened C-structures)
 */
#define TCL_OPT_NOHDR		0		/* invalid nexthdr offset (end of list) */

#define TCL_OPT_NOOP		1		/* no-op opt -- default */
#define TCL_OPT_SETID		2		/* set ident */
#define TCL_OPT_GETID		3		/* get ident */
#define TCL_OPT_UID		4		/* uid info */
#define TCL_OPT_GID		5		/* gid info */
#define TCL_OPT_RUID		6		/* ruid info */
#define TCL_OPT_RGID		7		/* rgid info */

#define TCL_IDFLG_UID		0x1		/* uid flag */
#define TCL_IDFLG_GID		0x2		/* gid flag */
#define TCL_IDFLG_RUID		0x4		/* ruid flag */
#define TCL_IDFLG_RGID		0x8		/* rgid flag */

/* header for maintaining list of opts; one for each opt in list;
   offsets are measured from beginning of options buffer;
   headers must occur in increasing order, to avoid loops */
struct tcl_opt_hdr {
	long			hdr_thisopt_off;	/* offset to current opt */
	long			hdr_nexthdr_off;	/* offset to next hdr */
};

/* no-op opt -- the default */
struct tcl_opt_noop {
	long			noop_type;	/* TCL_OPT_NOOP; must be first */
};

/* set ident opt -- subsequent t_rcvudata()'s
   (i.e., T_UNITDATA_IND) will contain peer's ident info */
struct tcl_opt_setid {
	long			setid_type;	/* TCL_OPT_SETID; must be first */
	long			setid_flg;	/* which id opts to set */
};

/* get ident opt */
struct tcl_opt_getid {
	long			getid_type;	/* TCL_OPT_GETID; must be first */
	long			getid_flg;	/* which id opts are set */
};

/* uid info opt */
struct tcl_opt_uid {
	long			uid_type;	/* TCL_OPT_UID; must be first */
	uid_t			uid_val;	/* effective user id */
};

/* gid info opt */
struct tcl_opt_gid {
	long			gid_type;	/* TCL_OPT_GID; must be first */
	gid_t			gid_val;	/* effective group id */
};

/* ruid info opt */
struct tcl_opt_ruid {
	long			ruid_type;	/* TCL_OPT_RUID; must be first */
	uid_t			ruid_val;	/* real user id */
};

/* rgid info opt */
struct tcl_opt_rgid {
	long			rgid_type;	/* TCL_OPT_RGID; must be first */
	gid_t			rgid_val;	/* real group id */
};

/* union of all the opts */
union tcl_opt {
	long			opt_type;	/* opt type; must be first */
	struct tcl_opt_noop	opt_noop;	/* noop opt */
	struct tcl_opt_setid	opt_setid;	/* set ident opt */
	struct tcl_opt_getid	opt_getid;	/* get ident opt */
	struct tcl_opt_uid	opt_uid;	/* uid info opt */
	struct tcl_opt_gid	opt_gid;	/* gid info opt */
	struct tcl_opt_ruid	opt_ruid;	/* ruid info opt */
	struct tcl_opt_rgid	opt_rgid;	/* rgid info opt */
};

/******************************************************************************/

#ifdef _KERNEL

/*
 *	transport endpoint structure
 */
struct tcl_endpt {
	struct tcl_endpt	*te_folist;	/* forw ptr, list of open endpts */
	struct tcl_endpt	*te_bolist;	/* back ptr, list of open endpts */
	queue_t			*te_rq;		/* stream read queue */
	queue_t			*te_backwq;	/* back q on WR side for flow cntl */
	struct tcl_addr		*te_addr;	/* address bound to this endpt */
	minor_t			te_min;		/* minor number */
	char			te_state;	/* state of interface */
	char			te_flg;		/* internal flags */
	long			te_idflg;	/* ident flags */
	uid_t			te_uid;		/* uid */
	gid_t			te_gid;		/* gid */
	uid_t			te_ruid;	/* ruid */
	gid_t			te_rgid;	/* rgid */
};
typedef struct tcl_endpt	tcl_endpt_t;

/*
 *	transport addr structure
 */
struct tcl_addr {
	struct tcl_addr		*ta_falist;	/* forw ptr, list of bound addrs */
	struct tcl_addr		*ta_balist;	/* back ptr, list of bound addrs */
	struct tcl_endpt	*ta_blist;	/* list (<= 1) of endpts bound to this addr */
	unsigned short		ta_ahash;	/* addr hash bucket */
	long			ta_alen;	/* length of abuf */
	char			*ta_abuf;	/* the addr itself */
};
typedef struct tcl_addr		tcl_addr_t;

/* M_CTL types.
 */
#define		TCL_IOCTL	('T'<<8)
#define		TCL_LINK	(TCL_IOCTL|101)
#define		TCL_UNLINK	(TCL_IOCTL|102)

/* Socket link M_CTL structure.
 */
struct tcl_sictl {
	long	type;
	long	ADDR_offset;
	long	ADDR_len;
};

/*
 *	registered id
 */
#define TCL_ID			10001

/*
 *	macro to change state
 *	NEXTSTATE(event, current state)
 */
#define NEXTSTATE(X,Y)		ti_statetbl[X][Y]	/* should be standardized */
#define NR 			127	/* unreachable state */	/* should be standardized */

/*
 *	basic constants
 */
#define TCL_NENDPT		(OMAXMIN+1)
#define TCL_SERVTYPE		T_CLTS
#define TCL_TIDUSZ		(4*1024)		/* max packet size */
#define TCL_DEFAULTADDRSZ	4			/* default addr sz */
/* can't make the following 2 sizes -1 (unlimited), because of bug in TLI/TPI specs:
   unlimited data can be sent but cannot received in a well-specified way
   (receiver doesn't know how big to make buffer, and T_MORE flag can't be used) */
#define TCL_ADDRSZ		(256-24)		/* 24 = sizeof(struct T_bind_req) + 8
							      = sizeof(struct T_bind_ack) + 8
							      = sizeof(struct T_unitdata_req) + 4
							      = sizeof(struct T_uderror_ind) */
#define TCL_OPTSZ		(TCL_TIDUSZ-24)		/* 24 = sizeof(struct T_optmgmt_req) + 8
							      = sizeof(struct T_optmgmt_ack) + 8
							      = sizeof(struct T_unitdata_req) + 4
							      = sizeof(struct T_uderror_ind) */
#define TCL_CDATASZ		-2			/* connectionless */
#define TCL_DDATASZ		-2			/* connectionless */
#define TCL_TSDUSZ		TCL_TIDUSZ		/* connectionless */
#define TCL_ETSDUSZ		-2			/* connectionless */
#define TCL_MINPSZ		0
#define TCL_MAXPSZ		TCL_TIDUSZ
#define TCL_LOWAT		(TCL_TIDUSZ/4)
#define TCL_HIWAT		(4*TCL_TIDUSZ)

/*
 *	te_flg
 */
#define TCL_ZOMBIE		0x1			/* fatal error on endpoint */

/*
 *	pass/fail indicators
 */
#define TCL_PASS		0
#define TCL_FAIL		(!TCL_PASS)
#define TCL_REALOPT		0x01			/* for tcl_ckopt() */
#define TCL_NOOPOPT		0x02			/* for tcl_ckopt() */
#define TCL_BADFORMAT		0x04			/* for tcl_ckopt() */
#define TCL_BADTYPE		0x08			/* for tcl_ckopt() */
#define TCL_BADVALUE		0x10			/* for tcl_ckopt() */
#define UNIX_PASS		0			/* should be standardized */
#define UNIX_FAIL		(!UNIX_PASS)		/* should be standardized */

/*
 *	internal defines
 */
#define TCL_BIND		1
#define TCL_DEST		2
#define TCL_OPEN		3
#define TCL_IDFLG_ALL		(TCL_IDFLG_UID | TCL_IDFLG_GID | TCL_IDFLG_RUID | TCL_IDFLG_RGID)
#define TCL_MHASH		5
#define TCL_NMHASH		(1 << TCL_MHASH)	/* num of hash buckets in open endpt table */
#define TCL_MMASK		(TCL_NMHASH - 1)
#define TCL_AHASH		5			/* must be <= NBBY*sizeof(tcl_addr.ta_ahash) */
#define TCL_NAHASH		(1 << TCL_AHASH)	/* num of hash buckets in bound addr table */
#define TCL_AMASK		(TCL_NAHASH - 1)

/*
 *	some useful macros
 */
#define tcl_min(TE)		((TE)->te_min)
#define tcl_mkmhash(TE)		((unsigned)(tcl_min(TE)) & TCL_MMASK)
#define tcl_mhash(TE)		tcl_mkmhash(TE)
#define tcl_alen(TA)		((TA)->ta_alen)
#define tcl_abuf(TA)		((TA)->ta_abuf)
#define tcl_mkahash(TA)		((unsigned)(tcl_sumbytes(tcl_abuf(TA),tcl_alen(TA)) & TCL_AMASK))
#define tcl_ahash(TA)		((unsigned)(TA)->ta_ahash)
#define tcl_eqabuf(TA,TB)	((tcl_alen(TA) == tcl_alen(TB)) \
				 && tcl_bequal(tcl_abuf(TA),tcl_abuf(TB),tcl_alen(TA)))

/*
 *	STRLOG tracing levels:
 *
 *	0 = urgent
 *	1 = fatal
 *	2 = errack, uderr
 *	3 = interesting stuff
 *	4 = chit-chat
 */

#endif /* _KERNEL */
