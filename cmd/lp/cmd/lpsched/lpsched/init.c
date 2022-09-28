/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/init.c	1.10.4.1"

#include "lpsched.h"

CSTATUS		*CStatus;		/* Status of same          */
EXEC		*Exec_Table;		/* Running processes       */
EXEC		*Exec_Slow;		/*   Slow filters	   */
EXEC		*Exec_Notify;		/*   Notifications	   */
FSTATUS		*FStatus;		/* status of same	   */
PSTATUS		*PStatus;		/* Status of same          */
PWSTATUS	*PWStatus;		/* Status of same          */ 
RSTATUS		*Request_List;		/* Queue of print requests */

SSTATUS		**SStatus;		/* Array of systems */

int		CT_Size,
		ET_Size,
		ET_SlowSize	= 1,
		ET_NotifySize	= 1,
		FT_Size,
		PT_Size,
		PWT_Size;

static ALERT	*Alert_Table;		/* Printer fault alerts    */
static ALERT	*FAlert_Table;		/* Form mount alerts       */
static ALERT	*PAlert_Table;		/* PrintWheel mount alerts */
static CLASS	*Class_Table;		/* Known classes           */
static _FORM	*Form_Table;		/* Known forms             */
static PRINTER	*Printer_Table;		/* Known printers          */
static PWHEEL	*PrintWheel_Table;	/* Known print wheels      */

int		ST_Size,
		ST_Count;

/*
**
**	CLRMEM clears memory pointed to by <addr> over the range of
**	<addr>[<cnt> .. <size>].  <datum> is the size of an element of <addr>.
**
*/
# define	CLRMEM(addr, cnt, size, datum) \
                      (void) memset((char *)(addr + cnt), 0, \
		                    (int)((size - cnt ) * sizeof(datum)))

static void	init_systems(),
		init_printers(),
		init_classes(),
		init_forms(),
		init_pwheels(),
		init_exec();


static RSTATUS	*init_requests();

void
#if	defined(__STDC__)
init_memory (
	void
)
#else
init_memory()
#endif
{
    ENTRY ("init_memory")

    init_systems();
    init_printers();
    init_classes();
    init_forms();
    init_pwheels();
    init_exec();

    /*
     * Load the status after the basic structures have been loaded,
     * but before loading requests still in the queue. This is so
     * the requests can be compared against accurate status information
     * (except rejection status--accept the jobs anyway).
     */
    load_status();

    Loadfilters(Lp_A_Filters);

    Request_List = init_requests();
}

static void
init_printers()
{
    ENTRY ("init_printers")

    PRINTER	*p;
    PRINTER	*pt_pointer;
    int		pt_allocation;
    int		at_allocation;
    int		PT_Count;
    int		i;


    PT_Size = 10;
    PT_Count = 0;
    pt_allocation = PT_Size * sizeof(PRINTER);

    Printer_Table = (PRINTER *)Malloc(pt_allocation);

    CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);
    
    pt_pointer = Printer_Table;
    
    while((p = Getprinter(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*pt_pointer = *p;
	pt_pointer++;

	if (++PT_Count < PT_Size)
	    continue;
	
	PT_Size += 10;
	pt_allocation = PT_Size * sizeof(PRINTER);

	Printer_Table = (PRINTER *)Realloc(Printer_Table, pt_allocation);

	CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);

	pt_pointer = Printer_Table + PT_Count;
    }

    PT_Size = PT_Count + 40;

    pt_allocation = PT_Size * sizeof(PRINTER);

    Printer_Table = (PRINTER *)Realloc(Printer_Table, pt_allocation);

    CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);

    at_allocation = PT_Size * sizeof(ALERT);

    Alert_Table = (ALERT *)Malloc(at_allocation);

    CLRMEM(Alert_Table, 0, PT_Size, ALERT);

    pt_allocation = PT_Size * sizeof(PSTATUS);
    
    PStatus = (PSTATUS *)Malloc(pt_allocation);

    CLRMEM(PStatus, 0, PT_Size, PSTATUS);

    for (i = 0; i < PT_Size; i++)
    {
	char	buf[15];
	PSTATUS	*psp;
	
	psp = PStatus + i;
	p = psp->printer = Printer_Table + i;
	psp->alert = Alert_Table + i;
	sprintf(buf, "A-%d", i);
	Alert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(Alert_Table[i].msgfile);
	if (i < PT_Count)
	{
	    load_userprinter_access (
		    p->name,
		    &(psp->users_allowed),
		    &(psp->users_denied)
	    );
	    load_formprinter_access (
		    p->name,
		    &(psp->forms_allowed),
		    &(psp->forms_denied)
	    );
	    load_sdn (&(psp->cpi), p->cpi);
	    load_sdn (&(psp->lpi), p->lpi);
	    load_sdn (&(psp->plen), p->plen);
	    load_sdn (&(psp->pwid), p->pwid);

	    init_remote_printer (psp, p);

	}
    }
    
