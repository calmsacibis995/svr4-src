/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_consvr.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_consvr.c	3.43	LCC);	/* Modified: 16:27:28 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*		PC Interface Connection Server			*/

#include	<stdio.h>
#include	<ctype.h>
#include	<unistd.h>	/* for possible #define _POSIX_VERSION */

#if	defined(SYS5_4)
#include	<sys/filio.h>
#endif
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#if	defined(BERK42FILE) && !defined(LOCUS) && !defined(DGUX)
#include	<fstab.h>
#include	<mtab.h>
#include	<sys/stat.h>
#include	<fs.h>
#include 	<wait.h>
#endif	  /* defined(BERK42FILE)  && !defined(LOCUS) && !defined(DGUX) */

#ifdef	MICOM
#include	<interlan/il_ioctl.h>
#endif

#ifdef	ATT3B2
#include	<sys/inetioctl.h>
#endif 

#if	!defined(BERKELEY42) || defined(AIX_RT) && !defined(LOCUS)
#include	<termio.h>
#endif	/* !defined(BERKELEY42) || defined(AIX_RT) && !defined(LOCUS) */
#ifdef  SYS19
#include	<ioctl42.h>
#else
#ifdef 	RIDGE
#include	<bsdioctl.h>
#else
#include	<sys/ioctl.h>
#endif	  	/* RIDGE */
#endif		/* SYS19 */

#include	"pci_types.h"
#include	"flip.h"

#define UNIX
#include	"serial.h"
#define PRINT_CS  /**/

#define	PNAME		"pciconsvr"

char
	CopyRight[] =
"PC INTERFACE UNIX FILE SERVER.  COPYRIGHT (C) 1984, LOCUS COMPUTING CORP.\n\
ALL RIGHTS RESERVED.  USE OF THIS SOFTWARE IS GRANTED ONLY UNDER LICENSE\n\
FROM LOCUS COMPUTING CORPORATION.  UNAUTHORIZED USE IS STRICTLY PROHIBITED.\n";


extern char *
	nAddrFmt();		/* Format the net address */

extern int
	xmtPacket(),		/* Writes a packet to the net device */
	rcvPacket();		/* Receives a packet from the net device */

#ifndef SIGNAL_KNOWN
int    
	(*signal())();
#endif

void
	cleanUp();		/* Final cleanup */

typedef struct pcNex	pcNex;

/* Forward declarations for conTable indexing functions */
void			addCTIndex();
void			rmvCTIndex();
struct pcNex		*getCTAddr();
struct pcNex		*getCTSerial();
struct pcNex		*getCTSvrPID();
struct pcNex		*htAdd();
struct pcNex		*htRmv();
struct pcNex		*htGet();
void			htInit();
void			initConTable();
int			allocCTSlot();
void			freeCTSlot();

void			killSvr();

static struct pcNex
	conTable[MAX_PORTS];	/* List of current connections */

static struct ni2
	mapSvrHeader,		/* Only one map server address in IP */
	myHeader;		/* This connection server's address */

#ifdef	LOCUS
extern char
	*sfsname();
#endif	/* LOCUS */

extern int
	errno;

#define	SCHED_INTERVAL	30	/* Check connections this often */
#define	PROBE_TIMEOUT	5	/* Max missed probes before disconnect */

#define	HASH_COMMENT	'#'

int
	noDisconnect,		/* Don't disconnect on probe timeout */
	ConSvrPid;		/* The PID of the Consvr */
static	int	line_pos = 0;	/* for feature reading */

long
	dosLogCtl;		/* Dos Server debug level */

char
	*netDev = NETDEV,	/* Network interface device */
	fqNetDev[32],		/* Fully qualified netDev name */
	*logArg,		/* Debug argument string */
	*dosLogArg,		/* Dos server debug argument */
	*attr_String = "F0";	/* pointer to feature string */

char *myname;

extern long
	lseek();

netIntFace	intFaceList[MAX_NET_INTFACE];

int	numIntFace = 0;			/* The # of configured interfaces */


main(argc, argv)
int	argc;
char	*argv[];
{
char
	*malloc(), *realloc(),		/* gets ram */
	*strchr(), *strcpy(),
	*fp0, *fp1,
	*readFeatureFile();
void
	catchSignal();		/* Signal catching routine */
int
	netdesc = -1,		/* Network descriptor */
#if	defined(SecureWare)
	luid,			/* value returned by getluid() */
#endif
	argN;			/* Current argument number */
char
	*arg;			/* Current argument */
register char *i;		/*temp for sin_zero init */
	int t;			/* file descriptor from open of /dev/tty */

	/* Save Consvr PID to check after forking for Dossvr */ 
	ConSvrPid = getpid();

	myname = argv[0];
	if (*myname == '\0')
		myname = "unknown";

	/* Open logfile to record unexpected occurences */
	logOpen(CONSVR_LOG, 0);

	for (argN = 1; argN < argc; argN++) {
		arg = argv[argN];

		if (*arg != '-') {
			fprintf(stderr, "pciconsvr: Bad usage\n");
			exit(1);
		}

		switch (arg[1]) {
		case 'x':
			noDisconnect = 1;
			break;

		case 'F':
			attr_String = readFeatureFile(&arg[2]);
			break;

		case 'D':
			dbgSet(strtol(&arg[2], NULL, 16));
			logArg = &arg[2];
			break;

		case 'L':
			dosLogCtl = strtol(&arg[2], NULL, 16);
			dosLogArg = &arg[2];
			break;


		case 'N':			/* Network device */
			netDev = &arg[2];
			if (netDev == '\0')
				netDev = NETDEV;
			else if (*netDev != '/') {
				sprintf(fqNetDev, "/dev/%s", netDev);
				netDev = fqNetDev;
			}
			break;

		case 'n':		/* Network descriptor */
			netdesc = atoi(&arg[2]);
			log("netdesc on %d\n",netdesc);
			break;

		case 'I':		/* Interface list */
			numIntFace = get_interface_list(&arg[2]);
			break;
		}
	}

	/* Standard in/out/err not needed */
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	log("PC Interface Connection Server Start\n");

#if	defined(SecureWare)
	if ((luid = getluid()) != -1 || errno != EPERM)
		fatal("luid already set to %d\n", luid);
#endif

	if (netdesc >= 0)
		netUse(netdesc);
	else if (netUse(netOpen(netDev, PCI_CONSVR_PORT)) < 0)
		fatal("Can't open network - Bye\n");


	/* Enable signals */
	signal(SIG_CHILD,  catchSignal);	/* Server death */
	signal(SIG_DBG1, catchSignal);  /* Change debug level */
	signal(SIG_DBG2, catchSignal);  /* Change dos server debug level */
	signal(SIGTERM, catchSignal);	/* Go Away */

	/* Detach myself from my invoker */
#if	defined(_POSIX_VERSION) && !defined(CTSPC)
	setsid();
#elif	defined(SYS5)
	setpgrp();
#else
	setpgrp(0,getpid());

	/* disassociate us from the old control tty (if any) */

	if ((t = open("/dev/tty", 2)) >= 0) {
		ioctl(t, TIOCNOTTY, 0);
		close(t);
	}
#endif

	/* Initialize the connection table */
	initConTable();

	/*
	   This is where the map servers listen out
	   Create header for each type of map server
	*/
	if (numIntFace == 0)
		netaddr(intFaceList, &numIntFace, 0);
	mapSvrHeader.dst_sin.sin_family = AF_INET;
	mapSvrHeader.dst_sin.sin_port = htons(PCI_MAPSVR_PORT);
#define destin mapSvrHeader.dst_sin.sin_zero
	for (i = destin; i < destin + 8; i++)
 	    *i = 0;
#undef destin

	toMapSvr(CONSVR_HERE);
	schedOn();

	/* Everything is initialized - Start main connection service loop */
	connectService();

	log("Connection service ended - Bye\n");
	exit(0);
}


