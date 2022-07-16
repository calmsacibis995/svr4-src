/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ticlts.c	1.3.1.1"

/*
 *	TPI loopback transport provider.
 *	Datagram mode.
 *	Connectionless type.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/debug.h>
#include <sys/signal.h>
#include <sys/tss.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/cred.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/mkdev.h>
#include <sys/ticlts.h>

int ticlts_tracealloc=0;
int ticlts_tracefree=0;

#if defined(__STDC__)

#define KMEM_alloc(a,b) Kmem_alloc(a,b,__LINE__,#a,#b)
#define KMEM_free(a,b)  Kmem_free(a,b,__LINE__,#a,#b)

char *Kmem_alloc(a,b,c,d,e)
unsigned int a;
int b;
char *c,*d,*e;
{
char *r;
r=kmem_alloc(a,b);
if (ticlts_tracealloc)
printf("kmem_alloc(%d,%d)=%x from %d (%s,%s)\n",a,b,r,c,d,e);
return(r);
}

void Kmem_free(a,b,c,d,e)
char * a;
int b;
char *c,*d,*e;
{
if (ticlts_tracefree)
printf("kmem_free(%x,%d) from %d (%s,%s)\n",a,b,c,d,e);
kmem_free(a,b);
}

#else

#define KMEM_alloc(a,b) Kmem_alloc(a,b,__LINE__)
#define KMEM_free(a,b)  Kmem_free(a,b,__LINE__)

char *Kmem_alloc(a,b,c)
unsigned int a;
int b;
char *c;
{
char *r;
r=kmem_alloc(a,b);
if (ticlts_tracealloc)
printf("kmem_alloc(%d,%d)=%x from %d\n",a,b,r,c);
return(r);
}

void Kmem_free(a,b,c)
char * a;
int b;
char *c;
{
if (ticlts_tracefree)
printf("kmem_free(%x,%d) from %d\n",a,b,c);
kmem_free(a,b);
}

#endif


extern char			ti_statetbl[TE_NOEVENTS][TS_NOSTATES];
extern int			nulldev();
int				tclinit();
STATIC int			tcl_bequal(),tcl_bind(),tcl_blink(),tcl_ckopt(),
				tcl_ckstate(),tcl_close(),tcl_cpabuf(),tcl_data(),
				tcl_errack(),tcl_fatal(),tcl_flush(),tcl_ireq(),
				tcl_okack(),tcl_olink(),tcl_open(),tcl_optmgmt(),
				tcl_rsrv(),tcl_sumbytes(),tcl_uderr(),tcl_unbind(),
				tcl_unblink(),tcl_unconnect(),tcl_unolink(),tcl_wput(),
				tcl_wropt(),tcl_wsrv();
STATIC void			tcl_link();
STATIC tcl_endpt_t		*tcl_endptinit(),*tcl_getendpt();
STATIC tcl_addr_t		*tcl_addrinit(),*tcl_getaddr();


STATIC tcl_endpt_t		*tcl_endptopen[TCL_NMHASH];	/* open endpt hash table */
STATIC tcl_endpt_t		tcl_defaultendpt;
STATIC tcl_addr_t		*tcl_addrbnd[TCL_NAHASH];	/* bound addr hash table */
STATIC tcl_addr_t		tcl_defaultaddr;
STATIC char			tcl_defaultabuf[TCL_DEFAULTADDRSZ];
STATIC struct module_info	tcl_info = {TCL_ID,"tcl",TCL_MINPSZ,TCL_MAXPSZ,TCL_HIWAT,TCL_LOWAT};
STATIC struct qinit		tcl_winit = {tcl_wput,tcl_wsrv,tcl_open,tcl_close,nulldev,&tcl_info,NULL};
STATIC struct qinit		tcl_rinit = {NULL,tcl_rsrv,tcl_open,tcl_close,nulldev,&tcl_info,NULL};
struct streamtab		tclinfo = {&tcl_rinit,&tcl_winit,NULL,NULL};


/*
 *	tcl_bequal()
 *
 *	buf equality checker
 */
STATIC int
tcl_bequal(a,b,n)
	register char			*a,*b;
	register int			n;
{
	register int			i;


	ASSERT(a != NULL);
	ASSERT(b != NULL);
	ASSERT(n >= 0);
	/*
	 *	check equality of buffers for n bytes
	 */
	for (i = 0; i < n; i += 1) {
		if (*a++ != *b++) {
			return(0);
		}
	}
	return(!0);
}


/*
 *	tcl_olink()
 *
 *	link endpt to tcl_endptopen[] hash table
 */
STATIC int
tcl_olink(te)
	register tcl_endpt_t		*te;
{
	register tcl_endpt_t		**tep;


	ASSERT(te != NULL);
	/*
	 *	add te to tcl_endptopen[] table
	 */
	tep = &tcl_endptopen[tcl_mhash(te)];
	if (*tep != NULL) {
		(*tep)->te_bolist = te;
	}
	te->te_folist = *tep;
	te->te_bolist = NULL;
	*tep = te;
	return(TCL_PASS);
}


/*
 *	tcl_unolink()
 *
 *	unlink endpt from tcl_endptopen[] hash table
 */
STATIC int
tcl_unolink(te)
	register tcl_endpt_t		*te;
{
	ASSERT(te != NULL);
	/*
	 *	remove te from tcl_endptopen[] table
	 */
	if (te->te_bolist != NULL) {
		te->te_bolist->te_folist = te->te_folist;
	} else {
		tcl_endptopen[tcl_mhash(te)] = te->te_folist;
	}
	if (te->te_folist != NULL) {
		te->te_folist->te_bolist = te->te_bolist;
	}
	/*
	 *	free te
	 */
	(void)KMEM_free(te,sizeof(tcl_endpt_t));
	return(TCL_PASS);
}


/*
 *	tcl_blink()
 *
 *	link endpt to addr, and addr to tcl_addrbnd[] hash table
 */
STATIC int
tcl_blink(te,ta)
	register tcl_endpt_t		*te;
	register tcl_addr_t		*ta;
{
	register tcl_addr_t		**tap;


	ASSERT(te != NULL);
	ASSERT(te->te_addr == NULL);
	ASSERT(ta != NULL);
	ASSERT(ta->ta_blist == NULL);
	/*
	 *	add ta to tcl_addrbnd[] table
	 */
	tap = &tcl_addrbnd[tcl_ahash(ta)];
	if (*tap != NULL) {
		(*tap)->ta_balist = ta;
	}
	ta->ta_falist = *tap;
	ta->ta_balist = NULL;
	*tap = ta;
	/*
	 *	link te and ta together
	 */
	te->te_addr = ta;
	ta->ta_blist = te;
	return(TCL_PASS);
}


/*
 *	tcl_unblink()
 *
 *	unlink endpt from addr, and addr from tcl_addrbnd[] hash table
 */
STATIC int
tcl_unblink(te)
	register tcl_endpt_t		*te;
{
	register tcl_addr_t		*ta;


	ASSERT(te != NULL);
	ta = te->te_addr;
	if (ta != NULL) {
		/*
		 *	unlink te from ta
		 */
		te->te_addr = NULL;
		ASSERT(ta->ta_blist == te);
		ta->ta_blist = NULL;
		/*
		 *	remove ta from tcl_addrbnd[] table
		 */
		if (ta->ta_balist != NULL) {
			ta->ta_balist->ta_falist = ta->ta_falist;
		} else {
			tcl_addrbnd[tcl_ahash(ta)] = ta->ta_falist;
		}
		if (ta->ta_falist != NULL) {
			ta->ta_falist->ta_balist = ta->ta_balist;
		}
		/*
		 *	free ta
		 */
		(void)KMEM_free(tcl_abuf(ta),tcl_alen(ta));
		(void)KMEM_free(ta,sizeof(tcl_addr_t));
	}
	return(TCL_PASS);
}


/*
 *	tcl_sumbytes()
 *
 *	sum bytes of buffer (used for hashing)
 */
STATIC int
tcl_sumbytes(a,n)
	register char			*a;
	register int			n;
{
	register char			*cp,*ep;
	register unsigned		sum;


	ASSERT(a != NULL);
	ASSERT(n > 0);
	sum = 0;
	for (cp = &a[0], ep = &a[n]; cp < ep; cp += 1) {
		sum += (unsigned)*cp;
	}
	return((int)sum);
}


/*
 *	tcl_cpabuf()
 *
 *	copy ta_abuf part of addr, together with ta_len, ta_ahash
 *	(this routine will create a ta_abuf if necessary, but won't resize one)
 */
