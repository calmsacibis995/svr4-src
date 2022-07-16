/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getut.c	1.19.1.9"

/*	Routines to read and write the /etc/utmp file.			*/
/*									*/
#ifdef __STDC__
	#pragma weak endutent = _endutent
	#pragma weak getutent = _getutent
	#pragma weak getutid = _getutid
	#pragma weak getutline = _getutline
	#pragma weak getutmp = _getutmp
	#pragma weak getutmpx = _getutmpx
	#pragma weak makeut = _makeut
	#pragma weak modut = _modut
	#pragma weak pututline = _pututline
	#pragma weak setutent = _setutent
	#pragma weak synchutmp = _synchutmp
	#pragma weak updutfile = _updutfile
	#pragma weak updutxfile = _updutxfile
	#pragma weak updutmpx = _updutmpx
	#pragma weak updwtmp = _updwtmp
	#pragma weak utmpname = _utmpname
#endif
#include	"synonyms.h"
#include "shlib.h"
#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<utmpx.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>

#define IDLEN	4	/* length of id field in utmp */
#define SC_WILDC	0xff	/* wild char for utmp ids */
#define	MAXFILE	79	/* Maximum pathname length for "utmp" file */
#define MAXVAL	255	/* max value for an id 'character' */
#define IPIPE	"/etc/initpipe"	/* FIFO to send pids to init */

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

#ifdef ut_time
#undef ut_time
#endif

#ifdef	DEBUG
#undef	UTMP_FILE
#define	UTMP_FILE "utmp"
#undef	UTMPX_FILE
#define	UTMPX_FILE "utmpx"
#endif

extern long	lseek();
extern void	setutent();
extern int	stat(), write(), read(), close();
extern void	unlockut();
extern void	sendpid();

static int fd = -1;	/* File descriptor for the utmp file. */
static int fd_u = -1;	/* File descriptor for the utmpx file. */
static const char *utmpfile = UTMP_FILE;	/* Name of the current */
static const char *utmpxfile = UTMPX_FILE;	/* "utmp" like file.   */

#ifdef ERRDEBUG
static long loc_utmp;	/* Where in "utmp" the current "ubuf" was found.*/
#endif

static struct utmp ubuf;	/* Copy of last entry read in. */


/* "getutent" gets the next entry in the utmp file.
 */

struct utmp *getutent()
{
	extern int fd;
	extern struct utmp ubuf;
	register char *u;
	register int i;
	int fdx;

/* If the "utmp" file is not open, attempt to open it for
 * reading.  If there is no file, attempt to create one.  If
 * both attempts fail, return NULL.  If the file exists, but
 * isn't readable and writeable, do not attempt to create.
 */

	if (fd < 0) {
		if ((fd = open(utmpfile, O_RDWR|O_CREAT, 0644)) < 0) {

/* If the open failed for permissions, try opening it only for
 * reading.  All "pututline()" later will fail the writes.
 */
		if ((fd = open(utmpfile, O_RDONLY)) < 0)
				return(NULL);
		} 
		if (access(utmpxfile, F_OK) < 0) {
			if ((fdx = open(utmpxfile, O_CREAT|O_RDWR, 0644)) < 0) {
				return(NULL);
		}
			close(fdx);
		}

		/* Make sure files are in synch */
		if (synchutmp(utmpfile, utmpxfile))
			return(NULL);
	}

/* Try to read in the next entry from the utmp file.  */
	if (read(fd,&ubuf,sizeof(ubuf)) != sizeof(ubuf)) {

/* Make sure ubuf is zeroed. */
		for (i=0,u=(char *)(&ubuf); i<sizeof(ubuf); i++) *u++ = '\0';
		return(NULL);
	}

/* Save the location in the file where this entry was found. */
	(void) lseek(fd,0L,1);
	return(&ubuf);
}

/*	"getutid" finds the specified entry in the utmp file.  If	*/
/*	it can't find it, it returns NULL.				*/

