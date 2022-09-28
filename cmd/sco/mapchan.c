/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mapchan.c	1.1"

/*
 *	@(#) mapchan.c 1.2 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */


#include <sys/termio.h>
#include <sys/param.h>
#include <sys/emap.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "defs.h"

bool 	    aopt,		/* all channels in /etc/default/mapchan */
            dopt,		/* decimal representation for reading a map */
            nopt, 		/* no mapping at all */
            oopt, 		/* octal representation for reading a map */
            sopt, 		/* set a map */
	    Dopt;		/* Debuging flag */

static char fopt_file[100];	/* the name of the map file (-f option) */

/*
 * we could keep a list of channels and map file names
 * but most likely channels will share maps.
 * to avoid duplicating the processing of map files,
 * we have a list of channels with pointers into a list of maps.
 */
static struct chan {		/* a linked list of channels */
	char *chan_name;	/* device name */
	struct map *chan_map;	/* pointer to map */
	struct chan *chan_next; /* next link in list */
} *chan_first;		/* pointer to first channel */

static struct map { 		/* a linked list of maps */
	char *map_name;		/* map file name */
	char map_buf[E_TABSZ]; 	/* structured buffer representing the map */
	struct map *map_next;	/* next link in list */
} *map_first;		/* pointer to first map */

/*
 * functions that return a value
 */
char *sname(), *strdup(), *ttyname(), *defread(), *malloc();
static char *xdefread(), *xttyname();
static int open_chan();
int convert();

/*
 * externals
 */
extern int errno;
extern char *sys_errlist[];

/*
 * forward declarations of functions that do not return values.
 */
static void get_opts(), get_channels(), check_channels(), convert_maps(),
	map_the_channels(), add_chan(), usage();

/*
 * globals to make the parameters to main accessible
 * to all with the normal names.
 */
static int argc;
static char **argv;

main(argc0, argv0)
int argc0;
char **argv0;
{
	argc = argc0;
	argv = argv0;
	get_opts();
	get_channels();
	check_channels();
	if (!sopt)
		show_map();		
	else {
		if (!nopt)
			convert_maps();
		map_the_channels();
	}
}

/*
 * return a pointer to the final component of the path
 * sname stands for simple name.
 */
char *
sname(path)
char *path;
{
	char *p;

	for (p = path; *p; ++p)
		if (*p == '/')
			path = p+1;
	return(path);
}

/*
 * get all of the options and check for conflicts
 */
static void
get_opts()
{
	char *p;

	aopt = Dopt = dopt = nopt = oopt = sopt = FALSE;
	*fopt_file = CNULL;
	SHIFT;
	while (argc && **argv == '-') {
		p = (*argv)+1;
		SHIFT;
		while (*p) {
			switch (*p) {
			case 'a':
				aopt = TRUE;
				break;
			case 'd':
				dopt = TRUE;
				break;
			case 'D':
				Dopt = TRUE;
				break;
			case 'f':
				if (!argc)
					usage();
				strcpy(fopt_file, *argv);
				SHIFT;
				break;
			case 'n':
				nopt = TRUE;
				break;
			case 'o':
				oopt = TRUE;
				break;
			case 's':
				sopt = TRUE;
				break;
			default:
				usage();
			}
			++p;
		}
	}
	/*
	 * the a, n or f options imply that you want to set a map.
	 */
	if (aopt || *fopt_file || nopt)
		sopt = TRUE;
	/*
	 * now that we've gotten all the options
	 * check for incompatibilities
	 */
	if (nopt && *fopt_file)
		oops("-n cannot be used with -f mapfile\n");
	if (dopt && oopt)
		oops("-d cannot be used with -o\n");
	if ((dopt || oopt) && sopt)
		oops("-d and -o are used for reading a map\n");
	if (aopt && argc)
		oops("channels cannot be given along with -a\n");
	if (!sopt && (argc > 1 || aopt))
		oops("the mapping can be read on a single channel only\n");
}

