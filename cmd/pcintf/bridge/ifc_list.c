/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/ifc_list.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)ifc_list.c	3.4	LCC);	/* Modified: 16:56:27 2/12/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*
 *	Interface list parser
 */

#include	"pci_types.h"
#include	<errno.h>
#include	<string.h>

extern netIntFace	intFaceList[MAX_NET_INTFACE];

#if defined(MICOM) || defined (EXCELAN)
extern unsigned long inet_addr();
#endif	/* MICOM || EXCELAN */

get_interface_list(arg)
register char *arg;
{
	register char *ap;
	int num_ifc;

	num_ifc = 0;
	while ((ap = strchr(arg, ';')) != NULL) {
		*ap++ = '\0';
		if (parse_interface(arg, &intFaceList[num_ifc], num_ifc))
			num_ifc++;
		arg = ap;
	}
	if (*arg && parse_interface(arg, &intFaceList[num_ifc], num_ifc))
		num_ifc++;
	return num_ifc;
}

parse_interface(arg, ifc, ifc_num)
register char *arg;
netIntFace *ifc;
int ifc_num;
{
	register char *ap;
	long	*brdp;
	long	*subnetp;
	char *brd_arg;
	char *subnet_arg;
#ifdef DGUX		/* maybe this should be in all versions...[ERP] */
 	struct in_addr addr;
#else
	unsigned long addr;
#endif	/* DGUX */


#ifdef BUG_ALERT
	if (*ap == '0')	/* [ERP] this is a BUG, what is this *supposed* to do? */
		return 0;
#endif /* BUG */

	brdp = (long *) &(ifc->broadAddr);
	subnetp = (long *) &(ifc->subnetMask);

	brd_arg = subnet_arg = NULL;
	if ((ap = strchr(arg, ',')) != NULL) {
		*ap++ = '\0';
		if (*ap && *ap != ',')
			brd_arg = ap;
		if ((ap = strchr(ap, ',')) != NULL) {
			*ap++ = '\0';
			subnet_arg = ap;
		}
	}

#ifdef DGUX
	if ((addr = inet_addr(arg)).s_addr == 0xffffffffL) {
#else
	if ((addr = inet_addr(arg)) == 0xffffffffL) {
#endif	/* DGUX */
		log("got bogus local address <%s>...oops\n",arg);
		return 0;
	}
	
	/* Copy the local address */

	memcpy(&ifc->localAddr, &addr, sizeof(struct in_addr));

#ifdef DGUX
	if (brd_arg != NULL && (addr = inet_addr(brd_arg)).s_addr != 0xffffffffL)
#else
	if (brd_arg != NULL && (addr = inet_addr(brd_arg)) != 0xffffffffL)
#endif	/* DGUX */
		memcpy(&ifc->broadAddr, &addr, sizeof(struct in_addr));
	else {
#if defined(EXCELAN)
		*brdp = *((long *)&ifc->localAddr);
		*brdp |= !(*brdp & IN_CLASSA) ? IN_CLASSA_LNA :
			 !(*brdp & IN_CLASSB) ? IN_CLASSB_LNA :
						IN_CLASSC_LNA;
#else
		*brdp = ntohl(*((long *)&ifc->localAddr));

		if (IN_CLASSA(*brdp))
			 *brdp |= ~IN_CLASSA_NET;
		else if (IN_CLASSB(*brdp))
			 *brdp |= ~IN_CLASSB_NET;
		else if (IN_CLASSC(*brdp))
			 *brdp |= ~IN_CLASSC_NET;

		 *brdp = htonl(*brdp);
#endif	/* EXCELAN */
	}

#ifdef DGUX
	if (subnet_arg != NULL && 
		(addr = inet_addr(subnet_arg)).s_addr != 0xffffffffL)
#else
	if (subnet_arg != NULL && (addr = inet_addr(subnet_arg)) != 0xffffffffL)
#endif	/* DGUX */
		memcpy(&ifc->subnetMask, &addr, sizeof(struct in_addr));
	else {
#if defined(EXCELAN)
		*subnetp = !IN_LNAOF(*(struct in_addr *)brdp);
#else
		*brdp = ntohl(*brdp);

		if (IN_CLASSA(*brdp))
			 *subnetp = IN_CLASSA_NET;
		else if (IN_CLASSB(*brdp))
			 *subnetp = IN_CLASSB_NET;
		else if (IN_CLASSC(*brdp))
			 *subnetp = IN_CLASSC_NET;

		 *brdp = htonl(*brdp);
		 *subnetp = htonl(*subnetp);
#endif	/* EXCELAN */
	}

	log("intFaceList[%d]: Local %s Broadcast %s Subnet mask %s\n",
		ifc_num,
		nAddrFmt(&ifc->localAddr),
		nAddrFmt(&ifc->broadAddr),
		nAddrFmt(&ifc->subnetMask));
	return 1;
}
