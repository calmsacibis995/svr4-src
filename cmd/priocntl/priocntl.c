/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)priocntl:priocntl.c	1.6.3.1"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<search.h>
#include	<unistd.h>
#include	<sys/types.h>

#ifdef i386
#include	<sys/user.h>
#endif

#include	<dirent.h>
#include	<fcntl.h>
#include	<sys/procset.h>
#include	<sys/priocntl.h>
#include	<sys/procfs.h>
#include	<macros.h>
#include	<errno.h>

#include	"priocntl.h"

/*
 * This file contains the code implementing the class independent part
 * of the priocntl command.  Most of the useful work for the priocntl
 * command is done by the class specific sub-commands, the code for
 * which is elsewhere.  The class independent part of the command is
 * responsible for executing the appropriate class specific sub-commands
 * and providing any necessary input to the sub-commands.
 * Code in this file should never assume any knowledge of any specific
 * scheduler class (other than the SYS class).
 */

#define	CLASSPATH	"/usr/lib/class"

typedef struct classpids {
	char	clp_clname[PC_CLNMSZ];
	pid_t	*clp_pidlist;
	int	clp_pidlistsz;
	int	clp_npids;
} classpids_t;

static char usage[] =
"usage:	priocntl -l\n\
	priocntl -d [-i idtype] [idlist]\n\
	priocntl -s [-c class] [c.s.o.] [-i idtype] [idlist]\n\
	priocntl -e [-c class] [c.s.o.] command [argument(s)]\n";

static char	basenm[BASENMSZ];
static char	cmdpath[256];

static char	*procdir = "/proc";

static void	print_classlist(), set_procs(), exec_cmd(), print_procs();
static void	ids2pids(), add_pid_tolist();
static boolean_t	idmatch();


main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind, opterr;

	int		c;
	int		lflag, dflag, sflag, eflag, cflag, iflag, csoptsflag;
	char		*clname;
	char		*idtypnm;
	idtype_t	idtype;
	int		idargc;
	char		**idargv;

	strcpy(cmdpath, argv[0]);
	strcpy(basenm, basename(argv[0]));
	lflag = dflag = sflag = eflag = cflag = iflag = csoptsflag = 0;
	opterr = 0;
	while ((c = getopt(argc, argv, "ldsec:i:")) != -1) {

		switch(c) {

		case 'l':
			lflag++;
			break;

		case 'd':
			dflag++;
			break;

		case 's':
			sflag++;
			break;

		case 'e':
			eflag++;
			break;

		case 'c':
			cflag++;
			clname = optarg;
			break;

		case 'i':
			iflag++;
			idtypnm = optarg;
			break;

		case '?':
			if (strcmp(argv[optind - 1], "-c") == 0 ||
			    strcmp(argv[optind - 1], "-i") == 0) {

				/*
				 * getopt() will return ? if either
				 * of these appear without an argument.
				 */
				fatalerr(usage);
			}

			/*
			 * We assume for now that any option that
			 * getopt() doesn't recognize (with the
			 * exception of c and i) is intended for a
			 * class specific subcommand.  For now we also
			 * require that all class specific options
			 * take an argument (until we can get smarter
			 * about parsing our options).
			 */
			csoptsflag++;
			optind++;
			break;

		default:
			break;
		}
	}

	if (lflag) {
		if (dflag || sflag || eflag || cflag || iflag || csoptsflag)
			fatalerr(usage);
		print_classlist();
		exit(0);

	} else if (dflag) {
		if (lflag || sflag || eflag || cflag || csoptsflag)
			fatalerr(usage);
		if (iflag) {
			if (str2idtyp(idtypnm, &idtype) == -1)
				fatalerr("%s: bad idtype %s\n", cmdpath,
				    idtypnm);
		} else
			idtype = P_PID;

		if (optind < argc) {
			idargc = argc - optind;
			idargv = &argv[optind];
		} else
			idargc = 0;

		print_procs(idtype, idargc, idargv);
		exit(0);

	} else if (sflag) {
		if (lflag || dflag || eflag)
			fatalerr(usage);
		if (iflag) {
			if (str2idtyp(idtypnm, &idtype) == -1)
				fatalerr("%s: bad idtype %s\n", cmdpath,
				    idtypnm);
		} else
			idtype = P_PID;

		if (cflag == 0)
			clname = NULL;

		if (optind < argc) {
			idargc = argc - optind;
			idargv = &argv[optind];
		} else
			idargc = 0;

		set_procs(clname, idtype, idargc, idargv, argv);

	} else if (eflag) {
		if (lflag || dflag || sflag || iflag)
			fatalerr(usage);

		if (cflag == 0)
			clname = NULL;

		if (optind >= argc)
			fatalerr(usage);

		exec_cmd(clname, argv);

	} else {
		fatalerr(usage);
	}
}