STATIC int
tcl_cpabuf(to,from)
	tcl_addr_t			*to,*from;
{
	char				*abuf;


	ASSERT(to != NULL);
	ASSERT(from != NULL);
	ASSERT(tcl_alen(from) > 0);
	ASSERT(tcl_abuf(from) != NULL);
	if (tcl_abuf(to) == NULL) {
		ASSERT(tcl_alen(to) == 0);
		abuf = (char *)KMEM_alloc(tcl_alen(from),KM_NOSLEEP);
		if (abuf == NULL) {
			return(TCL_FAIL);
		}
		tcl_alen(to) = tcl_alen(from);
		to->ta_abuf = abuf;
	} else {
		ASSERT(tcl_alen(to) == tcl_alen(from));
	}
	(void)bcopy(tcl_abuf(from),tcl_abuf(to),tcl_alen(to));
	to->ta_ahash = from->ta_ahash;
	return(TCL_PASS);
}


/*
 *	tcl_endptinit()
 *
 *	initialize endpoint
 */
STATIC tcl_endpt_t *
tcl_endptinit(min)
	minor_t				min;
{
	register tcl_endpt_t		*te,*te1,*te2;
	minor_t				otcl_minor;
	int				i;


	/*
	 *	get an endpt
	 */
	te1 = (tcl_endpt_t *)KMEM_alloc(sizeof(tcl_endpt_t),KM_NOSLEEP);
	if (te1 == NULL) {
		u.u_error = ENOMEM;
		return(NULL);
	}
	/*
	 *	initialize data structure
	 */
	te1->te_addr = NULL;
	te1->te_state = TS_UNBND;
	te1->te_flg = 0;
	te1->te_idflg = 0;
	te1->te_rq = NULL;
	te1->te_backwq = NULL;
	te1->te_folist = NULL;
	te1->te_bolist = NULL;
	if (min == NODEV) {
		/*
		 *	no minor number requested; we will assign one
		 */
		te = &tcl_defaultendpt;
		otcl_minor = tcl_min(te);
		for (te2 = tcl_endptopen[tcl_mhash(te)]; te2 != NULL; te2 = te2->te_folist) {
			while (te2 != NULL && tcl_min(te2) == tcl_min(te)) {
				/*
				 *	bump default minor and try again
				 */
				if (++tcl_min(te) == TCL_NENDPT) {
					te->te_min = 0;
				}
				if (tcl_min(te) == otcl_minor) {
					/*
					 *	wrapped around
					 */
					(void)KMEM_free(te1,sizeof(tcl_endpt_t));
					u.u_error = ENOSPC;
					return(NULL);
				}
				te2 = tcl_endptopen[tcl_mhash(te)];
			}
			if (te2 == NULL)
				break;
		}
		te1->te_min = tcl_min(te);
		/*
		 *	bump default minor for next time
		 */
		if (++tcl_min(te) == TCL_NENDPT) {
			te->te_min = 0;
		}
	} else {
		/*
		 *	a minor number was requested; copy it in
		 */
		te1->te_min = min;
	}
	/*
	 *	ident info
	 */
	/* fix the following to use DKI interface ? */
	te1->te_uid = u.u_cred->cr_uid;
	te1->te_gid = u.u_cred->cr_gid;
	te1->te_ruid = u.u_cred->cr_ruid;
	te1->te_rgid = u.u_cred->cr_rgid;
	return(te1);
}


/*
 *	tcl_addrinit()
 *
 *	initialize address
 */
STATIC tcl_addr_t *
tcl_addrinit(ta)
	register tcl_addr_t		*ta;
{
	register tcl_addr_t		*ta1,*ta2;
	int				i;
	char				*cp;


	/*
	 *	get an address
	 */
	ta1 = (tcl_addr_t *)KMEM_alloc(sizeof(tcl_addr_t),KM_NOSLEEP);
	if (ta1 == NULL) {
		return(NULL);
	}
	/*
	 *	initialize data structure
	 */
	ta1->ta_blist = NULL;
	ta1->ta_falist = NULL;
	ta1->ta_balist = NULL;
	ta1->ta_alen = 0;	/* for tcl_cpabuf() */
	ta1->ta_abuf = NULL;
	if (ta == NULL) {
		/*
		 *	no abuf requested; we will assign one
		 */
		ta = &tcl_defaultaddr;
		/* following assertion so we don't have to worry about
		   wrap-around; sizeof(long) is big enough, because
		   num of addresses <= num of endpts <= num of minor numbers */
		ASSERT(TCL_DEFAULTADDRSZ >= sizeof(long));
		for (ta2 = tcl_addrbnd[tcl_ahash(ta)]; ta2 != NULL; ta2 = ta2->ta_falist) {
			if (tcl_eqabuf(ta2,ta)) {
				/*
				 *	bump defaultaddr and try again
				 */
				for (i = 0, cp = tcl_abuf(ta); i < tcl_alen(ta); i += 1, cp += 1) {
					if ((*cp += 1) != '\0') {
						break;
					}
				}
				ta->ta_ahash = tcl_mkahash(ta);
				ta2 = tcl_addrbnd[tcl_ahash(ta)];
				continue;
			}
		}
		if (tcl_cpabuf(ta1,ta) == TCL_FAIL) {
			(void)KMEM_free(ta1,sizeof(tcl_addr_t));
			return(NULL);
		}
		/*
		 *	bump defaultaddr for next time
		 */
		for (i = 0, cp = tcl_abuf(ta); i < tcl_alen(ta); i += 1, cp += 1) {
			if ((*cp += 1) != '\0') {
				break;
			}
		}
		ta->ta_ahash = tcl_mkahash(ta);
	} else {
		/*
		 *	an abuf was requested; copy it in
		 */
		if (tcl_cpabuf(ta1,ta) == TCL_FAIL) {
			(void)KMEM_free(ta1,sizeof(tcl_addr_t));
			return(NULL);
		}
	}
	return(ta1);
}


/*
 *	tcl_getendpt()
 *
 *	search tcl_endptopen[] for endpt
 */
STATIC tcl_endpt_t *
tcl_getendpt(flg,min)
	int				flg;
	minor_t				min;
{
	tcl_endpt_t			endpt,*te;


	switch (flg) {
	    default:
		/* NOTREACHED */
		ASSERT(0);	/* internal error */
	    case TCL_OPEN:
		/*
		 *	open an endpoint
		 */
		if (min == NODEV) {
			/*
			 *	no minor number requested; any free endpt will do
			 */
			return(tcl_endptinit(min));
		} else {
			/*
			 *	find endpt with the requested minor number
			 */
			endpt.te_min = min;
			for (te = tcl_endptopen[tcl_mhash(&endpt)]; te != NULL; te = te->te_folist) {
				if (tcl_min(te) == tcl_min(&endpt)) {
					return(te);
				}
			}
			return(tcl_endptinit(min));
		}
		/* NOTREACHED */
	}
}


/*
 *	tcl_getaddr()
 *
 *	search tcl_addrbnd[] for addr
 */
STATIC tcl_addr_t *
tcl_getaddr(flg,ta)
	int				flg;
	register tcl_addr_t		*ta;
{
	register tcl_addr_t		*ta1;


	switch (flg) {
	    default:
		/* NOTREACHED */
		ASSERT(0);	/* internal error */
	    case TCL_BIND:
		/*
		 *	get an addr that's free to be bound (i.e., not currently bound)
		 */
		if (ta == NULL) {
			/*
			 *	no abuf requested; any free addr will do
			 */
			return(tcl_addrinit(NULL));
		} else {
			/*
			 *	an abuf was requested; get addr with that abuf;
			 *	or any free addr if that one's busy
			 */
			for (ta1 = tcl_addrbnd[tcl_ahash(ta)]; ta1 != NULL; ta1 = ta1->ta_falist) {
				if (tcl_eqabuf(ta1,ta)) {
					ASSERT(ta1->ta_blist != NULL);
					return(tcl_getaddr(TCL_BIND,NULL));	/* recursion! */
				}
			}
			return(tcl_addrinit(ta));
		}
		/* NOTREACHED */
	    case TCL_DEST:
		/*
		 *	get addr that can be talked to (i.e., is currently bound)
		 */
		ASSERT(ta != NULL);
		for (ta1 = tcl_addrbnd[tcl_ahash(ta)]; ta1 != NULL; ta1 = ta1->ta_falist) {
			ASSERT(ta1->ta_blist != NULL);
			if (tcl_eqabuf(ta1,ta)) {
				return(ta1);
			}
		}
		return(NULL);
	}
	/* NOTREACHED */
}


/*
 *	tcl_ckopt()
 *
 *	check validity of opt list
 */
