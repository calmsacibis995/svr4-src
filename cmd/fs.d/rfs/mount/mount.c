/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfs.cmds:mount/mount.c	1.23.3.1"



#include "sys/types.h"
#include "nserve.h"
#include "sys/stropts.h"
#include "sys/rf_cirmgr.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fs/rf_vfs.h"
#include "sys/rf_sys.h"
#include "sys/list.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "signal.h"
#include "stdio.h"
#include "fcntl.h"
#include "sys/mnttab.h"
#include "unistd.h"  /*  defines F_TLOCK  */
#include "errno.h"
#include "pn.h"
#include "sys/mount.h"
#include "sys/conf.h"
#include "netconfig.h"

#define MS_RFFLAGS	(MS_CACHE)
#define NETSPEC		"/etc/rfs/netspec"
#define	FSTYPE		"rfs"

#define	TIME_MAX	16
#define	NAME_MAX	64

#define	RO_BIT		1

#define	RDONLY	0
#define	RDWRITE	1
#define O_NOCACHING 2
#define O_SUID 3
#define O_NOSUID 4

extern int	errno;
extern int	ns_errno;
extern int	optind;
extern char	*optarg;

extern char *strtok(), *strrchr();
extern char *findaddr();
extern char  *getnetspec();
extern struct address *ns_getaddr(), *ns_getaddrs();
extern time_t	time();
extern char *Bypass;
extern struct netconfig *getnetpath();
extern int endnetpath();


char	*n_name = NULL;
char	typename[NAME_MAX], *myname;
char	fstype[] = FSTYPE;
char	mnttab[] = MNTTAB;
struct netconfig *np;
char	*myopts[] = {
	"ro",
	"rw",
	"nocaching",
	"suid",
	"nosuid",
	NULL
};

main(argc, argv)
	int	argc;
	char	**argv;
{
	FILE	*fwp;
	char	*special, *mountp;
	char	*options, *value;
	char	tbuf[TIME_MAX];
	int	errflag = 0;
	int	cc, rwflag = 0, mntflags = MS_CACHE;
	struct mnttab	mm;
	static char	mmoptlist[128] = "";

	myname = strrchr(argv[0], '/');
	if (myname)
		myname++;
	else
		myname = argv[0];
	sprintf(typename, "%s %s", fstype, myname);
	argv[0] = typename;

	/* check for proper arguments */
	while ((cc = getopt(argc, argv, "?cn:o:r")) != -1)
		switch (cc) {
		case 'r':
			if (mntflags & RO_BIT)
				errflag = 1;
			else
				mntflags |= RO_BIT;
			break;
		case 'o':
			options = optarg;
			while (*options != '\0')
				switch (getsubopt(&options, myopts, &value)) {
				case RDONLY:
					if ((mntflags & RO_BIT) || rwflag)
						errflag = 1;
					else
						mntflags |= RO_BIT;
					break;
				case RDWRITE:
					if ((mntflags & RO_BIT) || rwflag)
						errflag = 1;
					else
						rwflag = 1;
					break;
				case O_NOCACHING:
					mntflags &= ~MS_CACHE;
					break;
				case O_SUID:
					if (mntflags & MS_NOSUID)
						errflag = 1;
					break;
				case O_NOSUID:
					mntflags |= MS_NOSUID;
					break;
				default:
					fprintf(stderr, "%s: illegal -o suboption -- %s\n", typename, value);
					errflag++;
					break;
				}
			break;
		case 'c':
			mntflags &= ~MS_CACHE;
			break;
		case 'n':
			/*
			 *	The -n option is an undocumented
			 *	option used to override the name
			 *	server.  It takes a machine name
			 *	(in the form of "netspec:dom.name")
			 *	as an argument.
			 */
			n_name = optarg;
			break;
		case '?':
			errflag = 1;
			break;
		}

	/*
	 *	There must be at least 2 more arguments, the
	 *	special file and the directory.
	 */

	if ( ((argc - optind) != 2) || (errflag) )
		usage();

	special = argv[optind];
	mountp = argv[optind + 1];

	if (getuid() != 0) {
		fprintf(stderr, "%s: not super user\n", myname);
		exit(2);
	}

	mm.mnt_special = special;
	mm.mnt_mountp = mountp;
	mm.mnt_fstype = fstype;
	mm.mnt_mntopts = mmoptlist;
	if (mntflags & RO_BIT)
		if (*mmoptlist)
			strcat(mmoptlist, ",ro");
		else
			strcpy(mmoptlist, "ro");
	else
		if (*mmoptlist)
			strcat(mmoptlist, ",rw");
		else
			strcpy(mmoptlist, "rw");
	if (!(mntflags & MS_CACHE))
		if (*mmoptlist)
			strcat(mmoptlist, ",nocaching");
		else
			strcpy(mmoptlist, "nocaching");
	if (mntflags & MS_NOSUID)
		if (*mmoptlist)
			strcat(mmoptlist, ",nosuid");
		else
			strcpy(mmoptlist, "nosuid");

	sprintf(tbuf, "%ld", time(0L));	/* assumes ld == long == time_t */
	mm.mnt_time = tbuf;

	/* Open /etc/mnttab read-write to allow locking the file */
	if ((fwp = fopen(mnttab, "a")) == NULL) {
		fprintf(stderr, "%s: cannot open mnttab\n", myname);
		exit(1);
	}

	/*
	 * Lock the file to prevent many updates at once.
	 * This may sleep for the lock to be freed.
	 * This is done to ensure integrity of the mnttab.
	 */
	if (lockf(fileno(fwp), F_LOCK, 0L) < 0) {
		fprintf(stderr, "%s: cannot lock mnttab\n", myname);
		perror(myname);
		exit(1);
	}

	/* end of file may have changed behind our backs */
	fseek(fwp, 0L, 2);

	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);

	/*
	 *	Perform the remote mount.
	 */

	do_mount(special, mountp, mntflags);

	putmntent(fwp, &mm);

	exit(0);
}

