/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfs.cmds:share/share.c	1.10.3.1"



#include  <stdio.h>
#include  <fcntl.h>
#include  <ctype.h>
#include  <string.h>
#include  <nserve.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <sys/errno.h>
#include  <sys/rf_sys.h>

#define   SHAREFILE "/etc/dfs/sharetab"   /* share table */
#define   SHARELOCK "/etc/dfs/sharetab.lck"    /* lock file for sharetab */
#define   TEMPSHARE "/etc/dfs/tmp.sharetab"	/* temp file */
#define   NUM_MACHS 100
#define   MASK      00644
#define   BUFSIZE   512
#define   RES	    1
#define   MACH      2
#define   SZ_CLIENT (SZ_DELEMENT + SZ_MACH + 1)
#define   SAME 	    0

void	exit();

static	int	pos = 0;
static	int	dflag = 0;
static	int	low_index = 0;

static	char	**clist = NULL;       /* pointer to list of clients    */
static	char	*cmd;

static char	*shareflg[] = {"rw", "ro"};

char	*malloc();
static	struct	stat	stbuf;
extern	int	errno;
extern	int	optind, opterr;
extern	char	*optarg;
extern	char	*dompart();
extern	char	*namepart();

#define O_RO 0
#define O_RW 1
char *myopts[] = { "ro", "rw", NULL };

main(argc,argv)
int  argc;
char *argv[];
{

	int	chr, temprec, subflag = 0;
	short	req_type;
	short	roflag = A_RDWR, errflag = 0;
	char	*options, *value;
	char	*path;
	char	*resource;
	char	*desc = "";
	char	*usage = 
		"share [-F rfs] [-o rw[=client:...]|ro[=client:...]] [-d desc] pathname resource";
	char	*mlist[50];

	cmd = argv[0];

	/*	
	 *	If no arguments, list the local advertise file, else
	 *	check for optional arguments.
	 */

	if (argc <= 1) {
		fprintf(stderr,"Usage:  %s\n",usage);
		exit(0);
	}

	/*
	 *	Check for optional arguments.
	 *	-o : subopts (access)
	 *	-d : description
	 */

	mlist[0] = NULL;
	while ((chr = getopt(argc,argv,"o:d:")) != EOF)
		switch(chr) {
		case 'o':
			options = optarg;
			while (*options != '\0') {
				switch(getsubopt(&options, myopts, &value)) {
				case O_RO:
					if (subflag++) {
						errflag = 1;
						break;
					}
					roflag = A_RDONLY;
					if (value)
						genlist(mlist, value);
					break;
				case O_RW:
					if (subflag++) {
						errflag = 1;
						break;
					}
					if (value)
						genlist(mlist, value);
					break;
				default:
					errflag = 1;
					break;
				}
			}
			break;

		case 'd':
			if (dflag)
				errflag = 1;
			else {
				dflag = 1;
				desc = optarg;
				if (strlen(desc) > SZ_DESC) {
					optarg[SZ_DESC] = '\0';
					fprintf(stderr,"%s: warning: description truncated to <%s>\n",cmd,desc);
				}
			}
			break;
		case '?':
			errflag = 1;
			break;
		}

	/*
	 *	If a resource is being advertised, there 
	 *	must be at least 2 arguments, the symbolic
	 *	name and the directory being advertised.
	 */

	if (errflag || (argc <= optind + 1)) {
		fprintf(stderr,"Usage:  %s\n",usage);
		exit(1);
	}

	if (geteuid() != 0) {
		fprintf(stderr,"%s: must be super-user\n",cmd);
		exit(1);
	}

	resource = argv[optind+1];

	/*
	 *	Determine if resource name contains '/', '.', or non-printable
	 *	characters.
	 */

	if (v_resname(resource) != 0) {
		fprintf(stderr,"%s: invalid characters specified in <%s>\n",cmd,resource);
		fprintf(stderr,"%s: resource name cannot contain '/', '.' ,' ' or non-printable characters\n",cmd);
		exit(1);
	}

	if (strlen(resource) > SZ_RES) {
		resource[SZ_RES] = '\0';
		fprintf(stderr,"%s: warning: resource name truncated to <%s>\n",cmd,resource);
	}

	path = argv[optind];
	if (*path != '/') {
		fprintf(stderr,"%s: full pathname required\n",cmd);
		exit(1);
	}

	/*
	 *	Form the authorized client list.
	 */

	if (mlist && mlist[0]) {
		creatlist(mlist);
		if (clist[0] == NULL) {
			fprintf(stderr,"%s: no valid client names\n",cmd);
			exit(1);
		}
		roflag |= A_CLIST;
	}

	req_type = NS_ADV;	

	/*
	 *	Lock a temporary file to prevent many advertises from
	 *	updating "/etc/dfs/sharetab" at the same time.
	 */

	if ((temprec = creat(SHARELOCK, 0600)) == -1 ||
	     lockf(temprec, F_LOCK, 0L) < 0) {
		fprintf(stderr, "%s: warning: cannot lock temp file <%s>\n",cmd,SHARELOCK);
	}
	
	/*
	 *	Advertise a new resource,
	 *	or to modify the client list of an advertised resource.
	 */

	if (rfsys(RF_ADVFS,path,resource,roflag,clist) == -1) {
		rpterr(resource,path);
		exit(1);	
	}
	roflag &= ~(A_CLIST | A_MODIFY);

	/*
	 *	Send the advertise information to the name server.
	 */

	if (ns_adv(req_type,path,resource,roflag,desc,clist) == FAILURE) {
		nserror(cmd);
		if (req_type == NS_ADV)
			rfsys(RF_UNADVFS, resource);
		exit(1);
	}

	add_entry(resource,path,roflag,desc,mlist);

	exit(0);
}

