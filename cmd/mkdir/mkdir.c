/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mkdir:mkdir.c	1.7.9.1"
/*
** make directory.
** If -m is used with a valid mode, directories will be
** created in that mode.  Otherwise, the default mode will
** be 777 possibly altered by the process's file mode creation
** mask.
** If -p is used, make the directory as well as
** its non-existing parent directories.
*/

#include	<signal.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<string.h>

#define  WPERM  02
extern int opterr, optind,  errno;
extern char *optarg,  *sys_errlist[];

char *strrchr();

main(argc, argv)
int argc;
char *argv[];
{
	int 	pflag, errflg, mflag;
	int 	i, c, local_errno;
	mode_t	mode;
	char 	*d, *p, *m; 
	char 	*ptr, *endptr;

	mode= S_IRWXU | S_IRWXG | S_IRWXO;
	pflag = mflag = errflg = 0;
	local_errno = 0;

	while ((c=getopt(argc, argv, "m:p")) != EOF) {
		switch (c) {
		case 'm':
			mflag++;
			m = optarg;
			for(i=0; *m != '\0'; m++) {
				if(*m < '0' || *m > '7') {
					fprintf(stderr,"Invalid mode.\n");
					exit(2);
				}
			}
			umask(0);
			mode = (mode_t) strtol(optarg, (char **)NULL, 8);
			break;
		case 'p':
			pflag++;
			break;
		case '?':
			errflg++;
			break;
		}
	}

	argc -= optind;
	if(argc < 1 || errflg) {
		fprintf(stderr, "mkdir: usage: mkdir [-m mode] [-p] dirname ...\n");
		exit(2);
	}
	argv = &argv[optind];

	/* Set id to real uid and gid */
	if (setuid(getuid()) || setgid(getgid())) {
		fprintf(stderr, "mkdir:  Failed to set effective user/group ids to real user/group ids");
	}

        while(argc--) {
		d = *argv++;
		/* Skip extra slashes at the end of path */
		while ((endptr=strrchr(d, '/')) != NULL){
			p=endptr;
			p++;
			if (*p == '\0')
				*endptr='\0';
			else
				break;
		}

		/* When -p is set, invokes mkdirp library routine.
		 * Although successfully invoked, mkdirp sets errno to ENOENT 
		 * if one of the directory in the pathname does not exist,
		 * thus creates a confusion on success/failure status 
		 * possibly checked by the calling routine or shell. 
		 * Therefore, errno is reset only when
		 * mkdirp has executed successfully, otherwise save
		 * in local_errno.
		 */ 
		if (pflag) { 
			if (mkdirp(d,mode) < 0) {
				fprintf(stderr, "mkdir: \"%s\": %s\n",d,sys_errlist[errno]);
				local_errno = errno;
				continue;
			}
			errno = 0;
			continue;
		}

		/* No -p. Make only one directory 
		 * Check write permission of the parent directory
		 */


		/* All the conditions are met, make the directory */
		if ((mkdir(d, mode)) < 0) {
                   	fprintf(stderr, "mkdir:  Failed to make directory \"%s\"; %s\n", d, sys_errlist[errno]);  
			continue;
		}
	} /* end while */

	/* When pflag is set, the errno is saved in local_errno */
	if (local_errno)
		errno = local_errno;
        exit(errno ? 2: 0);
}
