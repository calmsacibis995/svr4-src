/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:makedbm.c	1.4.4.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static  char sccsid[] = "@(#)makedbm.c 1.14 88/09/28 Copyr 1984 Sun Micro";
#endif

#undef NULL
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

#include "ypdefs.h"
#include "ypsym.h"
USE_YP_MASTER_NAME
USE_YP_LAST_MODIFIED
USE_YP_INPUT_FILE
USE_YP_OUTPUT_NAME
USE_YP_DOMAIN_NAME
USE_YP_SECURE
USE_DBM

#define MAXLINE 4096		/* max length of input line */
static char *get_date();
static char *any();

static void addpair();
static void unmake();
static void usage();

main(argc, argv)
	char **argv;
{
	FILE *infp;
	datum key, content, tmp;
	char buf[MAXLINE];
	char pagbuf[MAXPATHLEN];
	char tmppagbuf[MAXPATHLEN];
	char dirbuf[MAXPATHLEN];
	char tmpdirbuf[MAXPATHLEN];
	char *p,ic;
	char *infile, *outfile;
	char outalias[MAXPATHLEN];
	char outaliasmap[MAXNAMLEN];
	char outaliasdomain[MAXNAMLEN];
	char *last_slash, *next_to_last_slash;
	char *infilename, *outfilename, *mastername, *domainname,
	    *security, *lower_case_keys;
	char local_host[MAX_MASTER_NAME];
	int cnt,i;

	infile = outfile = NULL; /* where to get files */
	/* name to imbed in database */
	infilename = outfilename = mastername = domainname = 
	    security = lower_case_keys = NULL; 
	argv++;
	argc--;
	while (argc > 0) {
		if (argv[0][0] == '-' && argv[0][1]) {
			switch(argv[0][1]) {
				case 'i':
					infilename = argv[1];
					argv++;
					argc--;
					break;
				case 'o':
					outfilename = argv[1];
					argv++;
					argc--;
					break;
				case 'm':
					mastername = argv[1];
					argv++;
					argc--;
					break;
				case 'd':
					domainname = argv[1];
					argv++;
					argc--;
					break;
				case 'l':
					lower_case_keys = argv[0];
					break;
				case 's':
					security = argv[0];
					break;
				case 'u':
					unmake(argv[1]);
					argv++;
					argc--;
					exit(0);
				default:
					usage();
			}
		}
		else if (infile == NULL)
			infile = argv[0];
		else if (outfile == NULL)
			outfile = argv[0];
		else
			usage();
		argv++;
		argc--;
	}
	if (infile == NULL || outfile == NULL)
		usage();
	if (strcmp(infile, "-") != 0)
		infp = fopen(infile, "r");
	else
		infp = stdin;
	if (infp == NULL) {
		fprintf(stderr, "makedbm: can't open %s\n", infile);
		exit(1);
	}

	/*
	 *  do alias mapping if necessary
	 */
	last_slash = strrchr(outfile,'/');
	if (last_slash){
		 *last_slash='\0';
		 next_to_last_slash= strrchr(outfile,'/');
		if (next_to_last_slash) *next_to_last_slash='\0';
	}
	else next_to_last_slash = NULL;

#ifdef DEBUG
	if (last_slash) printf("last_slash=%s\n",last_slash+1);
	if (next_to_last_slash) printf("next_to_last_slash=%s\n",
		next_to_last_slash+1);
#endif DEBUG

	/* reads in alias file for system v filename translation */
	sysvconfig();

	if (last_slash && next_to_last_slash) {
		if (yp_getalias(last_slash+1, outaliasmap, MAXALIASLEN) < 0) {
			if ((int)strlen(last_slash+1) <= MAXALIASLEN)
				strcpy(outaliasmap, last_slash+1);
			else
				fprintf(stderr, 
				     "makedbm: warning: no alias for %s\n",
				    last_slash+1);
		}
#ifdef DEBUG
		printf("%s\n",last_slash+1);
		printf("%s\n",outaliasmap);
#endif DEBUG
		if (yp_getalias(next_to_last_slash+1, outaliasdomain, 
		    MAXALIASLEN) < 0) {
			if ((int)strlen(last_slash+1) <= MAXALIASLEN)
				strcpy(outaliasdomain, next_to_last_slash+1);
			else
				fprintf(stderr, 
				    "makedbm: warning: no alias for %s\n",
				    next_to_last_slash+1);
		}
#ifdef DEBUG
		printf("%s\n",next_to_last_slash+1);
		printf("%s\n",outaliasdomain);
#endif DEBUG
		sprintf(outalias,"%s/%s/%s", outfile, outaliasdomain,
			outaliasmap);
#ifdef DEBUG
		printf("outlias=%s\n",outalias);
#endif DEBUG

	} else if (last_slash) {
		if (yp_getalias(last_slash+1, outaliasmap, MAXALIASLEN) < 0){
			if ((int)strlen(last_slash+1) <= MAXALIASLEN)
				strcpy(outaliasmap, last_slash+1);
			else {
				fprintf(stderr, "makedbm: warning: no alias for %s\n",
				    last_slash+1);
			}
		}
		if (yp_getalias(outfile, outaliasdomain, MAXALIASLEN) < 0){
			if ((int)strlen(outfile) <= MAXALIASLEN)
				strcpy(outaliasmap, outfile);
			else
				fprintf(stderr, "makedbm: warning: no alias for %s\n",
				    last_slash+1);
		}	
		sprintf(outalias,"%s/%s", outaliasdomain, outaliasmap);
	} else {
		if (yp_getalias(outfile, outalias, MAXALIASLEN) < 0){
			if ((int)strlen(last_slash+1) <= MAXALIASLEN)
				strcpy(outalias, outfile);
			else 
				fprintf(stderr, "makedbm: warning: no alias for %s\n",
				    outfile);
			}
	}
#ifdef DEBUG
	fprintf(stderr,"outalias=%s\n",outalias);
	fprintf(stderr,"outfile=%s\n",outfile);
#endif DEBUG

	strcpy(tmppagbuf, outalias);
	strcat(tmppagbuf, ".t");
	strcpy(tmpdirbuf, tmppagbuf);
	strcat(tmpdirbuf, dbm_dir);
	strcat(tmppagbuf, dbm_pag);
	if (fopen(tmpdirbuf, "w") == NULL) {
	    	fprintf(stderr, "makedbm: can't create %s\n", tmpdirbuf);
		exit(1);
	}
	if (fopen(tmppagbuf, "w") == NULL) {
	    	fprintf(stderr, "makedbm: can't create %s\n", tmppagbuf);
		exit(1);
	}
	strcpy(dirbuf, outalias);
	strcat(dirbuf, ".t");
	if (dbminit(dirbuf) != 0) {
		fprintf(stderr, "makedbm: can't init %s\n", dirbuf);
		exit(1);
	}
	strcpy(dirbuf, outalias);
	strcpy(pagbuf, outalias);
	strcat(dirbuf, dbm_dir);
	strcat(pagbuf, dbm_pag);
	while (fgets(buf, sizeof(buf), infp) != NULL) {
		p = buf;
		cnt = strlen(buf) - 1; /* erase trailing newline */
		while (p[cnt-1] == '\\') {
			p+=cnt-1;
			if (fgets(p, sizeof(buf)-(p-buf), infp) == NULL)
				goto breakout;
			cnt = strlen(p) - 1;
		}
		p = any(buf, " \t\n");
		key.dptr = buf;
		key.dsize = p - buf;
		for (;;) {
			if (p == NULL || *p == NULL) {
				fprintf(stderr, "makedbm: yikes!\n");
				exit(1);
			}
			if (*p != ' ' && *p != '\t')
				break;
			p++;
		}
		content.dptr = p;
		content.dsize = strlen(p) - 1; /* erase trailing newline */
		if (lower_case_keys) 
			for (i=0; i<key.dsize; i++) {
				ic = *(key.dptr+i);
				if (isascii(ic) && isupper(ic)) 
					*(key.dptr+i) = tolower(ic);
			} 
		tmp = fetch(key);
		if (tmp.dptr == NULL) {
			if (store(key, content) != 0) {
				printf("problem storing %.*s %.*s\n",
				    key.dsize, key.dptr,
				    content.dsize, content.dptr);
				exit(1);
			}
		}
#ifdef DEBUG
		else {
			printf("duplicate: %.*s %.*s\n",
			    key.dsize, key.dptr,
			    content.dsize, content.dptr);
		}
#endif
	}
   breakout:
	addpair(yp_last_modified, get_date(infile));
	if (infilename)
		addpair(yp_input_file, infilename);
	if (outfilename)
		addpair(yp_output_file, outfilename);
	if (domainname)
		addpair(yp_domain_name, domainname);
	if (security)
		addpair(yp_secure, "");
	if (!mastername) {
		gethostname(local_host, sizeof (local_host) - 1);
		mastername = local_host;
	}
	addpair(yp_master_name, mastername);

	sprintf(buf,"mv %s %s", tmppagbuf, pagbuf);
	if (system(buf) < 0)
		perror("makedbm: rename");
	sprintf(buf,"mv %s %s", tmpdirbuf, dirbuf);
	if (system(buf) < 0)
		perror("makedbm: rename");
	return(0);
}


