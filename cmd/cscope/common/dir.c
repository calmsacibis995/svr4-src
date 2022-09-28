/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/dir.c	1.3"
/*	cscope - interactive C symbol cross-reference
 *
 *	directory searching functions
 */

#include <sys/types.h>	/* needed by stat.h */
#include <sys/stat.h>	/* stat */
#include <dirent.h>
#include "global.h"
#include "vp.h"		/* vpdirs and vpndirs */

#define	DIRSEPS	" ,:"	/* directory list separators */
#define HASHMOD	2003	/* must be a prime number */
#define	SRCINC	HASHMOD	/* source file list size increment */
			/* largest known database had 22049 files */

char	currentdir[PATHLEN + 1];/* current directory */
char	**incdirs;		/* #include directories */
char	**srcdirs;		/* source directories */
char	**srcfiles;		/* source files */
int	nincdirs;		/* number of #include directories */
int	nsrcdirs;		/* number of source directories */
int	nsrcfiles;		/* number of source files */
int	msrcfiles = SRCINC;	/* maximum number of source files */

static	char	**incnames;	/* #include directory names without view pathing */
static	int	nvpsrcdirs;	/* number of view path source directories */

static	struct	listitem {	/* source file names without view pathing */
	char	*text;
	struct	listitem *next;
} *srcnames[HASHMOD];

BOOL	issrcfile();
void	addsrcdir(), addincdir();

/* make the view source directory list */

void
makevpsrcdirs()
{
	int	i;

	/* return if this function has already been called */
	if (nsrcdirs > 0) {
		return;
	}
	/* get the current directory name */
	if (getwd(currentdir) == NULL) {
		(void) fprintf(stderr, "cscope: warning: cannot get current directory name\n");
		(void) strcpy(currentdir, "<unknown>");
	}
	/* see if there is a view path and this directory is in it */
	vpinit(currentdir);
	if (vpndirs > 1) {
		nsrcdirs = vpndirs;
	}
	else {
		nsrcdirs = 1;
	}
	/* create the source directory list */
	srcdirs = (char **) mymalloc(nsrcdirs * sizeof(char *));
	*srcdirs = ".";	/* first source dir is always current dir */
	for (i = 1; i < vpndirs; ++i) {
		srcdirs[i] = vpdirs[i];
	}
	/* save the number of original source directories in the view path */
	nvpsrcdirs = nsrcdirs;
}

/* add a source directory to the list for each view path source directory */

void
sourcedir(dirlist)
char	*dirlist;
{
	char	path[PATHLEN + 1];
	register char	*dir;
	register int	i;

	makevpsrcdirs();		/* make the view source directory list */
	dirlist = stralloc(dirlist);	/* don't change environment variable text */
	
	/* parse the directory list */
	dir = strtok(dirlist, DIRSEPS);
	while (dir != NULL) {
		addsrcdir(dir);

		/* if it isn't a full path name and there is a 
		   multi-directory view path */
		if (*dirlist != '/' && vpndirs > 1) {
			
			/* compute its path from higher view path source dirs */
			for (i = 1; i < nvpsrcdirs; ++i) {
				(void) sprintf(path, "%s/%s", srcdirs[i], dir);
				addsrcdir(path);
			}
		}
		dir = strtok((char *) NULL, DIRSEPS);
	}
}

/* add a source directory to the list */

void
addsrcdir(dir)
char	*dir;
{
	struct	stat	statstruct;

	/* make sure it is a directory */
	if (stat(compath(dir), &statstruct) == 0 && 
	    (statstruct.st_mode & S_IFDIR)) {

		/* note: there already is a source directory list */
		srcdirs = (char **) myrealloc(srcdirs, (nsrcdirs + 1) * sizeof(char *));
		srcdirs[nsrcdirs++] = stralloc(dir);
	}
}

/* add a #include directory to the list for each view path source directory */

void
includedir(dirlist)
char	*dirlist;
{
	char	path[PATHLEN + 1];
	register char	*dir;
	register int	i;

	makevpsrcdirs();		/* make the view source directory list */
	dirlist = stralloc(dirlist);	/* don't change environment variable text */
	
	/* parse the directory list */
	dir = strtok(dirlist, DIRSEPS);
	while (dir != NULL) {
		addincdir(dir, dir);

		/* if it isn't a full path name and there is a 
		   multi-directory view path */
		if (*dirlist != '/' && vpndirs > 1) {
			
			/* compute its path from higher view path source dirs */
			for (i = 1; i < nvpsrcdirs; ++i) {
				(void) sprintf(path, "%s/%s", srcdirs[i], dir);
				addincdir(dir, path);
			}
		}
		dir = strtok((char *) NULL, DIRSEPS);
	}
}

