/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)proto:i386/machid.c	1.3.1.1"

/*
 *	machid
 *
 *	This program is used by the installation script
 *	to determine the type of machine that it is running on.
 *	It exits with one of the following codes:
 *
 *	0 -> 49   for 80386 AT Bus
 *		0	Generic AT386
 *		1	Compaq 386
 *		2	AT&T 6386 WGS or 6386E WGS
 *		4	Cascade 2 - 6386/SX
 *		5	Cascade 3 & Cascade 3 CR - 6386/25
 *		6	Cascade 4 - 6386E/33
 *		7	Cascade 2U - 6386/SX20
 *		10	Cascade E-333A (Cascade 4 and boot SCSI) - 6386E/33 Model S
 *
 *	50 -> 99   for 80486 AT Bus
 *
 *	100 -> 149 for 80386 EISA Bus
 *
 *	150 -> 199 for 80486 EISA Bus
 *		150	Cascade 6	- 486 EISA tower
 *		151	Cascade 5U	- 486 EISA desktop
 *		160	EN8R1 (Enterprise 8 Release 1) 486 EISA
 *
 *	This program uses the new /dev/pmem to probe physical addresses.
 *	Also, it assumes the system was booted from the generic
 *	(default.at386) configuration.
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/bootinfo.h>
#include <sys/sdi_edt.h>
#include <sys/sysi86.h>
#include <sys/cram.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#define GENERIC	0
#define COMPAQ	1
#define ATT640	2
#define CASCADE2 4
#define CASCADE3 5
#define CASCADE4 6
#define CASCADE2U 7
#define E333A   10
#define CASCADE6 150
#define CASCADE5U 151
#define EN8R1	160

#define BAD 3

void bswap_sig();

struct bootinfo bootinfo;
int pmem_fd;

extern int errno;

main(argc, argv)
int argc;
char *argv[];
{
	int sflag = 0;
	int scsi;
	int eflag = 0;
	int hd_fd;
	int error = 0;
#ifdef SCSI
	extern char *calloc();
	struct scsi_edt *xedt, *xedt3;
	int tp_flag = 0;	/* More than 1 Streaming Tape Device */
	int ha_cnt;
#endif
	struct bus_type bus_type;
	if ((argc == 2) && ((*(argv[1]+1) == 's') || (*(argv[1]+1) == 'S'))) {
		sflag++;
	if ((argc == 2) && ((*(argv[1]+2) == 'e') || (*(argv[1]+2) == 'E')))
		eflag++;
	}
	if ((argc == 2) && ((*(argv[1]+1) == 'p')))
		exit (processor());
	if ((argc == 2) && ((*(argv[1]+1) == 'a') || (*(argv[1]+2) == 'A')))
		exit (arch());


	if ((argc == 2) && ((*(argv[1]+1) == 'e') || (*(argv[1]+1) == 'E'))) {
		eflag++;
	if ((argc == 2) && ((*(argv[1]+2) == 's') || (*(argv[1]+2) == 'S')))
		sflag++;
	}
	if (eflag) {
#ifdef SCSI /* Make sure QT is on TC3 and HD is on TC0
		/* Look at scsi:cmd/prtconf.c for details */
		if ((hd_fd = open("/dev/scsi/c0", O_RDONLY)) < 0) {
			printf ("Can not open /dev/scsi/c0\n");
			exit (BAD);
		}
		ha_cnt = 0;
		if (ioctl(hd_fd, B_HA_CNT, &ha_cnt) < 0) {
			printf ("ioctl (B_NA_CNT) failed\n");
			close (hd_fd);
			exit (BAD);
		}
		if (ha_cnt == 0) {
			printf ("Can not determine the number of Host Adaptor Boards\n");
			close (hd_fd);
			exit (BAD);
		}
		scsi = sizeof(struct scsi_edt) * MAX_TCS * ha_cnt;
		if ((xedt = (struct scsi_edt *) calloc(1, scsi)) == (struct scsi_edt *) 0) {
			printf ("Can not calloc memory for xedt\n");
			close (hd_fd);
			exit (BAD);
		}
		if (ioctl (hd_fd, B_REDT, xedt) < 0) {
			printf ("ioctl (B_REDT) failed.\n");
			close (hd_fd);
			exit (BAD);
		}
		close (hd_fd);
		scsi = scsi * MAX_TCS;

		/* Is the Host Addaptor on SCSI ID 0 ? */
		xedt3 = xedt;
		for (hd_fd = 0; hd_fd < MAX_TCS; hd_fd++, xedt3++)
			if (strncmp(xedt3->drv_name, "SCSI", 4) == 0)
				break;
		if (hd_fd >= MAX_TCS) {
			printf ("SCSI Host Adaptor not present\n");
			exit (BAD);
		}
		if (hd_fd != 7) {
			printf ("SCSI BUS ID is on %d.\n", hd_fd);
		}

		/* Is the Hard Disk Correct */
		xedt3 = xedt;
		for (hd_fd = 0; hd_fd < MAX_TCS; hd_fd++, xedt3++)
			if (strncmp(xedt3->drv_name, "SD01", 4) == 0)
				break;
		if (hd_fd >= MAX_TCS) {
			fprintf (stderr, "Can not find SCSI Hard Disk, check cables.\n");
			exit (BAD);
		}
		if ((hd_fd != 0) || (BUS_OCCUR(xedt3->ha_slot) != 0)) {
			printf ("Hard Disk on Host Adaptor Slot = %d, Target ID = %d.\n", BUS_OCCUR(xedt3->ha_slot), hd_fd);
			printf ("Should be on Host Adaptor Slot = 0, Target ID = 0.\n");
			exit (BAD);
		}

	if (!sflag) {
		/* Is the Cartridge Tape Correct */
		xedt3 = xedt;
		for (hd_fd = 0; hd_fd < MAX_TCS; hd_fd++, xedt3++) {
			if (strncmp(xedt3->drv_name, "ST01", 4) == 0)
			   if (hd_fd == 3) {
				break;
			   } else {
				tp_flag = hd_fd;
			   }
		}
		if (hd_fd >= MAX_TCS) {
		     fflush (stdout);
		     if (tp_flag != 0) {
			fprintf (stderr, "Streaming Tape on Target ID = %d.\n", tp_flag);
			fprintf (stderr, "Should be on Host Adaptor Slot = 0, Target ID = 3.\n");
		     } else {
			fprintf (stderr, "Can not find SCSI Cartridge Tape\n");
		     }
		     exit (BAD);
		}
		if ((hd_fd != 3) || (BUS_OCCUR(xedt3->ha_slot) != 0)) {
			fflush (stdout);
			fprintf (stderr, "Streaming Tape on Host Adaptor Slot = %d, Target ID = %d.\n", BUS_OCCUR(xedt3->ha_slot), hd_fd);
			fprintf (stderr, "Should   be   on  Host Adaptor Slot = 0, Target ID = 3.\n");
			exit (BAD);
		}
		if (tp_flag != 0) {
			fflush (stdout);
			fprintf (stderr, "More than one Streaming Tape drive found.  You must install off the\n");
			fprintf (stderr, "streaming tape drive on Host Adaptor Slot = 0, Target ID = 3.\n");
		}
		exit (0);
	}
#else
		exit (BAD);
#endif
	}
	scsi = 0;
	errno = 0;
	if ((hd_fd = open("/dev/rdsk/0s0", O_RDONLY)) < 0) {
/*
** If this fails, we ain't got no hard disk either SCSI or Non-SCSI!
*/
		if (sflag) {
			printf ("\nNo Hard Disk present.\nCheck your setup configuration.\n");
			exit (3);
		}
	} else {
	     if ((ioctl(hd_fd, B_GETTYPE, &bus_type) >= 0) &&
		(strncmp (bus_type.bus_name, "scsi", 4) == 0)) {
		     if (errno == 0)
			scsi = 1;
	     }
	     close (hd_fd);
	}
	if (sflag) { /* Do we have a bootable scsi ? */
#ifdef SCSI
		if (!error)
			exit (1);
#endif
		exit (cmos_hd() == 0);
	}
	switch (sysi86(SI86RDID)) {
		case 0: break; /* look at memory layout now */
		case 'E': exit (CASCADE2);
		case 'F': exit (CASCADE3);
		case 'G': if (scsi)
				exit (E333A);
			  exit (CASCADE4);
		case 'K': exit (CASCADE6);
		case 'L': exit (CASCADE2U);
		case 'M': exit (CASCADE5U);
		case 'R': exit (EN8R1);
		default: printf ("ID byte is: %d\n", sysi86(SI86RDID));
			 break;
	}

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
		exit(ATT640);
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

