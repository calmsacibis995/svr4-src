/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/disp1.c	1.11.4.1"

#include "dispatch.h"

#if	defined(__STDC__)
static char	*reqpath ( char *, char ** );
#else
static char	*reqpath();
#endif

RSTATUS			*NewRequest;

/**
 ** s_alloc_files()
 **/

#if	defined(__STDC__)
void s_alloc_files ( char * m, MESG * md )	/* funcdef */
#else
void s_alloc_files ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_alloc_files")

    char			*file_prefix;
    ushort			count;

    getmessage (m, S_ALLOC_FILES, &count);

    if ((file_prefix = _alloc_files(count, (char *)0, md->uid, md->gid)))
    {
	mputm (md, R_ALLOC_FILES, MOK, file_prefix);
	add_flt_act(md, FLT_FILES, file_prefix, count);
    }
    else
	mputm (md, R_ALLOC_FILES, MNOMEM, "");

    return;
}

/**
 ** s_print_request()
 **/

#if	defined(__STDC__)
void s_print_request ( char * m, MESG * md )	/* funcdef */
#else
void s_print_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_print_request")

    extern char			*Local_System;
    char			*file;
    char			*idno;
    char			*path;
    char			*req_file;
    char			*req_id	= 0;
    RSTATUS			*rp;
    REQUEST			*r;
    SECURE			*s;
    struct passwd		*pw;
    short			err;
    short			status;
    off_t			size;


    (void) getmessage (m, S_PRINT_REQUEST, &file);

    /*
     * "NewRequest" points to a request that's not yet in the
     * request list but is to be considered with the rest of the
     * requests (e.g. calculating # of requests awaiting a form).
     */
    if (!(rp = NewRequest = allocr()))
	status = MNOMEM;

    else
    {
	req_file = reqpath(file, &idno);
	path = makepath(Lp_Tmp, req_file, (char *)0);
	(void) Chmod(path, 0600);
	(void) Chown(path, Lp_Uid, Lp_Gid);
	Free (path);
    
	if (!(r = Getrequest(req_file)))
	    status = MNOOPEN;

	else
	{
	    *(rp->request) = *r;
	    rp->req_file = Strdup(req_file);

	    /*
	    **	Test for the presence of a secure file.
	    **	If found skip sanity checks.
	    **	The secure file will only exist if the request
	    **	originated on a different system.
	    */
	    if ((s = Getsecure(req_file)))
	    {
		rp->request->outcome = 0;
		*(rp->secure) = *s;
	    }
	    else
	    {
		rp->request->outcome = 0;
		rp->secure->uid = md->uid;
		rp->secure->gid = md->gid;
    
		pw = lp_getpwuid(md->uid);
		lp_endpwent();
		if (pw && pw->pw_name && *pw->pw_name)
		    rp->secure->user = Strdup(pw->pw_name);
		else
		{
		    rp->secure->user = Strdup(BIGGEST_NUMBER_S);
		    (void) sprintf (rp->secure->user, "%ld", md->uid);
		}
	    
		if ((rp->request->actions & ACT_SPECIAL) == ACT_HOLD)
		    rp->request->outcome |= RS_HELD;
		if ((rp->request->actions & ACT_SPECIAL) == ACT_RESUME)
		    rp->request->outcome &= ~RS_HELD;
		if((rp->request->actions & ACT_SPECIAL) == ACT_IMMEDIATE)
		{
		    if (!md->admin)
		    {
			status = MNOPERM;
			goto Return;
		    }
		    rp->request->outcome |= RS_IMMEDIATE;
		}

		size = chfiles(rp->request->file_list, Lp_Uid, Lp_Gid);
		if (size < 0)
		{
		    status = MUNKNOWN;
		    goto Return;
		}
		if (!(rp->request->outcome & RS_HELD) && size == 0)
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->secure->size = size;

		(void) time(&rp->secure->date);
		rp->secure->req_id = NULL;
		rp->secure->system = Strdup(Local_System);
	    }
	    
	    if((err = validate_request(rp, &req_id, 0)) != MOK)
		status = err;

	    else
	    {
		/*
		 * "req_id" will be supplied if this is from a
		 * remote system.
		 */
		if (rp->secure->req_id == NULL)
		{
		    req_id = makestr(req_id, "-", idno, (char *)0);
		    rp->secure->req_id = req_id;
		}
		
		if (
		    putsecure(req_file, rp->secure) == -1
		 || putrequest(req_file, rp->request) == -1
		)
		    status = MNOMEM;

		else
		{
		    status = MOK;

		    insertr(rp);
		    NewRequest = 0;
    
		    if (rp->slow)
			schedule (EV_SLOWF, rp);
		    else
			schedule (EV_INTERF, rp->printer);

		    del_flt_act(md, FLT_FILES);
		}
	    }
	}
    }