/*
   readFeatureFile: read the feature file
*/

static int
gchar(fp)
FILE	*fp;
{
	int	cchar;

	cchar = getc(fp);
	if (cchar >= 0) {
		if (cchar == '\n')
			line_pos = 0;
		else
			line_pos ++;
	}
	return(cchar);
}

static int
eatLine(fp)
FILE	*fp;
{
	int	cchar;	/* current character */

	do {
		cchar = gchar(fp);
		if (cchar < 0)
			return(0);
	} while (cchar != '\n');

	return(cchar);
}

int
getFeature(fp, buf)
FILE	*fp;
char	*buf;
{
	int	cchar;	/* current character */
	register int i;


	do {
	   cchar = gchar(fp);		/* get next character */
	   if (cchar < 0)
		return(0);		/* stop reading on error */
	   if ((cchar == HASH_COMMENT) && (line_pos == 1)) {
		if (!eatLine(fp))	/* error */
			return(0);
		cchar = ' ';		/* make this a space */
	   }
	} while (isspace(cchar));

	buf[0] = cchar;
	cchar = gchar(fp);
	if ((cchar < 0) || (isspace(cchar)))
			return(-1);
	buf[1] = cchar;

	buf[2] = '\0';
	return(1);
}


char *
readFeatureFile(fn)
char	*fn;
{
	FILE	*fp;
	char	*fb, *malloc(), *realloc(), fbuf[3];
	int	slen;
	int	i, j;

	fp = fopen(fn, "r");

	if (!fp) {
		fprintf(stderr, "%s: error opening feature file %s\n",
		   PNAME, fn);
		exit(1);
	}

	fb = malloc(MAX_FSTR_LEN);
	if (!fb) {
		fprintf(stderr, "%s: error allocating RAM for feature string\n",
		   PNAME, fn);
		fclose(fp);
		return(NULL);
	}

	strcpy(fb, "F0");
	slen = strlen(fb);

	while (i = getFeature(fp, fbuf)) {
		if (i < 0) {	/* error */
			fprintf(stderr, "%s: error in feature file %s\n",
			    PNAME, fn);
			fclose(fp);
			free(fb);
			return(NULL);
		}

		/* room to add this? */

		if ((slen + strlen(fbuf)) >= (size_t)MAX_FSTR_LEN) {
		   fprintf(stderr,
		     "%s: error allocating RAM for feature string\n",
	   		PNAME, fn);
			fclose(fp);
			free(fb);
			return(NULL);
		}

		strcat(fb, fbuf);
		slen += strlen(fbuf);
	}
	fclose(fp);
	return(realloc(fb, strlen(fb) + 1));
}

/*
   connectService: Create, maintain and delete bridge connections
*/

connectService()
{
struct input
	inPacket;		/* Input buffer */
struct output
	outPacket;		/* Output buffer */
int
	i,			/* loop variable */
	flipCode;		/* How to flip bytes in output packets */

	for (;;) {
		flipCode = rcvPacket(&inPacket);
		outPacket.hdr.res = SUCCESS;
		outPacket.hdr.stat = NEW;
		outPacket.hdr.seq = inPacket.hdr.seq;
		outPacket.hdr.req = inPacket.hdr.req;
		outPacket.pre.select = DAEMON;
		outPacket.hdr.t_cnt = 0;
#ifdef	ETHPORTS
		outPacket.net.head[PCIPORTOFF] = PCI_CONSVR_PORT;
#endif	/* ETHPORTS */

		/* Store address of destination PC */

		nAddrCpy(myHeader.dst, inPacket.net.src);
#ifdef EXCELAN
		myHeader.dst_sin.sin_family = AF_INET;	/* Excelan needs this. */
#endif	/* EXCELAN */

		/* Is packet destined for DAEMON stream? */
		if (inPacket.pre.select != DAEMON) {
			log("Bad sel %d\n", inPacket.pre.select);
			outPacket.hdr.res = INVALID_FUNCTION;
			xmtPacket(&outPacket, &myHeader, flipCode);
			continue;
		}

		switch (inPacket.hdr.req) {
		case CONNECT:
			newConnection(&inPacket, &outPacket, flipCode);
			break;

		case PROBE:
			conProbe(inPacket.net.src, &outPacket, flipCode);
			break;

		case SECURITY_CHECK:
			securityCheck(&inPacket, flipCode);
			break;

		case DISCONNECT:
			disConnect(&inPacket, &outPacket, flipCode);
			break;

#ifndef	DONT_START_GETTY
		case S_LOGIN:
			s_login(&inPacket, &outPacket, flipCode);
			break;

		case K_LOGIN:
			k_login(&inPacket, &outPacket, flipCode);
			break;
#endif   /* DONT_START_GETTY */

		case GET_SITE_ATTR:
			sendSiteAttr(&inPacket, &outPacket, flipCode);
			break;

		default:
			log("Bad req %d\n", inPacket.hdr.req);
			outPacket.hdr.res = INVALID_FUNCTION;
			xmtPacket(&outPacket, &myHeader, flipCode);
		}
	}
}


/*
   newConnection: Establish new PC Interface service
*/

newConnection(conPacket, rspPacket, flipCode)
register struct input
	*conPacket,			/* Packet requesting connection */
	*rspPacket;			/* Response packet */