do_mount(special, mountp, rflag)
	char	*special;
	char	*mountp;
	int	rflag;
{
	struct address *addr;
	struct rf_mountdata dmdata;
	int rfrtn;
	extern ndata_t ndata;
	char *nspec;
	char *sep;
	extern char *strchr();
	char *env_net;
	char npath[BUFSIZ];
	char ns_save[BUFSIZ];
	char *env_tok, *ns_tok[255], *tmp_tok;
	char netpath[BUFSIZ];
	int i, j;
	void *handlep;
	extern char *getenv();

	/* set id to CLIENT; later getoken() will clear it. */

	dmdata.rfm_token.t_id = CLIENT;

	/* transfer dufst-specific flags into dmdata */
	dmdata.rfm_flags = rflag & MS_RFFLAGS;
	rflag &= ~MS_RFFLAGS;

	/*
	 *	get an address of the remote resource from the
	 *	name server (or from the given name if "-n" is
	 *	specified).
	 */

	if (n_name == NULL) {
		/* Determine if RFS is running */
		if ((rfrtn = rfsys(RF_RUNSTATE)) < 0) {
			perror(myname);
			exit(2);
		}
		if (rfrtn != RF_UP) {
			fprintf(stderr, "%s: RFS not running\n", myname);
			exit(2);
		}

		/* set up NETPATH */
		if ((nspec = getnetspec()) == NULL) {
			fprintf(stderr, "%s: cannot obtain network specification\n", myname);
			exit(2);
		}
		while ((sep = strchr(nspec, ',')) != NULL)
			*sep = ':';
		npath[0] = '\0';
		if ((env_net = getenv(NETPATH)) == NULL) 
			strcpy (npath, nspec);
		else {
			strcpy (ns_save, nspec);
			/* prune out superfluous net elements in NETPATH */	
			i = 0;
			while ((tmp_tok = strtok(nspec, ":")) != NULL) {
				nspec = NULL;
				ns_tok[i] = tmp_tok;
				i++;
			}	
			while ((env_tok = strtok(env_net, ":"))!= NULL) {
				env_net= NULL;
				for (j=0; j<i; j++) {
					if ((strcmp(env_tok, ns_tok[j]))==0) {
						if (npath[0])
							strcat(npath,":");
						strcat(npath, env_tok);
						break;
					}
				}
			}
			/* if there's no match between NETPATH elements and
			   netspec, exit */
			if (!npath[0]) {
				fprintf(stderr, "mount: bad netspecs specified in NETPATH\n");
				exit(2);
			}
		}
		(void)sprintf (netpath, "NETPATH=%s", npath);
		(void)putenv(netpath);

		if ((handlep = (void *)setnetpath()) != NULL) {
			addr = NULL;
			while ((np = getnetpath(handlep)) != NULL) {
				Bypass = np->nc_netid;
				if ((addr = ns_getaddr(special, rflag, dmdata.rfm_token.t_uname)) != (struct address *)NULL) 
				break;
			}
			if (!addr)
			{
				fprintf(stderr,"mount: %s not available\n", special);
				nserror("mount");
				exit(2);
			}
		}
		else
			if ((addr = ns_getaddrs(special, rflag, dmdata.rfm_token.t_uname)) == (struct address *)NULL) {
				fprintf(stderr, "%s: %s not available\n", myname, special);
				nserror(myname);
				exit(2);
			}
	} else {
		char *tbuf = findaddr(n_name);
		if ((tbuf == (char *)NULL) ||
		    (addr = (struct address *)astoa(tbuf, NULL)) == (struct address *)NULL) {
			fprintf(stderr, "%s: invalid address specified: <%s>\n", myname, n_name);
			exit(2);
		}
		strncpy(dmdata.rfm_token.t_uname, dompart(n_name), MAXDNAME);
	}

	/* tell system to mount device */
	if (mount(special, mountp, rflag | MS_DATA,
	    fstype, &dmdata, sizeof(dmdata))) {
		if (errno != ENOLINK) {
			rpterr(special, mountp);
			exit(2);
		}
		while (u_getckt(addr,&dmdata.rfm_token) < 0) {
			if (n_name)
			{
				fprintf(stderr,"mount: could not connect to remote machine\n");
				exit(2);
			}
			addr = NULL;
			if ((np=getnetpath(handlep)) != NULL) {
				Bypass=np->nc_netid;
				addr = ns_getaddr(special, rflag, dmdata.rfm_token.t_uname);
			}
			else
				addr = ns_getaddrs(NULL, 0, NULL);

			if (!addr)
			{
				fprintf(stderr,"mount: could not connect to remote machine\n");
				exit(2);
			}
		}
		endnetpath(handlep);

		/*
		 *  Perform user and group id mapping for the host.
		 *  NOTE: ndata.n_netname is set via negotiate() in u_getckt().
		 */
		uidmap(0, (char *)NULL, (char *)NULL, &ndata.n_netname[0], 0);
		uidmap(1, (char *)NULL, (char *)NULL, &ndata.n_netname[0], 0);

		if (mount(special, mountp, rflag | MS_DATA,
		    fstype, &dmdata, sizeof(dmdata))) {
			rpterr(special, mountp);
			exit(2);
		}
	}
}