/* add a #include directory to the list */

void
addincdir(name, path)
char	*name;
char	*path;
{
	struct	stat	statstruct;

	/* make sure it is a directory */
	if (stat(compath(path), &statstruct) == 0 && 
	    (statstruct.st_mode & S_IFDIR)) {
		if (incdirs == NULL) {
			incdirs = (char **) mymalloc(sizeof(char *));
			incnames = (char **) mymalloc(sizeof(char *));
		}
		else {
			incdirs = (char **) myrealloc(incdirs, 
				(nincdirs + 1) * sizeof(char *));
			incnames = (char **) myrealloc(incnames, 
				(nincdirs + 1) * sizeof(char *));
		}
		incdirs[nincdirs] = stralloc(path);
		incnames[nincdirs++] = stralloc(name);
	}
}

/* make the source file list */

void
makefilelist()
{
	DIR	*dirfile;		/* directory file descriptor */
	struct	dirent	*entry;		/* directory entry pointer */
	FILE	*names;			/* name file pointer */
	char	dir[PATHLEN + 1];
	char	path[PATHLEN + 1];
	register char	*file;
	register char	*s;
	register int	i;

	makevpsrcdirs();	/* make the view source directory list */

	/* if there are source file arguments */
	if (fileargc > 0) {
		
		/* put them in a list that can be expanded */
		for (i = 0; i < fileargc; ++i) {
			file = fileargv[i];
			if (infilelist(file) == NO) {
				if ((s = inviewpath(file)) != NULL) {
					addsrcfile(file, s);
				}
				else {
					(void) fprintf(stderr, "cscope: cannot find file %s\n",
					    file);
					errorsfound = YES;
				}
			}
		}
		return;
	}
	/* see if a file name file exists */
	if (namefile == NULL && vpaccess(NAMEFILE, READ) == 0) {
		namefile = NAMEFILE;
	}
	/* if there is a file of source file names */
	if (namefile != NULL) {
		if ((names = vpfopen(namefile, "r")) == NULL) {
			(void) fprintf(stderr, "cscope: cannot open file %s\n", namefile);
			myexit(1);
		}
		/* get the names in the file */
		while (fscanf(names, "%s", path) == 1) {
			if (*path == '-') {	/* if an option */
				i = path[1];
				switch (i) {
				case 'T':	/* truncate symbols to 8 characters */
					truncate = YES;
					break;
				case 'I':	/* #include file directory */
				case 'p':	/* file path components to display */
					s = path + 2;		/* for "-Ipath" */
					if (*s == '\0') {	/* if "-I path" */
						(void) fscanf(names, "%s", path);
						s = path;
					}
					switch (i) {
					case 'I':	/* #include file directory */
						shellpath(dir, sizeof(dir), s);	/* expand $ and ~ */
						includedir(dir);
						break;
					case 'p':	/* file path components to display */
						if (*s < '0' || *s > '9') {
							(void) fprintf(stderr, "cscope: -p option in file %s: missing or invalid numeric value\n", 
								namefile);
						}
						dispcomponents = atoi(s);
						break;
					}
					break;
				default:
					(void) fprintf(stderr, "cscope: only -I, -p, and -T options can be in file %s\n", 
						namefile);
				}
			}
			else if ((s = inviewpath(path)) != NULL) {
				addsrcfile(path, s);
			}
			else {
				(void) fprintf(stderr, "cscope: cannot find file %s\n",
				    path);
				errorsfound = YES;
			}
		}
		(void) fclose(names);
		return;
	}
	/* make a list of all the source files in the directories */
	for (i = 0; i < nsrcdirs; ++i) {

		/* open the directory */
		/* note: failure is allowed because SOURCEDIRS may not exist */
		if ((dirfile = opendir(srcdirs[i])) != NULL) {

			/* read each entry in the directory */
			while ((entry = readdir(dirfile)) != NULL) {
				
				/* if it is a source file not already found */
				file = entry->d_name;
				if (entry->d_ino != 0 && issrcfile(file) &&
				    infilelist(file) == NO) {

					/* add it to the list */
					(void) sprintf(path, "%s/%s", srcdirs[i], file);
					addsrcfile(file, path);
				}
			}
			closedir(dirfile);
		}
	}
}

