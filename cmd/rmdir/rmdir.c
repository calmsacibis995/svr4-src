/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rmdir:rmdir.c	1.13"
/*
** Rmdir(1) removes directory.
** If -p option is used, rmdir(1) tries to remove the directory
** and it's parent directories.  It exits with code 0 if the WHOLE
** given path is removed and 2 if part of path remains.  
** Results are printed except when -s is used.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

extern int opterr, optind, errno, rmdirp();
extern char *optarg, *sys_errlist[];

main(argc,argv)
int argc;
char **argv;
{

	int c, pflag, sflag, errflg, rc;
        char *ptr, *remain, *msg, *path;
	unsigned int pathlen;

	pflag = sflag = 0;
	errflg = 0;
	/* set effective uid, euid, to be same as real	*/
	/* uid, ruid.  Rmdir(2) checks search & write	*/
	/* permissions for euid, but for compatibility	*/
	/* the check must be done using ruid.		*/
	if(setuid(getuid()) == -1) {
		fprintf(stderr, "rmdir: setuid(2) failed, %s\n", sys_errlist[errno]);
		exit(1);
	}

	while ((c = getopt(argc, argv, "ps")) != EOF)
		switch (c) {
			case 'p':
				pflag++;
				break;
			case 's':
				sflag++;
				break;
			case '?':
				errflg++;
				break;
		}
        if(argc < 2 || errflg) {
                fprintf(stderr, "rmdir: usage: rmdir [-ps] dirname ...\n");
                exit(2);
        }
	argc -= optind;
	argv = &argv[optind];
        while (argc--) {
		ptr = *argv++;
  					/* -p option. Remove directory and parents.
					** Prints results of removing */
		if (pflag) {
			pathlen = (unsigned)strlen(ptr);
			if ((path = (char *)malloc(pathlen + 4)) == NULL ||
			    (remain = (char *)malloc(pathlen + 4)) == NULL) {
				fprintf(stderr, "rmdir: Out of memory\n");
				exit(2);
			}
			strcpy(path,ptr);

				/* rmdirp() removes directory and parents */
				/* rc != 0 implies only part of path removed */

			if ((rc = rmdirp(path, remain)) == 0) {
				if (!sflag) 
					fprintf(stdout,
					"rmdir: %s: Whole path removed.\n",ptr);
			}
			else {
				if (!sflag) {
					switch (rc) {
					case -1:
						if (errno == EEXIST)
							msg="Directory not empty";
						else
							msg = sys_errlist[errno];
						break;
					case -2:
						msg="Can not remove . or ..";
						break;
					case -3:
						msg="Can not remove current directory";
						break;
					}	
					fprintf(stdout,
					"rmdir: %s: %s not removed; %s\n",ptr, remain, msg); 
				}
			}
			free(path);
			free(remain);
			continue;
		}

			/* No -p option. Remove only one directory */

		if (rmdir(ptr) == -1) {
			switch(errno) {
			case EEXIST:	msg = "Directory not empty";
					break;
			case ENOTDIR:	msg = "Path component not a directory";
					break;
			case ENOENT:	msg = "Directory does not exist";
					break;
			case EACCES:	msg = "Search or write permission needed";
					break;
			case EBUSY:	msg = "Directory is a mount point or in use";
					break;
			case EROFS:	msg = "Read-only file system";
					break;
			case EIO:	msg = "I/O error accessing file system";
					break;
			case EINVAL:	msg = "Can't remove current directory or ..";
					break;
			case EFAULT:
			default:	msg = sys_errlist[errno];
					break;
			}
			fprintf(stderr, "rmdir: %s: %s\n", ptr, msg);
		continue;
		}
        }
        exit(errno?2:0);
}
