/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/testmain.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)testmain.c	3.6	LCC);	/* Modified: 16:26:38 11/27/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#define	TI_CHECK_INTERVAL	5	/* log a check every 5 packets */
#define	TI_DUP_WINDOW		128	/* size of the window to forget about dups */

/*                       PC-INTERFACE  Test Main                           */
#include	"pci_types.h"
#include	<errno.h>
#include	<pwd.h>
#include	<fcntl.h>

#include	<string.h>
#include	<signal.h>

#ifdef	ETHNETPCI
#ifdef	ETHLOCUS
#include        <eth.h>			/* LOCUS Ethernet structs/constants */
#endif	/* ETHLOCUS */

#ifdef	ETH3BNET
#include <ni.h>			/* AT&T Ethernet structs/constants */
#endif	/* ETH3BNET */

#endif /* ETHNETPCI */

#ifdef RS232PCI
#ifdef	XENIX
#include <ioctl.h>
#endif	/* XENIX */
#include <termio.h>			/* TTY structs/constants */
#endif /* RS232PCI */

#include <stat.h>
#include "const.h"
#include "flip.h"

#define	pcDbg(dbgArgs)	debug(0x40, dbgArgs)

/*			    External Functions 				*/

extern	struct	passwd
	    *getpwuid(),		/* Points to user's password entry */
	    *getpwnam();		/* Returns pointer into password file */

extern char
		*crypt(),		/* Unix password encryptor */
		*nAddrFmt();		/* Format a net address for printing */

extern	int
	   (*signal())();


int
	    sig_catcher();		/* Signal catching routine */



/*			Global Variables & Structures			*/


int	
	    bridge = INITIALIZED,	/* Indicates bridge access is active */
	    swap_how,			/* Byte order for auto-sense flipping */
	    emulator = INITIALIZED,	/* Indicates term emulation is active */
	    versNum,			/* Version number of this server */

#ifdef ETHNETPCI
	    netdesc,			/* File descriptor of ethernet device */
#endif /* ETHNETPCI */
#ifdef	TCA
	    tcadesc = -1,		/* TCA descriptor */
#endif	/* TCA */
	    pipedesc,			/* File descriptor of control pipe */
	    ptydesc,			/* File descriptor of pseudo TTY */
	    length,			/* Length of last configuration frame */
	    accepted = -1,		/* Number bytes accepted on PTY write */
	    files_open,			/* Number of files open on Bridge PC */
	    brg_seqnum,			/* Last Bridge frame sequence number */
	    ti_seqnum,			/* Last emulator input seq number */
	ti_check,			/* counter to check input characters */
	    termoutpid,			/* Id of terminal output process */
	    descriptors[2];		/* File descriptors associated w/pipe */

char	
#ifdef	MULT_DRIVE
	    wDirs[NDRIVE][MAX_CWD],	/* Current working directory names */
#else	/* ~MULT_DRIVE */
	    cwd[MAX_CWD],		/* Contains current working directory */
#endif	/* MULT_DRIVE */
	    ptydevice[MAX_FNAME + 1],	/* Contains line in /etc/brgptys file */
	    cntrlpty[MAX_FNAME + 1],	/* Name of controlling PTY device */
	    slvpty[MAX_FNAME + 1],	/* Name of slave PTY device */
	    copyright[] =		"PC BRIDGE TO UNIX FILE SERVER\
COPYRIGHT (C) 1984, LOCUS COMPUTING CORPORATION.  ALL RIGHTS RESERVED.  USE OF\
THIS SOFTWARE IS GRANTED ONLU UNDER LICENSE FROM LOCUS COMPUTING CORPORATION.\
ANY UNAUTHORIZED USE IS STRICTLY PROHIBITED.";

struct  ni2 ndata;                      /* Ethernet device header structure */
					/* (dummy struct when not ethernet) */
#ifdef ETHNETPCI
struct  ni2 ntmp;                       /* Ethernet device header structure */

#ifdef	ETH3BNET
NI_PORT
	    lp;				/* Ethernet ioctl control block */
#endif	/* ETH3BNET */
#ifdef	ETHLOCUS
struct lpstatus 
	    lp;				/* Ethernet ioctl control block */
#endif	/* ETHLOCUS */
#endif	/* ETHNETPCI */

#ifdef RS232PCI
struct	termio
	    ttysave,			/* Saves TTY modes for later */
	    ttymodes;			/* TTY modes structure */
#endif /* RS232PCI */

static struct	input				/* Input buffer for RS-232 */
	    in;

static struct	output
	    out;			/* Output buffer for RS-232 */

extern	int
	    errno;			/* Error return from system calls */

char
	*logStr;			/* Log level string (shows in ps) */


#define	NULLstat	(struct stat *)0

struct {
	char *file;
	char *path;
} t[] = {
	{"/tmp/evedir/longdirname/afile","/u/eve"},
	{0,0}
};

/*
 *  main()  arguments:  (any order)
  -ny   ethernet network descriptor y.        : only when ETHERNET
  -pz   3B2 ni driver logical port number z.  : only when for 3b2
  -Dn   debug level n          default 0
#ifdef TCA
  -tdev talk on file "dev"
#endif /* TCA */
 */