/*
 * channels can be given in 3 ways:
 * 1) -a to get all of the ones in the default file
 * 2) specifying them after the options.
 * 3) if none were given after the options and -a was not given
 *    then the controlling tty is used.
 */
static void
get_channels()
{
	FILE *f;
	char *p, *q;
	char chanbuf[150], mapbuf[150], buf[150];

	/*
	 * if the -a option was given, get all the channels
	 * and mapping files from /etc/default/mapchan.
	 */
	if (aopt) {
		if ((f = fopen(MAPCHAN_DEF, "r")) == NULL)
			oops("cannot open %s\n", MAPCHAN_DEF);
		while (fgets(buf, 150, f) != NULL) {
			p = buf;
			if (*buf == '\n' || *buf == '#')
				continue;
			while (*p && !isspace(*p))
				++p;
			if (*p)
				*p++ = CNULL;
			while (*p && isspace(*p))
				++p;
			q = p;
			if (*q != '#')
				while (*q && !isspace(*q))
					++q;
			*q = CNULL;
			if (*buf == '/')
				strcpy(chanbuf, buf);
			else
				sprintf(chanbuf, "%s/%s", "/dev", buf);
			if (*fopt_file)
				strcpy(mapbuf, fopt_file);
			else if (!*p)
				*mapbuf = CNULL;
			else if (*p == '/')
				strcpy(mapbuf, p);
			else
				sprintf(mapbuf, "%s/%s", MAP_DIR, p);
			if (*mapbuf)
				add_chan(chanbuf, mapbuf);
			else
				add_chan(chanbuf, NULL_MAP_FILE);
		}
		return;
	}
	/*
	 * open the default file if we will need it
	 */
	if (sopt && !nopt && !*fopt_file && defopen(MAPCHAN_DEF) != 0)
		oops("cannot open %s\n", MAPCHAN_DEF);
	/*
	 * if there were no channels specified on the command line
	 * use the controlling tty.
	 */
	if (!argc) {
		strcpy(chanbuf, xttyname());
		if (nopt || !sopt)
			add_chan(chanbuf, NULL_MAP_FILE);
		else if (*fopt_file)
			add_chan(chanbuf, fopt_file);
		else {
			if (!(p = xdefread(chanbuf)))
				oops("no entry in %s for %s\n",
				     MAPCHAN_DEF, chanbuf);
			add_chan(chanbuf, p);
		}
		return;
	}

	/*
	 * get the channels on the command line
	 * and add them and their mapping file to a list.
	 */

	while (argc) {
		if (**argv == '/')
			strcpy(chanbuf, *argv);
		else
			sprintf(chanbuf, "/dev/%s", *argv);
		SHIFT;
		if (strcmp(chanbuf, "/dev/tty") == 0)
			strcpy(chanbuf, xttyname());
		if (nopt || !sopt)
			add_chan(chanbuf, NULL_MAP_FILE);
		else if (*fopt_file)
			add_chan(chanbuf, fopt_file);
		else {
			if (!(p = xdefread(chanbuf))) {
				error("no entry in %s for %s\n",
					MAPCHAN_DEF,
					chanbuf);
				continue;
			}
			add_chan(chanbuf, p);
		}
	}
}

/*
 * check the channels for file type, permissions and ownership.
 * if something is wrong, change the channel name to BAD_CHANNEL.
 */
