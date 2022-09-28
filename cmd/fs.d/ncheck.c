/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fs.cmds:ncheck.c	1.5.4.1"
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/vfstab.h>
#include <string.h>
#include <fcntl.h>

#define VFSTAB	"/etc/vfstab"
#define MAX_OPTIONS	20	/* max command line options */

char *argp[MAX_OPTIONS];	/* command line for specific module*/
int argpc;
int status;
char path[BUFSIZ];

/* Generic NCHECK */

/*
 * This is the generic part of the ncheck code. It is used as a
 * switchout mechanism and in turn executes the file system
 * type specific code located at /usr/lib/fs/FSType. 
 */

main (argc, argv)
int argc;
char *argv[];

{
	int arg;			/* argument from getopt() */
	int i_flg;			/* current options */
	int F_flg, o_flg, V_flg, usgflg;/* generic flags */
	int i;

	extern char *optarg;		/* getopt(3c) specific */
	extern int optind;

	char *FSType = NULL;		/* FSType */
	char *oargs = NULL;		/* FSType specific argument */
	char *iargs = NULL;		/* arguments for option i */
	char *cmdname = NULL; 		/* command name or path */
	char *special = NULL;		/* special device */
	char options[MAX_OPTIONS];	/* options for specific module */

	char *usage = "Usage:\n"
	"ncheck [-F FSType] [-V] [current_options] [-o specific_options] [special ...]\n"; 

	FILE *fp;
	struct vfstab	vfsbuf;

	i_flg =  0; 
	F_flg = V_flg = o_flg = usgflg =  0;
	cmdname = argv[0];	
	strcpy(options, "-");

	/* open VFSTAB */
	if (( fp = fopen(VFSTAB, "r")) == NULL){
		fprintf(stderr, "%s: cannot open vfstab\n", cmdname);
		exit(2);
	}

	/* 
 	* If there are no arguments to ncheck then the generic 
	* reads the VFSTAB and executes the specific module of
	* each entry which has a numeric fsckpass field.
 	*/

	if (argc == 1) {		/* no arguments or options */
		while (( i = getvfsent(fp, &vfsbuf)) == 0) {
			if (strcmp(vfsbuf.vfs_fsckpass, NULL) != 0){
				FSType = vfsbuf.vfs_fstype;
				special = vfsbuf.vfs_special;
				mk_cmdline( options, 
						o_flg, 
						oargs, 
						i_flg, 
						iargs, 
						special);
				build_path(FSType, path);
				exec_specific(FSType);
			}	
 		}
		exit(0);
	}

	/* One or more options or arguments */
	/* Process the Options */ 
	while ((arg = getopt(argc,argv,"V?F:o:i:as")) != -1) {
		switch(arg) {
		case 'a':	/* allows printing of names . and ..*/
			strcat(options, "a");
			break;
		case 's':	/*limits report to specials & SetUID/GID files */
			strcat(options, "s");
			break;
		case 'V':	/* echo complete command line */
			V_flg = 1;
			break;
		case 'F':	/* FSType specified */
			if (F_flg) {
				fprintf(stderr, "%s: more than one FSType specified\n", cmdname);
				fprintf(stderr, usage);
				exit(2);
			}
			F_flg = 1;
			FSType = optarg;
			if (( i = strlen(FSType)) > 8 ) {
				fprintf(stderr, "FSType name %s exceeds 8 characters\n", FSType);
			}
			break;
		case 'o':	/* FSType specific arguments */
			o_flg = 1;
			oargs = optarg;
			break;
		case 'i':	/* limit report to files whose i-numbers follow */
			i_flg = 1;
			iargs = optarg;
			break;
		case '?':	/* print usage message */
			usgflg = 1;
			strcat(options, "?");
		}
	}
	if ((!F_flg) && (usgflg)) {
		fprintf(stderr, usage);
		exit(2);
	}

	/* if no arguments (only options): */
	if (optind == argc) {	
		while (( i = getvfsent(fp, &vfsbuf)) == 0) {
			if (strcmp(vfsbuf.vfs_fsckpass, NULL) == 0) 
				continue;
			if ((F_flg) && (strcmp(FSType, vfsbuf.vfs_fstype) != 0))
				continue;
			mk_cmdline( options,
					o_flg,
					oargs,
					i_flg,
					iargs,
					vfsbuf.vfs_special);
			build_path(vfsbuf.vfs_fstype, path);
			if ((F_flg) && (usgflg)) {
				exec_specific(vfsbuf.vfs_fstype);
				exit(0);
			}
			if (V_flg)  {
				echo_cmdline(argp, argpc, vfsbuf.vfs_fstype);
				continue;
			}
			exec_specific(vfsbuf.vfs_fstype);
		}
		exit(0);
	}
	/* special provided */
	for (; optind < argc; optind++ ) {
		if (F_flg) {
			mk_cmdline( options,
					o_flg,
					oargs,
					i_flg,
					iargs,
					argv[optind]);
			build_path(FSType, path);
			if (V_flg) {
				echo_cmdline(argp, argpc, FSType);
				continue;
			}
			exec_specific(FSType);
			continue;
		}
		/* FSType has to be determined from vfstab */
		while ((( i = getvfsent(fp, &vfsbuf)) == 0)  &&
		 	(strcmp(vfsbuf.vfs_special, argv[optind]) != 0))
				continue;
		if (i == 0) {		/* an entry matches */
			if ((F_flg) && (strcmp(FSType, vfsbuf.vfs_fstype) != 0)) {
				fprintf(stderr, "%s: %s is not of type %s\n", cmdname, argv[optind], FSType);
				exit(2);
			}
			mk_cmdline( options,
					o_flg,
					oargs,
					i_flg,
					iargs,
					vfsbuf.vfs_special);
			build_path(vfsbuf.vfs_fstype, path);
			if (V_flg) {
				echo_cmdline(argp, argpc, vfsbuf.vfs_fstype);
				continue;
			}
			exec_specific(vfsbuf.vfs_fstype);
		}
		fprintf(stderr, "ncheck: FSType cannot be identified\n");
	}

}	/* end main */