long
	flipCode;			/* Flipping code */
{
int
	svrDesc,			/* Servers network port descriptor */
	conNum,				/* Servers port number */
	newSvrPid;			/* New server pid */
#ifdef	VERSION_MATCHING	
register struct connect_text
	*connText;			/* pointer to text area of pkt. */
#endif	/* VERSION_MATCHING */
int
	savenetDesc;			/* holds previous net desc  */
char
	svrProg[64],			/* Name of server program */
	debugArg[16],			/* Debug level argument */
	netArg[16],			/* Network descriptor number */
	portArg[16],			/* Port number */
	netDev[64];			/* Network device name */
register struct pcNex
	*ctSlot;			/* conTable table slot (old AND new) */

	log("Connect req from %s\n", nAddrFmt(conPacket->net.src));

	/* Look up requestor's address in conTable */
	if ((ctSlot = getCTAddr(conPacket->net.src)) != 0) {
		/* If old server from this client still exists, kill it */
		log("killing off old (but same) client\n");
		killSvr(ctSlot,  SIGTERM);
	}

	/* Allocate a new network endpoint/socket/port/etc */
	savenetDesc = netSave();
	if ((conNum = newPort(&svrDesc)) < 0) {
		log("...No net ports\n");
		rspPacket->hdr.res = FAILURE;
		rspPacket->hdr.b_cnt = 0;
		xmtPacket(rspPacket, &myHeader, flipCode);
		return;
	}
	ctSlot = &conTable[conNum];

	netUse(svrDesc);

#ifdef VERSION_MATCHING
	connText = (struct connect_text *) conPacket->text;
	log("PC version: %d.%d.%d \n", (int) connText->vers_major,
	    (int) connText->vers_minor, (int) connText->vers_submin);
#endif	/* VERSION_MATCHING */

    if (conPacket->hdr.versNum == 0) {	/* do only for "standard" startups */

#ifdef VERSION_MATCHING
	/* get version into server name */
	sprintf(svrProg, "%s/%d/%d/%d/%s", PCIDIR,
		(int) connText->vers_major, 
		(int) connText->vers_minor,
		(int) connText->vers_submin, PCIDOSSVR);
	if (access(svrProg, 0x01) < 0) {   /* check for correct file access */
		rspPacket->hdr.res = FAILURE;
		rspPacket->hdr.b_cnt = 1; /* version matching failure. */
		xmtPacket(rspPacket, &myHeader, flipCode);
		log("cannot access svrProg: %s\n", svrProg); /* JD */
		return;
	}
#else	/* !VERSION_MATCHING */
	sprintf(svrProg, "%s/%s", PCIDIR, PCIDOSSVR); 
#endif	/* VERSION_MATCHING */
#ifdef IBM_SERCHK
	if (serchk(conPacket,flipCode) == 0) { /* bad serial# for version */
		rspPacket->hdr.res = FAILURE;
		rspPacket->hdr.b_cnt = 1; /* version matching failure. */
		xmtPacket(rspPacket, &myHeader, flipCode);
		return;
	}
#endif /* IBM_SERCHK */
    }

	if ((newSvrPid = fork()) < 0) {
		log("...Can't fork\n");
		rspPacket->hdr.res = FAILURE;
		rspPacket->hdr.b_cnt = 0;
	}
	else if (newSvrPid == 0) {
		if (conPacket->hdr.versNum != 0)	/* cmd line "-s<num>" */
			sprintf(svrProg, "%s/%s%d", PCIDIR, PCIDOSSVR,
				conPacket->hdr.versNum);
		sprintf(debugArg, "-D%04x", dosLogCtl);
		sprintf(netArg, "-n%d", svrDesc);
		sprintf(portArg, "-p%d", conNum);
		logClose();
		close(savenetDesc);	/* disconnect from parent's port */

		execl(svrProg, strrchr(svrProg, '/') + 1, debugArg, netArg,
			portArg, (char *)0);
		/* All dressed up, no place to go */
		fatal("Can't exec DOS server `%s': %d\n", svrProg, errno);
	} else {
		/* Set-up the parent: build and store connection table entry */
		nAddrCpy(ctSlot->pcAddr, conPacket->net.src);
		serialCpy(ctSlot->pcSerial, conPacket->text);
		ctSlot->svrPid = newSvrPid;
		ctSlot->emPid = 0;
		ctSlot->probeCount = 0;
		ctSlot->indict = 0;

		/* Index the new conTable entry */
		addCTIndex(ctSlot);

		/* Tell PC whom to talk to */
		rspPacket->hdr.res = SUCCESS;
	}

	xmtPacket(rspPacket, &myHeader, flipCode);

	netUse(savenetDesc); /* restore our port */
	close(svrDesc);	/* close child's port */
}


/*
   conProbe: Keep connection alive
	     Check for copy protection violators
*/

conProbe(conAddr, rspPacket, flipCode)
char
	conAddr[];
struct input
	*rspPacket;
{
register struct pcNex
	*ctSlot;			/* conTable slot for probing client */

	log("Probe from %s\n", nAddrFmt(conAddr));

	if ((ctSlot = getCTAddr(conAddr)) == 0)
		return;

	ctSlot->probeCount = 0;
	/* 
	   If there is a connection here, we want to kill
	   any new one trying to use the same serial no.
	*/
	if (ctSlot->indict) {
		rspPacket->pre.select = UNSOLICITED;
		rspPacket->hdr.req = PC_CRASH;

		/* Kill the pc without a connection  here */
		nAddrCpy(myHeader.dst, ctSlot->pcAddr2);
		xmtPacket(rspPacket, &myHeader, flipCode);
		log("PC_CRASH to %s\n", nAddrFmt(ctSlot->pcAddr2));
		ctSlot->indict = 0;
	}
}


/*
   disConnect: Destroy connection
*/

disConnect(disPacket, rspPacket, flipCode)
struct input
	*disPacket,
	*rspPacket;
long
	flipCode;
{
struct pcNex
	*ctSlot;			/* Slot of session to disconnect */
	register int savedSvrPid;

	log("Discon req from %s\n", nAddrFmt(disPacket->net.src));

	ctSlot = getCTAddr(disPacket->net.src);
	if (ctSlot != 0) 
		killSvr(ctSlot, SIGTERM);
	else
		log("...Unknown connection\n");
	rspPacket->hdr.res = NO_SESSION;
	rspPacket->hdr.req = disPacket->hdr.req;
	rspPacket->hdr.seq = disPacket->hdr.seq;
	rspPacket->hdr.stat = NEW;
	rspPacket->hdr.t_cnt = 0;
	rspPacket->pre.select = DAEMON;
	xmtPacket(rspPacket, &myHeader, flipCode);
}


/*
	sendSiteAttr() - send attributes of site
*/

sendSiteAttr(inPack, outPack, flipCode)
struct input
	*inPack,
	*outPack;
long
	flipCode;
{

	log("Site attr req from %s\n", nAddrFmt(inPack->net.src));

	outPack->hdr.res = SUCCESS;
	outPack->hdr.req = inPack->hdr.req;
	outPack->hdr.seq = inPack->hdr.seq;
	outPack->hdr.stat = NEW;
	outPack->hdr.t_cnt = strlen(attr_String) + 1;
	strcpy(outPack->text, attr_String);
	outPack->pre.select = DAEMON;
	xmtPacket(outPack, &myHeader, flipCode);
}

/*
   securityCheck:

   The two things that we are trying to catch are: multiple copies
   of the same pci diskette being run at the same time and attempts
   to foil this protection by modifying the unique information recorded
   on each diskette.  We will detect multiple copies even if the servers
   are on different machines.

   The tests are performed as follows:
   The map server takes care of invalid serial numbers so that all that
   is handled here is multiple uses of a copy.
   The only thing to check is whether there is a current connection
   already for this serial number.  If not do nothing.  If there is then 
   we set the indict member in the connection table  entry for the pc with
   this number.  The net address of the new pc is added to the connection
   table as pcAddr2.  If a probe arrives on the old connection we can
   halt both or one of them.
*/

securityCheck(conPacket, flipCode)
register struct input
	*conPacket;
long
	flipCode;
{
register struct pcNex
	*ctSlot;			/* conTable entry relevant to check */
struct seriEthSt
	 *seriEthP;

	log("Security check from %s\n", nAddrFmt(conPacket->net.src));

	/* Get pointer to serialization structure in security check request */
	seriEthP = (struct seriEthSt *)conPacket->text;

	/* Get connection with this serial number */
	if ((ctSlot = getCTSerial(&seriEthP->serNum)) == 0)
		return;

	/* Potential duplicate bridge if addresses don't match */
	if (!nAddrEq(ctSlot->pcAddr, seriEthP->serAddr)) {
		/* Indict and save address of bad guy */
		ctSlot->indict = TRUE;
		nAddrCpy(ctSlot->pcAddr2, seriEthP->serAddr);
#ifdef	VERBOSE_LOG
		log("Security indict from %s \n", nAddrFmt(seriEthP->serAddr));
#endif
	}
}