static void
check_channels()
{
	struct chan *cp;
	char *name;
	struct stat statbuf;
	int euid;

	for (cp = chan_first; cp; cp = cp->chan_next) {
		name = cp->chan_name;
		if (stat(name, &statbuf) < 0) {
			error("cannot stat %s\n", name);
			cp->chan_name = BAD_CHANNEL;
			continue;
		}
		if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
			error("%s is not a character special file\n", name);
			cp->chan_name = BAD_CHANNEL;
			continue;
		}
		if (access(name, 4) < 0 && access(name, 2) < 0) {
			error("no read or write permission on %s\n",
			      name);
			cp->chan_name = BAD_CHANNEL;
			continue;
		}
		if (sopt) {
			/*
			 * we will map this channel
			 */
			euid = geteuid();
			if (euid != 0 && statbuf.st_uid != euid) {
				error(
				     "you do not own %s and so cannot map it\n",
				     name); 
				cp->chan_name = BAD_CHANNEL;
				continue;
			}
		}
	}
}

/*
 * display the map on the only channel (chan_first).
 */
show_map()
{
	int fd, i;
	char *name;
	char buf[E_TABSZ];

	name = chan_first->chan_name;
	if (name == BAD_CHANNEL)
		return;		/* check channels found something wrong */
	if ((fd = open_chan(name)) < 0)
		oops("cannot open %s\n", name);
	if (( i = ioctl(fd, LDGMAP, buf)) < 0) {
		if ( errno  == ENAVAIL) {
			printf("null\n");
			return;
		}
		error("%s\n", sys_errlist[errno]);
		error("LDGMAP ioctl failed on %s\n", name);
		return;
	}
	display(buf, dopt? 10:
		     oopt?  8:
			   16);
}

/*
 * for each map,
 * convert the file into a structured buffer
 * which is stored within the chan struct.
 * if some syntax error is found, change the name to BAD_MAP_FILE.
 * if the file contains only the word "null"
 * change the name to NULL_MAP_FILE.
 */
static void
convert_maps()
{
	int rc;
	struct map *mp;

	for (mp = map_first; mp; mp = mp->map_next) {
		rc = convert(mp->map_name, mp->map_buf);
		if (rc == BAD_MAP_FILE)
			mp->map_name = (char *) BAD_MAP_FILE;
		else if (rc == NULL_MAP_FILE)
			mp->map_name = (char *) NULL_MAP_FILE;
	}
}

/*
 * for each channel apply its corresponding map.
 */
static void
map_the_channels()
{
	struct chan *cp;
	struct map *mp;
	int fd;
	char *name;

	for (cp = chan_first; cp; cp = cp->chan_next) {
		name = cp->chan_name;
		mp = cp->chan_map;
		if (name == BAD_CHANNEL ||
		    mp->map_name == (char *) BAD_MAP_FILE)
			/*
			 * there was some problem with this channel
			 * that was found in check_channels()
			 * or a syntax error in the mapping file
			 * found by convert().
			 */
			continue;
		if ((fd = open_chan(name)) < 0) {
			error("could not open %s\n", name);
			continue;
		}
		if (mp && mp->map_name != NULL_MAP_FILE) {
			if (ioctl(fd, LDSMAP, mp->map_buf) < 0) {
				if (errno == ENAVAIL)
					oops("maximum number of maps exceeded\n");
				error("%s\n", sys_errlist[errno]);
				error("LDSMAP ioctl failed on %s\n",
				      name);
				continue;
			}
		} else {
			/*
			 * null mapping
			 */
			if (ioctl(fd, LDNMAP, NULL) < 0) {
				error("%s\n", sys_errlist[errno]);
				error("LDNMAP ioctl failed on %s\n",
				      name);
				continue;
			}
		}
	}
}

bool sig_caught;
/*
 * open a channel.
 * if it can't be opened for reading open it for writing.
 * if it can't be opened because it is BUSY, set an alarm and
 * try again without NDELAY set.
 * return the file descriptor or -1 in case of error.
 */
