/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:contents.c	1.3.1.1"

/*
** Based on setmods
** Create a contents file based on the Plist
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

static unsigned int sum();
static char *dolink();
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

main(argc,argv)
int argc;
char *argv[];
{
	extern int errno;
	extern int optind, opterr;
	extern char *optarg;
	extern struct p_entry pe;
	struct stat st_buf;
	FILE	*fp;
	int	tfd;	/* scratch file descriptor */
	int gid, uid, i;
	int defaultuid, defaultgid;
	char	*tempnam(),
		*savfile,
		c;
	struct group *g, *getgrnam();
	struct passwd *p, *getpwnam();
	int	args;

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

	for(lines = 1; fgets(buf, BUFSIZ, fp) != NULL; lines++) {
		if(buf[0] == '\n' || buf[0] == COMMENT)
			continue;

		pe.type = 'f';	/* initialize to file */
		args = sscanf(buf,"%o%8s%8s%s%1s%d%d",
			&pe.mode, pe.owner, pe.group, pe.name,
			&pe.type, &pe.maj, &pe.min);

		if (access (pe.name, 00) != 0)
			continue;

		if (args < 4) {
			fprintf(stderr,"Line %d: has less than 4 arguments\n", lines);
			continue;
		}

		stat (pe.name, &st_buf);

		switch (pe.type) {
		case 'd': fprintf (stdout ,"/%s d none %.4o %s %s foundation\n", pe.name,
pe.mode, pe.owner, pe.group);
			  break;
		case 'l': fprintf (stdout ,"/%s=/%s l none foundation\n", pe.name, dolink());
			  break;
		case 'x': 	/* Exclude these */
			  break;
		default:
		
			fprintf (stdout ,"/%s %c none %.4o %s %s %d %u %d foundation\n",
pe.name, ((args > 4)?pe.type :'f'), pe.mode, pe.owner, pe.group, st_buf.st_size, sum(pe.name), st_buf.st_mtime);
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

static char *
dolink()
{
	char	parent[BUFSIZ];

	sscanf(buf,"%o%8s%8s%s%1s%s",
		&pe.mode, pe.owner, pe.group, pe.name, &pe.type, parent);
	return (parent);
}

/*
 * sum - based on sum.c
*/

#define WDMSK 0177777L
struct part {
	short unsigned hi,lo;
};
union hilo { /* this only works right in case short is 1/2 of long */
	struct part hl;
	long	lg;
} tempa, suma;

static unsigned int
sum(name)
char *name;
{
	register FILE *f;
	int	ca;
	unsigned lsavhi,lsavlo;


	if ((f = fopen(name, "r")) == NULL) {
		if (!silent)
		   (void) fprintf(stderr, "contents - sum: Can't open %s\n", name);
		return (0);
	}

	suma.lg = 0;

	while((ca = getc(f)) != EOF) {
		suma.lg += ca & WDMSK;
	}

	if(ferror(f)) {
		if (!silent)
		   (void) fprintf(stderr, "contents - sum: read error on %s\n", name);
		return (0);
	}
	(void) fclose(f);

	tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
	lsavhi = (unsigned) tempa.hl.hi;
	lsavlo = (unsigned) tempa.hl.lo;
	
	return ((unsigned)(lsavhi + lsavlo));
}
