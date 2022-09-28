/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:pwd.c	1.14.8.1"
/* 
 *	UNIX shell
 */

#include	"mac.h"
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>

#define	DOT		'.'
#define	NULL	0
#define	SLASH	'/'
#define PARTLY	2

static void rmslash();
#ifdef __STDC__
extern const char	longpwd[];
#else
extern char	longpwd[];
#endif
extern char *getcwd();

unsigned char cwdname[PATH_MAX+1];

static int 	didpwd = FALSE;

cwd(dir)
	register unsigned char *dir;
{
	register unsigned char *pcwd;
	register unsigned char *pdir;

	/* First remove extra /'s */

	rmslash(dir);

	/* Now remove any .'s */

	pdir = dir;
	if(*dir == SLASH)
		pdir++;
	while(*pdir) 			/* remove /./ by itself */
	{
		if((*pdir==DOT) && (*(pdir+1)==SLASH))
		{
			movstr(pdir+2, pdir);
			continue;
		}
		pdir++;
		while ((*pdir) && (*pdir != SLASH)) 
			pdir++;
		if (*pdir) 
			pdir++;
	}
	/* take care of trailing /. */
	if(*(--pdir)==DOT && pdir > dir && *(--pdir)==SLASH) {
		if(pdir > dir) {
			*pdir = NULL;
		} else {
			*(pdir+1) = NULL;
		}
	
	}
	
	/* Remove extra /'s */

	rmslash(dir);

	/* Now that the dir is canonicalized, process it */

	if(*dir==DOT && *(dir+1)==NULL)
	{
		return;
	}


	if(*dir==SLASH)
	{
		/* Absolute path */

		pcwd = cwdname;
		*pcwd++ = *dir++;
		didpwd = PARTLY;
	}
	else
	{
		/* Relative path */

		if (didpwd == FALSE) 
			return;
		didpwd = PARTLY;	
		pcwd = cwdname + length(cwdname) - 1;
		if(pcwd != cwdname+1)
			*pcwd++ = SLASH;
	}
	while(*dir)
	{
		if(*dir==DOT && 
		   *(dir+1)==DOT &&
		   (*(dir+2)==SLASH || *(dir+2)==NULL))
		{
			/* Parent directory, so backup one */

			if( pcwd > cwdname+2 )
				--pcwd;
			while(*(--pcwd) != SLASH)
				;
			pcwd++;
			dir += 2;
			if(*dir==SLASH)
			{
				dir++;
			}
			continue;
		}
	 	if (pcwd >= &cwdname[PATH_MAX+1])
		{
			didpwd=FALSE;
			return;
		}
		*pcwd++ = *dir++;
		while((*dir) && (*dir != SLASH))
		{  
	 		if (pcwd >= &cwdname[PATH_MAX+1])
			{
				didpwd=FALSE;
				return;
			}
			*pcwd++ = *dir++;
		} 
		if (*dir) 
		{
	 		if (pcwd >= &cwdname[PATH_MAX+1])
			{
				didpwd=FALSE;
				return;
			}
			*pcwd++ = *dir++;
		}
	}
	if (pcwd >= &cwdname[PATH_MAX+1])
	{
		didpwd=FALSE;
		return;
	}
	*pcwd = NULL;

	--pcwd;
	if(pcwd>cwdname && *pcwd==SLASH)
	{
		/* Remove trailing / */

		*pcwd = NULL;
	}
	return;
}

void
cwd2()
{
	struct stat stat1, stat2;
	unsigned char *pcwd;
	/* check if there are any symbolic links in pathname */

	if(didpwd == FALSE)
		return;
	pcwd = cwdname + 1;
	if(didpwd == PARTLY) {
		while (*pcwd)
		{
			char c;
			while((c = *pcwd++) != SLASH && c != '\0');
			*--pcwd = '\0';
			if (lstat((char *)cwdname, &stat1) == -1
		    	|| (stat1.st_mode & S_IFMT) == S_IFLNK) {
				didpwd = FALSE;
				*pcwd = c;
				return;
			}
			*pcwd = c;
			if(c)
				pcwd++;
		}
		didpwd = TRUE;
	} else 
		if (stat((char *)cwdname, &stat1) == -1) {
			didpwd = FALSE;
			return;
		}
	/*
	 * check if ino's and dev's match; pathname could
	 * consist of symbolic links with ".."
	 */

	if (stat(".", &stat2) == -1
	    || stat1.st_dev != stat2.st_dev
	    || stat1.st_ino != stat2.st_ino)
		didpwd = FALSE;
	return;
}

unsigned char *
cwdget()
{
	cwd2();
	if (didpwd == FALSE) {
		if(getcwd(cwdname, PATH_MAX+1) == (char *)0)
			*cwdname = 0;
		didpwd = TRUE;
	} 
	return (cwdname);
}

/*
 *	Print the current working directory.
 */

cwdprint()
{
	cwd2();
	if (didpwd == FALSE) {
		if(getcwd(cwdname, PATH_MAX+1) == (char *)0) {
			if(errno && errno != ERANGE)
				error("cannot determine current directory");
			else
				error(longpwd);
		}
		didpwd = TRUE;
	}
	prs_buff(cwdname);
	prc_buff(NL);
	return;
}

/*
 *	This routine will remove repeated slashes from string.
 */

static void
rmslash(string)
	unsigned char *string;
{
	register unsigned char *pstring;

	pstring = string;
	while(*pstring)
	{
		if(*pstring==SLASH && *(pstring+1)==SLASH)
		{
			/* Remove repeated SLASH's */

			movstr(pstring+1, pstring);
			continue;
		}
		pstring++;
	}

	--pstring;
	if(pstring>string && *pstring==SLASH)
	{
		/* Remove trailing / */

		*pstring = NULL;
	}
	return;
}
