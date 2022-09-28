/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)fs.cmds:df.c	1.34.8.1"
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/vfstab.h>
#include <sys/mnttab.h>
#include <sys/param.h>		/* for MAXNAMELEN - used in devnm() */
#include <dirent.h> 	
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MNTTAB	"/etc/mnttab"
#define VFSTAB	"/etc/vfstab"
#define REMOTE_FST	"rfs"
#define MAX_OPTIONS	20	/* max command line options */

char *argp[MAX_OPTIONS];	/* command line for specific module*/
int argpc;
int status;
char path[BUFSIZ];
int k_header = 0;
int header = 0;
int eb_flg = 0;
unsigned long used;
unsigned long avail;
unsigned long kbytes;
unsigned long capacity;

#define DEVLEN 1024	/* for devnm() */
struct stat S;
struct stat Sbuf;
char *devnm();
char *basename();

/* Generic DF */

/*
 * This is the generic part of the df code. It is used as a
 * switchout mechanism and in turn executes the file system
 * type specific code located at /usr/lib/fs/FSType. 
 *
 */

#ifdef i386
	int	v_flg = 0,
		FreeBlocks,
		TotalBlocks,
		UsedBlocks;
#endif

main (argc, argv)
int argc;
char *argv[];

{
	int arg;			/* argument from getopt() */
	int usgflg, b_flg, e_flg, V_flg, o_flg, k_flg;
	int t_flg, g_flg, n_flg, l_flg, f_flg, F_flg; 
	int i, j;
	int status;

	mode_t mode;

	extern char *optarg;		/* getopt(3c) specific */
	extern int optind;
	extern int opterr;
	char	 *res_name, *s;
	int	 errcnt = 0;
	int	 exitcode = 0;
	int	 notfound=1;
	int	 res_found=0;

	char *FSType = NULL;		/* FSType */
	char *oargs;			/* FSType specific argument */
	char *cmdname; 			/* command name or path */
	char options[MAX_OPTIONS];	/* options for specific module */

#ifdef i386
	char *usage = "Usage:\ndf [-F FSType] [-begklntVv] [current_options] [-o specific_options] [directory | special ...]\n";
#else
	char *usage = "Usage:\ndf [-F FSType] [-begklntV] [current_options] [-o specific_options] [directory | special ...]\n";
#endif

	FILE *fp, *fp2, *fdopen();
	struct mnttab mountb;
	struct mnttab mm;
	struct statvfs statbuf;
	struct vfstab	vfsbuf;
	struct vfstab ss;
	struct stat stbuf;

	usgflg = b_flg = e_flg = V_flg = k_flg = eb_flg = 0;
	t_flg = g_flg = n_flg = l_flg = f_flg = 0; 


	F_flg = o_flg = 0;
	cmdname = argv[0];	
	strcpy(options, "-");

	/* see if the command called is devnm */

	s = basename(argv[0]);
	if(!strncmp(s,"devnm",5)) {

		while(--argc) {
			if(stat(*++argv, &S) == -1) {
				fprintf(stderr, "devnm: ");
				errcnt++;
				continue;
			}
			res_name = devnm();
			if(res_name[0] != '\0')
				printf("%s %s\n", res_name, *argv);
			else {
				fprintf(stderr, "devnm: %s not found\n", *argv);
				errcnt++;
			}
		}
		exit(errcnt);
	}

	/* open MNTTAB and VFSTAB */

	if (( fp = fopen(MNTTAB, "r")) == NULL){
		fprintf(stderr, "%s: cannot open mnttab\n", cmdname);
		exit(2);
	}

	if (( fp2 = fopen(VFSTAB, "r")) == NULL){
		fprintf(stderr, "%s: cannot open vfstab\n", cmdname);
		exit(2);
	}

	/* 
 	* If there are no arguments to df then the generic 
 	* determines the file systems mounted from /etc/mnttab
 	* and does a statvfs on them and reports on the generic
 	* superblock information
 	*/

	if (argc == 1) {		/* no arguments or options */

		while (( i = getmntent(fp, &mountb)) == 0 ) {
			if ((j = statvfs(mountb.mnt_mountp, &statbuf)) != 0){
				fprintf(stderr, "df: cannot statvfs %s\n", mountb.mnt_mountp);
				continue;
			}
			print_statvfs(&mountb, &statbuf, "x");
 		}
		if (i > 0 ) {
			mnterror(i,cmdname);
			exit(1);
		}
		exit(exitcode);
	}

	/* One or more options or arguments */
	/* Process the Options */ 

#ifdef i386
	while ((arg = getopt(argc,argv,"F:o:?bekVtgnlfv")) != -1) {
#else
	while ((arg = getopt(argc,argv,"F:o:?bekVtgnlf")) != -1) {
#endif

		switch(arg) {

#ifdef i386
		case 'v':	/* print verbose output */
			printf("Mount Dir  Filesystem           blocks      used      free  %%used\n");
			v_flg = 1;
			break;
#endif

		case 'b':	/* print kilobytes free */
			b_flg = 1;
			strcat(options, "b");
			break;
		case 'e':	/* print file entries free */
			e_flg = 1;
			strcat(options, "e");
			break;
		case 'V':	/* echo complete command line */
			V_flg = 1;
			break;
		case 't':	/* full listing with totals */
			t_flg = 1;
			strcat(options, "t");
			break;
		case 'g':	/* print entire statvfs structure */
			g_flg = 1;
			strcat(options, "g");
			break;
		case 'n':	/* print FSType name */
			n_flg = 1;
			strcat(options, "n");
			break;
		case 'l':	/* report on local File systems only */
			l_flg = 1;
			strcat(options, "l");
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
				fprintf(stderr, "df: FSType %s exceeds 8 characters\n", FSType);
				exit(2);
			}
			break;
		case 'o':	/* FSType specific arguments */
			o_flg = 1;
			oargs = optarg;
			break;
		case 'f':	/* perform actual count on free list */
			f_flg = 1;
			strcat(options, "f");
			break;
		case 'k':	/* new format */
			k_flg = 1;
			strcat(options, "k");
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
	if (b_flg && e_flg) {
		eb_flg++;
	}

	/* if no arguments (only options): process mounted file systems only */
	if (optind == argc) {
		if (V_flg) {
			while ((i = getmntent(fp, &mountb)) == 0 ) {
				if ((F_flg) && (strcmp(FSType, mountb.mnt_fstype) != 0))
					continue;
				if ((l_flg) && (strcmp(mountb.mnt_fstype, REMOTE_FST) == 0))
					continue;
				mk_cmdline(mountb.mnt_fstype,
						options,
						o_flg,
						oargs,
						mountb.mnt_special);
		  		echo_cmdline(argp, argpc, mountb.mnt_fstype);
		 	}
			if (i > 0 ) {
				mnterror(i,cmdname);
				exit(1);
			}
		 	exit(0);
		}
		if (f_flg || o_flg) {	/* specific df */
			while ((i = getmntent(fp, &mountb)) == 0 ) {
				if ((F_flg) && ( strcmp(FSType, mountb.mnt_fstype) != 0))
					continue;
				mk_cmdline(mountb.mnt_fstype,
						options,
						o_flg,
						oargs,
						mountb.mnt_special);
				build_path(mountb.mnt_fstype, path);
				exec_specific(mountb.mnt_fstype); 
			}
			if (i > 0 ) {
				mnterror(i,cmdname);
				exit(1);
			}
			exit(0);
		} /* end specific df */

		if ( F_flg && usgflg ) {
			mk_cmdline(FSType,
					options,
					o_flg,
					oargs,
					NULL);
			build_path(FSType, path);
			exec_specific(FSType);
			exit(0); 	/* only one usage mesg required */
		}
		/* f or o flags are not set */
		while (( i = getmntent(fp, &mountb)) == 0 ) {
			if ((F_flg) && (strcmp(FSType, mountb.mnt_fstype) != 0))
				continue;
	    		if ((l_flg) && (strcmp(mountb.mnt_fstype, REMOTE_FST) == 0)) 
				continue;
			if ((j = statvfs(mountb.mnt_mountp, &statbuf)) != 0){
				fprintf(stderr,"df: cannot statvfs %s\n", mountb.mnt_mountp); 
				continue;
			}
			if (g_flg) {
				print_statvfs(&mountb, &statbuf, 'g');
				continue;
			}
			if (k_flg) {
				print_statvfs(&mountb, &statbuf, 'k');
				continue;
			}

#ifdef i386
			if (v_flg) {
				print_statvfs(&mountb, &statbuf, 'v');
				continue;
			}
#endif

			if (t_flg) {
				print_statvfs(&mountb, &statbuf, 't');
				continue;
			}
			if (b_flg){
				print_statvfs(&mountb, &statbuf, 'b');
			}
			if (e_flg){
				print_statvfs(&mountb, &statbuf, 'e');
			}
			if (n_flg) {
				print_statvfs(&mountb, &statbuf, 'n');
			}
			if (b_flg || n_flg || e_flg)  
				continue;
			if (F_flg){
				print_statvfs(&mountb, &statbuf, 'x');
				continue;
			}
			print_statvfs(&mountb, &statbuf, 'x');
 		}
		if (i > 0 ) {
			mnterror(i,cmdname);
			exit(1);
		}
		exit(0);

	}  /* end case of no arguments */

	/* arguments could be mounted/unmounted file systems */
	for (; optind < argc; optind++) {

		fclose(fp);
		fclose(fp2);
		if (( fp = fopen(MNTTAB, "r")) == NULL){
			fprintf(stderr,"%s: cannot open mnttab\n", cmdname);
			exit(2);
		}
		if (( fp2 = fopen(VFSTAB, "r")) == NULL){
			fprintf(stderr, "%s: cannot open vfstab\n", cmdname);
			exit(2);
		}

		mntnull(&mm);
		mm.mnt_special = argv[optind];

		/* case argument is special from mount table */
		if ((i = getmntany(fp, &mountb, &mm)) == 0){

			if ((l_flg) && (strcmp(mountb.mnt_fstype, REMOTE_FST) == 0))
				continue;
			if ((F_flg)&&(strcmp(mountb.mnt_fstype, FSType) != 0)){
				fprintf(stderr, "df: Warning: %s mounted as a %s file system\n", mm.mnt_special, mountb.mnt_fstype);	
				exitcode=1;
				continue;
			}
			if (V_flg) {
				if (!F_flg) 
					FSType = mountb.mnt_fstype;
				mk_cmdline(FSType,
						options,
						o_flg,
						oargs,
						mountb.mnt_special);
				echo_cmdline(argp, argpc, FSType);
				continue;
			}
			if (f_flg || o_flg) {
				if (!F_flg) 
					FSType = mountb.mnt_fstype;
				mk_cmdline(FSType,
						options,
						o_flg,
						oargs,
						mountb.mnt_special);
				build_path(FSType, path);
				exec_specific(FSType);
				continue;
			}
			if ((j = statvfs(mountb.mnt_mountp, &statbuf)) != 0) {
				fprintf(stderr, "df: cannot statvfs %s\n", mountb.mnt_mountp); 
				exit(2);
			}
			if (F_flg && usgflg) {
				mk_cmdline(FSType,
					options,
					o_flg,
					oargs,
					mountb.mnt_special);
				exec_specific(FSType);
				exit(0);
			}
			if (g_flg) {
				print_statvfs(&mountb, &statbuf, 'g');
				continue;
			}
			if (k_flg) {
				print_statvfs(&mountb, &statbuf, 'k');
				continue;
			}
			if (t_flg) {
				print_statvfs(&mountb, &statbuf, 't');
				continue;
			}
			if (n_flg) {
				print_statvfs(&mountb, &statbuf,'n');
			}
			if (b_flg) {
				print_statvfs(&mountb, &statbuf, 'b');
			}
			if (e_flg) {
				print_statvfs(&mountb, &statbuf, 'e');
			}
			if ( b_flg || e_flg || n_flg )
				continue;
			if (F_flg) {
				print_statvfs(&mountb, &statbuf, 'x');
				continue;
			}
			print_statvfs(&mountb, &statbuf, 'x');
			continue;
		}

		/* perform a stat(2) to determine file type */

		/* stat fails */
		if (( i = stat(argv[optind], &stbuf)) == -1) {
			fprintf(stderr, "%s: cannot stat %s\n", cmdname, argv[optind]);
		 	exit(2);	
		}
		if ((( stbuf.st_mode & S_IFMT) == S_IFREG) ||
			(( stbuf.st_mode & S_IFMT) == S_IFIFO )) {
			fprintf(stderr, "df: (%-10.32s) not a file system, directory or mounted resource\n", argv[optind]);
			continue;
		}

		/* if block or character device */

		if ((( mode = ( stbuf.st_mode & S_IFMT )) == S_IFBLK) || 
			( mode == S_IFCHR )) {

			/* check if the device exists in vfstab */
			if (( i = getvfsspec(fp2, &vfsbuf, argv[optind])) != 0) {
				if (!F_flg) {
					fprintf(stderr, "%s: FSType cannot be identified\n", cmdname);
					exit(2);	
				}
				mk_cmdline(FSType, 
					options,
					o_flg,
					oargs,
					argv[optind]);
				if (V_flg) {
					echo_cmdline(argp, argpc, FSType);
					continue;
				}
				build_path(FSType, path);
				exec_specific(FSType);
				continue;
			}

			/* device exists in vfstab */
			if (!F_flg) 
				FSType = vfsbuf.vfs_fstype;
			if ( g_flg || n_flg || l_flg ){
				fprintf(stderr, "df: options g, n or l not supported for unmounted FSTypes\n");
				exit(2);
			}
			mk_cmdline(FSType, 
					options,
					o_flg,
					oargs,
					vfsbuf.vfs_special);
			if (V_flg) {
				echo_cmdline(argp, argpc, FSType);
				continue;
			}
			build_path(FSType, path);
			exec_specific(FSType);
			continue;

		} /* end: block or character device */
		/* argument is a path */

		if ((j = statvfs(argv[optind], &statbuf)) != 0) {
			 fprintf(stderr, "df: cannot statvfs %s\n", mountb.mnt_mountp); 
			exit(2);
		}
		if ((F_flg)&&(strcmp(statbuf.f_basetype, FSType) != 0)){
			fprintf(stderr, "df: Warning: %s mounted as a %s file system\n", argv[optind], statbuf.f_basetype);	
			exitcode=1;

			continue;
		}
		if (V_flg) {
			mk_cmdline(statbuf.f_basetype,
					options,
					o_flg,
					oargs,
					argv[optind]);
			echo_cmdline(argp, argpc, statbuf.f_basetype);
			continue;
		}
		if (f_flg || o_flg) {
			mk_cmdline(statbuf.f_basetype,
					options,
					o_flg,
					oargs,	
					argv[optind]);
			build_path(statbuf.f_basetype, path);
			exec_specific(statbuf.f_basetype);
			continue;
		}
		/* rest handled by generic */
		mountb.mnt_mountp = argv[optind];	/* mount pt is file */
		if (( i = stat(argv[optind], &S)) == -1) {
			fprintf(stderr, "%s: cannot stat %s\n", cmdname, argv[optind]);
		 	exit(2);	
		}
		res_name = devnm();
		/* Even if the resource name is found here, we may not
		   have the correct mountpoint(in the case where a path
		   was given below a mountpoint, ie, /usr is the mntpt,
		   but the argument given was /usr/include/sys.)
		*/
		   
		if(res_name[0] != '\0') {
			res_found++;
		}
			fclose(fp);
			if (( fp = fopen(MNTTAB, "r")) == NULL) {
				fprintf(stderr,"%s: cannot open mnttab\n", cmdname);
				exit(2);
			}
			mntnull(&mm);
			mm.mnt_mountp = argv[optind];
			/* case argument is mountpoint from mount table */
			/* if argument is a path below a mountpoint, then sat
			   each entry in the mttab and check if the device no.
			   matches.  
			*/
			if ((i = getmntany(fp, &mountb, &mm)) != 0){
				if (i < 0) {
					fclose(fp);
					if (( fp = fopen(MNTTAB, "r")) == NULL){
						fprintf(stderr,"%s: cannot open mnttab\n", cmdname);
						exit(2);
					}
					mntnull(&mountb);
					if (stat(argv[optind], &S) == -1) {
						fprintf(stderr, "%s: cannot stat %s\n", cmdname, argv[optind]);
		 				exit(2);	
					}

					while (getmntent(fp,&mountb) == 0) {
						if (stat(mountb.mnt_mountp,&Sbuf) <0 )  {
							fprintf(stderr, "%s: cannot stat %s\n", cmdname, argv[optind]);
		 					exit(2);	
						
						}
						if (S.st_dev == Sbuf.st_dev) {
							notfound=0;
							break;
						}
					}
					/* argument may be a path with a mounted
					   resource under it. ie, arg=/mnt/var,
					   where /mnt is mounted via rfs and /var
					   is a mounted file system.  In this case
					   stat will not return the same device
					   numbers so a match will never be found.
					   Since we have the info from statvfs print
					   it anyway.
					*/

					if (notfound) {
						mountb.mnt_mountp=argv[optind];
						if (res_found) {
							mountb.mnt_special=res_name; 
							res_found=0;
						}
						else {
							mountb.mnt_special="***************"; 
						}
					}
				}
			}

		mountb.mnt_fstype  = statbuf.f_basetype;
		if ((l_flg) && (strcmp(mountb.mnt_fstype, REMOTE_FST) == 0)) 
			continue;
		if (g_flg) {
			print_statvfs(&mountb, &statbuf, 'g');
			continue;
		}
		if (k_flg) {
			print_statvfs(&mountb, &statbuf, 'k');
			continue;
		}

#ifdef i386
		if (v_flg) {
			print_statvfs(&mountb, &statbuf, 'v');
			continue;
		}
#endif

		if (t_flg) {
			print_statvfs(&mountb, &statbuf, 't');
			continue;
		}
		if (b_flg) 
			print_statvfs(&mountb, &statbuf, 'b');
		if (e_flg) 
			print_statvfs(&mountb, &statbuf, 'e');
		if (n_flg) 
			print_statvfs(&mountb, &statbuf,'n');
		if (b_flg || n_flg || e_flg) 
			continue;
		print_statvfs(&mountb, &statbuf, 'x');

	}	/* end: for all arguments */
	exit(0);
}	/* end main */

echo_cmdline(argp, argpc, fstype)
char *argp[];
int argpc;
char *fstype;
{
	int i;
	printf("%s", argp[0]);
	if (fstype != NULL )	
		printf(" -F %s", fstype);
	for( i= 1; i < argpc; i++) 
	        printf(" %s", argp[i]);
	printf("\n");

}

mnterror(flag,cmdname)
	int	flag;
	char	*cmdname;
{
	switch (flag) {
	case MNT_TOOLONG:
		fprintf(stderr, "%s: line in mnttab exceeds %d characters\n",
			cmdname, MNT_LINE_MAX-2);
		break;
	case MNT_TOOFEW:
		fprintf(stderr, "%s: line in mnttab has too few entries\n",
			cmdname);
		break;
	case MNT_TOOMANY:
		fprintf(stderr, "%s: line in mnttab has too many entries\n",
			cmdname);
		break;
	}
	exit(1);
}

/* function to generate command line to be passed to specific */

mk_cmdline(fstype, options, o_flg, oargs, argument)
char *fstype;
char *options;
int  o_flg;
char *oargs;
char *argument;
{


	argpc = 0;
	argp[argpc++] = "df";
	if (strcmp(options, "-") != 0)
		argp[argpc++] = options;
	if (o_flg) {
		argp[argpc++] = "-o";
		argp[argpc++] = oargs;
	}
	argp[argpc++] = argument;
	argp[argpc++] = NULL;
}


print_statvfs(mountb, statbuf, flag)
struct mnttab *mountb;
struct statvfs *statbuf;
int flag;
{
	int physblks;

	physblks=statbuf->f_frsize/512;
	switch(flag) {

#ifdef i386
	case 'v':
		TotalBlocks = statbuf->f_blocks*physblks;
		UsedBlocks = statbuf->f_blocks*physblks - statbuf->f_bfree*physblks;
		FreeBlocks = statbuf->f_bfree*physblks;

		printf("%-10.10s %-17.17s %9ld %9ld %9ld %4d%%\n",
			mountb->mnt_mountp, 
			mountb->mnt_special,
			TotalBlocks,
			UsedBlocks,
			FreeBlocks,
			TotalBlocks ? (100 * UsedBlocks / TotalBlocks) : 0);
		break;
#endif

	case 'g':
		printf("%-18s(%-15s):  %6u block size  %7u frag size\n", 
			mountb->mnt_mountp, 
			mountb->mnt_special,
			statbuf->f_bsize, 
			statbuf->f_frsize);
		printf("%7u total blocks%7u free blocks%7u available   %7d total files\n", 
			statbuf->f_blocks*physblks, 
			statbuf->f_bfree*physblks,
			statbuf->f_bavail*physblks,
			statbuf->f_files);
		printf("%7d free files  %7u filesys id %32s \n", 
			statbuf->f_ffree,
			statbuf->f_fsid,
			statbuf->f_fstr);
		printf("%7s fstype   0x%8.8X flag       %7d filename length\n\n", 
			statbuf->f_basetype,
			statbuf->f_flag,
			statbuf->f_namemax);
		break;

	case 't':
		printf("%-19s(%-16s):  %8d blocks%8d files\n", 
			mountb->mnt_mountp, 
			mountb->mnt_special,
			statbuf->f_bfree*physblks, 
			statbuf->f_ffree);
		printf("                                total:\t%8d blocks%8d files\n",
			statbuf->f_blocks*physblks,
			statbuf->f_files);
		break;

	case 'b':
		if (eb_flg) {
			printf("%-19s(%-16s):  %8d kilobytes\n", 
				mountb->mnt_mountp, 
				mountb->mnt_special,
				((statbuf->f_bfree*physblks*512)/1024));
		}
		else {
			if (!header) {
				printf("Filesystem             avail\n");
				header++;
			}
			printf("%-16s       %-8d\n", 
				mountb->mnt_special,
				((statbuf->f_bfree*physblks*512)/1024));
		}
		break;

	case 'e':
		if (eb_flg) {
			printf("%-19s(%-16s):  %8d files\n", 
			mountb->mnt_mountp, 
			mountb->mnt_special,
			statbuf->f_ffree);
		}
		else {
			if (!header) {
				printf("Filesystem             ifree\n");
				header++;
			}
			printf("%-16s       %-8d\n", 
				mountb->mnt_special, statbuf->f_ffree);
		}
		break;

	case 'n':
		printf("%-19s: %-10s\n",
			mountb->mnt_mountp,
			statbuf->f_basetype);
		break;
	case 'k':
		if (!k_header) {
			printf("filesystem         kbytes   used     avail    capacity  mounted on\n");
			k_header = 1;
		} 
		kbytes = statbuf->f_blocks*statbuf->f_frsize/1024;
		avail = statbuf->f_bavail*statbuf->f_frsize/1024; 
		used = kbytes - avail;
		capacity = kbytes > 0 ? (((used * 100.0)/kbytes) + 0.5) :0;
		printf("%-18s %-8ld %-8ld %-8ld %2ld%%       %-19s\n",
			mountb->mnt_special,
			kbytes,
			used,
			avail,
			capacity,
			mountb->mnt_mountp);
		break;

        default:	
		printf("%-19s(%-16s):%8d blocks%8d files\n", 
				mountb->mnt_mountp, 
				mountb->mnt_special,
				statbuf->f_bfree*physblks, 
				statbuf->f_ffree);
		break;
	}
}

int
build_path(FSType, path)
char *FSType;
char *path;
{

	strcpy(path, "/usr/lib/fs/");
	strcat(path, FSType );
	strcat(path, "/df");
 	return 0;	
}


exec_specific(FSType)
char *FSType;
{
int  pid,c_ret;

	switch(pid = fork()) {
	case (pid_t)-1:
		fprintf(stderr, "df: cannot fork process\n");
		exit(2);

	case 0:	
		if (execvp(path, argp) == -1) {
			if (errno == EACCES) {
				fprintf(stderr, "df: cannot execute %s\n", path);
				exit(2);
			}
			fprintf(stderr, "df: operation not applicable for FSType %s\n", FSType);
			exit(2);
		}
		exit(2);
		
	default:
		if (wait(&status) == pid) {
			if ((c_ret=WHIBYTE(status)) != 0){
				exit(c_ret);
			}
		}
	} 
}

/* code used by devnm */
char *
basename(s)
char *s;
{
	int n = 0;

	while(*s++ != '\0') n++;
	while(n-- > 0)
		if(*(--s) == '/') 
			return(++s);
	return(--s);
}

struct dirent *dbufp;

char *
devnm()
{
	int i;
	static dev_t fno;
	static struct devs {
		char *devdir;
		DIR *dfd;
	} devd[] = {		/* in order of desired search */
		"/dev/dsk",0,
		"/dev",0,
		"/dev/rdsk",0
	};
	static char devnam[DEVLEN];

	devnam[0] = '\0';
	if(!devd[1].dfd) {	/* if /dev isn't open, nothing happens */
		for(i = 0; i < 3; i++) {
			devd[i].dfd = opendir(devd[i].devdir);
		}
	}
	fno = S.st_dev;

	for(i = 0; i < 3; i++) {
		   if ((chdir(devd[i].devdir) == 0) 
		   && (dsearch(devd[i].dfd,fno))) {
			strcpy(devnam, devd[i].devdir);
			strcat(devnam,"/");
			strncat(devnam,dbufp->d_name,MAXNAMELEN);
			return(devnam);
		}
	}
	return(devnam);

}

dsearch(ddir,fno)
DIR *ddir;
dev_t fno;
{
	lseek(ddir, (long)0, 0);
	while((dbufp=readdir(ddir)) != (struct dirent *)NULL) {
		if(!dbufp->d_ino) continue;
		if(stat(dbufp->d_name, &S) == -1) {
			fprintf(stderr, "devnm: cannot stat %s\n",dbufp->d_name);
			return(0);
		}
		if((fno != S.st_rdev) 
		|| ((S.st_mode & S_IFMT) != S_IFBLK)
		|| (strcmp(dbufp->d_name,"swap") == 0)
		|| (strcmp(dbufp->d_name,"pipe") == 0)
			) continue;
		return(1);
	}
	return(0);
}
