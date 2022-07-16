/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getutx.c	1.7"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*	Routines to read and write the /etc/utmpx file.			*/

#ifdef __STDC__
	#pragma weak getutxent = _getutxent
	#pragma weak getutxid = _getutxid
	#pragma weak getutxline = _getutxline
	#pragma weak makeutx = _makeutx
	#pragma weak modutx = _modutx
	#pragma weak pututxline = _pututxline
	#pragma weak setutxent = _setutxent
	#pragma weak endutxent = _endutxent
	#pragma weak utmpxname = _utmpxname
	#pragma weak updutmp = _updutmp
	#pragma weak updwtmpx = _updwtmpx
#endif

#include	"synonyms.h"
#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<utmpx.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<string.h>
#include	<unistd.h>

#define IDLEN	4	/* length of id field in utmp */
#define SC_WILDC	0xff	/* wild char for utmp ids */
#define	MAXFILE	79	/* Maximum pathname length for "utmpx" file */

# define MAXVAL 255	/* max value for an id `character' */
# define IPIPE	"/etc/initpipe"	/* FIFO to send pids to init */


/*
 * format of message sent to init
 */

struct	pidrec {
	int	pd_type;	/* command type */
	pid_t	pd_pid;		/* pid */
};

/*
 * pd_type's
 */

# define ADDPID 1	/* add a pid to "godchild" list */
# define REMPID 2	/* remove a pid to "godchild" list */


#ifdef	DEBUG
#undef	UTMPX_FILE
#define	UTMPX_FILE "utmpx"
#undef	UTMP_FILE
#define	UTMP_FILE "utmp"
#endif

extern void	_setutxent();
extern void	unlockutx();
extern void	sendpid();

static int fd = -1;	/* File descriptor for the utmpx file. */
static int fd_u = -1;	/* File descriptor for the utmp file. */
static char utmpxfile[MAXFILE+1] = UTMPX_FILE;	/* Name of the current */
static char utmpfile[MAXFILE+1] = UTMP_FILE;	/* "utmpx" and "utmp"  */
						/* like file.          */

#ifdef ERRDEBUG
static long loc_utmp;	/* Where in "utmpx" the current "ubuf" was found.*/
#endif

static struct utmpx ubuf;	/* Copy of last entry read in. */


/* "getutxent" gets the next entry in the utmpx file.
 */

struct utmpx *getutxent()
{
	extern int fd;
	extern char utmpxfile[];
	extern struct utmpx ubuf;
	register char *u;
	register int i;
	int fd2;

/* If the "utmpx" file is not open, attempt to open it for
 * reading.  If there is no file, attempt to create one.  If
 * both attempts fail, return NULL.  If the file exists, but
 * isn't readable and writeable, do not attempt to create.
 */

        if (fd < 0) {
                if ((fd = open(utmpxfile, O_RDWR|O_CREAT, 0644)) < 0) {

/* If the open failed for permissions, try opening it only for
 * reading.  All "pututxline()" later will fail the writes.
 */
                if ((fd = open(utmpxfile, O_RDONLY)) < 0)
                                return(NULL);
                }
                if (access(utmpfile, F_OK) < 0) {
                        if ((fd2 = open(utmpfile, O_RDWR|O_CREAT, 0644)) < 0) 
                                return(NULL); 
                        close(fd2); 
                } 
 
                /* Make sure files are in synch */
                if (synchutmp(utmpfile, utmpxfile))
                        return(NULL);
        }

/* Try to read in the next entry from the utmpx file.  */
	if (read(fd,&ubuf,sizeof(ubuf)) != sizeof(ubuf)) {

/* Make sure ubuf is zeroed. */
		for (i=0,u=(char *)(&ubuf); i<sizeof(ubuf); i++) *u++ = '\0';
		return(NULL);
	}

/* Save the location in the file where this entry was found. */
	(void) lseek(fd,0L,1);
	return(&ubuf);
}

/*	"getutxid" finds the specified entry in the utmpx file.  If	*/
/*	it can't find it, it returns NULL.				*/