Return:
    NewRequest = 0;
    Free(req_file);
    Free(idno);
    if (status != MOK && rp)
    {
	rmfiles(rp, 0);
	freerstatus(rp);
    }
    mputm(md, R_PRINT_REQUEST, status, NB(req_id), chkprinter_result);
    return;
}

/**
 ** s_start_change_request()
 **/

#if	defined(__STDC__)
void s_start_change_request ( char * m, MESG * md )	/* funcdef */
#else
void start_change_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_start_change_request")

    char		*req_id;
    char		*req_file	= "";
    short		status;
    RSTATUS		*rp;
#if	defined(ALLOW_CHANGE_REMOTE)
    char		*system;
    char		*cp;
#endif
    char		*path;
    
    (void) getmessage(m, S_START_CHANGE_REQUEST, &req_id);

#if	defined(ALLOW_CHANGE_REMOTE)
    if ((cp = strchr(req_id, BANG_C)) != NULL)
    {
	*cp++ = '\0';
	system = req_id;
	req_id = cp;
    }
#endif
    
    if (!(rp = request_by_id(req_id)))
	status = MUNKNOWN;

    else if (rp->request->outcome & RS_GONEREMOTE)
	status = MGONEREMOTE;

    else if (rp->request->outcome & RS_DONE)
	status = M2LATE;

    else if (!md->admin && md->uid != rp->secure->uid)
	status = MNOPERM;

    else if (rp->request->outcome & RS_CHANGING)
	status = MNOOPEN;

    else if (rp->request->outcome & RS_NOTIFYING)
	status = MBUSY;

    else
    {
	status = MOK;

	if (
	    rp->request->outcome & RS_FILTERING
	 && !(rp->request->outcome & RS_STOPPED)
	)
	{
	    rp->request->outcome |= (RS_REFILTER|RS_STOPPED);
	    terminate (rp->exec);
	}

	if (
	    rp->request->outcome & RS_PRINTING
	 && !(rp->request->outcome & RS_STOPPED)
	)
	{
	    rp->request->outcome |= RS_STOPPED;
	    terminate (rp->printer->exec);
	}

	rp->request->outcome |= RS_CHANGING;	
    
	/*
	 * Change the ownership of the request file to be "md->uid".
	 * Either this is identical to "rp->secure->uid", or it is
	 * "Lp_Uid" or it is root. The idea is that the
	 * person at the other end needs access, and that may not
	 * be who queued the request.
	 */
	path = makepath(Lp_Tmp, rp->req_file, (char *)0);
	(void) Chown(path, md->uid, rp->secure->gid);
	Free (path);

	add_flt_act(md, FLT_CHANGE, rp);
	req_file = rp->req_file;

    }
    mputm(md, R_START_CHANGE_REQUEST, status, req_file);
    return;
}

/**
 ** s_end_change_request()
 **/

#if	defined(__STDC__)
void s_end_change_request ( char * m, MESG * md )	/* funcdef */
#else
void s_end_change_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_end_change_request")

    char		*req_id;
    RSTATUS		*rp;
    off_t		size;
    off_t		osize;
    short		err;
    short		status;
    REQUEST		*r		= 0;
    REQUEST		oldr;
    int			call_schedule	= 0;
    int			move_ok		= 0;
#if	defined(ALLOW_CHANGE_REMOTE)
    char		*system = Local_System;
    char		*cp;
