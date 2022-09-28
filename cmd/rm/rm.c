/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved. 					*/

#ident	"@(#)rm:rm.c	1.22"
/*
 * rm [-fir] file ...
 */

#include        <stdio.h>
#include        <fcntl.h>
#include	<string.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>

#define ARGCNT		5		/* Number of arguments */
#define CHILD		0	 
#define	DIRECTORY	((buffer.st_mode&S_IFMT) == S_IFDIR)
#define	SYMLINK		((buffer.st_mode&S_IFMT) == S_IFLNK)
#define	FAIL		-1
#define MAXFORK		100		/* Maximum number of forking attempts */
#define MAXFILES        OPEN_MAX  - 2	/* Maximum number of open files */
#define	MAXLEN		DIRBUF-24  	/* Name length (1024) is limited */
				        /* stat(2).  DIRBUF (1048) is defined */
				        /* in dirent.h as the max path length */
#define	NAMESIZE	MAXNAMLEN + 1	/* "/" + (file name size) */
#define TRUE		1
#define	WRITE		02
static	int	errcode;
void	exit();
static	int interactive, recursive, silent; /* flags for command line options */
void	free();
void	*malloc();
void	perror();
unsigned sleep();
char	*strcpy();
int	getopt();
int	lstat();
int	access();
int	isatty();
int	rmdir();
int	unlink();

static	void	rm();
static	void	undir();
static	int	yes();
static	int	mypath();

main(argc, argv)
	int	argc;
	char	*argv[];
{
	extern int	optind;
	int	errflg = 0;
	int	c;

	while ((c = getopt(argc, argv, "fri")) != EOF)
		switch (c) {
		case 'f':
			silent = TRUE;
			break;
		case 'i':
			interactive = TRUE;
			break;
		case 'r':
			recursive = TRUE;
			break;
		case '?':
			errflg = 1;
			break;
		}

	/* 
	 * For BSD compatibility allow '-' to delimit the end 
	 * of options.
	 */
	if (optind < argc && !strcmp(argv[optind], "-")) 
		optind++;

	argc -= optind;
	argv = &argv[optind];
	
	if (argc < 1 || errflg) {
		(void)fprintf(stderr, "usage: rm [-fir] file ...\n");
		exit(2);
	}

	while (argc-- > 0) {
		rm (*argv);
		argv++;
	}

	exit(errcode ? 2 : 0);
	/* NOTREACHED */
}

static void
rm(path)
	char	*path;
{
	struct stat buffer;
	/* 
	 * Check file to see if it exists.
	 */
	if (lstat(path, &buffer) == FAIL) {
		if (!silent) {
			perror(path);
			++errcode;
		}
		return;
	}
	
	/*
	 * If it's a directory, remove its contents.
	 */
	if (DIRECTORY) {
		undir(path, buffer.st_dev, buffer.st_ino);
		return;
	}
	
	/*
	 * If interactive, ask for acknowledgement.
	 */
	if (interactive) {
		printf("rm: remove  %s: (y/n)? ", path);
		if (!yes())
			return;
	} else if (!silent) {
		/* 
		 * If not silent, and stdin is a terminal, and there's
		 * no write access, and the file isn't a symblic link,
		 * ask for permission.
		 */
		if (access(path, WRITE) == FAIL
		   && isatty(fileno(stdin)) && !SYMLINK) {
			printf("rm: %s: %o mode ? ",path,buffer.st_mode & 0777);
			/*
			 * If permission isn't given, skip the file.
			 */
			if (!yes())
				return;
		}
	}
	
	/*
	 * If the unlink fails, inform the user if interactive or not silent.
	 */
	if (unlink(path) == FAIL && (!silent || interactive)) {
		fprintf(stderr, "rm: %s not removed.\n", path);
		perror("");
		++errcode;
	}
}

