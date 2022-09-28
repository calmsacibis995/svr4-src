/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)gcore:gcore.c	1.1.1.1"

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

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/file.h>

#ifdef i386
#include <sys/immu.h>
#else
#include <sys/psw.h>
#include <sys/immu.h>
#include <sys/pcb.h>
#endif

#include <sys/user.h>
#include <sys/sysmacros.h>
#include <sys/procfs.h>
#include <sys/elf.h>

#ifdef i386
#include <sys/elf_386.h>
#else
#include <sys/elf_M32.h>
#endif

#include <sys/mman.h>

#define	TRUE	1
#define	FALSE	0

/* Error returns from Pgrab() */
#define	G_NOPROC	(-1)	/* No such process */
#define	G_ZOMB		(-2)	/* Zombie process */
#define	G_PERM		(-3)	/* No permission */
#define	G_BUSY		(-4)	/* Another process has control */
#define	G_SYS		(-5)	/* System process */
#define	G_SELF		(-6)	/* Process is self */
#define	G_STRANGE	(-7)	/* Unanticipated error, perror() was called */
#define	G_INTR		(-8)	/* Interrupt received while grabbing */

#define	TADDR	((caddr_t)(0x80800000))

typedef struct {
	Elf32_Word namesz;
	Elf32_Word descsz;
	Elf32_Word type;
	char	   name[8];
} Elf32_Note;

#define NT_PRSTATUS 1
#define NT_PRFPREG  2
#define NT_PRPSINFO 3

extern	void	exit();
extern	void	perror();
extern	unsigned alarm();
extern	long	lseek();
extern	int	ioctl();
extern	int	stat();
extern	long	ulimit();
extern	long	strtol();

extern	int	getopt();
extern	char *	optarg;
extern	int	optind;

static	void	alrm();
static	pid_t	getproc();
static	int	dumpcore();
static	int	grabit();
static	int	isprocdir();
static	int	Pgrab();
static	int	Ioctl();

char *	command = NULL;		/* name of command ("gcore") */
char *	filename = "core";	/* default filename prefix */
char *	procdir = "/proc";	/* default PROC directory */
int	timeout = FALSE;	/* set TRUE by SIGALRM catcher */
long	buf[4096];		/* big buffer, used for almost everything */

