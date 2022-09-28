/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/ufsdump/dumpoptr.c	1.6.3.1"

#include "dump.h"

/*
 *	This is from /usr/include/grp.h
 *	That defined struct group, which conflicts
 *	with the struct group defined in param.h
 */
struct	Group { /* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};
struct	Group *getgrnam();

/*
 *	Query the operator; This fascist piece of code requires
 *	an exact response.
 *	It is intended to protect dump aborting by inquisitive
 *	people banging on the console terminal to see what is
 *	happening which might cause dump to croak, destroying
 *	a large number of hours of work.
 *
 *	Every 2 minutes we reprint the message, alerting others
 *	that dump needs attention.
 */
int	timeout;
char	*attnmessage;		/* attention message */
query(question)
	char	*question;
{
	char	replybuffer[64];
	int	back;
	FILE	*mytty;

	if ( (mytty = fopen("/dev/tty", "r")) == NULL){
		msg("fopen on /dev/tty fails\n");
		exit(-1);
	}
	attnmessage = question;
	timeout = 0;
	alarmcatch();
	for(;;){
		if ( fgets(replybuffer, 63, mytty) == NULL){
			if (ferror(mytty)){
				clearerr(mytty);
				continue;
			}
		} else if ( (strcmp(replybuffer, "yes\n") == 0) ||
			    (strcmp(replybuffer, "Yes\n") == 0)){
				back = 1;
				goto done;
		} else if ( (strcmp(replybuffer, "no\n") == 0) ||
			    (strcmp(replybuffer, "No\n") == 0)){
				back = 0;
				goto done;
		} else {
			msg("\"Yes\" or \"No\"?\n");
			alarmcatch();
		}
	}
    done:
	/*
	 *	Turn off the alarm, and reset the signal to trap out..
	 */
	alarm(0);
	if (signal(SIGALRM, sigalrm) == SIG_IGN)
		signal(SIGALRM, SIG_IGN);
	fclose(mytty);
	return(back);
}
/*
 *	Alert the console operator, and enable the alarm clock to
 *	sleep for 2 minutes in case nobody comes to satisfy dump
 */
void
alarmcatch()
{
	if (timeout)
		msgtail("\n");
	msg("NEEDS ATTENTION: %s: (\"yes\" or \"no\") ",
		attnmessage);
	signal(SIGALRM, alarmcatch);
	alarm(120);
	timeout = 1;
}
/*
 *	Here if an inquisitive operator interrupts the dump program
 */
void
ufsinterrupt()
{
	msg("Interrupt received.\n");
	if (query("Do you want to abort dump?"))
		dumpabort();
	signal(SIGINT, ufsinterrupt);
}

/*
 *	The following variables and routines manage alerting
 *	operators to the status of dump.
 *	This works much like wall(1) does.
 */
struct	Group *gp;

/*
 *	Get the names from the group entry "operator" to notify.
 */	
set_operators()
{
	if (!notify)		/*not going to notify*/
		return;
	gp = getgrnam(OPGRENT);
	endgrent();
	if (gp == (struct Group *)0){
		msg("No entry in /etc/group for %s.\n",
			OPGRENT);
		notify = 0;
		return;
	}
}

struct tm *localtime();
struct tm *localclock;

/*
 *	We fork a child to do the actual broadcasting, so
 *	that the process control groups are not messed up
 */
broadcast(message)
	char	*message;
{
	time_t		clock;
	FILE	*f_utmp;
	struct	utmp	utmp;
	int	nusers;
	char	**np;
	int	pid, s;

	switch (pid = fork()) {
	case -1:
		return;
	case 0:
		break;
	default:
		while (wait(&s) != pid)
			continue;
		return;
	}

	if (!notify || gp == 0)
		exit(0);
	clock = time(0);
	localclock = localtime(&clock);

	if((f_utmp = fopen("/var/adm/utmp", "r")) == NULL) {
		msg("Cannot open /var/adm/utmp\n");
		return;
	}

	nusers = 0;
	while (!feof(f_utmp)){
		if (fread(&utmp, sizeof (struct utmp), 1, f_utmp) != 1)
			break;
		if (utmp.ut_name[0] == 0)
			continue;
		nusers++;
		for (np = gp->gr_mem; *np; np++){
			if (strncmp(*np, utmp.ut_name, sizeof(utmp.ut_name)) != 0)
				continue;
			/*
			 *	Do not send messages to operators on dialups
			 */
			if (strncmp(utmp.ut_line, DIALUP, strlen(DIALUP)) == 0)
				continue;
#ifdef DEBUG
			msg("Message to %s at %s\n",
				utmp.ut_name, utmp.ut_line);
#endif /* DEBUG */
			sendmes(utmp.ut_line, message);
		}
	}
	fclose(f_utmp);
	Exit(0);	/* the wait in this same routine will catch this */
	/* NOTREACHED */
}

sendmes(tty, message)
	char *tty, *message;
{
	char t[50], buf[BUFSIZ];
	register char *cp;
	register int c, ch;
	int	msize;
	FILE *f_tty;

	msize = strlen(message);
	strcpy(t, "/dev/");
	strcat(t, tty);

	if((f_tty = fopen(t, "w")) != NULL) {
		setbuf(f_tty, buf);
		fprintf(f_tty, "\nMessage from the dump program to all operators at %d:%02d ...\r\n\n"
		       ,localclock->tm_hour
		       ,localclock->tm_min);
		for (cp = message, c = msize; c-- > 0; cp++) {
			ch = *cp;
			if (ch == '\n')
				putc('\r', f_tty);
			putc(ch, f_tty);
		}
		fclose(f_tty);
	}
}

/*
 *	print out an estimate of the amount of time left to do the dump
 */

time_t	tschedule = 0;

timeest()
{
	time_t	tnow, deltat;

	time (&tnow);
	if (tnow >= tschedule){
		tschedule = tnow + 300;
		if (blockswritten < 500)
			return;	
		deltat = tstart_writing - tnow +
			(((1.0*(tnow - tstart_writing))/blockswritten) * esize);
		msg("%3.2f%% done, finished in %d:%02d\n",
			(blockswritten*100.0)/esize,
			deltat/3600, (deltat%3600)/60);
	}
}

int blocksontape()
{
	/*
	 *	esize: total number of blocks estimated over all reels
	 *	blockswritten:	blocks actually written, over all reels
	 *	etapes:	estimated number of tapes to write
	 *
	 *	tsize:	blocks can write on this reel
	 *	asize:	blocks written on this reel
	 *	tapeno:	number of tapes written so far
	 */
	if (tapeno == etapes)
		return(esize - (etapes - 1)*tsize);
	return(tsize);
}

