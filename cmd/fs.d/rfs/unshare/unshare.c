/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfs.cmds:unshare/unshare.c	1.6.3.1"


#include  <stdio.h>
#include  <ctype.h>
#include  <string.h>
#include  <nserve.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <sys/errno.h>
#include  <sys/rf_sys.h>

#define   MASK      00644
#define   MAXFIELD  5
#define   BUFSIZE   512
#define   SAME      0
#define   SHAREFILE "/etc/dfs/sharetab"   /* share table */
#define   SHARELOCK "/etc/dfs/sharetab.lck"    /* lock file for sharetab */
#define   TEMPSHARE "/etc/dfs/tmp.sharetab"	/* temp file */

extern	int	errno;
extern	int	ns_errno;
extern	char	*dompart();
extern	char	*namepart();
extern	void	exit();
static	char	*fieldv[MAXFIELD];

main(argc,argv)
	int	argc;
	char	*argv[];
{
	char	*path_to_res();
	FILE	*sharefp;
	int	sys_err;
	int	temprec;
	int	resnum;				/* cursor for arg list */
	char	*usage = "unshare -F rfs {resource | pathname} ...";
	char	*resource;
	char	*cmd = argv[0];
	int	exit_status = 0;	/* set to 1 for failure(s) */
	int	shfile_flag = 1;	/* set to 0 if SHAREFILE not found */

	/*
	 *	There must be at least one argument, the resource
	 *	being unadvertised.
	 */
	if (argc == 1) {
		fprintf(stderr,"Usage: %s\n",usage);
		exit(1);
	}

	if (geteuid() != 0) {
		fprintf(stderr,"%s: must be super-user\n",cmd);
		exit(1);
	}

	/*
 	 * Lock a temporary file to prevent many advertises, or
 	 * unadvertises from updating "/etc/dfs/sharetab" at the same time.
 	 */
	if ((temprec = creat(SHARELOCK, 0600)) == -1 ||
     	    lockf(temprec, F_LOCK, 0L) < 0) {
		fprintf(stderr,
		    "%s: warning: cannot lock temp file <%s>\n",
		    cmd, SHARELOCK);
	}
	
	/* convert any path names to resource names first */

	if ((sharefp = fopen(SHAREFILE, "r")) == NULL)
		(void)fprintf(stderr,"unshare: warning: <%s> does not exist\n",
			SHAREFILE);
	else {
		for (resnum = 1; resnum < argc; resnum++)
			if (argv[resnum][0] == '/')
				if (!(argv[resnum] =
					path_to_res(sharefp, argv[resnum])))
					exit_status = 1;
		fclose(sharefp);
	}

	for (resnum = 1; resnum < argc; resnum++) {
		sys_err = 0;

		if (!argv[resnum])	/* problems with path_to_res */
			continue;

		if (!verify_name(argv[resnum], &resource, cmd)) {
			exit_status = 1;
			continue;
		}

		/*
	 	 * Unadvertise the resource locally.
		 */ 	 
		if (rfsys(RF_UNADVFS, resource) == -1)  {
			if (errno != ENODEV) {
				rpterr(resource, cmd);
				exit_status = 1;
				continue;
			} else {
				sys_err = 1;
			}
		}
	
		/*
		 * Update the sharetab file, if any.
		 */
		if (shfile_flag) {
			shfile_flag = update_entry(resource, sys_err, cmd);
		}
	
		/*
		 * Send the resource name to the name server to unadvertise
		 * it there.
		 */
		if (ns_unadv(argv[resnum]) == FAILURE) {
			if (sys_err) {
				if (ns_errno == R_PERM) {
					fprintf(stderr, "%s: <%s> not advertised by this host\n",
					    cmd, resource);
				} else {
					nserror(cmd);
				}
				exit_status = 1;
			}
		}
	}
	exit(exit_status);
}

/*
 * Verify that the domain and resource parts of name are acceptable.
 * If they are, assign resource to point to the (possibly truncated)
 * resource part of name.  Return 1 for success, 0 for failure, also 
 * print appropriate diagnostics in error cases. 
 */
int
verify_name(name, resource, cmd)
	char	*name;
	char	**resource;
	char	*cmd;
{
	char	*domain;
	int	qname = 0;

	if (*(domain = dompart(name)) != '\0') {
		qname = 1;
		if (strlen(domain) > SZ_DELEMENT) {
			fprintf(stderr,"%s: domain name in <%s> exceeds <%d> characters\n",cmd,name,SZ_DELEMENT);
			return 0;
		}
		if (v_dname(domain) != 0) {
			fprintf(stderr,"%s: domain name in <%s> contains invalid characters\n",cmd,name);
			return 0;
		}
	}

	if (*(*resource = namepart(name)) == '\0') {
		fprintf(stderr,"%s: resource name must be specified\n",cmd);
		return 0;
	}

	if (v_resname(*resource) != 0) {
		fprintf(stderr,
		    "%s: resource name %s<%s> contains invalid characters\n",
		    cmd, qname ? "in " : "", name);
		return 0;
	}

	if (strlen(*resource) > SZ_RES) {
		(*resource)[SZ_RES] = '\0';
		if (qname) {
			name[strlen(domain) + SZ_RES + 1] = '\0';
		} else {
			name[SZ_RES] = '\0';
		}
		fprintf(stderr,"%s: warning: resource name truncated to <%s>\n",
		    cmd, *resource);
	}
	return 1;
}

