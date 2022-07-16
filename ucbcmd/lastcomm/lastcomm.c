/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#ident	"@(#)ucblastcomm:lastcomm.c	1.2.1.1"

/*
 * last command
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/acct.h>
#include <sys/file.h>
#include <sys/fcntl.h>

#include <stdio.h>
#include <pwd.h>
#include <sys/stat.h>
#include <utmp.h>
#include <ctype.h>

#define DEV_BSIZE 512
#define DEV_BSHIFT 9
#define AHZ	64
#define fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))

struct	acct buf[DEV_BSIZE / sizeof (struct acct)];

char	*getname();
char	*getdev();
time_t  expand();
char    *flagbits();

main(argc, argv)
	char *argv[];
{
	register int bn, cc;
	register struct acct *acp;
	int fd;
	struct stat sb;

	fd = open("/var/adm/pacct", O_RDONLY);
	if (fd < 0) {
		perror("/var/adm/pacct");
		exit(1);
	}
	fstat(fd, &sb);
	for (bn = (unsigned)sb.st_size >> DEV_BSHIFT; bn >= 0; bn--) {
		lseek(fd, (unsigned)bn << DEV_BSHIFT, 0);
		cc = read(fd, buf, DEV_BSIZE);
		if (cc < 0) {
			perror("read");
			break;
		}
		acp = buf + (cc / sizeof (buf[0])) - 1;
		for (; acp >= buf; acp--) {
			register char *cp;
			time_t x;

			if (acp->ac_comm[0] == '\0')
				strcpy(acp->ac_comm, "?");
			for (cp = &acp->ac_comm[0];
			     cp < &acp->ac_comm[fldsiz(acct, ac_comm)] && *cp;
			     cp++)
				if (!isascii(*cp) || iscntrl(*cp))
					*cp = '?';
			if (argc > 1 && !ok(argc, argv, acp))
				continue;
			x = expand(acp->ac_utime) + expand(acp->ac_stime);
			printf("%-*.*s %s %-*s %-*s %6.2f secs %.16s\n",
				fldsiz(acct, ac_comm), fldsiz(acct, ac_comm),
				acp->ac_comm,
				flagbits(acp->ac_flag),
				fldsiz(utmp, ut_name), getname(acp->ac_uid),
				fldsiz(utmp, ut_line), getdev(acp->ac_tty),
				x / (double)AHZ, ctime(&acp->ac_btime));
		}
	}
	exit(0);
}

time_t
expand (t)
	unsigned t;
{
	register time_t nt;

	nt = t & 017777;
	t >>= 13;
	while (t) {
		t--;
		nt <<= 3;
	}
	return (nt);
}

char *
flagbits(f)
	register int f;
{
	register int i = 0;
	static char flags[20];

#define BIT(flag, ch)	flags[i++] = (f & flag) ? ch : ' '
	BIT(ASU, 'S');
	BIT(AFORK, 'F');
	flags[i] = '\0';
	return (flags);
}

ok(argc, argv, acp)
	register int argc;
	register char *argv[];
	register struct acct *acp;
{
	register int j;

	for (j = 1; j < argc; j++)
		if (strcmp(getname(acp->ac_uid), argv[j]) &&
		    strcmp(getdev(acp->ac_tty), argv[j]) &&
		    strncmp(acp->ac_comm, argv[j], fldsiz(acct, ac_comm)))
			break;
	return (j == argc);
}

/* should be done with nameserver or database */

struct	utmp utmp;

#define NUID	2048
#define	NMAX	(sizeof (utmp.ut_name))

char	names[NUID][NMAX+1];
char	outrangename[NMAX+1];
uid_t	outrangeuid = (uid_t)-1;

char *
getname(uid)
	uid_t uid;
{
	register struct passwd *pw;
	static init;
	static char uidname[NMAX];
	struct passwd *getpwent();

	if (uid >= (uid_t)0 && uid < (uid_t)NUID && names[uid][0])
		return (&names[uid][0]);
	if (uid >= (uid_t)0 && uid == outrangeuid)
		return (outrangename);
	if (init == 2) {
		if (uid < NUID) {
			(void) sprintf(uidname, "%d", uid);
			return (uidname);
		}
		setpwent();
		while (pw = getpwent()) {
			if (pw->pw_uid != uid)
				continue;
			outrangeuid = pw->pw_uid;
			strncpy(outrangename, pw->pw_name, NMAX);
			endpwent();
			return (outrangename);
		}
		endpwent();
		(void) sprintf(uidname, "%d", uid);
		return (uidname);
	}
	if (init == 0)
		setpwent(), init = 1;
	while (pw = getpwent()) {
		if (pw->pw_uid < 0 || pw->pw_uid >= NUID) {
			if (pw->pw_uid == uid) {
				outrangeuid = pw->pw_uid;
				strncpy(outrangename, pw->pw_name, NMAX);
				return (outrangename);
			}
			continue;
		}
		if (names[pw->pw_uid][0])
			continue;
		strncpy(names[pw->pw_uid], pw->pw_name, NMAX);
		if (pw->pw_uid == uid)
			return (&names[uid][0]);
	}
	init = 2;
	endpwent();
	(void) sprintf(uidname, "%d", uid);
	return (uidname);
}

#include <dirent.h>

#define PATHNAMLEN	32		/* path length can't be larger than 17 */

#define N_DEVS		43		/* hash value for device names */
#define NDEVS		500		/* max number of file names in /dev */

struct	devhash {
	dev_t	dev_dev;
	char	dev_name [PATHNAMLEN];
	struct	devhash * dev_nxt;
};
struct	devhash *dev_hash[N_DEVS];
struct	devhash	*dev_chain;
#define HASH(d)	(((int) d) % N_DEVS)
int	ndevs = NDEVS;
struct devhash * hashtab;


