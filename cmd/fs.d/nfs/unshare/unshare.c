/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/unshare/unshare.c	1.2.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
/*
 * nfs unshare
 */
#include <stdio.h>
#include <string.h>
#include <varargs.h>
#include <unistd.h>
#include <sys/errno.h>
#include "sharetab.h"

#define RET_OK		0
#define RET_ERR		32

void usage();
void pr_err();

main(argc, argv)
	int argc;
	char **argv;
{

	if (argc != 2) {
		usage();
		exit(1);
	}

	exit(do_unshare(argv[1]));
}

int
do_unshare(path)
	char *path;
{
	if (shared(path) < 0)
		return (RET_ERR);

	/*
	 * Only tell the kernel if there are no other shared resources
	 * that resolve to the same as this one by following symlinks
	 */
	if (c_link(path) >= 0)
		if (exportfs(path, NULL) < 0) {
			pr_err("");
			perror(path);
			return (RET_ERR);
		}

	if (sharetab_del(path) < 0)
		return (RET_ERR);

	return (RET_OK);
}

int
shared(path)
	char *path;
{
	FILE *f;
	struct share *sh;
	int res;

	f = fopen(SHARETAB, "r");
	if (f == NULL) {
		pr_err("");
		perror(SHARETAB);
		return (-1);
	}

	while ((res = getshare(f, &sh)) > 0) {
		if (strcmp(path, sh->sh_path) == 0
		 && strcmp("nfs", sh->sh_fstype) == 0) {
			(void) fclose(f);
			return (1);
		}
	}
	if (res < 0) {
		pr_err("error reading %s\n", SHARETAB);
		(void) fclose(f);
		return (-1);
	}
	pr_err("%s not shared\n", path);
	(void) fclose(f);
	return (-1);
}

/*
 * Remove an entry from the sharetab file.
 */
int
sharetab_del(path)
	char *path;
{
	FILE *f;

	f = fopen(SHARETAB, "r+");
	if (f == NULL) {
		pr_err("");
		perror(SHARETAB);
		return (-1);
	}
	if (lockf(fileno(f), F_LOCK, 0L) < 0) {
		pr_err("cannot lock");
		perror(SHARETAB);
		(void) fclose(f);
		return (-1);
	}
	if (remshare(f, path) < 0) {
		pr_err("remshare\n");
		return (-1);
	}
	(void) fclose(f);
	return (0);
}

void
usage()
{
	(void) fprintf(stderr, "Usage: unshare { pathname | resource }\n");
}

void
pr_err(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list ap;

	va_start(ap);
	(void) fprintf(stderr, "nfs unshare: ");
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
}

/*
 * Check if 'path' is either a symbolic link that
 * resolves to something else that is already shared or
 * if something else shared is a symbolic link to 'path'.
 * Return -1 if we find a symlink.
 * Return 0 on errors to be sure we unshare the resource.
 */
static int
c_link(path)
	char *path;
{
	char buf[BUFSIZ];
	int i, j;
	FILE *fp;
	struct share *sh;
	int res;

	if ((i = readlink(path, buf, BUFSIZ)) > 0) {
		/* check if 'path' is a symlink to any other entries in sharetab. */
		buf[i] = '\0';
		if ((fp = fopen(SHARETAB, "r")) == NULL) {
			pr_err("");
			perror(SHARETAB);
			return (0);
		}
		while ((res = getshare(fp, &sh)) > 0) {
			if (strcmp(buf, sh->sh_path) == 0 &&
			    strcmp("nfs", sh->sh_fstype) == 0 &&
			    strcmp(path, sh->sh_path) != 0) {
				fclose(fp);
				return (-1);
			}
		}
		if (res < 0) {
			pr_err("error reading %s\n", SHARETAB);
			fclose(fp);
			return (0);
		}
	} else {
		/* check if any entry in sharetab is a symlink to 'path' */
		if ((fp = fopen(SHARETAB, "r")) == NULL) {
			pr_err("");
			perror(SHARETAB);
			return (0);
		}
		while ((res = getshare(fp, &sh)) > 0) {
			if ((j = readlink(sh->sh_path, buf, BUFSIZ)) > 0) {
				buf[j] = '\0';
				if (strcmp(path, buf) == 0 &&
			 	    strcmp("nfs", sh->sh_fstype) == 0 &&
				    strcmp(path, sh->sh_path) != 0) {
					fclose(fp);
					return (-1);
				}
			}
		}
		if (res < 0) {
			pr_err("error reading %s\n", SHARETAB);
			fclose(fp);
			return (0);
		}
	}
	return(0);
}
