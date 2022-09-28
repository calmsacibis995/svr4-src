/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:installf/installf.c	1.10.6.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

#define LSIZE	1024
#define MALSIZ	164

#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_MAJOR 	"invalid major number <%s> specified for <%s>"
#define ERR_MINOR 	"invalid minor number <%s> specified for <%s>"
#define ERR_MODE	"invalid mode <%s> specified for <%s>"
#define ERR_RELPATH 	"relative pathname <%s> not permitted"
#define ERR_LINK	"invalid link specification <%s>"
#define ERR_LINKFTYPE	"ftype <%c> does not match link specification <%s>"
#define ERR_LINKARGS	"extra arguments in link specification <%s>"
#define ERR_LINKREL	"relative pathname in link specification <%s>"
#define ERR_FTYPE	"invalid ftype <%c> for <%s>"
#define ERR_ARGC 	"invalid number of arguments for <%s>"
#define ERR_SPECALL	"ftype <%c> requires all fields to be specified"

extern char	*classname;
extern int	errno, eptnum;
extern struct cfent 
		**eptlist;

extern void	*calloc(),
		*realloc(),
		exit(), 
		usage(),
		progerr(),
		quit(),
		canonize(),
		qsort();
extern char	*pathdup();
extern long	strtol();
extern int	gpkgmap();

static int	validate();
int		cfentcmp();

installf(argc, argv)
int argc;
char *argv[];
{
	struct cfent *new;
	char	line[LSIZE];
	char	*largv[8];
	int	myerror;

	if(strcmp(argv[0], "-")) {
		if(argc < 1)
			usage(); /* at least pathname is required */
		eptlist = (struct cfent **) calloc(2, sizeof(struct cfent *));
		eptlist[0] = new = (struct cfent *) calloc(1, 
			sizeof(struct cfent));
		eptnum = 1;

		/* use command line arguments as entry input */
		if(validate(new, argc, argv))
			quit(1);
		return(0);
	} 

	/* read stdin to obtain entries, which need to be sorted */
	eptnum = 0;
	myerror = 0;
	eptlist = (struct cfent **) calloc(MALSIZ, sizeof(struct cfent *));
	while(fgets(line, LSIZE, stdin) != NULL) {
		argc = 0;
		argv = largv;
		argv[argc++] = strtok(line, " \t\n");
		while(argv[argc] = strtok(NULL, " \t\n")) 
			argc++;

		if(argc < 1)
			usage(); /* at least pathname is required */
			
		new = (struct cfent *) calloc(1, sizeof(struct cfent));
		if(new == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
		if(validate(new, argc, argv))
			myerror++;
		eptlist[eptnum] = new;
		if((++eptnum % MALSIZ) == 0) { 
			eptlist = (struct cfent **) realloc((void *)eptlist, 
			   (unsigned) (sizeof(struct cfent *) * 
			   (eptnum+MALSIZ)));
			if(!eptlist) {
				progerr(ERR_MEMORY, errno);
				quit(99);
			}
		}
	}
	eptlist[eptnum] = (struct cfent *)NULL;
	qsort((char *)eptlist, (unsigned)eptnum, sizeof(struct cfent *), 
		cfentcmp);
	return(myerror);
}

static int
validate(ept, argc, argv)
struct cfent *ept;
int	argc;
char	*argv[];
{
	char	*ret, *pt;
	int	n, allspec;
	int 	relative = 0;

	/* initialize cfent structure */
	ept->pinfo = (struct pinfo *) 0;
	(void) gpkgmap(ept, NULL);

	n = allspec = 0;
	if(classname)
		(void) strncpy(ept->class, classname, CLSSIZ);

	ept->path = pathdup(argv[n++]);
	canonize(ept->path);
	if(ept->path[0] != '/') {
		progerr(ERR_RELPATH, ept->path);
		return(1);
	}
	if(pt = strchr(ept->path, '=')) {
		/* links specifications should be handled right here */
		ept->ftype = ((n >= argc) ? 'l' : argv[n++][0]);
		if(!pt[1]) {
			progerr(ERR_LINK, ept->path);
			return(1);
		}
		if(argc != n) {
			progerr(ERR_LINKARGS, ept->path);
			return(1);
		}
		if(!strchr("sl", ept->ftype)) {
			progerr(ERR_LINKFTYPE, ept->ftype, ept->path);
			return(1);
		}
		ept->ainfo.local = pathdup(pt+1);
		if(strchr("s", ept->ftype)) {
			if(!isdot(ept->ainfo.local) && 
			   !isdotdot(ept->ainfo.local))
				canonize(ept->ainfo.local);
			else
				relative++;
		} else
			canonize(ept->ainfo.local);

		if(ept->ainfo.local[0] != '/') {
			if(!relative) {
				progerr(ERR_LINKREL, ept->path);
				return(1);
			}
		}
		*pt = '\0'; /* cut off pathname at the = sign */
		return(0);
	} else if(n >= argc) {
		/* we are expecting to change object's contents */
		return(0);
	}

	ept->ftype = argv[n++][0];
	if(strchr("sl", ept->ftype)) {
		progerr(ERR_LINK, ept->path);
		return(1);
	} else if(!strchr("?fvedxcbp", ept->ftype)) {
		progerr(ERR_FTYPE, ept->ftype, ept->path);
		return(1);
	}

	if(ept->ftype == 'b' || ept->ftype == 'c') {
		if(n < argc) {
			ept->ainfo.major = strtol(argv[n++], &ret, 0);
			if(ret && *ret) {
				progerr(ERR_MAJOR, argv[n-1], ept->path);
				return(1);
			}
		} 
		if(n < argc) {
			ept->ainfo.minor = strtol(argv[n++], &ret, 0);
			if(ret && *ret) {
				progerr(ERR_MINOR, argv[n-1], ept->path);
				return(1);
			}
			allspec++;
		} 
	} 

	allspec = 0;
	if(n < argc) {
		if(!strcmp(argv[n], "?")) {
			ept->ainfo.mode = BADMODE;
			n++;
		} else {
			ept->ainfo.mode = strtol(argv[n++], &ret, 8);
			if(ret && *ret) {
				progerr(ERR_MODE, argv[n-1], ept->path);
				return(1);
			}
		}
	}
	if(n < argc)
		(void) strncpy(ept->ainfo.owner, argv[n++], ATRSIZ);
	if(n < argc) {
		(void) strncpy(ept->ainfo.group, argv[n++], ATRSIZ);
		allspec++;
	}
	if(strchr("dxbcp", ept->ftype) && !allspec) {
		progerr(ERR_ARGC, ept->path);
		progerr(ERR_SPECALL, ept->ftype);
		return(1);
	}
	
	if(n < argc) {
		progerr(ERR_ARGC, ept->path);
		return(1);
	}
	return(0);
}

int
cfentcmp(ept1, ept2)
struct cfent **ept1, **ept2;
{
	return(strcmp((*ept1)->path, (*ept2)->path));
}
