/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)//usr/src/head/rpcsvc/yp_prot.h.sl 1.1 4.0 12/08/90 33991 AT&T-USL"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 

/*
 * This file contains symbols and structures defining the rpc protocol
 * between the YP clients and the YP servers.  The servers are the YP 
 * database servers, and the YP.   
 */

/*
 * The following procedures are supported by the protocol:
 * 
 * YPPROC_NULL() returns () takes nothing, returns nothing.  This indicates
 * that the yp server is alive.
 * 
 * YPPROC_DOMAIN (char *) returns (bool_t) TRUE.  Indicates that the
 * responding yp server does serve the named domain; FALSE indicates no
 * support.
 * 
 * YPPROC_DOMAIN_NONACK (char *) returns (TRUE) if the yp server does serve
 * the named domain, otherwise does not return.  Used in the broadcast case.
 * 
 * YPPROC_MATCH (struct ypreq_key) returns (struct ypresp_val).  Returns the
 * right-hand value for a passed left-hand key, within a named map and
 * domain.
 * 
 * YPPROC_FIRST (struct ypreq_nokey) returns (struct ypresp_key_val).
 * Returns the first key-value pair from a named domain and map.
 * 
 * YPPROC_NEXT (struct ypreq_key) returns (struct ypresp_key_val).  Returns
 * the key-value pair following a passed key-value pair within a named
 * domain and map.
 *
 * YPPROC_XFR (struct ypreq_xfr) returns nothing.  Indicates to a server that
 * a map should be updated.
 *
 * YPPROC_NEWXFR (struct ypreq_newxfr) returns nothing.  Indicates to a server 
 * that a map should be updated. Uses protocol independent request struct.
 *
 * YPPROC_CLEAR	takes nothing, returns nothing.  Instructs a yp server to
 * close the current map, so that old versions of the disk file don't get
 * held open.
 * 
 * YPPROC_ALL (struct ypreq_nokey), returns
 * 	union switch (bool more) {
 *		TRUE:	(struct ypresp_key_val);
 *		FALSE:	(struct) {};
 *	}
 *
 * YPPROC_MASTER (struct ypreq_nokey), returns (ypresp_master)
 *
 * YPPROC_ORDER (struct ypreq_nokey), returns (ypresp_order)
 *
 * YPPROC_MAPLIST (char *), returns (struct ypmaplist *)
 */
#ifndef BOOL_DEFINED
typedef unsigned int bool;
#define BOOL_DEFINED
#endif

extern bool xdr_datum();
extern bool xdr_ypdomain_wrap_string();
extern bool xdr_ypmap_wrap_string();
extern bool xdr_ypreq_key();
extern bool xdr_ypreq_nokey();
extern bool xdr_ypreq_xfr();
extern bool xdr_ypreq_newxfr();
extern bool xdr_ypresp_val();
extern bool xdr_ypresp_key_val();
extern bool xdr_yp_inaddr();
extern bool xdr_ypmap_parms();
extern bool xdr_ypowner_wrap_string();
extern bool xdr_yppushresp_xfr();
extern bool xdr_ypresp_order();
extern bool xdr_ypresp_master();
extern bool xdr_ypall();
extern bool xdr_ypresp_maplist();

/* Program and version symbols, magic numbers */

#define YPPROG		((u_long)100004)
#define YPVERS		((u_long)2)
#define YPVERS_ORIG	((u_long)1)
#define YPMAXRECORD	((u_long)1024)
#define YPMAXDOMAIN	((u_long)256)
#define YPMAXMAP	((u_long)64)
#define YPMAXPEER	((u_long)256)

/* byte size of a large yp packet */
#define YPMSGSZ		1600

#ifndef DATUM
typedef struct {
	char	*dptr;
	int	dsize;
} datum;
#define DATUM
#endif

struct ypmap_parms {
	char *domain;			/* Null string means not available */
	char *map;			/* Null string means not available */
	unsigned long int ordernum;	/* 0 means not available */
	char *owner;			/* Null string means not available */
};

/*
 * Request parameter structures
 */

struct ypreq_key {
	char *domain;
	char *map;
	datum keydat;
};

struct ypreq_nokey {
	char *domain;
	char *map;
};

struct ypreq_xfr {
	struct ypmap_parms map_parms;
	unsigned long transid;
	unsigned long proto;
	unsigned short port;
};

struct ypreq_newxfr {
	struct ypmap_parms map_parms;
	unsigned long transid;
	unsigned long proto;
	char *name;
};

#define ypxfr_domain map_parms.domain
#define ypxfr_map map_parms.map
#define ypxfr_ordernum map_parms.ordernum
#define ypxfr_owner map_parms.owner

/*
 * Response parameter structures
 */

struct ypresp_val {
	long unsigned status;
	datum valdat;
};

struct ypresp_key_val {
	long unsigned status;
	datum keydat;
	datum valdat;
};