#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (PT_Count > 1)
	    note ("Loaded %d printers\n", PT_Count);
	else
	    if (PT_Count == 1)
		note ("Loaded 1 printer\n");
	    else
		note ("Loaded no printers\n");
    }
#endif
}

static void
init_classes()
{
    ENTRY ("init_classes")

    CLASS	*p;
    CLASS	*ct_pointer;
    int		ct_allocation;
    int		CT_Count;
    int		i;


    CT_Size = 10;
    CT_Count = 0;
    ct_allocation = CT_Size * sizeof(CLASS);

    Class_Table = (CLASS *)Malloc(ct_allocation);

    CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);
    
    ct_pointer = Class_Table;

    while((p = Getclass(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*ct_pointer = *p;
	ct_pointer++;

	if (++CT_Count < CT_Size)
	    continue;
	
	CT_Size += 10;
	ct_allocation = CT_Size * sizeof(CLASS);

	Class_Table = (CLASS *)Realloc(Class_Table, ct_allocation);

	CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);

	ct_pointer = Class_Table + CT_Count;

    }

    CT_Size = CT_Count + 40;

    ct_allocation = CT_Size * sizeof(CLASS);

    Class_Table = (CLASS *)Realloc(Class_Table, ct_allocation);

    CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);

    ct_allocation = CT_Size * sizeof(CSTATUS);
    
    CStatus = (CSTATUS *)Malloc(ct_allocation);

    CLRMEM(CStatus, 0, CT_Size, CSTATUS);

    for (i = 0; i < CT_Size; i++)
	CStatus[i].class = Class_Table + i;

#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (CT_Count > 1)
	    note ("Loaded %d classes\n", CT_Count);
	else
	    if (CT_Count == 1)
		note ("Loaded 1 class\n");
	    else
		note ("Loaded no classes\n");
    }
#endif
}

static void
init_forms()
{
    ENTRY ("init_forms")

    _FORM	*ft_pointer,
		*f;
    int		at_allocation;
    int		ft_allocation;
    int		FT_Count;
    int		i;


    FT_Size = 10;
    FT_Count = 0;
    ft_allocation = FT_Size * sizeof(_FORM);

    Form_Table = (_FORM *)Malloc(ft_allocation);

    CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);
    
    ft_pointer = Form_Table;

    while((f = Getform(NAME_ALL)) != NULL)
    {
	*(ft_pointer++) = *f;

	if (++FT_Count < FT_Size)
	    continue;
	
	FT_Size += 10;
	ft_allocation = FT_Size * sizeof(_FORM);

	Form_Table = (_FORM *)Realloc(Form_Table, ft_allocation);

	CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);

	ft_pointer = Form_Table + FT_Count;
    }

    FT_Size = FT_Count + 40;

    ft_allocation = FT_Size * sizeof(_FORM);

    Form_Table = (_FORM *)Realloc(Form_Table, ft_allocation);

    CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);

    at_allocation = FT_Size * sizeof(ALERT);

    FAlert_Table = (ALERT *)Malloc(at_allocation);

    CLRMEM(FAlert_Table, 0, FT_Size, ALERT);

    ft_allocation = FT_Size * sizeof(FSTATUS);
    
    FStatus = (FSTATUS *)Malloc(ft_allocation);

    CLRMEM(FStatus, 0, FT_Size, FSTATUS);

    for (i = 0; i < FT_Size; i++)
    {
	char	buf[15];
	
	FStatus[i].form = Form_Table + i;
	FStatus[i].alert = FAlert_Table + i;
	FStatus[i].trigger = Form_Table[i].alert.Q;
	sprintf(buf, "F-%d", i);
	FAlert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(FAlert_Table[i].msgfile);

	if (i < FT_Count)
	{
	    load_userform_access (
		    Form_Table[i].name,
		    &(FStatus[i].users_allowed),
		    &(FStatus[i].users_denied)
	    );
	    load_sdn (&(FStatus[i].cpi), Form_Table[i].cpi);
	    load_sdn (&(FStatus[i].lpi), Form_Table[i].lpi);
	    load_sdn (&(FStatus[i].plen), Form_Table[i].plen);
	    load_sdn (&(FStatus[i].pwid), Form_Table[i].pwid);
	}
    }
    
