/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/paths.c	1.5"

/* PATH setup and search functions */


#include <limits.h>
#include <fcntl.h>
#include "rtinc.h"

/* directory search path - linked list of directory names */
struct pathnode {
	CONST char *name;
	int len;
	struct pathnode *next;
};

#define NODENUM	10	/* number of nodes allocated at once */

static struct pathnode *rt_dir_list = (struct pathnode *)0;

/* default search directory */
static CONST struct pathnode libdirs[] = {
	{ LIBDIR, LIBDIRLEN, (struct pathnode *)0 }
};

/* function to locate an object and open it;
 * returns open file descriptor for object and full path name,
 * if successful, -1 if not successful
 */
int _so_find(filename, pathname)
CONST char *filename;
CONST char **pathname;
{
	register char *p; 
	register struct pathnode *q;
	char pname[PATH_MAX];
	int slash = 0, fd, flen = 0;

	DPRINTF(LIST, (2, "rtld: _so_find(%s, 0x%x)\n", (filename ? filename : (CONST char *)"0"), (unsigned long)pathname));

	if (!filename) {
		_rt_lasterr("%s: %s: attempt to open file with null name",(char*) _rt_name,_proc_name);
		return(-1);
	}

	/* if filename contains any  '/'s, use filename 
	 * itself as the pathname 
	 */
	for (p = (char *)filename; *p; p++) {
		if (*p == '/')
			slash = 1;
	}

	flen = (p - filename) + 1; /* length includes null at end */
	if (flen >= PATH_MAX) {
		_rt_lasterr("%s: %s: file name too long: %s",(char*) _rt_name,_proc_name,filename);
		return(-1);
	}
	if (slash) {
		if ((fd = _open(filename, O_RDONLY)) == -1) {
			_rt_lasterr("%s: %s: can't open %s",(char*) _rt_name,_proc_name,filename);
			return(-1);
		}
		if ((p = _rtmalloc(flen)) == 0) {
			(void)_close(fd);
			return(-1);
		}
		(void)_rtstrcpy(p, filename);
		*pathname = p;
		return(fd);
	}
	/* no '/' - for each directory on list, make
	 * a pathname using that directory and filename and
	 * try to open that file
	 */

	for (q = rt_dir_list; q; q = q->next) {
		if ((q->len + flen + 1) > PATH_MAX) 
			continue;  /* ??? should this be an error ??? */
		(void)_rtstr3cpy(pname, q->name, "/", filename);
		if ((fd = _open(pname, O_RDONLY)) != -1) {
			if ((p = (char *)_rtmalloc(q->len + flen + 1)) == 0) {
				(void)_close(fd);
				return(-1);
			}
			(void)_rtstrcpy(p, pname);
			*pathname = p;
			return(fd);
		}
	}
	/* if here, no files found */
	_rt_lasterr("%s: %s: can't find %s",(char*) _rt_name,_proc_name,filename);
	return(-1);
}

/* set up directory search path: rt_dir_list
 * consists of directories (if any) from run-time list
 * in a.out's dynamic, followed by directories (if any)
 * in environment string LD_LIBRARY_PATH, followed by LIBDIR;
 * if we are running setuid or setgid, no directories from LD_LIBRARY_PATH
 * are permitted
 * returns 1 on success, 0 on error
 */
int _rt_setpath(envdirs, rundirs)
CONST char *envdirs, *rundirs;
{
	int secure = 0, elen = 0, rlen = 0, ndx, flen;
	register char *rdirs, *edirs;
	register struct pathnode *p1, *p2 = (struct pathnode *)0;

	DPRINTF(LIST,(2, "rtld: rt_setpath(%s, %s)\n",envdirs ? envdirs :
		(CONST char *)"0", rundirs ? rundirs : (CONST char *)"0"));


	/* first determine if we are running setuid/setgid */
	if ((_getuid() != _geteuid()) || (_getgid() != _getegid())) 
		secure = 1;

	/* allocate enough space for rundirs, envdirs and first
	 * chunk of pathnode structs - we allocate space for
	 * twice the size of envdirs and rundirs to allow for
	 * extra nulls at the end of each directory (foo::bar
	 * becomes foo\0.\0bar\0); this is overkill, but allows
	 * for the worst case and is faster than malloc'inc space
	 * for each directory individually
	 */
	if (envdirs)
		elen = _rtstrlen(envdirs);
	if (rundirs)
		rlen = _rtstrlen(rundirs);
	if ((rlen + elen) > 0) {
		if ((p1 = rt_dir_list = (struct pathnode *)_rtmalloc((2 * elen) + (2 * rlen) + (NODENUM 
			* sizeof(struct pathnode)))) == 0) 
			return 0;
		rdirs = (char *)p1 + (NODENUM * sizeof(struct pathnode));
		edirs = rdirs + (2 * rlen);
		ndx = 0;
		if (rundirs) {
			while (*rundirs) {
				p1->name = rdirs;
				p2 = p1;
				ndx++;
				if (ndx >= NODENUM) {
				/* allocate another set of pathnodes */
					if ((p1->next = (struct pathnode *)
						_rtmalloc(NODENUM * sizeof(struct 
						pathnode))) == 0) 
						return 0;
					ndx = 0;
				}
				else p1->next = p1 + 1;
				p1 = p1->next;
				if (*rundirs == ':') {
					*rdirs++ = '.';
					flen = 1;
				}
				else {
					flen = 0;
					while (*rundirs != ':' && *rundirs) {
						*rdirs++ = *rundirs++;
						flen++;
					}
				}
				*rdirs++ = '\0';
				p2->len = flen;
				if (*rundirs) 
					rundirs++;
			} /* end while (*rundirs) */
		} /* end if (rundirs) */
		/* envdirs is of form [PATH1][;PATH2] */
		if (envdirs && !secure) {
			if (*envdirs == ';')
				++envdirs;
			while (*envdirs) {
				p1->name = edirs;
				p2 = p1;
				ndx++;
				if (ndx >= NODENUM) {
				/* allocate another set of pathnodes */
					if ((p1->next = (struct pathnode *)
						_rtmalloc(NODENUM * sizeof(struct 
						pathnode))) == 0) 
						return 0;
					ndx = 0;
				}
				else p1->next = p1 + 1;
				p1 = p1->next;
				if (*envdirs == ':') {
					*edirs++ = '.';
					flen = 1;
				}
				else {
					flen = 0;
					while (*envdirs != ':' && *envdirs != ';' && *envdirs) {
						*edirs++ = *envdirs++;
						flen++;
					}
				}
				*edirs++ = '\0';
				p2->len = flen;
				if (*envdirs) 
					envdirs++;
			} /* while (*envdirs) */
		} /*if (envdirs) */
	} 

	/* add LIBDIR to end of list */
	if (!p2)
		rt_dir_list = (struct pathnode *)libdirs;
	else 
		p2->next = (struct pathnode *)libdirs;
#ifdef DEBUG
	if (_debugflag & LIST) {
		p1 = rt_dir_list;
		while(p1) {
			_rtfprintf(2, "%s\n",p1->name);
			p1 = p1->next;
		}
	}
#endif
	return(1);
}
