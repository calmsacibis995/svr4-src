/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/pcnfsd/pcnfsd.c	1.1.3.1"
#ifdef sccs
static char     sccsid[] = "@(#)rpc.pcnfsd.c	9.2 6/18/90";

#endif

/* #define SUNOS 1 */
#define SHADOW_SUPPORT 1
/* #define INTERACTIVE_2POINT0 1 */
/* #define USER_CACHE 1 */
#define SYS5 1


/*
 * Copyright (c) 1986, 1987, 1988, 1989, 1990 by Sun Microsystems, Inc.
 *
 * The extensive assistance, suggestions, bugfixes and support of NFS
 * developers in companies all over the world is gratefully acknowleged.
 * The list is too long to include here, but you know who you are.
 */
#include <sys/types.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <netconfig.h>
#include <netdir.h>
#include <memory.h>
#include <stropts.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <rpc/clnt_soc.h>	/* for UDPMSGSIZE */
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <assert.h>


#ifdef SHADOW_SUPPORT
#include <shadow.h>
#endif

#ifdef INTERACTIVE_2POINT0
#include <sys/fcntl.h>
#include <shadow.h>
#endif

#ifdef USER_CACHE
#include <string.h>
#endif

extern char *crypt();
extern int errno;
int        buggit = 0;
int        use_lpr = 0;

int	_rpcsvcdirty;		/* Still serving? */

/*
 * *************** RPC parameters ********************
 */
#define	PCNFSDPROG	(long)150001
#define	PCNFSDVERS	(long)1
#define	PCNFSD_AUTH	(long)1
#define	PCNFSD_PR_INIT	(long)2
#define	PCNFSD_PR_START	(long)3

/*
 * ************* Other #define's **********************
 */
#ifndef SPOOLDIR
#define SPOOLDIR	"/usr/spool/pcnfs"
#endif	/* SPOOLDIR */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#define	zchar		0x5b

#define _RPCSVC_CLOSEDOWN 120

#ifndef assert
#define assert(ex) {if (!(ex)) \
    {(void)fprintf(stderr,"pcnfsd: Assertion failed: line %d of %s: \"%s\"\n", \
    __LINE__, __FILE__, "ex"); \
    exit(1);}}
#endif

/*
 * *********** XDR structures, etc. ********************
 */
enum arstat {
	AUTH_RES_OK, AUTH_RES_FAKE, AUTH_RES_FAIL
};
enum pirstat {
	PI_RES_OK, PI_RES_NO_SUCH_PRINTER, PI_RES_FAIL
};
enum psrstat {
	PS_RES_OK, PS_RES_ALREADY, PS_RES_NULL, PS_RES_NO_FILE,
	PS_RES_FAIL
};

struct auth_args {
	char           *aa_ident;
	char           *aa_password;
};

struct auth_results {
	enum arstat     ar_stat;
	long            ar_uid;
	long            ar_gid;
};

struct pr_init_args {
	char           *pia_client;
	char           *pia_printername;
};

struct pr_init_results {
	enum pirstat    pir_stat;
	char           *pir_spooldir;
};

struct pr_start_args {
	char           *psa_client;
	char           *psa_printername;
	char           *psa_username;
	char           *psa_filename;	/* within the spooldir */
	char           *psa_options;
};

struct pr_start_results {
	enum psrstat    psr_stat;
};

#ifdef USER_CACHE
#define CACHE_SIZE 16		/* keep it small, as linear searches are
				 * done */
struct cache {
	int             uid;
	int             gid;
	char            passwd[32];
	char            username[10];	/* keep this even for machines
					 * with alignment problems */
}               User_cache[CACHE_SIZE];

#endif

/*
 * ****************** Misc. ************************
 */

char           *authproc();
char           *pr_start();
char           *pr_init();
struct stat     statbuf;

char            pathname[MAXPATHLEN];
char            new_pathname[MAXPATHLEN];
char            spoolname[MAXPATHLEN];

void		closedown();

#ifdef SYS5
struct passwd  *getpwnam();

#endif


/*
 * ************** Support procedures ***********************
 */
