/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dname:dname.c	1.10.12.1"
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <nserve.h>
#include <sys/rf_sys.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	DOMAIN	1
#define	NETSPEC	2
#define	NSNET	"/etc/rfs/netspec"
#define	OPASSFILE "/etc/rfs/%s/Oloc.passwd"

extern char *optarg;
extern int   optind;

extern char *getdname();
extern char *strpbrk();

main(argc, argv)
int    argc;
char **argv;
{
	int	c, error = 0;
	int	printflag = 0;
	int	setflag = 0;
	char	*newdomain, *newnetspec;
	int	rtn = 0;

	/*
	 *	Parse the command line.
	 */

	while ((c = getopt(argc, argv, "dnaD:N:")) != EOF) {
		switch (c) {
			case 'd':
				printflag |= DOMAIN;
				break;
			case 'n':
				printflag |= NETSPEC;
				break;
			case 'a':
				printflag = DOMAIN | NETSPEC;
				break;
			case 'D':
				if (setflag & DOMAIN)
					error = 1;
				else {
					setflag |= DOMAIN;
					newdomain = optarg;
				}
				break;
			case 'N':
				if (setflag & NETSPEC)
					error = 1;
				else {
					setflag |= NETSPEC;
					newnetspec = optarg;
				}
				break;
			case '?':
				error = 1;
		}
	}

	if (argc == 1)
		printflag = DOMAIN;

	if (optind < argc) {
		fprintf(stderr, "dname: extra arguments given\n");
		error = 1;
	}

	if (error) {
		fprintf(stderr, "dname: usage: dname [-D domain] [-N netspec] [-dna]\n");
		exit(1);
	}

	if (setflag & DOMAIN)
		rtn |= setdomain(newdomain);

	if (setflag & NETSPEC)
		rtn |= setnetspec(newnetspec);

	if (printflag)
		rtn |= pr_info(printflag);

	exit(rtn);
}