main(argc, argv)
	int argc;
	char **argv;
{
	int retc = 0;
	int opt;
	int errflg = FALSE;

	command = argv[0];

	/* options */
	while ((opt = getopt(argc, argv, "o:p:")) != EOF) {
		switch (opt) {
		case 'o':		/* filename prefix (default "core") */
			filename = optarg;
			break;
		case 'p':		/* alternate /proc directory */
			procdir = optarg;
			break;
		default:
			errflg = TRUE;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (errflg || argc <= 0) {
		(void) fprintf(stderr,
			"usage:\t%s [-o filename] [-p procdir] pid ...\n",
			command);
		exit(2);
	}

	if (!isprocdir(procdir)) {
		(void) fprintf(stderr,
			"%s: %s is not a PROC directory\n",
			command, procdir);
		exit(2);
	}

	/* catch alarms */
	(void) sigset(SIGALRM, alrm);

	while (--argc >= 0) {
		int pfd;
		pid_t pid;
		char *pdir;

		/* get the specified pid and its /proc directory */
		pid = getproc(*argv++, &pdir);

		if (pid < 0 || (pfd = grabit(pdir, pid)) < 0) {
			retc++;
			continue;
		}

		if (dumpcore(pfd, pid) != 0)
			retc++;

		(void) close(pfd);
	}

	return(retc);
}

static pid_t		/* get process id and /proc directory */
getproc(path, pdirp)	/* return pid on success, -1 on failure */
	register char * path;	/* number or /proc/nnn */
	char ** pdirp;		/* points to /proc directory on success */
{
	register char * name;
	register pid_t pid;
	char *next;

	if ((name = strrchr(path, '/')) != NULL)	/* last component */
		*name++ = '\0';
	else {
		name = path;
		path = procdir;
	}

	pid = strtol(name, &next, 10);
	if (isdigit(*name) && pid >= 0 && *next == '\0') {
		if (strcmp(procdir, path) != 0
		 && !isprocdir(path)) {
			(void) fprintf(stderr,
				"%s: %s is not a PROC directory\n",
				command, path);
			pid = -1;
		}
	} else {
		(void) fprintf(stderr, "%s: invalid process id: %s\n",
			command, name);
		pid = -1;
	}

	if (pid >= 0)
		*pdirp = path;
	return(pid);
}

static int
grabit(dir, pid)		/* take control of an existing process */
	char * dir;
	pid_t pid;
{
	int gcode;

	gcode = Pgrab(dir, pid);

	if (gcode >= 0)
		return(gcode);
	
	if (gcode == G_INTR)
		return(-1);

	(void) fprintf(stderr, "%s: %s.%d not dumped", command, filename, pid);
	switch (gcode) {
	case G_NOPROC:
		(void) fprintf(stderr, ": %d: No such process", pid);
		break;
	case G_ZOMB:
		(void) fprintf(stderr, ": %d: Zombie process", pid);
		break;
	case G_PERM:
		(void) fprintf(stderr, ": %d: Permission denied", pid);
		break;
	case G_BUSY:
		(void) fprintf(stderr, ": %d: Process is traced", pid);
		break;
	case G_SYS:
		(void) fprintf(stderr, ": %d: System process", pid);
		break;
	case G_SELF:
		(void) fprintf(stderr, ": %d: Cannot dump self", pid);
		break;
	}
	(void) fputc('\n', stderr);

	return(-1);
}

/*ARGSUSED*/
static void
alrm(sig)
	int sig;
{
	timeout = TRUE;
}

	
static int
dumpcore(pfd, pid)
	int pfd;		/* process file descriptor */
	pid_t pid;		/* process-id */
{
	int dfd;			/* dump file descriptor */
	int nsegments;			/* current number of segments */
	char * bp = (char *)&buf[0];	/* pointer to big buffer */
	Elf32_Ehdr ehdr;		/* ELF header */
	Elf32_Phdr *v;			/* ELF program header */
	prmap_t *pdp = (prmap_t *)bp;
	prstatus_t piocstat;
	prpsinfo_t psstat;
	fpregset_t fpregs;
	ulong hdrsz;
	off_t poffset;
	int nhdrs, i;
	int size, count, ncount;
	char cname[MAXPATHLEN];

	/*
	 * Fetch the memory map and look for text, data, and stack.
	 */
	if (Ioctl(pfd, PIOCNMAP, (int)&nsegments) == -1
				  || nsegments <= 0) {
		perror("dumpcore(): PIOCNMAP");
		return(-1);
	}
	if (nsegments >= (sizeof(buf)/sizeof(prmap_t))) {
		(void) fprintf(stderr, "dumpcore(): too many segments\n");
		return(-1);
	}
	if (Ioctl(pfd, PIOCMAP, (int)pdp) == -1) {
		perror("dumpcore(): PIOCMAP");
		return(-1);
	}

	nhdrs = nsegments + 1;
	hdrsz = nhdrs * sizeof(Elf32_Phdr);
	
	v = (Elf32_Phdr *)malloc(hdrsz);

	memset(&ehdr, sizeof(Elf32_Ehdr),0);
	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = ELFCLASS32;
	ehdr.e_ident[EI_DATA] = ELFDATA2MSB;

#ifdef i386
	ehdr.e_type = ET_CORE;
	ehdr.e_machine = EM_386;
#else
	ehdr.e_type = ET_CORE|EF_M32_MAU;
	ehdr.e_machine = EM_M32;
#endif

        ehdr.e_version = EV_CURRENT;
        ehdr.e_phoff = sizeof(Elf32_Ehdr);
        ehdr.e_ehsize = sizeof(Elf32_Ehdr);
        ehdr.e_phentsize = sizeof(Elf32_Phdr);
        ehdr.e_phnum = nhdrs;

	/*
	 * Create the core dump file.
	 */
	(void) sprintf(cname, "%s.%d", filename, pid);
	if ((dfd = creat(cname, 0666)) < 0) {
		perror(cname);
		return(-1);
	}

	if (write(dfd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		perror("dumpcore(): write");
		return(-1);
	}	

	poffset = sizeof(Elf32_Ehdr) + hdrsz;
	v[0].p_type = PT_NOTE;
        v[0].p_flags = PF_R;
        v[0].p_offset = poffset;
        v[0].p_filesz = (sizeof(Elf32_Note) * 3 ) + 
		roundup(sizeof(prstatus_t), sizeof(Elf32_Word)) +
		roundup(sizeof(prpsinfo_t), sizeof(Elf32_Word)) +
		roundup(sizeof(fpregset_t), sizeof(Elf32_Word));
	poffset += v[0].p_filesz;

	for (i = 1; i < nhdrs; i++, pdp++) {
		caddr_t naddr;		
		v[i].p_type = PT_LOAD;
		v[i].p_vaddr = (Elf32_Word) pdp->pr_vaddr;
		naddr = pdp->pr_vaddr;
		while ((naddr += PAGESIZE) < (pdp->pr_vaddr + pdp->pr_size))
			;
		size = naddr - pdp->pr_vaddr;
		v[i].p_memsz = size;
		if (pdp->pr_mflags & MA_WRITE)
			v[i].p_flags |= PF_W;
		if (pdp->pr_mflags & MA_READ)
			v[i].p_flags |= PF_R;
		if (pdp->pr_mflags & MA_EXEC)
			v[i].p_flags |= PF_X;
		if ((pdp->pr_mflags & (MA_WRITE|MA_EXEC)) != MA_EXEC) {
			v[i].p_offset = poffset;
			v[i].p_filesz = size;
			poffset += size;
		}	
	}

	if (write(dfd, v, hdrsz) != hdrsz) {
		perror("dumpcore(): write");
		return(-1);
	}	

	if (Ioctl(pfd, PIOCSTATUS, (int)&piocstat) == -1) {
		perror("dumpcore(): PIOCSTATUS");
		return(-1);
	}
	elfnote(dfd, NT_PRSTATUS, (char *)&piocstat, sizeof(prstatus_t));

	if (Ioctl(pfd, PIOCPSINFO, (int)&psstat) == -1) {
		perror("dumpcore(): PIOCPSINFO");
		return(-1);
	}
	elfnote(dfd, NT_PRPSINFO, (char *)&psstat, sizeof(prpsinfo_t));

	if (Ioctl(pfd, PIOCGFPREG, (int)&fpregs) == -1) {
		perror("dumpcore(): PIOCGFPREG");
		return(-1);
	}
	elfnote(dfd, NT_PRFPREG, (char *)&fpregs, sizeof(fpregset_t));

	/*
	 * Dump data and stack
	 */
	for (i = 1; i<nhdrs; i++) {
		if (v[i].p_filesz == 0)
			continue;
		(void) lseek(pfd, v[i].p_vaddr, 0);
		count = (v[i].p_filesz > sizeof(buf)) ? 
						sizeof(buf) : v[i].p_filesz;
		while (count > 0) {
			if ((ncount = read(pfd, buf, count)) <= 0)
				break;
			(void) write(dfd, buf, ncount); 
			count -= ncount;
		}
	}
		
	(void) fprintf(stderr,"%s: %s.%d dumped\n", command, filename, pid);
	(void) close(dfd);
	return(0);
}


static int
elfnote(dfd, type, ptr, size)
	int dfd;
	int type;
	char *ptr;
	int size;
{
	Elf32_Note note;		/* ELF note */

	memset(&note, sizeof(Elf32_Note), 0);
	memcpy("CORE", note.name, 4);
	note.type = type;
	note.namesz = 8;
	note.descsz = roundup(size, sizeof(Elf32_Word));
	(void) write(dfd, &note, sizeof(Elf32_Note));
	(void) write(dfd, ptr, size);
}

 

static int
isprocdir(dir)	/* return TRUE iff dir is a PROC directory */
	char *dir;	/* this is filthy */
{
	/* This is based on the fact that "/proc/0" and "/proc/00" are the */
	/* same file, namely process 0, and are not linked to each other. */

	struct stat stat1;	/* dir/0  */
	struct stat stat2;	/* dir/00 */
	char * path = (char *)&buf[0];
	register char * p;

	/* make a copy of the directory name without trailing '/'s */
	if (dir == NULL)
		(void) strcpy(path, ".");
	else {
		(void) strcpy(path, dir);
		p = path + strlen(path);
		while (p > path && *--p == '/')
			*p = '\0';
		if (*path == '\0')
			(void) strcpy(path, ".");
	}

	/* append "/0" to the directory path and stat() the file */
	p = path + strlen(path);
	*p++ = '/';
	*p++ = '0';
	*p = '\0';
	if (stat(path, &stat1) != 0)
		return(FALSE);

	/* append "/00" to the directory path and stat() the file */
	*p++ = '0';
	*p = '\0';
	if (stat(path, &stat2) != 0)
		return(FALSE);

	/* see if we ended up with the same file */
	if (stat1.st_dev   != stat2.st_dev
	 || stat1.st_ino   != stat2.st_ino
	 || stat1.st_mode  != stat2.st_mode
	 || stat1.st_nlink != stat2.st_nlink
	 || stat1.st_uid   != stat2.st_uid
	 || stat1.st_gid   != stat2.st_gid
	 || stat1.st_size  != stat2.st_size)
		return(FALSE);

	/* return TRUE iff we have a regular file with a single link */
	return ((stat1.st_mode&S_IFMT) == S_IFREG && stat1.st_nlink == 1);
}

static int
Pgrab(pdir, pid)		/* grab existing process */
	char *pdir;			/* /proc directory */
	register pid_t pid;		/* UNIX process ID */
{
	register int pfd = -1;
	int err;
	prstatus_t prstat;
	char * procname = (char *)&buf[0];

again:	/* Come back here if we lose it in the Window of Vulnerability */
	if (pfd >= 0) {
		(void) close(pfd);
		pfd = -1;
	}

	/* generate the /proc/pid filename */
	(void) sprintf(procname, "%s/%d", pdir, pid);

	/* Request exclusive open to avoid grabbing someone else's	*/
	/* process and to prevent others from interfering afterwards.	*/
	if ((pfd = open(procname, (O_RDWR|O_EXCL))) < 0) {
		switch (errno) {
		case EBUSY:
			return(G_BUSY);
		case ENOENT:
			return(G_NOPROC);
		case EACCES:
		case EPERM:
			return(G_PERM);
		default:
			perror("Pgrab open()");
			return(G_STRANGE);
		}
	}

	/* Make sure the filedescriptor is not one of 0, 1, or 2 */
	if (0 <= pfd && pfd <= 2) {
		int dfd = fcntl(pfd, F_DUPFD, 3);

		(void) close(pfd);
		if (dfd < 0) {
			perror("Pgrab fcntl()");
			return(G_STRANGE);
		}
		pfd = dfd;
	}

	/* ---------------------------------------------------- */
	/* We are now in the Window of Vulnerability (WoV).	*/
	/* The process may exec() a setuid/setgid or unreadable	*/
	/* object file between the open() and the PIOCSTOP.	*/
	/* We will get EAGAIN in this case and must start over.	*/
	/* ---------------------------------------------------- */

	/*
	 * Get the process's status.
	 */
	if (Ioctl(pfd, PIOCSTATUS, (int)&prstat) == -1) {
		int rc;

		if (errno == EAGAIN)	/* WoV */
			goto again;

		if (errno == ENOENT)	/* Don't complain about zombies */
			rc = G_ZOMB;
		else {
			perror("Pgrab PIOCSTATUS");
			rc = G_STRANGE;
		}
		(void) close(pfd);
		return(rc);
	}

	/*
	 * If the process is a system process, we can't dump it.
	 */
	if (prstat.pr_flags & PR_ISSYS) {
		(void) close(pfd);
		return(G_SYS);
	}

	/*
	 * We can't dump ourself.
	 */
	if (pid == getpid()) {
		/*
		 * Verify that the process is really ourself:
		 * Set a magic number, read it through the
		 * /proc file and see if the results match.
		 */
		long magic1 = 0;
		long magic2 = 2;

		if (lseek(pfd, (long)&magic1, 0) == (long)&magic1
		 && read(pfd, (char *)&magic2, sizeof(magic2)) == sizeof(magic2)
		 && magic2 == 0
		 && (magic1 = 0xfeedbeef)
		 && lseek(pfd, (long)&magic1, 0) == (long)&magic1
		 && read(pfd, (char *)&magic2, sizeof(magic2)) == sizeof(magic2)
		 && magic2 == 0xfeedbeef) {
			(void) close(pfd);
			return(G_SELF);
		}
	}

	/*
	 * If the process is already stopped or has been directed
	 * to stop via /proc, there us nothing more to do.
	 */
	if (prstat.pr_flags & (PR_ISTOP|PR_DSTOP))
		return(pfd);

	/*
	 * Mark the process run-on-last-close so
	 * it runs even if we die from SIGKILL.
	 */
	if (Ioctl(pfd, PIOCSRLC, 0) == -1) {
		int rc;

		if (errno == EAGAIN)	/* WoV */
			goto again;

		if (errno == ENOENT)	/* Don't complain about zombies */
			rc = G_ZOMB;
		else {
			perror("Pgrab PIOCSRLC");
			rc = G_STRANGE;
		}
		(void) close(pfd);
		return(rc);
	}
	
	/*
	 * Direct the process to stop.
	 * Set an alarm to avoid waiting forever.
	 */
	timeout = FALSE;
	err = 0;
	(void) alarm(2);
	if (Ioctl(pfd, PIOCSTOP, (int)&prstat) == 0)
		(void) alarm(0);
	else {
		err = errno;
		(void) alarm(0);
		if (err == EINTR
		 && timeout
		 && Ioctl(pfd, PIOCSTATUS, (int)&prstat) != 0) {
			timeout = FALSE;
			err = errno;
		}
	}

	if (err) {
		int rc;

		switch (err) {
		case EAGAIN:		/* we lost control of the the process */
			goto again;
		case EINTR:		/* timeout or user typed DEL */
			rc = G_INTR;
			break;
		case ENOENT:
			rc = G_ZOMB;
			break;
		default:
			perror("Pgrab PIOCSTOP");
			rc = G_STRANGE;
			break;
		}
		if (!timeout || err != EINTR) {
			(void) close(pfd);
			return(rc);
		}
	}

	/*
	 * Process should either be stopped via /proc or
	 * there should be an outstanding stop directive.
	 */
	if ((prstat.pr_flags & (PR_ISTOP|PR_DSTOP)) == 0) {
		(void) fprintf(stderr, "Pgrab: process is not stopped\n");
		(void) close(pfd);
		return(G_STRANGE);
	}

	return(pfd);
}

static int
Ioctl(fd, request, arg)		/* deal with RFS congestion */
	int fd;
	int request;
	int arg;
{
	register int rc;

	for(;;) {
		if ((rc = ioctl(fd, request, arg)) != -1
		 || errno != ENOMEM)
			return(rc);
	}
}