char *
getdev(dev)
	dev_t dev;
{
	register struct devhash *hp, *nhp;
	struct stat statb;
	char name[PATHNAMLEN];
	static dev_t lastdev = (dev_t) -1;
	static char *lastname;
	static int init = 0;

	if (dev == NODEV)
		return ("__");
	if (dev == lastdev)
		return (lastname);
	if (!init) {
		setupdevs();
		init++;
	}
	for (hp = dev_hash[HASH(dev)]; hp; hp = hp->dev_nxt)
		if (hp->dev_dev == dev) {
			lastdev = dev;
			return (lastname = hp->dev_name);
		}
	for (hp = dev_chain; hp; hp = nhp) {
		nhp = hp->dev_nxt;
		strcpy(name, "/dev/");
		strcat(name, hp->dev_name);
		if (stat(name, &statb) < 0)	/* name truncated usually */
			continue;
		if ((statb.st_mode & S_IFMT) != S_IFCHR)
			continue;
		hp->dev_dev = statb.st_rdev;
		hp->dev_nxt = dev_hash[HASH(hp->dev_dev)];
		dev_hash[HASH(hp->dev_dev)] = hp;
		if (hp->dev_dev == dev) {
			dev_chain = nhp;
			lastdev = dev;
			return (lastname = hp->dev_name);
		}
	}
	dev_chain = (struct devhash *) 0;
	return ("??");
}


setupdevs()
{
	char	**srch_dirs;		/* priority directories */
	char	**get_pri_dirs();
	int	dirno = 0;
	
	hashtab = (struct devhash *)malloc(NDEVS * sizeof(struct devhash));
	if (hashtab == (struct devhash *)0) {
		fprintf(stderr, "No mem for dev table\n");
		return;
	}

	srch_dirs = get_pri_dirs();

	while (srch_dirs[dirno] != NULL) {
		if (srch_dir(srch_dirs[dirno]) < 0)
			return;
		dirno++;
	}

	dirno = 0;
	while (srch_dirs[dirno] != NULL) {
		if (!strcmp("/dev", srch_dirs[dirno]))
			return;			/* don't search /dev twice */
		dirno++;
	}

	if (srch_dir("/dev") < 0)
		return;	
}



/* The following 2 routines are modified version of get_pri_dirs
 * and srch_dir in ttyname.c. 
 */	
static char dev_dir[] = "/dev";
static char *def_srch_dirs[] = {        /* default search list */
        "/dev/term",
        "/dev/pts",
        "/dev/xt",
        NULL
};

static char *raw_sf;	/* buffer containing raw image of the search file */
#define SRCH_FILE_NAME  "/etc/ttysrch"
#define COMMENT_CHAR    '#'
#define EOLN            '\n'
#define START_STATE     1
#define COMMENT_STATE   2
#define DIRNAME_STATE   3
    
static char **
get_pri_dirs()
{
        int bcount = 0;
        int c;
        int sf_lines = 0;       /* number of lines in search file */
        int dirno = 0;
        int state;
        FILE *sf;
        char **pri_dirs;        /* priority search list */
        char *sfp;              /* pointer inside the raw image buffer */
        struct stat sfsb;       /* search file's stat structure buffer */
 


	if ((sf = fopen(SRCH_FILE_NAME, "r")) == NULL)
		return (def_srch_dirs);
	if (stat(SRCH_FILE_NAME, &sfsb) < 0) {
		fclose(sf);
                return (def_srch_dirs);
        }
	raw_sf = (char *)malloc(sfsb.st_size + 1);
        sfp = raw_sf;
        while ((bcount++ < sfsb.st_size) && ((c = getc(sf)) != EOF)) {
                *sfp++ = (char) c;
		if (c == EOLN)
                        sf_lines++;
        }
	fclose(sf);
        *sfp = EOLN;
 
	pri_dirs = (char **) malloc(++sf_lines * sizeof(char *));

	sfp = raw_sf;
        state = START_STATE;
        while (--bcount) {
                switch(state) {
		case START_STATE:
                        if (*sfp == COMMENT_CHAR) {
                                state = COMMENT_STATE;
                        } else if (!isspace(*sfp)) {
				state = DIRNAME_STATE;
                                pri_dirs[dirno++] = sfp;
                        }
                        break;
                case COMMENT_STATE:
                        if (*sfp == EOLN)
                                state = START_STATE;
                        break;
                case DIRNAME_STATE:
                        if (*sfp == EOLN) {
                                *sfp = '\0';
                                state = START_STATE;
                        } else if (isspace(*sfp)) {
	                        *sfp = '\0';
                                state = COMMENT_STATE;
                        }
                        break;

		} /* switch */
		sfp++;
	}   

	*sfp = '\0';
	pri_dirs[dirno] = NULL;
	return (pri_dirs);
}


srch_dir(path)
char *path;
{
	DIR *dirp;
	struct dirent *direntp;
	struct stat st;
	char file_name[PATHNAMLEN];

	if ((dirp = opendir(path)) == NULL) 
		return(0);

	if ((readdir(dirp) == NULL) || (readdir(dirp) == NULL))
		return(0);

	while ((direntp = readdir(dirp)) != NULL) {
		strcpy(file_name, path);
		strcat(file_name, "/");
		strcat(file_name, direntp->d_name);
		if (stat((const char *)file_name, &st) < 0)
			continue;
		if ((st.st_mode & S_IFMT) == S_IFCHR) {
			strcpy(hashtab->dev_name, file_name+strlen("/dev/"));
			hashtab->dev_nxt = dev_chain;
			dev_chain = hashtab;
			hashtab++;
			if (--ndevs < 0)
				return(-1);
		}
	}
	closedir(dirp);
	return(1);
}
