struct utmp *getutid(entry)
register const struct utmp *entry;
{
	extern struct utmp ubuf;
	struct utmp *getutent();
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
	} while (getutent() != NULL);

/* Return NULL since the proper entry wasn't found. */
	return(NULL);
}

/* "getutline" searches the "utmp" file for a LOGIN_PROCESS or
 * USER_PROCESS with the same "line" as the specified "entry".
 */

struct utmp *getutline(entry)
register const struct utmp *entry;
{
	extern struct utmp ubuf,*getutent();
	register struct utmp *cur;

/* Start by using the entry currently incore.  This prevents */
/* doing reads that aren't necessary. */
	cur = &ubuf;
	do {
/* If the current entry is the one we are interested in, return */
/* a pointer to it. */
		if (cur->ut_type != EMPTY && (cur->ut_type == LOGIN_PROCESS
		    || cur->ut_type == USER_PROCESS) && strncmp(&entry->ut_line[0],
		    &cur->ut_line[0],sizeof(cur->ut_line)) == 0) return(cur);
	} while ((cur = getutent()) != NULL);

/* Since entry wasn't found, return NULL. */
	return(NULL);
}

/*	"pututline" writes the structure sent into the utmp file.	*/
/*	If there is already an entry with the same id, then it is	*/
/*	overwritten, otherwise a new entry is made at the end of the	*/
/*	utmp file.							*/

struct utmp *pututline(entry)
const struct utmp *entry;
{
	int fc;
	struct utmp *answer;
	extern struct utmp ubuf;
	extern struct utmp *getutid();
	extern int fd;
	struct utmp tmpbuf, savbuf;

/* Copy the user supplied entry into our temporary buffer to */
/* avoid the possibility that the user is actually passing us */
/* the address of "ubuf". */
	tmpbuf = *entry;
	(void)getutent();
	if (fd < 0) {
#ifdef	ERRDEBUG
		gdebug("pututline: Unable to create utmp file.\n");
#endif
		return((struct utmp *)NULL);
	}
/* Make sure file is writable */
	if ((fc=fcntl(fd, F_GETFL, NULL)) == -1
	    || (fc & O_RDWR) != O_RDWR) {
		return((struct utmp *)NULL);
	}

/* Find the proper entry in the utmp file.  Start at the current */
/* location.  If it isn't found from here to the end of the */
/* file, then reset to the beginning of the file and try again. */
/* If it still isn't found, then write a new entry at the end of */
/* the file.  (Making sure the location is an integral number of */
/* utmp structures into the file incase the file is scribbled.) */

	if (getutid(&tmpbuf) == NULL) {
#ifdef	ERRDEBUG
		gdebug("First getutid() failed.  fd: %d",fd);
#endif
		setutent();
		if (getutid(&tmpbuf) == NULL) {
#ifdef	ERRDEBUG
			loc_utmp = lseek(fd, 0L, 1);
			gdebug("Second getutid() failed.  fd: %d loc_utmp: %ld\n",fd,loc_utmp);
#endif
			fcntl(fd, F_SETFL, fc | O_APPEND);
		} else {
			lseek(fd, -(long)sizeof(struct utmp), 1);
		}
	} else {
		lseek(fd, -(long)sizeof(struct utmp), 1);
	}

/* Write out the user supplied structure.  If the write fails, */
/* then the user probably doesn't have permission to write the */
/* utmp file. */
	if (write(fd,&tmpbuf,sizeof(tmpbuf)) != sizeof(tmpbuf)) {
#ifdef	ERRDEBUG
		gdebug("pututline failed: write-%d\n",errno);
#endif
		answer = (struct utmp *)NULL;
	} else {
/* Save the user structure that was overwritten. Copy the new user  */
/* structure into ubuf so that it will be up to date in the future. */
		savbuf = ubuf;
		ubuf = tmpbuf;
		answer = &ubuf;

#ifdef	ERRDEBUG
		gdebug("id: %c%c loc: %x\n",ubuf.ut_id[0],ubuf.ut_id[1],
		    ubuf.ut_id[2],ubuf.ut_id[3],loc_utmp);
#endif
	}
/* update the parallel utmpx file */
	if (updutmpx(entry)) {
		lseek(fd, -(long)sizeof(struct utmp), 1);
		write(fd, &savbuf, sizeof(savbuf));
		answer = (struct utmp *)NULL;
	}

	fcntl(fd, F_SETFL, fc);
	return(answer);
}

