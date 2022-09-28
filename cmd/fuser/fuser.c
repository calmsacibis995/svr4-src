/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fuser:fuser.c	1.27.3.1"



#include <nlist.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/mnttab.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/var.h>
#include <sys/utssys.h>
#ifdef RFSUMOUNTHACK
#include <sys/rf_sys.h>
#endif

void exit(), perror();
extern char *malloc();
extern int errno;

/*
 * Return a pointer to the mount point matching the given special name, if
 * possible, otherwise, exit with 1 if mnttab corruption is detected, else
 * return NULL.
 *
 * NOTE:  the underlying storage for mget and mref is defined static by
 * libos.  Repeated calls to getmntany() overwrite it; to save mnttab
 * structures would require copying the member strings elsewhere.
 */
char *
spec_to_mount(specname)
	char	*specname;
{
	FILE	*frp;
	int 	ret;
	struct mnttab mref, mget;

	/* get mount-point */
	if ((frp = fopen(MNTTAB, "r")) == NULL) {
		return NULL;
	}
	mntnull(&mref);
	mref.mnt_special = specname;
	ret = getmntany(frp, &mget, &mref);
	fclose(frp);
	if (ret == 0) {
		return (mget.mnt_mountp);
	} else if (ret > 0) {
		fprintf(stderr, "mnttab is corrupted\n");
		exit(1);
	} else {
		return NULL;
	}
}

/* symbol names */
#if pdp11 || vax
#define V_STR		"_v"
#endif
#if u3b || u3b15 || u3b2 || i386
#define V_STR		"v"
#endif

#if vax || u3b15 || u3b2 || i386
#define MEMF "/dev/kmem"
#else
#define MEMF "/dev/mem"
#endif

#define SYSTEM "/stand/unix"

/*
 * The main objective of this routine is to allocate an array of f_user_t's.
 * In order for it to know how large an array to allocate, it must know
 * the value of v.v_proc in the kernel.  To get this, use nlist to find
 * the memory address of v, then seek and read the struct var there into
 * the local v.  Return the allocation result.  Exit with a status of 1
 * if any error is encountered in this process.
 */

f_user_t *
get_f_user_buf()
{
	int mem, tmperr;	/* errno is cleared by successful printfs */
	unsigned var_addr;
	struct var v;
	static struct nlist nl[] = {
		{V_STR},
		{""}
	};

	/* open file to access memory */
	if ((mem = open(MEMF, O_RDONLY)) == -1) {
		tmperr = errno;
		fprintf(stderr, "fuser:  open of %s failed: ", MEMF);
		errno = tmperr;
		perror("");
		exit(1);
	}
	/* get values of system variables */
	if (nlist(SYSTEM, nl) == -1) {
		perror("fuser: nlist");
		exit(1);
	}

	if (strcmp(V_STR, nl[0].n_name) || nl[0].n_value == 0) {
		printf("fuser:  %s not found in system image\n", V_STR);
		exit(1);
	}
	var_addr = nl[0].n_value;
#ifdef vax
	var_addr &= 0x3fffffff;
#endif

	if (lseek(mem, (long)var_addr, 0) == -1) {
		tmperr = errno;
		fprintf(stderr, "fuser:  seek of %s failed: ", MEMF);
		errno = tmperr;
		perror("");
		exit(1);
	} else if (read(mem, (caddr_t)&v, sizeof(struct var)) != 
	  sizeof(struct var)) {
		tmperr = errno;
		fprintf(stderr, "fuser:  read of %s failed: ", MEMF);
		errno = tmperr;
		perror("");
		exit(1);
	}
	return (f_user_t *)malloc(v.v_proc * sizeof(f_user_t));
}

/*
 * display the fuser usage message and exit
 */
void
usage()
{
	fprintf(stderr,
	  "Usage:  fuser [-ku[c|f]] files [-[ku[c|f]] files]\n");
	exit(1);
}

struct co_tab {
		int	c_flag;
		char	c_char;
};

static struct co_tab code_tab[] = {
	{F_CDIR, 'c'},		/* current directory */
	{F_RDIR, 'r'},		/* root directory (via chroot) */
	{F_TEXT, 't'},		/* textfile */
	{F_OPEN, 'o'},		/* open (creat, etc.) file */
	{F_MAP, 'm'},		/* mapped file */
	{F_TTY, 'y'},		/* controlling tty */
	{F_TRACE, 'a'}		/* trace file */
};

/*
 * Show pids and usage indicators for the nusers processes in the users list.
 * When usrid is non-zero, give associated login names.  When gun is non-zero,
 * issue kill -9's to those processes.
 */
void
report(users, nusers, usrid, gun)
	f_user_t *users;
	int nusers;
	int usrid;
	int gun;
{
	int cind;

	for ( ; nusers; nusers--, users++) {
		fprintf(stdout, " %7d", users->fu_pid);
		fflush(stdout);
		for (cind = 0; cind < sizeof(code_tab) / sizeof(struct co_tab);
		  cind++) {
			if (users->fu_flags & code_tab[cind].c_flag) {
				fprintf(stderr, "%c", code_tab[cind].c_char);
			}
		}
		if (usrid) {
			/*
			 * print the login name for the process
			 */
			struct passwd *getpwuid(), *pwdp;

			if ((pwdp = getpwuid(users->fu_uid)) != NULL) {
				fprintf(stderr, "(%s)", pwdp->pw_name);
			}
		}
		if (gun) {
			(void)kill(users->fu_pid, 9);
		}
	}
}

/*
 * Determine which processes are using a named file or file system.
 * On stdout, show the pid of each process using each command line file
 * with indication(s) of its use(s).  Optionally display the login
 * name with each process.  Also optionally, issue a kill to each process.
 *
 * When any error condition is encountered, possibly after partially
 * completing, fuser exits with status 1.  If no errors are encountered,
 * exits with status 0.
 *
 * The preferred use of the command is with a single file or file system.
 */

