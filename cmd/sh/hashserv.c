/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:hashserv.c	1.10.8.1"

/* 	Portions Copyright(c) 1988, Sun Microsystems, Inc.      */
/* 	All Rights Reserved.                                    */
/*
 *	UNIX shell
 */

#include	"hash.h"
#include	"defs.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

#define		EXECUTE		01

static unsigned char	cost;
static int	dotpath;
static int	multrel;
static struct entry	relcmd;

static int	argpath();

short
pathlook(com, flg, arg)
	unsigned char	*com;
	int		flg;
	register struct argnod	*arg;
{
	register unsigned char	*name = com;
	register ENTRY	*h;

	ENTRY		hentry;
	int		count = 0;
	int		i;
	int		pathset = 0;
	int		oldpath = 0;
	struct namnod	*n;



	hentry.data = 0;

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	
	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);

		if ((h->data & DOT_COMMAND) == DOT_COMMAND)
		{
			if (multrel == 0 && hashdata(h->data) > dotpath)

#ifdef VPIX
				/* need to tell findpath whether old
				 * program was DOS.
				 */
				oldpath = hashdata(h->data) |
						(h->data & DOSFIELD);
#else

				oldpath = hashdata(h->data);
#endif

			else
				oldpath = dotpath;

			h->data = 0;
			goto pathsrch;
		}

		if (h->data & (COMMAND | REL_COMMAND))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		h->data = 0;
		h->cost = 0;
	}

	if (i = syslook(name, commands, no_commands))
	{
		hentry.data = (BUILTIN | i);
		count = 1;
	}
	else
	{
		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);
pathsrch:
			count = findpath(name, oldpath);
	}

	if (count > 0)
	{
		if (h == 0)
		{
			hentry.cost = 0;
			hentry.key = make(name);
			h = henter(hentry);
		}

		if (h->data == 0)
		{

#ifdef VPIX
			/* count includes DOSFIELD bits */
			if (hashdata(count) < dotpath)
#else
			if (count < dotpath)
#endif

				h->data = COMMAND | count;
			else
			{
				h->data = REL_COMMAND | count;
				h->next = relcmd.next;
				relcmd.next = h;
			}
		}


		h->hits = flg;
		h->cost += cost;
		return(h->data);
	}
	else 
	{
		return(-count);
	}
}
			

static void
zapentry(h)
	ENTRY *h;
{
	h->data &= HASHZAP;
}

void
zaphash()
{
	hscan(zapentry);
	relcmd.next = 0;
}

void 
zapcd()
{
	ENTRY *ptr = relcmd.next;
	
	while (ptr)
	{
		ptr->data |= CDMARK;
		ptr = ptr->next;
	}
	relcmd.next = 0;
}


static void
hashout(h)
	ENTRY *h;
{
	sigchk();

	if (hashtype(h->data) == NOTFOUND)
		return;

	if (h->data & (BUILTIN | FUNCTION))
		return;

	prn_buff(h->hits);

	if (h->data & REL_COMMAND)
		prc_buff('*');


	prc_buff(TAB);
	prn_buff(h->cost);
	prc_buff(TAB);

	pr_path(h->key, hashdata(h->data));
	prc_buff(NL);
}

void
hashpr()
{
	prs_buff("hits	cost	command\n");
	hscan(hashout);
}


set_dotpath()
{
	register unsigned char	*path;
	register int	cnt = 1;

	dotpath = 10000;
	path = getpath("");

	while (path && *path)
	{
		if (*path == '/')
			cnt++;
		else
		{
			if (dotpath == 10000)
				dotpath = cnt;
			else
			{
				multrel = 1;
				return;
			}
		}
	
		path = nextpath(path);
	}

	multrel = 0;
}


hash_func(name)
	unsigned char *name;
{
	ENTRY	*h;
	ENTRY	hentry;

	h = hfind(name);

	if (h)
		h->data = FUNCTION;
	else
	{
		hentry.data = FUNCTION;
		hentry.key = make(name);
		hentry.cost = 0;
		hentry.hits = 0;
		henter(hentry);
	}
}

