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


#ident	"@(#)libyp:yp_enum.c	1.2.1.1"

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
static  char sccsid[] = "@(#)yp_enum.c 1.9 88/02/08 Copyr 1985 Sun Micro";
#endif

#define NULL 0
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

static int dofirst(), donext();

extern void free();
extern char *malloc();
extern void *memcpy();
extern unsigned sleep();
extern size_t strlen();
extern struct timeval _ypserv_timeout;
extern unsigned int _ypsleeptime;
extern int _yp_dobind();

/*
 * This requests the yp server associated with a given domain to return the
 * first key/value pair from the map data base.  The returned key should be
 * used as an input to the call to ypclnt_next.  This part does the parameter
 * checking, and the do-until-success loop.
 */
int
yp_first (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;

	if ( (map == NULL) || (domain == NULL) ) {
		return (YPERR_BADARGS);
	}
	
	domlen = (int) strlen(domain);
	maplen = (int) strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ) {
		return (YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return (reason);
		}

		if (pdomb->dom_binding->ypbind_hi_vers == YPVERS) {

			reason = dofirst(domain, map, pdomb, _ypserv_timeout,
			    key, keylen, val, vallen);

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
	
	return (reason);
}

/*
 * This part of the "get first" interface talks to ypserv.
 */

static int
dofirst (domain, map, pdomb, timeout, key, keylen, val, vallen)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **key;
	int  *keylen;
	char **val;
	int  *vallen;

{
	struct ypreq_nokey req;
	struct ypresp_key_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	resp.keydat.dptr = resp.valdat.dptr = NULL;
	resp.keydat.dsize = resp.valdat.dsize = 0;

	/*
	 * Do the get first request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	memset((char *)&resp, 0, sizeof(struct ypresp_key_val));

	if(clnt_call(pdomb->dom_client, YPPROC_FIRST, xdr_ypreq_nokey,
	    &req, xdr_ypresp_key_val, &resp, timeout) != RPC_SUCCESS) {
		return (YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {

		if ((*key =
		    (char *) malloc((unsigned)
		        resp.keydat.dsize + 2)) != NULL) {

			if ((*val = (char *) malloc(
			    (unsigned) resp.valdat.dsize + 2) ) == NULL) {
				free((char *) *key);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*keylen = resp.keydat.dsize;
		(void) memcpy(*key, resp.keydat.dptr, 
		    (unsigned) resp.keydat.dsize);
		(*key)[resp.keydat.dsize] = '\n';
		(*key)[resp.keydat.dsize + 1] = '\0';
		
		*vallen = resp.valdat.dsize;
		(void) memcpy(*val, resp.valdat.dptr, 
		    (unsigned) resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_key_val, &resp); 
	return (retval);
}

/*
 * This requests the yp server associated with a given domain to return the
 * "next" key/value pair from the map data base.  The input key should be
 * one returned by ypclnt_first or a previous call to ypclnt_next.  The
 * returned key should be used as an input to the next call to ypclnt_next.
 * This part does the parameter checking, and the do-until-success loop.
 */
int
yp_next (domain, map, inkey, inkeylen, outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;


	if ( (map == NULL) || (domain == NULL) || (inkey == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = (int) strlen(domain);
	maplen = (int) strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		if (pdomb->dom_binding->ypbind_hi_vers == YPVERS) {

			reason = donext(domain, map, inkey, inkeylen, pdomb,
			    _ypserv_timeout, outkey, outkeylen, val, vallen);

			if (reason == YPERR_RPC) {
				yp_unbind(domain);
				(void) sleep(_ypsleeptime);
			} else {
				break;
			}
		}
	}
	
	return(reason);
}

/*
 * This part of the "get next" interface talks to ypserv.
 */
static int
donext (domain, map, inkey, inkeylen, pdomb, timeout, outkey, outkeylen,
    val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */

{
	struct ypreq_key req;
	struct ypresp_key_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	req.keydat.dptr = inkey;
	req.keydat.dsize = inkeylen;
	
	resp.keydat.dptr = resp.valdat.dptr = NULL;
	resp.keydat.dsize = resp.valdat.dsize = 0;

	/*
	 * Do the get next request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client,
	    YPPROC_NEXT, xdr_ypreq_key, &req, xdr_ypresp_key_val, &resp,
	    timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {
		if ( (*outkey = (char *) malloc((unsigned)
		    resp.keydat.dsize + 2) ) != NULL) {

			if ( (*val = (char *) malloc((unsigned)
			    resp.valdat.dsize + 2) ) == NULL) {
				free((char *) *outkey);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*outkeylen = resp.keydat.dsize;
		(void) memcpy(*outkey, resp.keydat.dptr,
		    (unsigned) resp.keydat.dsize);
		(*outkey)[resp.keydat.dsize] = '\n';
		(*outkey)[resp.keydat.dsize + 1] = '\0';
		
		*vallen = resp.valdat.dsize;
		(void) memcpy(*val, resp.valdat.dptr, 
		    (unsigned) resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_key_val, &resp);
	return(retval);
}
