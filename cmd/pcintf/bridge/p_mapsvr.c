/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_mapsvr.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_mapsvr.c	3.20	LCC);	/* Modified: 15:08:07 1/29/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*			PCI Configuration server		*/
#include	"pci_types.h"
#include	<errno.h>
#include	<string.h>
#include	<memory.h>
#include	"flip.h"

#define	UNIX
#include 	"serial.h"
#define	PRINT_CS  /**/


char
	CopyRight[] =
"PC INTERFACE UNIX FILE SERVER.  COPYRIGHT (C) 1984, LOCUS COMPUTING CORP.\n\
ALL RIGHTS RESERVED.  USE OF THIS SOFTWARE IS GRANTED ONLY UNDER LICENSE\n\
FROM LOCUS COMPUTING CORPORATION.  UNAUTHORIZED USE IS STRICTLY PROHIBITED.\n";



/*			External Functions			*/

extern char *
	nAddrFmt();		/* Format the net address */

extern int
	xmtPacket(),		/* Transmit a packet */
	rcvPacket();		/* Recieve a packet */

extern int
	errno;

char
	*logArg;

#ifndef SIGNAL_KNOWN
extern int
	(*signal())();
#endif

void
	catchSignal();

struct nameAddr
	siteMap[MAX_SITES];		/* The current host map */
char
	siteAges[MAX_SITES];		/* Notification ages */
int
	siteCount;			/* Number of entries in siteMap */
int
	sortingMap;			/* Flag set when sorting map */

#define	DAEMON_THERE	1		/* Consvr has notified us recently */
#define	DAEMON_GONE	3		/* Max missed notifications */
#define	MAPSVR_PERIOD	30		/* Table scan period */
#define	START_DELAY	1		/* # of periods before service starts */

unsigned long
	schedCount;			/* Number of timed executions */

char
	nilAddr[SZNADDR];		/* Nil network address */
static struct ni2
	conSvrHeader;		/* Local consvr address */

struct ni2 
	niHeader;			/* Ethernet device header structure */

char
	*netDev = NETDEV,		/* Network interface device */
	fqNetDev[32];			/* Fully qualified netDev name */

int	oldserial = 1;			/* until upgraded, assume old
					*  serialization algorithm
					*/


char *myname;


netIntFace	intFaceList[MAX_NET_INTFACE];

int	numIntFace = 0;			/* The # of configured interfaces */


/*
   main():		Service Host-Sitemap requests and forward
			Security/Anti-Piracy tuples.
*/

main(argc, argv)
int
	argc;