/*
 * Update SHAREFILE to reflect the unshare of res.  Print a warning and return 1
 * if that file doesn't exist.
 */
int
update_entry(res, sys_err, cmd)
	char	*res;
	int	sys_err;
	char	*cmd;
{
	register FILE  *fp, *fp1;
	int	found = 0;
	char	advbuf[BUFSIZE], orig[BUFSIZE];
	struct	stat	stbuf;

	if ((fp = fopen(SHAREFILE, "r")) == NULL) {
		fprintf(stderr, "%s: warning: <%s> does not exist\n",
		    cmd, SHAREFILE);
		return 0;
	}
	stat(SHAREFILE, &stbuf);

	if ((fp1 = fopen(TEMPSHARE, "w")) == NULL) {
		fprintf(stderr,
		    "%s: cannot create temporary advertise file <%s>\n",
		    cmd,TEMPSHARE);
		exit(1);
	}

	/*
	 *	Update the local advertise file.
	 */
	while (fgets(advbuf,BUFSIZE,fp)) {
		strcpy(orig, advbuf);
		get_data(advbuf);
		
		if (!strcmp(fieldv[2], "rfs") && !(strcmp(fieldv[1], res)))
			/* we've got the resource */
			found = 1;
		else
			fprintf(fp1, "%s", orig);
	}

	if (!found && !sys_err) {
		fprintf(stderr,"%s: warning: <%s> not in <%s>\n",
		    cmd, res, SHAREFILE);
	}

	fclose(fp);
	fclose(fp1);
	unlink(SHAREFILE);
	link(TEMPSHARE, SHAREFILE);
	chmod(SHAREFILE, MASK);
	chown(SHAREFILE, (int)stbuf.st_uid, (int)stbuf.st_gid);
	unlink(TEMPSHARE);
	return 1;
}

static
ns_unadv(res)
char	*res;
{
	struct	nssend	send;
	struct	nssend	*ns_getblock();

	/*
	 *	Initialize structure with the resource name to be 
	 *	unadvertised by the name server.
	 */
	send.ns_code = NS_UNADV;
	send.ns_type = 1;
	send.ns_flag = 0;
	send.ns_name = res;
	send.ns_path = NULL;
	send.ns_desc = NULL;
	send.ns_mach = NULL;

	/*
	 *	Send the structure using the name server function
	 *	ns_getblock().
	 */
	if (ns_getblock(&send) == NULL)
		return(FAILURE);
	
	return(SUCCESS);
}

static
rpterr(res, cmd)
	char	*res;
	char	*cmd;
{
	switch (errno) {
	case EPERM:
		fprintf(stderr,"%s: must be super-user\n", cmd);
		break;
	case ENONET:
		fprintf(stderr,"%s: machine not on the network\n", cmd);
		break;
	case EINVAL:
		fprintf(stderr,"%s: invalid resource name: \n", cmd, res);
		break;
	case EFAULT:
		fprintf(stderr,"%s: bad user address\n", cmd);
		break;
	case ENOPKG:
		fprintf(stderr,"%s: RFS package not installed\n", cmd);
		break;
	default:
		fprintf(stderr, "%s: errno <%d>, cannot unshare <%s>\n",
		    cmd, errno, res);
		break;
	}
}

char empty[] = "";

static
get_data(s)
char	*s;
{
	register int fieldc = 0;

	/*
 	 *	This function parses an advertise entry from 
 	 *	/etc/dfs/sharetab and sets the pointers appropriately.
	 *	fieldv[0] :  pathname
	 *	fieldv[1] :  resource
	 *	fieldv[2] :  fstype
	 *	fieldv[3] :  options
	 *	fieldv[4] :  description
 	 */

	while ((*s != '\n') && (*s != '\0') && (fieldc < 5)) {
		while (isspace(*s))
			s++;
		fieldv[fieldc++] = s;

		if (*s == '"') {	/* get a quoted string */
			for (++s; *s != '"'; ++s) ;
			++s;
		}
		else
			while (*s && !isspace(*s)) ++s;
		if (*s)
			*s++ = '\0';
	}
	while (fieldc <5)
		fieldv[fieldc++] = empty;
}


/*
	path_to_res - convert a pathname to a resource name by looking it
	up in the advertise table.

	return (char *)0 on error
*/

static char resbuf[BUFSIZE];

static char *
path_to_res(fp, path)
FILE	*fp;
char	*path;
{
	char	*fpath, *fs, *rem, *rname;

	/* Look for the path */

	rewind(fp);
	while (fgets(resbuf,BUFSIZE,fp)) {
		fpath = strtok(resbuf, " \t");
		rname = strtok(NULL, " \t");
		fs = strtok(NULL, " \t");
		if (!strcmp(fs, "rfs") && !strcmp(path, fpath))
			return rname;
	}

	(void)fprintf(stderr,"unshare: <%s> not shared\n", path);
	return (char *)0;
}