scramble(s1, s2)
	char           *s1;
	char           *s2;
{
	while (*s1) {
		*s2++ = (*s1 ^ zchar) & 0x7f;
		s1++;
	}
	*s2 = 0;
}

void
free_child()
{
	int             pid;
	int		status;

	while((pid = wait(&status)) != -1) {

		if (buggit || WEXITSTATUS(status))
		(void)fprintf(stderr, "FREE_CHILD: process #%d exited with status 0X%x\n",
			pid, WEXITSTATUS(status));
	}
	return;
}

/*
 * *************** XDR procedures *****************
 */
bool_t
xdr_auth_args(xdrs, aap)
	XDR            *xdrs;
	struct auth_args *aap;
{
	return (xdr_string(xdrs, &aap->aa_ident, 32) &&
		xdr_string(xdrs, &aap->aa_password, 64));
}

bool_t
xdr_auth_results(xdrs, arp)
	XDR            *xdrs;
	struct auth_results *arp;
{
	return (xdr_enum(xdrs, (enum_t *)&arp->ar_stat) &&
		xdr_long(xdrs, &arp->ar_uid) &&
		xdr_long(xdrs, &arp->ar_gid));
}

bool_t
xdr_pr_init_args(xdrs, aap)
	XDR            *xdrs;
	struct pr_init_args *aap;
{
	return (xdr_string(xdrs, &aap->pia_client, 64) &&
		xdr_string(xdrs, &aap->pia_printername, 64));
}

bool_t
xdr_pr_init_results(xdrs, arp)
	XDR            *xdrs;
	struct pr_init_results *arp;
{
	return (xdr_enum(xdrs, (enum_t *)&arp->pir_stat) &&
		xdr_string(xdrs, &arp->pir_spooldir, 255));
}

bool_t
xdr_pr_start_args(xdrs, aap)
	XDR            *xdrs;
	struct pr_start_args *aap;
{
	return (xdr_string(xdrs, &aap->psa_client, 64) &&
		xdr_string(xdrs, &aap->psa_printername, 64) &&
		xdr_string(xdrs, &aap->psa_username, 64) &&
		xdr_string(xdrs, &aap->psa_filename, 64) &&
		xdr_string(xdrs, &aap->psa_options, 64));
}

bool_t
xdr_pr_start_results(xdrs, arp)
	XDR            *xdrs;
	struct pr_start_results *arp;
{
	return (xdr_enum(xdrs, (enum_t *)&arp->psr_stat));
}




/*
 * ********************** main *********************
 */

