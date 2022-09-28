/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:service.c	1.22.10.2"

/*
 * UNIX shell
 */

#ifdef VPIX
#include	"hash.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#endif

#include	"defs.h"
#include	<errno.h>
#include	<fcntl.h>

#define ARGMK	01

static unsigned char	*execs();
static int	gsort();
static int	split();

#ifdef __STDC__
extern const char	*sysmsg[];
#else
extern char	*sysmsg[];
#endif

extern short topfd;



/*
 * service routines for `execute'
 */
initio(iop, save)
	struct ionod	*iop;
	int		save;
{
	register unsigned char	*ion;
	register int	iof, fd;
	int		ioufd;
	short	lastfd;

	lastfd = topfd;
	while (iop)
	{
		iof = iop->iofile;
		ion = mactrim(iop->ioname);
		ioufd = iof & IOUFD;

		if (*ion && (flags&noexec) == 0)
		{
			if (save)
			{
				fdmap[topfd].org_fd = ioufd;
				fdmap[topfd++].dup_fd = savefd(ioufd);
			}

			if (iof & IODOC)
			{
				struct tempblk tb;

				subst(chkopen(ion), (fd = tmpfil(&tb)));

				poptemp();	/* pushed in tmpfil() --
						   bug fix for problem with
						   in-line scripts
						*/

				fd = chkopen(tmpout);
				unlink(tmpout);
			}
			else if (iof & IOMOV)
			{
				if (eq(minus, ion))
				{
					fd = -1;
					close(ioufd);
				}
				else if ((fd = stoi(ion)) >= USERIO)
					failed(ion, badfile);
				else
					fd = dup(fd);
			}
			else if ((iof & IOPUT) == 0)
				fd = chkopen(ion);
			else if (flags & rshflg)
				failed(ion, restricted);
			else if (iof & IOAPP && (fd = open((char *)ion, 1)) >= 0)
				lseek(fd, 0L, 2);
			else
				fd = create(ion);
			if (fd >= 0)
				rename(fd, ioufd);
		}

		iop = iop->ionxt;
	}
	return(lastfd);
}

unsigned char *
simple(s)
unsigned char	*s;
{
	unsigned char	*sname;

	sname = s;
	while (1)
	{
		if (any('/', sname))
			while (*sname++ != '/')
				;
		else
			return(sname);
	}
}

unsigned char *
getpath(s)
	unsigned char	*s;
{
	register unsigned char	*path, *newpath;
	register int pathlen;	
	
	if (any('/', s))
	{
		if (flags & rshflg)
			failed(s, restricted);
		else
			return((unsigned char *)nullstr);
	}
	else if ((path = pathnod.namval) == 0)
		return((unsigned char *)defpath);
	else {
		pathlen = length(path)-1;
		/* Add extra ':' if PATH variable ends in ':' */
		if(pathlen > 2 && path[pathlen - 1] == ':' && path[pathlen - 2] != ':') {
			newpath = locstak();
			(void) memcpy(newpath, path, pathlen);
			newpath[pathlen] = ':';
			endstak(newpath + pathlen + 1);
			return(newpath);
		} else
			return(cpystak(path));
	}
}

pathopen(path, name)
register unsigned char *path, *name;
{
	register int	f;

	do
	{

#ifdef VPIX
		path = catpath(path, name, (char *)0);
#else
		path = catpath(path, name);
#endif

	} while ((f = open((char *)curstak(), 0)) < 0 && path);
	return(f);
}

unsigned char *

#ifdef VPIX
catpath(path, name, ext)
unsigned char *ext;
#else
catpath(path, name)
#endif

register unsigned char	*path;
unsigned char	*name;
{
	/*
	 * leaves result on top of stack
	 */
	register unsigned char	*scanp = path;
	register unsigned char	*argp = locstak();

	while (*scanp && *scanp != COLON)
		*argp++ = *scanp++;
	if (scanp != path) 
		*argp++ = '/';
	if (*scanp == COLON)
		scanp++;
	path = (*scanp ? scanp : 0);
	scanp = name;
	while ((*argp++ = *scanp++))
		;

#ifdef VPIX
	/* append DOS filename extension if asked */
	if ((scanp = ext)) {
		argp--;
		while (*argp++ = *scanp++)
			;
	}
#endif

	return(path);
}