/*
   closeShop: Tell map servers this site is closing down
*/

closeShop()
{
	/* Stop the clock */
	alarm(0);

	/* Send bye-bye message twice to make sure everyone hears */
	toMapSvr(CONSVR_BYE);
	sleep(5);
	toMapSvr(CONSVR_BYE);
}

/*
   toMapSvr: Send message to Map Server multicast address; text of
		message is always a nameAddr for this consvr;
		mapSvrReq is the request code passed.
*/

toMapSvr(mapSvrReq)
int
	mapSvrReq;
{
	int	i;			/* Loop counter */
	struct nameAddr	*myName;
	struct output	outPacket;

	myName = (struct nameAddr *) outPacket.text;

	/* Store system name and network address */ 

	memset(myName, 0, sizeof(struct nameAddr));
	setnameAddr(myName);

	outPacket.hdr.req = mapSvrReq;
	outPacket.hdr.t_cnt = sizeof(struct nameAddr);

	for (i = 0; i < numIntFace; ++i) {
		memcpy(myName->address, &(intFaceList[i].localAddr), SZHADDR);
		memcpy(&(mapSvrHeader.dst_sin.sin_addr.s_addr),
			&(intFaceList[i].broadAddr), SZHADDR);
		if ((xmtPacket(&outPacket, &mapSvrHeader, NOFLIP) < 0)
		    && (errno == EACCES)) {
				xmtPacket(&outPacket, &mapSvrHeader, NOFLIP);
		}
	}
}


/*
   schedOn: Start periodic monitoring activities
*/

schedOn()
{
void
	catchSignal();

	signal(SIGALRM, catchSignal);
	alarm(SCHED_INTERVAL);
}


/*
   newPort:  Allocate a unique ethernet port number.
*/

int
newPort(toNetDesc)
int *toNetDesc;			/* Put net device descriptor here */
{
register int
	i,
	netDesc;			/* File descriptor of net dev */
char
	netName[64];			/* Name of net device */

	if ((i = allocCTSlot()) >= 0) {
		if ((netDesc = netOpen(netDev, ANY_PORT)) < 0) {
			log("...Can't open net for dossvr; errno %d\n", errno);
			return -1;
		}
		*toNetDesc = netDesc;
		return i;
	}
	return -1;
}


/*
   cleanUp: cleans up before exiting.
*/

void
cleanUp()
{
register struct pcNex
	*conScan;
	register int savedSvrPid;

	signal(SIG_CHILD, SIG_IGN);

	for (conScan = conTable; conScan < &conTable[MAX_PORTS]; conScan++) {
		if (conScan->svrPid > 0)
			killSvr(conScan, SIGTERM);
	}
}


/*
   sched: Periodic housekeeping
*/

sched()
{
	ageConnects();
	toMapSvr(CONSVR_HERE);
	alarm(SCHED_INTERVAL);
}


/*
   ageConnects: Age connection table
*/

ageConnects()
{
register struct pcNex
	*conScan;
	register int savedSvrPid;

	/*
	   Check all connections to determine if any have NOT
	   received PROBE messages within the last period.  If
	   a connection misses two PROBE messages in a row, delete it!
	*/
	for (conScan = conTable; conScan < &conTable[MAX_PORTS]; conScan++) {
		/* Is there a connection on this port? */
		if (conScan->svrPid == 0)
			continue;

		/* Probe timeout reached? */
		if (++conScan->probeCount < PROBE_TIMEOUT)
			continue;

		if (noDisconnect) {
			log("Probe timeout - DON'T disconnect %s\n",
				nAddrFmt(conScan->pcAddr));
		} else {
			log("Probe timeout - disconnect %s pid %d\n",
				nAddrFmt(conScan->pcAddr), conScan->svrPid);
			killSvr(conScan, SIGTERM);
		}
	}
}


/*
   catchSignal: Signal catcher.
*/

void
catchSignal(sigNum)
register int
	sigNum;
{
int
	exitPid,
	exitStat;
register struct pcNex	*ctSlot;

	/* If this is not the actual consvr, return; this is a pre-exec'ed
	   dossvr */
	if (getpid() != ConSvrPid)
		return;

	switch (sigNum) {
	case SIG_DBG1:
		if (newLogs(CONSVR_LOG, 0, &dosLogCtl, NULL) & CHG_CHILD) {
			if (dosLogArg != NULL)
				sprintf(dosLogArg, "%04x", dosLogCtl);
			log("dossvr logs %x\n", dosLogCtl);
		} else {
			if (logArg != NULL)
				sprintf(logArg, "%04x", dbgEnable);
			log("my logs %x\n", dbgEnable);
		}
		break;

	case SIG_DBG2:
		dosLogCtl ^= 1;
		if (logArg != NULL)
			sprintf(dosLogArg, "%04x", dosLogCtl);
		log("dossvr logs %x\n", dosLogCtl);
		break;

	case SIGALRM:
		sched();
		alarm(SCHED_INTERVAL);
		break;

	case SIG_CHILD:

#ifdef	BERKELEY42
		while ((exitPid = wait3(&exitStat,WNOHANG,0))> 0) {
			if ((ctSlot = getCTSvrPID(exitPid)) == 0)
				break;
			if (ctSlot->svrPid == exitPid) {
				/* Remove entry from conTable index */
				rmvCTIndex(ctSlot);
				ctSlot->svrPid = 0;
				if(ctSlot->emPid) {
					rmut(ctSlot->emPty);
					kill(ctSlot->emPid, SIGHUP);
					ctSlot->emPid = 0;
				}
			}
#ifndef	DONT_START_GETTY
			if (ctSlot->emPid == exitPid) {
				rmut(ctSlot->emPty);
				ctSlot->emPid = 0;
				exitStat = 0;
			}
#endif   /* DONT_START_GETTY */
			if (ctSlot->svrPid == 0 && ctSlot->emPid == 0)
				freeCTSlot(ctSlot);

			if (exitStat != 0)
				log("%d server exit status %d:%d\n", exitPid,
				((exitStat >> 8) & 0xff), exitStat & 0xff);
		}
#else	/* !BERKELEY42 */
		exitPid = wait(&exitStat);

		if ((ctSlot = getCTSvrPID(exitPid)) == 0)
			break;
		if (ctSlot->svrPid == exitPid) {
			/* Remove entry from conTable index */
			rmvCTIndex(ctSlot);
			ctSlot->svrPid = 0;
			if(ctSlot->emPid)
				kill(ctSlot->emPid, SIGHUP);
		}
#ifndef	DONT_START_GETTY
		if (ctSlot->emPid == exitPid) {
			/* rmut(ctSlot->emPty); */
			ctSlot->emPid = 0;
			exitStat = 0;
		}
#endif   /* DONT_START_GETTY */
		if (ctSlot->svrPid == 0 && ctSlot->emPid == 0)
			freeCTSlot(ctSlot);

		if (exitStat != 0)
			log("%d server exit status %d:%d\n", exitPid,
			((exitStat >> 8) & 0xff), exitStat & 0xff);
#endif	/* !BERKELEY42 */
		break;

	case SIGTERM:
		cleanUp();
		closeShop();
		log("Quit signalled - Bye\n");
		exit(0);
	}

	signal(sigNum, catchSignal);
}