static
int
open_chan(name)
char *name;
{
	int fd;
	void (*catch_alarm)();

	if ((fd = open(name, O_RDONLY | O_NDELAY)) >= 0)
		return(fd);
	if (errno == EACCES &&
	    (fd = open(name, O_WRONLY | O_NDELAY)) >= 0)
		return(fd);
	if (errno == EBUSY) { 
		signal(SIGALRM, catch_alarm);
		sig_caught = FALSE;
		alarm(NSECS);
		if ((fd = open(name, O_RDONLY)) >= 0) {
			alarm(0);
			return(fd);
		}
		alarm(0);
		if (sig_caught)
			/*
			 * the open may have hung.
			 * only happens if another process closes
			 * the line between the two opens.
			 * maybe.
			 */
			return(-1);
		alarm(NSECS);
		if (errno == EACCES &&
	    	    (fd = open(name, O_WRONLY)) >= 0) {
			alarm(0);
			return(fd);
		}
		alarm(0);
		if (sig_caught) 
			return(-1);
	}			
	return(-1);
}

/*
 * catch an alarm signal by setting sig_caught to TRUE.
 */
catch_alarm()
{
	sig_caught = TRUE;
}

/*
 * an extended defread
 * look in the default file for the pattern.
 * if it isn't there and it began with /dev/
 * then look for the part AFTER /dev/.
 * after locating a line beginning with the pattern,
 * skip white space and return the next string (up to a blank).
 * precede it with MAP_DIR if it isn't a full pathname.
 * return NULL in case of error.
 */
static
char *
xdefread(pattern)
char *pattern;
{
	char *p, *q;
	static char buf[100];

	if ((p = defread(pattern)) == NULL) {
		if (strncmp(pattern, "/dev/", 5) != 0 ||
		    (p = defread(pattern+5)) == NULL)
			return(NULL);
	}
	while (isspace(*p))
		++p;
	q = p;
	if (*q == '#')
		*q = CNULL;
	else {
		while (*q && !isspace(*q))
			++q;
		*q = CNULL;
	}
	if (!*p || *p == '/')
		return(p);
	sprintf(buf, "%s/%s", MAP_DIR, p);
	return(buf);
}

/*
 * an extended ttyname()
 */
static
char *
xttyname()
{
	char *p;

	if ((p = ttyname(0)) == NULL &&
	    (p = ttyname(1)) == NULL &&
	    (p = ttyname(2)) == NULL   )
		oops("cannot get controlling tty name\n");
	return(p);
}

/*
 * add the channel and mapfile to the lists.
 * place them at the end.
 */
static void
add_chan(c_name, m_name)
char *c_name, *m_name;
{
	struct chan *ncp, *cp;
	struct map *nmp, *mp;

	ncp = (struct chan *) malloc(sizeof(struct chan));
	if (ncp == NULL)
		oops("out of memory\n");
	ncp->chan_name = strdup(c_name);
	ncp->chan_next = NULL;
	if (!chan_first)
		chan_first = ncp;
	else {
		for (cp = chan_first; cp->chan_next; cp = cp->chan_next)
			;
		cp->chan_next = ncp;
	}
		
	/*
	 * a null map?
	 */
	if (m_name == NULL_MAP_FILE) {
		ncp->chan_map = NULL_MAP_FILE;
		return;
	}
	/*
	 * if the map already exists use it
	 */
	for (mp = map_first; mp; mp = mp->map_next)
		if (strcmp(m_name, mp->map_name) == 0) {
			ncp->chan_map = mp;
			return;
		}
	nmp = (struct map *) malloc(sizeof(struct map));
	if (nmp == NULL)
		oops("out of memory\n");
	nmp->map_name = strdup(m_name);
	nmp->map_next = NULL;
	if (!map_first)
		map_first = nmp;
	else {
		for (mp = map_first; mp->map_next; mp = mp->map_next)
			;
		mp->map_next = nmp;
	}
	ncp->chan_map = nmp;
}

/*
 * print a usage message and quit
 */
static void
usage()
{
	error("usage: mapchan [-ans] [-f mapfile] [channels]\n");
	error("          or\n");
	error("       mapchan [-do] [channel]\n");
	exit(1);
}