unsigned char *
nextpath(path)
	register unsigned char	*path;
{
	register unsigned char	*scanp = path;

	while (*scanp && *scanp != COLON)
		scanp++;

	if (*scanp == COLON)
		scanp++;

	return(*scanp ? scanp : 0);
}


#ifdef VPIX
/* see if path is in DOSPATH */
dospath(path)
	char *path;
{
	register char   *scanp;
	register unsigned char *scandp;

	if (path == 0) return 0;
	if ((scandp = dpathnod.namval) == 0) return 0;
	for (;;)
	{       scanp = path;
		for (;;)
		{       if ((*scanp == 0 || *scanp == COLON) &&
					(*scandp == 0 || *scandp == COLON))
				return 1;
			if (*scanp != *scandp) break;
			scanp++;
			scandp++;
		}
		while (*scandp && *scandp != COLON) scandp++;
		if (*scandp == 0) return 0;
		scandp++;        /* scanp points after COLON */
	}
}

unsigned char **
dosargs(args, path, dosext)
	unsigned char    **args;
	char    *path;
	char    *dosext;
{       int     i;
	unsigned char    **answer;
	unsigned char    *p;
	int	ok;

	/* determine how big the args array is (including null) */
	for (i = 0; args[i++];)
		;
	i += 2;         /* we will add two more */
	answer = (unsigned char **)alloc (i * sizeof path);
	/* check for out of space.  Unlikely to happen, so handle it
	 * just well enough to prevent disaster */
	if (answer == 0) return args;
	answer[0] = (unsigned char *)vpix;
	answer[1] = (unsigned char *)vpixflag;
	answer[2] = alloc (length (path) +
			length (args[0]) + length (dosext) + 2);
	/* again merely stave off disaster if out of space */
	if (answer[2] == 0) answer[2] = args[0];
	else
	{       p = answer[2];
		while (*path && *path != COLON) *p++ = *path++;
		if (p != answer[2]) *p++ = '/';
		movstr (dosext, movstr (args[0], p));
	}
	i--;
	while (--i > 0) answer[i + 2] = args[i];

	ok = chk_access(answer[2], S_IEXEC, 1);
	if (ok == 3)
		failed (answer[2], badexec);

	return answer;
}
#endif


static unsigned char	*xecmsg;
static unsigned char	**xecenv;

int	execa(at, pos)
	unsigned char	*at[];
	short pos;
{
	register unsigned char	*path;
	register unsigned char	**t = at;
	int		cnt;

#ifdef VPIX
	short		dosval;
	char		*dosext;
	unsigned char	**dost;
	unsigned char	*dospath;	
#endif 

	if ((flags & noexec) == 0)
	{
		xecmsg = (unsigned char *)notfound;
		path = getpath(*t);

#ifdef VPIX
		dosval = (pos < 0) ? 0 : (pos & DOSFIELD);
		pos = hashdata(pos);
		dospath = vpixdirname;
#endif

		xecenv = setenv();

		if (pos > 0)
		{
			cnt = 1;
			while (cnt != pos)
			{
				++cnt;
				path = nextpath(path);
			}

#ifdef VPIX
			switch (dosval) { 
			case DOSDOTCOM:
				dosext = dotcom;
				break;
			case DOSDOTEXE:
				dosext = dotexe;
				break;
			case DOSDOTBAT:
				dosext = dotbat;
				break;
			default:
				dosext = 0;
				break;
			}
			if (dosext == 0)
			{       dost = t;
				dospath = path;
			}
			else dost = (unsigned char **)dosargs (t, path, dosext);
			execs(dospath, dost);
#else
			execs(path, t);
#endif

			path = getpath(*t);
		}
		while (path = execs(path,t))
			;
		failed(*t, xecmsg);
	}
}