setnameAddr(myName)
struct nameAddr
	*myName;
{
	char *p;
	extern char *myhostname();

static struct hostent  *uName = 0;
extern struct hostent *myhostent();

	p = myhostname();
	strncpy(myName->name, p, sizeof myName->name);
	if (!uName)
		uName = myhostent();
	nAddrCpy(myName->address, uName->h_addr);
}

#ifndef	DONT_START_GETTY
#include <utmp.h>
#if	!defined(BERKELEY42) && !defined(SYS5_4)
extern void setutent(),endutent(),pututline();
extern struct utmp *getutline();
#endif	/* !BERKELEY42 */

struct	utmp wtmp,*ut;
char	wtmpf[]	= WTMP_FILE;
char	utmp[] = UTMP_FILE;
#define SCPYN(a, b)	strncpy(a, b, sizeof (a))
#define SCMPN(a, b)	strncmp(a, b, sizeof (a))

#ifndef BERKELEY42
addut(entry)
char *entry;
{
int entryLength;
char *cp;

	log("addut: parameter - %s", entry);
	strcpy(wtmp.ut_user, "consvr");
	strcpy(wtmp.ut_line, entry);

	/* use the last three bytes of the dev name as the ID field */
	entryLength = strlen(entry);
	if (entryLength > 3)
		cp = entry + entryLength - 3;
	else
		cp = entry;
	strcpy(wtmp.ut_id, cp);

	log("addut: wtmp.ut_id = %s \n", wtmp.ut_id);

	wtmp.ut_pid = getpid();
	wtmp.ut_type = INIT_PROCESS;
	wtmp.ut_time = time((long *) 0);

	pututline(&wtmp);

	endutent();

	return;
}
#endif

rmut(cleanup)
char *cleanup;
{
	register f;
	int found;
	int entryLength;
	char *cp;
	char pty[20];

	found = 0;
#ifdef	BERKELEY42
	f = open(utmp, 2);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof (wtmp)) == sizeof (wtmp)) {
			if (SCMPN(wtmp.ut_line, cleanup) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof (wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof (wtmp));
			found++;
		}
		close(f);
	}
#else	/* !BERKELEY42 */
	strcpy(wtmp.ut_line, cleanup);

	/* use the last three bytes of the dev name as the ID field */
	entryLength = strlen(cleanup);
	if (entryLength > 3)
		cp = cleanup + entryLength - 3;
	else
		cp = cleanup;
	strcpy(wtmp.ut_id, cp);


	wtmp.ut_type = LOGIN_PROCESS;
	setutent();
	if ((ut = getutline(&wtmp)) != (struct utmp *) NULL) {
		time(&(ut->ut_time));
#ifndef SYS5
		SCPYN(ut->ut_host, "");
#endif	/* !SYS5 */

		ut->ut_type = DEAD_PROCESS;

		found++;
		wtmp = *ut;
		pututline(&wtmp);
	}
	endutent();

#endif	/* BERKELEY42 */

	if (found) {
#ifdef	BERKELEY42
		f = open(wtmpf, 1);
#else	/* !BERKELEY42 */
		f = open(wtmpf, O_WRONLY | O_APPEND);
#endif	/* BERKELEY42 */
		if (f >= 0) {
			SCPYN(wtmp.ut_line, cleanup);
			SCPYN(wtmp.ut_name, "");
#ifndef SYS5
			SCPYN(wtmp.ut_host, "");
#endif	/* !SYS5 */
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof (wtmp));
			close(f);
		}
	}
	strcpy(pty, "/dev/");
	strcat(pty, cleanup);
	chmod(pty, 0666);
	chown(pty, 0, 0);
	pty[0] = 'p';
	chmod(pty, 0666);
	chown(pty, 0, 0);

}

#define	ARGVSIZE	20

s_login(in, out, flip)
struct input *in;
struct output *out;
{
	register struct pcNex *ctSlot;
	register int svrPid, i;
	char *argv[ARGVSIZE];		/* args for execv() call on getty */
	char *argp;
	char *loginProg;
	char stdio[30];
	int std_in;
#ifdef	BERKELEY42
	struct sgttyb b;
#else	/* !BERKELEY42 */
	struct termio b;
#endif	/* BERKELEY42 */
	int tt;

#if	!defined(EXL316) && !defined(AIX_RT)
#if defined(CCI) || defined(BERKELEY42)
	static int on = 0;
#else
	static int on = 1;
#endif
#endif	/* !EXL316 and !AIX_RT */

	static char whitestr[] = " \t";		/* whitespace */

	svrPid = in->hdr.fdsc;

	out->hdr.res = SUCCESS;
	out->hdr.stat = NEW;
	out->hdr.req = S_LOGIN;

	/* Look up conTable slot by consvr PID */
	if ((ctSlot = getCTSvrPID(svrPid)) == 0) {
		/* Send error return */
		log("s_login: dossvr unknown\n");
		out->hdr.res = LOGIN_FAILED;
		xmtPacket(out, &myHeader, flip);
		return;
	}

	if(ctSlot->emPid) {
		/* Send successful return */
		log("s_login: login already exists\n");
		xmtPacket(out, &myHeader, flip);
		return;
	}

	strcpy(ctSlot->emPty, in->text);
	if (loginProg = strtok(&in->text[strlen(ctSlot->emPty)+1], whitestr)) {
		/* build argument array for execv() call */
		i = 0;
		if (argp = strrchr(loginProg, '/'))
			argp++;
		else
			argp = loginProg;
		while (argp && i < ARGVSIZE-1) {
			argv[i++] = argp;
			argp = strtok(NULL, whitestr);
		}
		argv[i] = NULL;
	} else {
		/* Send error return */
		log("s_login: bad getty command string\n");
		out->hdr.res = LOGIN_FAILED;
		xmtPacket(out, &myHeader, flip);
		return;
	}