func_unhash(name)
	unsigned char *name;
{
	ENTRY 	*h;
	int i;

	h = hfind(name);

	if (h && (h->data & FUNCTION)) {
		if(i = syslook(name, commands, no_commands))
			h->data = (BUILTIN|i);
		else
			h->data = NOTFOUND;
	}
}


short
hash_cmd(name)
	unsigned char *name;
{
	ENTRY	*h;

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
			return(h->data);
		else if ((h->data & REL_COMMAND) == REL_COMMAND)
		{ /* unlink h from relative command list */
			ENTRY *ptr = &relcmd;
			while(ptr-> next != h)
				ptr = ptr->next;
			ptr->next = h->next;
		}
		zapentry(h);
	}

	return(pathlook(name, 0, 0));
}


what_is_path(name)
	register unsigned char *name;
{
	register ENTRY	*h;
	int		cnt;
	short	hashval;

#ifdef VPIX
	short dosval;
#endif

	h = hfind(name);

	prs_buff(name);
	if (h)
	{
		hashval = hashdata(h->data);

#ifdef VPIX
		dosval = h->data & DOSFIELD;
#endif

		switch (hashtype(h->data))
		{
			case BUILTIN:
				prs_buff(" is a shell builtin\n");
				return;
	
			case FUNCTION:
			{
				struct namnod *n = lookup(name);

				prs_buff(" is a function\n");
				prs_buff(name);
				prs_buff("(){\n");
				prf(n->namenv);
				prs_buff("\n}\n");
				return;
			}
	
			case REL_COMMAND:
			{
				short hash;

				if ((h->data & DOT_COMMAND) == DOT_COMMAND)
				{
					hash = pathlook(name, 0, 0);
					if (hashtype(hash) == NOTFOUND)
					{
						prs_buff(" not found\n");
						return;
					}
					else
#ifdef VPIX				
					{
						hashval = hashdata(hash);
						dosval = hash & DOSFIELD;
					}
#else
						hashval = hashdata(hash);
#endif

				}
			}

			case COMMAND:					
				prs_buff(" is hashed (");
				pr_path(name, hashval);

#ifdef VPIX
				switch (dosval) { 
				case DOSDOTCOM:
					prs_buff (dotcom);
					break;
				case DOSDOTEXE:
					prs_buff (dotexe);
					break;
				case DOSDOTBAT:
					prs_buff (dotbat);
					break;
				}
#endif

				prs_buff(")\n");
				return;
		}
	}

	if (syslook(name, commands, no_commands))
	{
		prs_buff(" is a shell builtin\n");
		return;
	}

	if ((cnt = findpath(name, 0)) > 0)
	{
		prs_buff(" is ");

#ifdef VPIX
		pr_path(name, hashdata(cnt));
		switch (cnt & DOSFIELD) { 
		case DOSDOTCOM:
			prs_buff (dotcom);
			break;
		case DOSDOTEXE:
			prs_buff (dotexe);
			break;
		case DOSDOTBAT:
			prs_buff (dotbat);
			break;
		}
#else
		pr_path(name, cnt);
#endif

		prc_buff(NL);
	}
	else
		prs_buff(" not found\n");
}