/*	"setutent" just resets the utmp file back to the beginning.	*/

void
setutent()
{
	register char *ptr;
	register int i;
	extern int fd;
	extern struct utmp ubuf;

	if (fd != -1) lseek(fd,0L,0);

/* Zero the stored copy of the last entry read, since we are */
/* resetting to the beginning of the file. */

	for (i=0,ptr=(char*)&ubuf; i < sizeof(ubuf);i++) *ptr++ = '\0';
}

/*	"endutent" closes the utmp file.				*/

void
endutent()
{
	extern int fd;
	extern struct utmp ubuf;
	register char *ptr;
	register int i;

	if (fd != -1) close(fd);
	fd = -1;
	for (i=0,ptr= (char *)(&ubuf); i < sizeof(ubuf);i++) *ptr++ = '\0';
}

/*	"utmpname" allows the user to read a file other than the	*/
/*	normal "utmp" file.						*/
utmpname(newfile)
const char *newfile;
{
	static char *saveptr;
	static int savelen = 0;
	int len;

/* Determine if the new filename will fit.  If not, return 0. */
	if ((len = strlen(newfile)) >= MAXFILE) return (0);

	/* malloc enough space for utmp, utmpx, and null bytes */
	if (len > savelen)
	{
		if (saveptr)
			free(saveptr);
		if ((saveptr = malloc(2 * len + 3)) == 0)
			return (0);
		savelen = len;
	}

	/* copy in the new file name. */
	utmpfile = (const char *)saveptr;
	(void)strcpy(saveptr, newfile);
	utmpxfile = (const char *)saveptr + len + 2;
	(void)strcpy(saveptr + len + 2, newfile);
	strcat(saveptr + len + 2, "x");

/* Make sure everything is reset to the beginning state. */
	endutent();
	return(1);
}

/* "updutmpx" updates the utmpx file. Uses the same
 * search algorithm as pututline to make sure records
 * end up in the same place. 
 */
int updutmpx(entry)
struct utmp *entry;
{
	int fc, type;
	struct stat stbuf;
	struct utmpx uxbuf, *uxptr = NULL;
	extern int fd_u;

	if(fd_u < 0) {
		if ((fd_u = open(utmpxfile, O_RDWR|O_CREAT, 0644)) < 0) {
#ifdef ERRDEBUG
		gdebug("Could not open utmpxfile\n");
#endif
			return(1);
		}
	}

	if ((fc = fcntl(fd_u, F_GETFL, NULL)) == -1) {
		close(fd_u);
		fd_u = -1;
		return(1);
	}

	while (read(fd_u, &uxbuf, sizeof(uxbuf)) == sizeof(uxbuf)) {
		if (uxbuf.ut_type != EMPTY) {
			switch (entry->ut_type) {
				case EMPTY:
				    goto done;	
				case RUN_LVL:
				case BOOT_TIME:
				case OLD_TIME:
				case NEW_TIME:
				    if (entry->ut_type == uxbuf.ut_type) {
					uxptr = &uxbuf;
				        goto done;
				    }
				case INIT_PROCESS:
				case LOGIN_PROCESS:
				case USER_PROCESS:
				case DEAD_PROCESS:
				    if (((type = uxbuf.ut_type) == INIT_PROCESS
					|| type == LOGIN_PROCESS
					|| type == USER_PROCESS
					|| type == DEAD_PROCESS)
				      && uxbuf.ut_id[0] == entry->ut_id[0]
				      && uxbuf.ut_id[1] == entry->ut_id[1]
				      && uxbuf.ut_id[2] == entry->ut_id[2]
				      && uxbuf.ut_id[3] == entry->ut_id[3]) {
					uxptr = &uxbuf;
				        goto done;
				    }
			}
		}
	}

done:	
	if (uxptr)
		lseek(fd_u, -(long)sizeof(uxbuf), 1);
	else 
		fcntl(fd_u, F_SETFL, fc|O_APPEND);

