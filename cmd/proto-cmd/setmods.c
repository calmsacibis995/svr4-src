/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:setmods.c	1.3.1.1"

/* Setmods takes as input the following: */
/* mode (tab) owner (tab) group (tab) filename */
/* Setmods will take the information and do respective */
/* chown and chmod system calls. */

/*
 *	to create empty files/directories and special nodes.
 *	Also, blank lines and lines beginning with '#' are skipped.
 *
 *	Additional fields (tab separated, of course) are:
 *	<TAB> type <TAB> major <TAB> minor
 * or for links,
 *	<TAB> type <TAB> parent_path
*/

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BIN 2
#define COMMENT '#'
#define TMP_PFX	"smod"

typedef short	BOOLEAN;

char	buf[BUFSIZ];
BOOLEAN no_create=0, silent=0, lasttime=0;

int lines, saved;
FILE	*savfp;
struct p_entry {
	int mode;		/* file permissions (octal) */
	char	owner[9],	/* file owner		*/
		group[9],	/* file group		*/
		name[BUFSIZ];	/* file name		*/
	char type;		/* file type (d,p,c,b or l) */
	int	maj,		/* 	major device number */
		min;		/*	minor device number */
} pe;

int	args;

main(argc,argv)
int argc;
char *argv[];
{
	extern int errno;
	extern int optind, opterr;
	extern char *optarg;
	extern struct p_entry pe;
	FILE	*fp;
	int	tfd;	/* scratch file descriptor */
	int gid, uid, i;
	int defaultuid, defaultgid;
	char	*tempnam(),
		*savfile,
		c;
	struct group *g, *getgrnam();
	struct passwd *p, *getpwnam();

	/* Parse command-line args */
	while ((c = getopt(argc, argv, "lns")) != -1)
		switch (c) {
			case 'l':	/* internal flag for recursion */
				lasttime=1;
				break;
			case 'n':	/* don't create missing files */
				no_create = 1;
				break;
			case 's':	/* don't report on missing files */
				silent = 1;
				break;
			case '?':
				fprintf(stderr,"USAGE: %s [-n] [-s] [Plist]\n",
						argv[0]);
				exit(10);
		}

	if ( optind == argc)
		fp=stdin;
	else
		if((fp=fopen(argv[optind], "r")) == NULL){
			fprintf(stderr,"%s: Cannot open %s\n",argv[0],argv[optind]);
			exit(1);
		}

	if (!lasttime) {
		savfile=tempnam((char *)0,TMP_PFX);
		if((savfp=fopen(savfile, "w")) == NULL){
			fprintf(stderr,"%s: Can't open %s\n",argv[0],savfile);
			exit(2);
		}
	}

	lines = 0;

	/* get default values for uid & gid */
	if((p=getpwnam("bin")) != NULL)
		defaultuid=p->pw_uid;
	else
		defaultuid=BIN;

	if((g=getgrnam("bin")) != NULL)
		defaultgid=g->gr_gid;
	else
		defaultgid=BIN;

	for(lines = 1; fgets(buf, BUFSIZ, fp) != NULL; lines++) {
		if(buf[0] == '\n' || buf[0] == COMMENT)
			continue;

		args = sscanf(buf,"%o%8s%8s%s%1s%d%d",
			&pe.mode, pe.owner, pe.group, pe.name,
			&pe.type, &pe.maj, &pe.min);

		if (args < 4) {
			fprintf(stderr,"Line %d: has less than 4 arguments\n", lines);
			continue;
		}

		/* Check for links - special case. */
		if ( (args > 4) && (pe.type == 'l') ) {
			dolink();
			continue;
		}

		if((p=getpwnam(pe.owner)) != NULL)
			uid=p->pw_uid;
		else {
			fprintf(stderr, "Line %d: cannot find %s in passwd file.  Using bin as owner.\n", lines, pe.owner);
			uid=defaultuid;
		}

		if((g=getgrnam(pe.group)) != NULL)
			gid=g->gr_gid;
		else {
			fprintf(stderr, "Line %d: cannot find %s in group file.  Using bin as group.\n", lines, pe.group);
			gid=defaultgid;
		}

		if (access(pe.name, 00) == -1) {
			if(!silent)
				fprintf(stderr,
				  "Line %d: %s does not exist", lines, pe.name);
			if(no_create) {
				if(!silent) fprintf(stderr, "- Skipped.\n");
				continue;
			}
			else if(!silent) fprintf(stderr, "- Creating !\n");

			if ((args == 5) &&
			   ((pe.type == 'x') || (pe.type == 'v') || (pe.type == 'f')))
				args = 4;
			if (args == 4)
				if((tfd=creat(pe.name, pe.mode)) == -1) {
					sprintf(buf,"Failed to create %s",pe.name);
					perror(buf);
					continue;
				}
				else close(tfd);
			else if (pe.type == 'd')
				mkdir(pe.name, pe.mode);
			else if (pe.type == 'l') {
				dolink(buf);
				continue;
			}
			else if (pe.type == 'p') {
				sprintf(buf, "mknod %s p", pe.name);
				system(buf);
			}
			else if (pe.type == 'b' || pe.type == 'c') {
				sprintf(buf, "mknod %s %c %d %d",
					pe.name, pe.type, pe.maj, pe.min);
				system(buf);
			}
			else {
				fprintf(stderr,"Unknown type: '%c'\n", pe.type);
				continue;
			}

		}
		/* Set MODE */
		if((chmod(pe.name, pe.mode)) == -1){
			if (errno == ENOENT) 
				fprintf(stderr, "Line %d: %s does not exist\n",
						lines, pe.name);
			else
				fprintf(stderr, "Line %d: cannot chmod %s; errno = %d\n",
						lines, pe.name, errno);
			continue;
		}

		/* Set OWNER & GROUP */
		if((chown(pe.name, uid, gid) == -1)) {
			if (errno == ENOENT)
				fprintf(stderr,"Line %d: %s does not exist\n",
					lines, pe.name);
			else
				fprintf(stderr,"Lines %d: cannot chown or chgrp %s; errno = %d\n",
					lines, pe.name, errno);
			continue;
		}
	}
	fclose(fp);
	if (!lasttime) fclose(savfp);

	if (lasttime || !saved)	unlink(savfile);
	else {	/* recurse to pick up saved entries */
		sprintf(buf, "%s -l %s %s", argv[0], (silent?"-s":""), savfile);
		if (!silent)
		   fprintf(stderr,"%s: Phase II - recursing.\n", argv[0]);
		system(buf);
	}

	exit(0);
}