STATIC int
tcl_ckopt(obuf,ebuf)
	char				*obuf,*ebuf;
{
	struct tcl_opt_hdr		*ohdr,*ohdr1;
	union tcl_opt			*opt;
	int				retval = 0;


	/*
	 *	validate format & hdrs & opts of opt list
	 */
	ASSERT(obuf < ebuf);
	for (ohdr = (struct tcl_opt_hdr *)(obuf + 0)
	;
	;    ohdr = ohdr1) {
		if ((int)ohdr%NBPW != 0) {	/* alignment */
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: bad alignment",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
		if ((char *)ohdr + sizeof(struct tcl_opt_hdr) > ebuf) {
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: bad offset",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
		if (ohdr->hdr_thisopt_off < 0) {
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: bad offset",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
		opt = (union tcl_opt *)(obuf + ohdr->hdr_thisopt_off);
		if ((int)opt%NBPW != 0) {	/* alignment */
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: bad alignment",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
		switch (opt->opt_type) {
		    default:
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: unknown opt",__LINE__);
			retval |= TCL_BADTYPE;
			break;
		    case TCL_OPT_NOOP:
			if ((char *)opt + sizeof(struct tcl_opt_noop) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_NOOPOPT;
			break;
		    case TCL_OPT_SETID:
			if ((char *)opt + sizeof(struct tcl_opt_setid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			if ((opt->opt_setid.setid_flg & ~TCL_IDFLG_ALL) != 0) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad opt",__LINE__);
				retval |= TCL_BADVALUE;
				break;
			}
			retval |= TCL_REALOPT;
			break;
		    case TCL_OPT_GETID:
			if ((char *)opt + sizeof(struct tcl_opt_setid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_REALOPT;
			break;
		    case TCL_OPT_UID:
			if ((char *)opt + sizeof(struct tcl_opt_uid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_REALOPT;
			break;
		    case TCL_OPT_GID:
			if ((char *)opt + sizeof(struct tcl_opt_gid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_REALOPT;
			break;
		    case TCL_OPT_RUID:
			if ((char *)opt + sizeof(struct tcl_opt_ruid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_REALOPT;
			break;
		    case TCL_OPT_RGID:
			if ((char *)opt + sizeof(struct tcl_opt_rgid) > ebuf) {
				STRLOG(TCL_ID,-1,4,SL_TRACE,
				    "tcl_ckopt _%d_: bad offset",__LINE__);
				return(retval|TCL_BADFORMAT);
			}
			retval |= TCL_REALOPT;
			break;
		}
		if (ohdr->hdr_nexthdr_off < 0) {
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: bad offset",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
		if (ohdr->hdr_nexthdr_off == TCL_OPT_NOHDR) {
			return(retval);
		}
		ohdr1 = (struct tcl_opt_hdr *)(obuf + ohdr->hdr_nexthdr_off);
		if (ohdr1 <= ohdr) {
			/* potential loop */
			STRLOG(TCL_ID,-1,4,SL_TRACE,
			    "tcl_ckopt _%d_: potential loop",__LINE__);
			return(retval|TCL_BADFORMAT);
		}
	}
	/* NOTREACHED */
}


/*
 *	tcl_wropt()
 *
 *	write opt info into buf
 */
STATIC int
tcl_wropt(idflg,te,obuf)
	long				idflg;
	tcl_endpt_t			*te;
	char				*obuf;
{
	struct tcl_opt_hdr		hdr,*ohdr,*oohdr;
	union tcl_opt			*opt;


	/*
	 *	blindly write the opt info into obuf (assume obuf already set up properly)
	 */
	ASSERT(idflg & TCL_IDFLG_ALL);
	ASSERT(((int)obuf)%NBPW == 0);
	oohdr = &hdr;
	oohdr->hdr_nexthdr_off = 0;
	if (idflg & TCL_IDFLG_UID) {
		ohdr = (struct tcl_opt_hdr *)(obuf + oohdr->hdr_nexthdr_off);
		ohdr->hdr_thisopt_off = oohdr->hdr_nexthdr_off + sizeof(struct tcl_opt_hdr);
		ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
		ohdr->hdr_nexthdr_off = ohdr->hdr_thisopt_off + sizeof(struct tcl_opt_uid);
		ASSERT((ohdr->hdr_nexthdr_off)%NBPW == 0);	/* alignment */
		opt = (union tcl_opt *)(obuf + ohdr->hdr_thisopt_off);
		opt->opt_uid.uid_type = TCL_OPT_UID;
		opt->opt_uid.uid_val = te->te_uid;
		oohdr = ohdr;
	}
	if (idflg & TCL_IDFLG_GID) {
		ohdr = (struct tcl_opt_hdr *)(obuf + oohdr->hdr_nexthdr_off);
		ohdr->hdr_thisopt_off = oohdr->hdr_nexthdr_off + sizeof(struct tcl_opt_hdr);
		ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
		ohdr->hdr_nexthdr_off = ohdr->hdr_thisopt_off + sizeof(struct tcl_opt_gid);
		ASSERT((ohdr->hdr_nexthdr_off)%NBPW == 0);	/* alignment */
		opt = (union tcl_opt *)(obuf + ohdr->hdr_thisopt_off);
		opt->opt_gid.gid_type = TCL_OPT_GID;
		opt->opt_gid.gid_val = te->te_gid;
		oohdr = ohdr;
	}
	if (idflg & TCL_IDFLG_RUID) {
		ohdr = (struct tcl_opt_hdr *)(obuf + oohdr->hdr_nexthdr_off);
		ohdr->hdr_thisopt_off = oohdr->hdr_nexthdr_off + sizeof(struct tcl_opt_hdr);
		ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
		ohdr->hdr_nexthdr_off = ohdr->hdr_thisopt_off + sizeof(struct tcl_opt_ruid);
		ASSERT((ohdr->hdr_nexthdr_off)%NBPW == 0);	/* alignment */
		opt = (union tcl_opt *)(obuf + ohdr->hdr_thisopt_off);
		opt->opt_ruid.ruid_type = TCL_OPT_RUID;
		opt->opt_ruid.ruid_val = te->te_ruid;
		oohdr = ohdr;
	}
	if (idflg & TCL_IDFLG_RGID) {
		ohdr = (struct tcl_opt_hdr *)(obuf + oohdr->hdr_nexthdr_off);
		ohdr->hdr_thisopt_off = oohdr->hdr_nexthdr_off + sizeof(struct tcl_opt_hdr);
		ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
		ohdr->hdr_nexthdr_off = ohdr->hdr_thisopt_off + sizeof(struct tcl_opt_rgid);
		ASSERT((ohdr->hdr_nexthdr_off)%NBPW == 0);	/* alignment */
		opt = (union tcl_opt *)(obuf + ohdr->hdr_thisopt_off);
		opt->opt_rgid.rgid_type = TCL_OPT_RGID;
		opt->opt_rgid.rgid_val = te->te_rgid;
		oohdr = ohdr;
	}
	oohdr->hdr_nexthdr_off = TCL_OPT_NOHDR;
	return(TCL_PASS);
}


/*
 *	tclinit()
 *
 *	driver init routine
 */
int
tclinit()
{
	register tcl_endpt_t		*te;
	register tcl_addr_t		*ta;
	register char			*cp;


	/*
	 *	following sizes are assumed in ticlts.h
	 */
	ASSERT(sizeof(struct T_bind_req) == 16);
	ASSERT(sizeof(struct T_bind_ack) == 16);
	ASSERT(sizeof(struct T_optmgmt_req) == 16);
	ASSERT(sizeof(struct T_optmgmt_ack) == 16);
	ASSERT(sizeof(struct T_unitdata_req) == 20);
	ASSERT(sizeof(struct T_uderror_ind) == 24);
	/*
	 *	initialize default minor and addr
	 */
	tcl_defaultendpt.te_min = 0;
	tcl_defaultaddr.ta_alen = TCL_DEFAULTADDRSZ;
	tcl_defaultaddr.ta_abuf = tcl_defaultabuf;
	tcl_defaultaddr.ta_ahash = tcl_mkahash(&tcl_defaultaddr);
	return(UNIX_PASS);
}


/*
 *	tcl_open()
 *
 *	driver open routine
 */
STATIC int
tcl_open(q,dev,oflg,sflg)
	register queue_t		*q;
	int				dev,oflg,sflg;
{
	register tcl_endpt_t		*te;
	minor_t				min;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	/*
	 *	is it already open?
	 */
	if (te != NULL) {
		STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
		    "tcl_open _%d_: re-open",__LINE__);
		return(tcl_min(te));
	}
	/*
	 *	get endpt with requested minor number
	 */
	if (sflg == CLONEOPEN) {
		min = NODEV;
	} else {
		min = minor(dev);
		ASSERT(min >= 0);
		if (min >= TCL_NENDPT) {
			u.u_error = ENODEV;
			return(OPENFAIL);
		}
	}
	te = tcl_getendpt(TCL_OPEN,min);
	if (te == NULL) {
		STRLOG(TCL_ID,-1,3,SL_TRACE,
		    "tcl_open _%d_: cannot allocate endpoint, q=%x",__LINE__,q);
		return(OPENFAIL);
	}
	/*
	 *	assign te to queue private pointers
	 */
	te->te_rq = q;
	q->q_ptr = (caddr_t)te;
	WR(q)->q_ptr = (caddr_t)te;
	/*
	 *	link to tcl_endptopen[] table
	 */
	(void)tcl_olink(te);
	STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
	    "tcl_open _%d_: endpoint allocated",__LINE__);
	return(tcl_min(te));
}


/*
 *	tcl_close()
 *
 *	driver close routine
 */
STATIC int
tcl_close(q)
	register queue_t		*q;
{
	register tcl_endpt_t		*te;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	/*
	 * If the queue was looped then unloop it.
	 */
	if ((WR(q))->q_next != NULL) {
		(WR ((WR(q)->q_next)))->q_next = NULL;
		(WR(q))->q_next = NULL;
	}	

	(void)tcl_unconnect(te);
	(void)tcl_unblink(te);
	STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
	    "tcl_close _%d_: endpoint deallocated",__LINE__);
	(void)tcl_unolink(te);
	return(UNIX_PASS);
}


/*
 *	tcl_wput()
 *
 *	driver write side put procedure
 */
STATIC int
tcl_wput(q,mp)
	register queue_t		*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim;
	int				spl,msz;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	/* 
	 *	switch on streams msg type
	 */
	msz = mp->b_wptr - mp->b_rptr;
	switch (mp->b_datap->db_type) {
	    default:
		STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
		    "tcl_wput _%d_: got illegal msg, M_type=%d",__LINE__,mp->b_datap->db_type);
		(void)freemsg(mp);
		return(UNIX_FAIL);

		/* 
		 * Link and unlink the endpoint.
		 */
	case M_CTL:
		switch(*(long *)mp->b_rptr) {
		default:
			break;

		case TCL_LINK:
			tcl_link(q, mp);
			break;

		case TCL_UNLINK:
			if (q->q_next)
				q->q_next = NULL;
			break;

		}
		freemsg(mp);
		return(UNIX_PASS);

	    case M_IOCTL:
		/* no ioctl's supported */
		STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
		    "tcl_wput _%d_: got M_IOCTL msg",__LINE__);
		mp->b_datap->db_type = M_IOCNAK;
		(void)qreply(q,mp);
		return(UNIX_PASS);
	    case M_FLUSH:
		STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
		    "tcl_wput _%d_: got M_FLUSH msg",__LINE__);
		if (*mp->b_rptr & FLUSHW) {
			(void)flushq(q,0);
		}
		if (!(*mp->b_rptr & FLUSHR)) {
			(void)freemsg(mp);
		} else {
			*mp->b_rptr &= ~FLUSHW;
			(void)flushq(OTHERQ(q),0);
			(void)qreply(q,mp);
		}
		return(UNIX_PASS);
	    case M_DATA:
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_wput _%d_ fatal: got M_DATA msg",__LINE__);
		(void)tcl_fatal(q,mp);
		return(UNIX_PASS);
	    case M_PCPROTO:
		/*
		 *	switch on tpi msg type
		 */
		if (msz < sizeof(prim->type)) {
			STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
			    "tcl_wput _%d_ fatal: bad msg ctl",__LINE__);
			(void)tcl_fatal(q,mp);
			return(UNIX_FAIL);
		}
		ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
		prim = (union T_primitives *)mp->b_rptr;
		ASSERT(prim != NULL);
		/* can the splstr()'s be minimized ? */
		spl = splstr();
		switch (prim->type) {
		    default:
			STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
			    "tcl_wput _%d_ fatal: bad prim type=%d",__LINE__,prim->type);
			(void)tcl_fatal(q,mp);
			(void)splx(spl);
			return(UNIX_FAIL);
		    case T_INFO_REQ:
			STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
			    "tcl_wput _%d_: got T_INFO_REQ msg",__LINE__);
			(void)tcl_ireq(q,mp);
			(void)splx(spl);
			return(UNIX_PASS);
		}
		/* NOTREACHED */
	    case M_PROTO:
		/*
		 *	switch on tpi msg type
		 */
		if (msz < sizeof(prim->type)) {
			STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
			    "tcl_wput _%d_ fatal: bad control",__LINE__);
			(void)tcl_fatal(q,mp);
			return(UNIX_FAIL);
		}
		ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
		prim = (union T_primitives *)mp->b_rptr;
		ASSERT(prim != NULL);
		/* can the splstr()'s be minimized ? */
		spl = splstr();
		switch (prim->type) {
		    default:
			STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
			    "tcl_wput _%d_ fatal: bad prim type=%d",__LINE__,prim->type);
			(void)tcl_fatal(q,mp);
			(void)splx(spl);
			return(UNIX_FAIL);
#ifdef DEBUG
#define JUMP	goto jump
#else
#define JUMP	/* nothing */
#endif
		    case T_BIND_REQ:
			STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
			    "tcl_wput _%d_: got T_BIND_REQ msg",__LINE__);
			JUMP;
		    case T_UNBIND_REQ:
			STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
			    "tcl_wput _%d_: got T_UNBIND_REQ msg",__LINE__);
			JUMP;
		    case T_OPTMGMT_REQ:
			STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
			    "tcl_wput _%d_: got T_OPTMGMT_REQ msg",__LINE__);
			JUMP;
#ifdef DEBUG
    jump:
#endif
#undef JUMP
			/*
			 *	if endpt is hosed, do nothing
			 */
			if (te->te_flg & TCL_ZOMBIE) {
				STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
				    "tcl_wput _%d_: ZOMBIE",__LINE__);
				(void)freemsg(mp);
				(void)splx(spl);
				return(UNIX_FAIL);
			}
			(void)tcl_ckstate(q,mp);
			(void)splx(spl);
			return(UNIX_PASS);
		    case T_UNITDATA_REQ:
			/*
			 *	if endpt is hosed, do nothing
			 */
			STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
			    "tcl_wput _%d_: got T_UNITDATA_REQ msg",__LINE__);
			if (te->te_flg & TCL_ZOMBIE) {
				STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
				    "tcl_wput _%d_: ZOMBIE",__LINE__);
				(void)freemsg(mp);
				(void)splx(spl);
				return(UNIX_FAIL);
			}
			if (NEXTSTATE(TE_UNITDATA_REQ,te->te_state) == NR) {
				STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
				    "tcl_wput _%d_ fatal: TE_UNITDTA_REQ out of state, current state=%d->127",__LINE__,te->te_state);
				(void)tcl_fatal(q,mp);
				(void)splx(spl);
				return(UNIX_FAIL);
			}
			(void)putq(q,mp);
			(void)splx(spl);
			return(UNIX_PASS);
		}
		/* NOTREACHED */
	}
	/* NOTREACHED */
}


