/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dustart:dustart.c	1.25.21.1"


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/utsname.h>
#include <nserve.h>
#include <time.h>
#include <sys/rf_sys.h>
#include <errno.h>
#include <ctype.h>

#define NSERVE  	"/usr/lib/rfs/nserve"
#define NETSPEC  	"/etc/rfs/netspec"

#define WAIT_TIME	600
#define MAX_ARGS	100
#define ERROR(str)	fprintf(stderr,"%s: %s\n", cmd_name, str)

static	char   *cmd_name;
static	int	ok_flag = 0;
static	pid_t	ns_process;
extern	char   *Bypass;

extern	int	errno;

main( argc, argv )
int   argc;
char *argv[];
{
	char   *n_argv[MAX_ARGS];

	int	error = 0, pflag = 0, vflag = 0, cflag = 0;
	pid_t	parent;
	int	i, c, rec;
	int	indx;
	int	cr_pass = 0;
	void	ok_rtn(), error_rtn(), intrp_rtn();
	char    newpass[20];
	char   *rtn;
	char   *mach_name;
	char   *netspec;
	char   *tp;
	char   *dname;
	char   *getdname();
	char   *getnetspec();
	char   *getpass();
	char   *ns_getpass();
	char   *ns_verify();
	extern char *strtok();

	struct utsname utname;

	extern int optind;

	cmd_name = argv[0];

	/*
	 *	Process arguments.
	 *	Dustart will exit with one of three exit codes:
	 *		0 - success
	 *		1 - error because name server failed - try
	 *			again later.
	 *		2 - error because something else failed.
	 */

	while ((c = getopt(argc, argv, "vp:l:f:c")) != EOF) {
		switch (c) {
			case 'v':
				if (vflag)
					error = 1;
				else
					vflag = 1;
				break;
			case 'p':
				if (pflag)
					error = 1;
				else
					pflag = 1;
				break;
			case 'c':
				if (cflag)
					error = 1;
				else
					cflag = 1;
				break;
			case '?':
				error = 1;
		}
	}

	if (optind < argc) {
		ERROR("extra arguments given");
		error = 1;
	}

	if (error) {
		ERROR("usage: rfstart [-v] [-p primary_name_server_address]");
		exit(2);
	}

	if (geteuid() != 0) {
		ERROR("must be super-user");
		exit(2);
	}

	chdir("/");

	/*
	 *	Get the network specification to send to the name server.
	 */

	if ((netspec = getnetspec()) == NULL) {
		ERROR("network specification not set");
		exit(2);
	}

	/*
	 *	Set up the argument list of the name server.  The
	 *	"-k" option is always given to the name server (the
	 *	option tells the name server to signal the parent
	 *	when it is set up correctly).
	 *	The "-c" option is not recognized by the name server.
	 */

	n_argv[0] = NSERVE;
	n_argv[1] = "-k";
	indx = 2;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0)
			continue;
		n_argv[indx++] = argv[i];
	}
	n_argv[indx++] = netspec;
	n_argv[indx++] = NULL;;

	/*
	 *	Get the machine name of the host.
	 */

	if (uname(&utname) == -1) {
		ERROR("cannot get machine name");
		exit(2);
	}

	mach_name = utname.nodename;

	/*
	 *	Check to make sure the node name is valid.
	 */

	if (pv_uname(cmd_name, mach_name, 0, "node") != 0) {
		exit(1);
	}

	/*
	 *	Get the domain name and pass it into the kernel.
	 */

	if ((dname = getdname()) == NULL) {
		ERROR("domain name information not set");
		exit(2);
	}

	if (rfsys(RF_SETDNAME, dname, strlen(dname)+1) < 0) {
		if (errno == EEXIST)
			ERROR("RFS is already running");
		else
			perror(cmd_name);
		exit(2);
	}

	/*
	 *	Issue the rfstart(2) system call to initiate the
	 *	kernel functions of Distributed UNIX.
	 */

	if (rfsys(RF_START) < 0) {
		if (errno == EAGAIN) {
			ERROR("An RFS tunable parameter is set too low");
			ERROR("Consult Administrator's Guide");
		} else {
			perror(cmd_name);
		}
		exit(2);
	}

	/*
	 *	Catch the SIGUSR1 and SIGUSR2 signals from the
	 *	child processes which will be spawned.
	 *	SIGUSR1 will signify everything within the
	 *	child is OK and SIGUSR2 will signify that something
	 *	went wrong.
	 *	If any other interrupt, stop the name server and exit.
	 */

	sigset(SIGUSR1, ok_rtn);
	sigset(SIGUSR2, error_rtn);
	sigset(SIGHUP,  intrp_rtn);
	sigset(SIGINT,  intrp_rtn);
	sigset(SIGQUIT, intrp_rtn);

	/*
	 *	Clean up the /etc/dfs/sharetab file.
	 */

	share_clr();

	/*
	 *	Fork off a child which will eventually start off
	 *	the name server process.
	 */

	if ((parent = ns_process = fork()) == (pid_t)-1) {
		perror(cmd_name);
		ERROR("cannot fork to start name server");
		rfsys(RF_STOP);
		exit(2);
	}

	if (parent) {
		/*
		 *	The parent expects the name server to send
		 *	a SIGUSR1 signal or a SIGUSR2 signal.
		 *	If the parent sleeps for WAIT_TIME seconds and
		 *	does not get a signal (i.e., ok_flag is still
	 	 *	not set), it assumes something
		 *	went wrong and exits.
		 */

		if (!ok_flag) {
			sleep(WAIT_TIME);
			if (!ok_flag) {
				ERROR("timed out waiting for name server signal");
			rfsys(RF_STOP);
				killns();
				exit(2);
			}
		}
	} else {
		setpgrp();
		if (execv(NSERVE, n_argv) == -1) {
			perror(cmd_name);
			ERROR("cannot exec name server");
			kill(getppid(), SIGUSR2);
			exit(2);
		}
	}

	/*
	 *	Call ns_initaddr(), which sends the address of
	 *	this machine to the primary name server and tells the
	 *	primary name server to unadvertise any resources
	 *	currently advertised by this machine.
	 *	If the "-c" flag is specified, don't kill the name
	 *	server on failure, simply exit.
	 */

	if (ns_initaddr(mach_name) == FAILURE) {
		if (cflag) {
			ERROR("warning: could not contact primary name server");
			system("/usr/bin/setpgrp /usr/lib/rfs/rfudaemon &");
			exit(2);
		} else {
			ERROR("could not contact primary name server");
			nserror(cmd_name);
			rfsys(RF_STOP);
			killns();
			exit(1);
		}
	}

	/*
	 *	Get the password of this machine.
	 *	If no password exists for this machine, prompt for
	 *	password.  The password is verified by the primary
	 *	name server.
	 */

	while (tp = strtok(netspec, ",")) {
	char passfile[BUFSIZ];
	char pidfile[256];
	char path[BUFSIZ];
	struct stat sbuf, pbuf;
		netspec=NULL;
		sprintf (path, "/dev/%s", tp);
		if (stat(path, &sbuf) < 0) {
			fprintf(stderr, "rfstart: name server not started for %s\n", tp);
			continue;
		}

		sprintf (pidfile, TPNSPID, tp);
		if (stat(pidfile, &pbuf) < 0) {
			fprintf(stderr, "rfstart: name server not started for %s\n", tp);
			continue;
		}

		Bypass=tp;
		sprintf(passfile, PASSFILE, tp);
		if ((rec = open(passfile, O_RDONLY)) == -1) {
		char prompt[BUFSIZ];
			sprintf(prompt, "rfstart: Please enter machine password for %s:", tp);
			
			

#ifdef i386
			if ((rtn=getpass(prompt)) == 0)
				newpass[0] = '\0'; 
			else
				strncpy(newpass, rtn, sizeof(newpass));
#else
			strncpy(newpass, getpass(prompt), sizeof(newpass));
#endif

			cr_pass = 1;
		} else {
			int num = read(rec, newpass, sizeof(newpass) - 1);
			if (num > 0)
				newpass[num] = '\0';
			else
				newpass[0] = '\0';
			close(rec);
		}

		rtn = ns_verify(mach_name, newpass);
		if (rtn == (char *)NULL) {
			fprintf(stderr,"rfstart: warning: no entry for this host in domain passwd file on current %s name server\n",tp);
			cr_pass = 0;
		}
		else {
			if (strcmp(rtn, INCORRECT) == 0) {
				fprintf(stderr,"rfstart: warning: host password does not match registered password on current %s name server\n",tp);
				cr_pass = 0;
			}
		}

		if (cr_pass == 1) {
			if ((rec = creat(passfile, S_IRUSR|S_IWUSR)) < 0) { 
				fprintf(stderr, "rfstart: warning: cannot create password file for %s\n",tp);
			} else if (write(rec, newpass, strlen(newpass)) < 0) {
				fprintf(stderr,"rfstart: warning: cannot write password for %s\n",tp);
				close(rec);
				unlink(passfile);
			}
		}
	}

	/*
	 *	Start up a daemon that will wait for a messages from
	 *	other systems (for fumount, etc.).
	 */

	system("/usr/bin/setpgrp /usr/lib/rfs/rfudaemon &");

	exit(0);
}