static void
undir(path, dev, ino)
	char	*path;
	dev_t	dev;
	ino_t	ino;
{
	char	*newpath;
	DIR	*name;
	struct dirent *direct;

	/*
	 * If "-r" wasn't specified, trying to remove directories
	 * is an error.
	 */
	if (!recursive) {
		fprintf(stderr, "rm: %s directory\n", path);
		++errcode;
		return;
	}

	/*
	 * If interactive and this file isn't in the path of the
	 * current working directory, ask for acknowledgement.
	 */
	if (interactive && !mypath(dev, ino)) {
		printf("rm: directory %s: ? ", path);
		/*
		 * If the answer is no, skip the directory.
		 */
		if (!yes())
			return;
	}
	
	/*
	 * Open the directory for reading.
	 */
	if ((name = opendir(path)) == NULL) {
		fprintf(stderr, "rm: cannot open %s\n", path);
		perror("");
		exit(2);
	}
	
	/*
	 * Read every directory entry.
	 */
	while ((direct = readdir(name)) != NULL) {
		/*
		 * Ignore "." and ".." entries.
 		 */
		if(!strcmp(direct->d_name, ".")
		  || !strcmp(direct->d_name, ".."))
			continue;
		/*
		 * Try to remove the file.
		 */
		newpath = (char *)malloc((strlen(path) + NAMESIZE + 1));

		if (newpath == NULL) {
			fprintf(stderr,"rm: Insufficient memory.\n");
			perror("");
			exit(1);
		}
		
		/*
		 * Limit the pathname length so that a clear error message
		 * can be provided.
		 */
		if (strlen(path) + strlen(direct->d_name)+2 >= (size_t)MAXLEN) {
			fprintf(stderr,"rm: Path too long (%d/%d).\n",
			  strlen(path)+strlen(direct->d_name)+1, MAXLEN);
			exit(2);
		}

		sprintf(newpath, "%s/%s", path, direct->d_name);
 
		/*
		 * If a spare file descriptor is available, just call the
		 * "rm" function with the file name; otherwise close the
		 * directory and reopen it when the child is removed.
		 */
		if (name->dd_fd >= MAXFILES) {
			closedir(name);
			rm(newpath);
			if ((name = opendir(path)) == NULL) {
				fprintf(stderr, "rm: cannot open %s\n", path);
				perror("");
				exit(2);
			}
		} else
			rm(newpath);
 
		free(newpath);
	}

	/*
	 * Close the directory we just finished reading.
	 */
	closedir(name);
	
	/*
	 * The contents of the directory have been removed.  If the
	 * directory itself is in the path of the current working
	 * directory, don't try to remove it.
	 * When the directory itself is the current working directory, mypath()
	 * has a return code == 2.
	 */
	switch (mypath(dev, ino)) {
	case 2:
		fprintf(stderr,"rm: Can not remove any directory in the path of the current working directory\n%s\t\n",path);
		++errcode;
		return;
	case 1:
		return;
	case 0:
		break;
	}
	
	/*
	 * If interactive, ask for acknowledgement.
	 */
	if (interactive) {
		printf("rm: remove  %s: (y/n)? ", path);
		if (!yes())
			return;
	}
	if (rmdir(path) == FAIL) {
		fprintf(stderr, "rm: Unable to remove directory\n");
		perror("");
		++errcode;
	}
}


static int
yes()
{
	int	i, b;

	i = b = getchar();
	while (b != '\n' && b != '\0' && b != EOF)
		b = getchar();
	return(i == 'y');
}

static int
mypath(dev, ino)
	dev_t	dev;
	ino_t	ino;
{
	struct stat buffer;
	dev_t	lastdev = (dev_t) -1;
	ino_t	lastino = (ino_t) -1;
	char	*path;
	int	i, j;

	for (i = 1; ; i++) {
		/*
		 * Starting from ".", walk toward the root, looking at
		 * each directory along the way.
		 */
		path = (char *)malloc((3 * (uint)i));

		if (path == NULL) {
			fprintf(stderr, "rm: Insufficient memory.\n");
			perror("");
			exit(1);
		}

		strcpy(path, ".");
		for (j = 1; j < i; j++) 
			if (j == 1)
				strcpy(path, "..");
			else
				strcat(path,"/..");
		lstat(path, &buffer);
		/*
		 * If we find a match, the directory (dev, ino) passed to mypath()
		 * is an ancestor of ours. Indicated by return 1;
		 *
		 * If (i == 1) the directory (dev, ino) passed to mypath() is our
		 * current working directory. Indicated by return 2;
		 * 
		 */
		if (buffer.st_dev == dev && buffer.st_ino == ino)
			if (i == 1)
				return 2;
			else
				return 1;
		
		/*
		 * If we reach the root without a match, the given
		 * directory is not in our path.
		 */
		if (buffer.st_dev == lastdev && buffer.st_ino == lastino) 
			return 0;
	
		/*
		 * Save the current dev and ino, and loop again to go
		 * back another level.
		 */
		lastdev = buffer.st_dev;
		lastino = buffer.st_ino;
		free(path);
	}
}