findpath(name, oldpath)
	register unsigned char *name;
	int oldpath;
{
	register unsigned char 	*path;
	register int	count = 1;

	unsigned char	*p;
	int	ok = 1;
	int 	e_code = 1;
	
	cost = 0;
	path = getpath(name);

#ifdef VPIX
	/* oldpath includes DOSFIELD bits */
	if (hashdata(oldpath))
#else
	if (oldpath)
#endif

	{
		count = dotpath;
		while (--count)
			path = nextpath(path);


#ifdef VPIX
		if (hashdata(oldpath) > dotpath)
#else
		if (oldpath > dotpath)
#endif

		{

#ifdef VPIX
			p = curstak();
			cost = 1;

			/* if this path is in DOSPATH as well as PATH, first
			 * look for DOS executables.  If found, change the
			 * count to reflect the file type.
			 */
			if (dospath (path))
			{       catpath (path, name, dotcom);
				if ((ok = chk_access(p, S_IREAD, 1)) == 0)
					return (dotpath | DOSDOTCOM);
				catpath (path, name, dotexe);
				if ((ok = chk_access(p, S_IREAD, 1)) == 0)
					return (dotpath | DOSDOTEXE);
				catpath (path, name, dotbat);
				if ((ok = chk_access(p, S_IREAD, 1)) == 0)
					return (dotpath | DOSDOTBAT);
			}

			catpath(path, name, (char *)0);
#else
			catpath(path, name);
			p = curstak();
			cost = 1;
#endif

			if ((ok = chk_access(p, S_IEXEC, 1)) == 0)
				return(dotpath);
			else
				return(oldpath);
		}
		else 
			count = dotpath;
	}

	while (path)
	{

#ifdef VPIX
		cost++;
		p = curstak();
		/* if this path is in DOSPATH as well as PATH, first
		 * look for DOS executables.  If found, change the
		 * count to reflect the file type.
		 */
		if (dospath (path))
		{       catpath (path, name, dotcom);
			if ((ok = chk_access(p, S_IREAD, 1)) == 0)
			{       count |= DOSDOTCOM;
				break;
			}
			catpath (path, name, dotexe);
			if ((ok = chk_access(p, S_IREAD, 1)) == 0)
			{       count |= DOSDOTEXE;
				break;
			}
			catpath (path, name, dotbat);
			if ((ok = chk_access(p, S_IREAD, 1)) == 0)
			{       count |= DOSDOTBAT;
				break;
			}
		}

		path = catpath(path, name, (char *)0);
#else
		path = catpath(path, name);
		cost++;
		p = curstak();
#endif
		if ((ok = chk_access(p, S_IEXEC, 1)) == 0)
			break;
		else
			e_code = max(e_code, ok);

		count++;
	}

	return(ok ? -e_code : count);
}

/*
 * Determine if file given by name is accessible with permissions
 * given by mode.
 * Regflag argument non-zero means not to consider 
 * a non-regular file as executable. 
 */

chk_access(name, mode, regflag)
register unsigned char	*name;
mode_t mode;
int regflag;
{	
	static int flag;
	static uid_t euid; 
	struct stat statb;
	mode_t ftype;
	
	if(flag == 0) {
		euid = geteuid();
		flag = 1;
	}
	ftype = statb.st_mode & S_IFMT;
	if (stat((char *)name, &statb) == 0) {
		ftype = statb.st_mode & S_IFMT;
		if(mode == S_IEXEC && regflag && ftype != S_IFREG)
			return(2);
		if(access((char *)name, 010|(mode>>6)) == 0) {
			if(euid == 0) {
				if (ftype != S_IFREG || mode != S_IEXEC)
					return(0);
		    		/* root can execute file as long as it has execute 
			   	permission for someone */
				if (statb.st_mode & (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6)))
					return(0);
				return(3);
			}
			return(0);
		}
	}
	return(errno == EACCES ? 3 : 1);
}


pr_path(name, count)
	register unsigned char	*name;
	int count;
{
	register unsigned char	*path;

	path = getpath(name);

	while (--count && path)
		path = nextpath(path, name);

#ifdef VPIX
	catpath(path, name, (char *)0);
#else
	catpath(path,name);
#endif

	prs_buff(curstak());
}


static
argpath(arg)
	register struct argnod	*arg;
{
	register unsigned char 	*s;
	register unsigned char	*start;

	while (arg)
	{
		s = arg->argval;
		start = s;

		if (letter(*s))		
		{
			while (alphanum(*s))
				s++;

			if (*s == '=')
			{
				*s = 0;

				if (eq(start, pathname))
				{
					*s = '=';
					return(1);
				}
				else
					*s = '=';
			}
		}
		arg = arg->argnxt;
	}

	return(0);
}


#ifdef VPIX
/* Determine where the vpix program is */
set_vpixdir()
{       register int count;
	register unsigned char *path;

	if (vpixdirname) free (vpixdirname);
	vpixdirname = 0;
	count = pathlook (vpix, 0, (struct argnod *)0);
	count = hashdata (count);
	if (count <= 0) return;
	path = getpath (vpix);
	while (count > 1)
	{
		count--;
		path = nextpath(path);
	}
	if (path) vpixdirname = make (path);
}
#endif
