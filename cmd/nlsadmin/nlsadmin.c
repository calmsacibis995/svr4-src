/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nlsadmin:nlsadmin.c	1.6.5.1"

/*
 * nlsadmin.c -- control program for the network listener service
 *
 * This program replaces a previous version of nlsadmin.
 * 
 * This version of nlsadmin works with the service access facility to
 * control the network listener.  The functionality of the SVR3.2 nlsadmin
 * command is supported through calls to the more general sacadm and pmadm
 * commands available through SAF.  Users should migrate away from nlsadmin
 * to sacadm and pmadm for these functions.
 *
 * The -m option of the SVR3.2 nlsadmin command is now ignored.
 *
 * The -t option associates an address with service code 1 (same as in SVR3.2).
 * The -l option associates an address with service code 0.
 *
 * nlsadmin also contains new functionality -- the ability to format a 
 * "listener-specific" string to put in the _pmtab database.  This
 * functionality is required by SAF.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sac.h>
#include "nlsadmin.h"

#define OPTIONS	"a:c:d:e:ikl:mo:p:qr:st:vw:xy:z:A:N:VDR:"
#ifndef FALSE
#define TRUE	1
#define FALSE	0
#endif
/*
 * defines for -q exit codes: QZERO is used for conditions that the
 * man page documents as returning 0, QONE for those that return 1
 */
#define QZERO	0
#define QONE	1

/*
 * defines for simulated standard error format code
 */
#define MM_NOSEV        0
#define MM_HALT         1
#define MM_ERROR        2
#define MM_WARNING      3
#define MM_INFO         4

char	*Nlsname;		/* set to argv[0]			*/
char	Label[25];		/* label component for fmtmsg		*/
int	Quietflag = FALSE;	/* set to TRUE when -q used		*/

extern	int errno;
void	nlsmesg();
uid_t	geteuid();
char	*nexttok();
char	*pflags();
char	*gencmdstr();

struct	svcfields {
	char	*pmtag;
	char	*pmtype;
	char	*svc_code;
	char	*flags;
	char	*id;
	char	*res1;
	char	*res2;
	char	*res3;
	char	*addr;
	char	*rpc;
	char	*lflags;
	char	*modules;
	char	*command;
	char	*comment;
};