dolink(line)
char line[];
{
	extern	BOOLEAN lasttime;

	char	parent[BUFSIZ],

	args = sscanf(buf,"%o%8s%8s%s%1s%s",
		&pe.mode, pe.owner, pe.group, pe.name, &pe.type, parent);
	if (args < 1 ) {
		fprintf(stderr,"%d: cannot determine parent file for link.\n", lines);
		return(-1);
	}
	if( link(parent,pe.name) == -1 ) {
		if (lasttime) { /* lasttime and link attempt failed */
			sprintf(buf,"%d:: cannot link %s - %s",
						lines, parent, pe.name);
			perror(buf);
			return(-1);
		}
		switch(errno) {
		  case EXDEV:	/* different filesystems */
			xcopy(parent,pe.name);
			break;
		  case ENOENT:	/* source file doesn't exist */
			/* record entry for 2nd try */
			fprintf(savfp,"%s",buf);
			++saved;
			break;
		  case EEXIST:	/* file2 already exists */
			unlink(pe.name);
			if(link(parent,pe.name) == 0) break;
		  default:
			sprintf(buf,"%d: cannot link %s - %s",
				lines, parent, pe.name);
			perror(buf);
			return(-1);
		}
	}

	return(1);	/* success */
}


xcopy(orig,target)	/* copy file if linked to different FS */
char *orig, *target;
{
	struct stat s_buf;	/* to determine uid/gid of src file */
	int fd_i, fd_o;		/* temporary file descriptors */
	int ret;

	if((fd_i=open(orig, O_RDONLY)) == -1) {
		sprintf(buf,"Can't open %s for copy.",orig);
		perror(buf);
		return(-1);	/* failure */
	}
	else if (fstat(fd_i, (struct stat *)&s_buf) == -1) {
		sprintf(buf,"Can't stat %s for copy.",orig);
		perror(buf);
		close(fd_i);
		return(-1);	/* failure */
	}
	if ((fd_o=open(target,O_WRONLY|O_CREAT|O_EXCL,(int)s_buf.st_mode)) == -1 ) {
		sprintf(buf,"Can't open %s for copy.",target);
		perror(buf);
		close(fd_i);
		return(-2);	/* error */
	}

	while( (ret=read(fd_i, buf, (unsigned)sizeof(buf))) != 0) {
		if(ret == -1 ) {
			sprintf(buf,"Read on Copy failed.");
			perror(buf);
			break;
		}
		if( write(fd_o, buf, (unsigned)ret) == -1) {
			sprintf(buf,"Copy from %s to %s failed.", orig, target);
			perror(buf);
			break;
		}
	}
	close(fd_i);
	close(fd_o);

	if((chown(target, (int)s_buf.st_uid, (int)s_buf.st_gid) == -1)) {
		sprintf(buf,"Cannot chown or chgrp for %s", target);
		perror(buf);
		return(-3);	/* error */
	}

	return(0);	/* normal return */
}
