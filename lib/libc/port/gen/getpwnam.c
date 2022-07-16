/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getpwnam.c	1.19"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak getpwnam = _getpwnam
	#pragma weak getpwuid = _getpwuid
#endif
#include "synonyms.h"
#include "shlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


#define PASSWD "/etc/passwd"
static char *line;
static size_t linesz;
static char *buf;

static struct passwd *entry();
static void cleanup(), done();
static char *begin();

struct passwd *
getpwnam(name)
register const char *name;
{
	register struct passwd *p;
	char *current, *after;

	if ((current = begin(&after)) == 0)
		return 0;
	while((p = entry(&current, after)) != 0 && strcmp(p->pw_name, name))
		;
	if (p == 0)
		cleanup();
	done();
	return(p);
}

struct passwd *
getpwuid(uid)
register uid_t uid;
{
	register struct passwd *p;
	char *current, *after;

	if ((current = begin(&after)) == 0)
		return 0;
	while((p = entry(&current, after)) != 0 && p->pw_uid != uid)
		;
	if (p == 0)
		cleanup();
	done();
	return(p);
}

static char *
begin(pafter)
	char		**pafter;
{
	struct stat	sb;
	int		fd;
	size_t		sz;

	if ((fd = open(PASSWD, 0)) < 0)
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

static void
cleanup()
{
	if (line != 0)
	{
		linesz = 0;
		free(line);
		line = 0;
	}
}


static void
done()
{
	if (buf != 0)
	{
		free(buf);
		buf = 0;
	}
}

static char *
skip(p)
char *p;
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p)
	 	*p++ = '\0';
	return(p);
}

static struct passwd *
entry(pcur, after)
	char	**pcur, *after;
{
	register char *p = *pcur;
	long	x;
	size_t len;

	static struct passwd *pwd;

	if (!pwd && (pwd = (struct passwd *)malloc(sizeof(struct passwd))) == 0)
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
			cleanup();
			return 0;
		}
		linesz = len;
	}
	memcpy(line, p, len);
	p = line;
	pwd->pw_name = p;
	p = skip(p);
	pwd->pw_passwd = p;
	p = skip(p);
	if (p == 0 || *p == ':') {
		/* check for non-null uid */
		errno = EINVAL;
		return (0);
	}
	x = strtol(p, &after, 10);	
	if (after != memchr(p, ':', strlen(p))){
		/* check for numeric value */
		errno = EINVAL;
		return (0);
	}
	p = skip(p);
	pwd->pw_uid = (x < 0 || x > MAXUID)? (UID_NOBODY): x;
	if (p == 0 || *p == ':') {
		/* check for non-null uid */
		errno = EINVAL;
		return (0);
	}
	x = strtol(p, &after, 10);	
	if (after != memchr(p, ':', strlen(p))) {
		/* check for numeric value */
		errno = EINVAL;
		return (0);
	}
	p = skip(p);
	pwd->pw_gid = (x < 0 || x > MAXUID)? (UID_NOBODY): x;
	pwd->pw_comment = p;
	pwd->pw_gecos = p;
	p = skip(p);
	pwd->pw_dir = p;
	p = skip(p);
	pwd->pw_shell = p;
	(void) skip(p);

	p = pwd->pw_passwd;
	while(*p && *p != ',')
		p++;
	if(*p)
		*p++ = '\0';
	pwd->pw_age = p;
	return(pwd);
}
