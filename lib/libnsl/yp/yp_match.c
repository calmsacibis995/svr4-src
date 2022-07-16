/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:yp_match.c	1.2.1.1"

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
#if !defined(lint) && defined(SCCSIDS)
static  char sccsid[] = "@(#)yp_match.c 1.13 88/02/08 Copyr 1985 Sun Micro";
#endif

#define NULL 0
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

extern struct timeval _ypserv_timeout;
extern int _yp_dobind();
extern unsigned int _ypsleeptime;
extern char *malloc();
extern void free();
extern int gettimeofday();
extern int memcmp();
extern void *memcpy();
extern void *memset();
extern unsigned sleep();
extern int strcmp();
extern char *strcpy();
extern size_t strlen();
static int domatch();

struct cache {
	struct cache *next;
	unsigned int birth;
	char *domain;
	char *map;
	char *key;
	int  keylen;
	char *val;
	int  vallen;
};

static struct cache *head;
#define CACHESZ 16
#define CACHETO 600

static void
detachnode(prev, n)
	register struct cache *prev, *n;
{

	if (prev == 0) {	
		/* assertion: n is head */
		head = n->next;
	} else {
		prev->next = n->next;
	}
	n->next = 0;
}

static void
freenode(n)
	register struct cache *n;
{

	if (n->val != 0)
	    free(n->val);
	if (n->key != 0)
	    free(n->key);
	if (n->map != 0)
	    free(n->map);
	if (n->domain != 0)
	    free(n->domain);
	memset((char *) n, 0, sizeof(*n));
	free((char *) n);
}

static struct cache *
makenode(domain, map, keylen, vallen)
	char *domain, *map;
	int keylen, vallen;
{
	register struct cache *n =
	    (struct cache *) malloc(sizeof(struct cache));

	if (n == 0)
	    return (0);
	memset((char *) n, 0, sizeof(*n));
	for (;;) {
		if ((n->domain = malloc((unsigned)
		    (1 + (int) strlen(domain)))) == 0)
			break;
		if ((n->map = malloc((unsigned)
		    (1 + (int) strlen(map)))) == 0)
			break;
		if ((n->key = malloc((unsigned) keylen)) == 0)
			break;
		if ((n->val = malloc((unsigned) vallen)) == 0)
			break;
		else return (n);
	}
	
	freenode(n);
	return (0);
}

/*
 * Requests the yp server associated with a given domain to attempt to match
 * the passed key datum in the named map, and to return the associated value
 * datum. This part does parameter checking, and implements the "infinite"
 * (until success) sleep loop.
 */
int
yp_match (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char *key;
	register int  keylen;
	char **val;		/* returns value array */
	int  *vallen;		/* returns bytes in val */
{
	int domlen;
	int maplen;
	int reason;
	struct dom_binding *pdomb;
	register struct cache *c, *prev;
	int cnt, savesize;
	struct timeval now;
	struct timezone tz;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = (int) strlen(domain);
	maplen = (int) strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (key == NULL) || (keylen == 0) ) {
		return(YPERR_BADARGS);
	}
	/* is it in our cache ? */
	prev = 0;
	for (prev=0, cnt=0, c=head; c != 0; prev=c, c=c->next, cnt++) {
		if ((c->keylen == keylen) &&
		    (memcmp(key, c->key, (unsigned) keylen) == 0) &&
		    (strcmp(map, c->map) == 0) &&
		    (strcmp(domain, c->domain) == 0)) {
			/* cache hit */
			(void) gettimeofday(&now, &tz);
			if ((now.tv_sec - c->birth) > CACHETO) {
				/* rats.  it it too old to use */
				detachnode(prev, c);
				freenode(c);
				break;
			} else {
				/* NB: Copy two extra bytes; see below */
				savesize = c->vallen + 2;
				*val = malloc((unsigned) savesize);
				if (*val == 0) {
					return (YPERR_RESRC);
				}
				(void) memcpy(*val, c->val,
				    (unsigned) savesize);
				*vallen = c->vallen;
				detachnode(prev, c);
				c->next = head;
				head = c;
				return (0);
			}
		}
		if (cnt >= CACHESZ) {
			detachnode(prev, c);
			freenode(c);
			break;
		}
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		if (pdomb->dom_binding->ypbind_hi_vers >= YPVERS) {

			reason = domatch(domain, map, key, keylen, pdomb,
			    _ypserv_timeout, val, vallen);

			if (reason == YPERR_RPC) {
				yp_unbind(domain);
				(void) sleep(_ypsleeptime);
			} else {
				break;
			}
		} else {
			return(YPERR_VERS);
		}
	}
	
	/* add to our cache */
	if (reason == 0) {
		/*
		 * NB: allocate and copy extract two bytes of the value;
		 * these two bytes are mandatory CR and NULL bytes.
		 */
		savesize = *vallen + 2;
		c = makenode(domain, map, keylen, savesize);
		if (c != 0) {
			(void) gettimeofday(&now, &tz);
			c->next = head;
			head = c;
			c->birth = now.tv_sec;
			(void) strcpy(c->domain, domain);
			(void) strcpy(c->map, map);
			(void) memcpy(c->key, key,
			    (unsigned)(c->keylen = keylen));
			(void) memcpy(c->val, *val,
			    (unsigned) savesize);
			c->vallen = *vallen;
		}
	}
	return(reason);
}

/*
 * This talks v3 protocol to ypserv
 */
static int
domatch (domain, map, key, keylen, pdomb, timeout, val, vallen)
	char *domain;
	char *map;
	char *key;
	int  keylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	struct ypreq_key req;
	struct ypresp_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	req.keydat.dptr = key;
	req.keydat.dsize = keylen;
	
	resp.valdat.dptr = NULL;
	resp.valdat.dsize = 0;
	memset((char *)&resp, 0, sizeof(struct ypresp_val));

	/*
	 * Do the match request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client,
	    YPPROC_MATCH, xdr_ypreq_key, &req, xdr_ypresp_val, &resp,
	    timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval && ((*val = malloc((unsigned)
	    resp.valdat.dsize + 2)) == NULL)) {
		retval = YPERR_RESRC;
	}

	/* Copy the returned value byte string into the new memory */

	if (!retval) {
		*vallen = resp.valdat.dsize;
		(void) memcpy(*val, resp.valdat.dptr,
		    (unsigned) resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}

	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_val, &resp);
	return(retval);

}