main(argc, argv)
int argc;
char **argv;
{
	extern	char *optarg;
	extern	int optind;
	int	c;			/* used for return from getopt  */
	char	*addrptr = NULL;	/* set when -A address is used	*/
	char	*rpcptr = NULL;		/* set when -R rpcinfo is used	*/
	char	*cmdptr = NULL;		/* set with -c command		*/
	char	*comptr = NULL;		/* set with -y comment (old)	*/
	char	*idptr = NULL;		/* set with -w id (old)		*/
	char	*lptr = NULL;		/* set with -l addr (old)	*/
	char	*moduleptr = NULL;	/* set with -m modules		*/
	char	*pipeptr = NULL;	/* set with -o pipe		*/
	char	*svcptr = NULL;		/* set when service code used (old) */
	char	*tptr = NULL;		/* set when -t addr used (old)	*/
	char	*netspec = NULL;	/* set to the network specification */
	int	flag = 0;		/* bit flag of type of command	*/
	int	exitcode = 0;		/* exit status of this command	*/
	int	lflags = 0;		/* listener flags		*/
	char	buf[BUFSIZ];		/* temp buffer #1		*/
	char	mesg[BUFSIZ];		/* temp buffer #2		*/
	FILE	*fp;			/* used for checking netspec	*/
	char	*ptr;			/* temp pointer			*/
	char	*ptr2;			/* temp pointer			*/
	int	sawsep = 0;		/* flag for RPC separator	*/

	Nlsname = argv[0];
	sprintf(Label, "UX:%.14s", argv[0]);	/* for standard message fmt */

	while ((c = getopt(argc, argv, OPTIONS)) != -1) {
		switch (c) {
		case 'a':
			if ( (flag && (flag != CMDFLAG)) || svcptr || Quietflag
			      || addrptr || rpcptr || lflags)
				usage(INCONSISTENT);
			svcptr = optarg;
			break;
		case 'c':
			if ( (flag && (flag != CMDFLAG)) || cmdptr || Quietflag )
				usage(INCONSISTENT);
			cmdptr = optarg;
			flag |= CMDFLAG;
			break;
		case 'd':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			svcptr = optarg;
			flag |= DISFLAG;
			break;
		case 'e':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			svcptr = optarg;
			flag |= ENAFLAG;
			break;
		case 'i':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= INIFLAG;
			break;
		case 'k':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= KILFLAG;
			break;
		case 'l':
			if ( ( flag && (flag != ADRFLAG)) || svcptr || lptr
			      || Quietflag || comptr || addrptr || rpcptr
			      || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			lptr = optarg;
			flag |= ADRFLAG;
			break;
		case 'm':
			if ( (flag && (flag != CMDFLAG)) || Quietflag || rpcptr || lflags )
				usage(INCONSISTENT);
			flag |= CMDFLAG;
			break;
		case 'o':
			if ( flag || svcptr || Quietflag || comptr || idptr || netspec )
				usage(INCONSISTENT);
			pipeptr = optarg;
			flag |= PIPFLAG;
			break;
		case 'p':
			if ( (flag && (flag != CMDFLAG) && (flag != PIPFLAG)) || Quietflag )
				usage(INCONSISTENT);
			moduleptr = optarg;
			break;
		case 'q':
			if ( (flag && (flag != ZZZFLAG)) || Quietflag || comptr 
			     || rpcptr || lflags || idptr )
				usage(INCONSISTENT);
			Quietflag = TRUE;
			break;
		case 'r':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= REMFLAG;
			svcptr = optarg;
			break;
		case 's':
			if ( flag || svcptr || Quietflag || comptr || addrptr
			     || rpcptr || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= STAFLAG;
			break;
		case 't':
			if ( (flag && (flag != ADRFLAG)) || svcptr || tptr
			      || Quietflag || comptr || addrptr || rpcptr
			      || cmdptr || idptr || lflags )
				usage(INCONSISTENT);
			tptr = optarg;
			flag |= ADRFLAG;
			break;
		case 'v':
			if ( flag || svcptr || Quietflag || comptr || rpcptr
			     || addrptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= VBSFLAG;
			break;
		case 'w':
			if ( (flag && (flag != CMDFLAG)) || Quietflag || idptr
			     || rpcptr || addrptr || lflags )
				usage(INCONSISTENT);
			idptr = optarg;
			break;
		case 'x':
			if ( flag || svcptr || Quietflag || netspec || comptr
			     || rpcptr || addrptr || lflags || idptr )
				usage(INCONSISTENT);
			flag |= NETFLAG;
			break;
		case 'y':
			if ( (flag && (flag != CMDFLAG)) || Quietflag || comptr
			     || rpcptr || addrptr || lflags )
				usage(INCONSISTENT);
			comptr = optarg;
			break;
		case 'z':
			if ( flag || svcptr || comptr || addrptr || rpcptr
			     || idptr || lflags )
				usage(INCONSISTENT);
			flag |= ZZZFLAG;
			svcptr = optarg;
			break;
		case 'A':
			if ( (flag && (flag != CMDFLAG) && (flag != PIPFLAG))
			     || netspec || svcptr || idptr || comptr )
				usage(INCONSISTENT);
			addrptr = optarg;
			break;
		case 'D':
			if ( (flag && (flag != CMDFLAG) && (flag != PIPFLAG))
			     || netspec || svcptr || idptr || comptr || addrptr
			     || lflags )
				usage(INCONSISTENT);
			lflags |= DFLAG;
			break;
		case 'N':
			if ( netspec )
				usage(INCONSISTENT);
			netspec = optarg;
			break;
		case 'R':
			if ( (flag && (flag != CMDFLAG) && (flag != PIPFLAG))
			     || netspec || svcptr || idptr || comptr )
				usage(INCONSISTENT);
			for (ptr = optarg; *ptr; ++ptr) {
				if ((*ptr == ':') && !sawsep) {
					/*
					 * skip separator - note that if
					 * separator has been seen, it's not
					 * a digit so it will generate a usage
					 * message below like we want
					 */
					sawsep++;
					continue;
				}
				if (!isdigit(*ptr))
					usage(USAGE);
			}
			ptr = strchr(optarg, ':');
			if (ptr)
				/* change the ':' to a ',' */
				*ptr = ',';
			else
				usage(USAGE);
			rpcptr = optarg;
			break;
		case 'V':
			if ( flag || svcptr || Quietflag || comptr || netspec
			     || rpcptr || addrptr || idptr || lflags )
				usage(INCONSISTENT);
			flag |= VERFLAG;
			break;
		case '?':
			usage(USAGE);
		}
	}

	if ((optind < argc) && ! netspec)
		netspec = argv[optind++];
	if (optind < argc)
		usage(USAGE);


	/* determine if this command requires a netspec */
	if (flag != CMDFLAG) {
		/* if flag is CMDFLAG, more complicated checking of netspec
		 * is done below in switch 
		 */
		if ((flag == PIPFLAG || flag == VERFLAG || flag == NETFLAG)) {
			if (netspec)
				usage(USAGE);
		}
		else if (!netspec)
			usage(USAGE);
	}

	if (netspec && (flag != INIFLAG)) {
		sprintf(buf, SAC_LSPM, netspec);

		if ((fp = popen(buf, "r")) == NULL) {
			nlsmesg(MM_ERROR, "System error");
			exit(NLS_SYSERR);
		}

		if (fgets(buf, BUFSIZ, fp) == NULL) {
			nlsmesg(MM_ERROR, "Invalid network specification");
			exit(NLS_BADPM);
		}
		else {
			ptr = strchr(buf, ':');
			ptr++;
			ptr2 = strchr(ptr, ':');
			*ptr2 = NULL;
			if (strcmp(ptr, LISTENTYPE) != 0) {
				sprintf(mesg, "Network specification \"%s\" is not of type %s", netspec, LISTENTYPE);
				nlsmesg(MM_ERROR, mesg);
				exit(NLS_BADPM);
			}
		}

		pclose(fp);
	}

	if (svcptr) {
		/* check to see if service code is "correct" -- right range
		 * and format.  The -m flag is ignored, so no check for
		 * "administrative" service codes (0-100) is done.
		 */
		c = strlen(svcptr);
		if ((c == 0) || (c >= SVC_CODE_SZ)) {
			sprintf(mesg, "Service code contains more than %d characters", SVC_CODE_SZ);
			nlsmesg(MM_ERROR, mesg);
			exit(NLS_SERV);
		}
	}

	switch (flag) {
	default:
		usage(USAGE);
		break;
	case NONE:
		if ( svcptr || comptr || rpcptr || lflags || idptr )
			usage(INCONSISTENT);
		exitcode = prt_nets(netspec);
		break;
	case INIFLAG:
		if (geteuid() != ROOT) 
			no_permission();
		exitcode = add_pm(netspec);
		break;
	case CMDFLAG:
		if ( svcptr || comptr || idptr || netspec ) {
			if (geteuid() != ROOT)
				no_permission();
			if ((exitcode = old_addsvc(svcptr, "", cmdptr, comptr, moduleptr, idptr, NULL, netspec)) != NLS_OK)
				switch (exitcode) {
				case NLS_SERV:
					nlsmesg(MM_ERROR, "Service code already exists");
					break;
				default:
					nlsmesg(MM_ERROR, "Could not add service");
					break;
				}
		}
		else {
			if (netspec)
				usage(INCONSISTENT);
			exitcode = prt_cmd(cmdptr, CFLAG | lflags, moduleptr, addrptr, rpcptr);
		}
		break;
	case PIPFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = prt_cmd(pipeptr, PFLAG | lflags, moduleptr, addrptr, rpcptr);
		break;
	case VERFLAG:
		printf("%d\n", VERSION);
		exit(NLS_OK);
		break;
	case DISFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = disable_svc(svcptr, netspec);
		break;
	case ENAFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = enable_svc(svcptr, netspec);
		break;
	case KILFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = kill_listener(netspec);
		break;
	case ADRFLAG:
		/* check for root permissions in setup_addr */
		exitcode = setup_addr(lptr, tptr, netspec);
		break;
	case REMFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = remove_svc(svcptr, netspec, TRUE);
		break;
	case STAFLAG:
		if (geteuid() != ROOT)
			no_permission();
		exitcode = start_listener(netspec);
		break;
	case VBSFLAG:
		exitcode = prt_svcs(NULL, netspec);
		break;
	case NETFLAG:
		exitcode = prt_nets(NULL);
		break;
	case ZZZFLAG:
		exitcode = prt_svcs(svcptr, netspec);
		break;
	}
	if (exitcode == NLS_SYSERR)
		nlsmesg(MM_ERROR, "System error in SAC command");
	exit(exitcode);
}


static char umsg[] = "usage: %s -x\n\
       %s [ options ] netspec\n\
       %s [ options ] -N port_monitor_tag\n\
       %s -V\n\
       %s -c cmd | -o pipename [ -p modules ] [ -A addr | -D ] \\\n\
          [ -R prognum:versnum ]\n\
\n\
       [ options ] are:\n\
       [ -a svc_code -c \"cmd\" -y \"cmt\" [-p modules] [-w id] ]\n\
       [-q] | [-v] | [-s] | [-k] | [-i] |\n\
       [-e svc_code] | [-d svc_code] | [-r svc_code] | [[-q] -z svc_code]\n\
       [[-l addr | -] [-t addr | -]] |\n\
";

usage(flag)
int	flag;
{
	switch (flag) {
	case INCONSISTENT:
		nlsmesg(MM_ERROR, "Inconsistent options");
		break;
	case MISSINGARG:
		nlsmesg(MM_ERROR, "Missing argument");
		break;
	case USAGE:
		break;
	}
	fprintf(stderr, umsg, Nlsname, Nlsname, Nlsname, Nlsname, Nlsname);
	exit(NLS_CMD);
}


/*
 * no_permission:  print out error message and exit when the user needs to
 *                 needs to be root and isn't.
 */

no_permission()
{
	nlsmesg(MM_ERROR, "Must be super user");
	exit(NLS_PERM);
}

/*
 * nlsmesg:  print out either an error or a warning message.  severity must
 *           be either MM_ERROR or MM_WARNING.  this routine will be converted
 *           to use the standard message format later.
 */

void
nlsmesg(severity, text)
int	severity;
char	*text;
{
	int	class;

	if (severity == MM_ERROR)
		fprintf(stderr, "%s: error: %s\n", Nlsname, text);
	else
		fprintf(stderr, "%s: warning: %s\n", Nlsname, text);
	return;
}

/*
 * prt_cmd:  print out the listener-dependent string for sacadm.
 */

prt_cmd(path, flags, modules, addr, rpcp)
char	*path;		/* full path of command or pipe */
long	flags;		/* listener flags		*/
			/* PFLAG for pipe		*/
			/* CFLAG for command		*/
			/* DFLAG for dynamic addr	*/
char	*modules; 	/* STREAMS modules to push	*/
char	*addr;		/* private address		*/
char	*rpcp;		/* RPC prog and ver #		*/
{
	struct	stat	sbuf;
	char	mesgbuf[BUFSIZ];
	char	*tmp;

	if (*path != '/') {
		nlsmesg(MM_ERROR, "Must specify full path name");
		return(NLS_CMD);
	}

	if ((tmp = strchr(path, ' ')) != NULL) 
		*tmp = NULL;

	if (stat(path, &sbuf) < 0) {
		if (errno != EFAULT) {
			sprintf(mesgbuf, "%s does not exist", path);
			nlsmesg(MM_WARNING, mesgbuf);
		}
		else
			return(NLS_SYSERR);
	}

	if (tmp)
		*tmp = ' ';

	printf("%s:%s:%s:%s:%s\n", (addr ? addr : ""), (rpcp ? rpcp : ""),
		pflags(flags), (modules ? modules : ""), path);
	return(NLS_OK);
}

/*
 * old_addsvc:  use pmadm to add a service code to the listener.  this will
 *              not allow specification of a private address -- use pmadm!
 */

old_addsvc(svc, addr, cmd, com, module, id, flags, netspec)
char	*svc;
char	*addr;
char	*cmd;
char	*com;
char	*module;
char	*id;
char	*flags;
char	*netspec;
{
	char	buf[BUFSIZ];
	char	mesgbuf[BUFSIZ];
	int	rtn;
	struct	stat	sbuf;
	char	*tmp;

	if (!svc || !cmd || !com || !netspec)
		usage(MISSINGARG);

	/* create "port-monitor specific" info in the same way as prt_cmd */

	if (*cmd != '/') {
		nlsmesg(MM_ERROR, "Must specify full path name");
		return(NLS_CMD);
	}

	if ((tmp = strchr(cmd, ' ')) != NULL) 
		*tmp = NULL;

	if (stat(cmd, &sbuf) < 0) {
		if (errno != EFAULT) {
			sprintf(mesgbuf, "%s does not exist", cmd);
			nlsmesg(MM_WARNING, mesgbuf);
		}
		else
			return(NLS_SYSERR);
	}

	if (tmp)
		*tmp = ' ';

	if (addr)
		sprintf(mesgbuf, "'%s::c:%s:%s'", addr, module ? module : "" , cmd);
	else
		sprintf(mesgbuf, "'::c:%s:%s'", module ? module : "" , cmd);

	if (flags && *flags)
		sprintf(buf, PM_ADDSVCF, netspec, svc, (id)?id:DEFAULTID, flags, mesgbuf, VERSION, com ? com : "");
	else
		sprintf(buf, PM_ADDSVC, netspec, svc, (id)?id:DEFAULTID, mesgbuf, VERSION, com ? com : "");

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_NOEXIST:
	case E_PMRUN:
	case E_PMNOTRUN:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_DUP:
		return(NLS_SERV);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}

/*
 * prt_nets:  print the status of one network, or all nets if netspec 
 *            is NULL
 */
prt_nets(netspec)
char	*netspec;
{
	char	buf[BUFSIZ];
	FILE	*fp;
	char	*name;
	char	*state;
	char	*type;
	int	found = FALSE;
	int	rtn = NLS_OK;

	if (netspec == NULL) 
		sprintf(buf, SAC_LSTY, LISTENTYPE);
	else 
		sprintf(buf, SAC_LSPM, netspec);

	if ((fp = popen(buf, "r")) == NULL) 
		return(NLS_SYSERR);

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if ((name = nexttok(buf, ":")) == NULL)
			return(NLS_SYSERR);
		if ((type = nexttok(NULL, ":")) == NULL)
			return(NLS_SYSERR);

		if (strcmp(type, LISTENTYPE) != 0)
			continue;  /* ignore other types of port monitors */

		found = TRUE;
		if (nexttok(NULL, ":") == NULL)
			return(NLS_SYSERR);
		if (nexttok(NULL, ":") == NULL)
			return(NLS_SYSERR);
		if ((state = nexttok(NULL, ":")) == NULL)
			return(NLS_SYSERR);
		if (strcmp(state, "ENABLED") == NULL ||
		    strcmp(state, "STARTING") == NULL) {
			rtn = QZERO;
			if (!Quietflag)
				printf("%s\t%s\n", name, "ACTIVE");
		}
		else {
			rtn = QONE;
			if (!Quietflag)
				printf("%s\t%s\n", name, "INACTIVE");
		}
	}
	pclose(fp);

	if (netspec && !found) {
		nlsmesg(MM_ERROR, "Invalid network specification");
		return(NLS_BADPM);
	}

	if (netspec)
		return(rtn);
	else
		return(NLS_OK);

}


/*
 * print info about service on netspec, or all services on netspec 
 * if svc is NULL
 */

prt_svcs(svc, netspec)
char	*svc;
char	*netspec;
{
	char	buf[BUFSIZ];
	char	mesg[BUFSIZ];
	FILE	*fp;
	struct	svcfields entry;
	int	rtn;
	int	found = FALSE;
	char	*p;

	if (svc == NULL) 
		sprintf(buf, PM_LSALL, netspec);
	else 
		sprintf(buf, PM_LSONE, netspec, svc);

	if ((fp = popen(buf, "r")) == NULL) 
		return(NLS_SYSERR);

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if ((rtn = svc_format(buf, &entry)) != 0) {
			switch (rtn) {
			case NOTLISTEN:
				continue;
				break;
			case BADPMFMT:
				return(NLS_SYSERR);
				break;
			case BADLISFMT:
				sprintf(mesg, "Entry for code \"%s\" has incorrect format", entry.svc_code);
				nlsmesg(MM_WARNING, mesg);
				continue;
				break;
			}
		}
		found = TRUE;

		if (!Quietflag) {
			printf("%s\t", entry.svc_code);
			if (*entry.addr)
				printf("%s\t", entry.addr);
			else if (strchr(entry.lflags, 'd'))
				printf("DYNAMIC\t");
			else
				printf("NOADDR\t");

			if (strchr(entry.flags, 'x') == NULL)
				printf("ENABLED \t");
			else
				printf("DISABLED\t");


			printf("%s\t%s\t%s\t%s\t# %s",
				(*entry.rpc)?entry.rpc:"NORPC", entry.id,
				(*entry.modules)?entry.modules:"NOMODULES",
				entry.command, (*entry.comment)?entry.comment:"");
		}
		else {
			if (strchr(entry.flags, 'x') == NULL)
				return(QZERO);
			else
				return(QONE);
		}
	}

	pclose(fp);

	if (rtn == NOTLISTEN) { /* check last return to see if error */
		sprintf(mesg, "Network specification \"%s\" is not of type %s", netspec, LISTENTYPE);
		nlsmesg(MM_ERROR, mesg);
		return(NLS_BADPM);
	}
	if (svc && !found) {
		if (!Quietflag) {
			sprintf(mesg, "Service \"%s\" unknown", svc);
			nlsmesg(MM_ERROR, mesg);
		}
		return(NLS_SERV);
	}

	return(NLS_OK);
}

/*
 * disable_svc:  use pmadm to disable a service 
 */

disable_svc(svc, netspec)
char	*svc;
char	*netspec;
{
	char	buf[BUFSIZ];
	int	rtn;

	sprintf(buf, PM_DISABLE, netspec, svc);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_PMRUN:
	case E_PMNOTRUN:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_NOEXIST:
	case E_DUP:
		nlsmesg(MM_ERROR, "Non-existent service.");
		return(NLS_SERV);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}


enable_svc(svc, netspec)
char	*svc;
char	*netspec;
{
	char	buf[BUFSIZ];
	int	rtn;

	sprintf(buf, PM_ENABLE, netspec, svc);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_PMRUN:
	case E_PMNOTRUN:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_NOEXIST:
	case E_DUP:
		nlsmesg(MM_ERROR, "Non-existent service.");
		return(NLS_SERV);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}


remove_svc(svc, netspec, printerrors)
char	*svc;
char	*netspec;
int	printerrors;
{
	char	buf[BUFSIZ];
	int	rtn;

	sprintf(buf, PM_REMSVC, netspec, svc);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_PMRUN:
	case E_PMNOTRUN:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_NOEXIST:
	case E_DUP:
		if (printerrors)
			nlsmesg(MM_ERROR, "Non-existent service.");
		return(NLS_SERV);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}


kill_listener(netspec)
char	*netspec;
{
	char	buf[BUFSIZ];
	char	mesg[BUFSIZ];
	int	rtn;

	sprintf(buf, SAC_KILLPM, netspec);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_DUP:
	case E_SAFERR:
	case E_SYSERR:
	case E_PMRUN:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_PMNOTRUN:
		sprintf(mesg, "No listener active on network \"%s\"", netspec);
		nlsmesg(MM_ERROR, mesg);
		return(NLS_FAILED);
	case E_NOEXIST:
		nlsmesg(MM_ERROR, "Non-existent port monitor.");
		return(NLS_SERV);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}


/*
 * add_pm:  add a port monitor (initialize directories) using sacadm
 */

add_pm(netspec)
char	*netspec;
{
	char	buf[BUFSIZ];
	char	mesg[BUFSIZ];
	int	rtn;

	sprintf(buf, SAC_ADDPM, netspec, LISTENTYPE, gencmdstr(netspec), VERSION);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	/* get child return value out of exit word */

	switch (rtn) {
	case 0:
		old_addsvc(NLPSSVCCODE, NULL, NLPSSRV, "NLPS server", "", "root", NULL, netspec);
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_RECOVER:
	case E_NOEXIST:
	case E_PMNOTRUN:
	default:
		return(NLS_SYSERR);
		break;
	case E_DUP:
	case E_PMRUN:
		nlsmesg(MM_ERROR, "Listener already initialized");
		return(NLS_FAILED);
		break;
	case E_NOPRIV:
		no_permission();
		break;
	}
}


/*
 * gencmdstr:  generate the correct string to invoke the listener (starlan
 *             requires special handling)
 */

char *
gencmdstr(netspec)
char *netspec;
{
	static char buf[BUFSIZ];

	(void) strcpy(buf, LISTENCMD);
	if (!strcmp(netspec, "starlan"))
		(void) strcat(buf, " -m slan");
	(void) strcat(buf, " ");
	(void) strcat(buf, netspec);
	return(buf);
}


/*
 * start_listener: start the listener
 */

start_listener(netspec)
char	*netspec;
{
	char	buf[BUFSIZ];
	char	scratch[BUFSIZ];
	int	rtn;

	sprintf(buf, SAC_STARTPM, netspec);

	if ((rtn = system(buf)) < 0) 
		return(NLS_SYSERR);
	rtn = (rtn>>8) & 0xff;	
	switch (rtn) {
	case 0:
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_RECOVER:
	case E_PMNOTRUN:
	default:
		return(NLS_SYSERR);
		break;
	case E_NOEXIST:
	case E_DUP:
		nlsmesg(MM_ERROR, "Non-existent port monitor.");
		return(NLS_BADPM);
		break;
	case E_PMRUN:
		nlsmesg(MM_ERROR, "Listener already running");
		return(NLS_FAILED);
	case E_NOPRIV:
		no_permission();
		break;
	}

	sprintf(buf, SAC_ENABLPM, netspec);

	if ((rtn = system(buf)) < 0) {
		return(NLS_SYSERR);
	}
	rtn = (rtn>>8) & 0xff;	
	switch (rtn) {
	case 0:
		return(NLS_OK);
		break;
	case E_BADARGS:
	case E_SAFERR:
	case E_SYSERR:
	case E_RECOVER:
	default:
		return(NLS_SYSERR);
		break;
	case E_NOEXIST:
	case E_DUP:
		nlsmesg(MM_ERROR, "Non-existent port monitor.");
		return(NLS_BADPM);
		break;
	case E_PMRUN:
		nlsmesg(MM_ERROR, "Listener already running");
		return(NLS_FAILED);
	case E_PMNOTRUN:
		nlsmesg(MM_ERROR, "Listener start failed");
		return(NLS_FAILED);
	case E_NOPRIV:
		no_permission();
		break;
	}
}


/*
 * setup_addr:  setup the -l and -t addresses.
 */

setup_addr(laddr, taddr, netspec)
char	*laddr;
char	*taddr;
char	*netspec;
{
	char	buf[BUFSIZ];
	char	mesg[BUFSIZ];
	char	*p;
	int	rtn;
	int	qlisten = FALSE;
	int	qtty = FALSE;
	FILE	*fp;
	struct	svcfields entry;

	if (laddr && *laddr == '-') 
		qlisten = TRUE;

	if (taddr && *taddr == '-')
		qtty = TRUE;

	if (laddr) {
		sprintf(buf, PM_LSONE, netspec, NLPSSVCCODE);

		if ((fp = popen(buf, "r")) == NULL) {
			return(NLS_SYSERR);
		}

		if (fgets(buf, BUFSIZ, fp) != NULL) {
			if ((rtn = svc_format(buf, &entry)) != 0) {
				switch (rtn) {
				case NOTLISTEN:	
					nlsmesg(MM_ERROR, "Incorrect port monitor type.  Must be of type listen");
					return(NLS_FAILED);
					break;
				case BADPMFMT:
					return(NLS_SYSERR);
					break;
				case BADLISFMT:
					sprintf(mesg, "Entry for code \"%s\" has incorrect format", entry.svc_code);
					nlsmesg(MM_WARNING, mesg);
					break;
				}
			}
			else {
				if (qlisten)
					printf("%s\n", entry.addr);
				else {
					if (geteuid() != ROOT)
						no_permission();
					/* add address */
					remove_svc(NLPSSVCCODE, netspec, FALSE);
					p = strchr(entry.comment, '\n');
					if (p)
						*p = '\0';
					old_addsvc(NLPSSVCCODE, laddr, entry.command, entry.comment, entry.modules, entry.id, entry.flags, netspec);
				}
			}
			pclose(fp);
		}
		else if (!qlisten)
			nlsmesg(MM_WARNING, "NLPS service not defined");
	}
	if (taddr) {
		sprintf(buf, PM_LSONE, netspec, TTYSVCCODE);

		if ((fp = popen(buf, "r")) == NULL) {
			return(NLS_SYSERR);
		}

		if (fgets(buf, BUFSIZ, fp) != NULL) {
			if ((rtn = svc_format(buf, &entry)) != 0) {
				switch (rtn) {
				case NOTLISTEN:	
					nlsmesg(MM_ERROR, "Incorrect port monitor type.  Must be of type listen");
					return(NLS_FAILED);
					break;
				case BADPMFMT:
					return(NLS_SYSERR);
					break;
				case BADLISFMT:
					sprintf(mesg, "Entry for code \"%s\" has incorrect format", entry.svc_code);
					nlsmesg(MM_WARNING, mesg);
					break;
				}
			}
			else {
				if (qtty)
					printf("%s\n", entry.addr);
				else {
					if (geteuid() != ROOT)
						no_permission();
					/* add address */
					remove_svc(TTYSVCCODE, netspec, FALSE);
					p = strchr(entry.comment, '\n');
					if (p)
						*p = '\0';
					old_addsvc(TTYSVCCODE, taddr, entry.command, entry.comment, entry.modules, entry.id, entry.flags, netspec);
				}
			}
			pclose(fp);
		}
		else if (!qtty)
			nlsmesg(MM_WARNING, "remote login service not defined");
	}
	return(NLS_OK);
}


/*
 * svc_format:  scan a line of output from pmadm to separate it into fields.
 *              returns BADPMFMT for missing fields or incorrect syntax.
 *                      NOTLISTEN is the port monitor type is not listen.
 *                      BADLISFMT if the listener-specific data is incorrect.
 *                      NLS_OK if everything checked out and data is broken
 *                             into the structure.
 */

svc_format(buf, entry)
char	*buf;
struct	svcfields *entry;
{
	char	*ptr;		/* temporary pointer into buffer	*/
	char	*tmp;		/* temporary pointer into buffer	*/

	entry->pmtag = buf;
	if ((ptr = strchr(buf, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->pmtype = ptr;
	if ((ptr = strchr(entry->pmtype, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->svc_code = ptr;

	if (strcmp(entry->pmtype, LISTENTYPE) != 0)
		return(NOTLISTEN);

	if ((ptr = strchr(entry->svc_code, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->flags = ptr;
	if ((ptr = strchr(entry->flags, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->id = ptr;
	if ((ptr = strchr(entry->id, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->res1 = ptr;
	if ((ptr = strchr(entry->res1, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->res2 = ptr;
	if ((ptr = strchr(entry->res2, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->res3 = ptr;
	if ((ptr = strchr(entry->res3, ':')) == NULL)
		return(BADPMFMT);
	*ptr++ = NULL;
	entry->addr = ptr;
	if ((ptr = strchr(entry->addr, ':')) == NULL)
		return(BADLISFMT);
	*ptr++ = NULL;
	entry->rpc = ptr;
	if ((ptr = strchr(entry->rpc, ':')) == NULL)
		return(BADLISFMT);
	*ptr++ = NULL;
	if (*entry->rpc) {
		if ((tmp = strchr(entry->rpc, ',')) == NULL)
			return(BADLISFMT);
		*tmp = ':';
	}
	entry->lflags = ptr;
	if ((ptr = strchr(entry->lflags, ':')) == NULL)
		return(BADLISFMT);
	*ptr++ = NULL;
	entry->modules = ptr;
	if ((ptr = strchr(entry->modules, ':')) == NULL)
		return(BADLISFMT);
	*ptr++ = NULL;
	entry->command = ptr;
	if ((ptr = strchr(entry->command, '#')) == NULL)
		return(BADLISFMT);
	*ptr++ = NULL;
	entry->comment = ptr;
	return(NLS_OK);
}


/*
 * nexttok - return next token, essentially a strtok, but it can
 *	deal with null fields and strtok can not
 *
 *	args:	str - the string to be examined, NULL if we should
 *		      examine the remembered string
 *		delim - the list of valid delimiters
 */


char *
nexttok(str, delim)
char *str;
register char *delim;
{
	static char *savep;	/* the remembered string */
	register char *p;	/* pointer to start of token */
	register char *ep;	/* pointer to end of token */

	p = (str == NULL) ? savep : str ;
	if (p == NULL)
		return(NULL);
	ep = strpbrk(p, delim);
	if (ep == NULL) {
		savep = NULL;
		return(p);
	}
	savep = ep + 1;
	*ep = '\0';
	return(p);
}


/*
 * pflags - put flags into intelligible form for output
 *
 *	args:	flags - binary representation of flags
 */

char *
pflags(flags)
long flags;
{
	register int i;			/* scratch counter */
	static char buf[BUFSIZ];	/* formatted flags */

	if (flags == 0)
		return("");
	i = 0;
	if (flags & CFLAG) {
		buf[i++] = 'c';
		flags &= ~CFLAG;
	}
	if (flags & DFLAG) {
		buf[i++] = 'd';
		flags &= ~DFLAG;
	}
	if (flags & PFLAG) {
		buf[i++] = 'p';
		flags &= ~PFLAG;
	}
	if (flags) {
		nlsmesg(MM_ERROR, "Internal error in pflags");
		exit(NLS_FAILED);
	}
	buf[i] = '\0';
	return(buf);
}