main(Argc, Argv)
	int             Argc;
	char           *Argv[];
{
	int             f1, f2, f3;
	int             FromInetd = 0;
	int             fd;
	SVCXPRT        *TransportHandle;
	void            Dispatch();
	char		mname[FMNAMESZ + 1];
	struct netconfig	*nconf = NULL;

	(void)strcpy(spoolname, SPOOLDIR);

	setbuf(stderr, (char *)NULL);

	if (geteuid() != 0) {
		(void)fprintf(stderr, "rpc.pcnfsd must be run by 'root'\n");
		exit(1);
	}
	/*
	 * If we're called from inetd: - an open RPC socket is passed as
	 * fd 0. and note that we are already registered with the
	 * portmapper. Otherwise: - we must parse any command-line
	 * arguments which may be present. - we must create an RPC socket
	 * (svcudp_create will do this). - we are not yet registered with
	 * the portmapper, and must do so.
	 */
	/*
	 * This is completely different now. For SVR4 we can be called from the
	 * listener, from inetd, or from the shell. If the top module is timod
	 * or sockmod we were called from a port monitor. Otherwise we were
	 * called from the shell.
	 */
	if (!ioctl(0, I_LOOK, mname) &&
		(!strcmp(mname, "sockmod") || !strcmp(mname, "timod"))) {
		char *netid;
		int pmclose;
		extern char *getenv();

		FromInetd = 1;
		if ((netid = getenv("NLSPROVIDER")) == NULL) {
			fprintf(stderr, "cannot get transport name");
		} else if ((nconf = getnetconfigent(netid)) == NULL) {
			fprintf(stderr, "cannot get transport info");
		}
		/*
		 * If called from inetd we're given a socket. Turn it into a
		 * TLI file descriptor for RPC.
		 */
		if (strcmp(mname, "sockmod") == 0) {
			if (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, "timod")) {
				fprintf(stderr, "could not get the right module");
				exit(1);
			}
		}
		pmclose = (t_getstate(0) != T_DATAXFER);
		if ((TransportHandle = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL) {
			fprintf(stderr, "cannot create server handle");
			exit(1);
		}
		if (nconf)
			freenetconfigent(nconf);
		if (!svc_reg(TransportHandle, PCNFSDPROG, PCNFSDVERS, Dispatch, 0)) {
			fprintf(stderr, "unable to register (PCNFSDPROG, PCNFSDVERS).");
			exit(1);
		}
		if (pmclose) {
			(void) signal(SIGALRM, closedown);
			(void) alarm(_RPCSVC_CLOSEDOWN);
		}
	} else {
		struct t_bind *baddr;
		/*
		 * Set up our RPC environment: not inetd/listener
		 */
		if ((nconf = getnetconfigent("udp")) == NULL) {
			fprintf(stderr,"pcnfsd: Assertion failed: line %d of %s: \"%s\"\n",
				__LINE__, __FILE__,
				"(nconf = getnetconfigent(\"udp\")) != NULL");
		}
		assert((fd = t_open(nconf->nc_device, O_RDWR, NULL)) >= 0);
		assert((baddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ALL)) != NULL);
		assert(netdir_options(nconf, ND_SET_RESERVEDPORT, fd, NULL) >= 0);
		assert(t_getname(fd, &baddr->addr, LOCALNAME) >= 0);
		assert((TransportHandle = svc_tli_create(fd, nconf, &baddr->addr, 0, 0)) != (SVCXPRT *)NULL);
		svc_unreg(PCNFSDPROG, PCNFSDVERS);
		assert(svc_reg(TransportHandle, PCNFSDPROG, PCNFSDVERS, Dispatch, nconf) != 0);
		freenetconfigent(nconf);
		t_free(baddr, T_BIND);
	}

	if (!FromInetd) {
		while (++Argv, --Argc > 0) {
			if (strcmp(*Argv, "-d") == 0) {
				++buggit;
				continue;
			}
			if (strcmp(*Argv, "-s") == 0) {
				if (!(++Argv, --Argc > 0)) {
					(void)fprintf(stderr,
						"pc-nfsd error: -s option must be followed by a spooling directory path\n");
					exit(1);
				}
				(void)strcpy(spoolname, *Argv);
				continue;
			}
			if (strncmp(*Argv, "-s", 2) == 0) {
				(void)strcpy(spoolname, &(*Argv)[2]);
				continue;
			}
			if (strcmp(*Argv, "-u") == 0) {
				use_lpr = 1;
				continue;
			}
		}
	}
	if (!FromInetd && !buggit) {
		switch (fork()) {
		case 0:
			break;
		case -1:
			perror("pc-nfsd: fork failed");
			exit(1);
		default:
			exit(0);
		}
	}
	if (!buggit) {
		/*
		 * Can't mess with STDIN if invoked from inetd, 'cause our
		 * incoming RPC request datagram is passed in on STDIN.
		 */
		if (!FromInetd) {
			if ((f1 = open("/dev/null", O_RDONLY)) == -1) {
				(void)fprintf(stderr, "pc-nfsd: couldn't open /dev/null\n");
				exit(1);
			}
			(void) dup2(f1, 0);
			(void) close(f1);
		}
		if ((f2 = open("/dev/console", O_WRONLY)) == -1) {
			(void)fprintf(stderr, "pc-nfsd: couldn't open /dev/console\n");
			exit(1);
		}
		(void) dup2(f2, 1);
		(void) close(f2);

		if ((f3 = open("/dev/console", O_WRONLY)) == -1) {
			(void)fprintf(stderr, "pc-nfsd: couldn't open /dev/console\n");
			exit(1);
		}
		(void) dup2(f3, 2);
		(void) close(f3);

		/*
		 * Disconnect ourself from the control tty:
		 */
		setsid();
	}

	(void)mkdir(spoolname, 0777);	/* just in case, ignoring the result */
	if (stat(spoolname, &statbuf) || !(statbuf.st_mode & S_IFDIR)) {
		(void)fprintf(stderr, "pc-nfsd: invalid spool directory %s\n", spoolname);
		exit(1);
	}
	svc_run();
	(void)fprintf(stderr, "pc-nfsd: error: svc_run returned\n");
	exit(1);