/*
 *	tcl_wsrv()
 *
 *	driver write side service routine
 */
STATIC int
tcl_wsrv(q)
	register queue_t		*q;
{
	tcl_endpt_t			*te;
	register mblk_t			*mp;
	register union T_primitives	*prim;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	/*
	 *	loop through msgs on queue
	 */
	while ((mp = getq(q)) != NULL) {
		/*
		 *	switch on streams msg type
		 */
		switch (mp->b_datap->db_type) {
		    default:
			/* NOTREACHED */
			ASSERT(0);	/* internal error */
		    case M_PROTO:
			/*
			 *	switch on tpi msg type
			 */
			ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
			prim = (union T_primitives *)mp->b_rptr;
			switch (prim->type) {
			    default:
				/* NOTREACHED */
				ASSERT(0);	/* internal error */
			    case T_UNITDATA_REQ:
				STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
				    "tcl_wsrv _%d_: got T_UNITADTA_REQ msg",__LINE__);
				if (tcl_data(q,mp) != 0) {
					STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
					    "tcl_wsrv _%d_: tcl_data() failure",__LINE__);
					return(0);	/* or just break ?? */
				}
				break;
			}
			break;
		}
	}
	return(UNIX_PASS);
}


/*
 *	tcl_rsrv()
 *
 *	driver read side service routine
 */
STATIC int
tcl_rsrv(q)
	register queue_t		*q;
{
	register tcl_endpt_t		*te;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	/*
	 *	enable queue for data transfer
	 */
	ASSERT(te != NULL);
	if (te->te_state == TS_IDLE) {
		ASSERT(te->te_backwq != NULL);
		(void)qenable(te->te_backwq);
	}
	return(UNIX_PASS);
}


/*
 *	tcl_okack()
 *
 *	handle ok ack
 */
STATIC int
tcl_okack(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register union T_primitives	*prim;
	mblk_t				*mp1;
	long				type;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	/*
	 *	prepare ack msg
	 */
	type = prim->type;
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(struct T_ok_ack)) {
		ASSERT(sizeof(struct T_ok_ack) <= TCL_TIDUSZ);
		if ((mp1 = allocb(sizeof(struct T_ok_ack),BPRI_HI)) == NULL) {
			STRLOG(TCL_ID,-1,1,SL_TRACE,
			    "tcl_okack _%d_ fatal: allocb() failure",__LINE__);
			(void)tcl_fatal(q,mp);
			return(TCL_FAIL);
		}
		(void)freemsg(mp);
		mp = mp1;
	}
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ok_ack);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	prim->ok_ack.PRIM_type = T_OK_ACK;
	prim->ok_ack.CORRECT_prim = type;
	/*
	 *	send ack msg
	 */
	(void)freemsg(unlinkb(mp));
	if (q->q_flag & QREADR) {
		(void)putnext(q,mp);
	} else {
		(void)qreply(q,mp);
	}
	return(TCL_PASS);
}