rpterr(bs, mp)
	register char	*bs, *mp;
{
	switch (errno) {
	case EPERM:
		fprintf(stderr, "%s: not super user\n", myname);
		break;
	case ENXIO:
		fprintf(stderr, "%s: %s no such device\n", myname, bs);
		break;
	case ENOTDIR:
		fprintf(stderr,
			"%s: %s not a directory,\n\tor a component of %s is not a directory\n",
			myname, mp, bs);
		break;
	case ENOENT:
		fprintf(stderr, "%s: %s or %s, no such file or directory\n",
			myname, bs, mp);
		break;
	case EINVAL:
		fprintf(stderr, "%s: %s not a valid resource for this fstype.\n",
			myname, bs);
		break;
	case EBUSY:
		fprintf(stderr,
			"%s: %s is already mounted, %s is busy,\n\tor allowable number of mount points exceeded\n",
			myname, bs, mp);
		break;
	case ENOTBLK:
		fprintf(stderr, "%s: %s not a block device\n", myname, bs);
		break;
	case EROFS:
		fprintf(stderr, "%s: %s write-protected\n", myname, bs);
		break;
	case ENOSPC:
		fprintf(stderr,
			"%s: remote machine cannot accept any more mounts;\n\tNSRMOUNT on remote machine exceeded\n",
			myname);
		break;
	case EREMOTE:
		fprintf(stderr, "%s: %s is remote and cannot be mounted\n",
			myname, bs);
		break;
	case EMULTIHOP:
		fprintf(stderr,
			"%s: components of %s require\n\thopping to multiple remote machines\n",
			myname, mp);
		break;
	default:
		perror(myname);
		fprintf(stderr, "%s: cannot mount %s\n", myname, bs);
	}
}

u_getckt(addr,token)
	struct address	*addr;
	struct rf_token	*token;
{
	int fd;
	char mypasswd[20];
	struct gdpmisc gdpmisc;
	int pfd;
	int num;
	char passfile[BUFSIZ];
	char *netspec;
	char *tp;
	char modname[FMNAMESZ];
	extern ndata_t ndata;
	char *getnetspec();

 	gdpmisc.hetero = gdpmisc.version = gdpmisc.ngroups_max = 0;