/*NOTREACHED*/

	return(0);
}


/*
 * ******************* RPC procedures **************
 */
void 
Dispatch(ServiceRequest, Transport)
	struct svc_req *ServiceRequest;
	SVCXPRT        *Transport;
{
	char           *outdata;
 	union {
 		struct auth_args xdrb_aa;
 		struct pr_init_args xdrb_pia;
 		struct pr_start_args xdrb_psa;
 		char	xdrb_buf[UDPMSGSIZE];
 	} xdrbuf;


	_rpcsvcdirty = 1;
	memset(xdrbuf.xdrb_buf, 0, sizeof(xdrbuf)); /* said to be required... */

	switch (ServiceRequest->rq_proc) {
	case 0:
		assert(svc_sendreply(Transport, xdr_void, (caddr_t)NULL));
		_rpcsvcdirty = 0;
		break;
	case PCNFSD_AUTH:
		assert(svc_getargs(Transport, xdr_auth_args, xdrbuf.xdrb_buf));
		outdata = authproc(&xdrbuf.xdrb_aa);
		assert(svc_sendreply(Transport, xdr_auth_results, outdata));
		break;
	case PCNFSD_PR_INIT:
		assert(svc_getargs(Transport, xdr_pr_init_args, xdrbuf.xdrb_buf));
		outdata = pr_init(&xdrbuf.xdrb_pia);
		assert(svc_sendreply(Transport, xdr_pr_init_results, outdata));
		break;
	case PCNFSD_PR_START:
		assert(svc_getargs(Transport, xdr_pr_start_args, xdrbuf.xdrb_buf));
		outdata = pr_start(&xdrbuf.xdrb_psa);
		assert(svc_sendreply(Transport, xdr_pr_start_results, outdata));
		break;
	default:
		(void)fprintf(stderr,
			"pc-nfsd error: unknown function %d called in Dispatch()\n",
			ServiceRequest->rq_proc);
		svcerr_noproc(Transport);
		_rpcsvcdirty = 0;
		break;
	}
	_rpcsvcdirty = 0;
	return;
}


struct passwd  *
get_password(username)
	char           *username;
{
	struct passwd  *p;
	static struct passwd localp;
	char           *pswd;

#ifdef SHADOW_SUPPORT
	struct spwd    *sp;
	int             shadowfile;

#endif

#ifdef SHADOW_SUPPORT
	/*
	 * Check the existence of SHADOW.  If it is there, then we are
	 * running a two-password-file system.
	 */
	if (access(SHADOW, 0))
		shadowfile = 0;	/* SHADOW is not there */
	else
		shadowfile = 1;

	setpwent();
	if (shadowfile)
		(void) setspent();	/* Setting the shadow password
					 * file */
	if ((p = getpwnam(username)) == (struct passwd *)NULL ||
		(shadowfile && (sp = getspnam(username)) == (struct spwd *)NULL))
		return ((struct passwd *)NULL);

	if (shadowfile) {
		pswd = sp->sp_pwdp;
		(void) endspent();
	} else
		pswd = p->pw_passwd;


#else
	p = getpwnam(username);
	if (p == (struct passwd *)NULL)
		return ((struct passwd *)NULL);
	pswd = p->pw_passwd;
#endif

#ifdef INTERACTIVE_2POINT0
	/* We may have an 'x' in which case look in /etc/shadow .. */
	if (((strlen(pswd)) == 1) && pswd[0] == 'x') {
		struct spwd    *shadow = getspnam(username);

		if (!shadow)
			return ((struct passwd *)NULL);
		pswd = shadow->sp_pwdp;
	}
#endif
	localp = *p;
	localp.pw_passwd = pswd;
	return (&localp);
}