	getutmpx(entry, &uxbuf);
	
	if (write(fd_u, &uxbuf, sizeof(uxbuf)) != sizeof(uxbuf)) {
#ifdef ERRDEBUG
		gdebug("updutmpx failed: write-%d\n", errno);
#endif
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
updwtmp(file, ut)
	const char *file;
	struct utmp *ut;
{
	char filex[256];
	struct utmpx utx;
	int fd, fdx;

	strcpy(filex, file);
	strcat(filex, "x");

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
	
	write(fd, ut, sizeof(struct utmp));
	getutmpx(ut, &utx);
	write(fdx, &utx, sizeof(struct utmpx));

done:
	close(fd);
	close(fdx);
}


/*
 * "getutmp" - convert a utmpx record to a utmp record.
 */
void
getutmp(utx, ut)
        const struct utmpx *utx;
        struct utmp  *ut;
{
        strncpy(ut->ut_user, utx->ut_user, sizeof(ut->ut_user));
        strncpy(ut->ut_line, utx->ut_line, sizeof(ut->ut_line));
	(void) memcpy(ut->ut_id, utx->ut_id, sizeof(utx->ut_id));
        ut->ut_pid = utx->ut_pid;
        ut->ut_type = utx->ut_type;
        ut->ut_exit = utx->ut_exit;
        ut->ut_time = utx->ut_tv.tv_sec;
}


/*
 * "getutmpx" - convert a utmp record to a utmpx record.
 */
void
getutmpx(ut, utx)
	const struct utmp *ut;
	struct utmpx *utx;
{
        strncpy(utx->ut_user, ut->ut_user, sizeof(ut->ut_user));
	(void) memset(&utx->ut_user[sizeof(ut->ut_user)], '\0',
	    sizeof(utx->ut_user) - sizeof(ut->ut_user));
        strncpy(utx->ut_line, ut->ut_line, sizeof(ut->ut_line));
	(void) memset(&utx->ut_line[sizeof(ut->ut_line)], '\0',
	    sizeof(utx->ut_line) - sizeof(ut->ut_line));
	(void) memcpy(utx->ut_id, ut->ut_id, sizeof(ut->ut_id));
        utx->ut_pid = ut->ut_pid;
        utx->ut_type = ut->ut_type;
        utx->ut_exit = ut->ut_exit;
        utx->ut_tv.tv_sec = ut->ut_time;
        utx->ut_tv.tv_usec = 0;
	utx->ut_session = 0;
	(void) memset(utx->pad, 0, sizeof(utx->pad));
	(void) memset(utx->ut_host, '\0', sizeof(utx->ut_host));
}


/* "synchutmp" make sure utmp and utmpx files are in synch.
 * Returns an error code if the files are not multiples
 * of their respective struct size. Updates the out of 
 * date file.
*/
synchutmp(utf, utxf)
	char *utf, *utxf;
{
	struct stat stbuf, stxbuf;

	if (stat(utf, &stbuf) == 0 &&
				stat(utxf, &stxbuf) == 0) {
		/* Make sure file is a multiple of 'utmp'  entries long */
		if((stbuf.st_size % sizeof(struct utmp)) != 0 ||
		   (stxbuf.st_size % sizeof(struct utmpx)) != 0) {
			errno = EINVAL;
			return(1);
		}

		if (stbuf.st_size) {
			if (!stxbuf.st_size)
				return(updutxfile(utf, utxf));
		} else if (stxbuf.st_size)
			return(updutfile(utf, utxf));
				
		if (abs(stxbuf.st_mtime-stbuf.st_mtime) >= MOD_WIN) {
			/* files are out of sync */
			if (stxbuf.st_mtime > stbuf.st_mtime) 
				return(updutfile(utf, utxf));
			else 
				return(updutxfile(utf, utxf));
		}
		return(0);
	}
	return(1);
}



/* "updutfile" updates the utmp file using the contents of the
 * umptx file.
 */
updutfile(utf, utxf)
	char *utf, *utxf;
{
	struct utmpx utx;
	struct utmp  ut;
	int fd1, fd2, n;

	if ((fd1 = open(utf, O_RDWR|O_TRUNC)) < 0)
		return(1);

	if ((fd2 = open(utxf, O_RDONLY)) < 0) {
		close(fd1);
		return(1);
	}

	while ((n = read(fd2, &utx, sizeof(utx))) == sizeof(utx)) {
		getutmp(&utx, &ut);
		if (write(fd1, &ut, sizeof(ut)) != sizeof(ut)) {
			close(fd1);
			close(fd2);
			return(1);
		}
	}
	close(fd1);
	close(fd2);
	utime(utxf, NULL);
	return(0);
}


/* "updutxfile" updates the utmpx file using the contents of the 
 * utmp file. Tries to preserve the host information as much
 * as possible.
 */
updutxfile(utf, utxf)
	char *utf, *utxf;
{
	struct utmp  ut;
	struct utmpx utx;
	int fd1, fd2;
	int n1, n2, cnt=0;

	if ((fd1 = open(utf, O_RDONLY)) < 0)
		return(1);
	if ((fd2 = open(utxf, O_RDWR|O_TRUNC)) < 0) {
		close(fd1);
		return(1);
	}

	/* As long as the entries match, copy the records from the
	 * utmpx file to keep the host information.
	 */
	while ((n1 = read(fd1, &ut, sizeof(ut))) == sizeof(ut)) {
		if ((n2 = read(fd2, &utx, sizeof(utx))) != sizeof(utx)) 
			break;
		if (ut.ut_pid != utx.ut_pid || ut.ut_type != utx.ut_type 
		   || !memcmp(ut.ut_id, utx.ut_id, sizeof(ut.ut_id))
		   || ! memcmp(ut.ut_line, utx.ut_line, sizeof(ut.ut_line))) {
			getutmpx(&ut, &utx);
			lseek(fd2, -(long)sizeof(struct utmpx), 1);
			if (write(fd2, &utx, sizeof(utx)) != sizeof(utx)) {
				close(fd1);
				close(fd2);
				return(1);
			}
			cnt += sizeof(struct utmpx); 
		}
	}

	/* out of date file is shorter, copy from the up to date file
	 * to the new file.
	 */
	if (n1 > 0) {
		do {
			getutmpx(&ut, &utx);
			if (write(fd2, &utx, sizeof(utx)) != sizeof(utx)) {
				close(fd1);
				close(fd2);
				return(1);
			}
		} while ((n1 = read(fd1, &ut, sizeof(ut))) == sizeof(ut));
	} else {
		/* out of date file was longer, truncate it */
		truncate(utxf, cnt);
	}

	close(fd1);
	close(fd2);
	utime(utf, NULL);
	return(0);
}

/*
 * makeut - create a utmp entry, recycling an id if a wild card is
 *	specified.  Also notify init about the new pid
 *
 *	args:	utmp - point to utmp structure to be created
 */


struct utmp *makeut(utmp)
register struct utmp *utmp;
{
	register int i;			/* scratch variable */
	register struct utmp *utp;	/* "current" utmp entry being examined */
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
 * try to lock the utmp file, only needed if we're doing wildcard matching
 */

                if (lockut())
                        return((struct utmp *) NULL);

                setutent();
		/* find the first alphanumeric character */
		for (i = 0; i < MAXVAL; ++i) {
                        if (isalnum(i))
                                break;
		}
                (void) memset(saveid, i, IDLEN);
		while (utp = getutent()) {
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

 			(void) memcpy(utmp->ut_id, utp->ut_id, IDLEN);
			utp = pututline(utmp);
			if (utp)
                                updwtmp(WTMP_FILE, utp);
			endutent();
			unlockut();
			sendpid(ADDPID, (pid_t)utmp->ut_pid);
			return(utp);
		}
		else {

/*
 * nothing available, try to allocate an id
 */

			if (allocid(utmp->ut_id, saveid)) {
                                endutent();
                                unlockut();
                                return((struct utmp *) NULL);
			}
			else {
                                utp = pututline(utmp);
                                if (utp)
                                        updwtmp(WTMP_FILE, utp);
                                endutent();
                                unlockut();
                                sendpid(ADDPID, (pid_t)utmp->ut_pid);
                                return(utp);
			}
		}
	}
	else {
		utp = pututline(utmp);
		if (utp)
			updwtmp(WTMP_FILE, utp);
		endutent();
		sendpid(ADDPID, (pid_t)utmp->ut_pid);
		return(utp);
	}
}


/*
 * modut - modify a utmp entry.	 Also notify init about new pids or
 *	old pids that it no longer needs to care about
 *
 *	args:	utmp - point to utmp structure to be created
 */


struct utmp *modut(utp)
register struct utmp *utp;
{
	register int i;				/* scratch variable
*/
	struct utmp utmp;			/* holding area */
	register struct utmp *ucp = &utmp;	/* and a pointer to
it */
	struct utmp *up;			/* "current" utmp entry being examined */

	for (i = 0; i < IDLEN; ++i) {
		if ((unsigned char) utp->ut_id[i] == SC_WILDC)
			return((struct utmp *) NULL);
	}
	/* copy the supplied utmp structure someplace safe */
	utmp = *utp;
	setutent();
	while (up = getutent()) {
		if (idcmp(ucp->ut_id, up->ut_id))
                        continue;
		/* only get here if ids are the same, i.e. found right entry */
           	if (ucp->ut_pid != up->ut_pid) {
                        sendpid(REMPID, (pid_t)up->ut_pid);
                        sendpid(ADDPID, (pid_t)ucp->ut_pid);
		}
                break;
	}
        up = pututline(ucp);
	if (ucp->ut_type == DEAD_PROCESS)
		sendpid(REMPID, (pid_t)ucp->ut_pid);
	if (up)
                updwtmp(WTMP_FILE, up);
	endutent();
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
 * lockut - lock utmp and utmpx files
 */


static
lockut()
{
 	if ((fd = open(UTMP_FILE, O_RDWR|O_CREAT, 0644)) < 0)
		return(-1);
	if ((fd_u = open(UTMPX_FILE, O_RDWR|O_CREAT, 0644)) < 0) {
		close(fd);
		fd = -1;
		return(-1);
	}
	if (synchutmp(UTMP_FILE, UTMPX_FILE)) {
		close(fd); fd = -1;
		close(fd_u); fd_u = -1;
		return(-1);
	}
		
	if ((lockf(fd, F_LOCK, 0) < 0) || (lockf(fd_u, F_LOCK, 0) < 0)) {
		close(fd); close(fd_u);
		fd = fd_u = -1;
		return(-1);
	}
	return(0);
}


/*
 * unlockut - unlock utmp and utmpx files
 */


static void
unlockut()
{
	(void) lockf(fd, F_ULOCK, 0);
	(void) lockf(fd_u, F_ULOCK, 0);
	(void) close(fd);
	(void) close(fd_u);
	fd = -1; fd_u = -1;
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

gdebug(format,arg1,arg2,arg3,arg4,arg5,arg6)
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
