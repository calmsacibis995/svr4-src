/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/loadpci.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)loadpci.c	3.4	LCC);	/* Modified: 13:59:14 1/29/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*
 *	PCI program loader.
 *
 *	Loads PCI consvr, mapsvr after opening network and reading ifc list.
 */

#include	"pci_types.h"
#include	<errno.h>
#include	<string.h>
#if defined(EXCELAN)
extern char *inet_ntoa();	/* defined in net.c */
#endif	/* EXCELAN */


char *myname = "loadpci";

netIntFace	intFaceList[MAX_NET_INTFACE];

int		numIntFace = 0;

#define MAXARGS	64

char *args[MAXARGS];

char net_buf[8];
char int_list_buf[4096];


main(argc, argv)
int argc;
char **argv;
{
	char **argp;
	register char *arg;
	char *pgm = NULL;
	int brd_local = 0;
	int mode = 0;
	int have_interfaces = 0;
	int net_desc;
	int port = PCI_CONSVR_PORT;
	char *netDev = NETDEV;		/* Network interface device */
	char fqNetDev[32];		/* Fully qualified netDev name */

	logOpen(LOADPCI_LOG, 0);
	argp = &args[1];
	while (argc-- > 1) {
		arg = *++argv;

		if (*arg != '-') {
			pgm = arg;
			args[0] = pgm;
			continue;
		}

		switch (arg[1]) {

		case 'B':
			mode |= BCAST43;
			break;

		case 'I':
			*argp++ = arg;
			have_interfaces++;
			break;

		case 'N':			/* Network device */
			netDev = &arg[2];
			if (netDev == '\0')
				netDev = NETDEV;
			else if (*netDev != '/') {
				sprintf(fqNetDev, "/dev/%s", netDev);
				netDev = fqNetDev;
			}
			*argp++ = arg;
			break;

		case 'P':
			if (!strcmp(&arg[2], "CONSVR"))
				port = PCI_CONSVR_PORT;
			else if (!strcmp(&arg[2], "MAPSVR"))
				port = PCI_MAPSVR_PORT;
			else
				sscanf(&arg[2], "%d", &port);
			break;

		case 'S':
			mode |= USESUBNETS;
			break;

		case 'b':
			brd_local++;
			break;

		case 'n':
			fprintf(stderr, "loadpci: Can't specify network descriptor\n");
			exit(1);

		case 'D':
			dbgSet(strtol(&arg[2], NULL, 16));
			/* fall through to default so dbg arg gets passed on */

		default:
			*argp++ = arg;
			break;
		}
	}
	if ((net_desc = netOpen(netDev, port)) < 0) {
		fprintf(stderr, "loadpci: Can't open network - Bye\n");
		exit(1);
	}
	sprintf(net_buf, "-n%d", net_desc);
	*argp++ = net_buf;

	if (!have_interfaces) {
		netaddr(intFaceList, &numIntFace, mode);
		if (brd_local)
			add_local_interface();
		build_interface_list();
		*argp++ = int_list_buf;
	}

	if (pgm == NULL) {
		fprintf(stderr, "loadpci: No program name\n");
		exit(1);
	}

	*argp = NULL;
	execv(pgm, args);
	fprintf(stderr, "loadpci: Couldn't exec %s\n", pgm);
}


add_local_interface()
{
	struct sockaddr_in sa;
	netIntFace *ifp;

	hostaddr(&sa);
	ifp = &intFaceList[numIntFace++];
	ifp->localAddr.s_addr = sa.sin_addr.s_addr;
	ifp->broadAddr.s_addr = sa.sin_addr.s_addr;
	ifp->subnetMask.s_addr = 0xffffffffL;
}


build_interface_list()
{
	register char *bp;
	register int i;
	register netIntFace *ifp;

	sprintf(int_list_buf, "-I");
	bp = &int_list_buf[2];
	ifp = intFaceList;
	for (i = 0; i < numIntFace; i++) {
		sprintf(bp, "%s,", inet_ntoa(ifp->localAddr));
		bp = &bp[strlen(bp)];
		sprintf(bp, "%s,", inet_ntoa(ifp->broadAddr));
		bp = &bp[strlen(bp)];
		sprintf(bp, "%s;", inet_ntoa(ifp->subnetMask));
		bp = &bp[strlen(bp)];
		ifp++;
	}
}