char           *
authproc(a)
	struct auth_args *a;
{
	static struct auth_results r;
	char            username[32];
	char            password[64];
	int             c1, c2;
	struct passwd  *p;

#ifdef SHADOW_SUPPORT
	struct spwd    *sp;
	int             shadowfile;

#endif
#ifdef USER_CACHE
	int             cache_entry;;
#endif

	r.ar_stat = AUTH_RES_FAIL;	/* assume failure */
	r.ar_uid = -2;
	r.ar_gid = -2;

	scramble(a->aa_ident, username);
	scramble(a->aa_password, password);

	if (buggit)
		(void)fprintf(stderr, "AUTHPROC username=%s\n", username);

#ifdef USER_CACHE
	cache_entry = check_cache(username);
	if (cache_entry >= 0) {
		if (buggit)
			(void)fprintf(stderr, "...cache hit?\n");
		c1 = strlen(password);
		c2 = strlen(User_cache[cache_entry].passwd);
		if ((!c1 && !c2) ||
			!(strcmp(User_cache[cache_entry].passwd,
					crypt(password, User_cache[cache_entry].passwd)))) {
			if (buggit)
				(void)fprintf(stderr, "...cache hit\n");
			r.ar_stat = AUTH_RES_OK;
			r.ar_uid = User_cache[cache_entry].uid;
			r.ar_gid = User_cache[cache_entry].gid;
			return ((char *) &r);
		}
		User_cache[cache_entry].username[0] = '\0';	/* nuke entry */
	}
	if (buggit)
		(void)fprintf(stderr, "...cache miss\n");
#endif
	p = get_password(username);
	if (p == (struct passwd *)NULL)
		return ((char *) &r);

	c1 = strlen(password);
	c2 = strlen(p->pw_passwd);
	if ((c1 && !c2) || (c2 && !c1) ||
		(strcmp(p->pw_passwd, crypt(password, p->pw_passwd)))) {
		return ((char *) &r);
	}
	r.ar_stat = AUTH_RES_OK;
	r.ar_uid = p->pw_uid;
	r.ar_gid = p->pw_gid;
#ifdef USER_CACHE
	add_cache_entry(p);
#endif
	return ((char *) &r);
}


char           *
pr_init(pi_arg)
	struct pr_init_args *pi_arg;
{
	int             dir_mode = 0777;
	static struct pr_init_results pi_res;

	mkdir(spoolname, dir_mode);	/* just in case, ignoring the result */
	chmod(spoolname, dir_mode);

	/* get pathname of current directory and return to client */
	(void)strcpy(pathname, spoolname);	/* first the spool area */
	(void)strcat(pathname, "/");	/* append a slash */
	(void)strcat(pathname, pi_arg->pia_client);
	/* now the host name */
	mkdir(pathname, dir_mode);	/* ignore the return code */
	if (stat(pathname, &statbuf) || !(statbuf.st_mode & S_IFDIR)) {
		(void)fprintf(stderr,
			"pc-nfsd: unable to create spool directory %s\n",
			pathname);
		pathname[0] = 0;/* null to tell client bad vibes */
		pi_res.pir_stat = PI_RES_FAIL;
	} else {
		pi_res.pir_stat = PI_RES_OK;
	}
	pi_res.pir_spooldir = &pathname[0];
	chmod(pathname, dir_mode);

	if (buggit)
		(void)fprintf(stderr, "PR_INIT pathname=%s\n", pathname);

	return ((char *) &pi_res);
}

char           *
pr_start(ps_arg)
	struct pr_start_args *ps_arg;
{
	static struct pr_start_results ps_res;
	int             pid;
	char            printer_opt[64];
	char            jobid_opt[64];
	char            title_opt[128];
	char            clientname_opt[64];
	char            snum[20];
	char            scratch[512];