/*
 * Print the heading for the class list and execute the class
 * specific sub-command with the -l option for each configured class.
 */
static void
print_classlist()
{
	id_t		cid;
	int		nclass;
	pcinfo_t	pcinfo;
	static char	subcmdpath[128];
	int		status;
	pid_t		pid;

	/*
	 * No special privileges required for this operation.
	 * Set the effective UID back to the real UID.
	 */
	if (setuid(getuid()) == -1)
		fatalerr("%s: Can't set effective UID back to real UID\n",
		    cmdpath);

	if ((nclass = priocntl(0, 0, PC_GETCLINFO, NULL)) == -1)
		fatalerr("%s: Can't get number of configured classes, priocntl \
system call failed with errno %d\n", cmdpath, errno);

	printf("CONFIGURED CLASSES\n==================\n\n");
	printf("SYS (System Class)\n");
	for (cid = 1; cid < nclass; cid++) {
		printf("\n");
		fflush(stdout);
		pcinfo.pc_cid = cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			fatalerr("%s: can't get class name (class ID = %d)\n",
			    cmdpath, cid);
		sprintf(subcmdpath, "%s/%s/%s%s", CLASSPATH, pcinfo.pc_clname,
		    pcinfo.pc_clname, basenm);
		if ((pid = fork()) == 0) {
			(void)execl(subcmdpath, subcmdpath, "-l", (char *)0);
			printf("%s\n", pcinfo.pc_clname);
			fatalerr("\tCan't execute %s specific subcommand\n",
			    pcinfo.pc_clname);
		} else if (pid == (pid_t)-1) {
			printf("%s\n", pcinfo.pc_clname);
			fprintf(stderr, "Can't execute %s specific subcommand)\n",
			    pcinfo.pc_clname);
		} else {
			wait(&status);
		}
	}
}


/*
 * For each class represented within the set of processes specified by
 * idtype/idargv, print_procs() executes the class specific sub-command
 * with the -d option.  We pipe to each sub-command a list of pids in
 * the set belonging to that class.
 */