#endif
    char		*path;
    
    (void) getmessage(m, S_END_CHANGE_REQUEST, &req_id);

#if	defined(ALLOW_CHANGE_REMOTE)
    if ((cp = strchr(req_id, BANG_C)) != NULL)
    {
	*cp++ = '\0';
	system = req_id;
	req_id = cp;
    }
#endif

    if (!(rp = request_by_id(req_id)))
	status = MUNKNOWN;

    else if (rp->request->outcome & RS_GONEREMOTE)
	status = MGONEREMOTE;	/* should never happen, but... */

    else if (!(rp->request->outcome & RS_CHANGING))
	status = MNOSTART;

    else
    {
	path = makepath(Lp_Tmp, rp->req_file, (char *)0);
	(void) Chmod(path, 0600);
	(void) Chown(path, Lp_Uid, Lp_Gid);
	Free (path);

	rp->request->outcome &= ~(RS_CHANGING);
	del_flt_act(md, FLT_CHANGE);
	/*
	 * The RS_CHANGING bit may have been the only thing preventing
	 * this request from filtering or printing, so regardless of what
	 * happens below, we must check to see if the request can proceed.
	 */
	call_schedule = 1;

	if (!(r = Getrequest(rp->req_file)))
	    status = MNOOPEN;

	else
	{
	    oldr = *(rp->request);
	    *(rp->request) = *r;

	    move_ok = STREQU(oldr.destination, r->destination);

	    /*
	     * Preserve the current request status!
	     */
	    rp->request->outcome = oldr.outcome;

	    /*
	     * Here's an example of the dangers one meets when public
	     * flags are used for private purposes. ".actions" (indeed,
	     * anything in the REQUEST structure) is set by the person
	     * changing the job. However, lpsched uses ".actions" as
	     * place to indicate that a job came from a remote system
	     * and we must send back job completion--this is a strictly
	     * private flag that we must preserve.
	     */
	    rp->request->actions |= (oldr.actions & ACT_NOTIFY);

	    if ((rp->request->actions & ACT_SPECIAL) == ACT_HOLD)
	    {
		rp->request->outcome |= RS_HELD;
		/*
		 * To be here means either the user owns the request
		 * or he or she is the administrator. Since we don't
		 * want to set the RS_ADMINHELD flag if the user is
		 * the administrator, the following compare will work.
		 */
		if (md->uid != rp->secure->uid)
		    rp->request->outcome |= RS_ADMINHELD;
	    }

	    if ((rp->request->actions & ACT_SPECIAL) == ACT_RESUME)
	    {
		if (
		    (rp->request->outcome & RS_ADMINHELD)
		 && !md->admin
		)
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->request->outcome &= ~(RS_ADMINHELD|RS_HELD);
	    }

	    if((rp->request->actions & ACT_SPECIAL) == ACT_IMMEDIATE)
	    {
		if (!md->admin)
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->request->outcome |= RS_IMMEDIATE;
	    }

	    size = chfiles(rp->request->file_list, Lp_Uid, Lp_Gid);
	    if (size < 0)
	    {
		status = MUNKNOWN;
		goto Return;
	    }
	    if (!(rp->request->outcome & RS_HELD) && size == 0)
	    {
		status = MNOPERM;
		goto Return;
	    }

	    osize = rp->secure->size;
	    rp->secure->size = size;

	    if ((err = validate_request(rp, (char **)0, move_ok)) != MOK)
	    {
		status = err;
		rp->secure->size = osize;
	    }
	    else
	    {
		status = MOK;

		if(rp->request->outcome & RS_IMMEDIATE)
		{
		    remover(rp);
		    insertr(rp);
		}

		(void) time(&rp->secure->date);

		freerequest(&oldr);
		(void) putrequest(rp->req_file, rp->request);
		(void) putsecure(rp->req_file, rp->secure);
	    }
	}
    }

Return:
    if (status != MOK && rp)
    {
	if (r)
	{
	    freerequest(r);
	    *(rp->request) = oldr;
	}
	if (status != MNOSTART)
	    (void) putrequest(rp->req_file, rp->request);
    }

    if (call_schedule)
	maybe_schedule(rp);

    mputm(md, R_END_CHANGE_REQUEST, status, chkprinter_result);
    return;
}