	signal(SIGCHLD, free_child);	/* when child terminates it sends */
	/* a signal which we must get */
	(void)strcpy(pathname, spoolname);	/* build filename */
	(void)strcat(pathname, "/");
	(void)strcat(pathname, ps_arg->psa_client);	/* /spool/host */
	(void)strcat(pathname, "/");	/* /spool/host/ */
	(void)strcat(pathname, ps_arg->psa_filename);	/* /spool/host/file */

	if (buggit) {
		(void)fprintf(stderr, "PR_START pathname=%s\n", pathname);
		(void)fprintf(stderr, "PR_START username= %s\n", ps_arg->psa_username);
		(void)fprintf(stderr, "PR_START client= %s\n", ps_arg->psa_client);
	}
	if (stat(pathname, &statbuf)) {
		/*
		 * We can't stat the file. Let's try appending '.spl' and
		 * see if it's already in progress.
		 */

		if (buggit)
			(void)fprintf(stderr, "...can't stat it.\n");

		(void)strcat(pathname, ".spl");
		if (stat(pathname, &statbuf)) {
			/*
			 * It really doesn't exist.
			 */

			if (buggit)
				(void)fprintf(stderr, "...PR_START returns PS_RES_NO_FILE\n");

			ps_res.psr_stat = PS_RES_NO_FILE;
			return ((char *) &ps_res);
		}
		/*
		 * It is already on the way.
		 */

		if (buggit)
			(void)fprintf(stderr, "...PR_START returns PS_RES_ALREADY\n");

		ps_res.psr_stat = PS_RES_ALREADY;
		return ((char *) &ps_res);
	}
	if (statbuf.st_size == 0) {
		/*
		 * Null file - don't print it, just kill it.
		 */
		unlink(pathname);

		if (buggit)
			(void)fprintf(stderr, "...PR_START returns PS_RES_NULL\n");

		ps_res.psr_stat = PS_RES_NULL;
		return ((char *) &ps_res);
	}
	/*
	 * The file is real, has some data, and is not already going out.
	 * We rename it by appending '.spl' and exec "lpr" to do the
	 * actual work.
	 */
	(void)strcpy(new_pathname, pathname);
	(void)strcat(new_pathname, ".spl");

	if (buggit)
		(void)fprintf(stderr, "...renaming %s -> %s\n", pathname, new_pathname);

	/*
	 * See if the new filename exists so as not to overwrite it.
	 */