static
setdomain(domain)
char	 *domain;
{
	FILE *fp;
	FILE *nfp;
	char passfile[BUFSIZ];
	char opassfile[BUFSIZ];
	char *tp;
	extern char *strtok();
	register char *ptr;
	int  inval_chr;
	char netspec[BUFSIZ];
	char *nsp;

	if (geteuid() != 0) {
		fprintf(stderr, "dname: must be super user to change domain name\n");
		return(1);
	}

	/*
	 *	The new domain name must between 1 and 14 characters
	 *	and must be alphanumeric, "-",  or "_".
	 */

	if (strlen(domain) > 14 || strlen(domain) < 1) {
		fprintf(stderr, "dname: new domain name must be between 1 and 14 characters in length\n");
		return(1);
	}

	inval_chr = 0;
	for (ptr = domain; *ptr; ptr++) {
		if (isalnum(*ptr) || *ptr == '_' || *ptr == '-')
			continue;
		else
			inval_chr = 1;
	}

	if (inval_chr) {
		fprintf(stderr, "dname: invalid characters specified in <%s>\n", domain);
		fprintf(stderr, "dname: domain name must contain only alpha-numeric, '-', or '_' characters\n");
		return(1);
	}

	if (rfsys(RF_SETDNAME, domain, strlen(domain)+1) < 0) {
		if (errno == EEXIST) {
			fprintf(stderr, "dname: cannot change domain name while RFS is running\n");
		} else {
			perror("dname");
		}
		return(1);
	}

	if (((fp = fopen(NSDOM,"w")) == NULL)
	  || (fputs(domain,fp) == NULL)
	  || (chmod(NSDOM, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0)
	  || (chown(NSDOM, 0, 3) < 0)) {
		fprintf(stderr, "dname: warning: error in writing domain name into <%s>\n", NSDOM);
	}

	/*
	 *	Remove the domain password file.
	 */
	if (((nfp = fopen(NSNET,"r")) == NULL)
	|| (fgets(netspec,BUFSIZ,nfp) == NULL)) 
		return(0);
	if (netspec[strlen(netspec)-1] == '\n')
		netspec[strlen(netspec)-1] = '\0';
	nsp = netspec;
	while (tp=strtok(nsp, ",")) {
		nsp = NULL;
		sprintf(passfile, PASSFILE, tp);
		sprintf(opassfile, OPASSFILE, tp);
		rename(passfile,opassfile);
	}
	fclose(nfp);
	return(0);
}

static
setnetspec(netspec)
char *netspec;
{
	register char *tp;
	FILE *fp;
	char  dname[MAXDNAME];
	char  path[BUFSIZ];
	struct stat sbuf;
	extern char *strtok();
	extern char *strchr();
	char	*s, *a;
	int	done;

	if (geteuid() != 0) {
		fprintf(stderr, "dname: must be super user to change network specification\n");
		return(1);
	}

	/*
	 *	Check to see if RFS is running.  If it is, then
	 *	the netspec cannot be changed.
	 */

	if (rfs_up() == 0) {
		fprintf(stderr, "dname: cannot change network specification while RFS is running\n");
		return(1);
	}

	if (*netspec == '\0') {
		fprintf(stderr, "dname: network specification cannot have a null value\n");
		return(1);
	}

	/* check the condition of dname -N netspec, */
	if ((s = (char *)malloc(strlen(netspec)+1)) == NULL) {
		fprintf(stderr, "malloc failed\n");
		return(1);
	}
	s = netspec;
	done = 0;
	while (!done) {
		if ((a = strchr(s, ',')) == NULL)
			done++;
		else {
			if (*++a == '\0') {
				fprintf(stderr, "dname: usage: dname -N netspec[,netspec...]\n");
				return(1);
			}
		}
		s = a;
	}

	if (((fp = fopen(NSNET,"w")) == NULL)
	  || (fputs(netspec,fp) == NULL)
	  || (chmod(NSNET, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0)
	  || (chown(NSNET, 0, 3) < 0)
	  || (fclose(fp) < 0)) {
		fprintf(stderr, "dname: error: error in writing network specification into <%s>\n", NSNET);
		return(1);
	}

	/* check each of the transport names provided */
	while(tp = strtok(netspec, ","))
	{
		netspec = NULL;
		sprintf(path, "/dev/%s", tp);
		if (stat(path, &sbuf) < 0)
			fprintf(stderr, "dname: WARNING: %s does not exist\n", path);
	}

	return(0);
}

static
pr_info(flag)
int flag;
{
	int rtn = 0;

	if (flag & DOMAIN)
		rtn |= printdomain();

	if (flag & NETSPEC) {
		if (rtn == 0 && (flag & DOMAIN))
			putchar(' ');
		rtn |= printnetspec();
	}
	putchar('\n');
	return(rtn);
}

static
printdomain()
{
	char *name;
	register char *ptr;
	int  inval_chr;

	if ((name = getdname()) == NULL) {
		fprintf(stderr, "dname: domain name not set; contact System Administrator\n");
		return(1);
	}

	inval_chr = 0;
	for (ptr = name; *ptr; ptr++) {
		if (isalnum(*ptr) || *ptr == '_' || *ptr == '-')
			continue;
		else
			inval_chr = 1;
	}

	if (strlen(name) > 14 || strlen(name) < 1 || inval_chr == 1) {
		fprintf(stderr, "dname: domain name <%s> corrupted; contact System Administrator\n", name);
		return(1);
	}

	printf("%s", name);
	return(0);
}

static
printnetspec()
{
	FILE *fp;
	char netspec[BUFSIZ];

	if (((fp = fopen(NSNET,"r")) == NULL)
	|| (fgets(netspec,BUFSIZ,fp) == NULL)) {
		fprintf(stderr, "dname: network specification not set; contact System Administrator\n");
		return(1);
	}

	/*
	 *	get rid of trailing newline, if there
	 */
	if (netspec[strlen(netspec)-1] == '\n')
		netspec[strlen(netspec)-1] = '\0';

	printf("%s", netspec);
	fclose(fp);
	return(0);
}

static
char	*
getdname()
{
	static char dname[MAXDNAME];
	FILE	*fp;

	/*
	 *	If the domain name cannot be obtained from the system,
	 *	get it from the domain file.
	 */

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
