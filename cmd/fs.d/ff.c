/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fs.cmds:ff.c	1.10.3.1"

#include	<stdio.h>
#include 	<limits.h>
#include	<string.h>
#include	<sys/fstyp.h>  
#include	<sys/errno.h>  
#include	<sys/vfstab.h>
#include	<sys/wait.h>
#include	<sys/types.h>

#define	FSTYPE_MAX	8
#define	FULLPATH_MAX	64
#define	ARGV_MAX	1024
#define	VFS_PATH	"/usr/lib/fs"

extern	int errno;
char	*special = NULL;  /*  device special name  */
char	*fstype = NULL;	  /*  fstype name is filled in here  */
char	*basename;	  /* name of command */
char	*newargv[ARGV_MAX]; 	/* args for the fstype specific command  */
char	vfstab[] = VFSTAB;
	char	full_path[FULLPATH_MAX];
	char	*vfs_path = VFS_PATH;
int	newargc = 2;

struct commands {
	char *c_basename;
	char *c_optstr;
	char *c_usgstr;
} cmd_data[] = {
	"ff", "F:o:p:a:m:c:n:i:?IlsuV",
		"[-F FSType] [-V] [current_options] [-o specific_options] special ...",
	"ncheck", "F:o:?i:asV",
		"[-F FSType] [-V] [current_options] [-o specific_options] [special ...]",
	NULL, "F:o:?V",
	 	"[-F FSType] [-V] [current_options] [-o specific_options] special ..."
	};
struct 	commands *c_ptr;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE *fp;
	struct vfstab	vfsbuf;
	register char *ptr;
	int	i; 
        int	verbose = 0;		/* set if -V is specified */
	int	F_flg = 0;
	int	usgflag = 0; 
	int	fs_flag = 0; 
	int	arg;			/* argument from getopt() */
	extern	char *optarg;		/* getopt specific */
	extern	int optind;
	extern	int opterr;
	size_t	strlen();

	basename = ptr = argv[0];
	while (*ptr) {
		if (*ptr++ == '/')
			basename = ptr;
	}
	/*
 	* If there are no arguments and command is ncheck then the generic 
	* reads the VFSTAB and executes the specific module of
	* each entry which has a numeric fsckpass field.
 	*/

	if (argc == 1) {		/* no arguments or options */
		if (strcmp(basename, "ncheck") == 0) {
			/* open VFSTAB */
			if (( fp = fopen(VFSTAB, "r")) == NULL){
				fprintf(stderr, "%s: cannot open vfstab\n", basename);
				exit(2);
			}
			while (( i = getvfsent(fp, &vfsbuf)) == 0) {
				if (numbers(vfsbuf.vfs_fsckpass)){
					fstype = vfsbuf.vfs_fstype;
					newargv[newargc]  = vfsbuf.vfs_special;
					exec_specific();
				}	
 			}
			exit(0);
		} 
		fprintf(stderr, "Usage:\n");
	 	fprintf(stderr, "%s [-F FSType] [-V] [current_options] [-o specific_options] special ...\n", basename);
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
	}
	if (strlen(fstype) > FSTYPE_MAX) {
		fprintf(stderr, "%s: FSType %s exceeds %d characters\n",
			basename, fstype, FSTYPE_MAX);
		exit (2);
	}
	if (optind == argc) {
		/* all commands except ncheck must exit now */
		if (strcmp(basename, "ncheck") != 0) {
			if ((F_flg) && (usgflag)) {
				exec_specific();
				exit(0);
			}
			usage(basename, c_ptr->c_usgstr);
		}
		if ((F_flg) && (usgflag)) {
			exec_specific();
			exit(0);
		}
		if (usgflag)
			usage(basename, c_ptr->c_usgstr);

		/* open VFSTAB */
		if (( fp = fopen(VFSTAB, "r")) == NULL){
			fprintf(stderr, "%s: cannot open vfstab\n", basename);
			exit(2);
		}
		while (( i = getvfsent(fp, &vfsbuf)) == 0) {
			if (!numbers(vfsbuf.vfs_fsckpass)) 
				continue;
			if ((F_flg) && (strcmp(fstype, vfsbuf.vfs_fstype) != 0))
				continue;
			fs_flag++;
			newargv[newargc] = vfsbuf.vfs_special;
			if (verbose) {
				printf("%s -F %s ", basename, vfsbuf.vfs_fstype);
				for (i = 2; newargv[i]; i++)
					printf("%s\n", newargv[i]);
				continue;
			}
			exec_specific();
		}
		/* if (! fs_flag) {
			if (sysfs(GETFSIND, fstype) == (-1)) {
				fprintf(stderr, "%s: FSType %s not installed in the kernel\n",
					basename, fstype);
				exit(1);
			}
		} */	

		exit(0);
	}

	/* All other arguments must be specials */
	/*  perform a lookup if fstype is not specified  */

	for (; optind < argc; optind++)  {
		newargv[newargc] = argv[optind];
		special = newargv[newargc];
		if ((F_flg) && (usgflag)) {
			exec_specific();
			exit(0);
		}
		if (usgflag)
			usage(basename, c_ptr->c_usgstr);
		if (fstype == NULL) 
			lookup();
		if (verbose) {
			printf("%s -F %s ", basename, fstype);
			for (i = 2; newargv[i]; i++)
				printf("%s ", newargv[i]);
			printf("\n");
			continue;
		}
		exec_specific();
		if (!F_flg)
			fstype = NULL;
	}
	exit(0);
}

/* see if all numbers */
numbers(yp)
	char	*yp;
{
	if (yp == NULL)
		return	0;
	while ('0' <= *yp && *yp <= '9')
		yp++;
	if (*yp)
		return	0;
	return	1;
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
			basename, special);
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
	case VFS_TOOMANY:
		fprintf(stderr, "%s: line in vfstab has too many entries\n",
			basename);
		exit(1);
		break;
	}
}
exec_specific()
{
int status,pid,ret;

	sprintf(full_path, "%s/%s/%s",vfs_path, fstype, basename);
	newargv[1] = &full_path[FULLPATH_MAX];
	while (*newargv[1]-- != '/');
	newargv[1] += 2;
	switch(pid=fork()) {
	case 0:
		execv(full_path, &newargv[1]);
		if (errno == ENOEXEC) {
			newargv[0] = "sh";
			newargv[1] = full_path;	
			execv("/sbin/sh", &newargv[0]);
		}
		if (errno != ENOENT) {
			perror(basename);
			fprintf(stderr, "%s: cannot execute %s\n", basename, full_path);
			exit(1);
		}
		if (sysfs(GETFSIND, fstype) == (-1)) {
			fprintf(stderr, "%s: FSType %s not installed in the kernel\n",
				basename, fstype);
			exit(1);
		}
		fprintf(stderr, "%s: operation not applicable for FSType %s\n", basename,fstype);
		exit(1);
	case -1:
		fprintf(stderr,"%s: cannot fork process\n",basename);
		exit(2);
	default:
		/* if cannot exec specific, or fstype is not installed, exit
		   after first 'exec_specific' to avoid printing duplicate
		   error messages */

		if (wait(&status) == pid) {
			ret=WHIBYTE(status);
			if (ret > 0){
				exit(ret);
			}
		}
	}
}