/*
 *	tcl_errack()
 *
 *	handle error ack
 */
STATIC int
tcl_errack(q,mp,tli_err,unix_err)
	queue_t				*q;
	register mblk_t			*mp;
	long				tli_err,unix_err;
{
	tcl_endpt_t			*te;
	register union T_primitives	*prim;
	mblk_t				*mp1;
	long				type;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT(mp != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	/*
	 *	prepare nack msg
	 */
	ASSERT(prim != NULL);
	type = prim->type;
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(struct T_error_ack)) {
		ASSERT(sizeof(struct T_error_ack) <= TCL_TIDUSZ);
		if ((mp1 = allocb(sizeof(struct T_error_ack),BPRI_HI)) == NULL) {
			STRLOG(TCL_ID,-1,1,SL_TRACE,
			    "tcl_errack _%d_ fatal: couldn't allocate msg",__LINE__);
			(void)tcl_fatal(q,mp);
			return(TCL_FAIL);
		}
		(void)freemsg(mp);
		mp = mp1;
	}
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_error_ack);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	prim->error_ack.PRIM_type = T_ERROR_ACK;
	prim->error_ack.ERROR_prim = type;
	prim->error_ack.TLI_error = tli_err;
	prim->error_ack.UNIX_error = unix_err;
	/*
	 *	send nack msg
	 */
	(void)freemsg(unlinkb(mp));
	if (q->q_flag & QREADR) {
		(void)putnext(q,mp);
	} else {
		(void)qreply(q,mp);
	}
	return(TCL_PASS);
}


/*
 *	tcl_fatal()
 *
 *	handle fatal condition (endpt is hosed)
 */
STATIC int
tcl_fatal(q,mp)
	register queue_t		*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;

printf("FATAL tcl_fatal called %x %x\n",q,mp);
	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	/*
	 *	prepare err msg
	 */
	(void)tcl_unconnect(te);
	te->te_flg |= TCL_ZOMBIE;
	mp->b_datap->db_type = M_ERROR;
	ASSERT(mp->b_datap->db_lim - mp->b_datap->db_base >= sizeof(char));
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(char);
	*mp->b_rptr = EPROTO;
	/*
	 *	send err msg
	 */
	(void)freemsg(unlinkb(mp));
	if (q->q_flag & QREADR) {
		(void)putnext(q,mp);
	} else {
		(void)qreply(q,mp);
	}
	return(TCL_PASS);
}


/*
 *	tcl_flush()
 *
 *	flush rd & wr queues
 */
STATIC int
tcl_flush(q)
	queue_t				*q;
{
	mblk_t				*mp;


	ASSERT(q != NULL);
	/*
	 *	prepare flush msg
	 */
	ASSERT(sizeof(char) <= TCL_TIDUSZ);
	if ((mp = allocb(sizeof(char),BPRI_HI)) == NULL) {
		return(TCL_FAIL);
	}
	mp->b_datap->db_type = M_FLUSH;
	mp->b_wptr = mp->b_rptr + sizeof(char);
	*mp->b_rptr = FLUSHRW;
	/*
	 *	send flush msg
	 */
	if (q->q_flag & QREADR) {
		(void)putnext(q,mp);
	} else {
		(void)qreply(q,mp);
	}
	return(TCL_PASS);
}


/*
 *	tcl_ckstate()
 *
 *	check interface state and handle event
 */
STATIC int
tcl_ckstate(q,mp)
	register queue_t		*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim;
	register char			ns;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	/*
	 *	switch on tpi msg type
	 */
	switch (prim->type) {
	    default:
		/* NOTREACHED */
		ASSERT(0);	/* internal error */
	    case T_BIND_REQ:
		if ((ns = NEXTSTATE(TE_BIND_REQ,te->te_state)) == NR) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_ckstate _%d_ errack: tli_err=TOUTSTATE, unix_err=0, state=%d->127",__LINE__,te->te_state);
			/* te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);	-- no */
			(void)tcl_errack(q,mp,TOUTSTATE,0);
			return(TCL_FAIL);
		}
		te->te_state = ns;
		(void)tcl_bind(q,mp);
		return(TCL_PASS);
	    case T_UNBIND_REQ:
		if ((ns = NEXTSTATE(TE_UNBIND_REQ,te->te_state)) == NR) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_ckstate _%d_ errack: tli_err=TOUTSTATE, unix_err=0, state=%d->127",__LINE__,te->te_state);
			/* te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);	-- no */
			(void)tcl_errack(q,mp,TOUTSTATE,0);
			return(TCL_FAIL);
		}
		te->te_state = ns;
		(void)tcl_unbind(q,mp);
		return(TCL_PASS);
	    case T_OPTMGMT_REQ:
		if ((ns = NEXTSTATE(TE_OPTMGMT_REQ,te->te_state)) == NR) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_ckstate _%d_ errack: tli_err=TOUTSTATE, unix_err=0, state=%d->127",__LINE__,te->te_state);
			/* te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);	-- no */
			(void)tcl_errack(q,mp,TOUTSTATE,0);
			return(TCL_FAIL);
		}
		te->te_state = ns;
		(void)tcl_optmgmt(q,mp);
		return(TCL_PASS);
	}
	/* NOTREACHED */
}


/*
 *	tcl_ireq()
 *
 *	handle info request
 */
STATIC int
tcl_ireq(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim,*prim1;
	mblk_t				*mp1;


	ASSERT(q != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_INFO_REQ);
	/*
	 *	prepare ack msg
	 */
	ASSERT(sizeof(struct T_info_ack) <= TCL_TIDUSZ);
	if ((mp1 = allocb(sizeof(struct T_info_ack),BPRI_HI)) == NULL) {
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_ireq _%d_ fatal: allocb() failure",__LINE__);
		(void)tcl_fatal(q,mp);
		return(TCL_FAIL);
	}
	mp1->b_datap->db_type = M_PCPROTO;
	mp1->b_wptr = mp1->b_rptr + sizeof(struct T_info_ack);
	ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
	prim1 = (union T_primitives *)mp1->b_rptr;
	prim1->info_ack.PRIM_type = T_INFO_ACK;
	prim1->info_ack.SERV_type = TCL_SERVTYPE;
	prim1->info_ack.ADDR_size = TCL_ADDRSZ;
	prim1->info_ack.OPT_size = TCL_OPTSZ;
	prim1->info_ack.TIDU_size = TCL_TIDUSZ;
	prim1->info_ack.TSDU_size = TCL_TSDUSZ;
	prim1->info_ack.ETSDU_size = TCL_ETSDUSZ;
	prim1->info_ack.CDATA_size = TCL_CDATASZ;
	prim1->info_ack.DDATA_size = TCL_DDATASZ;
	prim1->info_ack.CURRENT_state = te->te_state;
	(void)freemsg(mp);
	/*
	 *	send ack msg 
	 */
	(void)qreply(q,mp1);
	return(TCL_PASS);
}


/*
 *	tcl_bind()
 *
 *	handle bind request
 */
