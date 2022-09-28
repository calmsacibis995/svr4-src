/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfs.cmds:dfshares/dfshares.c	1.8.3.1"


#include  <sys/types.h>
#include  <sys/vnode.h>
#include  <sys/list.h>
#include  <sys/nserve.h>
#include  <sys/rf_sys.h>
#include  <sys/rf_messg.h>
#include  <sys/rf_comm.h>
#include  <stdio.h>
#include  <ctype.h>
#include  <nserve.h>
#include  <errno.h>

void	exit();
static	char	*cmd;
extern	int	errno;
extern	int	ns_errno;
extern	int	optind,	opterr;
extern	char	*optarg;
extern	char	*dompart();
extern	char	*namepart();

main(argc,argv)
int	argc;
char	*argv[];
{

	int	i, chr;
	int	retval = 0, errflag = 0, hflag = 0;
	int	rtn;
	char 	*usage = "dfshares -F rfs [-h] [server ...]";


	cmd = argv[0];

	if ((rtn = rfsys(RF_RUNSTATE)) < 0) {
		perror(cmd);
		exit(1);
	}

	if (rtn != RF_UP) {
		fprintf(stderr, "%s: RFS is not running\n", cmd);
		exit(1);
	}

	while ((chr = getopt(argc,argv,"h")) != EOF)
		switch(chr) {
	 	case 'h':
			if (hflag)
				errflag = 1;
			else
				hflag = 1;	
			break;
		case '?':
			errflag = 1;
			break;
		}

	if (errflag) {
		fprintf(stderr,"Usage: %s\n",usage);
		exit(1);
	}

	/* verify all names first */
	for (i=optind; i < argc; ++i)
		verify_name(argv[optind]);
	
	if (!hflag)
		printf("RESOURCE              SERVER    ACCESS  TRANSPORT  DESCRIPTION\n");

	if (optind < argc)
		for (i=optind; i < argc; ++i)
			retval |= nsquery(argv[i]);
	else
		retval |= nsquery("*");
	exit(retval);
}

nsquery(name)
char *name;
{
	if (ns_dfinfo(name) == FAILURE) {
		if (ns_errno == R_SETUP) {
			fprintf(stderr, "%s: cannot set up communication with the name server\n", cmd);
			fprintf(stderr, "%s: possible cause: heavily loaded RFS activity\n", cmd);
		} else if (ns_errno == R_RCV) {
			fprintf(stderr, "%s: no information received from name server\n", cmd);
			fprintf(stderr, "%s: possible cause: unknown domain name specified or a connection\n", cmd);
			fprintf(stderr, "         to the name server could not be established\n");
		} else
			nserror(cmd);
		return 1;
	}
	return 0;
}

static
verify_name(name)
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
			fprintf(stderr,"%s: domain name %s<%s> exceeds <%d> characters\n",cmd,dname ? "":"in ",name,SZ_DELEMENT);
			exit(1);
		}

		if (v_dname(domain) != 0) {
			fprintf(stderr,"%s: domain name %s<%s> contains invalid characters\n",cmd,dname ? "":"in ",name);
			exit(1);
		}
	}

	if (*(mach = namepart(name)) != '\0') {
		if (strlen(mach) > SZ_MACH) {
			fprintf(stderr,"%s: nodename %s<%s> exceeds <%d> characters\n",cmd,qname ? "in ":"",name,SZ_MACH);
			exit(1);
		}

		if (v_uname(mach) != 0) {
			fprintf(stderr,"%s: nodename %s<%s> contains invalid characters\n",cmd,qname ? "in ":"",name);
			exit(1);
		}
	}
}