struct utmpx *getutxid(entry)
const struct utmpx *entry;
{
	extern struct utmpx ubuf;
	register short type;

/* Start looking for entry.  Look in our current buffer before */
/* reading in new entries. */
	do {

/* If there is no entry in "ubuf", skip to the read. */
		if (ubuf.ut_type != EMPTY) {
			switch(entry->ut_type) {

/* Do not look for an entry if the user sent us an EMPTY entry. */
			case EMPTY:
				return(NULL);

/* For RUN_LVL, BOOT_TIME, OLD_TIME, and NEW_TIME entries, only */
/* the types have to match.  If they do, return the address of */
/* internal buffer. */
			case RUN_LVL:
			case BOOT_TIME:
			case OLD_TIME:
			case NEW_TIME:
				if (entry->ut_type == ubuf.ut_type) return(&ubuf);
				break;

/* For INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, and DEAD_PROCESS */
/* the type of the entry in "ubuf", must be one of the above and */
/* id's must match. */
			case INIT_PROCESS:
			case LOGIN_PROCESS:
			case USER_PROCESS:
			case DEAD_PROCESS:
				if (((type = ubuf.ut_type) == INIT_PROCESS
					|| type == LOGIN_PROCESS
					|| type == USER_PROCESS
					|| type == DEAD_PROCESS)
				    && ubuf.ut_id[0] == entry->ut_id[0]
				    && ubuf.ut_id[1] == entry->ut_id[1]
				    && ubuf.ut_id[2] == entry->ut_id[2]
				    && ubuf.ut_id[3] == entry->ut_id[3])
					return(&ubuf);
				break;

/* Do not search for illegal types of entry. */
			default:
				return(NULL);
			}
		}
	} while (getutxent() != NULL);

/* Return NULL since the proper entry wasn't found. */
	return(NULL);
}

/* "getutxline" searches the "utmpx" file for a LOGIN_PROCESS or
 * USER_PROCESS with the same "line" as the specified "entry".
 */

struct utmpx *getutxline(entry)
const struct utmpx *entry;
{
	extern struct utmpx ubuf;
	register struct utmpx *cur;

/* Start by using the entry currently incore.  This prevents */
/* doing reads that aren't necessary. */
	cur = &ubuf;
	do {
/* If the current entry is the one we are interested in, return */
/* a pointer to it. */
		if (cur->ut_type != EMPTY && (cur->ut_type == LOGIN_PROCESS
		    || cur->ut_type == USER_PROCESS) && strncmp(&entry->ut_line[0],
		    &cur->ut_line[0],sizeof(cur->ut_line)) == 0) return(cur);
	} while ((cur = getutxent()) != NULL);

/* Since entry wasn't found, return NULL. */
	return(NULL);
}

/*	"pututxline" writes the structure sent into the utmpx file.	*/
/*	If there is already an entry with the same id, then it is	*/
/*	overwritten, otherwise a new entry is made at the end of the	*/
/*	utmpx file.							*/