	/* VARARGS1 */
	/* ARGSUSED */
msg(fmt, a1, a2, a3, a4, a5)
	char	*fmt;
	int	a1, a2, a3, a4, a5;
{
	fprintf(stderr,"  UFSDUMP: ");
#ifdef TDEBUG
	fprintf(stderr,"pid=%d ", getpid());
#endif
	fprintf(stderr, fmt, a1, a2, a3, a4, a5);
	fflush(stdout);
	fflush(stderr);
}

	/* VARARGS1 */
	/* ARGSUSED */
msgtail(fmt, a1, a2, a3, a4, a5)
	char	*fmt;
	int	a1, a2, a3, a4, a5;
{
	fprintf(stderr, fmt, a1, a2, a3, a4, a5);
}
/*
 *	Tell the operator what has to be done;
 *	we don't actually do it
 */

struct vfstab *
allocfsent(fs)
	register struct vfstab *fs;
{
	register struct vfstab *new;
	register char *cp;
	char *malloc();

	new = (struct vfstab *)malloc(sizeof (*fs));
	cp = malloc(strlen(fs->vfs_mountp) + 1);
	strcpy(cp, fs->vfs_mountp);
	new->vfs_mountp = cp;
	cp = malloc(strlen(fs->vfs_fstype) + 1);
	strcpy(cp, fs->vfs_fstype);
	new->vfs_fstype = cp;
	cp = malloc(strlen(fs->vfs_special) + 1);
	strcpy(cp, fs->vfs_special);
	new->vfs_special = cp;
#ifdef never
	new->fs_passno = fs->fs_passno;
	new->fs_freq = fs->fs_freq;
#endif /* never */
	return (new);
}

struct	pfstab {
	struct	pfstab *pf_next;
	struct	vfstab *pf_fstab;
};

static	struct pfstab *table = NULL;

getfstab()
{
	struct vfstab *fs;
	struct vfstab vfsbuf;
	register struct pfstab *pf;
	FILE	*vfstab;
	int	status;

	if ((vfstab = fopen(VFSTAB, "r")) == 0) {
		msg("Can't open %s for dump table information.\n", VFSTAB);
		return;
	}
	while ((status = getvfsent(vfstab, &vfsbuf)) == NULL) {
		if (strcmp(vfsbuf.vfs_fstype, MNTOPT_RW) &&
		    strcmp(vfsbuf.vfs_fstype, MNTOPT_RO) &&
		    strcmp(vfsbuf.vfs_fstype, MNTOPT_RQ))
			continue;
		fs = allocfsent(&vfsbuf);
		pf = (struct pfstab *)malloc(sizeof (*pf));
		pf->pf_fstab = fs;
		pf->pf_next = table;
		table = pf;
	}
	fclose (vfstab);
}

/*
 * Search in the vfstab for a file name.
 * This file name can be either the special or the path file name.
 *
 * The entries in the vfstab are the BLOCK special names, not the
 * character special names.
 * The caller of fstabsearch assures that the character device
 * is dumped (that is much faster)
 *
 * The file name can omit the leading '/'.
 */
struct vfstab *
fstabsearch(key)
	char *key;
{
	register struct pfstab *pf;
	register struct vfstab *fs;
	char *rawname();

	if (table == NULL)
		return ((struct vfstab *)0);
	for (pf = table; pf; pf = pf->pf_next) {
		fs = pf->pf_fstab;
		if (strcmp(fs->vfs_mountp, key) == 0)
			return (fs);
		if (strcmp(fs->vfs_special, key) == 0)
			return (fs);
		if (strcmp(rawname(fs->vfs_special), key) == 0)
			return (fs);
		if (key[0] != '/'){
			if (*fs->vfs_special == '/' &&
			    strcmp(fs->vfs_special + 1, key) == 0)
				return (fs);
			if (*fs->vfs_mountp == '/' &&
			    strcmp(fs->vfs_mountp + 1, key) == 0)
				return (fs);
		}
	}
	return (0);
}

/*
 *	Tell the operator what to do
 */
lastdump(arg)
	char	arg;		/* w ==> just what to do; W ==> most recent dumps */
{
			char	*lastname;
			char	*date;
	register	int	i;
			time_t	tnow;
	register	struct	vfstab	*dt;
			int	dumpme;
	register	struct	idates	*itwalk;

	int	idatesort();

	time(&tnow);
	getfstab();		/* /etc/vfstab input */
	inititimes();		/* /etc/dumpdates input */
	qsort(idatev, nidates, sizeof(struct idates *), idatesort);

	if (arg == 'w')
		fprintf(stdout, "Dump these file systems:\n");
	else
		fprintf(stdout, "Last dump(s) done (Dump '>' file systems):\n");
	lastname = "??";
	ITITERATE(i, itwalk){
		if (strncmp(lastname, itwalk->id_name, sizeof(itwalk->id_name)) == 0)
			continue;
		date = (char *)ctime(&itwalk->id_ddate);
		date[16] = '\0';		/* blast away seconds and year */
		lastname = itwalk->id_name;
		dt = fstabsearch(itwalk->id_name);
		dumpme = (  (dt != 0)
#ifdef never
			 && (dt->fs_file != 0)
			 && (itwalk->id_ddate < tnow - (dt->fs_freq*DAY))
#endif /* never */
		);
		if ( (arg != 'w') || dumpme)
		  fprintf(stdout,"%c %8s\t(%6s) Last dump: Level %c, Date %s\n",
			dumpme && (arg != 'w') ? '>' : ' ',
			itwalk->id_name,
			dt ? dt->vfs_mountp : "",
			itwalk->id_incno,
			date
		    );
	}
}

int	idatesort(p1, p2)
	struct	idates	**p1, **p2;
{
	int	diff;

	diff = strncmp((*p1)->id_name, (*p2)->id_name, sizeof((*p1)->id_name));
	if (diff == 0)
		return ((*p2)->id_ddate - (*p1)->id_ddate);
	else
		return (diff);
}

int max(a,b)
	int a, b;
{
	return(a>b?a:b);
}
int min(a,b)
	int a, b;
{
	return(a<b?a:b);
}