echo_cmdline(argp, argpc, fstype)
char *argp[];
int argpc;
char *fstype;
{
	int i;
	printf("ncheck ");
	if (strcmp(fstype, NULL) != 0)	
		printf("-F %s ", fstype);
	for( i= 1; i < argpc; i++) 
	        printf("%s ", argp[i]);
	printf("\n");

}

/* function to generate command line to be passed to specific */

mk_cmdline(options, o_flg, oargs, i_flg, iargs, argument)
char *options;
int  o_flg;
char *oargs;
int i_flg;
char *iargs;
char *argument;
{


	argpc = 0;
	argp[argpc++] = "ncheck";
	if (strcmp(options, "-") != 0)
		argp[argpc++] = options;
	if (i_flg) {
		argp[argpc++] = "-i";
		argp[argpc++] = iargs;
	}
	if (o_flg) {
		argp[argpc++] = "-o";
		argp[argpc++] = oargs;
	}
	argp[argpc++] = argument;
	argp[argpc++] = NULL;
}


int
build_path(FSType, path)
char *FSType;
char *path;
{
	strcpy(path, "/usr/lib/fs/");
	strcat(path, FSType );
	strcat(path, "/ncheck");
 	return 0;	
}

int
exec_specific(FSType)
char *FSType;
{
	switch( fork()) {
	case -1:
		fprintf(stderr, "ncheck: cannot fork process\n");
		exit(2);
		break;

	case 0:	
		if ((execvp(path, argp)) == -1) {
			fprintf(stderr, "ncheck: cannot execute %s\n", path);
		}
		exit(2);
		break;
		
	default:
		wait(&status);

	} 
}