#ifdef AUTOLOGIN
int atf = 0;
char auto_uname[15]= {0,};
char auto_pw[15];
void do_login();
#endif /* AUTOLOGIN */

char *myname;

main(argc, argv)
int argc;
char *argv[];
{
    register int 
	len,			/* Temporary length of string */
	status,			/* Return value from system call */
	i;			/* Loop counter for loading source addresses */
	int length;
#ifdef	RS232PCI
    char
	ptybuf[10];		/* Character buffer for terminal emulation */
#endif

    register struct passwd
	*pptr;			/* Pointer to entry in password file */

    int
	portnum;

    char
	c;			/* Character for writing onto PIPE */

    FILE 
	*fptr;			/* File pointer into PTY configuration table */

    char
	fileName[MAX_PATH],	/* Logfile temporary name */
	*ptr,			/* Pointer to character array */
	*txtptr,		/* Pointer into input text buffer */
	*cryptPass,		/* Return from crypt(3) */
#ifdef TCA
	*tcaname,		/* name of tca device to talk on */
#endif /* TCA */
	fname[120];             /* a file name */

char
	errArg[8],		/* Error descriptor */
	pipeArg[8],		/* Ack pipe descriptor */
	ptyArg[8],		/* Pty descriptor */
	debugArg[8],		/* Debug level */
	swapArg[8],		/* Byte swapping code */
	descArg[8],		/* Network (eth or 232) descriptor */
	naddrArg[32];		/* Network address (eth) */
int
	argN;			/* Current argument number */
char
	*arg;			/* Current argument */

	myname = argv[0];
	if (*myname == '\0')
		myname = "unknown";

#ifdef TCA

#ifndef	NFILE
#define	NFILE	20
#endif	/* !NFILE */

	/* Close all open files */
	for(i = 0; i < NFILE; i++)
		close(i);

#endif /* TCA */

	/* Extract version number of this server from its name */
	versNum = getVers(argv[0]);

	/* Decode arguments */
	for (argN = 1; argN < argc; argN++) {
		arg = argv[argN];

		if (*arg != '-')
			continue;

		switch (arg[1]) {
		case 'D':               /* Log level */
			logStr = &arg[2];
			dbgSet(strtol(logStr, NULL, 16));
			break;

#ifdef	ETHNETPCI
		case 'n':		/* Network descriptor */
			netdesc = atoi(&arg[2]);
			break;
#endif	/* ETHNETPCI */

#ifdef TCA
		case 't':
			tcaname = arg+2;
			break;
#endif /* TCA */
		case 'p':		/* 3B2 ni driver logical port number */
			portnum = atoi(&arg[2]);
			break;
		}
	}

    if (dbgCheck(~0))
	logOpen(DOSSVR_LOG, getpid());

    signal(SIGTERM, sig_catcher);       /* Catch signal from pcidaemon */
    signal(SIGINT, sig_catcher);        /* Catch signal from tty (break) */
    signal(SIGHUP, sig_catcher);        /* Catch signal from tty (disconnect) */
    signal(SIG_DBG1, sig_catcher);      /* Catch signal to toggle logs */
    signal(SIG_DBG2, sig_catcher);      /* Same */
    signal(SIG_CHILD, sig_catcher);     /* Catch signal if pcitermout dies */

    while (getstuff(file,path)) {
	    maptest(file,path,mappedname);
	    unmaptest(file,path,mappedname);
	    if (strcmp(file,mappedname) != 0) {
		    printf("ERROR:file %s path %s unmapped %s\n",
			    file,path,mappedname);
	    }
    }
}

maptest(file,path,mappedname)
char *file;	/* name to be mapped */
char *path;	/* directory path to name */
char *mappedname;	/* place to put results */
{
	register int i;

	strcpy(mappedname,file);
	printf("maptest:mappedname %s,path %s\n",mappedname,path);
	i = mapfilename(path,mappedname);
	printf("maptest:got %d, path %s, mappedname %s\n",
		i,path,mappedname);
}
unmaptest(file,path,mappedname)
char *file;	/* name to be mapped */
char *path;	/* directory path to name */
char *mappedname;	/* place to put results */
{
	register int i;
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

	printf("unmaptest:mappedname %s,path %s\n",mappedname,path);
#ifdef HIDDEN_FILES
	i = unmapfilename(path,mappedname,&attrib);
#else
	i = unmapfilename(path,mappedname);
#endif /* HIDDEN_FILES */
	printf("unmaptest:got %d, path %s, mappedname %s\n",
		i,path,mappedname);
}

getstuff(file,path)
char *file;
char *path;
{
	register char *p;
	register int c;

	p = path;
	while(((c=getchar()) != EOF) && (c != ',')) *p++=c;
	if (c == EOF) return(0);
	*p = 0;

	p = file;
	while(((*p=getchar()) != EOF) && (*p != '\n')) p++;
	if (c == EOF) return(0);
	*p = 0;
	return(1);
}

sig_catcher(signo)
int signo;
{
	log("got sig %d\n",signo);
}

stopService()
{       log("Stop service called\n");
	exit(0);
}
childExit()
{
	log("childExit called\n");
}