/**
 ** _cancel()
 **/

static char *
#if	defined(__STDC__)
_cancel ( MESG * md, char * dest, char * user, char * req_id)	/* funcdef */
#else
_cancel ( md, dest, user, req_id )
MESG	*md;
char	*dest;
char	*user;
char	*req_id;
#endif
{
    ENTRY ("_cancel")

    static RSTATUS	*rp;
    static char		*s_dest;
    static char		*s_user;
    static char		*s_req_id;
    static int		current;
    RSTATUS		*crp;
    char		*creq_id;
    
    if (dest || user || req_id)
    {
	s_dest = dest;
	s_user = user;
	s_req_id = req_id;
	rp = Request_List;
	current = 0;
	if (STREQU(s_req_id, CURRENT_REQ))
	{
	    current = 1;
	    s_req_id = NULL;
	}
    }

    while (rp != NULL)
    {
	crp = rp;
	rp = rp->next;
	
	if (*s_dest && !STREQU(s_dest, crp->request->destination))
	    continue;

	if (current && !(crp->request->outcome & RS_PRINTING))
	    continue;

	if (s_req_id && *s_req_id && !STREQU(s_req_id, crp->secure->req_id))
	    continue;

	if (*s_user && !bangequ(s_user, crp->secure->user))
	    continue;

	if (!md->admin && md->uid != crp->secure->uid)
	{
	    errno = MNOPERM;
	    return(Strdup(crp->secure->req_id));
	}

	crp->reason = MOK;
	creq_id = Strdup(crp->secure->req_id);

	if (cancel(crp, (md->uid != crp->secure->uid)))
	    errno = MOK;
	else
	    errno = M2LATE;
	return(creq_id);
    }

    errno = MUNKNOWN;
    return(NULL);
}

/**
 ** s_cancel_request()
 **/

#if	defined(__STDC__)
void s_cancel_request ( char * m, MESG * md )	/* funcdef */
#else
void s_cancel_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_cancel_request")

    char	*req_id;
    char	*rid;
    short	status;

    (void) getmessage(m, S_CANCEL_REQUEST, &req_id);

    if (!request_by_id(req_id))
	status = MUNKNOWN;
    else
    {
	if ((rid = _cancel(md, "", "", req_id)) != NULL)
	    Free(rid);
	status = (short)errno;
    }
    mputm(md, R_CANCEL_REQUEST, status);
}

/**
 ** s_cancel()
 **/

#if	defined(__STDC__)
void s_cancel ( char * m, MESG * md )	/* funcdef */
#else
void s_cancel ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_cancel")

    char	*req_id;
    char	*user;
    char	*destination;
    char	*rid;
    char	*nrid;
    int		nerrno;
    int		oerrno;

    (void) getmessage(m, S_CANCEL, &destination, &user, &req_id);

    if (STREQU(user, NAME_ALL))
	user = "";
    if (STREQU(destination, NAME_ALL))
	destination = "";
    if (STREQU(req_id, NAME_ALL))
	req_id = "";

    if (rid = _cancel(md, destination, user, req_id))
    {
	oerrno = errno;

	while ((nrid = _cancel(md, NULL, NULL, NULL)) != NULL)
	{
	    nerrno = errno;
	    mputm(md, R_CANCEL, MOKMORE, oerrno, rid);
	    Free(rid);
	    rid = nrid;
	    oerrno = nerrno;
	}
	mputm(md, R_CANCEL, MOK, oerrno, rid);
	Free(rid);
	return;
    }

    mputm(md, R_CANCEL, MOK, MUNKNOWN, "");
}

/**
 ** s_inquire_request()
 **/

