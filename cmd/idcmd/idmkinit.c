/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idmkinit.c	1.3"

/* This program reads Init files in /etc/conf/init.d/* and gerneates
 * inittab entries. These entries are combined with /etc/conf/cf.d/init.base
 * to produce /etc/conf/cf.d/inittab.
 * An inittab entry has the form:
 *	id:rstate:action:process
 * The files in /etc/conf/init.d must have the form:
 *	CASE 1: action:process   , OR
 *	CASE 2: rstate:action:process
 *	CASE 3: id:rstate:action:process
 * Idmkinit will insert the fields:
 *	CASE 1:  id:rstate
 *	CASE 2:  id
 *	CASE 3:  	(nothing)
 * The command line options are:
 *	-o directory	- an alternate installation directory.
 *	-i directory	- the directory containing init.base.
 *	-e directory	- the directory containing the Init files.
 *	-#		- print diagnostics.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include "inst.h"

/* directories */
#define	ENVIRON		0
#define INPUT		1
#define OUTPUT		2
#define FULL_PATH	3

/* error messages */
#define USAGE	"Usage: idmkinit [-i dir] [-o dir] [-e dir]"
#define	CHDIR	"Can not chdir to %s"
#define	OPEN	"%s: can not open for mode %s"

/* misc. */
#define	FOUND		0
#define	UNFOUND		1

/* directories */
char current[80];		/* current directory */
char envirmnt[80];		/* path name of init directory */
char input[80];			/* path name of init.base directory */
char output[80];		/* path name of '/dev' directory */

/* flags */
int iflag;			/* 'init.base' directory specified */
int oflag;			/* 'etc' directory specified */
int eflag;			/* 'init.d' directory specified */
int debug;			/* debug flag */
int errors;			/* number of errors */
int colons;			/* number of colons */

/* new inittab variables */
FILE *initp;			/* file pointer to new inittab */
int initid;			/* two character id used in field 1 */

FILE *open1();
DIR  *open2();
char errbuf[100];		/* hold error messages */
extern char *optarg;		/* used by getopt */

main(argc, argv)
int argc;
char *argv[];
{
	char buf[100];
	int c;

	while ((c = getopt(argc, argv, "i:o:e:#?")) != EOF)
		switch (c) {
		case 'e':	/* contains init files */
			strcpy(envirmnt, optarg);
			eflag++;
			break;
		case 'o':	/* /dev directory */
			strcpy(output, optarg);
			oflag++;
			break;
		case 'i':	/* contains init.base */
			strcpy(input, optarg);
			iflag++;
			break;
		case '#':
			debug++;
			break;
		case '?':
			sprintf(errbuf, USAGE);
			error(1);
		}

	/* get current directory */
	getcwd(current, 80);

	/* get full path name */
	sprintf(buf, "%s/init.d", ROOT);
	getpath(eflag, envirmnt, buf);
	sprintf(buf, "%s/cf.d", ROOT);
	getpath(iflag, input, buf);
	getpath(oflag, output, buf);

	if (debug) {
		fprintf(stderr, "debug:\tinput=%s\n\toutput=%s\n\tenvirmnt=%s\n",
			input, output, envirmnt);
		fprintf(stderr, "\tcurrent=%s\n", current);
	}

	cpinit();		/* copy init.base to inittab */
	mkinit();		/* add new entries */
	fclose(initp);
	exit(errors);
}



/* copy init.base to inittab */

cpinit()
{
	FILE *basep;
	char buf[100];

	basep = open1("init.base", "r", INPUT);
	initp = open1("inittab", "w", OUTPUT);

	while (fgets(buf, 100, basep) != NULL)
		fputs(buf, initp);

	/* check if last line has a newline character */
	if (strchr(buf, '\n') == NULL)
		fputc('\n', initp);

	fclose(basep);
}



/* get Init files from environment directory (/etc/conf/init.d) */

