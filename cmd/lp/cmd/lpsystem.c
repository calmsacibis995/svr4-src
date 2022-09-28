/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsystem.c	1.8.5.1"

/*==================================================================*/
/*
*/
#include	<sys/utsname.h>
#include	<stdio.h>
#include	<tiuser.h>
#include	<netconfig.h>
#include	<netdir.h>

#include	"lp.h"
#include	"systems.h"
#include	"msgs.h"
#include	"boolean.h"
#include	"debug.h"

#define WHO_AM_I	I_AM_LPSYSTEM
#include "oam.h"

#define	DEFAULT_TIMEOUT	-1
#define	DEFAULT_RETRY	10

static	int	Timeout;
static	int	Retry;
static	char	*Sysnamep;
static	char	*Protocolp;
static	char	*Timeoutp;
static	char	*Retryp;
static	char	*Commentp;

#ifdef	__STDC__
static	int	NotifyLpsched (int, char *);
static	void	SecurityCheck (void);
static	void	TcpIpAddress (void);
static	void	ListSystems (char * []);
static	void	RemoveSystems (char * []);
static	void	AddModifySystems (char * []);
static	void	formatsys (SYSTEM *);
static	void	usage (void);
#else
static	int	NotifyLpsched ();
static	void	SecurityCheck ();
static	void	TcpIpAddress ();
static	void	ListSystems ();
static	void	RemoveSystems ();
static	void	AddModifySystems ();
static	void	formatsys ();
static	void	usage ();
#endif

/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
int
main (int argc, char * argv [])
#else
int
main (argc, argv)

int	argc;
char	*argv [];
#endif
{
		int	c;
		boolean	lflag = False,
			rflag = False,
			Aflag = False,
			badOptions = False;
	extern	int	opterr,
			optind;
	extern	char	*optarg;
 

	while ((c = getopt(argc, argv, "t:T:R:y:lrA?")) != EOF)
	switch (c & 0xFF)
	{
	case 't':
		if (Protocolp)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -t options.\n");
			return	1;
		}
		Protocolp = optarg;
		if (! STREQU(NAME_S5PROTO, Protocolp) &&
		    ! STREQU(NAME_BSDPROTO, Protocolp))
		{
			(void)	fprintf (stderr,
				"ERROR:  Supported protocols are "
				"\"%s\" and \"%s\".\n",
			   	NAME_S5PROTO, NAME_BSDPROTO);
			return	1;
		}
		break;
		
	case 'T':
		if (Timeoutp)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -T options.\n");
			return	1;
		}
		Timeoutp = optarg;
		if (*Timeoutp == 'n')
			Timeout = -1;
		else
		if (sscanf (Timeoutp, "%d", &Timeout) != 1 || Timeout < 0)
		{
			(void)	fprintf (stderr,
				"ERROR:  Bad timeout argument: %s\n",
				Timeoutp);
			return	1;
		}
		break;
		
	case 'R':
		if (Retryp)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -R options.\n");
			return	1;
		}
		Retryp = optarg;
		if (*Retryp == 'n')
			Retry = -1;
		else 
		if (sscanf (Retryp, "%d", &Retry) != 1 || Retry < 0)
		{
			(void)	fprintf (stderr,
				"ERROR:  Bad retry argument: %s\n", Retryp);
			return	1;
		}
		break;
		
	case 'y':
		if (Commentp)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -y options.\n");
			return	1;
			}
		Commentp = optarg;
		break;

	case 'l':
		if (lflag)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -l options.\n");
			return	1;
		}
		lflag++;
		break;
		
	case 'r':
		if (rflag)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -r options.\n");
			return	1;
		}
		rflag++;
		break;

	case 'A':
		if (Aflag)
		{
			(void)	fprintf (stderr,
				"ERROR:  Too many -A options.\n");
			return	1;
		}
		Aflag++;
		break;

	default:
		(void)	fprintf (stderr,
			"ERROR:  Unrecognized option \"-%c\".\n", c & 0xFF);
		return	1;
		
	case '?':
		usage ();
		return	0;
	}

	/*
	**  Check for valid option combinations.
	**
	**  The '-A' option is mutually exclusive.
	**  The '-l' option is mutually exclusive.
	**  The '-r' option is mutually exclusive.
	*/
	if (Aflag && (Protocolp || Timeoutp || Retryp || Commentp))
		badOptions = True;

	if (lflag &&
	   (Protocolp || Timeoutp || Retryp || Commentp || rflag || Aflag))
		badOptions = True;

	if (rflag && (Protocolp || Timeoutp || Retryp || Commentp || Aflag))
		badOptions = True;

	if (badOptions)
	{
		(void)	fprintf (stderr, "ERROR:  Improper usage.\n\n");
		return	1;
	}

	/*
	**	Lets do some processing.
	**	We'll start with the flags.
	*/
	if (Aflag)
	{
		TcpIpAddress ();
		/*NOTREACHED*/
	}
	if (lflag)
	{
		ListSystems (&argv [optind]);
		/*NOTREACHED*/
	}
	if (rflag)
	{
		RemoveSystems (&argv [optind]);
		/*NOTREACHED*/
	}

	AddModifySystems (&argv [optind]);

	return	0;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
