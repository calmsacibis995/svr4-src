/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/rstatus.c	1.7.4.1"

#include "lpsched.h"

static RSTATUS		*FreeList	= 0;

/**
 ** freerstatus()
 **/

void
#if	defined(__STDC__)
freerstatus (
	register RSTATUS *	r
)
#else
freerstatus (r)
	register RSTATUS	*r;
#endif
{
	ENTRY ("freerstatus")

	if (r->exec) {
		if (r->exec->pid > 0)
			terminate (r->exec);
		r->exec->ex.request = 0;
	}

	if (r->secure) {
		if (r->secure->system)
			Free (r->secure->system);
		if (r->secure->user)
			Free (r->secure->user);
		if (r->secure->req_id)
			Free (r->secure->req_id);
	}

	if (r->request)
		freerequest (r->request);

	if (r->req_file)
		Free (r->req_file);
	if (r->slow)
		Free (r->slow);
	if (r->fast)
		Free (r->fast);
	if (r->pwheel_name)
		Free (r->pwheel_name);
	if (r->printer_type)
		Free (r->printer_type);
	if (r->cpi)
		Free (r->cpi);
	if (r->lpi)
		Free (r->lpi);
	if (r->plen)
		Free (r->plen);
	if (r->pwid)
		Free (r->pwid);

	remover (r);
	r->next = FreeList;
	FreeList = r;

	return;
}

/**
 ** allocr()
 **/

RSTATUS *
#if	defined(__STDC__)
allocr (
	void
)
#else
allocr ()
#endif
{
	ENTRY ("allocr")

	register RSTATUS	*prs;
	register REQUEST	*req;
	register SECURE		*sec;
	

	if ((prs = FreeList)) {

		FreeList = prs->next;
		req	= prs->request;
		sec	= prs->secure;

	} else {

		prs = (RSTATUS *)Malloc(sizeof(RSTATUS));
		req = (REQUEST *)Malloc(sizeof(REQUEST));
		sec = (SECURE *)Malloc(sizeof(SECURE));

	}

	memset ((char *)prs, 0, sizeof(RSTATUS));
	memset ((char *)(prs->request = req), 0, sizeof(REQUEST));
	memset ((char *)(prs->secure = sec), 0, sizeof(SECURE));
	
	return (prs);
}
			
/**
 ** insertr()
 **/

void
#if	defined(__STDC__)
insertr (
	RSTATUS *		r
)
#else
insertr (r)
	RSTATUS			*r;
#endif
{
	ENTRY ("insertr")

	RSTATUS			*prs;


	if (!Request_List) {
		Request_List = r;
		return;
	}
	
	for (prs = Request_List; prs; prs = prs->next) {
		if (rsort(&r, &prs) < 0) {
			r->prev = prs->prev;
			if (r->prev)
				r->prev->next = r;
			r->next = prs;
			prs->prev = r;
			if (prs == Request_List)
				Request_List = r;
			return;
		}

		if (prs->next)
			continue;

		r->prev = prs;
		prs->next = r;
		return;
	}
}

/**
 ** remover()
 **/

void
#if	defined(__STDC__)
remover (
	RSTATUS *		r
)
#else
remover (r)
	RSTATUS			*r;
#endif
{
	ENTRY ("remover")

	if (r == Request_List)
		Request_List = r->next;
	
	if (r->next)
		r->next->prev = r->prev;
	
	if (r->prev)
		r->prev->next = r->next;
	
	r->next = 0;
	r->prev = 0;
	return;
}

/**
 ** request_by_id()
 **/

RSTATUS *
#if	defined(__STDC__)
request_by_id (
	char *			id
)
#else
request_by_id (id)
	char			*id;
#endif
{
	ENTRY ("request_by_id")

	register RSTATUS	*prs;
	
	for (prs = Request_List; prs; prs = prs->next)
		if (STREQU(id, prs->secure->req_id))
			return (prs);
	return (0);
}

/**
 ** request_by_jobid()
 **/

RSTATUS *
#if	defined(__STDC__)
request_by_jobid (
	char *			printer,
	char *			jobid
)
#else
request_by_jobid (printer, jobid)
	char			*printer;
	char			*jobid;
#endif
{
	ENTRY ("request_by_jobid")

	RSTATUS *			prs;

	for (prs = Request_List; prs; prs = prs->next)
		if (
			STREQU(printer, prs->printer->printer->name)
		     && STREQU(jobid, getreqno(prs->secure->req_id))
		)
			return (prs);
    
	return (0);
}

/**
 ** rsort()
 **/

#if	defined(__STDC__)
static int		later ( RSTATUS * , RSTATUS * );
#else
static int		later();
#endif

int
#if	defined(__STDC__)
rsort (
	register RSTATUS **	p1,
	register RSTATUS **	p2
)
#else
rsort (p1, p2)
	register RSTATUS	**p1,
				**p2;
#endif
{
	ENTRY ("rsort")

	/*
	 * Of two requests needing immediate handling, the first
	 * will be the request with the LATER date. In case of a tie,
	 * the first is the one with the larger request ID (i.e. the
	 * one that came in last).
	 */
	if ((*p1)->request->outcome & RS_IMMEDIATE)
		if ((*p2)->request->outcome & RS_IMMEDIATE)
			if (later(*p1, *p2))
				return (-1);
			else
				return (1);
		else
			return (-1);

	else if ((*p2)->request->outcome & RS_IMMEDIATE)
		return (1);

	/*
	 * Of two requests not needing immediate handling, the first
	 * will be the request with the highest priority. If both have
	 * the same priority, the first is the one with the EARLIER date.
	 * In case of a tie, the first is the one with the smaller ID
	 * (i.e. the one that came in first).
	 */
	else if ((*p1)->request->priority == (*p2)->request->priority)
		if (!later(*p1, *p2))
			return (-1);
		else
			return (1);

	else
		return ((*p1)->request->priority - (*p2)->request->priority);
	/*NOTREACHED*/
}

static int
#if	defined(__STDC__)
later (
	register RSTATUS *	prs1,
	register RSTATUS *	prs2
)
#else
later (prs1, prs2)
	register RSTATUS	*prs1,
				*prs2;
#endif
{
	ENTRY ("later")

	if (prs1->secure->date > prs2->secure->date)
		return (1);

	else if (prs1->secure->date < prs2->secure->date)
		return (0);

	/*
	 * The dates are the same, so compare the request IDs.
	 * One problem with comparing request IDs is that the order
	 * of two IDs may be reversed if the IDs wrapped around. This
	 * is a very unlikely problem, because the cycle should take
	 * more than one second to wrap!
	 */
	else {
		register int		len1 = strlen(prs1->req_file),
					len2 = strlen(prs2->req_file);

		/*
		 * Use the request file name (ID-0) for comparison,
		 * because the real request ID (DEST-ID) won't compare
		 * properly because of the destination prefix.
		 * The strlen() comparison is necessary, otherwise
		 * IDs like "99-0" and "100-0" will compare wrong.
		 */
		if (len1 > len2)
			return (1);
		else if (len1 < len2)
			return (0);
		else
			return (strcmp(prs1->req_file, prs2->req_file) > 0);
	}
	/*NOTREACHED*/
}