static void
ok_rtn()
{
	/*
	 *	everyting went smoothly in the name server startup
	 */

	ok_flag = 1;
}

static void
error_rtn()
{
	rfsys(RF_STOP);
	exit(2);
}

static void
intrp_rtn()
{
	rfsys(RF_STOP);
	killns();
	exit(2);
}

static
killns()
{
	/*
	 *	kill the name server process.
	 */

	if (kill(ns_process, SIGTERM) < 0) {
		perror(cmd_name);
		ERROR("error in killing name server");
	}
}

static
char	*
getdname()
{
	static char dname[MAXDNAME];
	FILE	*fp;

	if ((rfsys(RF_GETDNAME, dname, MAXDNAME) < 0) || (dname[0] == '\0')) {
		if (((fp = fopen(NSDOM,"r")) == NULL)
		|| (fgets(dname,MAXDNAME,fp) == NULL))
			return(NULL);
		/*
		 *	get rid of trailing newline, if there
		 */
		if (dname[strlen(dname)-1] == '\n')
			dname[strlen(dname)-1] = '\0';
		fclose(fp);
	}
	return(dname);
}

static
char	*
getnetspec()
{
	static char netspec[BUFSIZ];
	FILE	*fp;

	if (((fp = fopen(NETSPEC,"r")) == NULL)
	|| (fgets(netspec,BUFSIZ,fp) == NULL))
		return(NULL);
	/*
	 *	get rid of trailing newline, if there
	 */
	if (netspec[strlen(netspec)-1] == '\n')
		netspec[strlen(netspec)-1] = '\0';

	fclose(fp);
	return(netspec);
}