#if	defined(__STDC__)
void s_inquire_request ( char * m, MESG * md )	/* funcdef */
#else
void s_inquire_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_inquire_request")

    char	*form;
    char	*dest;
    char	*pwheel;
    char	*user;
    char	*req_id;
    RSTATUS	*rp;
    RSTATUS	*found;

    found = (RSTATUS *)0;

    (void) getmessage(m, S_INQUIRE_REQUEST,&form,&dest,&req_id,&user,&pwheel);

    for(rp = Request_List; rp != NULL; rp = rp->next)
    {
	if (*form && !SAME(form, rp->request->form))
	    continue;

	if (*dest && !STREQU(dest, rp->request->destination))
	    continue;
	
	if (*req_id && !STREQU(req_id, rp->secure->req_id))
	    continue;

	if (*user && !bangequ(user, rp->secure->user))
	    continue;

	if (*pwheel && !SAME(pwheel, rp->pwheel_name))
	    continue;
	
	if (found)
	    mputm(md, R_INQUIRE_REQUEST,
		 MOKMORE,
		 found->secure->req_id,
		 found->secure->user,
		 found->secure->size,
		 found->secure->date,
		 found->request->outcome,
		 found->printer->printer->name,
		 (found->form? found->form->form->name : ""),
		 NB(found->pwheel_name)
	    );
	found = rp;
    }

    if (found)
	mputm(md, R_INQUIRE_REQUEST,
	     MOK,
	     found->secure->req_id,
	     found->secure->user,
	     found->secure->size,
	     found->secure->date,
	     found->request->outcome,
	     found->printer->printer->name,
	     (found->form? found->form->form->name : ""),
	     NB(found->pwheel_name)
	);
    else
	mputm(md, R_INQUIRE_REQUEST, MNOINFO, "", "", 0L, 0L, 0, "", "", "");

    return;
}

/**
 ** s_inquire_request_rank()
 **/

#if	defined(__STDC__)
void s_inquire_request_rank ( char * m, MESG * md )	/* funcdef */
#else
void s_inquire_request_rank ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_inquire_request_rank")

    char			*form;
    char			*dest;
    char			*pwheel;
    char			*user;
    char			*req_id;
    RSTATUS			*rp;
    RSTATUS			*found;
    char			**sstlist = NULL;
    int				index;
    PSTATUS			*pps;
    int				found_rank;
    short			prop;

    found = (RSTATUS *)0;

    (void) getmessage(m, S_INQUIRE_REQUEST_RANK,&prop, 
	   &form,&dest,&req_id,&user,&pwheel);

    if (Redispatch || !prop)
	goto SendBackStatus;

    for(rp = Request_List; rp != NULL; rp = rp->next)
    {
	if (*form && !SAME(form, rp->request->form))
	    continue;

	if (*dest && !STREQU(dest, rp->request->destination))
	    continue;

	if (*req_id && !STREQU(req_id, rp->secure->req_id))
	    continue;

	if (*user && !bangequ(user, rp->secure->user))
	    continue;

	if (*pwheel && !SAME(pwheel, rp->pwheel_name))
	    continue;
	
	if (
		rp->printer->status & PS_REMOTE
	     && rp->request->outcome & (RS_SENT | RS_SENDING)
	     && !(rp->request->outcome & RS_DONE)
	)
	    (void) addlist(&sstlist, rp->printer->system->system->name);
    }

    if (lenlist(sstlist) > 0)
    {
	for (index = 0; sstlist[index]; index++)
	    askforstatus(search_stable(sstlist[index]), md);
	
	if (waitforstatus(m, md) == 0)
	    return;
    }