#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (FT_Count > 1)
	    note ("Loaded %d forms\n", FT_Count);
	else
	    if (FT_Count == 1)
		note ("Loaded 1 form\n");
	    else
		note ("Loaded no forms\n");
    }
#endif
}

static void
init_pwheels()
{
    ENTRY ("init_pwheels")

    PWHEEL	*pwt_pointer;
    PWHEEL	*p;
    int		at_allocation;
    int		pwt_allocation;
    int		PWT_Count;
    int		i;
    

    PWT_Count = 0;
    PWT_Size = 10;
    pwt_allocation = PWT_Size * sizeof(PWHEEL);

    PrintWheel_Table = (PWHEEL *)Malloc(pwt_allocation);

    CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);
    
    pwt_pointer = PrintWheel_Table;

    while((p = Getpwheel(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*pwt_pointer = *p;
	pwt_pointer++;

	if (++PWT_Count < PWT_Size)
	    continue;
	
	PWT_Size += 10;
	pwt_allocation = PWT_Size * sizeof(PWHEEL);

	PrintWheel_Table = (PWHEEL *)Realloc(PrintWheel_Table, pwt_allocation);

	CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);

	pwt_pointer = &PrintWheel_Table[PWT_Count];

    }

    PWT_Size = PWT_Count + 40;

    pwt_allocation = PWT_Size * sizeof(PWHEEL);

    PrintWheel_Table = (PWHEEL *)Realloc(PrintWheel_Table, pwt_allocation);

    CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);

    at_allocation = PWT_Size * sizeof(ALERT);

    PAlert_Table = (ALERT *)Malloc(at_allocation);

    CLRMEM(PAlert_Table, 0, PWT_Size, ALERT);

    pwt_allocation = PWT_Size * sizeof(PWSTATUS);
    
    PWStatus = (PWSTATUS *)Malloc(pwt_allocation);

    CLRMEM(PWStatus, 0, PWT_Size, PWSTATUS);

    for (i = 0; i < PWT_Size; i++)
    {
	char	buf[15];
	
	PWStatus[i].pwheel = PrintWheel_Table + i;
	PWStatus[i].trigger = PrintWheel_Table[i].alert.Q;
	PWStatus[i].alert = PAlert_Table + i;
	sprintf(buf, "P-%d", i);
	PAlert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(PAlert_Table[i].msgfile);
    }
    
#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (PWT_Count > 1)
	    note ("Loaded %d print wheels\n", PWT_Count);
	else
	    if (PWT_Count == 1)
		note ("Loaded 1 print wheel\n");
	    else
		note ("Loaded no print wheels\n");
    }
#endif
}

