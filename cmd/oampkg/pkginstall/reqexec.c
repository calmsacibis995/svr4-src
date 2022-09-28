/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/reqexec.c	1.4.3.1"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include "install.h"

extern char	*respfile,
		tmpdir[];
extern int	nointeract;

extern void	progerr(),
		ptext(),
		quit();
extern int	access(),
		unlink(),
		creat(),
		close(),
		chown(),
		pkgexecl();
extern char	*qstrdup(),
		*mktemp();

reqexec(script, output)
char	*script;
char	*output;
{
	char	path[PATH_MAX];
	int	fd;
	uid_t	instuid;
	gid_t	instgid;

	/* this code should eventually be modified to get
	 * an appropriate user and group id to execute the
	 * request script
	 */
	instuid = (uid_t) 0; /* root */
	instgid = (gid_t) 1; /* other */
	
	if(access(script, 0) != 0)
		return(0);

	if(nointeract) {
		ptext(stderr, "Interactive request script supplied by package");
		quit(5);
	}

	if(output == NULL) {
		/* place output in temporary file */
		(void) sprintf(path, "%s/respXXXXXX", tmpdir);
		respfile = mktemp(path);
		if(respfile == NULL) {
			progerr("unable to create temporary response file");
			quit(99);
		}
		respfile = qstrdup(respfile);
	} else {
		respfile = output;
		if((access(respfile, 0) == 0) && unlink(respfile)) {
			progerr("unable to remove response file <%s>", respfile);
			quit(99);
		}
	}


	/* 
	 * create a zero length response file which is only writable
	 * by the non-priveledged installation user-id, but is readable
	 * by the world
	 */
	if((fd = creat(respfile, 644)) < 0) {
		progerr("unable to create response file <%s>", respfile);
		quit(99);
	}
	(void) close(fd);
	(void) chown(respfile, instuid, instgid);

	return(pkgexecl(NULL, NULL, SHELL, script, respfile, NULL));
}
