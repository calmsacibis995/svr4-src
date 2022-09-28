/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:name.c	1.14.3.1"

/*	Portions Copyright(c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/
/*
 * UNIX shell
 */

#include	"defs.h"

extern BOOL	chkid();
extern unsigned char	*simple();
extern int	mailchk;
static void	namwalk();

static void	set_builtins_path();
static int	patheq();

struct namnod ps2nod =
{
	(struct namnod *)NIL,
	&acctnod,
	(unsigned char *)ps2name
};
struct namnod cdpnod = 
{
	(struct namnod *)NIL,

#ifdef VPIX
	&dpathnod,
#else
	(struct namnod *)NIL,
#endif

	(unsigned char *)cdpname
};

#ifdef VPIX
struct namnod dpathnod =
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	(unsigned char *)dpathname
};
#endif

struct namnod pathnod =
{
	&mailpnod,
	(struct namnod *)NIL,
	(unsigned char *)pathname
};
struct namnod ifsnod =
{
	&homenod,
	&mailnod,
	(unsigned char *)ifsname
};
struct namnod ps1nod =
{
	&pathnod,
	&ps2nod,
	(unsigned char *)ps1name
};
struct namnod homenod =
{
	&cdpnod,
	(struct namnod *)NIL,
	(unsigned char *)homename
};
struct namnod mailnod =
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	(unsigned char *)mailname
};
struct namnod mchknod =
{
	&ifsnod,
	&ps1nod,
	(unsigned char *)mchkname
};
struct namnod acctnod =
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	(unsigned char *)acctname
};
struct namnod mailpnod =
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	(unsigned char *)mailpname
};


struct namnod *namep = &mchknod;

/* ========	variable and string handling	======== */

syslook(w, syswds, n)
	register unsigned char *w;
	register struct sysnod syswds[];
	int n;
{
	int	low;
	int	high;
	int	mid;
	register int cond;

	if (w == 0 || *w == 0)
		return(0);

	low = 0;
	high = n - 1;

	while (low <= high)
	{
		mid = (low + high) / 2;

		if ((cond = cf(w, syswds[mid].sysnam)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(syswds[mid].sysval);
	}
	return(0);
}

setlist(arg, xp)
register struct argnod *arg;
int	xp;
{
	if (flags & exportflg)
		xp |= N_EXPORT;

	while (arg)
	{
		register unsigned char *s = mactrim(arg->argval);
		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr)
		{
			prs(s);
			if (arg)
				blank();
			else
				newline();
		}
	}
}


setname(argi, xp)	/* does parameter assignments */
unsigned char	*argi;
int	xp;
{
	register unsigned char *argscan = argi;
	register struct namnod *n;

	if (letter(*argscan))
	{
		while (alphanum(*argscan))
			argscan++;

		if (*argscan == '=')
		{
			*argscan = 0;	/* make name a cohesive string */

			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM)
			{
				n->namenv = n->namval = argscan;
				if (n == &pathnod)
					set_builtins_path();
			}
			else
				assign(n, argscan);
			return;
		}
	}
}

replace(a, v)
register unsigned char	**a;
unsigned char	*v;
{
	free(*a);
	*a = make(v);
}

dfault(n, v)
struct namnod *n;
unsigned char	*v;
{
	if (n->namval == 0)
		assign(n, v);
}

assign(n, v)
struct namnod *n;
unsigned char	*v;
{
	if (n->namflg & N_RDONLY)
		failed(n->namid, wtfailed);

#ifndef RES
	
	else if (flags & rshflg)
	{
		if (n == &pathnod || eq(n->namid,"SHELL"))
			failed(n->namid, restricted);
	}
#endif

	else if (n->namflg & N_FUNCTN)
	{
		func_unhash(n->namid);
		freefunc(n);

		n->namenv = 0;
		n->namflg = N_DEFAULT;
	}

	if (n == &mchknod)
	{
		mailchk = stoi(v);
	}
		
	replace(&n->namval, v);
	attrib(n, N_ENVCHG);

	if (n == &pathnod)
	{
		zaphash();
		set_dotpath();
#ifdef VPIX
		set_vpixdir ();
#endif

		set_builtins_path();
		return;
	}

#ifdef VPIX
	if (n == &dpathnod) {
		set_vpixdir ();
		return;
	}
#endif
	
	if (flags & prompt)
	{
		if ((n == &mailpnod) || (n == &mailnod && mailpnod.namflg == N_DEFAULT))
			setmail(n->namval);
	}
}

