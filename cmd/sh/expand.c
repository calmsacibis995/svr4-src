/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:expand.c	1.18.5.1"

/*
 *	UNIX shell
 *
 */

#include	"defs.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>



/*
 * globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */
static int	addg();
DIR *_opendir();


expand(as, rcnt)
	unsigned char	*as;
{
	int	count; 
	DIR	*dirf;
	BOOL	dir = 0;
	unsigned char	*rescan = 0;
	unsigned char 	*slashsav = 0;
	register unsigned char	*s, *cs;
	unsigned char *s2 = 0;
	struct argnod	*schain = gchain;
	BOOL	slash;

	if (trapnote & SIGSET)
		return(0);
	s = cs = as;
	/*
	 * check for meta chars
	 */
	{
		register BOOL open;

		slash = 0;
		open = 0;
		do
		{
			switch (*cs++)
			{
			case 0:
				if (rcnt && slash)
					break;
				else
					return(0);

			case '/':
				slash++;
				open = 0;
				continue;

			case '[':
				open++;
				continue;

			case ']':
				if (open == 0)
					continue;

			case '?':
			case '*':
				if (rcnt > slash)
					continue;
				else
					cs--;
				break;


			case '\\':
				cs++;
			default:
				continue;
			}
			break;
		} while (TRUE);
	}

	for (;;)
	{
		if (cs == s)
		{
			s = (unsigned char *)nullstr;
			break;
		}
		else if (*--cs == '/')
		{
			*cs = 0;
			if (s == cs)
				s = (unsigned char *)"/";
			else {
			/* push trimmed copy of directory prefix
			   onto stack */
				s2 = cpystak(s);
				trim(s2);
				s = s2;
			}
			break;
		}
	}

	if ((dirf = _opendir(*s ? s : (unsigned char *)".")) != 0)
		dir++;

	/* Let s point to original string because it will be trimmed later */
	if(s2)
		s = as;
	count = 0;
	if (*cs == 0)
		slashsav = cs++; /* remember where first slash in as is */

	/* check for rescan */
	if (dir)
	{
		register unsigned char *rs;
		struct dirent *e;

		rs = cs;
		do /* find next / in as */
		{
			if (*rs == '/')
			{
				rescan = rs;
				*rs = 0;
				gchain = 0;
			}
		} while (*rs++);

		while ((e = readdir(dirf)) && (trapnote & SIGSET) == 0)
		{
			if (e->d_name[0] == '.' && *cs != '.')
				continue;

			if (gmatch(e->d_name, cs))
			{
				addg(s, e->d_name, rescan, slashsav);
				count++;
			}
		}
		(void)_closedir(dirf);

		if (rescan)
		{
			register struct argnod	*rchain;

			rchain = gchain;
			gchain = schain;
			if (count)
			{
				count = 0;
				while (rchain)
				{
					count += expand(rchain->argval, slash + 1);
					rchain = rchain->argnxt;
				}
			}
			*rescan = '/';
		}
	}

	if(slashsav)
		*slashsav = '/';
	return(count);
}

/*
	opendir -- C library extension routine

*/

extern int	open(), close(), fstat();

#define NULL	0

DIR *
_opendir(filename)
char		*filename;	/* name of directory */
{
	static DIR direct;
	static char dirbuf[DIRBUF];
	register DIR	*dirp = &direct;		/* static storage */
	register int	fd;		/* file descriptor for read */
	struct stat	sbuf;		/* result of fstat() */

	if (stat(filename, &sbuf) < 0
	  || (sbuf.st_mode & S_IFMT) != S_IFDIR 
	  || (fd = open(filename, 0)) < 0 )	
		return NULL;		

	dirp->dd_fd = fd;
	dirp->dd_loc = dirp->dd_size = 0;	/* refill needed */
	dirp->dd_buf = dirbuf;
	return dirp;
}

/*
	closedir -- C library extension routine

*/


extern int	close();

_closedir(dirp) 
register DIR	*dirp;		/* stream from opendir() */
{
	return close(dirp->dd_fd);
}

static int
addg(as1, as2, as3, as4)
unsigned char	*as1, *as2, *as3, *as4;
{
	register unsigned char	*s1, *s2;

	s2 = locstak() + BYTESPERWORD;
	s1 = as1;
	if(as4) {
		while (*s2 = *s1++)
			s2++; 
	/* Restore first slash before the first metacharacter if as1 is not "/" */
		if(as4 + 1 == s1)
			*s2++ = '/';
	}
/* add matched entries, plus extra \\ to escape \\'s */
	s1 = as2;
	while (*s2 = *s1++) {
		if(*s2 == '\\')
			*++s2 = '\\';
		s2++;
	}
	if (s1 = as3)
	{
		*s2++ = '/';
		while (*s2++ = *++s1);
	}
	makearg(endstak(s2));
}

makearg(args)
	register struct argnod *args;
{
	args->argnxt = gchain;
	gchain = args;
}