struct utmpx *pututxline(entry)
const struct utmpx *entry;
{
	int fc;
	struct utmpx *answer;
	extern struct utmpx ubuf;
	extern int fd;
	struct utmpx tmpxbuf, savbuf;

/* Copy the user supplied entry into our temporary buffer to */
/* avoid the possibility that the user is actually passing us */
/* the address of "ubuf". */
	if (entry == (struct utmpx *) NULL)
		return((struct utmpx *)NULL);
	tmpxbuf = *entry;
	(void)getutxent();
	if (fd < 0) {
#ifdef	ERRDEBUG
		gxdebug("pututxline: Unable to create utmpx file.\n");
#endif
		return((struct utmpx *)NULL);
	}

/* Make sure utmpx file is writable */
	if ((fc=fcntl(fd, F_GETFL, NULL)) == -1
	    || (fc & O_RDWR) != O_RDWR) {
		return((struct utmpx *)NULL);
	}


/* Find the proper entry in the utmpx file.  Start at the current */
/* location.  If it isn't found from here to the end of the */
/* file, then reset to the beginning of the file and try again. */
/* If it still isn't found, then write a new entry at the end of */
/* the file.  (Making sure the location is an integral number of */
/* utmp structures into the file incase the file is scribbled.) */

	if (getutxid(&tmpxbuf) == NULL) {
#ifdef	ERRDEBUG
		gxdebug("First getutxid() failed.  fd: %d",fd);
#endif
		setutxent();
		if (getutxid(&tmpxbuf) == NULL) {
#ifdef	ERRDEBUG
			loc_utmp = lseek(fd, 0L, 1);
			gxdebug("Second getutxid() failed.  fd: %d loc_utmp: %ld\n",fd,loc_utmp);
#endif
			fcntl(fd, F_SETFL, fc | O_APPEND);
		} else {
			lseek(fd, -(long)sizeof(struct utmpx), 1);
		}
	} else {
		lseek(fd, -(long)sizeof(struct utmpx), 1);
	}


/* Write out the user supplied structure.  If the write fails, */
/* then the user probably doesn't have permission to write the */
/* utmpx file. */
	if (write(fd,&tmpxbuf,sizeof(tmpxbuf)) != sizeof(tmpxbuf)) {
#ifdef	ERRDEBUG
		gxdebug("pututxline failed: write-%d\n",errno);
#endif
		answer = (struct utmpx *)NULL;
	} else {
/* Save the user structure that was overwritten. Copy the new user  */
/* structure into ubuf so that it will be up to date in the future. */
		savbuf = ubuf;
		ubuf = tmpxbuf;
		answer = &ubuf;

#ifdef	ERRDEBUG
		gxdebug("id: %c%c loc: %x\n",ubuf.ut_id[0],ubuf.ut_id[1],
		    ubuf.ut_id[2],ubuf.ut_id[3],loc_utmp);
#endif
	}
	if (updutmp(entry)) {
		lseek(fd, -(long)sizeof(struct utmpx), 1);
		write(fd, &savbuf, sizeof(savbuf));
		answer = (struct utmpx *)NULL;
	}

	fcntl(fd, F_SETFL, fc);
	return(answer);
}

/*	"setutxent" just resets the utmpx file back to the beginning.	*/

void
setutxent()
{
	register char *ptr;
	register int i;
	extern int fd;
	extern struct utmpx ubuf;

	if (fd != -1) lseek(fd,0L,0);

/* Zero the stored copy of the last entry read, since we are */
/* resetting to the beginning of the file. */

	for (i=0,ptr=(char*)&ubuf; i < sizeof(ubuf);i++) *ptr++ = '\0';
}

/*	"endutxent" closes the utmpx file.				*/

void
endutxent()
{
	extern int fd;
	extern struct utmpx ubuf;
	register char *ptr;
	register int i;

	if (fd != -1) close(fd);
	fd = -1;
	for (i=0,ptr= (char *)(&ubuf); i < sizeof(ubuf);i++) *ptr++ = '\0';
}

/*	"utmpxname" allows the user to read a file other than the	*/
/*	normal "utmpx" file.						*/

utmpxname(newfile)
const char *newfile;
{
	extern char utmpxfile[];
	extern char utmpfile[];
	int len;

/* Determine if the new filename will fit.  If not, return 0. */
	if ((len = strlen(newfile)) > MAXFILE-1) return (0);
/* The name of the utmpx file has to end with 'x' */
	if (newfile[len-1] != 'x') return(0);

/* Otherwise copy in the new file name. */
	else {
		(void)strcpy(&utmpxfile[0],newfile);
		(void)strcpy(&utmpfile[0],newfile);
		/* strip the 'x' */
		utmpfile[len-1] = '\0';
	}
/* Make sure everything is reset to the beginning state. */
	endutxent();
	return(1);
}

/* "updutmp" updates the utmp file, uses same algorithm as 
 * pututxline so that the records end up in the same spot.
 */
int updutmp(entry)
struct utmpx *entry;
{
	int fc, type;
	struct stat stbuf;
	struct utmp ubuf, *uptr = NULL;
	extern int fd_u;