main(argc, argv)
	int argc;
	char **argv;
{
	int gun = 0, usrid = 0, contained = 0, file_only = 0;
	int newfile = 0;
	register i, j, k;
	char *mntname;
	int nusers;
	f_user_t *users;

	if (argc < 2) {
		usage();
	}
	if ((users = get_f_user_buf()) == NULL) {
		fprintf(stderr, "fuser: could not allocate buffer\n");
		exit(1);
	}
	for (i = 1; i < argc; i++) {
		int okay = 0;
#ifdef RFSUMOUNTHACK
		int cap;
		char cappn[MAXPATHLEN];
		int error = 0;
#endif

		if (argv[i][0] == '-') {
			/* options processing */
			if (newfile) {
				gun = usrid = contained = file_only =
				  newfile = 0;
			}
			for (j = 1; argv[i][j] != '\0'; j++) {
				switch(argv[i][j]) {
				case 'k':
					if (gun) {
						usage();
					}
					gun = 1; 
					break;
				case 'u':
					if (usrid) {
						usage();
					}
					usrid = 1; 
					break;
				case 'c':
					if (contained) {
						usage();
					}
					if (file_only) {
						fprintf(stderr, 
"'c' and 'f' can't both be used for a file\n");
						usage();
					}
					contained = 1;
					break;
				case 'f':
					if (file_only) {
						usage();
					}
					if (contained) {
						fprintf(stderr, 
"'c' and 'f' can't both be used for a file\n");
						usage();
					}
					file_only = 1;
					break;
				default:
					fprintf(stderr,
					  "Illegal option %c.\n",
					  argv[i][j]);
					usage();
				}
			}
			continue;
		} else {
			newfile = 1;
		}

/*
* if not file_only, attempt to translate special name to mount point, via
* /etc/mnttab.  issue: utssys -c mount point if found, and utssys special, too?
* take union of results for report?????
*/
		fflush(stdout);
	
		/* First print file name on stderr (so stdout (pids) can
		 * be piped to kill) */
		fprintf(stderr, "%s: ", argv[i]);

		if (!(file_only || contained) && (mntname =
		  spec_to_mount(argv[i])) != NULL) {
#ifdef RFSUMOUNTHACK
			/* Try the RFS interface to do recursive fusers. */
			if ((cap = rfsys(RF_GETCAP, mntname, cappn)) != -1) {
				/* RFS knows how to deal with the pathname */
				if (!(error =
				  cap_fusers(cap, users, &nusers))) {
					report(users, nusers, usrid, gun);
				}
				(void)rfsys(RF_PUTCAP, cap);
				if (error) {
					errno = error;
					perror("fuser");
					exit(1);
				}
				fprintf(stderr,"\n");
				exit(0);
			}
			/* We assume we were barking up the wrong tree. */
			errno = 0;
#endif
			if ((nusers = utssys(mntname, F_CONTAINED, UTS_FUSERS,
			  users)) != -1) {
				report(users, nusers, usrid, gun);
				okay = 1;
			}
		}
#ifdef RFSUMOUNTHACK
		else if ((cap = rfsys(RF_GETCAP, argv[i], cappn)) != -1) {
				/* RFS knows how to deal with the pathname */
			if (!(error = cap_fusers(cap, users, &nusers))) {
				report(users, nusers, usrid, gun);
			}
			(void)rfsys(RF_PUTCAP, cap);
			if (error) {
				errno = error;
				perror("fuser");
				exit(1);
			}
			fprintf(stderr,"\n");
			exit(0);
		}
		/* We assume we were barking up the wrong tree. */
		errno = 0;
#endif
		if ((nusers = utssys(argv[i], contained ? F_CONTAINED : 0,
		   UTS_FUSERS, users)) == -1) {
			if (!okay) {
				perror("fuser");
				exit(1);
			}
		} else {
			report(users, nusers, usrid, gun);
		}
		fprintf(stderr,"\n");
	}

	/* newfile is set when a file is found.  if it isn't set here,
	 * then the user did not use correct syntax  */
	if (!newfile) {
		fprintf(stderr, "fuser: missing file name\n");
		usage();
	}
	exit(0);
}

#ifdef RFSUMOUNTHACK
#define MAXSUBCAP	256

/*
 * generate and return nusers with undocumented rfsys interface
 */
int
cap_fusers(cap, buf, nup)
	int cap;	/* capability for a VFS */
	f_user_t *buf;
	int *nup;
{
	int subcap[MAXSUBCAP];
	int nsubcap;
	int snx;
	int subusers = 0;
	int nusers;
	int error;

	/* get capabilities for submounts */
	if ((nsubcap = rfsys(RF_SUBMNTS, cap, MAXSUBCAP, (caddr_t)subcap))
	  == -1) {
		error = errno;
		errno = 0;
		return errno;
	}

	/* collect users from submounts */
	for (snx = 0; snx < nsubcap; snx++) {
		int moreusers;

		if (error =
		  cap_fusers(subcap[snx], buf + subusers, &moreusers)) {
			for (snx = 0; snx < nsubcap; snx++) {
				(void)rfsys(RF_PUTCAP, subcap[snx]);
			}
			return error;
		}
		subusers += moreusers;
		(void)rfsys(RF_PUTCAP, subcap[snx]);
	}

	/* add in users from this mount */
	if ((nusers =
	  rfsys(RF_FUSERS, cap, F_CONTAINED, buf + subusers)) == -1) {
		error = errno;
		errno = 0;
		return errno;
	}
	*nup = nusers + subusers;
	return 0;
}
#endif