static void
set_builtins_path()
{
        register unsigned char *path;

        ucb_builtins = 0;
        path = getpath("");
        while (path && *path)
        {
                if (patheq(path, "/usr/ucb"))
                {
                        ucb_builtins++;
                        break;
                }
                else if (patheq(path, "/usr/bin"))
                        break;
                path = nextpath(path);
        }
}
 
static int
patheq(component, dir)
register unsigned char   *component;
register char   *dir;
{
        register unsigned char   c;
 
        for (;;)
        {
                c = *component++;
                if (c == COLON)
                        c = '\0';       /* end of component of path */
                if (c != *dir++)
                        return(0);
                if (c == '\0')
                        return(1);
        }
}
 
readvar(names)
unsigned char	**names;
{
	struct fileblk	fb;
	register struct fileblk *f = &fb;
	unsigned char	c[MULTI_BYTE_MAX+1];
	register int	rc = 0;
	struct namnod *n = lookup(*names++);	/* done now to avoid storage mess */
	unsigned char	*rel = (unsigned char *)relstak();
	unsigned char *oldstak;
	register unsigned char *pc, *rest, d;

	push(f);
	initf(dup(0));

	if (lseek(0, 0L, 1) == -1)
		f->fsiz = 1;

	/*
	 * strip leading IFS characters
	 */
	for (;;) 
	{
		d = nextc();
		if(eolchar(d))
			break;
		rest = readw(d);
		pc = c;
		while(*pc++ = *rest++);
		if(!anys(c, ifsnod.namval))
			break;
	}
	
	oldstak = curstak();
	for (;;)
	{
		if ((*names && anys(c, ifsnod.namval)) || eolchar(d))
		{
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names)
				n = lookup(*names++);
			else
				n = 0;
			if (eolchar(d))
			{
				break;
			}
			else		/* strip imbedded IFS characters */
				while(1) {
					d = nextc();
					if(eolchar(d))
						break;
					rest = readw(d);
					pc = c;
					while(*pc++ = *rest++);
					if(!anys(c, ifsnod.namval))
						break;
				}
		}
		else
		{
			if(d == '\\') {
				d = readc();
				rest = readw(d);
				while(d = *rest++)
					pushstak(d);
				oldstak = staktop;
			}
			else
			{
				pc = c;
				while(d = *pc++) 
					pushstak(d);
				if(!anys(c, ifsnod.namval))
					oldstak = staktop;
			}
			d = nextc();

			if (eolchar(d))
				staktop = oldstak;
			else 
			{
				rest = readw(d);
				pc = c;
				while(*pc++ = *rest++);
			}
		}
	}
	while (n)
	{
		assign(n, nullstr);
		if (*names)
			n = lookup(*names++);
		else
			n = 0;
	}

	if (eof)
		rc = 1;
	lseek(0, (long)(f->fnxt - f->fend), 1);
	pop();
	return(rc);
}

assnum(p, i)
unsigned char	**p;
long i;
{
	int j = ltos(i);
	replace(p, &numbuf[j]);
}

unsigned char *
make(v)
unsigned char	*v;
{
	register unsigned char	*p;

	if (v)
	{

		movstr(v, p = (unsigned char *) alloc(length(v)));
		return(p);
	}
	else
		return( 0);
}


struct namnod *
lookup(nam)
	register unsigned char	*nam;
{
	register struct namnod *nscan = namep;
	register struct namnod **prev;
	int		LR;

	if (!chkid(nam))
		failed(nam, notid);
	
	while (nscan)
	{
		if ((LR = cf(nam, nscan->namid)) == 0)
			return(nscan);

		else if (LR < 0)
			prev = &(nscan->namlft);
		else
			prev = &(nscan->namrgt);
		nscan = *prev;
	}
	/*
	 * add name node
	 */
	nscan = (struct namnod *)alloc(sizeof *nscan);
	nscan->namlft = nscan->namrgt = (struct namnod *)NIL;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;

	return(*prev = nscan);
}

