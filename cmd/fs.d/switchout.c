/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fs.cmds:switchout.c	1.19.3.1"

#include	<stdio.h>
#include 	<limits.h>
#include	<sys/fstyp.h>  
#include	<sys/errno.h>  
#include	<sys/vfstab.h>

#define	FSTYPE_MAX	8
#define	FULLPATH_MAX	64
#define	ARGV_MAX	1024
#define	VFS_PATH	"/usr/lib/fs"
#define	ALT_PATH	"/etc/fs"

extern	int errno;
char	*special = NULL;  /*  device special name  */
char	*fstype = NULL;	  /*  fstype name is filled in here  */
char	*basename;	  /* name of command */
char	*newargv[ARGV_MAX]; 	/* args for the fstype specific command  */
char	vfstab[] = VFSTAB;
int	newargc = 2;

struct commands {
	char *c_basename;
	char *c_optstr;
	char *c_usgstr;
} cmd_data[] = {
	"clri", "F:o:?V", 
		"[-F FSType] [-V] special inumber ...",
	"mkfs", "F:o:mb:?V", 
		"[-F FSType] [-V] [-m] [current_options] [-o specific_options] special [operands]",
	"dcopy", "F:o:s:a:f:?dvV", 
		"[-F FSType] [-V] [current_options] [-o specific_options] inputfs outputfs",
	"fsdb", "F:o:z:?V", 
		"[-F FSType] [-V] [current_options] [-o specific_options] special",
	"labelit", "F:o:?nV",
		"[-F FSType] [-V] [current_options] [-o specific_options] special [operands]",
	NULL, "F:o:?V",
	 	"[-F FSType] [-V] [-o specific_options] special [operands]"
	};
struct 	commands *c_ptr;