	if (fd_u < 0) {
		if ((fd_u = open(utmpfile, O_RDWR|O_CREAT, 0644)) < 0) {
#ifdef ERRDEBUG
		gxdebug("Could not open utmpfile\n");
#endif
			return(1);
		}
	}

	if ((fc = fcntl(fd_u, F_GETFL, NULL)) == -1) {
		close(fd_u);
		fd_u = -1;
		return(1);
	}

	while (read(fd_u, &ubuf, sizeof(ubuf)) == sizeof(ubuf)) {
		if (ubuf.ut_type != EMPTY) {
			switch (entry->ut_type) {
				case EMPTY:
				    goto done;	
				case RUN_LVL:
				case BOOT_TIME:
				case OLD_TIME:
				case NEW_TIME:
				    if (entry->ut_type == ubuf.ut_type) {
					uptr = &ubuf;
				        goto done;
				    }
				case INIT_PROCESS:
				case LOGIN_PROCESS:
				case USER_PROCESS:
				case DEAD_PROCESS:
				    if (((type = ubuf.ut_type) == INIT_PROCESS
					|| type == LOGIN_PROCESS
					|| type == USER_PROCESS
					|| type == DEAD_PROCESS)
				      && ubuf.ut_id[0] == entry->ut_id[0]
				      && ubuf.ut_id[1] == entry->ut_id[1]
				      && ubuf.ut_id[2] == entry->ut_id[2]
				      && ubuf.ut_id[3] == entry->ut_id[3]) {
					uptr = &ubuf;
				        goto done;
				    }
			}
		}
	}

done:
	if (uptr) 
		lseek(fd_u, -(long)sizeof(ubuf), 1);
	else 
		fcntl(fd_u, F_SETFL, fc|O_APPEND);

	getutmp(entry, &ubuf);
	
	if (write(fd_u, &ubuf, sizeof(ubuf)) != sizeof(ubuf)) {
		close(fd_u);
		fd_u = -1;
		return(1);
	}
	
	fcntl(fd_u, F_SETFL, fc);

	close(fd_u);
	fd_u = -1;

	return(0);
}


/*
 * If one of wtmp and wtmpx files exist, create the other, and the record.
 * If they both exist add the record.
 */
void
updwtmpx(filex, utx)
	const char *filex;
	struct utmpx *utx;
{
	char file[MAXFILE+1];
	struct utmp ut;
	int fd, fdx;

	strcpy(file, filex);
	file[strlen(filex) - 1] = '\0';

	fd = open(file, O_WRONLY | O_APPEND);
	fdx = open(filex, O_WRONLY | O_APPEND);

	if (fd < 0) {
		if (fdx < 0)
			return;
		if ((fd = open(file, O_WRONLY|O_CREAT)) < 0) {
			close(fdx);
			return;
		}
	} else if ((fdx < 0) && ((fdx = open(filex, O_WRONLY|O_CREAT)) < 0)) {
		close(fd);
		return;
	}


	/* Both files exist, synch them */
	if (synchutmp(file, filex))
		goto done;

	/* seek to end of file, in case synchutmp has appended to */
	/* the files. 					 	  */
	lseek(fd, 0, 2); lseek(fdx, 0, 2);

	getutmp(utx, &ut);
	write(fd, &ut, sizeof(struct utmp));
	write(fdx, utx, sizeof(struct utmpx));

done:
	close(fd);
	close(fdx);
}


/*
 * makeutx - create a utmpx entry, recycling an id if a wild card is *	specified.  Also notify init about the new pid
 *
 *	args:	utmpx - point to utmpx structure to be created
 */