static void
print_procs(idtype, idargc, idargv)
idtype_t	idtype;
int		idargc;
char		**idargv;
{
	int		i;
	id_t		id;
	id_t		idlist[NIDS];
	int		nids;
	classpids_t	*clpids;
	int		nclass;
	id_t		cid;
	pcinfo_t	pcinfo;
	int		maxpids, pidexists;
	FILE		*pipe_to_subcmd;
	char		subcmd[128];


	/*
	 * Build a list of ids eliminating any duplicates in idargv.
	 */
	nids = 0;
	if (idargc == 0 && idtype != P_ALL) {

		/*
		 * No ids supplied by user; use current id.
		 */
		if (getmyid(idtype, &idlist[0]) == -1)
			fatalerr("%s: Can't get ID for current process, \
idtype = %d\n", cmdpath, idtype);
		nids = 1;
	} else {
		for (i = 0; i < idargc && nids < NIDS; i++) {
			if (idtype == P_CID) {
				if ((id = clname2cid(idargv[i])) == -1)
					fprintf(stderr, "%s: Invalid \
or unconfigured class %s in idlist - ignored\n", cmdpath, idargv[i]);
			} else {
				id = (id_t)atol(idargv[i]);
			}

			/*
			 * lsearch(3C) adds ids to the idlist,
			 * eliminating duplicates.
			 */
			(void)lsearch((char *)&id, (char *)idlist,
			    (unsigned int *)&nids, sizeof(id), idcompar);
		}
	}

	if ((nclass = priocntl(0, 0, PC_GETCLINFO, NULL)) == -1)
		fatalerr("%s: Can't get number of configured classes, priocntl \
system call failed with errno %d\n", cmdpath, errno);

	if ((clpids= (classpids_t *)malloc(sizeof(classpids_t)*nclass)) == NULL)
		fatalerr("%s: Can't allocate memory for clpids.\n", cmdpath);

	for (cid = 1; cid < nclass; cid++) {
		pcinfo.pc_cid = cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			fatalerr("%s: Can't get class name, cid = %d\n",
			    cmdpath, cid);
		strcpy(clpids[cid].clp_clname, pcinfo.pc_clname);

		/*
		 * Allocate memory for the pidlist.  We won't know for sure
		 * how many pids we'll need until we actually build the list
		 * but we try not to be too wasteful here.  We allocate enough
		 * min(NPIDS , nids * our best guess at the max number
		 * of pids a given id will yield).
		 */
		maxpids = idtyp2maxprocs(idtype);
		if (idtype == P_ALL)
			maxpids = min(maxpids, NPIDS);
		else
			maxpids = min(maxpids * nids, NPIDS);
		if ((clpids[cid].clp_pidlist =
		    (pid_t *)malloc(maxpids * sizeof(pid_t))) == NULL)
			fatalerr("%s: Can't allocate memory for pidlist.\n",
			    cmdpath);
		clpids[cid].clp_pidlistsz = maxpids;
		clpids[cid].clp_npids = 0;
	}

	/*
	 * Build the pidlist.
	 */
	ids2pids(idtype, idlist, nids, clpids, nclass);

	/*
	 * No need for special privileges any more.
	 * Set the effective UID back to the real UID.
	 */
	if (setuid(getuid()) == -1)
		fatalerr("%s: Can't set effective UID back to real UID\n",
		    cmdpath);

	pidexists = 0;
	for (cid = 1; cid < nclass; cid++) {
		if (clpids[cid].clp_npids == 0)
			continue;
		pidexists = 1;
		sprintf(subcmd, "%s/%s/%s%s -d", CLASSPATH, clpids[cid].clp_clname,
		    clpids[cid].clp_clname, basenm);
		if ((pipe_to_subcmd = popen(subcmd, "w")) == NULL) {
			printf("%s\n", clpids[cid].clp_clname);
			fprintf(stderr,"Can't execute %s specific subcommand\n",
			    clpids[cid].clp_clname);
		}
		fwrite(clpids[cid].clp_pidlist, sizeof(pid_t),
		    clpids[cid].clp_npids, pipe_to_subcmd);
		(void)pclose(pipe_to_subcmd);
	}
	if (pidexists == 0)
		fatalerr("%s: Process(es) not found.\n", cmdpath);
}


/*
 * Execute the appropriate class specific sub-command with the arguments
 * pointed to by subcmdargv.  If the user specified a class we simply
 * exec the sub-command for that class.  If no class was specified we
 * verify that the processes in the set specified by idtype/idargv are
 * all in the same class and then execute the sub-command for that class.
 */