	if (!stat(new_pathname, &statbuf)) {
		(void)strcpy(new_pathname, pathname);	/* rebuild a new name */
		(void)sprintf(snum, "%d", rand());	/* get some number */
		(void)strncat(new_pathname, snum, 3);
		(void)strcat(new_pathname, ".spl");	/* new spool file */
		if (buggit)
			(void)fprintf(stderr, "...created new spl file -> %s\n", new_pathname);

	}
	if (rename(pathname, new_pathname)) {
		/*
		 * CAVEAT: Microsoft changed rename for Microsoft C V3.0.
		 * Check this if porting to Xenix.
		 */
		/*
		 * Should never happen.
		 */
		(void)fprintf(stderr, "pc-nfsd: spool file rename (%s->%s) failed.\n",
			pathname, new_pathname);
		ps_res.psr_stat = PS_RES_FAIL;
		return ((char *) &ps_res);
	}
	pid = fork();
	if (pid == 0) {
		/*
		 * FLUKE jps 28-jul-86 - Invoke lpr as the requesting
		 * user.
		 * 
		 * If possible, invoke lpr under the user-id/group-id of the
		 * person (apparently) making this RPC request.  Good for
		 * accounting, proper banner page, etc.  It is not
		 * mandatory.
		 */
		struct passwd  *pw = getpwnam(ps_arg->psa_username);

		if (buggit)
			(void)fprintf(stderr, "username is %s\n", ps_arg->psa_username);
		if (pw) {
			if (buggit)
				(void)fprintf(stderr, "uid is %d\ngid is %d\n",
					pw->pw_uid, pw->pw_gid);
			setuid(pw->pw_uid);
			setgid(pw->pw_gid);
		}
		/*
		 *  Since true filename is not available, print the user and
		 *  system name as title.
		 */
		if (use_lpr) {
			sprintf(printer_opt, "-P%s", ps_arg->psa_printername);
			sprintf(title_opt, "-JNot available - Request from %s@%s",
					ps_arg->psa_username, ps_arg->psa_client);
			sprintf(clientname_opt, "-C%s", ps_arg->psa_client);
		} else {
			sprintf(printer_opt, "-d%s", ps_arg->psa_printername);
			sprintf(title_opt, "-t'Not available - Request from %s@%s'",
					ps_arg->psa_username, ps_arg->psa_client);
		}
#ifdef SUNOS
#ifdef HACK_FOR_ROTATED_TRANSCRIPT
		if (!strcmp(ps_arg->psa_printername, "rotated")) {
			sprintf(scratch, "enscript -lrq -fCourier7  %s",
				new_pathname);
			if (buggit)
				(void)fprintf(stderr, "system(%s)\n", scratch);
			system(scratch);
			unlink(new_pathname);
			exit(0);
		}
		if (!strcmp(ps_arg->psa_printername, "2column")) {
			sprintf(scratch, "enscript -2rqG -J\"PC-NFS spool file\" %s",
				new_pathname);
			if (buggit)
				(void)fprintf(stderr, "system(%s)\n", scratch);
			system(scratch);
			unlink(new_pathname);
			exit(0);
		}
#endif /* HACK_FOR_ROTATED_TRANSCRIPT */

		if (ps_arg->psa_options[1] == 'd') {
			/*
			 * This is a Diablo print stream. Apply the ps630
			 * filter with the appropriate arguments.
			 */
			if (buggit)
				(void)fprintf(stderr, "...run_ps630 invoked\n");
			(void)run_ps630(new_pathname, ps_arg->psa_options);
		}
#endif /* SUNOS */
		/*
		 * New - let's close up 0, 1 and 2 to force getlogin to
		 * fail in lpr
		 */
		close(0);
		close(1);
		close(2);

		if (use_lpr) {
			execlp("/usr/ucb/lpr",
				"lpr",
				"-s",
				"-r",
				printer_opt,
				title_opt,
				clientname_opt,
				new_pathname,
				0);
			perror("pc-nfsd: exec lpr failed");
			exit(1);	/* end of child process */
		} else {
			/*
			 *  When the lp command to finished, the file copy is complete
			 *  and we can remove the file.
			 */
			sprintf(scratch, "/bin/lp -c %s %s %s", printer_opt,
											title_opt, new_pathname);
			if (buggit)
				(void)fprintf(stderr, "system(%s)\n", scratch);
			if (system(scratch) == -1) {
				unlink(new_pathname);
				(void)fprintf(stderr, "spool request %s FAILED", scratch);
				exit(1);
			} else {
				unlink(new_pathname);
				exit(0);
			}
		}
	} else if (pid == -1) {
		perror("pc-nfsd: fork failed");

		if (buggit)
			(void)fprintf(stderr, "...PR_START returns PS_RES_FAIL\n");

		ps_res.psr_stat = PS_RES_FAIL;
	} else {

		if (buggit)
			(void)fprintf(stderr, "...forked child #%d\n", pid);


		if (buggit)
			(void)fprintf(stderr, "...PR_START returns PS_RES_OK\n");

		ps_res.psr_stat = PS_RES_OK;
	}
	return ((char *) &ps_res);
}


char           *
mapfont(f, i, b)
	char            f;
	char            i;
	char            b;
{
	static char     fontname[64];

	fontname[0] = 0;	/* clear it out */

	switch (f) {
	case 'c':
		(void)strcpy(fontname, "Courier");
		break;
	case 'h':
		(void)strcpy(fontname, "Helvetica");
		break;
	case 't':
		(void)strcpy(fontname, "Times");
		break;
	default:
		(void)strcpy(fontname, "Times-Roman");
		goto finis ;
	}
	if (i != 'o' && b != 'b') {	/* no bold or oblique */
		if (f == 't')	/* special case Times */
			(void)strcat(fontname, "-Roman");
		goto finis;
	}
	(void)strcat(fontname, "-");
	if (b == 'b')
		(void)strcat(fontname, "Bold");
	if (i == 'o')		/* o-blique */
		(void)strcat(fontname, f == 't' ? "Italic" : "Oblique");

finis:	return (&fontname[0]);
}