	if(ctSlot->emPid = fork()) {	/* Parent! */
		if(ctSlot->emPid < 0) {
			/* Send error return */
			log("s_login: Can't fork login process\n");
			out->hdr.res = LOGIN_FAILED;
			xmtPacket(out, &myHeader, flip);
			return;
		}
		/* Don't return a packet, let child do it */
		return;
	} else {	/***** CHILD PROCESS *****/

		/* Cancel any alarms */
		alarm(0);

		if (access(loginProg, 1) < 0) {
			/* Send error return */
			log("s_login: can't execute \"%s\"\n", loginProg);
			out->hdr.res = LOGIN_FAILED;
			xmtPacket(out, &myHeader, flip);
			exit(255);
		}

#ifndef BERKELEY42
		addut(ctSlot->emPty);
#endif

		/* Construct name of tty */
		if (ctSlot->emPty[0] == '/')
			strcpy(stdio, ctSlot->emPty);
		else {
			strcpy(stdio, "/dev/");
			strcat(stdio, ctSlot->emPty);
		}

		close(0);
		close(1);
		close(2);

#if	defined(_POSIX_VERSION) && !defined(CTSPC)
		setsid();
#elif	defined(SYS5)
		setpgrp();
#else
		setpgrp(0, getpid());
		/* disassociate us from any controlling tty */
		if ((tt = open("/dev/tty", 2)) >= 0)
		{
			ioctl(tt, TIOCNOTTY, 0);
			close(tt);
		}
#endif

		/* getty re-opens stdin, stdout and stderr, and sets up	*/
		/* tty modes, so we don't need to do it		*/

		if (strcmp(argv[0], "getty")) {
			if ((std_in = open(stdio, 2)) < 0) {
				/* Send error return */
				log("open of %s failed, errno %d\n", stdio, errno);
				out->hdr.res = LOGIN_FAILED;
				xmtPacket(out, &myHeader, flip);
				exit(255);
			}
#ifdef	BERKELEY42
			ioctl(std_in,TIOCGETP,&b);
			b.sg_flags = CRMOD|XTABS|ANYP|ECHO;
			ioctl(std_in,TIOCSETP,&b);
#else	/* !BERKELEY42 */
			ioctl(std_in,TCGETA,&b);
			b.c_iflag &= ~(INLCR | ICRNL | BRKINT);
			b.c_iflag |= IXON;
			b.c_oflag |= OPOST;
			b.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
			b.c_lflag &= ~(ICANON | ECHO | ISIG);
			b.c_cc[VMIN] 	 = '\01';
			b.c_cc[VTIME] 	 = '\0';
			ioctl(std_in,TCSETAW,&b);
#endif	/* !BERKELEY42 */
#if	!defined(EXL316) && !defined(AIX_RT)
			ioctl(std_in, FIONBIO, &on);
#ifdef BERKELEY42
			ioctl(std_in, FIOASYNC, &on);
#endif
#endif	/* !EXL316 and !AIX_RT */
			dup(0);
			dup(0);
		}

		/* Send success packet */
		out->hdr.fdsc = getpid();
		xmtPacket(out, &myHeader, flip);

		/* close all file descriptors above stderr */
		for (i = 3; i < _NFILE; i++)
			close(i);

		execv(loginProg, argv);

#ifndef BERKELEY42
		rmut(stdio);
#endif
		_exit(errno);
	}
}

k_login(in, out, flip)
struct input *in;
register struct output *out;
{
	register struct pcNex *ctSlot;

	/* Build success response */
	out->hdr.res = SUCCESS;
	out->hdr.stat = NEW;
	out->hdr.req = K_LOGIN;

	/* Get connection table slot */
	if ((ctSlot = getCTSvrPID(in->hdr.fdsc)) == 0)
		out->hdr.res = LOGIN_FAILED;
	else {
		if(ctSlot->emPid == 0)
			/* Dosout already gone - return success */
			log("k_login: login already killed\n");
		else {
			/* Getty|login|shell is running - kill it */
			kill(ctSlot->emPid, SIGHUP);
			ctSlot->emPid = 0;
		}
		if (ctSlot->emPty[0])  
			rmut(ctSlot->emPty);
		
		ctSlot->emPty[0] = '\0';
	}

	/* Send reply */
	xmtPacket(out, &myHeader, flip);
}
#endif /* DONT_START_GETTY */

void
killSvr(ctSlot, sigNum)
struct pcNex		*ctSlot;	/* conTable slot of server to kill */
int			sigNum;		/* Signal to send */
{
int			emPID;		/* Stable copy of dosout PID */
int			svrPID;		/* Ditto for consvr */

	/* Remove conTable slot from index so it won't be seen any longer */
	rmvCTIndex(ctSlot);

	/* Kill dossvr process */
	if ((svrPID = ctSlot->svrPid) > 0)
		kill(svrPID, sigNum);

	/* Kill em process, if it exists */
	if ((emPID = ctSlot->emPid) > 0)
		kill(emPID, SIGHUP);

	/* Don't let this go unnoticed */
	log("SIG%d to %d; SIG%d to %d\n", sigNum, svrPID, SIGHUP, emPID);
}


/*	Hash table indexing for conTable[]	*/

/*
	The essentials of the hashing used here are based on the Double
	Hashing described in chapter 16 of Sedgewick's book "Algorithms"
*/

#define	r0	register
#define	r1	register
#define r2	register

typedef int		(*iFuncPtr)();
typedef unsigned	(*uFuncPtr)();
typedef char		*ref;

unsigned int		addrHash1();
unsigned int		addrHash2();
int			addrCmp();

unsigned int		serialHash1();
unsigned int		serialHash2();
int			serialCmp();

unsigned int		svrPIDHash1();
unsigned int		svrPIDHash2();
int			svrPIDCmp();

#define		NIL_HASHENT	-1		/* Empty hash table slot */
#define		DEL_HASHENT	-2		/* Deleted hash table slot */

typedef struct hashTable {
	uFuncPtr	hFunc1;			/* First probe hash function */
	uFuncPtr	hFunc2;			/* Probe increment hash func */
	iFuncPtr	cmpFunc;		/* Key/entry comparison func */
	short		table[CT_HASH_SIZE];	/* Table of conTable indices */
} hashTable;

hashTable	addrHash = {
	addrHash1, addrHash2, addrCmp		/* Net addr hash table */
};

#undef	PID_HASH				/* PID hashing is not right */

#ifdef	PID_HASH
hashTable	svrPIDHash = {
	svrPIDHash1, svrPIDHash2, svrPIDCmp	/* Server PID hash table */
};
#endif

#ifdef	SERIAL_HASH
hashTable	serialHash = {
	serialHash1, serialHash2, serialCmp	/* Serial number hash table */
};
#endif


/* ----- Connection Table Index Functions ----- */

/*
   addCTIndex:			Index a new conTable entry
   rmvCTIndex:			Remove a conTable entry from index
   getCTAddr:			Lookp up a conTable entry by client address
   getCTSerial:			Look up an entry by serial number
   getCTSvrPID:			Look up an entry by consvr PID
*/


void
addCTIndex(ctSlot)
struct pcNex		*ctSlot;	/* Index this conTable slot */
{
	htAdd(&addrHash, ctSlot, ctSlot->pcAddr);
#ifdef	PID_HASH
	htAdd(&svrPIDHash, ctSlot, ctSlot->svrPid);
#endif
#ifdef	SERIAL_HASH
	htAdd(&serialHash, ctSlot, ctSlot->pcSerial);
#endif
}


void
rmvCTIndex(ctSlot)
struct pcNex		*ctSlot;	/* Remove this slot from index */
{
	/* Remove the entry from the indexes */
	htRmv(&addrHash, ctSlot, ctSlot->pcAddr);
#ifdef	PID_HASH
	/* PID index isn't removed so signal code can find conTable entry */
#endif
#ifdef	SERIAL_HASH
	htRmv(&serialHash, ctSlot, ctSlot->pcSerial);
#endif
}


struct pcNex *
getCTAddr(netAddr)
char			*netAddr;	/* Look up connection on this address */
{
	return htGet(&addrHash, netAddr);
}


#ifdef	SERIAL_HASH

struct pcNex *
getCTSerial(serNum)
char			*serNum;	/* Look up connection on this address */
{
	return htGet(&serialHash, serNum);
}

#else	/* +SERIAL_HASH- */

