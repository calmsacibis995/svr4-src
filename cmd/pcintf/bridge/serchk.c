/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/serchk.c	1.1"
#ifdef IBM_SERCHK
#include	<sccs.h>

SCCSID(@(#)serchk.c	1.2	LCC);	/* Modified: 13:52:40 7/14/87 */

#include "pci_types.h"
#include "serial.h"
#include "flip.h"

/* this table is index by the "vers_major" value of the 3 part PC-Interface
** version number; it is used to limit use of PC-Interface to a specific
** range of serial numbers, based on the version number the DOS portion of
** PC-Interface presents to the host connection servers.
*/

#define	NMAJOR	4
struct serchktab{ unsigned long minser; unsigned long maxser;};
struct serchktab serchktab[NMAJOR] = {
	{ 0x00000000,0x00000000},		/* MAJOR <= 0 */
	{ 0x08000000,0x08ffffff},		/* MAJOR == 1 */
	{ 0x0a000000,0x0affffff},		/* MAJOR == 2 */
	{ 0x00000000,0x00000000},		/* MAJOR >= 3 */
};

/*
** the "serchk" routine does not do serial number validation; what it
** does do is to check whether the serial number presented is within the
** range of allowed serial numbers for a specific version of the DOS
** PC-Interface component.
*/
serchk(pkt,flipCode)
struct input *pkt;
int	flipCode;
{
	struct connect_text *ct = (struct connect_text *) pkt->text;
	SERI_ST *sp;
	unsigned long serno;
	unsigned long tl;
	struct serchktab *s;
	int vmaj;
	

	vmaj = ct->vers_major;
	sp = (SERI_ST *) ct->serial_num;
	serno = sp->ser_no;

	lflipm(serno,tl,flipCode);	/* get serial number in normal order */

	vmaj = (vmaj <= 0 ? 0 : (vmaj >= NMAJOR ? NMAJOR-1 : vmaj));
	s = &serchktab[vmaj];

	if ((serno < s->minser) || (serno > s->maxser)) {

		log("%s:vmaj %d, ckmaj %d, ser# %lx, minser %lx, maxser %lx\n",
			"serchk failure",ct->vers_major,vmaj,
			serno,s->minser,s->maxser);
		return(0);
	} else return(1);
}
#endif	/* IBM_SERCHK */