static unsigned char *
execs(ap, t)
unsigned char	*ap;
register unsigned char	*t[];
{
	register unsigned char *p, *prefix;

#ifdef VPIX
	prefix = catpath(ap, t[0], (char *)0);
#else
	prefix = catpath(ap, t[0]);
#endif

	trim(p = curstak());
	sigchk();
	
	execve(p, &t[0] ,xecenv);
	switch (errno)
	{
	case ENOEXEC:		/* could be a shell script */
		funcnt = 0;
		flags = 0;
		*flagadr = 0;
		comdiv = 0;
		ioset = 0;
		clearup();	/* remove open files and for loop junk */
		if (input)
			close(input);
		input = chkopen(p);
	
#ifdef ACCT
		preacct(p);	/* reset accounting */
#endif

		/*
		 * set up new args
		 */
		
		setargs(t);
		longjmp(subshell, 1);

	case ENOMEM:
		failed(p, toobig);

	case E2BIG:
		failed(p, arglist);

	case ETXTBSY:
		failed(p, txtbsy);

	case ELIBACC:
		failed(p, libacc);

	case ELIBBAD:
		failed(p, libbad);

	case ELIBSCN:
		failed(p, libscn);

	case ELIBMAX:
		failed(p, libmax);

	default:
		xecmsg = (unsigned char *)badexec;
	case ENOENT:
		return(prefix);
	}
}

BOOL		nosubst;

trim(at)
unsigned char	*at;
{
	register unsigned char	*last;
	register unsigned char 	*current;
	register unsigned char	c;

	nosubst = 0;
	if (current = at)
	{
		last = at;
		while (c = *current++)
		{
			if(c == '\\')  { /* remove \ and quoted nulls */
				nosubst = 1;
				if(c = *current++)
					*last++ = c;
			} else
				*last++ = c;
		}

		*last = 0;
	}
}

/* Same as trim, but only removes backlashes before slashes */
trims(at)
unsigned char	*at;
{
	register unsigned char	*last;
	register unsigned char 	*current;
	register unsigned char	c;

	if (current = at)
	{
		last = at;
		while (c = *current++)
		{
			if(c == '\\')  { /* remove \ and quoted nulls */
				if((c = *current++)=='/')
					*last++ = c;
				else if(c) {
					*last++ = '\\';
					*last++ = c;
				}
			} else
				*last++ = c;
		}
		*last = 0;
	}
}

unsigned char *
mactrim(s)
unsigned char	*s;
{
	register unsigned char	*t = macro(s);

	trim(t);
	return(t);
}

unsigned char **
scan(argn)
int	argn;
{
	register struct argnod *argp = (struct argnod *)(Rcheat(gchain) & ~ARGMK);
	register unsigned char **comargn, **comargm;

	comargn = (unsigned char **)getstak(BYTESPERWORD * argn + BYTESPERWORD);
	comargm = comargn += argn;
	*comargn = ENDARGS;
	while (argp)
	{
		*--comargn = argp->argval;

		trim(*comargn);
		argp = argp->argnxt;

		if (argp == 0 || Rcheat(argp) & ARGMK)
		{
			gsort(comargn, comargm);
			comargm = comargn;
		}
		argp = (struct argnod *)(Rcheat(argp) & ~ARGMK);
	}
	return(comargn);
}

static int
gsort(from, to)
unsigned char	*from[], *to[];
{
	int	k, m, n;
	register int	i, j;

	if ((n = to - from) <= 1)
		return;
	for (j = 1; j <= n; j *= 2)
		;
	for (m = 2 * j - 1; m /= 2; )
	{
		k = n - m;
		for (j = 0; j < k; j++)
		{
			for (i = j; i >= 0; i -= m)
			{
				register unsigned char **fromi;

				fromi = &from[i];
				if (cf(fromi[m], fromi[0]) > 0)
				{
					break;
				}
				else
				{
					unsigned char *s;

					s = fromi[m];
					fromi[m] = fromi[0];
					fromi[0] = s;
				}
			}
		}
	}
}