struct pcNex *
getCTSerial(serNum)
r1 char			*serNum;	/* Search for this net address */
{
r0 struct pcNex		*conScan;	/* Scan current connection table */

	/* Look for an existing connection with this serial number */
	for (conScan = conTable; conScan < &conTable[MAX_PORTS]; conScan++) {

		/* Ignore slots that are not in use */
		if (conScan->svrPid == 0)
			continue;

		/* Does serial number match? */
		if (serialEq(conScan->pcSerial, serNum))
			return conScan;
	}
	return (struct pcNex*) 0;
}
#endif	/* -SERIAL_HASH */


#ifdef	PID_HASH

struct pcNex *
getCTSvrPID(svrPID)
int			svrPID;		/* Look up connection on this address */
{
	return htGet(&svrPIDHash, (long)svrPID);
}

#else	/* +PID_HASH- */

struct pcNex *
getCTSvrPID(svrPID)
r1 int			svrPID;		/* Search for this net address */
{
r0 struct pcNex		*conScan;	/* Scan current connection table */

	/* Look for an existing connection with this server PID */
	/* or em pid (pid of getty|login|shell, see s_login() ) */
	for (conScan = conTable; conScan < &conTable[MAX_PORTS]; conScan++) {

		if (conScan->svrPid == svrPID || conScan->emPid == svrPID)
			return conScan;
	}

	return (struct pcNex *) 0;
}
#endif	/* -PID_HASH */


/* ----- Generic Hash Table Functions ----- */

/*
   htAdd:			Add an item to a hash table
   htRmv:			Remove an item from a hash table
   htGet:			Look up an item in a hash table
   htInit:			Initialize a hash table
*/

struct pcNex *
htAdd(hashTab, newNex, keyVal)
r0 hashTable		*hashTab;	/* Hash table */
pcNex			*newNex;	/* Index this connection */
ref			keyVal;		/* Key value (ref or value) */
{
int			htStart;	/* Start slot for hash table search */
r2 int			htScan;		/* Index used in search */
int			htIncr;		/* htScan increment from addrHash2() */
int			hashEnt;	/* hashTab->table[] entry */
r1 pcNex		*ctSlot;	/* Ptr from hash table to conTable */
int			htFree = -1;	/* Free hash table slot */

	/* Scan the hash table looking for entry with matching key */
	htScan = htStart = (*hashTab->hFunc1)(keyVal);
	htIncr = (*hashTab->hFunc2)(keyVal);
	do {
		hashEnt = hashTab->table[htScan];
		ctSlot = &conTable[hashEnt];

		/* An empty slot implies search failure */
		if (hashEnt == NIL_HASHENT) {
			/* If a free slot was detected earlier, use it */
			if (htFree >= 0)
				hashTab->table[htFree] = newNex - conTable;
			/* Otherwise use the current empty slot */
			else
				hashTab->table[htScan] = newNex - conTable;
			return newNex;
		}

		/* Note deleted entries */
		if (hashEnt == DEL_HASHENT) {
			/* Remember first deleted slot */
			if (htFree < 0)
				htFree = htScan;
		} else
			/* If slot is already indexed, return */
			if (newNex == ctSlot)
				return ctSlot;

		/* Go on to next slot */
		htScan = (htScan + htIncr) % CT_HASH_SIZE;
	} while (htScan != htStart);

	/*
	   This only happens if the hash table overflows (or if the
	   hash functions or hash table size are not properly chosen)
	*/
	/* TODO - log an error */
	log("htAdd: Hash Table overflow\n");
	return (struct pcNex *) 0;
}


struct pcNex *
htRmv(hashTab, oldNex, keyVal)
r0 hashTable		*hashTab;	/* Hash table */
pcNex			*oldNex;	/* De-index this connection */
ref			*keyVal;	/* Key value (ref or value) */
{
int			htStart;	/* Start slot for search of conTable */
r2 int			htScan;		/* Index used in search */
int			htIncr;		/* htScan increment from addrHash2() */
int			hashEnt;	/* hashTab->table[] entry */
r1 struct pcNex		*ctSlot;	/* Ptr from hash table to conTable */

	/* Scan the hash table looking for oldNex->pcAddr */
	htScan = htStart = (*hashTab->hFunc1)(keyVal);
	htIncr = (*hashTab->hFunc2)(keyVal);
	do {
		hashEnt = hashTab->table[htScan];
		ctSlot = &conTable[hashEnt];

		/* An empty slot implies search failure */
		if (hashEnt == NIL_HASHENT)
			return (struct pcNex *) 0;

		/* When old conTable slot is found, delete it from hash table */
		if (hashEnt != DEL_HASHENT && ctSlot == oldNex) {
			hashTab->table[htScan] = DEL_HASHENT;
			return ctSlot;
		}

		/* Go on to next slot */
		htScan = (htScan + htIncr) % CT_HASH_SIZE;
	} while (htScan != htStart);

	return (struct pcNex *) 0;
}


struct pcNex *
htGet(hashTab, keyVal)
r0 hashTable		*hashTab;	/* The hash table to search */
ref			keyVal;		/* Key value */
{
int			htStart;	/* Starting search slot from hFunc1() */
r2 int			htScan;		/* Index used in search */
int			htIncr;		/* htScan increment from hFunc2() */
int			hashEnt;	/* hashTab->table[] entry */
r1 struct pcNex		*ctSlot;	/* Ptr from hash table to conTable */

	/* Scan the hash table looking for netAddr */
	htScan = htStart = (*hashTab->hFunc1)(keyVal);
	htIncr = (*hashTab->hFunc2)(keyVal);
	do {
		hashEnt = hashTab->table[htScan];
		ctSlot = &conTable[hashEnt];

		/* An empty slot implies search failure */
		if (hashEnt == NIL_HASHENT)
			return (struct pcNex *) 0;

		/* Return conTable entry upon match of netAddr */
		if (hashEnt != DEL_HASHENT &&
		    (*hashTab->cmpFunc)(keyVal, ctSlot) == 0)
			return ctSlot;

		/* Go on to next slot */
		htScan = (htScan + htIncr) % CT_HASH_SIZE;
	} while (htScan != htStart);

	return (struct pcNex *) 0;
}


void
htInit(hashTab)
hashTable		*hashTab;		/* Hash table to init */
{
r0 short		*htaFill;		/* Fill table w/ NIL_HASHENT */

	for (htaFill = hashTab->table;
	     htaFill < &hashTab->table[CT_HASH_SIZE]; )
		*htaFill++ = NIL_HASHENT;
}


/* ----- Hash Code and Key/Element Comparison Functions ----- */

/*
   addrHash1:
   addrHash2:
   addCmp:		Hash and compare functions for address hash table
*/

unsigned int
addrHash1(netAddr)
r0 char			*netAddr;	/* Net address to hash */
{
r1 char			*addrScan;	/* Scan the net address */
r2 unsigned 		addrSum = 0;	/* Cumulative sum of bytes in netAddr */

	/* Sum the bytes in the netAddr (modulo the max unsigned int) */
	/* Pardon my magic numbers.  See comment in addrCmp() RAK 5/89 */
	for (addrScan = netAddr+4; addrScan < netAddr+8;)
	     addrSum += (unsigned) *addrScan++;

	return addrSum % CT_HASH_SIZE;
}