STATIC int
tcl_bind(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim,*prim2;
	tcl_addr_t			addr,*ta;
	mblk_t				*mp1,*mp2;
	struct stroptions		*so;
	int				alen,aoff,msz,msz2;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_BIND_REQ);
	ASSERT(te->te_addr == NULL);
	/*
	 *	set stream head options
	 */
	ASSERT(sizeof(struct stroptions) <= TCL_TIDUSZ);
	if ((mp1 = allocb(sizeof(struct stroptions),BPRI_HI)) == NULL) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_bind _%d_ errack: tli_err=TSYSERR, unix_err=ENOMEM",__LINE__);
		(void)tcl_errack(q,mp,TSYSERR,ENOMEM);
		return(TCL_FAIL);
	}
	mp1->b_datap->db_type = M_SETOPTS;
	mp1->b_wptr = mp1->b_rptr + sizeof(struct stroptions);
	ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
	so = (struct stroptions *)mp1->b_rptr;
	so->so_flags = SO_MINPSZ|SO_MAXPSZ|SO_HIWAT|SO_LOWAT;
	so->so_readopt = 0;
	so->so_wroff = 0;
	so->so_minpsz = TCL_MINPSZ;
	so->so_maxpsz = TCL_MAXPSZ;
	so->so_lowat = TCL_LOWAT;
	so->so_hiwat = TCL_HIWAT;
	(void)qreply(q,mp1);
	/*
	 *	validate the request
	 */
	msz = mp->b_wptr - mp->b_rptr;
	if (msz < sizeof(struct T_bind_req)) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_bind _%d_ errack: tli_err=TSYSERR, unix_err=EINVAL",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TSYSERR,EINVAL);
		return(TCL_FAIL);
	}
	alen = prim->bind_req.ADDR_length;
	aoff = prim->bind_req.ADDR_offset;
	if ((alen < 0) || (alen > TCL_ADDRSZ)) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_bind _%d_ errack: tli_err=TBADADDR, unix_err=0",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TBADADDR,0);
		return(TCL_FAIL);
	}
	if (alen > 0) {
		if ((aoff < 0) || ((aoff + alen) > msz)) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_bind _%d_ errack: tli_err=TSYSERR, unix_err=EINVAL",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TSYSERR,EINVAL);
			return(TCL_FAIL);
		}
	}
	/*
	 *	negotiate addr
	 */
	if (alen == 0) {
		ta = tcl_getaddr(TCL_BIND,NULL);
	} else {
		addr.ta_alen = alen;
		addr.ta_abuf = (char *)(mp->b_rptr + aoff);
		addr.ta_ahash = tcl_mkahash(&addr);
		ta = tcl_getaddr(TCL_BIND,&addr);
	}
	if (ta == NULL) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_bind _%d_ errack: tli_err=TSYSERR, unix_err=ENOMEM",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TSYSERR,ENOMEM);
		return(TCL_FAIL);
	}
	ASSERT(tcl_abuf(ta) != NULL);
	ASSERT(tcl_alen(ta) != 0);	/* may be != alen */
	/*
	 *	prepare ack msg
	 */
	msz2 = sizeof(struct T_bind_ack) + tcl_alen(ta);
	ASSERT(sizeof(struct T_bind_req) == sizeof(struct T_bind_ack));
	ASSERT(msz2 <= TCL_TIDUSZ);
	if ((mp2 = allocb(msz2,BPRI_HI)) == NULL) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_bind _%d_ errack: tli_err=TSYSERR, unix_err=ENOMEM",__LINE__);
		(void)KMEM_free(tcl_abuf(ta),tcl_alen(ta));
		(void)KMEM_free(ta,sizeof(tcl_addr_t));
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TSYSERR,ENOMEM);
		return(TCL_FAIL);
	}
	mp2->b_datap->db_type = M_PCPROTO;
	mp2->b_wptr = mp2->b_rptr + msz2;
	ASSERT((int)(mp2->b_rptr)%NBPW == 0);	/* alignment */
	prim2 = (union T_primitives *)mp2->b_rptr;
	prim2->bind_ack.PRIM_type = T_BIND_ACK;
	prim2->bind_ack.CONIND_number = 0;	/* connectionless */
	prim2->bind_ack.ADDR_offset = sizeof(struct T_bind_ack);
	prim2->bind_ack.ADDR_length = tcl_alen(ta);
	addr.ta_alen = tcl_alen(ta);
	addr.ta_abuf = (char *)(mp2->b_rptr + prim2->bind_ack.ADDR_offset);
	/* addr.ta_ahash = tcl_mkahash(&addr);	-- not needed */
	(void)tcl_cpabuf(&addr,ta);	/* cannot fail */
	(void)freemsg(mp);
	/*
	 *	do the bind
	 */
	(void)tcl_blink(te,ta);
	te->te_state = NEXTSTATE(TE_BIND_ACK,te->te_state);
	ASSERT(te->te_state != NR);
	/*
	 *	send ack msg
	 */
	(void)qreply(q,mp2);
	STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
	    "tcl_bind _%d_: bound",__LINE__);
	return(TCL_PASS);
}


/*
 *	tcl_unbind()
 *
 *	handle unbind request
 */
STATIC int
tcl_unbind(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_UNBIND_REQ);
	/*
	 *	flush queues
	 */
	if (tcl_flush(q) == TCL_FAIL) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_unbind _%d_ errack: tli_err=TSYSERR, unix_err=EIO",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TSYSERR,EIO);
		return(TCL_FAIL);
	}
	/*
	 *	do the unbind
	 */
	(void)tcl_unblink(te);
	te->te_state = NEXTSTATE(TE_OK_ACK1,te->te_state);
	ASSERT(te->te_state != NR);
	/*
	 *	send ack msg
	 */
	(void)tcl_okack(q,mp);
	STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
	    "tcl_unbind _%d_: unbound",__LINE__);
	return(TCL_PASS);
}


/*
 *	tcl_optmgmt()
 *
 *	handle option mgmt request
 */