/*
** Return 3 if 80386, 4 if 80486, ....
*/

#define AS_486

#ifdef AS_486
asm int
check_486()
{
	push %eax
	bswap %eax
	pop %eax
}
#else
/*
** 4.1.6 compiler (assembler) doesn't know about bswap (new 486 instruction),
** therefore, for now, lets just exit 3.  Later, perhaps, we can do something
** else.
*/
check_486()
{
	exit (3);
}
#endif /* AS_486 */

processor()
{

	switch (sysi86(SI86RDID)) {
		case 'K': return (4);
		default: break;
	}
	signal (SIGILL,bswap_sig);
	signal (SIGFPE,bswap_sig);	/* needed if fp is microsoft */
	check_486();
	return (4); /* Wasn't caught in bswap_sig, there it's a 486 */
}

void
bswap_sig()
{
	exit (3);
}

arch()
{
	switch (sysi86(SI86RDID)) {
		case 'K': return (1); /* EISA Architecture */
		case 'R': return (1); /* EISA Architecture */
		default: break;
	}
	/* Look at how crash(1M) does the od eisa_bus stuff */
	return (0); /* AT Architecture */
}

cmos_hd()
{
	int	cnt, fd;
	unsigned char buf[2];
	unsigned char dbuf[2];

	buf[0] = FDTB;
	cnt = 0;
	if ((fd = open("/dev/cram", O_RDONLY)) < 0) {
		fprintf (stderr, "Can not open /dev/cram\n");
		exit (3);
	}
	ioctl(fd, CMOSREAD, buf);

	if ((buf[1] >> 4) & 0x0F)  { /* Hard Disk 0 (C:) */
		dbuf[0] = DCEB;
		ioctl (fd, CMOSREAD, dbuf);
#ifdef DEBUG
		printf ("Hard Disk drive 0 = %d\n", dbuf[1]);
#endif
		cnt++;
	}
	if (buf[1] & 0x0F) {         /* Hard Disk 1 (D:) */
		dbuf[0] = DDEB;
		ioctl (fd, CMOSREAD, dbuf);
#ifdef DEBUG
		printf ("Hard Disk drive 1 = %d\n", dbuf[1]);
#endif
		cnt++;
	}
	close (fd);
	return (cnt);
}
