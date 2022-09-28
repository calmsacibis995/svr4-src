/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:machine_type.c	1.3.1.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	machine_type
 *
 *	This program is used by the installation script
 *	to determine the type of machine that it is running on.
 *	It exits with one of the following codes:
 *
 *		0	Generic AT386
 *		1	Compaq 386
 *		2	AT&T 6386	(640K base mem)
 *		3	AT&T 6386	(512K base mem)
 *
 *	This program uses the new /dev/pmem to probe physical addresses.
 *	Also, it assumes the system was booted from the generic
 *	(default.at386) configuration.
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/bootinfo.h>
#include <sys/sysi86.h>

#define GENERIC	0
#define COMPAQ	1
#define ATT640	2
#define ATT512	3

struct bootinfo bootinfo;
int pmem_fd;


main()
{
	if (sysi86(SI86RDID) != 0)
		exit(GENERIC);

	if ((pmem_fd = open("/dev/pmem", O_RDWR)) < 0) {
		perror("/dev/pmem");
		exit(GENERIC);
	}

	lseek(pmem_fd, BOOTINFO_LOC, 0);
	if (read(pmem_fd, (char *)&bootinfo, sizeof(bootinfo)) < 0) {
		perror("/dev/pmem");
		exit(GENERIC);
	}

	/* First check for Compaq-style extra memory.
		This will be at 15M+640K with a gap before it. */
	if (!in_memavail(0xFA0000) && mem_probe(0xFA0000))
		exit(COMPAQ);

	/* Now check for AT&T-style aliased memory.
		This will be at 2G+640K.  */
	if (mem_probe(0x800F0000)) {
		if (in_memavail(0x80000))
			exit(ATT640);
		exit(ATT512);
	}

	exit(GENERIC);
}


in_memavail(addr)
	paddr_t	addr;
{
	register int	i;

	for (i = bootinfo.memavailcnt; i-- > 0;) {
		if (addr >= bootinfo.memavail[i].base &&
				addr < bootinfo.memavail[i].base +
					bootinfo.memavail[i].extent) {
			return 1;
		}
	}
	return 0;
}


do_probe(addr)
	paddr_t	addr;
{
	unchar	val;

	val = 0xA5;
	if (write(pmem_fd, &val, 1) < 1)
		return 0;
	lseek(pmem_fd, -1L, 1);
	if (read(pmem_fd, &val, 1) < 1)
		return 0;
	if (val == 0xA5) {
		val = 0x5A;
		lseek(pmem_fd, -1L, 1);
		if (write(pmem_fd, &val, 1) < 1)
			return 0;
		lseek(pmem_fd, -1L, 1);
		if (read(pmem_fd, &val, 1) < 1)
			return 0;
		if (val == 0x5A)
			return 1;
	}
	return 0;
}


mem_probe(addr)
	paddr_t	addr;
{
	unchar	old_val;
	int	retval;

	lseek(pmem_fd, addr, 0);
	if (read(pmem_fd, &old_val, 1) < 1)
		return 0;
	lseek(pmem_fd, -1L, 1);
	retval = do_probe(addr);
	lseek(pmem_fd, addr, 0);
	write(pmem_fd, &old_val, 1);
	return retval;
}