struct utmpx *makeutx(utmp)
const struct utmpx *utmp;
{
 	register int i;			/* scratch variable */
	register struct utmpx *utp;	/* "current" utmpx entry being examined */
	int wild;			/* flag, true iff wild card
char seen */
	unsigned char saveid[IDLEN];	/* the last id we matched that was
                                           NOT a dead proc */

	wild = 0;
	for (i = 0; i < IDLEN; i++) {
		if ((unsigned char) utmp->ut_id[i] == SC_WILDC) {
			wild = 1;
			break;
		}
	}

	if (wild) {

/*
 * try to lock the utmpx and utmp files, only needed if we're doing
 * wildcard matching
 */

		if (lockutx()) {
			return((struct utmpx *) NULL);
		}

		setutxent();
		/* find the first alphanumeric character */
		for (i = 0; i < MAXVAL; ++i) {
			if (isalnum(i))
                                break;
		}
		(void) memset(saveid, i, IDLEN);
		while (utp = getutxent()) {
			if (idcmp(utmp->ut_id, utp->ut_id)) {
                                continue;
			}
			else {
                                if (utp->ut_type == DEAD_PROCESS) {
                                        break;
                                }
                                else {
                                        (void) memcpy(saveid, utp->ut_id, IDLEN);
                                }
			}
		}
		if (utp) {

/*
 * found an unused entry, reuse it
 */

			(void) memcpy((char *)(utmp->ut_id), utp->ut_id, IDLEN);
			utp = pututxline(utmp);
			if (utp) 
                                updwtmpx(WTMPX_FILE, utp);
			endutxent();
			unlockutx();
			sendpid(ADDPID, (pid_t)utmp->ut_pid);
			return(utp);
		}
		else {

/*
 * nothing available, try to allocate an id
 */

                        if (allocid(utmp->ut_id, saveid)) {
                                endutxent();
                                unlockutx();
                                return((struct utmpx *) NULL);
                        }
                        else {
                              	utp = pututxline(utmp);
                                if (utp) 
                                        updwtmpx(WTMPX_FILE, utp);
                                endutxent();
                                unlockutx();
                                sendpid(ADDPID, (pid_t)utmp->ut_pid);
                                return(utp);
                        }
		}
	}
        else {
              	utp = pututxline(utmp);
		if (utp) 
                        updwtmpx(WTMPX_FILE, utp);
		endutxent();
		sendpid(ADDPID, (pid_t)utmp->ut_pid);
		return(utp);
	}
}


/*
 * modutx - modify a utmpx entry.  Also notify init about new pids or
 *	old pids that it no longer needs to care about
 *
 *	args:	utp- point to utmpx structure to be created
 */

struct utmpx *modutx(utp)
const struct utmpx *utp;
{
	register int i;				/* scratch variable
*/
	struct utmpx utmp;			/* holding area */
	register struct utmpx *ucp = &utmp;	/* and a pointer to
it */
	struct utmpx *up;			/* "current" utmpx entry being examined */

	for (i = 0; i < IDLEN; ++i) {
		if ((unsigned char) utp->ut_id[i] == SC_WILDC)
			return((struct utmpx *) NULL);
	}
	/* copy the supplied utmpx structure someplace safe */
	utmp = *utp;
	setutxent();
	while (up = getutxent()) {
		if (idcmp(ucp->ut_id, up->ut_id))
			continue;
		/* only get here if ids are the same, i.e. found right entry */
		if (ucp->ut_pid != up->ut_pid) {
			sendpid(REMPID, (pid_t)up->ut_pid);
			sendpid(ADDPID, (pid_t)ucp->ut_pid);
		}
		break;
	}
	up = pututxline(ucp);
	if (ucp->ut_type == DEAD_PROCESS)
		sendpid(REMPID, (pid_t)ucp->ut_pid);
	if (up)
		updwtmpx(WTMPX_FILE, up);
	endutxent();
	return(up);
}


/*
 * idcmp - compare two id strings, return 0 if same, non-zero if not *
 *	args:	s1 - first id string
 *		s2 - second id string
 */


static
idcmp(s1, s2)
register char *s1;
register char *s2;
{
	register int i;		/* scratch variable */

	for (i = 0; i < IDLEN; ++i) {
		if ((unsigned char) *s1 != SC_WILDC && (*s1++ != *s2++))
			return(-1);
	}
	return(0);
}