	if (Bypass)
		sprintf(passfile, PASSFILE, Bypass);
	else {
		if ((netspec = getnetspec()) == NULL) {
			fprintf(stderr,"network specification not set\n");
			return(-1);
		}
		tp = strtok (netspec, ",");
		sprintf(passfile, PASSFILE, tp);
	}
	if (((pfd = open(passfile, O_RDONLY)) < 0)
	  || ((num = read(pfd, &mypasswd[0], sizeof(mypasswd)-1)) < 0)) {
		strcpy(mypasswd, "np");
	} else { 
		mypasswd[num] = '\0';
		(void) close(pfd);
	}

	if ((fd = att_connect(addr, RFS)) == -1) {
		return(-1);
	}
	if (rf_request(fd, RF_RF) == -1) {
		t_cleanup(fd);
		return(-1);
	}
 	if ((gdpmisc.version =
 	    negotiate(fd, &mypasswd[0], CLIENT, &gdpmisc.ngroups_max)) < 0) {
		(void) fprintf(stderr,
			"%s: negotiations failed\n%s: possible cause: machine password incorrect\n",
			myname, myname);
		t_cleanup(fd);
		return(-1);
	}
	gdpmisc.hetero = ndata.n_hetero;
	if (ioctl(fd, I_LOOK, modname) >= 0) {
		if (strcmp(modname, TIMOD) == 0)
			if (ioctl(fd, I_POP) < 0) {
				fprintf(stderr, "%s: ", myname);
				perror("warning");
			}
	}
	if (rfsys(RF_FWFD, fd, token, &gdpmisc) <0) {
		perror(myname);
		(void) t_close(fd);
		return(-1);
	}
	return(0);
}

t_cleanup(fd)
	int fd;
{
	(void) t_snddis(fd, (struct t_call *)NULL);
	(void) t_unbind(fd);
	(void) t_close(fd);
}

static
char *
findaddr(mach_name)
char *mach_name;
{
	char *netspec;
	char *ret;
	char *tpl;
	char *findaddr1();
	extern char *strchr();

	if((tpl = getnetspec()) == NULL)
	{
		fprintf(stderr, "mount: cannot obtain network specification\n");
		return(NULL);
	}
	ret = NULL;
	do
	{
		netspec = tpl;
		if (tpl = strchr(tpl, ','))
			*tpl++ = '\0';

		if (ret = findaddr1(netspec, mach_name))
			break;
	} while(tpl);

	return(ret);
}

static char *
findaddr1(netspec, mach_name)
	char	*netspec;
	char	*mach_name;
{
	char	netmaster[BUFSIZ];
	char	dommaster[BUFSIZ];
	char	*file[2];
	char	*f_name, *f_cmd, *f_addr;
	FILE	*fd;
	int	i;
	char	abuf[BUFSIZ];
	static	char	retbuf[BUFSIZ];

	(void) sprintf(netmaster, TPNETMASTER, netspec);
	(void) sprintf(dommaster, TPDOMMASTER, netspec);
	file[0] = netmaster;
	file[1] = dommaster;

	/*
	 *	Create a string of the form "netspec machaddr"
	 *	and return that string or NULL if error.
	 */

	for (i = 0; i < 2; i ++) {
		if ((fd = fopen(file[i], "r")) == NULL)
			continue;
		while (fgets(abuf, sizeof(abuf), fd) != NULL) {
			f_name = strtok(abuf, " \t");
			f_cmd  = strtok(NULL, " \t");
			if ((strcmp(f_cmd, "a") == 0 || strcmp(f_cmd, "A") == 0)
			  && (strcmp(f_name, mach_name) == 0)) {
				strncpy(retbuf, netspec, sizeof(retbuf));
				strncat(retbuf, " ", sizeof(retbuf)-strlen(retbuf));
				if ((f_addr = strtok(NULL, "\n")) != NULL)
					strncat(retbuf, f_addr, sizeof(retbuf)-strlen(retbuf));
				fclose(fd);
				return(retbuf);
			}
		}
	}
	fclose(fd);
	return(NULL);
}

char *
getnetspec()
{
	static char netspec[BUFSIZ];
	FILE   *fp;

	if (((fp = fopen(NETSPEC, "r")) == NULL)
	 || (fgets(netspec, BUFSIZ, fp) == NULL))
		return(NULL);
	/*
	 *	get rid of training newline if present.
	 */
	if (netspec[strlen(netspec)-1] == '\n')
		netspec[strlen(netspec)-1] = '\0';

	fclose(fp);
	return(netspec);
}

usage()
{
	fprintf(stderr,
		"%s Usage:\n%s [-F %s] [-rcd] [-o specific_options] {special | mount_point}\n%s [-F %s] [-rcd] [-o specific_options] special mount_point\n",
		fstype, myname, fstype, myname, fstype);

	exit(1);
}
