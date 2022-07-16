/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:xmmu.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"

/*
 * copyio - copy n bytes from/to a physical address (paddr)
 *	to/from a long address (caddr).
 *	
 */

int
copyio(addr1, addr2, size, mapping)
paddr_t addr1;
caddr_t addr2;
int size;
int mapping;
{
	int rslt;

	if (size == 0)
		return(-1);
	rslt = 0;
	addr1 = phystokv(addr1);
	if ( mapping == U_RKD )
		bcopy((caddr_t)addr1, addr2, size);
	else if ( mapping == U_WKD )
		bcopy(addr2, (caddr_t)addr1, size);
	else if ( mapping == U_RUD )
		rslt= copyout((caddr_t)addr1, addr2, size);
	else if ( mapping == U_WUD )
		rslt= copyin(addr2, (caddr_t)addr1, size);
	else
		cmn_err(CE_PANIC, "bad mapping in copyio\n");
	return(rslt);
}

/*
 * The routines cvttoaddr(), cvttoint(), and ldtalloc() are
 * provided for XENIX device driver source compatibility.
 * Their functions are now handled by the 286 emulator, so
 * these routines just return errors.  They should never
 * be called if the emulator works correctly.
 * The routine ldtfree() is also provided for XENIX device
 * driver compatibility.  ldtfree() is just a stub as the
 * XENIX routine did not return any indication of success
 * or failure.
 *
 * These four routines will go away in a future release of
 * the operating system.
 */

caddr_t
cvttoaddr(addr286)
char *addr286;
{
	return((caddr_t)NULL);
}

int
cvttoint(addr286)
char *addr286;
{
	return(NULL);
}

unsigned short
ldtalloc(startsel, cnt)
unsigned short startsel;
int cnt;
{
	return((unsigned short)-1);
}

ldtfree(startsel, cnt)
unsigned short startsel;
int cnt;
{
}

memget(clicks)
int clicks;
{
	return( btoc(getcpages(clicks, 1)) );
}

/*
 * map - map physical memory address to a virtual address
 */
mapphys(physaddr, nbytes)
char *physaddr;
int nbytes;
{
	return( phystokv((int)physaddr) );
}

/*
 * free up entries from previous mapphys call. 
 */
void unmapphys(va, nbytes)
char *va;
int nbytes;
{
	/* nothing to do - first 4Mb of physical memory always mapped. */
}