/*
 * Argument list generation
 */
getarg(ac)
struct comnod	*ac;
{
	register struct argnod	*argp;
	register int		count = 0;
	register struct comnod	*c;

	if (c = ac)
	{
		argp = c->comarg;
		while (argp)
		{
			count += split(macro(argp->argval),1);
			argp = argp->argnxt;
		}
	}
	return(count);
}

static int
split(s)		/* blank interpretation routine */
unsigned char	*s;
{
	register unsigned char	*argp;
	register int	c;
	int		count = 0;
	for (;;)
	{
		register int length;
		sigchk();
		argp = locstak() + BYTESPERWORD;
		while (c = *s) { 
			wchar_t l;
			length = mbtowc(&l, (char *)s, MULTI_BYTE_MAX);
			if(c == '\\') { /* skip over quoted characters */
				*argp++ = c;
				s++;
				/* get rest of multibyte character */
				length = mbtowc(&l, (char *)s, MULTI_BYTE_MAX);
				*argp++ = *s++;
				while(--length > 0)
					*argp++ = *s++;
			} else if (anys(s, ifsnod.namval)) {
			/* skip to next character position */
				s += length;
				break;
			} else {
				*argp++ = c;
				s++;
				while(--length > 0)
					*argp++ = *s++;
			}
		}
		if (argp == staktop + BYTESPERWORD)
		{
			if (c)
			{
				continue;
			}
			else
			{
				return(count);
			}
		}
		
		/*
		 * file name generation
		 */

		argp = endstak(argp);
		trims(((struct argnod *)argp)->argval);
		if ((flags & nofngflg) == 0 && 
			(c = expand(((struct argnod *)argp)->argval, 0)))
			count += c;
		else
		{
			makearg(argp);
			count++;
		}
		gchain = (struct argnod *)((int)gchain | ARGMK);
	}
}

#ifdef ACCT
#include	<sys/types.h>
#include	<sys/acct.h>
#include 	<sys/times.h>

struct acct sabuf;
struct tms buffer;
static clock_t before;
static int shaccton;	/* 0 implies do not write record on exit
			   1 implies write acct record on exit
			*/


/*
 *	suspend accounting until turned on by preacct()
 */

suspacct()
{
	shaccton = 0;
}

preacct(cmdadr)
	unsigned char *cmdadr;
{
	unsigned char *simple();

	if (acctnod.namval && *acctnod.namval)
	{
		sabuf.ac_btime = time((long *)0);
		before = times(&buffer);
		sabuf.ac_uid = getuid();
		sabuf.ac_gid = getgid();
		movstrn(simple(cmdadr), sabuf.ac_comm, sizeof(sabuf.ac_comm));
		shaccton = 1;
	}
}


doacct()
{
	int fd;
	clock_t after;

	if (shaccton)
	{
		after = times(&buffer);
		sabuf.ac_utime = compress(buffer.tms_utime + buffer.tms_cutime);
		sabuf.ac_stime = compress(buffer.tms_stime + buffer.tms_cstime);
		sabuf.ac_etime = compress(after - before);

		if ((fd = open((char *)acctnod.namval, O_WRONLY | O_APPEND | O_CREAT, 0666)) != -1)
		{
			write(fd, &sabuf, sizeof(sabuf));
			close(fd);
		}
	}
}

/*
 *	Produce a pseudo-floating point representation
 *	with 3 bits base-8 exponent, 13 bits fraction
 */

compress(t)
	register clock_t t;
{
	register exp = 0;
	register rund = 0;

	while (t >= 8192)
	{
		exp++;
		rund = t & 04;
		t >>= 3;
	}

	if (rund)
	{
		t++;
		if (t >= 8192)
		{
			t >>= 3;
			exp++;
		}
	}

	return((exp << 13) + t);
}
#endif