/*
 * allocid - allocate an unused id for utmp, either by recycling a
 *	DEAD_PROCESS entry or creating a new one.  This routine only *	gets called if a wild card character was specified.
 *
 *	args:	srcid - pattern for new id
 *		saveid - last id matching pattern for a non-dead process
 */


static
allocid(srcid, saveid)
register char *srcid;
register unsigned char *saveid;
{
	register int i;		/* scratch variable */
	int changed;		/* flag to indicate that a new id has been generated */
	char copyid[IDLEN];	/* work area */

	(void) memcpy(copyid, srcid, IDLEN);
	changed = 0;
	for (i = 0; i < IDLEN; ++i) {
		/* if this character isn't wild, it'll be part of the generated id */
		if ((unsigned char) copyid[i] != SC_WILDC)
			continue;
		/* it's a wild character, retrieve the character from the saved id */
		copyid[i] = saveid[i];
		/* if we haven't changed anything yet, try to find a new char to use */
		if (!changed && (saveid[i] < MAXVAL)) {

/*
 * Note: this algorithm is taking the "last matched" id and trying to make
 * a 1 character change to it to create a new one.  Rather than special-case
 * the first time (when no perturbation is really necessary), just don't
 * allocate the first valid id.
 */

			while (++saveid[i] <= MAXVAL) {
                                /* make sure new char is alphanumeric */
                                if (isalnum(saveid[i])) {
                                        copyid[i] = saveid[i];
                                        changed = 1;
                                        break;
                                }
			}
		}
	}
	/* changed is true if we were successful in allocating an id */
	if (changed) {
		(void) memcpy(srcid, copyid, IDLEN);
		return(0);
	}
	else {
		return(-1);
	}
}


/*
 * lockutx - lock utmpx and utmp files
 */

static
lockutx()
{
        if ((fd = open(UTMPX_FILE, O_RDWR|O_CREAT, 0644)) < 0)
                return(-1);
        if ((fd_u = open(UTMP_FILE, O_RDWR|O_CREAT, 0644)) < 0) {
                close(fd);
                fd = -1;
                return(-1);
        }
        if (synchutmp(UTMP_FILE, UTMPX_FILE)) {
                close(fd); fd = -1;
                close(fd_u); fd_u = -1;
                return(-1);
        } 
	if ((lockf(fd, F_LOCK, 0) < 0) || (lockf(fd_u, F_LOCK, 0) <0)) {
		close(fd); close(fd_u);
		fd = -1; fd_u = -1;
		return(-1);
	}
	return(0);
}


/*
 * unlockutx - unlock utmp and utmpx files
 */

static void
unlockutx()
{
	(void) lockf(fd, F_ULOCK, 0);
	(void) lockf(fd_u, F_ULOCK, 0);
	(void) close(fd);
	(void) close(fd_u);
	fd = fd_u = -1;
}


/*
 * sendpid - send message to init to add or remove a pid from the
 *	"godchild" list
 *
 *	args:	cmd - ADDPID or REMPID
 *		pid - pid of "godchild"
 */


static void
sendpid(cmd, pid)
int cmd;
pid_t pid;
{
	int pfd;		/* file desc. for init pipe */
	struct pidrec prec;	/* place for message to be built */

/*
 * if for some reason init didn't open initpipe, open it read/write
 * here to avoid sending SIGPIPE to the calling process
 */

	pfd = open(IPIPE, O_RDWR);
	if (pfd < 0)
		return;
	prec.pd_pid = pid;
	prec.pd_type = cmd;
	(void) write(pfd, &prec, sizeof(struct pidrec));
	(void) close(pfd);
}


#ifdef  ERRDEBUG
#include        <stdio.h>

gxdebug(format,arg1,arg2,arg3,arg4,arg5,arg6)
char *format;
int arg1,arg2,arg3,arg4,arg5,arg6;
{
        register FILE *fp;
        register int errnum;

        if ((fp = fopen("/etc/dbg.getut","a+")) == NULL) return;
        fprintf(fp,format,arg1,arg2,arg3,arg4,arg5,arg6);
        fclose(fp);
}
#endif
