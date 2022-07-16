/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)librpc:openchild.c	1.1.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char     sccsid[] = "@(#)openchild.c 1.2 89/03/10 Copyr 1986 Sun Micro";
#endif
/*
 * openchild.c, 
 * 
 * Open two pipes to a child process, one for reading, one for writing. The
 * pipes are accessed by FILE pointers. This is NOT a public interface, but
 * for internal use only!
 */
#include <stdio.h>
/* #include <vfork.h> */

extern char    *malloc();
extern char    *strrchr();

static char    *basename();
static char    *SHELL = "/bin/sh";

/*
 * returns pid, or -1 for failure
 */
_rpc_openchild(command, fto, ffrom)
	char           *command;
	FILE          **fto;
	FILE          **ffrom;
{
	int             i;
	int             pid;
	int             pdto[2];
	int             pdfrom[2];
	char           *com;
	char            static_kludge[1024];

	if (pipe(pdto) < 0) {
		goto error1;
	}
	if (pipe(pdfrom) < 0) {
		goto error2;
	}
	switch (pid = fork()) {
	case -1:
		goto error3;

	case 0:
		/*
		 * child: read from pdto[0], write into pdfrom[1]
		 */
		{
			int             ii, j;
			(void) close(0);
			ii = dup(pdto[0]);
			(void) close(1);
			j = dup(pdfrom[1]);
			fflush(stderr);
		}
		for (i = _rpc_dtbsize() - 1; i >= 3; i--) {
			(void) close(i);
		}
		fflush(stderr);
		/* com = malloc((unsigned) strlen(command) + 6); */
		com = static_kludge;
		fflush(stderr);
		if (com == NULL) {
			perror("malloc");
			_exit(~0);
		}
		execlp(command, command, 0);
		perror("exec");
		_exit(~0);

	default:
		/*
		 * parent: write into pdto[1], read from pdfrom[0]
		 */
		*fto = fdopen(pdto[1], "w");
		(void) close(pdto[0]);
		*ffrom = fdopen(pdfrom[0], "r");
		(void) close(pdfrom[1]);
		break;
	}
	return (pid);

	/*
	 * error cleanup and return
	 */
error3:
	(void) close(pdfrom[0]);
	(void) close(pdfrom[1]);
error2:
	(void) close(pdto[0]);
	(void) close(pdto[1]);
error1:
	return (-1);
}

static char    *
basename(path)
	char           *path;
{
	char           *p;

	p = strrchr(path, '/');
	if (p == NULL) {
		return (path);
	} else {
		return (p + 1);
	}
}