static
creatlist(clients)
char	*clients[];
{

	register int index = 0;
	char	 *clname;


	if ((clist = (char **)malloc(NUM_MACHS * sizeof(char *))) == NULL) {
		fprintf(stderr,"%s: cannot allocate memory for client list\n",cmd);
		exit(1);
	}

	/*
	 *	Assume the names to be valid client names and form 
	 *	a client list.
	 */

	while ((clname = clients[index]) != NULL) {
		if (*clname == '\0' || verify(clname)) {
			clients[index++][0] = '\0';
			continue;
		}

		if (!invalid(clname) && !in_clist(clname))
			add_client(clname);
		else
			clients[index][0] = '\0';
		index++;
	}

	if (pos == (low_index + NUM_MACHS))
		alloc_mem();
	clist[pos] = NULL;
}

static
in_clist(name)
char	*name;
{

 	register int i;

	for (i = 0; i < pos; i++) {
		if (strcmp(name,clist[i]) == SAME)
			return(1);
	}
	return(0);
}

static
add_client(name)
char	*name;
{

	if (pos == (low_index + NUM_MACHS))
		alloc_mem();
	if ((clist[pos] = malloc((unsigned)strlen(name)+1)) == NULL ) {
		fprintf(stderr,"%s: cannot allocate memory for client list\n",cmd);
		exit(1);
	}
	strcpy(clist[pos++],name);
}

static
verify(name)
char	*name;
{

	register char *chr;

	if (strlen(name) > SZ_CLIENT) {
		fprintf(stderr,"%s: client name <%s> exceeds <%d> characters, ignored\n",cmd,name,SZ_CLIENT);
		return(1);
	}

	chr = name;
	while (*chr != '\0') {
		if (!isprint(*chr)) {
			fprintf(stderr,"%s: client name <%s> contains non-printable characters, ignored\n",cmd,name);
			return(1);
		}
		chr++;
	}
	return(0);
}

static
alloc_mem()
{

	char   *realloc();

	if ((clist = (char **)realloc((char *)clist,(NUM_MACHS + pos) * sizeof (char *))) == NULL) {
		fprintf(stderr,"%s: cannot allocate memory for client list\n",cmd);
		exit(1);
	}
	low_index = pos;
}

static
invalid(name)
char	*name;
{

	char	*mach;
	char	*domain;
	int	qname = 0, dname = 0;

	if (name[strlen(name)-1] == SEPARATOR)
		dname = 1;

	if (*(domain = dompart(name)) != '\0') {
		qname = 1;
		if (strlen(domain) > SZ_DELEMENT) {
			fprintf(stderr,"%s: domain name %s<%s> exceeds <%d> characters, ignored\n",cmd,dname ? "":"in ",name,SZ_DELEMENT);
			return(1);
		}

		if (v_dname(domain) != 0) {
			fprintf(stderr,"%s: domain name %s<%s> contains invalid characters, ignored\n",cmd,dname ? "":"in ",name);
			return(1);
		}
	}

	if (*(mach = namepart(name)) != '\0') {
		if (strlen(mach) > SZ_MACH) {
			fprintf(stderr,"%s: nodename %s<%s> exceeds <%d> characters, ignored\n",cmd,qname ? "in ":"",name,SZ_MACH);
			return(1);
		}

		if (v_uname(mach) != 0) {
			fprintf(stderr,"%s: nodename %s<%s> contains invalid characters, ignored\n",cmd,qname ? "in ":"",name);
			return(1);
		}
	}
	return(0);
}