SecurityCheck (void)
#else
static	void
SecurityCheck ()
#endif
{
	if (geteuid () != 0)
	{
		(void)	fprintf (stderr,
			"ERROR:  You must be root.\n");
		(void)	exit (1);
	}
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
TcpIpAddress (void)
#else
static	void
TcpIpAddress ()
#endif
{
	int	i;
	struct	utsname		utsbuf;
	struct	netconfig	*configp;
	struct	nd_hostserv	hostserv;
	struct	nd_addrlist	*addrsp;

	struct	netconfig	*getnetconfigent ();

	(void)	uname (&utsbuf);
	configp = getnetconfigent ("tcp");
	if (! configp)
	{
/*
**		(void)	fprintf (stderr,
**			"ERROR:  TCP/IP is not installed.\n");
*/
		LP_ERRMSG (ERROR, E_SYS_NOTCPIP);
		(void)	exit (2);
	}
	hostserv.h_host = utsbuf.nodename;
	hostserv.h_serv = "printer";
	if (netdir_getbyname (configp, &hostserv, &addrsp))
	{
		(void)	fprintf (stderr, "ERROR:  ");
		(void)	perror ("netdir_getbyname");
		(void)	exit (2);
	}
	for (i=0; i < addrsp->n_addrs->len; i++)
		(void)	printf ("%02x", addrsp->n_addrs->buf [i]);
	(void)	printf ("\n");
	(void)	exit (0);
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
ListSystems (char *syslistp [])
#else
static	void
ListSystems (syslistp)

char *syslistp [];
#endif
{
	char	*sysnamep;
	SYSTEM	*systemp;

	if (! *syslistp)
	{
		while ((systemp = getsystem (NAME_ALL)) != NULL)
			formatsys (systemp);
	}
	else
	for (sysnamep = *syslistp; sysnamep; sysnamep = *++syslistp)
	{
		if (STREQU(NAME_ANY, sysnamep)  ||
		    STREQU(NAME_NONE, sysnamep) ||
		    STREQU(NAME_ALL, sysnamep))
		{
			(void)	fprintf (stderr,
				"WARNING:  \"%s\" is a reserved word and may "
				"not be used for a system name.\n",
				sysnamep);
			continue;
		}
		if ((systemp = getsystem (sysnamep)) == NULL)
		{
			(void)	fprintf (stderr,
				"WARNING:  \"%s\" not found.\n",
				sysnamep);
			continue;
		}
		formatsys (systemp);
	}
	(void)	exit (0);
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
RemoveSystems (char *syslistp [])
#else
static	void
RemoveSystems (syslistp)

char	*syslistp [];
#endif
{
	char	*sysnamep;
	SYSTEM	*systemp;


	SecurityCheck ();

	if (! syslistp || ! *syslistp)
	{
		(void)	fprintf (stderr, "ERROR:  Improper usage.\n\n");
		(void)	exit (1);
	}
	for (sysnamep = *syslistp; sysnamep; sysnamep = *++syslistp)
	{
		if (STREQU(NAME_ANY, sysnamep)  ||
		    STREQU(NAME_NONE, sysnamep) ||
		    STREQU(NAME_ALL, sysnamep))
		{
			(void)	fprintf (stderr,
				"WARNING:  \"%s\" is a reserved word and may "
				"not be used for a sysnamep name.\n",
				sysnamep);
			continue;
		}
		if (! (systemp = getsystem (sysnamep)))
		{
			(void)	fprintf (stderr,
				"WARNING:  \"%s\" not found.\n",
				sysnamep);
			continue;
		}
		if (NotifyLpsched (S_UNLOAD_SYSTEM, sysnamep) != MOK ||
		    delsystem (sysnamep))
		{
			(void)	fprintf (stderr,
				"ERROR:  Could not remove \"%s\".\n",
				sysnamep);
			(void)	exit (2);
		}
		else
			(void)	printf ("Removed \"%s\".\n", sysnamep);
	}
	(void)	exit (0);
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
AddModifySystems (char *syslistp [])
#else
static	void
AddModifySystems (syslistp)

char	*syslistp [];
#endif
{
		char	*sysnamep;
		SYSTEM	*systemp,
			sysbuf;
		boolean	modifiedFlag;

	static	SYSTEM	DefaultSystem =
			{
				NULL, NULL, NULL, S5_PROTO, NULL,
				DEFAULT_TIMEOUT, DEFAULT_RETRY,
				NULL, NULL, NULL
			}; 


	SecurityCheck ();

	for (sysnamep = *syslistp; sysnamep; sysnamep = *++syslistp)
	{
		modifiedFlag = False;
		if (systemp = getsystem (sysnamep))
		{
			sysbuf = *systemp;
			modifiedFlag = True;
		}
		else
		{
			sysbuf = DefaultSystem;
			sysbuf.name = sysnamep;
		}
		if (Protocolp)
		{
			if (STREQU (NAME_S5PROTO, Protocolp))
				sysbuf.protocol = S5_PROTO;
			else
				sysbuf.protocol = BSD_PROTO;
		}
		if (Timeoutp)
		{
			sysbuf.timeout = Timeout;
		}
		if (Retryp)
		{
			sysbuf.retry = Retry;
		}
		if (Commentp)
		{
			sysbuf.comment = Commentp;
		}
		if (putsystem (sysnamep, &sysbuf))
		{
			(void)	fprintf (stderr,
				"ERROR:  Could not %s \"%s\".\n",
				modifiedFlag ? "modify" : "add", sysnamep);
			(void)	exit (2);
		}
		if (NotifyLpsched (S_LOAD_SYSTEM, sysnamep) != MOK)
		{
			/*
			**  Try to put the old system back.
			**
			*/
			if (systemp)
			{
				(void)	putsystem (sysnamep, systemp);
			}
			(void)	fprintf (stderr,
				"ERROR:  Could not %s \"%s\".\n",
				modifiedFlag ? "modify" : "add", sysnamep);
			(void)	exit (2);
		}
		if (modifiedFlag)
		{
			(void)	printf ("\"%s\" has been modified.\n",
				sysnamep);
		}
		else
		{
			(void)	printf ("\"%s\" has been added.\n",
				sysnamep);
		}
	}
	(void)	exit (0);
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	int
NotifyLpsched (int msgType, char *sysnamep)
#else
static	int
NotifyLpsched ()

int	msgType;
char	*sysnamep;
#endif
{
	/*----------------------------------------------------------*/
	/*
	*/
		char	msgbuf [MSGMAX];
		ushort	status;

	static	boolean	OpenedChannel	= False;
	static	char	FnName []	= "NotifyLpsched";


	/*----------------------------------------------------------*/
	/*
	*/
	if (! OpenedChannel)
	{
		if (mopen () < 0)
		{
			/*
			**  Assume scheduler is down and we can
			**  do what we want.
			*/
#ifdef	DEBUG
			(void)	fprintf (stderr, "ERROR:  ");
			(void)	perror ("mopen");
#endif
			return	MOK;
		}
		OpenedChannel = True;
	}
	if (putmessage (msgbuf, msgType, sysnamep) < 0)
	{
		(void)	fprintf (stderr, "ERROR:  ");
		(void)	perror ("putmessage");
		(void)	exit (2);
	}
	if (msend (msgbuf) < 0)
	{
		(void)	fprintf (stderr, "ERROR:  ");
		(void)	perror ("putmessage");
		(void)	exit (2);
	}
	if (mrecv (msgbuf, sizeof (msgbuf)) < 0)
	{
		(void)	fprintf (stderr, "ERROR:  ");
		(void)	perror ("mrecv");
		(void)	exit (2);
	}
	if (getmessage (msgbuf, mtype (msgbuf), &status) < 0)
	{
		(void)	fprintf (stderr, "ERROR:  ");
		(void)	perror ("mrecv");
		(void)	exit (2);
	}
	return	(int)	status;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#if	defined(__STDC__)
static	void
usage (void)
#else
static void
usage ()
#endif
{
	(void)	fprintf (stdout,
		"Usage:  lpsystem [ options ] [system-name ... ]\n");
	(void)	fprintf (stdout,
		"\nTo add or modify an existing system:\n");
	(void)	fprintf (stdout,
		"        lpsystem [ -t type ] [ -T timeout ]\n");
	(void)	fprintf (stdout,
	"                 [ -R retry ] [ -y comment ] system-name ...\n");
	(void)	fprintf (stdout,
		"\nTo list a system (or all systems):\n");
	(void)	fprintf (stdout,
		"        lpsystem -l [ system-name ...]\n");
	(void)	fprintf (stdout,
		"\nTo remove a system:\n");
	(void)	fprintf (stdout,
		"        lpsystem -r system-name ...\n");
	(void)	fprintf (stdout,
		"\nTo get the TCP/IP address for the local port-monitor:\n");
	(void)	fprintf (stdout,
		"        lpsystem -A\n");
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#ifdef	__STDC__
static	void
formatsys (SYSTEM * sys)
#else
static	void
formatsys (sys)
SYSTEM	*sys;
#endif
{
	(void)	printf ("System:                     %s\n", sys->name);
	(void)	printf ("Type:                       %s\n",
		(sys->protocol == S5_PROTO ? NAME_S5PROTO : NAME_BSDPROTO));
	if (sys->timeout == -1)
		(void)	printf ("Connection timeout:         never\n");
	else
		(void)	printf ("Connection timeout:         %d minutes\n",
			sys->timeout);
	if (sys->retry == -1)
		(void)	printf ("Retry failed connections:   no\n");
	else
		(void)	printf (
			"Retry failed connections:   after %d minutes\n",
			 sys->retry);
	(void)	printf ("Comment:                    %s\n",
		sys->comment == NULL ? "none" : sys->comment);
	(void)	printf ("\n");
}
/*==================================================================*/
