/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:alias.c	1.5.4.1"
/*
    NAME
	mailalias - look up alias names

    SYNOPSIS
	mailalias [-s] [-v] name ...

    DESCRIPTION
	Look up the name in the alias files. First look in
	$HOME/lib/names, then in the files listed in
	/etc/mail/namefiles. For each name found at the
	beginning of a line, output the remainder of the line.

	If -s is not specified and multiple names are requested,
	each line is prefixed with the name being looked up.

	If -v is specified, debugging output is given.
*/
#include <ctype.h>
#include <stdio.h>
#include "mail.h"
#include "config.h"

/* predeclared */
#ifdef __STDC__
string *getdbfiles(void);
int translate_name(char *name, string *files, string *alias);
int lookup(char *name, string *file, string *alias);
int compare(string *s1, char *p2);
#else
string *getdbfiles();
int translate_name();
int lookup();
int compare();
#endif

char *progname = "";
int verbose, simple;

usage()
{
    (void) printf("Usage: %s [-s] [-v] name ...\n", progname);
    exit(1);
}

/* loop through the names to be translated */
main(argc, argv)
	int argc;
	char *argv[];
{
	string *alias;		/* the alias for the name */
	string *files;		/* list of files to search */
	int i;

	progname = argv[0];

	while ((i = getopt(argc, argv, "sv")) != -1)
		switch (i) {
		case 's': simple++; break;
		case 'v': verbose++; break;
		case '?': usage();
		}

	/* get environmental info */
	files = getdbfiles();
	alias = s_new();

	/* loop through the names to be translated (from standard input) */
	if (optind == (argc-1))
		simple++;

	for (i = optind; i < argc; i++) {
		s_reset(alias);
		(void)translate_name(argv[i], files, alias);
		if (!simple)
			printf("%s\t", argv[i]);
		if (*s_to_c(alias) == '\0')
			printf("%s\n", argv[i]);
		else
			printf("%s\n", s_to_c(alias));
		fflush(stdout);
	}
	return 0;
}

/* get the list of dbfiles to search */
string *
getdbfiles()
{
	FILE *fp;
	string *files = s_new();

	/* system wide aliases */
	if (chdir(libdir) < 0) {
		perror("mailalias(chdir):");
		return files;
	}
	if ((fp = fopen(sysalias, "r")) != NULL) {
		while (s_gettoken(fp, files) != NULL)
			s_append(files, " ");
		fclose(fp);
	}

	return files;
}

/* loop through the translation files */
int
translate_name(name, files, alias)
	char *name;		/* name to translate */
	string *files;
	string *alias;		/* where to put the alias */
{
	string *file = s_new();
	char *home;

	if (verbose)
		printf("translate_name(%s, %s, %s)\n", name,
			s_to_c(files), s_to_c(alias));

	/* look at user's local names */
	s_restart(file);
	home = getenv("HOME");
	if (home != NULL) {
		s_append(file, home);
		s_append(file, useralias);
		if (lookup(name, file, alias)==0) {
			s_free(file);
			return 0;
		}
	}

	/* look at system-wide names */
	s_restart(files);
	while (s_parse(files, s_restart(file)) != NULL) {
		if (lookup(name, file, alias)==0) {
			s_free(file);
			return 0;
		}
	}

	/* first look for mailbox */
	s_restart(file);
	abspath(name, maildir, file);
	if (access(s_to_c(file), 0) == 0) {
		s_append(alias, name);
		s_free(file);
		return 0;
	}

	return -1;
}

/*  Loop through the entries in a translation file looking for a match.
 *  Return 0 if found, -1 otherwise.
 */
int
lookup(name, file, alias)
	char *name;
	string *file;
	string *alias;	/* returned string */
{
	FILE *fp;
	string *line = s_new();
	string *token = s_new();
	struct stat st;
	int rv = -1;

	if (verbose)
		printf("lookup(%s, %s, %s)\n", name,
			s_to_c(file), s_to_c(alias));

	s_reset(alias);

	/*  Check if file is really a directory.
	 *  If it is, use file/file instead.
	 */
	if (stat(s_to_c(file), &st)==0 && (st.st_mode & S_IFMT)==S_IFDIR) {
		s_append(file, "/");
		s_append(file, name);
	}

	if ((fp = fopen(s_to_c(file), "r")) == NULL)
		return -1;

	/* look for a match */
	while (s_getline(fp, s_restart(line))!=NULL) {
		if (s_parse(s_restart(line), s_restart(token))==NULL)
			continue;

		if (compare(token, name)!=0)
			continue;

		/* match found, get the alias */
		while (s_parse(line, s_restart(token))!=NULL) {
			/* avoid definition loops */
			if (compare(token, name)==0) {
				s_append(alias, name);
			} else {
				s_append(alias, s_to_c(token));
			}
			s_append(alias, " ");
		}
		rv = 0;
		break;
	}

	s_free(line);
	s_free(token);
	fclose(fp);
	return rv;
}

/* compare two strings (case insensitive) */
#define lower(c) (isupper(c)?c-('A'-'a'):c)
int
compare(s1, p2)
	string *s1;
	register char *p2;
{
	register char *p1 = s_to_c(s1);
	register int rv;

	while(1) {
		rv = lower(*p1) - lower(*p2);
		if (rv)
			return rv;
		if (*p1 == '\0')
			return 0;
		p1++;
		p2++;
	}
}