#if	defined(__STDC__)
static RSTATUS	* init_requests ( void )	/* funcdef */
#else
static RSTATUS	* init_requests ( )
#endif
{
    ENTRY ("init_requests")

    REQUEST		*r;
    RSTATUS		**table;
    RSTATUS		*rp = NULL;
    SECURE		*s;
    char		*name;
    char		*sysdir;
    char		*sysname;
    char		*reqfile = NULL;
    int			count;
    int			i;
    long		addr = -1;
    long		sysaddr = -1;
    unsigned long	size;

    size = 20;
    count = 0;
    

    table = (RSTATUS **)Malloc(size * sizeof(RSTATUS *));
    
    while((sysname = next_dir(Lp_Requests, &addr)) != NULL)
    {
	sysname = Strdup(sysname);
	
	sysdir = makepath(Lp_Requests, sysname, NULL);

	while((name = next_file(sysdir, &sysaddr)) != NULL)
	{
	    table[count] = allocr();
    
	    reqfile = makepath(sysname, name, NULL);	

	    if ((s = Getsecure(reqfile)) == NULL)
	    {
		rmfiles(table[count], 0);
		freerstatus(table[count]);
		Free(reqfile);
		continue;
	    }
	    *(table[count]->secure) = *s;

	    if((r = Getrequest(reqfile)) == NULL)
	    {
		rmfiles(table[count], 0);
		freerstatus(table[count]);
		Free(reqfile);
		continue;
	    }
	    r->outcome &= ~RS_ACTIVE;	/* it can't be! */
	    *(table[count]->request) = *r;

	    table[count]->req_file = reqfile;
	    reqfile = NULL;

	    if ((r->outcome & (RS_CANCELLED|RS_FAILED)) &&
		!(r->outcome & RS_NOTIFY))
	    {
		rmfiles(table[count], 0);
		freerstatus(table[count]);
		continue;
	    }

	    /*
	     * So far, the only way RS_NOTIFY can be set without there
	     * being a notification file containing the message to the
	     * user, is if the request was cancelled. This is because
	     * cancelling a request does not cause the creation of the
	     * message if the request is currently printing or filtering.
	     * (The message is created after the child process dies.)
	     * Thus, we know what to say.
	     *
	     * If this behaviour changes, we may have to find another way
	     * of determining what to say in the message.
	     */
	    if (r->outcome & RS_NOTIFY)
	    {
		char	*file = makereqerr(table[count]);

schedlog ("Old %s with RS_NOTIFY\n", s->req_id);
		if (Access(file, F_OK) == -1)
		{
schedlog ("No notification file (%s)\n", file);
		    if (!(r->outcome & RS_CANCELLED))
		    {
schedlog ("Defunct, removing request\n");
			Free(file);
			rmfiles(table[count], 0);
			freerstatus(table[count]);
			continue;
		    }
schedlog ("Calling notify\n");
		    notify(table[count], NULL, 0, 0, 0);
		}
		Free(file);
	    }

	    if (validate_request(table[count], NULL, 1) != MOK)
		cancel(table[count], 1);


	    if (++count < size)
		continue;

	    size += 20;

	    table = (RSTATUS **)Realloc((char *)table, size * sizeof(RSTATUS *));
	}
	Free(sysdir);
	Free(sysname);
	sysaddr = -1;
    }
    
    if (!count)
	Free ((char *)table);
    else
	if ((size = count) > 0)
	{
	    table = (RSTATUS **)Realloc((char *)table, size * sizeof(RSTATUS *));

#if	defined(__STDC__)
	    qsort((void *)table, size, sizeof(RSTATUS *), (int (*)(const void * , const void *))rsort);
#else
	    qsort((char *)table, size, sizeof(RSTATUS *), rsort);
#endif

	    for (i = 0; i < size - 1; i++)
	    {
		table[i]->next = table[i + 1];
		table[i + 1]->prev = table[i];
	    }

	    table[0]->prev = 0;
	    table[size - 1]->next = 0;

	    rp = *table;
	    Free(table);

	}

#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (count > 1)
	    note ("Loaded %d print requests\n", count);
	else
	    if (count == 1)
		note ("Loaded 1 print request\n");
	    else
		note ("Loaded no print requests\n");
    }
#endif

    return(rp);
}

static void
init_exec()
{
    ENTRY ("init_exec")

    EXEC	*et_pointer;
    int		et_allocation;
    int		i;

    ET_Size	= ET_SlowSize
		+ ET_NotifySize
    		+ PT_Size * 2	/* 1 for interface, 1 for alert */
		+ PWT_Size
		+ FT_Size;

    et_allocation = ET_Size * sizeof(EXEC);

    Exec_Table = (EXEC *)Malloc(et_allocation);
    
    CLRMEM(Exec_Table, 0, ET_Size, EXEC);

    et_pointer = Exec_Table;

    Exec_Slow = et_pointer;
    for (i = 0; i < ET_SlowSize; i++)
	(et_pointer++)->type = EX_SLOWF;

    Exec_Notify = et_pointer;
    for (i = 0; i < ET_NotifySize; i++)
	(et_pointer++)->type = EX_NOTIFY;

    for (i = 0; i < PT_Size; i++)
    {
	PStatus[i].exec = et_pointer;
	et_pointer->type = EX_INTERF;
	et_pointer->ex.printer = PStatus + i;
	et_pointer++;

	PStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_ALERT;
	et_pointer->ex.printer = PStatus + i;
	et_pointer++;
    }

    for (i = 0; i < PWT_Size; i++)
    {
	PWStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_PALERT;
	et_pointer->ex.pwheel = PWStatus + i;
	et_pointer++;
    }

    for (i = 0; i < FT_Size; i++)
    {
	FStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_FALERT;
	et_pointer->ex.form = FStatus + i;
	et_pointer++;
    }
    
}