static
share_clr()
{
	struct stat stbuf;
	FILE *fp, *fp1;
	char sbuf[BUFSIZ], sbuf_save[BUFSIZ];
	char *s, *fld;
	int f_count;

	if (stat("/etc/dfs/sharetab", &stbuf) < 0) 
		return;
	if ((fp = fopen("/etc/dfs/sharetab", "r")) == NULL)
		return;
	if ((fp1 = fopen("/etc/dfs/tmp.share", "w")) == NULL) 
		return;
	while (fgets (sbuf, BUFSIZ, fp)) {
		strcpy (sbuf_save, sbuf);
		s = sbuf;
		f_count = 0;
		while ((*s!='\n')&&(*s!='\0')&&(f_count<3)) {
			while (isspace(*s))
				s++;
			fld = s;
			while (!isspace(*s))
				s++;
			if (*s) 
				*s++ = '\0';
			f_count++;
			if (f_count == 3)
				if (strcmp(fld, "rfs"))
					fprintf(fp1, "%s", sbuf_save);
		}
	}
	fclose(fp);
	fclose(fp1);
	rename("/etc/dfs/tmp.share", "/etc/dfs/sharetab");
	chmod("/etc/dfs/sharetab", 00644);
	chown("/etc/dfs/sharetab", (int)stbuf.st_uid, (int)stbuf.st_gid);
}