SendBackStatus:
    for (pps = walk_ptable(1); pps; pps = walk_ptable(0))
	pps->nrequests = 0;

    for(rp = Request_List; rp != NULL; rp = rp->next)
    {
	if (rp->printer && !(rp->request->outcome & RS_DONE))
	    rp->printer->nrequests++;

	if (*form && !SAME(form, rp->request->form))
	    continue;

	if (*dest && !STREQU(dest, rp->request->destination))
	    continue;
	
	if (*req_id && !STREQU(req_id, rp->secure->req_id))
	    continue;

	if (*user && !bangequ(user, rp->secure->user))
	    continue;

	if (*pwheel && !SAME(pwheel, rp->pwheel_name))
	    continue;

	if (found)
	    mputm(md, R_INQUIRE_REQUEST_RANK,
		  MOKMORE,
		  found->secure->req_id,
		  found->secure->user,
		  found->secure->size,
		  found->secure->date,
		  found->request->outcome,
		  found->printer->printer->name,
		  (found->form? found->form->form->name : ""),
		  NB(found->pwheel_name),
		  (found->status & RSS_RANK) ? found->rank : found_rank
	    );
	found = rp;
	found_rank = found->printer->nrequests;
    }

    if (found)
	mputm(md, R_INQUIRE_REQUEST_RANK,
	     MOK,
	     found->secure->req_id,
	     found->secure->user,
	     found->secure->size,
	     found->secure->date,
	     found->request->outcome,
	     found->printer->printer->name,
	     (found->form? found->form->form->name : ""),
	     NB(found->pwheel_name),
	     (found->status & RSS_RANK) ? found->rank : found_rank
	);
    else
	mputm(md, R_INQUIRE_REQUEST_RANK, MNOINFO, "", "", 0L, 0L, 0, "", "", "", 0);

    return;
}

/**
 ** s_move_request()
 **/

#if	defined(__STDC__)
void s_move_request ( char * m, MESG * md )	/* funcdef */
#else
void s_move_request ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_move_request")

    char	*olddest;
    RSTATUS	*rp;
    short	err;
    char	*req_id;
    char	*dest;
    EXEC	*oldexec;
#if	defined(ALLOW_CHANGE_REMOTE)
    char	*system = Local_System;
    char	*cp;
#endif

    (void) getmessage(m, S_MOVE_REQUEST, &req_id, &dest);

#if	defined(ALLOW_CHANGE_REMOTE)
    if ((cp = strchr(req_id, BANG_C)) != NULL)
    {
	*cp++ = '\0';
	system = req_id;
	req_id = cp;
    }
#endif

    if (!(search_ptable(dest)) && !(search_ctable(dest)))
    {
	mputm(md, R_MOVE_REQUEST, MNODEST, 0L);
	return;
    }

    if (rp = request_by_id(req_id))
    {
	if (rp->request->outcome & RS_GONEREMOTE)
	{
	    mputm(md, R_MOVE_REQUEST, MGONEREMOTE, 0L);
	    return;
	}
	if (STREQU(rp->request->destination, dest))
	{
	    mputm(md, R_MOVE_REQUEST, MOK, 0L);
	    return;
	}
	if (rp->request->outcome & (RS_DONE|RS_NOTIFYING))
	{
	    mputm(md, R_MOVE_REQUEST, M2LATE, 0L);
	    return;
	}
	if (rp->request->outcome & RS_CHANGING)
	{
	    mputm(md, R_MOVE_REQUEST, MBUSY, 0L);
	    return;
	}
	oldexec = rp->printer->exec;
	olddest = rp->request->destination;
	rp->request->destination = Strdup(dest);
	if ((err = validate_request(rp, (char **)0, 1)) == MOK)
	{
	    Free(olddest);
	    (void) putrequest(rp->req_file, rp->request);
	    mputm(md, R_MOVE_REQUEST, MOK, 0L);

	    /*
	     * If the request was being filtered or was printing,
	     * it would have been stopped in "validate_request()",
	     * but only if it has to be refiltered. Thus, the
	     * filtering has been stopped if it has to be stopped,
	     * but the printing may still be going.
	     */
	    if (
		rp->request->outcome & RS_PRINTING
	     && !(rp->request->outcome & RS_STOPPED)
	    )
	    {
		rp->request->outcome |= RS_STOPPED;
		terminate (oldexec);
	    }

	    maybe_schedule(rp);

	    return;
	}
	mputm(md, R_MOVE_REQUEST, err, chkprinter_result);
	Free(rp->request->destination);
	rp->request->destination = olddest;
	return;
    }
    mputm(md, R_MOVE_REQUEST, MUNKNOWN, 0L);
}

/**
 ** s_move_dest()
 **/