/*
 * run_ps630 performs the Diablo 630 emulation filtering process. ps630
 * was broken in certain Sun releases: it would not accept point size or
 * font changes. If your version is fixed, undefine the symbol
 * PS630_IS_BROKEN and rebuild pc-nfsd.
 */
/* #define PS630_IS_BROKEN 1 */

run_ps630(file, options)
	char           *file;
	char           *options;
{
	char            temp_file[256];
	char            commbuf[256];
	int             i;

	(void)strcpy(temp_file, file);
	(void)strcat(temp_file, "X");	/* intermediate file name */

#ifndef PS630_IS_BROKEN
	(void)sprintf(commbuf, "ps630 -s %c%c -p %s -f ",
		options[2], options[3], temp_file);
	(void)strcat(commbuf, mapfont(options[4], options[5], options[6]));
	(void)strcat(commbuf, " -F ");
	(void)strcat(commbuf, mapfont(options[7], options[8], options[9]));
	(void)strcat(commbuf, "  ");
	(void)strcat(commbuf, file);
#else	/* PS630_IS_BROKEN */
	/*
	 * The pitch and font features of ps630 appear to be broken at
	 * this time.
	 */
	sprintf(commbuf, "ps630 -p %s %s", temp_file, file);
#endif	/* PS630_IS_BROKEN */


	if (i = system(commbuf)) {
		/*
		 * Under (un)certain conditions, ps630 may return -1 even
		 * if it worked. Hence the commenting out of this error
		 * report.
		 */
		 /* (void)fprintf(stderr, "\n\nrun_ps630 rc = %d\n", i) */ ;
		/* exit(1); */
	}
	if (rename(temp_file, file)) {
		perror("run_ps630: rename");
		exit(1);
	}
	return(i); /* never used, but keeps lint happy */
}

/*
 * Determine if a descriptor belongs to a socket or not
 */
issock(fd)
	int             fd;
{
	struct stat     st;

	if (fstat(fd, &st) < 0) {
		return (0);
	}
	/*
	 * SunOS returns S_IFIFO for sockets, while 4.3 returns 0 and does
	 * not even have an S_IFIFO mode.  Since there is confusion about
	 * what the mode is, we check for what it is not instead of what
	 * it is.
	 */
	switch (st.st_mode & S_IFMT) {
	case S_IFCHR:
	case S_IFREG:
	case S_IFLNK:
	case S_IFDIR:
	case S_IFBLK:
		return (0);
	default:
		return (1);
	}
}



#ifdef USER_CACHE
int
check_cache(name)
	char           *name;
{
	int             i;

	for (i = 0; i < CACHE_SIZE; i++) {
		if (!strcmp(User_cache[i].username, name))
			return (i);
	}
	return (-1);
}

add_cache_entry(p)
	struct passwd  *p;
{
	int             i;

	for (i = CACHE_SIZE - 1; i > 0; i--)
		User_cache[i] = User_cache[i - 1];
	User_cache[0].uid = p->pw_uid;
	User_cache[0].gid = p->pw_gid;
	(void)strcpy(User_cache[0].passwd, p->pw_passwd);
	(void)strcpy(User_cache[0].username, p->pw_name);
}


#endif				/* USER_CACHE */

void
closedown()
{
	if (_rpcsvcdirty == 0) {
		extern fd_set svc_fdset;
		static struct rlimit rl;
		int i, openfd;
		struct t_info tinfo;

		if (t_getinfo(0, tinfo) || (tinfo.servtype = T_CLTS))
			exit(0);
		if (rl.rlim_max == 0)
			getrlimit(RLIMIT_NOFILE, &rl);
		for (i=0, openfd = 0; i < rl.rlim_max && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			exit(0);
	}
	(void) alarm(_RPCSVC_CLOSEDOWN);
}