main(argc, argv)
int	argc;
char	*argv[];
{
	register char *ptr;
	char	full_path[FULLPATH_MAX];
	char	*vfs_path = VFS_PATH;
	char	*alt_path = ALT_PATH;
	int	i; 
        int	verbose = 0;		/* set if -V is specified */
	int	F_flg = 0;
	int	mflag = 0;
	int	usgflag = 0; 
	int	arg;			/* argument from getopt() */
	extern	char *optarg;		/* getopt specific */
	extern	int optind;
	extern	int opterr;

	basename = ptr = argv[0];
	while (*ptr) {
		if (*ptr++ == '/')
			basename = ptr;
	}
	if (argc == 1) {
		for (c_ptr = cmd_data; ((c_ptr->c_basename != NULL) && (strcmp(c_ptr->c_basename, basename) != 0));  c_ptr++) 
		;
		usage(basename, c_ptr->c_usgstr);
		exit(2);
	}

	for (c_ptr = cmd_data; ((c_ptr->c_basename != NULL) && (strcmp(c_ptr->c_basename, basename) != 0));  c_ptr++) 
		; 
	while ((arg = getopt(argc,argv,c_ptr->c_optstr)) != -1) {
			switch(arg) {
			case 'V':	/* echo complete command line */
				verbose = 1;
				break;
			case 'F':	/* FSType specified */
				F_flg++;
				fstype = optarg;
				break;
			case 'o':	/* FSType specific arguments */
				newargv[newargc++] = "-o";
				newargv[newargc++] = optarg; 
				break;
			case '?':	/* print usage message */
				newargv[newargc++] = "-?";
				usgflag = 1;
				break;
			case 'm':	/* FSType specific arguments */
				mflag=1;
				newargv[newargc] = (char *)malloc(3);
				sprintf(newargv[newargc++], "-%c", arg);
				if (optarg)
					newargv[newargc++] = optarg;
				break;
			default:
				newargv[newargc] = (char *)malloc(3);
				sprintf(newargv[newargc++], "-%c", arg);
				if (optarg)
					newargv[newargc++] = optarg;
				break;
			}
			optarg = NULL;
	}
	if (F_flg > 1) {
		fprintf(stderr, "%s: more than one FSType specified\n",
			basename);
		usage(basename, c_ptr->c_usgstr);
		exit (2);
	}
	if (fstype != NULL) {
		if (strlen(fstype) > FSTYPE_MAX) {
			fprintf(stderr, "%s: FSType %s exceeds %d characters\n",
				basename, fstype, FSTYPE_MAX);
			exit (2);
		}
	}

	/*  perform a lookup if fstype is not specified  */
	special = argv[optind];
	optind++;
	if ((special == NULL) && (!usgflag)) {
		fprintf(stderr,"%s: special not specified\n", basename);	
		usage(basename, c_ptr->c_usgstr);
		exit(2);
	}
	if ( (fstype == NULL)  && (usgflag))
		usage(basename, c_ptr->c_usgstr);
	if (fstype == NULL) 
		lookup();
	if (fstype == NULL) {
		fprintf(stderr, "%s: FSType cannot be identified\n",
			basename);
		usage(basename, c_ptr->c_usgstr);
		exit (2);
	}
	newargv[newargc++] = special;
	for (; optind < argc; optind++) 
		newargv[newargc++] = argv[optind];

	/*  build the full pathname of the fstype dependent command  */
	sprintf(full_path, "%s/%s/%s",vfs_path, fstype, basename);

	newargv[1] = &full_path[FULLPATH_MAX];
	while (*newargv[1]-- != '/');
	newargv[1] += 2;

	if (verbose) {
		printf("%s -F %s ", basename, fstype);
		for (i = 2; newargv[i]; i++)
			printf("%s ", newargv[i]);
		printf("\n");
		exit(0);
	}

	/*
	 *  Execute the FSType specific command.
	 */
		if (!strncmp(basename,"mkfs",4)) {
			if ((!access(full_path,2)) && (!mflag) && (!usgflag)) {
				fprintf(stdout,"Mkfs: make %s file system? \n(DEL if wrong)\n",fstype);
				fflush(stdout);
				sleep(10);	/* 10 seconds to DEL */
			}
		}
	execv(full_path, &newargv[1]);
	if ((errno == ENOENT) || (errno == EACCES)) {
		/*  build the alternate pathname */
		sprintf(full_path, "%s/%s/%s",alt_path, fstype, basename);
		if (verbose) {
			printf("%s -F %s ", basename, fstype);
			for (i = 2; newargv[i]; i++)
				printf("%s ", newargv[i]);
			printf("\n");
			exit(0);
		}
		if (!strncmp(basename,"mkfs",4)) {
			if ((!access(full_path,2)) && (!mflag) && (!usgflag)) {
				fprintf(stdout,"Mkfs: make %s file system? \n(DEL if wrong)\n",fstype);
				fflush(stdout);
				sleep(10);	/* 10 seconds to DEL */
			}
		}
		execv(full_path, &newargv[1]);
	}
	if (errno == ENOEXEC) {
		if (!strncmp(basename,"mkfs",4)) {
			if ((!access(full_path,2)) && (!mflag) && (!usgflag)) {
				fprintf(stdout,"Mkfs: make %s file system? \n(DEL if wrong)\n",fstype);
				fflush(stdout);
				sleep(10);	/* 10 seconds to DEL */
			}
		}
		newargv[0] = "sh";
		newargv[1] = full_path;
		execv("/sbin/sh", &newargv[0]);
	}
	if (errno != ENOENT) {
		perror(basename);
		fprintf(stderr, "%s: cannot execute %s\n", basename, full_path);
		exit (2);
	}

	if (sysfs(GETFSIND, fstype) == (-1)) {
		fprintf(stderr, "%s: FSType %s not installed in the kernel\n",
			basename, fstype);
		exit (2);
	}
	fprintf(stderr, "%s: Operation not applicable for FSType %s \n",basename, fstype);
	exit(2);
}

usage (cmd, usg)
char *cmd, *usg;
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s %s\n", cmd, usg);
	exit (2);
}


/*
 *  This looks up the /etc/vfstab entry given the device 'special'.
 *  It is called when the fstype is not specified on the command line.
 *
 *  The following global variables are used:
 *	special, fstype
 */

lookup()
{
	FILE	*fd;
	int	ret;
	struct vfstab	vget, vref;

	if ((fd = fopen(vfstab, "r")) == NULL) {
		fprintf (stderr, "%s: cannot open vfstab\n", basename);
		exit (1);
	}
	vfsnull(&vref);
	vref.vfs_special = special;
	ret = getvfsany(fd, &vget, &vref);
	if (ret == -1) {
		rewind(fd);
		vfsnull(&vref);
		vref.vfs_fsckdev = special;
		ret = getvfsany(fd, &vget, &vref);
	}
	fclose(fd);

	switch (ret) {
	case -1:
		fprintf(stderr, "%s: FSType cannot be identified\n",
			basename);
		exit(1);
		break;
	case 0:
		fstype = vget.vfs_fstype;
		break;
	case VFS_TOOLONG:
		fprintf(stderr, "%s: line in vfstab exceeds %d characters\n",
			basename, VFS_LINE_MAX-2);
		exit(1);
		break;
	case VFS_TOOFEW:
		fprintf(stderr, "%s: line in vfstab has too few entries\n",
			basename);
		exit(1);
		break;
	}
}