struct ypresp_master {
	long unsigned status;
	char *master;
};

struct ypresp_order {
	long unsigned status;
	unsigned long int ordernum;
};

struct ypmaplist {
	char ypml_name[YPMAXMAP + 1];
	struct ypmaplist *ypml_next;
};

struct ypresp_maplist {
	long unsigned status;
	struct ypmaplist *list;
};

/*
 * Procedure symbols.  YPPROC_NULL, YPPROC_DOMAIN, and YPPROC_DOMAIN_NONACK
 * must keep the same values (0, 1, and 2) that they had in the first version
 * of the protocol.
 */

#define YPPROC_NULL	((u_long)0)
#define YPPROC_DOMAIN	((u_long)1)
#define YPPROC_DOMAIN_NONACK ((u_long)2)
#define YPPROC_MATCH	((u_long)3)
#define YPPROC_FIRST	((u_long)4)
#define YPPROC_NEXT	((u_long)5)
#define YPPROC_XFR	((u_long)6)
#define YPPROC_NEWXFR	((u_long)12)
#define YPPROC_CLEAR	((u_long)7)
#define YPPROC_ALL	((u_long)8)
#define YPPROC_MASTER	((u_long)9)
#define YPPROC_ORDER	((u_long)10)
#define YPPROC_MAPLIST	((u_long)11)

/* Return status values */

#define YP_TRUE	 	((long)1)	/* General purpose success code */
#define YP_NOMORE 	((long)2)	/* No more entries in map */
#define YP_FALSE 	((long)0)	/* General purpose failure code */
#define YP_NOMAP 	((long)-1)	/* No such map in domain */
#define YP_NODOM 	((long)-2)	/* Domain not supported */
#define YP_NOKEY 	((long)-3)	/* No such key in map */
#define YP_BADOP 	((long)-4)	/* Invalid operation */
#define YP_BADDB 	((long)-5)	/* Server data base is bad */
#define YP_YPERR 	((long)-6)	/* YP server error */
#define YP_BADARGS 	((long)-7)	/* Request arguments bad */
#define YP_VERS		((long)-8)	/* YP server version mismatch - server
					 *   can't supply requested service. */

enum ypreqtype {YPREQ_KEY = 1, YPREQ_NOKEY = 2, YPREQ_MAP_PARMS = 3};
struct yprequest {
	enum ypreqtype yp_reqtype;
	union {
		struct ypreq_key yp_req_keytype;
		struct ypreq_nokey yp_req_nokeytype;
		struct ypmap_parms yp_req_map_parmstype;
	}yp_reqbody;
};

#define YPMATCH_REQTYPE YPREQ_KEY
#define ypmatch_req_domain yp_reqbody.yp_req_keytype.domain
#define ypmatch_req_map yp_reqbody.yp_req_keytype.map
#define ypmatch_req_keydat yp_reqbody.yp_req_keytype.keydat
#define ypmatch_req_keyptr yp_reqbody.yp_req_keytype.keydat.dptr
#define ypmatch_req_keysize yp_reqbody.yp_req_keytype.keydat.dsize

#define YPFIRST_REQTYPE YPREQ_NOKEY
#define ypfirst_req_domain yp_reqbody.yp_req_nokeytype.domain
#define ypfirst_req_map yp_reqbody.yp_req_nokeytype.map

#define YPNEXT_REQTYPE YPREQ_KEY
#define ypnext_req_domain yp_reqbody.yp_req_keytype.domain
#define ypnext_req_map yp_reqbody.yp_req_keytype.map
#define ypnext_req_keydat yp_reqbody.yp_req_keytype.keydat
#define ypnext_req_keyptr yp_reqbody.yp_req_keytype.keydat.dptr
#define ypnext_req_keysize yp_reqbody.yp_req_keytype.keydat.dsize

#define YPPUSH_REQTYPE YPREQ_NOKEY
#define yppush_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppush_req_map yp_reqbody.yp_req_nokeytype.map

#define YPPULL_REQTYPE YPREQ_NOKEY
#define yppull_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppull_req_map yp_reqbody.yp_req_nokeytype.map

#define YPPOLL_REQTYPE YPREQ_NOKEY
#define yppoll_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppoll_req_map yp_reqbody.yp_req_nokeytype.map

#define YPGET_REQTYPE YPREQ_MAP_PARMS
#define ypget_req_domain yp_reqbody.yp_req_map_parmstype.domain
#define ypget_req_map yp_reqbody.yp_req_map_parmstype.map
#define ypget_req_ordernum yp_reqbody.yp_req_map_parmstype.ordernum
#define ypget_req_owner yp_reqbody.yp_req_map_parmstype.owner