BOOL
chkid(nam)
unsigned char	*nam;
{
	register unsigned char *cp = nam;

	if (!letter(*cp))
		return(FALSE);
	else
	{
		while (*++cp)
		{
			if (!alphanum(*cp))
				return(FALSE);
		}
	}
	return(TRUE);
}

static int (*namfn)();
namscan(fn)
	int	(*fn)();
{
	namfn = fn;
	namwalk(namep);
}

static void
namwalk(np)
register struct namnod *np;
{
	if (np)
	{
		namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	}
}

printnam(n)
struct namnod *n;
{
	register unsigned char	*s;

	sigchk();

	if (n->namflg & N_FUNCTN)
	{
		prs_buff(n->namid);
		prs_buff("(){\n");
		prf(n->namenv);
		prs_buff("\n}\n");
	}
	else if (s = n->namval)
	{
		prs_buff(n->namid);
		prc_buff('=');
		prs_buff(s);
		prc_buff(NL);
	}
}

static unsigned char *
staknam(n)
register struct namnod *n;
{
	register unsigned char	*p;

	p = movstr(n->namid, staktop);
	p = movstr("=", p);
	p = movstr(n->namval, p);
	return(getstak(p + 1 - stakbot));
}

static int namec;

exname(n)
	register struct namnod *n;
{
	register int 	flg = n->namflg;

	if (flg & N_ENVCHG)
	{

		if (flg & N_EXPORT)
		{
			free(n->namenv);
			n->namenv = make(n->namval);
		}
		else
		{
			free(n->namval);
			n->namval = make(n->namenv);
		}
	}

	
	if (!(flg & N_FUNCTN))
		n->namflg = N_DEFAULT;

	if (n->namval)
		namec++;

}

printro(n)
register struct namnod *n;
{
	if (n->namflg & N_RDONLY)
	{
		prs_buff(readonly);
		prc_buff(SP);
		prs_buff(n->namid);
		prc_buff(NL);
	}
}

printexp(n)
register struct namnod *n;
{
	if (n->namflg & N_EXPORT)
	{
		prs_buff(export);
		prc_buff(SP);
		prs_buff(n->namid);
		prc_buff(NL);
	}
}

setup_env()
{
	register unsigned char **e = environ;

	while (*e)
		setname(*e++, N_ENVNAM);
}


static unsigned char **argnam;

pushnam(n)
struct namnod *n;
{
	if (n->namval)
		*argnam++ = staknam(n);
}

unsigned char **
setenv()
{
	register unsigned char	**er;

	namec = 0;
	namscan(exname);

	argnam = er = (unsigned char **)getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return(er);
}

struct namnod *
findnam(nam)
	register unsigned char	*nam;
{
	register struct namnod *nscan = namep;
	int		LR;

	if (!chkid(nam))
		return(0);
	while (nscan)
	{
		if ((LR = cf(nam, nscan->namid)) == 0)
			return(nscan);
		else if (LR < 0)
			nscan = nscan->namlft;
		else
			nscan = nscan->namrgt;
	}
	return(0); 
}


unset_name(name)
	register unsigned char 	*name;
{
	register struct namnod	*n;

	if (n = findnam(name))
	{
		if (n->namflg & N_RDONLY)
			failed(name, wtfailed);

		if (n == &pathnod ||
		    n == &ifsnod ||
		    n == &ps1nod ||
		    n == &ps2nod ||
		    n == &mchknod)
		{
			failed(name, badunset);
		}

#ifndef RES

		if ((flags & rshflg) && eq(name, "SHELL"))
			failed(name, restricted);

#endif

		if (n->namflg & N_FUNCTN)
		{
			func_unhash(name);
			freefunc(n);
		}
		else
		{
			free(n->namval);
			free(n->namenv);
		}

		n->namval = n->namenv = 0;
		n->namflg = N_DEFAULT;

		if (flags & prompt)
		{
			if (n == &mailpnod)
				setmail(mailnod.namval);
			else if (n == &mailnod && mailpnod.namflg == N_DEFAULT)
				setmail(0);
		}
	}
}