STATIC int
tcl_optmgmt(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim,*prim1;
	mblk_t				*mp1;
	int				olen,ooff,msz,msz1,ckopt;
	struct tcl_opt_hdr		*ohdr;
	union tcl_opt			*opt;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_OPTMGMT_REQ);
	/*
	 *	validate the request
	 */
	msz = mp->b_wptr - mp->b_rptr;
	if (msz < (sizeof(struct T_optmgmt_req))) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_optmgmt _%d_ errack: tli_err=TSYSERR, unix_err=EINVAL",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TSYSERR,EINVAL);
		return(TCL_FAIL);
	}
	olen = prim->optmgmt_req.OPT_length;
	ooff = prim->optmgmt_req.OPT_offset;
	/* olen, ooff are validated below */
	/*
	 *	switch on optmgmt request type
	 */
	switch (prim->optmgmt_req.MGMT_flags) {
	    default:
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_optmgmt _%d_ errack: tli_err=TBADFLAG, unix_err=0",__LINE__);
		te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)tcl_errack(q,mp,TBADFLAG,0);
		return(TCL_FAIL);
	    case T_CHECK:
		/*
		 *	validate opt list
		 */
		if ((olen < 0) || (olen > TCL_OPTSZ)
		||  (ooff < 0) || (ooff + olen > msz)) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_optmgmt _%d_ errack: tli_err=TSYSERR, unix_err=EINVAL",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TSYSERR,EINVAL);
			return(TCL_FAIL);
		}
		ckopt = tcl_ckopt(mp->b_rptr+ooff,mp->b_rptr+ooff+olen);
		if (ckopt & TCL_BADFORMAT) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_optmgmt _%d_ errack: tli_err=TBADOPT, unix_err=0",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TBADOPT,0);
			return(TCL_FAIL);
		}
		/* re-use msg block: */
		ASSERT(sizeof(struct T_optmgmt_req) == sizeof(struct T_optmgmt_ack));
		ASSERT((int)&prim->optmgmt_req.PRIM_type - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.PRIM_type - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.OPT_length - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.OPT_length - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.OPT_offset - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.OPT_offset - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.MGMT_flags - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.MGMT_flags - (int)&prim->optmgmt_ack);
		mp->b_datap->db_type = M_PCPROTO;
		prim1 = prim;
		prim1->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		if (ckopt & (TCL_REALOPT|TCL_NOOPOPT)) {
			prim1->optmgmt_ack.MGMT_flags |= T_SUCCESS;
		}
		if (ckopt & (TCL_BADTYPE|TCL_BADVALUE)) {
			prim1->optmgmt_ack.MGMT_flags |= T_FAILURE;
		}
		te->te_state = NEXTSTATE(TE_OPTMGMT_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)qreply(q,mp);
		return(TCL_PASS);
	    case T_DEFAULT:
		/*
		 *	retrieve default opt
		 */
		msz1 = sizeof(struct T_optmgmt_ack) + sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_noop);
		ASSERT(msz1 <= TCL_TIDUSZ);
		if ((mp1 = allocb(msz1,BPRI_HI)) == NULL) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_optmgmt _%d_ errack: tli_err=TSYSERR, unix_err=ENOMEM",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TSYSERR,ENOMEM);
			return(TCL_FAIL);
		}
		mp1->b_datap->db_type = M_PCPROTO;
		mp1->b_wptr = mp1->b_rptr + msz1;
		ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
		prim1 = (union T_primitives *)mp1->b_rptr;
		prim1->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		prim1->optmgmt_ack.MGMT_flags = T_DEFAULT;
		prim1->optmgmt_ack.OPT_length = sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_noop);
		prim1->optmgmt_ack.OPT_offset = sizeof(struct T_optmgmt_ack);
		ASSERT((prim1->optmgmt_ack.OPT_offset)%NBPW == 0);	/* alignment */
		ohdr = (struct tcl_opt_hdr *)(mp1->b_rptr + prim1->optmgmt_ack.OPT_offset);
		ohdr->hdr_thisopt_off = sizeof(struct tcl_opt_hdr);
		ohdr->hdr_nexthdr_off = TCL_OPT_NOHDR;
		ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
		opt = (union tcl_opt *)(mp1->b_rptr + prim1->optmgmt_ack.OPT_offset + ohdr->hdr_thisopt_off);
		opt->opt_type = TCL_OPT_NOOP;	/* default opt */
		te->te_state = NEXTSTATE(TE_OPTMGMT_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)freemsg(mp);
		(void)qreply(q,mp1);
		return(TCL_PASS);
	    case T_NEGOTIATE:
		/*
		 *	negotiate opt
		 */
		if (olen == 0) {
			/*
			 *	retrieve default opt
			 */
			msz1 = sizeof(struct T_optmgmt_ack) + sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_noop);
			ASSERT(msz1 <= TCL_TIDUSZ);
			if ((mp1 = allocb(msz1,BPRI_HI)) == NULL) {
				STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
				    "tcl_optmgmt _%d_ errack: tli_err=TSYSERR, unix_err=ENOMEM",__LINE__);
				te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
				ASSERT(te->te_state != NR);
				(void)tcl_errack(q,mp,TSYSERR,ENOMEM);
				return(TCL_FAIL);
			}
			mp1->b_datap->db_type = M_PCPROTO;
			mp1->b_wptr = mp1->b_rptr + msz1;
			ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
			prim1 = (union T_primitives *)mp1->b_rptr;
			prim1->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
			prim1->optmgmt_ack.MGMT_flags = T_NEGOTIATE;
			prim1->optmgmt_ack.OPT_length = sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_noop);
			prim1->optmgmt_ack.OPT_offset = sizeof(struct T_optmgmt_ack);
			ASSERT((prim1->optmgmt_ack.OPT_offset)%NBPW == 0);	/* alignment */
			ohdr = (struct tcl_opt_hdr *)(mp1->b_rptr + prim1->optmgmt_ack.OPT_offset);
			ohdr->hdr_thisopt_off = sizeof(struct tcl_opt_hdr);
			ohdr->hdr_nexthdr_off = TCL_OPT_NOHDR;
			ASSERT((ohdr->hdr_thisopt_off)%NBPW == 0);	/* alignment */
			opt = (union tcl_opt *)(mp1->b_rptr + prim1->optmgmt_ack.OPT_offset + ohdr->hdr_thisopt_off);
			opt->opt_type = TCL_OPT_NOOP;	/* default opt */
			te->te_state = NEXTSTATE(TE_OPTMGMT_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)freemsg(mp);
			(void)qreply(q,mp1);
			return(TCL_PASS);
		}
		/*
		 *	validate opt list
		 */
		if ((olen < 0) || (olen > TCL_OPTSZ)
		||  (ooff < 0) || (ooff + olen > msz)) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_optmgmt _%d_ errack: tli_err=TSYSERR, unix_err=EINVAL",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TSYSERR,EINVAL);
			return(TCL_FAIL);
		}
		ckopt = tcl_ckopt(mp->b_rptr+ooff,mp->b_rptr+ooff+olen);
		if (ckopt & (TCL_BADFORMAT|TCL_BADTYPE|TCL_BADVALUE)) {
			STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
			    "tcl_optmgmt _%d_ errack: tli_err=TBADOPT, unix_err=0",__LINE__);
			te->te_state = NEXTSTATE(TE_ERROR_ACK,te->te_state);
			ASSERT(te->te_state != NR);
			(void)tcl_errack(q,mp,TBADOPT,0);
			return(TCL_FAIL);
		}
		/* re-use msg block: */
		ASSERT(sizeof(struct T_optmgmt_req) == sizeof(struct T_optmgmt_ack));
		ASSERT((int)&prim->optmgmt_req.PRIM_type - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.PRIM_type - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.OPT_length - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.OPT_length - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.OPT_offset - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.OPT_offset - (int)&prim->optmgmt_ack);
		ASSERT((int)&prim->optmgmt_req.MGMT_flags - (int)&prim->optmgmt_req
		    == (int)&prim->optmgmt_ack.MGMT_flags - (int)&prim->optmgmt_ack);
		mp->b_datap->db_type = M_PCPROTO;
		prim1 = prim;
		prim1->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		/*
		 *	do the opts
		 */
		for (ohdr = (struct tcl_opt_hdr *)(mp->b_rptr + ooff + 0)
		;
		;    ohdr = (struct tcl_opt_hdr *)(mp->b_rptr + ooff + ohdr->hdr_nexthdr_off)) {
			opt = (union tcl_opt *)(mp->b_rptr + ooff + ohdr->hdr_thisopt_off);
			switch (opt->opt_type) {
			    default:
				/* NOTREACHED */
				ASSERT(0);
			    case TCL_OPT_NOOP:
				break;
			    case TCL_OPT_SETID:
				ASSERT((opt->opt_setid.setid_flg & ~TCL_IDFLG_ALL) == 0);
				te->te_idflg = opt->opt_setid.setid_flg;
				break;
			    case TCL_OPT_GETID:
				opt->opt_getid.getid_flg = te->te_idflg;
				break;
			    case TCL_OPT_UID:
				opt->opt_uid.uid_val = te->te_uid;
				break;
			    case TCL_OPT_GID:
				opt->opt_gid.gid_val = te->te_gid;
				break;
			    case TCL_OPT_RUID:
				opt->opt_ruid.ruid_val = te->te_ruid;
				break;
			    case TCL_OPT_RGID:
				opt->opt_rgid.rgid_val = te->te_rgid;
				break;
			}
			if (ohdr->hdr_nexthdr_off == TCL_OPT_NOHDR) {
				break;
			}
		}
		te->te_state = NEXTSTATE(TE_OPTMGMT_ACK,te->te_state);
		ASSERT(te->te_state != NR);
		(void)qreply(q,mp);
		return(TCL_PASS);
	}
	/* NOTREACHED */
}


/*
 *	tcl_unconnect()
 *
 *	cleanup utility routine
 */
STATIC int
tcl_unconnect(te)
	register tcl_endpt_t		*te;
{
	register queue_t		*q;


	ASSERT(te != NULL);
	q = te->te_rq;
	ASSERT(q != NULL);
	(void)flushq(q,1);
	te->te_state = NR;
	return(TCL_PASS);
}


/*
 *	tcl_uderr()
 *
 *	handle unitdata error
 */
STATIC int
tcl_uderr(q,mp,err)
	register queue_t		*q;
	register mblk_t			*mp;
	long				err;
{
	register tcl_endpt_t		*te;
	register union T_primitives	*prim,*prim1;
	long				msz1;
	mblk_t				*mp1;
	tcl_addr_t			addr,addr1;
	int				alen,olen;

#ifndef	NO_UDERR
	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_UNITDATA_REQ);
	/*
	 *	prepare the indication msg
	 */
	msz1 = sizeof(struct T_uderror_ind);
	alen = prim->unitdata_req.DEST_length;
	if (alen > 0) {
		msz1 += alen;
	}
	olen = prim->unitdata_req.OPT_length;
	if (olen > 0) {
		msz1 += olen + NBPW;	/* allow for alignment */
	}
	if (msz1 > TCL_TIDUSZ) {
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_uderr _%d_ fatal: msg too big",__LINE__);
		(void)tcl_fatal(q,mp);
		return(TCL_FAIL);
	}
	if ((mp1 = allocb(msz1,BPRI_HI)) == NULL) {
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_uderr _%d_ fatal: allocb() failure",__LINE__);
		(void)tcl_fatal(q,mp);
		return(TCL_FAIL);
	}
	mp1->b_datap->db_type = M_PROTO;
	mp1->b_wptr = mp1->b_rptr + msz1;
	ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
	prim1 = (union T_primitives *)mp1->b_rptr;
	prim1->uderror_ind.PRIM_type = T_UDERROR_IND;
	prim1->uderror_ind.ERROR_type = err;
	prim1->uderror_ind.DEST_length = alen;
	if (alen <= 0) {
		prim1->uderror_ind.DEST_offset = 0;
	} else {
		prim1->uderror_ind.DEST_offset = sizeof(struct T_uderror_ind);
		addr.ta_alen = alen;
		addr.ta_abuf = (char *)(mp->b_rptr + prim->unitdata_req.DEST_offset);
		/* addr.ta_ahash = tcl_mkahash(&addr);	-- not needed */
		addr1.ta_alen = alen;
		addr1.ta_abuf = (char *)(mp1->b_rptr + prim1->uderror_ind.DEST_offset);
		/* addr1.ta_ahash = tcl_mkahash(&addr1);	-- not needed */
		(void)tcl_cpabuf(&addr1,&addr);		/* cannot fail */
	}
	prim1->uderror_ind.OPT_length = olen;
	if (olen <= 0) {
		prim1->uderror_ind.OPT_offset = 0;
	} else {
		prim1->uderror_ind.OPT_offset = sizeof(struct T_uderror_ind) + prim1->uderror_ind.DEST_length;
		/* blindly copy the option list, retaining alignment */
		while ((prim1->uderror_ind.OPT_offset)%NBPW != (prim->unitdata_req.OPT_offset)%NBPW) {
			prim1->uderror_ind.OPT_offset += 1;	/* alignment */
		}
		(void)bcopy(mp->b_rptr+prim->unitdata_req.OPT_offset,mp1->b_rptr+prim1->uderror_ind.OPT_offset,olen);
	}
	(void)freemsg(mp);
	/*
	 *	send the indication msg
	 */
	te->te_state = NEXTSTATE(TE_UDERROR_IND,te->te_state);
	ASSERT(te->te_state != NR);
	if (q->q_flag & QREADR) {
		(void)putnext(q,mp1);
	} else {
		(void)qreply(q,mp1);
	}
	return(TCL_PASS);