static void
set_procs(clname, idtype, idargc, idargv, subcmdargv)
char		*clname;
idtype_t	idtype;
int		idargc;
char		**idargv;
char		**subcmdargv;
{
	char			idstr[12];
	char			myidstr[12];
	char			clnmbuf[PC_CLNMSZ];
	pcparms_t		pcparms;
	pcinfo_t		pcinfo;
	static prpsinfo_t	prinfo;
	static prcred_t		prcred;
	DIR			*dirp;
	struct dirent		*dentp;
	static char		pname[100];
	int			procfd;
	int			saverr;
	static char		subcmdpath[128];
	boolean_t		procinset;

	if (clname == NULL && idtype == P_PID && idargc <= 1 ) {

		/*
		 * No class specified by user but only one process
		 * in specified set.  Get the class the easy way.
		 */
		pcparms.pc_cid = PC_CLNULL;
		if (idargc == 0) {
			if (priocntl(P_PID, P_MYID, PC_GETPARMS, &pcparms) == -1)
				if (errno == ESRCH)
					fatalerr("%s: Process not found.\n",
					    cmdpath);
				else
					fatalerr("%s: Can't get class of \
current process\npriocntl system call failed with errno %d\n", cmdpath, errno);
		} else {
			/* idargc == 1 */
			if (priocntl(P_PID, atoi(idargv[0]), PC_GETPARMS, &pcparms)
			    == -1)
				if (errno == ESRCH)
					fatalerr("%s: Process not found.\n",
					    cmdpath);
				else
					fatalerr("%s: Can't get class of \
specified  process\npriocntl system call failed with errno %d\n", cmdpath, errno);
		}
		pcinfo.pc_cid = pcparms.pc_cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			fatalerr("%s: Can't get class name (class ID = %d)\n",
			    cmdpath, pcinfo.pc_cid);
		clname = pcinfo.pc_clname;
	} else if (clname == NULL) {

		/*
		 * No class specified by user and potentially more
		 * than one process in specified set.  Verify that
		 * all procs in set are in the same class.
		 */
		if (idargc == 0 && idtype != P_ALL) {

			/*
			 * No ids supplied by user; use current id.
			 */
			if (getmyidstr(idtype, myidstr) == -1)
				fatalerr("%s: Can't get ID string for current \
process, idtype = %d\n", cmdpath, idtype);
		}
		if ((dirp = opendir(procdir)) == NULL)
			fatalerr("%s: Can't open PROC directory %s\n",
			    cmdpath, procdir);

		while ((dentp = readdir(dirp)) != NULL) {
			if (dentp->d_name[0] == '.')	/* skip . and .. */
				continue;

			sprintf(pname, "%s/%s", procdir, dentp->d_name);
retry:
			if ((procfd = open(pname, O_RDONLY)) < 0)
				continue;

			if (ioctl(procfd, PIOCPSINFO, (char *)&prinfo) == -1) {
				saverr = errno;
				close(procfd);
				if (saverr == EAGAIN)
					goto retry;
				if (saverr != ENOENT) {
					fprintf(stderr,
					    "%s: Can't get process info for %s\n",
					    cmdpath, pname);
				}
				continue;
			}
			if (idtype == P_UID || idtype == P_GID) {
				if (ioctl(procfd, PIOCCRED, (char *)&prcred) == -1) {
					saverr = errno;
					close(procfd);
					if (saverr == EAGAIN)
						goto retry;
					if (saverr != ENOENT) {
						fprintf(stderr,
						    "%s: Can't get process credentials \
for %s\n",
						    cmdpath, pname);
					}
					continue;
				}
			}
			close(procfd);

			if (prinfo.pr_state == 0 || prinfo.pr_zomb != 0)
				continue;


			switch (idtype) {

			case P_PID:
				itoa((long)prinfo.pr_pid, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_PPID:
				itoa((long)prinfo.pr_ppid, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_PGID:
				itoa((long)prinfo.pr_pgrp, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_SID:
				itoa((long)prinfo.pr_sid, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_CID:
				procinset = idmatch(prinfo.pr_clname, myidstr,
				    idargc, idargv);
				break;

			case P_UID:
				itoa((long)prcred.pr_euid, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_GID:
				itoa((long)prcred.pr_egid, idstr);
				procinset = idmatch(idstr, myidstr,
				    idargc, idargv);
				break;

			case P_ALL:
				procinset = B_TRUE;
				break;

			default:
				fatalerr("%s: Bad idtype %d in set_procs()\n",
				    cmdpath, idtype);
			}
			if (procinset == B_TRUE) {
				if (clname == NULL) {

					/*
					 * First proc found in set.
					 */
					strcpy(clnmbuf, prinfo.pr_clname);
					clname = clnmbuf;
				}else if (strcmp(clname,prinfo.pr_clname) != 0){
					fatalerr("%s: Specified processes \
from different classes.\n", cmdpath);
				}
			}
		}
		closedir(dirp);
		if (clname == NULL)
			fatalerr("%s: Process(es) not found.\n", cmdpath);
	} else {

		/*
		 * User specified class. Check it for validity.
		 */
		strcpy(pcinfo.pc_clname, clname);
		if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
			fatalerr("%s: Invalid or unconfigured class %s\n",
			    cmdpath, clname);
	}

	/*
	 * No need for special privileges any more.
	 * Set the effective UID back to the real UID.
	 */
	if (setuid(getuid()) == -1)
		fatalerr("%s: Can't set effective UID back to real UID\n",
		    cmdpath);

	sprintf(subcmdpath, "%s/%s/%s%s", CLASSPATH, clname, clname, basenm);
	subcmdargv[0] = subcmdpath;
	(void)execv(subcmdpath, subcmdargv);
	fatalerr("%s: Can't execute %s sub-command\n", cmdpath, clname);
}


/*
 * Execute the appropriate class specific sub-command with the arguments
 * pointed to by subcmdargv.  If the user specified a class we simply
 * exec the sub-command for that class.  If no class was specified we
 * execute the sub-command for our own current class.
 */
static void
exec_cmd(clname, subcmdargv)
char	*clname;
char	**subcmdargv;
{
	pcparms_t	pcparms;
	pcinfo_t	pcinfo;
	char		subcmdpath[128];

	/*
	 * No special privileges required for this operation.
	 * Set the effective UID back to the real UID.
	 */
	if (setuid(getuid()) == -1)
		fatalerr("%s: Can't set effective UID back to real UID\n",
		    cmdpath);

	if (clname == NULL) {
		pcparms.pc_cid = PC_CLNULL;
		if (priocntl(P_PID, P_MYID, PC_GETPARMS, &pcparms) == -1) {
			fatalerr("%s: Can't get class of current process\n\
priocntl system call failed with errno %d\n", cmdpath, errno);
		}
		pcinfo.pc_cid = pcparms.pc_cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			fatalerr("%s: Can't get class name (class ID = %d)\n",
			    cmdpath, pcinfo.pc_cid);
		clname = pcinfo.pc_clname;
	} else {

		/*
		 * User specified class. Check it for validity.
		 */
		strcpy(pcinfo.pc_clname, clname);
		if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
			fatalerr("%s: Invalid or unconfigured class %s\n",
			    cmdpath, clname);
	}

	sprintf(subcmdpath, "%s/%s/%s%s", CLASSPATH, clname, clname, basenm);
	subcmdargv[0] = subcmdpath;
	(void)execv(subcmdpath, subcmdargv);
	fatalerr("%s: Can't execute %s sub-command\n", cmdpath, clname);
}


/*
 * Fill in the classpids structures in the array pointed to by clpids
 * with pids for the processes in the set specified by idtype/idlist.
 * We use the /proc PIOCPSINFO operation to get the necessary process
 * information.
 */
static void
ids2pids(idtype, idlist, nids, clpids, nclass)
idtype_t	idtype;
id_t		*idlist;
int		nids;
classpids_t	*clpids;
int		nclass;
{
	static prpsinfo_t	prinfo;
	static prcred_t		prcred;
	DIR			*dirp;
	struct dirent		*dentp;
	char			pname[100];
	int			procfd;
	int			saverr;
	int			i;
	char			*clname;

	if ((dirp = opendir(procdir)) == NULL)
		fatalerr("%s: Can't open PROC directory %s\n",
		    cmdpath, procdir);

	while ((dentp = readdir(dirp)) != NULL) {
		if (dentp->d_name[0] == '.')	/* skip . and .. */
			continue;

		sprintf(pname, "%s/%s", procdir, dentp->d_name);
retry:
		if ((procfd = open(pname, O_RDONLY)) < 0)
			continue;
		if (ioctl(procfd, PIOCPSINFO, (char *)&prinfo) == -1) {
			saverr = errno;
			close(procfd);
			if (saverr == EAGAIN)
				goto retry;
			if (saverr != ENOENT) {
				fprintf(stderr,
				    "%s: Can't get process info for %s\n",
				    cmdpath, pname);
			}
			continue;
		}
		if (idtype == P_UID || idtype == P_GID) {
			if (ioctl(procfd, PIOCCRED, (char *)&prcred) == -1) {
				saverr = errno;
				close(procfd);
				if (saverr == EAGAIN)
					goto retry;
				if (saverr != ENOENT) {
					fprintf(stderr,
					    "%s: Can't get process credentials \
for %s\n",
					    cmdpath, pname);
				}
				continue;
			}
		}
		close(procfd);

		if (prinfo.pr_state == 0 || prinfo.pr_zomb != 0)
			continue;

		switch (idtype) {

		case P_PID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prinfo.pr_pid)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_PPID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prinfo.pr_ppid)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_PGID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prinfo.pr_pgrp)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_SID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prinfo.pr_sid)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_CID:
			for (i = 0; i < nids; i++) {
				clname = clpids[idlist[i]].clp_clname;
				if (strcmp(clname, prinfo.pr_clname) == 0)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_UID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prcred.pr_euid)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_GID:
			for (i = 0; i < nids; i++) {
				if (idlist[i] == (id_t)prcred.pr_egid)
					add_pid_tolist(clpids, nclass,
					    prinfo.pr_clname, prinfo.pr_pid);
			}
			break;

		case P_ALL:
			add_pid_tolist(clpids, nclass, prinfo.pr_clname,
			    prinfo.pr_pid);
			break;

		default:
			fatalerr("%s: Bad idtype %d in ids2pids()\n",
			    cmdpath, idtype);
		}
	}
	closedir(dirp);
}


/*
 * Search the array pointed to by clpids for the classpids
 * structure corresponding to clname and add pid to its
 * pidlist.
 */
static void
add_pid_tolist(clpids, nclass, clname, pid)
classpids_t	*clpids;
int		nclass;
char		*clname;
pid_t		pid;
{
	classpids_t	*clp;

	for (clp = clpids; clp != &clpids[nclass]; clp++) {
		if (strcmp(clp->clp_clname, clname) == 0) {
			(clp->clp_pidlist)[clp->clp_npids] = pid;
			clp->clp_npids++;
			return;
		}
	}
}


/*
 * Compare id strings for equality.  If idargv contains ids
 * (idargc > 0) compare idstr to each id in idargv, otherwise
 * just compare to curidstr.
 */
static boolean_t
idmatch(idstr, curidstr, idargc, idargv)
char	*idstr;
char	*curidstr;
int	idargc;
char	**idargv;
{
	int	i;

	if (idargc == 0) {
		if (strcmp(curidstr, idstr) == 0)
			return(B_TRUE);
	} else {
		for (i = 0; i < idargc; i++) {
			if (strcmp(idargv[i], idstr) == 0)
				return(B_TRUE);
		}
	}
	return(B_FALSE);
}