unsigned int
addrHash2(netAddr)
r0 char			*netAddr;	/* Net address to hash */
{
r1 char			*addrScan;	/* Scan the net address */
r2 unsigned 		addrSum = 0;	/* Cumulative sum of bytes in netAddr */

	/* Sum the bytes in the netAddr (modulo the max unsigned int) */
	/* See comment in addrCmp() */
	for (addrScan = netAddr+4; addrScan < netAddr+8;)
	     addrSum += (unsigned) *addrScan++;

	return (CT_HASH_SIZE - 2) - addrSum % (CT_HASH_SIZE - 2);
}


/* return 0 for equality, nonzero otherwise */
int
addrCmp(netAddr, ctSlot)
char			*netAddr;	/* Address against which to compare */
pcNex			*ctSlot;	/* conTable slot to compare */
{
	/* This bit of grossness makes sure we only compare the actual */
	/* host portion of the address structure.  There is a bug in Prime's */
	/* TCP/IP which causes the other bytes in the structure to change */
	/* (though the host bytes remain correct).  This is similar to */
	/* nAddrEq() in pci_types.h.  I tried using that here, but it didn't */
	/* work.  I don't know why.  This does. RAK 5/89 */
	return memcmp((netAddr+4), (ctSlot->pcAddr)+4, SZHADDR);
}


#ifdef	SERIAL_HASH
/*
   serialHash1:
   serialHash2:
   serialCmp:		Hash and compare functions for serial number hash table
*/

unsigned int
serialHash1(serNum)
r0 char			*serNum;	/* Net address to hash */
{
r1 char			*serScan;	/* Scan the serial number */
r2 unsigned 		serSum = 0;	/* Cumulative sum of bytes in serNum */

	/* Sum the bytes in the serNum (modulo the max unsigned int) */
	for (serScan = serNum; serScan < &serNum[SERIALSIZE];)
	     serSum += (unsigned) *serScan++;

	return serSum % CT_HASH_SIZE;
}


unsigned int
serialHash2(serNum)
r0 char			*serNum;	/* Net address to hash */
{
r1 char			*serScan;	/* Scan the serial number */
r2 unsigned 		serSum = 0;	/* Cumulative sum of bytes in serNum */

	/* Sum the bytes in the serNum (modulo the max unsigned int) */
	for (serScan = serNum; serScan < &serNum[SERIALSIZE];)
	     serSum += (unsigned) *serScan++;

	return (CT_HASH_SIZE - 2) - addrSum % (CT_HASH_SIZE - 2);
}


int
serialCmp(serNum, ctSlot)
char			*serNum;	/* Serial number */
pcNext			*ctSlot;	/* conTable slot to compare */
{
	return memcmp(serNum, ctSlot->pcSerial, SERIALSIZE);
}
#endif	/* -SERIAL_HASH */


#ifdef	PID_HASH
/*
   svrPIDHash1:
   svrPIDHash2:
   svrPIDCmp:		Hash and compare functions for address hash table
*/

unsigned int
svrPIDHash1(svrPID)
long			svrPID;		/* Process ID to hash */
{
	return svrPID % CT_HASH_SIZE;
}


unsigned int
svrPIDHash2(svrPID)
long			svrPID;		/* Process ID to hash */
{
	return (CT_HASH_SIZE - 2) - svrPID % (CT_HASH_SIZE - 2);
}


int
svrPIDCmp(svrPID, ctSlot)
long			svrPID;		/* Process ID to hash */
pcNext			*ctSlot;	/* conTable slot to compare */
{
	/* Matches on either dossvr PID or dosout PID */
	if (svrPID == ctSlot->svrPid || svrPID == ctSlot->emPid)
		return 0;
	else
		return 1;
}
#endif	/* +PID_HASH */



/* ----- Connection Table Allocation and De-allocation ----- */

/*
   For the purpose of maintaining a free list of pcNex structs in
   the conTable, that structure is overlayed with this union to provide
   free list links without increasing the size of the structure.
*/
union nexFree {
	struct pcNex	theNex;		/* The actual structure */
	union nexFree	*nextFree;	/* Free list link */
};

union nexFree		*ctFree;	/* Head of conTable free list */
union nexFree		*ctLastFree;	/* Tail of conTable free list */

/*
   initConTable:		Initialize conTable[]
*/

void
initConTable()
{
r0 union nexFree	*ctInit;	/* Scan table linking up free list */

	/* Point each slot of conTable at the next through it's nextFree link */
	for (ctInit = (union nexFree *)conTable;
	     ctInit < (union nexFree *)&conTable[MAX_PORTS - 1];
	     ctInit++)
	{
		/* Point each slot at the next */
		ctInit->nextFree = ctInit + 1;

	}

	/* Terminate the linked list with a nil pointer */
	ctInit->nextFree = 0;

	/* Point ctFree at head and ctLastFree at tail of the free list */
	/* TBD - This breaks if PCI_CONSVR_PORT == 0 */
	ctFree = (union nexFree *)&conTable[0];
	ctLastFree = ctInit;

	/* Initialize hash table */
	htInit(&addrHash);
#ifdef	PID_HASH
	htInit(&svrPIDHash);
#endif
#ifdef	SERIAL_HASH
	htInit(&serialHash);
#endif
}


/*
   When allocating, table slots are taken from the front of the free
   list while slots being freed are placed at the end of the list.
   This is done so that in ETHLOCUS versions of PCI, the failure to
   open an network port does not cause indefinite looping on the open
   attempt.
*/


/*
   allocCTSlot:			Allocate a free conTable slot to caller
*/

int
allocCTSlot()
{
r0 union nexFree	*freeSlot;		/* Newly allocated slot */

	/* Take first free slot on list... */
	freeSlot = ctFree;

	/* ... If any, then remove the newly allocated slot from free list */
	if (ctFree != 0)
		ctFree = ctFree->nextFree;

	/* When free list is empty, nil out ctLastFree as well */
	if  (ctFree == 0)
		ctLastFree = 0;

	/* Return pointer to allocated slot */
	memset((char *)freeSlot, 0, sizeof (pcNex));
	return (pcNex *)freeSlot - conTable;
}


/*
   freeCTSlot:			Release a no-longer-used conTable slot
*/

void
freeCTSlot(freeSlot)
r0 union nexFree	*freeSlot;		/* Slot to de-allocate */
{
#ifdef	PID_HASH
	/*
	   The PID index isn't removed by rmvCTIndex() so that the signal
	   handling code can find the correct entry and call freeCTSlot()
	*/
	htRmv(&svrPIDHash, freeSlot, freeSlot->svrPid);
#endif

	/* Clear out old conTable slot */
	memset((char *)freeSlot, 0, sizeof *freeSlot);

	/* TBD - Should only these fields of the slot be cleared? */
	nAddrClr(freeSlot->theNex.pcAddr);
	nAddrClr(freeSlot->theNex.pcAddr2);
	serialClr(freeSlot->theNex.pcSerial);
	freeSlot->theNex.svrPid = 0;
	freeSlot->theNex.emPid = 0;

	/* Put old slot at end of free list */
	freeSlot->nextFree = 0;
	if (ctLastFree != 0) {
		ctLastFree->nextFree = freeSlot;
		ctLastFree = freeSlot;
	}
	if (ctFree == 0)
		ctFree = ctLastFree;
}