#if	defined(__STDC__)
static void init_systems ( void )
#else
static void init_systems ( )
#endif
{
    ENTRY ("init_systems")

    SYSTEM		*s;
    SSTATUS		*ssp;


    ST_Size = 0;
    
    while ((s = Getsystem(NAME_ALL)) != NULL)
    {
	ssp = (SSTATUS *)Calloc(1, sizeof(SSTATUS));

	ssp->system = (SYSTEM *)Calloc(1, sizeof(SYSTEM));

	*(ssp->system) = *s;

	ssp->exec = (EXEC *)Calloc(1, sizeof(EXEC));

	addone((void ***)&SStatus, ssp, &ST_Size, &ST_Count);
    }

#if defined(DEBUG)
    if (debug & DB_INIT)
    {
	if (ST_Count)
		note ("Loaded %d system%s\n", ST_Count, (ST_Count > 1 ? "s" : ""));
	else
		note ("Loaded no systems\n");
    }
#endif
}

void
#if	defined(__STDC__)
init_remote_printer (
	PSTATUS *		psp,
	PRINTER *		p
)
#else
init_remote_printer (psp, p)
	PSTATUS		*psp;
	PRINTER		*p;
#endif
{
	ENTRY ("init_remote_printer")

	if (!p->remote) {
ItIsLocal:
		psp->system = 0;
		unload_str (&(psp->remote_name));
		psp->status &= ~PS_REMOTE;
		load_str (&(p->remote), Local_System);

	} else {
		char		*cp = strchr(p->remote, BANG_C);

		if (!cp) {
			if (STREQU(p->remote, Local_System))
				goto ItIsLocal;
			else {
				psp->status |= PS_REMOTE;
				load_str (&(psp->remote_name), p->name);
				psp->system = search_stable(p->remote);
			}
		} else {
			*cp++ = 0;
			if (STREQU(p->remote, Local_System))
				goto ItIsLocal;
			else {
				psp->status |= PS_REMOTE;
				load_str (&(psp->remote_name), cp);
				psp->system = search_stable(p->remote);

				/*
				 * Do this in the "else" part only, as
				 * the string where "cp" points is freed
				 * in the "if" part.
				 */
				cp[-1] = BANG_C;
			}
		}
	}
	return;
}

# define	ACLUSTERSIZE	10

#if	defined(__STDC__)
void addone ( void *** table, void * elem, int * tsize, int * tocc)
#else
void	addone ( table, elem, tsize, tocc)
void	***table;
void	*elem;
int	*tsize;
int	*tocc;
#endif
{
    ENTRY ("addone")

    if (*tsize == 0 || *table == NULL)
    {
	*tsize = ACLUSTERSIZE;
	*tocc = 0;
	*table = (void **)Malloc(*tsize * sizeof (void **));
    }

    if ((*tocc + 1) >= *tsize)
    {
	*tsize += ACLUSTERSIZE;
	*table = (void **)Realloc(*table, *tsize * sizeof(void **));
    }

    (*table)[(*tocc)++] = elem;
    (*table)[*tocc] = NULL;
}

#if	defined(__STDC__)
void delone ( void *** table, void * elem, int * tsize, int * tocc )
#else
void	delone ( table, elem, tsize, tocc)
void	***table;
void	*elem;
int	*tsize;
int	*tocc;
#endif
{
    ENTRY ("delone")

    int		i;

    if (*tsize <= 0 || *tocc <= 0 || *table == NULL)
	return;

    for (i = 0; i < *tocc; i++)
	if ((*table)[i] == elem)
	{
	    /*
	     * WARNING: If you change the way entries are removed
	     * you may have to fix up loops elsewhere. Look for
	     * DELONE DEPENDENT.
	     */
	    (*table)[i] = (*table)[*tocc - 1];
	    (*table)[*tocc - 1] = NULL;
	    (*tocc)--;
	}

    if ((*tsize - *tocc) > ACLUSTERSIZE)
    {
	*tsize -= ACLUSTERSIZE;
	*table = (void **)Realloc(*table, *tsize * sizeof(void **));
    }
}