char
	*argv[];
{
register int
	status,			/* Return value from system calls */
	flipCode;		/* How to flip bytes */
int
	argN;			/* Current argument number */
char
	*arg;			/* Current argument */
struct input *input;

int	use_subnets = 0;
int	netdesc = -1;

	myname = argv[0];
	if (*myname == '\0')
		myname = "unknown";

	/* Open logfile to record unusual occurences */
	logOpen(MAPSVR_LOG, 0);

	for (argN = 1; argN < argc; argN++) {
		arg = argv[argN];

		if (*arg != '-') {
			fprintf(stderr, "pcimapsvr: Bad usage\n");
			exit(1);
		}

		switch (arg[1]) {

		case 'D':
			logArg = &arg[2];
			dbgSet(strtol(logArg, NULL, 16));
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

		case 'S':
			use_subnets = USESUBNETS;
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

	/* Standard in/out/err not used */
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	log("PC Interface Map Server Start ********\n");

	/* SIG_DBG1 toggles logs on and off */
	signal(SIG_DBG1, catchSignal);
	signal(SIGTERM, catchSignal);

	/* Detach myself from my invoker */
#ifdef	SYS5
	setpgrp();
#else	/* !SYS5 */
	setpgrp(0,getpid());
#endif	/* !SYS5 */

	if (netdesc >= 0)
		netUse(netdesc);
	else if (netUse(netOpen(netDev, PCI_MAPSVR_PORT)) < 0)
		fatal("Connection server can't open network - Bye\n");

	/* This determines the configured net interfaces */

	if (numIntFace == 0)
		netaddr(intFaceList, &numIntFace, use_subnets);

	/* fill in the local consvr's address */
	conSvrHeader.dst_sin.sin_addr = intFaceList[0].localAddr;
	conSvrHeader.dst_sin.sin_family = AF_INET;
	conSvrHeader.dst_sin.sin_port = htons(PCI_CONSVR_PORT);
	memset(conSvrHeader.dst_sin.sin_zero, 0, 8);

	checkCode(); 	/*verify that code is not changed*/
	mapService();

	log("Map service stopped - Bye\n");
	exit(0);
}


/*
   mapService: Main map server loop
*/

mapService()
{
struct input
	inPacket;			/* Input packet buffer */
struct output
	outPacket;			/* Output packet buffer */
long
	flipCode;			/* Byte flipping sense pattern	*/

	/* Start timed execution tasks */
	signal(SIGALRM, catchSignal);
	alarm(MAPSVR_PERIOD);

	for (;;) {
		flipCode = rcvPacket(&inPacket);
		outPacket.hdr.res = SUCCESS;
		outPacket.pre.select = DAEMON;
		outPacket.hdr.seq = inPacket.hdr.seq;
		outPacket.hdr.req = inPacket.hdr.req;
		outPacket.hdr.stat = NEW;
		outPacket.hdr.t_cnt = 0;

		/* Store address of destination PC */
		nAddrCpy(niHeader.dst, inPacket.net.src);

		switch(inPacket.hdr.req) {
		case CONSVR_HERE:
			consvrHere(&inPacket);
			break;

		case CONSVR_BYE:
			consvrBye(&inPacket);
			break;

		case SEND_MAP:
			if (schedCount >= START_DELAY)
				sendMap(&inPacket, &outPacket, flipCode);
			break;

		default:
			log("Bad req: %d\n", inPacket.hdr.req);
			break;
		}
	}
}


/*
   sendMap: Send host map
*/

sendMap(reqPacket, rspPacket, flipCode)
register struct input	*reqPacket;
register struct input	*rspPacket;
long	flipCode;
{
	struct seriEthSt	*seriEthP;

	unsigned long clientHostAddr;	/* Host Address of requesting client */
	unsigned long clientSubnet;	/* Network # of requesting client */
	struct nameAddr	*mapScan;	/* Used to scan siteMap */
	char	*textP;			/* Scans text area of response */
	int	i;			/* Loop counter */
	int	sameNet = 0;		/* Is client on same net as map */
					/* server			*/
	unsigned long subnetMask;	/* subnet mask for response */
	int	timeleft;		/* alarm time remaining */

	clientHostAddr = reqPacket->net.src_sin.sin_addr.s_addr;

	/* If the client does not have the same network number as one of */
	/* this host's configured interfaces then do not send a response */

	for ( i = 0; i < numIntFace; ++i) {
		subnetMask = intFaceList[i].subnetMask.s_addr;
		if ((clientHostAddr & subnetMask) ==
		    (intFaceList[i].localAddr.s_addr & subnetMask)) {
			sameNet = 1;
			break;
		}
	}

	if (!sameNet)
		return;
	clientSubnet = clientHostAddr & subnetMask;

	log("Client subnet = 0x%x, subnet mask = 0x%x\n", clientSubnet, subnetMask);

	/* Build the map response including only those entries whose   */
	/* network number is the same as that of the requesting client */

	textP = rspPacket->text;
	rspPacket->hdr.b_cnt = 0;
	rspPacket->hdr.t_cnt = 0;

	for (mapScan = siteMap; mapScan < &siteMap[siteCount]; mapScan++) {
		if (clientSubnet !=
		    ((*((struct in_addr *) mapScan->address)).s_addr & subnetMask))
			continue;

		memcpy(textP, mapScan, sizeof(struct nameAddr));
		textP += sizeof(struct nameAddr);
		++rspPacket->hdr.b_cnt;
		rspPacket->hdr.t_cnt += sizeof(struct nameAddr);
	}

	/* 
	   If serial number is present and invalid send message that causes
	   pc to crash instead of a site map and return early.
	*/
	if (!validSerial(reqPacket->text, flipCode)) {
		/* Send message to halt pc */
		rspPacket->hdr.req = PC_CRASH;           
		rspPacket->pre.select = UNSOLICITED;
		rspPacket->hdr.b_cnt = 0;              
		debug(0, ("send PC_CRASH to %s\n",
			nAddrFmt(reqPacket->net.src)));
	}
	/*
	   This kludgey hack randomizes the arrival times of the
	   (many) site map tables at the requesting PC.  If this
	   is not done, the PC does not successfully receive any
	   of the sitemap tables!
	*/
	/*	The alarm is suspended around the sleep() call as a workaround
	**	for a bug in SVR4.0 Load B11, in which sleep() fails to restore
	**	the signal state.  May be removed later.		-- SDE
	*/
	timeleft = alarm(0);
	sleep(1);
	signal(SIGALRM, catchSignal);
	alarm(timeleft);

#ifdef EXCELAN
	niHeader.dst_sin.sin_family = AF_INET;
#endif /* EXCELAN */
	xmtPacket(rspPacket, &niHeader, flipCode);

	/* Send security check message to local consvr */

	rspPacket->hdr.req = SECURITY_CHECK;
	rspPacket->pre.select = DAEMON;
	rspPacket->hdr.stat = NEW;

	/* Send Ethernet address and serial number */
	seriEthP = (struct seriEthSt *)rspPacket->text;
	nAddrCpy(seriEthP->serAddr, reqPacket->net.src);
	memcpy(&seriEthP->serNum, reqPacket->text, reqPacket->hdr.t_cnt);
	rspPacket->hdr.t_cnt = sizeof(struct seriEthSt);
	rspPacket->hdr.b_cnt = 0;

	debug(0, ("Piracy check to consvr\n"));
	xmtPacket(rspPacket, &conSvrHeader, NOFLIP);
}


/*
   consvrHere:  Record presence of a connection server
*/

consvrHere(herePacket)
struct input
	*herePacket;
{
struct nameAddr
	*hereSite,			/* Sitentry of site */

	*mapScan;			/* Scan siteMap[] */
	int	i;


	hereSite = (struct nameAddr *)herePacket->text;
	if (inAddrEq(hereSite->address, nilAddr) != 0) {
		log("ConsvrHere packet with NIL address: %d %s %s\n",
					sizeof hereSite->name, hereSite->name,
					nAddrFmt(hereSite->address));
		return;
		}

	/* Scan map looking for address matching that of herePacket */
	for (mapScan = siteMap; mapScan < &siteMap[MAX_SITES]; mapScan++) {
		if (inAddrEq(mapScan->address, hereSite->address) != 0) {
			siteAges[mapScan - siteMap] = DAEMON_THERE;
			return;
		}
		if (inAddrEq(mapScan->address, nilAddr) != 0) {
			siteCount++;
			log("New consvr: %d %s %s\n",
				sizeof hereSite->name, hereSite->name,
				nAddrFmt(hereSite->address));

			*mapScan = *hereSite;
			siteAges[mapScan - siteMap] = DAEMON_THERE;
			siteMapSort();
			return;
		}
	}

	log("...Map full\n");
}


/*
   consvrBye:  Record loss of a connection server
*/

consvrBye(byePacket)
struct input
	*byePacket;
{
register struct nameAddr
	*byeSite;			/* Sitentry of site */
register int
	mapScan;			/* Scan siteMap[] */

	byeSite = (struct nameAddr *)byePacket->text;

	log("Consvr leaving: %.8s:%s\n", byeSite->name,
		nAddrFmt(byeSite->address));

	/* Scan map looking for address matching that in byePacket */
	for (mapScan = 0; mapScan < siteCount; mapScan++)
		if (inAddrEq(siteMap[mapScan].address, byeSite->address)) {
			rmvSite(mapScan);
			return;
		}
}


/*
   smCmp: Compare site map entries (for qsort)
*/

int
smCmp(sm1, sm2)
struct nameAddr
	*sm1,
	*sm2;
{
	return strncmp(sm1->name, sm2->name, sizeof(sm1->name));
}


/*
   siteMapSort: Keep all the names in the site map table in lexical order
*/

siteMapSort()
{
	sortingMap = 1;
	qsort((char *)siteMap, siteCount, sizeof(struct nameAddr), smCmp);
	sortingMap = 0;
}


/*
   sched: Perform timed actions
*/

sched()
{
	if (!sortingMap) {
		schedCount++;	/* Once every eon or so schedCount will be 0 */
		checkMap();
	}
	alarm(MAPSVR_PERIOD);
}


/*
   checkMap: Check current site map, age and delete entries
*/

checkMap()
{
register int
	mapIndex;			/* siteMap index */

	for (mapIndex = 0; mapIndex < siteCount; mapIndex++)
		if (siteAges[mapIndex]++ > DAEMON_GONE) {
			log("Remove consvr %s:%s\n", siteMap[mapIndex].name,
				nAddrFmt(siteMap[mapIndex].address));
			rmvSite(mapIndex);
		}
}


/*
   rmvSite: Remove a site from siteMap
*/

rmvSite(deadSlot)
register int
	deadSlot;
{
register struct nameAddr
	*deadSite;

	deadSite = &siteMap[deadSlot];

	/* Scrunch out entry being removed */
	for (deadSite = &siteMap[deadSlot]; deadSlot < siteCount;
	     deadSlot++, deadSite++)
	{
		inAddrCpy(deadSite->address, deadSite[1].address);
		strncpy(deadSite->name, deadSite[1].name, SZHNAME);
		siteAges[deadSlot] = siteAges[deadSlot + 1];
	};

	siteCount--;
	inAddrClr(siteMap[siteCount].address);
	siteMapSort();
}


/*
   catchSignal:  Signal catcher.
*/

void
catchSignal(sigNum)
int
	sigNum;
{
	switch (sigNum) {
	case SIG_DBG1:			/* Alter log level */
		newLogs(MAPSVR_LOG, 0, NULL, NULL);
		if (logArg != NULL)
			sprintf(logArg, "%04x", dbgEnable);
		break;

	case SIGALRM:			/* Periodic table management */
		sched();
		break;

	case SIGTERM:			/* Request for termination */
		log("Stop signalled - Bye\n");
		exit(0);
	}

	signal(sigNum, catchSignal);
}


checkCode()
{
long
	cSum = 0;
long
	*textp;
extern
	LabelzZ();
#ifdef NEVER
asm("LabelzZ:");

	for (textp = (long *)main; textp < (long *)LabelzZ; textp++)
		cSum += *textp;


#ifdef	PRINT_CS
	debug(0, ("checksum = %lx \n", cSum));
#else	/* !PRINT_CS */
	if (cSum != 0x35dc9fc9)
		exit(1);
#endif	/* !PRINT_CS */
#endif /* NEVER */
}