static
add_entry(res,path,rflag,desc,clients)
short	rflag;
char	*res, *path, *desc;
char	*clients[];
{
	register FILE	*fp;
	int	index = 0;
	register int firstclient = 1;


	if ((fp = fopen(SHAREFILE, "a")) == NULL) {
		fprintf(stderr,"%s: cannot open <%s>\n",cmd,SHAREFILE);
		exit(1);
	}
	
	fprintf(fp,"%s %s rfs %s",path, res, shareflg[rflag]);

	/* modify "ro" or "rw" option with an '=' followed
	   by a client list */

	if (clients[index])
		while (clients[index] != NULL) {
			if (clients[index][0] != '\0') {
				fprintf(fp,"%c%s", (firstclient)?'=':':',
					clients[index]);
				firstclient = 0;
			}
			index++;
		}

	fprintf(fp, " %s\n", desc);
	fclose(fp);
}

static
ns_adv(req_type,path,res,roflg,desc,namelist)
short	req_type, roflg;
char	*path, *res, *desc;
char	**namelist;
{

	struct	nssend	send;
	struct	nssend	*ns_getblock();

	/*	
	 *	Initialize structure with information to be sent
	 *	to the name server.
	 */

	send.ns_code = req_type;
	send.ns_type = 1;
	send.ns_flag = roflg;
	send.ns_name = res;
	send.ns_path = path;
	send.ns_desc = desc;
	send.ns_mach = namelist;

	/*
	 *	Send the structure using the name server function
	 *	ns_getblock().
	 */

	if (ns_getblock(&send) == NULL)
		return(FAILURE);
	
	return(SUCCESS);
} 

static
rpterr(res,dir)
char	*res;
char	*dir;
{

	switch(errno) {
	case EPERM:
		fprintf(stderr,"%s: must be super-user\n",cmd);
		break;
	case ENOENT:
		fprintf(stderr,"%s: <%s> no such file or directory\n",cmd,dir);
		break;
	case ENONET:
		fprintf(stderr,"%s: machine not on the network\n",cmd);
		break;
	case ENOTDIR:
		fprintf(stderr,"%s: <%s> not a directory\n",cmd,dir);
		break;
	case EREMOTE:
		fprintf(stderr,"%s: <%s> is remote\n",cmd,dir);
		break;
	case EADV:
		fprintf(stderr,"%s: <%s> already advertised\n",cmd,dir);
		break;
	case EROFS:
		fprintf(stderr,"%s: <%s> write-protected\n",cmd,dir);
		break;
	case EINTR:
		fprintf(stderr,"%s: system call interrupted\n",cmd);
		break;
	case EBUSY:
		fprintf(stderr,"%s: resource <%s> currently advertises a different directory\n",cmd,res);
		break;
	case EEXIST:
		fprintf(stderr,"%s: re-advertise error: <%s> was originally advertised under\n     a different resource name\n",cmd,dir);
		break;
	case ENOSPC:
		fprintf(stderr,"%s: advertise table overflow\n",cmd);
		break;
	case EINVAL:
		fprintf(stderr,"%s: invalid resource name\n",cmd);
		break;
	case EFAULT:
		fprintf(stderr,"%s: bad user address\n",cmd);
		break;
	case ENOMEM:
		fprintf(stderr,"%s: not enough memory\n",cmd);
		break;
	case ENODEV:
		fprintf(stderr,"%s: <%s> not advertised\n",cmd,res);
		break;
	case ENOPKG:
		fprintf(stderr,"%s: RFS package not installed\n",cmd);
		break;
	case ESRMNT:
		fprintf(stderr,"%s: re-advertise error: a client that is not in the specified\n     client list currently has <%s> mounted\n",cmd,res);
		break;
	case EACCES:
		fprintf(stderr,"%s: re-advertise error: resource <%s> originally advertised\n     with different permissions\n",cmd,res);
		break;
	default:
		fprintf(stderr,"%s: errno <%d>, cannot advertise <%s>\n",cmd,errno,res);
		break;
	}
}

genlist(dest, src)
char **dest;
char *src;
{
	int i = 0;

	while (dest[i] = strtok(src, ":")) {
		src = NULL;
		i++;
	}
}
