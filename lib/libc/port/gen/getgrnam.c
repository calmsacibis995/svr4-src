/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getgrnam.c	1.16"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak getgrnam = _getgrnam
	#pragma weak getgrgid = _getgrgid
#endif
#include "synonyms.h"
#include "shlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define GROUP "/etc/group"
static char *line, **gr_mem;
static size_t linesz, gr_cnt;
static char *buf;

extern struct group *_grp_entry();
extern void _grp_cleanup(), _grp_done();
extern char *_grp_begin();

struct group *
getgrnam(name)
register const char *name;
{
	register struct group *p;
	char *current, *after;

	if ((current = _grp_begin(&after)) == 0)
		return 0;
	while((p = _grp_entry(&current, after)) != 0 && strcmp(p->gr_name, name))
		;
	if (p == 0)
		_grp_cleanup();
	_grp_done();
	return(p);
}

struct group *
getgrgid(gid)
register uid_t gid;
{
	register struct group *p;
	char *current, *after;

	if ((current = _grp_begin(&after)) == 0)
		return 0;
	while((p = _grp_entry(&current, after)) != 0 && p->gr_gid != gid)
		;
	if (p == 0)
		_grp_cleanup();
	_grp_done();
	return(p);
}

char *
_grp_begin(pafter)
	char		**pafter;
{
	struct stat	sb;
	int		fd;
	size_t		sz;

	if ((fd = open(GROUP, 0)) < 0)
		return 0;
	if (fstat(fd, &sb) != 0
	|| (sz = sb.st_size) != sb.st_size
	|| (buf = malloc(sz)) == 0)
	{
		(void)close(fd);
		return 0;
	}
	if (read(fd, buf, sz) != sz)
	{
		free(buf);
		close(fd);
		return 0;
	}
	close(fd);
	*pafter = buf + sz;
	return buf;
}

void
_grp_cleanup()
{
	if (line != 0)
	{
		linesz = 0;
		free(line);
		line = 0;
	}
	if (gr_mem != 0)
	{
		gr_cnt = 0;
		free(gr_mem);
		gr_mem = 0;
	}
}


void
_grp_done()
{
	if (buf != 0)
	{
		free(buf);
		buf = 0;
	}
}

static char *
skip(p, c)
char *p;
int c;
{
	while (*p != '\0' && *p != c)
		++p;
	if (*p != '\0')
	 	*p++ = '\0';
	return(p);
}

struct group *
_grp_entry(pcur, after)
	char	**pcur, *after;
{
	char *p = *pcur, **q;
	size_t len;

	static struct group *grp;
	if (!grp && (grp = (struct group *)malloc(sizeof(struct group))) == 0)
		return 0;
	if ((after = memchr(p, '\n', after - p)) == 0)
		return 0;
	*pcur = ++after;
	len = after - p;
	if (linesz < len)
	{
		if (line)
			free(line);
		if ((line = malloc(len)) == 0)
		{
			linesz = 0;
			_grp_cleanup();
			return 0;
		}
		linesz = len;
	}
	memcpy(line, p, len);
	p = line;
	grp->gr_name = p;
	grp->gr_passwd = p = skip(p, ':');
	grp->gr_gid = atol(p = skip(p, ':'));
	p = skip(p, ':');
	(void) skip(p, '\n');
	{
		register char	*s;
		for (len = 2, s = p; *s != '\0'; ++s)
			if (*s == ',')
				++len;
	}
	if (gr_cnt < len)
	{
		if (gr_mem)
			free(gr_mem);
		if ((gr_mem = (char **)malloc(len * sizeof(char**))) == 0)
		{
			gr_cnt = 0;
			_grp_cleanup();
			return 0;
		}
		gr_cnt = len;
	}
	grp->gr_mem = gr_mem;
	q = grp->gr_mem;
	while (*p != '\0')
	{
		*q++ = p;
		p = skip(p, ',');
	}
	*q = 0;
	return(grp);
}