/* see if this is a source file */

BOOL
issrcfile(file)
char	*file;
{
	struct	stat	statstruct;
	char	*s;

	/* if there is a file suffix */
	if ((s = strrchr(file, '.')) != NULL) {
		if (*++s == '\0') {		/* no suffix */
			/* EMPTY */;
		}
		else if (s[1] == '\0') {	/* 1 character suffix */
			switch (*s) {
			case 'c':
			case 'h':
			case 'l':
			case 'y':
			case 'C':
			case 'G':
			case 'H':
			case 'L':
				return(YES);
			}
		}
		else if (s[2] == '\0') {	/* 2 character suffix */
			if ((*s == 'b' && s[1] == 'p') ||
			    (*s == 's' && s[1] == 'd')) {
			
				/* some directories have 2 character
				   suffixes so make sure it is a file */
				if (stat(file, &statstruct) == 0 && 
				    (statstruct.st_mode & S_IFREG)) {
					return(YES);
				}
			}
		}
	}
	return(NO);
}

/* add an include file to the source file list */

void
incfile(file, type)
char	*file;
char	type;
{
	char	name[PATHLEN + 1];
	char	path[PATHLEN + 1];
	char	*s;
	int	i;
	
	/* see if the file is already in the source file list */
	if (infilelist(file) == YES) {
		return;
	}
	/* look in current directory if it was #include "file" */
	if (type == '"' && (s = inviewpath(file)) != NULL) {
		addsrcfile(file, s);
	}
	else {
		/* search for the file in the #include directory list */
		for (i = 0; i < nincdirs; ++i) {
			
			/* don't include the file from two directories */
			(void) sprintf(name, "%s/%s", incnames[i], file);
			if (infilelist(name) == YES) {
				break;
			}
			/* make sure it exists and is readable */
			(void) sprintf(path, "%s/%s", incdirs[i], file);
			if (access(compath(path), READ) == 0) {
				addsrcfile(name, path);
				break;
			}
		}
	}
}

/* see if the file is already in the list */

BOOL
infilelist(file)
char	*file;
{
	register struct	listitem *p;
	
	for (p = srcnames[hash(compath(file)) % HASHMOD]; p != NULL; p = p->next) {
		if (strequal(file, p->text)) {
			return(YES);
		}
	}
	return(NO);
}

/* search for the file in the view path */

char *
inviewpath(file)
char	*file;
{
	static	char	path[PATHLEN + 1];
	int	i;

	/* look for the file */
	if (access(compath(file), READ) == 0) {
		return(file);
	}
	/* if it isn't a full path name and there is a multi-directory view path */
	if (*file != '/' && vpndirs > 1) {

		/* compute its path from higher view path source dirs */
		for (i = 1; i < nvpsrcdirs; ++i) {
			(void) sprintf(path, "%s/%s", srcdirs[i], file);
			if (access(compath(path), READ) == 0) {
				return(path);
			}
		}
	}
	return(NULL);
}

/* add a source file to the list */

void
addsrcfile(name, path)
char	*name;
char	*path;
{
	struct	listitem *p;
	int	i;
	
	/* make sure there is room for the file */
	if (nsrcfiles == msrcfiles) {
		msrcfiles += SRCINC;
		srcfiles = (char **) myrealloc(srcfiles, msrcfiles * sizeof(char *));
	}
	/* add the file to the list */
	srcfiles[nsrcfiles++] = stralloc(compath(path));
	p = (struct listitem *) mymalloc(sizeof(struct listitem));
	p->text = stralloc(compath(name));
	i = hash(p->text) % HASHMOD;
	p->next = srcnames[i];
	srcnames[i] = p;
}

/* free the memory allocated for the source file list */

void
freefilelist()
{
	register struct	listitem *p, *nextp;
	register int	i;
	
	while (nsrcfiles > 0) {
		free(srcfiles[--nsrcfiles]);
	}
	for (i = 0; i < HASHMOD; ++i) {
		for (p = srcnames[i]; p != NULL; p = nextp) {
			nextp = p->next;
			free(p);
		}
		srcnames[i] = NULL;
	}
}