mkinit()
{
	struct dirent *direntp;
	DIR *dp;

	dp = open2(envirmnt);
	while (direntp = readdir (dp)) {
		if (debug)
			fprintf(stderr, "debug: file='%s'\n", direntp->d_name);
		if (direntp->d_ino == 0 || direntp->d_name[0] == '.')
			continue;
		install(direntp->d_name);
	}
	closedir(dp);
}



/* Add Init Files from init.d  */

install(file)
char *file;
{
	FILE *ip;			/* pointer to node file */
	char buf[516], *p;

	ip = open1(file, "r", ENVIRON);
	while (fgets(buf, 516, ip) != NULL) {

		if (debug)
			fprintf(stderr, "debug: file='%s'\n\tentry='%s'\n",
				file, buf);
		/* Reject comment and blank lines */
		if (buf[0] == '#' || buf[0] == '\n' )
			continue;  
		/* check syntax of line */
		if (parse(buf)==FOUND){
			fprintf(initp, "%02d:2:%s", initid, buf);
			++initid;
		}
		else{
		    switch(colons){

		     case 1:
			fprintf(initp, "%02d:%s", initid, buf);
			++initid;
			break;

		     case 2:
			fprintf(initp, "%s", buf);
			break;

		     default:
			sprintf(errbuf, "Cannot parse line: %s", buf);
			error(0);
		  	break;
		    }
		}
	}
	fclose(ip);
}


parse(buf)
char *buf;
{
	int i;
	if (lookup(buf) == FOUND)
		return(FOUND);
	else{
		/* non 6300PLUS style; try to parse further */
		colons=0;
		for (i=0; i < (int)strlen(buf); i++){
			if (buf[i] == ':'){
				colons++;
				if (lookup(&buf[i+1])==FOUND)
					return(colons);
			}
		}
		colons = -1;
		return(UNFOUND);
	}
}

/* check for legitimate action keyword */

lookup(sptr)
char * sptr;
{

	static char *actions[] = {
		"off","respawn","ondemand","once","wait","boot",
		"bootwait","powerfail","powerwait","initdefault",
		"sysinit",
	};
	int i;
	char **keyptr;

	for (i=0,keyptr=actions; i<sizeof(actions)/sizeof(char *);i++) {
		if (!strncmp(keyptr[i], sptr, sizeof(keyptr[i])-1))
			return(FOUND);
	}
	return(UNFOUND);
}

/* open a directory */

DIR *
open2(directory)
char *directory;
{
	DIR *dp;

	if (debug)
		fprintf(stderr, "debug: open directory '%s' for mode 'r'\n",
			directory);

	if ((dp = opendir(directory)) == NULL) {
		sprintf(errbuf, OPEN, directory, "r");
		error(1);
	}
	return(dp);
}

/* open a file */

FILE *
open1(file, mode, dir)
char *file, *mode;
int dir;
{
	FILE *fp;
	char *p;
	char path[80];

	switch (dir) {
	case ENVIRON:
		sprintf(path, "%s/%s", envirmnt, file);
		p = path;
		break;
	case INPUT:
		sprintf(path, "%s/%s", input, file);
		p = path;
		break;
	case OUTPUT:
		sprintf(path, "%s/%s", output, file);
		p = path;
		break;
	case FULL_PATH:
		p = file;
		break;
	}

	if (debug)
		fprintf(stderr, "debug: open '%s' for mode '%s'\n",
			p, mode);

	if ((fp = fopen(p, mode)) == NULL) {
		sprintf(errbuf, OPEN, file, mode);
		error(1);
	}
	return(fp);
}



error(xit)
int xit;
{
	fprintf(stderr, "idmkinit: %s\n", errbuf);
	errors++;
	if (xit)
		exit(errors);
}



/* construct full path name */

getpath(flag, buf, def)
int flag;
char *buf, *def;
{
	switch (flag) {
	case 0:
		strcpy(buf, def);
		break;
	case 1:
		if (chdir(buf) != 0) {
			sprintf(errbuf, CHDIR, buf);
			error(1);
		}
		getcwd(buf, 80);
		chdir(current);
		break;
	}
}