/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

static char *
get_date(name)
	char *name;
{
	struct stat filestat;
	static char ans[MAX_ASCII_ORDER_NUMBER_LENGTH];/* ASCII numeric string*/

	if (strcmp(name, "-") == 0)
		sprintf(ans, "%010d", (long) time(0));
	else {
		if (stat(name, &filestat) < 0) {
			fprintf(stderr, "makedbm: can't stat %s\n", name);
			exit(1);
		}
		sprintf(ans, "%010d", (long) filestat.st_mtime);
	}
	return ans;
}

void
usage()
{
	fprintf(stderr,
"usage: makedbm -u file\n       makedbm [-s] [-i YP_INPUT_FILE] [-o YP_OUTPUT_FILE] [-d YP_DOMAIN_NAME] [-m YP_MASTER_NAME] infile outfile\n");
	exit(1);
}

void
addpair(str1, str2)
	char *str1, *str2;
{
	datum key;
	datum content;
	
	key.dptr = str1;
	key.dsize = strlen(str1);
	content.dptr  = str2;
	content.dsize = strlen(str2);
	if (store(key, content) != 0){
		printf("makedbm: problem storing %.*s %.*s\n",
		    key.dsize, key.dptr, content.dsize, content.dptr);
		exit(1);
	}
}

void
unmake(file)
	char *file;
{
	datum key, content;

	if (file == NULL)
		usage();
	
	if (dbminit(file) != 0) {
		fprintf(stderr, "makedbm: couldn't init %s\n", file);
		exit(1);
	}
	for (key = firstkey(); key.dptr != NULL; key = nextkey(key)) {
		content = fetch(key);
		printf("%.*s %.*s\n", key.dsize, key.dptr,
		    content.dsize, content.dptr);
	}
}