#if	defined(__STDC__)
void s_move_dest  ( char * m, MESG * md )	/* funcdef */
#else
void s_move_dest ( m, md )
char	*m;
MESG	*md;
#endif
{
    ENTRY ("s_move_dest")

    char		*dest;
    char		*fromdest;
    RSTATUS		*rp;
    char		*olddest;
    EXEC		*oldexec;
    char		*found = (char *)0;
    short		num_ok = 0;

    (void) getmessage(m, S_MOVE_DEST, &fromdest, &dest);

    if (!search_ptable(fromdest) && !search_ctable(fromdest))
    {
	mputm(md, R_MOVE_DEST, MNODEST, fromdest, 0);
	return;
    }

    if (!(search_ptable(dest)) && !(search_ctable(dest)))
    {
	mputm(md, R_MOVE_DEST, MNODEST, dest, 0);
	return;
    }

    if (STREQU(dest, fromdest))
    {
	mputm(md, R_MOVE_DEST, MOK, "", 0);
	return;
    }

    BEGIN_WALK_BY_DEST_LOOP (rp, fromdest)
	if (!(rp->request->outcome & (RS_DONE|RS_CHANGING|RS_NOTIFYING|RS_GONEREMOTE)))
	{
	    oldexec = rp->printer->exec;
	    olddest = rp->request->destination;
	    rp->request->destination = Strdup(dest);
	    if (validate_request(rp, (char **)0, 1) == MOK)
	    {
		num_ok++;
		Free(olddest);
		(void) putrequest(rp->req_file, rp->request);

		/*
		 * If the request was being filtered or was printing,
		 * it would have been stopped in "validate_request()",
		 * but only if it has to be refiltered. Thus, the
		 * filtering has been stopped if it has to be stopped,
		 * but the printing may still be going.
		 */
		if (
		    rp->request->outcome & RS_PRINTING
		 && !(rp->request->outcome & RS_STOPPED)
		)
		{
		    rp->request->outcome |= RS_STOPPED;
		    terminate (oldexec);
	        }

		maybe_schedule(rp);

		continue;
	    }
	    Free(rp->request->destination);
	    rp->request->destination = olddest;
	}

	if (found)
	    mputm(md, R_MOVE_DEST, MMORERR, found, 0);

	found = rp->secure->req_id;
    END_WALK_LOOP

    if (found)
	mputm(md, R_MOVE_DEST, MERRDEST, found, num_ok);
    else
	mputm(md, R_MOVE_DEST, MOK, "", num_ok);
}

/**
 ** reqpath
 **/

#if	defined(__STDC__)
static char * reqpath ( char * file, char ** idnumber )	/* funcdef */
#else
static char * reqpath ( file, idnumber )
char	*file;
char	**idnumber;
#endif
{
    ENTRY ("reqpath")

    char	*path;
    char	*cp;
    char	*cp2;
    
    /*
    **	/var/spool/lp/tmp/machine/123-0
    **	/var/spool/lp/temp/123-0
    **	/usr/spool/lp/temp/123-0
    **	/usr/spool/lp/tmp/machine/123-0
    **	123-0
    **	machine/123-0
    **
    **	/var/spool/lp/tmp/machine/123-0 + 123
    */
    if (*file == '/')
    {
	if (STRNEQU(file, Lp_Spooldir, strlen(Lp_Spooldir)))
	    cp = file + strlen(Lp_Spooldir) + 1;
	else
	    if(STRNEQU(file, "/usr/spool/lp", 13))
		cp = file + strlen("/usr/spool/lp") + 1;
	    else
	    {
		*idnumber = NULL;
		return(NULL);
	    }

	if (STRNEQU(cp, "temp", 4))
	{
	    cp += 5;
	    path = makepath(Local_System, cp, NULL);
	}
	else
	    path = Strdup(cp);
    }
    else
    {
	if (strchr(file, '/'))
	    path = makepath(file, NULL);
	else
	    path = makepath(Local_System, file, NULL);
    }

    cp = strrchr(path, '/');
    cp++;
    if ((cp2 = strchr(cp, '-')) == NULL)
	*idnumber = Strdup(cp);
    else
    {
	*cp2 = '\0';
	*idnumber = Strdup(cp);
	*cp2 = '-';
    }

    return(path);
}