enum ypresptype {YPRESP_VAL = 1, YPRESP_KEY_VAL = 2, YPRESP_MAP_PARMS = 3};
struct ypresponse {
	enum ypresptype yp_resptype;
	union {
		struct ypresp_val yp_resp_valtype;
		struct ypresp_key_val yp_resp_key_valtype;
		struct ypmap_parms yp_resp_map_parmstype;
	} yp_respbody;
};

#define YPMATCH_RESPTYPE YPRESP_VAL
#define ypmatch_resp_status yp_respbody.yp_resp_valtype.status
#define ypmatch_resp_valdat yp_respbody.yp_resp_valtype.valdat
#define ypmatch_resp_valptr yp_respbody.yp_resp_valtype.valdat.dptr
#define ypmatch_resp_valsize yp_respbody.yp_resp_valtype.valdat.dsize

#define YPFIRST_RESPTYPE YPRESP_KEY_VAL
#define ypfirst_resp_status yp_respbody.yp_resp_key_valtype.status
#define ypfirst_resp_keydat yp_respbody.yp_resp_key_valtype.keydat
#define ypfirst_resp_keyptr yp_respbody.yp_resp_key_valtype.keydat.dptr
#define ypfirst_resp_keysize yp_respbody.yp_resp_key_valtype.keydat.dsize
#define ypfirst_resp_valdat yp_respbody.yp_resp_key_valtype.valdat
#define ypfirst_resp_valptr yp_respbody.yp_resp_key_valtype.valdat.dptr
#define ypfirst_resp_valsize yp_respbody.yp_resp_key_valtype.valdat.dsize

#define YPNEXT_RESPTYPE YPRESP_KEY_VAL
#define ypnext_resp_status yp_respbody.yp_resp_key_valtype.status
#define ypnext_resp_keydat yp_respbody.yp_resp_key_valtype.keydat
#define ypnext_resp_keyptr yp_respbody.yp_resp_key_valtype.keydat.dptr
#define ypnext_resp_keysize yp_respbody.yp_resp_key_valtype.keydat.dsize
#define ypnext_resp_valdat yp_respbody.yp_resp_key_valtype.valdat
#define ypnext_resp_valptr yp_respbody.yp_resp_key_valtype.valdat.dptr
#define ypnext_resp_valsize yp_respbody.yp_resp_key_valtype.valdat.dsize

#define YPPOLL_RESPTYPE YPRESP_MAP_PARMS
#define yppoll_resp_domain yp_respbody.yp_resp_map_parmstype.domain
#define yppoll_resp_map yp_respbody.yp_resp_map_parmstype.map
#define yppoll_resp_ordernum yp_respbody.yp_resp_map_parmstype.ordernum
#define yppoll_resp_owner yp_respbody.yp_resp_map_parmstype.owner


extern bool _xdr_yprequest();
extern bool _xdr_ypresponse();
/*
 *		Protocol between clients (ypxfr, only) and yppush
 *		yppush speaks a protocol in the transient range, which
 *		is supplied to ypxfr as a command-line parameter when it
 *		is activated by ypserv.
 */
#define YPPUSHVERS		((u_long) 1)
#define YPPUSHVERS_ORIG		((u_long)1)

/* Procedure symbols */

#define YPPUSHPROC_NULL		((u_long)0)
#define YPPUSHPROC_XFRRESP	((u_long)1)

struct yppushresp_xfr {
	unsigned long transid;
	unsigned long status;
};

/* Status values for yppushresp_xfr.status */

#define YPPUSH_SUCC	((long)1)	/* Success */
#define YPPUSH_AGE	((long)2)	/* Master's version not newer */
#define YPPUSH_NOMAP 	((long)-1)	/* Can't find server for map */
#define YPPUSH_NODOM 	((long)-2)	/* Domain not supported */
#define YPPUSH_RSRC 	((long)-3)	/* Local resouce alloc failure */
#define YPPUSH_RPC 	((long)-4)	/* RPC failure talking to server */
#define YPPUSH_MADDR	((long)-5)	/* Can't get master address */
#define YPPUSH_YPERR 	((long)-6)	/* YP server/map db error */
#define YPPUSH_BADARGS 	((long)-7)	/* Request arguments bad */
#define YPPUSH_DBM	((long)-8)	/* Local dbm operation failed */
#define YPPUSH_FILE	((long)-9)	/* Local file I/O operation failed */
#define YPPUSH_SKEW	((long)-10)	/* Map version skew during transfer */
#define YPPUSH_CLEAR	((long)-11)	/* Can't send "Clear" req to local
					 *   ypserv */
#define YPPUSH_FORCE	((long)-12)	/* No local order number in map -
					 *   use -f flag. */
#define YPPUSH_XFRERR	((long)-13)	/* ypxfr error */
#define YPPUSH_REFUSED	((long)-14)	/* Transfer request refused by ypserv */
#define YPPUSH_NOALIAS	((long)-15)	/* Alias not found for map or domain */