#else
	(void)freemsg(mp);
	return(TCL_PASS);
#endif
}


/*
 *	tcl_data()
 *
 *	handle unitdata request
 */
STATIC int
tcl_data(q,mp)
	queue_t				*q;
	register mblk_t			*mp;
{
	register tcl_endpt_t		*te,*te1;
	register mblk_t			*mp1;
	register union T_primitives	*prim,*prim1;
	register tcl_addr_t		*ta;
	tcl_addr_t			addr;
	int				alen,aoff,olen,olen1,ooff,msz,msz1,i,ckopt;
	long				idflg;
	struct tcl_opt_hdr		*ohdr,*ohdr1,*oohdr1;
	union tcl_opt			*opt,*opt1;


	ASSERT(q != NULL);
	ASSERT(mp != NULL);
	te = (tcl_endpt_t *)q->q_ptr;		/* te = sender */
	ASSERT(te != NULL);
	ASSERT((int)(mp->b_rptr)%NBPW == 0);	/* alignment */
	prim = (union T_primitives *)mp->b_rptr;
	ASSERT(prim != NULL);
	ASSERT(prim->type == T_UNITDATA_REQ);
	/*
	 *	validate the request
	 */
	msz = mp->b_wptr - mp->b_rptr;
	alen = prim->unitdata_req.DEST_length;
	aoff = prim->unitdata_req.DEST_offset;
	olen = prim->unitdata_req.OPT_length;
	ooff = prim->unitdata_req.OPT_offset;
	if ((msz < sizeof(struct T_unitdata_req))
	||  ((alen > 0) && ((aoff + alen) > msz))
	||  ((olen > 0) && ((ooff + olen) > msz))) {
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_data _%d_ fatal: bad control info",__LINE__);
		/*(void)tcl_fatal(q,mp);		/* or tcl_uderr() ?? */
		(void)tcl_uderr(q,mp,TCL_BADADDR);
		return(0);
	}
	if ((alen <= 0) || (alen > TCL_ADDRSZ)) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad dest",__LINE__);
		(void)tcl_uderr(q,mp,TCL_BADADDR);
		return(0);
	}
	if (aoff < 0) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad dest",__LINE__);
		(void)tcl_uderr(q,mp,TCL_BADADDR);
		return(0);
	}
	if (olen < 0) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad opt",__LINE__);
		(void)tcl_uderr(q,mp,TCL_BADOPT);
		return(0);
	}
	if (olen > 0) {
		/*
		 *	no opts supported here
		 */
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad opt",__LINE__);
		(void)tcl_uderr(q,mp,TCL_BADOPT);
		return(0);
	}
	if (msgdsize(mp) > TCL_TSDUSZ) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad opt",__LINE__);
		(void)tcl_uderr(q,mp,TCL_BADOPT);
		return(0);
	}
	/*
	 *	get destination endpoint
	 */
	addr.ta_alen = alen;
	addr.ta_abuf = (char *)(mp->b_rptr + aoff);
	addr.ta_ahash = tcl_mkahash(&addr);
	ta = tcl_getaddr(TCL_DEST,&addr);
	if (ta == NULL) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: bad dest",__LINE__);
		(void)tcl_uderr(q,mp,TCL_NOPEER);
		return(0);
	}
	te1 = ta->ta_blist;		/* te1 = receiver */
	ASSERT(te1 != NULL);
	if (te1->te_state != TS_IDLE) {
		STRLOG(TCL_ID,tcl_min(te),2,SL_TRACE,
		    "tcl_data _%d_ uderr: dest busy",__LINE__);
		(void)tcl_uderr(q,mp,TCL_PEERBADSTATE);
		return(0);
	}
	if (!canput(te1->te_rq->q_next)) {
		STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
		    "tcl_data _%d_: canput() failure",__LINE__);
#ifndef NO_FLOW_CONTROL
		te1->te_backwq = q;
		putbq(q,mp);
		return(-1);
#else
		(void)freemsg(mp); /*Very reasonable thing to do for datagram*/
		return(0);
#endif
	}
	/*
	 *	prepare indication msg
	 */
	msz1 = sizeof(struct T_unitdata_ind) + tcl_alen(te->te_addr);
	idflg = te1->te_idflg;
	ASSERT((idflg & ~TCL_IDFLG_ALL) == 0);
	if (idflg == 0) {
		/* nothing */
	} else {
		olen1 = 0;
		if (idflg & TCL_IDFLG_UID) {
			olen1 += sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_uid);
		}
		if (idflg & TCL_IDFLG_GID) {
			olen1 += sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_gid);
		}
		if (idflg & TCL_IDFLG_RUID) {
			olen1 += sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_ruid);
		}
		if (idflg & TCL_IDFLG_RGID) {
			olen1 += sizeof(struct tcl_opt_hdr) + sizeof(struct tcl_opt_rgid);
		}
		msz1 += olen1 + NBPW;	/* allow for aligment */
	}
	if (msz1 >= TCL_TIDUSZ) {
		STRLOG(TCL_ID,tcl_min(te),1,SL_TRACE,
		    "tcl_data _%d_ fatal: msg too big",__LINE__);
		(void)tcl_fatal(q,mp);
		return(0);
	}
	if ((mp1 = allocb(msz1,BPRI_HI)) == NULL) {
		STRLOG(TCL_ID,tcl_min(te),3,SL_TRACE,
		    "tcl_data _%d_: allocb() failure",__LINE__);
		te1->te_backwq = q;
		putbq(q,mp);
		return(-1);
	}
	mp1->b_datap->db_type = M_PROTO;
	mp1->b_wptr = mp1->b_rptr + msz1;
	ASSERT((int)(mp1->b_rptr)%NBPW == 0);	/* alignment */
	prim1 = (union T_primitives *)mp1->b_rptr;
	prim1->type = T_UNITDATA_IND;
	prim1->unitdata_ind.SRC_length = tcl_alen(te->te_addr);
	prim1->unitdata_ind.SRC_offset = sizeof(struct T_unitdata_ind);
	addr.ta_alen = tcl_alen(te->te_addr);
	addr.ta_abuf = (char *)(mp1->b_rptr + prim1->unitdata_ind.SRC_offset);
	/* addr.ta_ahash = tcl_mkahash(&addr);	-- not needed */
	(void)tcl_cpabuf(&addr,te->te_addr);	/* cannot fail */
	if (idflg == 0) {
		prim1->unitdata_ind.OPT_offset = 0;
		prim1->unitdata_ind.OPT_length = 0;
	} else {
		prim1->unitdata_ind.OPT_offset = prim1->unitdata_ind.SRC_offset + prim1->unitdata_ind.SRC_length;
		while ((prim1->unitdata_ind.OPT_offset)%NBPW != 0) {
			prim1->unitdata_ind.OPT_offset += 1;	/* alignment */
		}
		prim1->unitdata_ind.OPT_length = olen1;
		(void)tcl_wropt(idflg,te,mp1->b_rptr+prim1->unitdata_ind.OPT_offset);
		ASSERT((tcl_ckopt(mp1->b_rptr+prim1->unitdata_ind.OPT_offset,
		    mp1->b_rptr+prim1->unitdata_ind.OPT_offset+prim1->unitdata_ind.OPT_length)
		    & (TCL_BADFORMAT|TCL_BADTYPE|TCL_BADVALUE)) == 0);
	}
	/*
	 *	relink data blocks from mp to mp1
	 */
	/* following is faster than (void)linkb(mp1,unlinkb(mp)); */
	mp1->b_cont = mp->b_cont;
	mp->b_cont = NULL;
	(void)freeb(mp);
	/*
	 *	send indication msg
	 */
	STRLOG(TCL_ID,tcl_min(te),4,SL_TRACE,
	    "tcl_data _%d_: sending T_UNITDATA_IND",__LINE__);
	te1->te_state = NEXTSTATE(TE_UNITDATA_IND,te1->te_state);
	ASSERT(te1->te_state != NR);
	(void)putnext(te1->te_rq,mp1);
	return(0);
}

static void
tcl_link(q, mp)
	register queue_t		*q;
	register mblk_t			*mp;
{
	tcl_addr_t			addr;
	register tcl_addr_t		*ta;
	register struct tcl_sictl	*tcl_sictl;
	register tcl_endpt_t		*te1;

	tcl_sictl = (struct tcl_sictl *)mp->b_rptr;

	addr.ta_alen = tcl_sictl->ADDR_len;
	addr.ta_abuf = (caddr_t)(mp->b_rptr + tcl_sictl->ADDR_offset);
	addr.ta_ahash = tcl_mkahash(&addr);

	if ((ta = tcl_getaddr(TCL_DEST, &addr)) != NULL) {
		/* Link my write queue to the
		 * destinations read queue.
		 */
		te1 = ta->ta_blist;
 
		if (q->q_next == NULL)
			q->q_next = te1->te_rq;
	}
}	

