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

#ident	"@(#)boot:boot/at386/boot.c	1.1.5.1"

#include "../sys/boot.h"
#include "../sys/libfm.h"
#include "../sys/error.h"
#include "../sys/dib.h"

#define DEFAULTFILE	"/etc/default/boot"
#define NOBFS_REVERT_ROOT  "BFS not fully populated, REVERTING back to ROOT\n" 

char	bootstring[B_STRSIZ];
struct	bootinfo	binfo;
extern	int memsz;
int 	scsidiskflag;
ushort	ourDS;

extern	void BL_init();
extern	void BL_file_open();

extern	struct dib  dib;
extern	ulong	status;
extern	off_t	boot_delta;
extern	off_t 	root_delta;
extern	bfstyp_t boot_fs_type;
extern	bfstyp_t root_fs_type;

extern struct biosparm {
	unsigned char	maxsec,
			maxcyl,
			ndrive,
			maxhd;
} hd0parm, hd1parm;


main()
{
	int	i, t;
	int	load_it;
	paddr_t	addr;

	ourDS = getDS();

	debug(printf("boot: in main()\n"));

	/* Zero the binfo struct (bss is not initialized) */

	for (i = 0; i < sizeof(binfo); i++) 
		((char *)&binfo)[i] = 0;

	/* SCSI-based disk sub-system assumed if no integral
	 * disks present
	 */

	scsidiskflag = ! (prdcmos(0x12) & 0xff);

	/* initialize the stand alone disk driver */
	/* gets the file system offset for BFS and root file system  */

	BL_init();

	/*    NOTE MB II users, this is an important aid for your debugging */
	/* In order to solve the chicken or the egg problem of BFS
	   configuration, we'll attempt to open some key files in the BFS
	   filesystem, and if BFS is partitioned but not fully prepared,
  	   revert to using S5.  Once BFS is fully configured and populated
	   with /etc/default/boot and /unix, open will succeed and
	   not switch to S5 type */

	switch(boot_fs_type) {

	/* unknown in case of floppy boot or a hard disk with no BFS */
	case UNKNOWN:
		boot_fs_type = root_fs_type;	
		boot_delta = root_delta;
		break;
	case BFS: 
		/* check if /stand is populated well */
		BL_file_open("boot", &dib, &status);

		if (status != E_OK) {
			printf(NOBFS_REVERT_ROOT);
			boot_fs_type = root_fs_type;
			boot_delta = root_delta;
			break;
		}

		BL_file_open("unix", &dib, &status);

		if (status != E_OK) {
			printf(NOBFS_REVERT_ROOT);
			boot_fs_type = root_fs_type;
			boot_delta = root_delta;
			break;
		}
		break;
	default:
		printf("boot.c: Invalid boot file system type");
		break;
	}	

	/* set up the binfo.hdparams structure for the kernel */

	if (scsidiskflag || (prdcmos(0x12) & 0xf0)) {
		debug(printf("Drive 0: "));
		bhdparam(0x104L, &(binfo.hdparams[0]));
		hdpverify(&hd0parm, &(binfo.hdparams[0]));
	} else
		binfo.hdparams[0].hdp_ncyl = 0;

	if (prdcmos(0x12) & 0x0f) {
		debug(printf("Drive 1: "));
		bhdparam(0x118L, &(binfo.hdparams[1]));
		hdpverify(&hd1parm, &(binfo.hdparams[1]));
	} else
		binfo.hdparams[1].hdp_ncyl = 0;

	/* Write the ID bytes F000:ED00 (actually fed00 virtual) from BIOS ROM
	   into the binfo structure */
	prdrom(0xfed00, physaddr(binfo.id), 5);

	debug(printf("memsize (in 1K) = 0x%x\n", memsz));

	/* read the defaults file */

	bdefault(DEFAULTFILE);

	/* load and run the initprog, as necessary */

	if ( strlen(initprog) != 0 ) {
		if ( (addr = bload( initprog, FALSE, 0xA000 )) != 0 ) {
			debug(printf("Calling bstart(0x%x): \n", addr));
			bstart(addr, TRUE);
		} else {
			printf("\nboot: Cannot load initprog: %s", initprog);
			halt();
		}
	}

	/* size memory */

	bmemsize();

	/* 
	 * setup the defaults for the 'user interface' and
	 * clear the keyboard buffer 
	 */

	strncpy(bootstring, defbootstr, B_STRSIZ);

	while ( ischar() )
		(void)getchar();

	/* present the bootmsg, bootprompt, etc. */

	load_it = autoboot;
	while (!load_it || (addr = loadprog()) == 0) {
		while (ischar())
			(void)getchar();

		printf("\n\n%s ", bootprompt);

		for (t = timeout; !ischar() && (timeout == 0 || t-- > 0);)
			wait1s();

		if (!ischar() || bgets(bootstring, B_STRSIZ) == 0) {
			strncpy(bootstring, defbootstr, B_STRSIZ);
			printf("%s", bootstring);
		}

		printf("\n\n\n\n");
		load_it = TRUE;
	}

	/* we used binfo.hdparams[0] to hold parameters for SCSI boot
	 * reset it to 0 so that the integral hard disk driver doesn't
	 * inadvertently think that there is an integral disk 0 out there
	 */

	if (scsidiskflag)
		binfo.hdparams[0].hdp_ncyl = 0;
			

	/* Now we can mark the bootstrap space available */

	for (i = 0; i < binfo.memavailcnt; i++) {
		if (binfo.memavail[i].flags & B_MEM_BOOTSTRAP) {
			binfo.memavail[i].extent += BOOTSIZE;
			binfo.memavail[i].flags &= ~B_MEM_BOOTSTRAP;
			break;
		}
	}

	/* Copy the booted program name into bargv[0] */

	strncpy(binfo.bargv[0], bootstring, B_STRSIZ);

	/* calculate checksum */

	binfo.checksum = 0;
	for ( i = 0; i < (sizeof(struct bootinfo) - sizeof(int)); i++ )
		binfo.checksum += ((char *)&binfo)[i];

	/* copy binfo down to the correct low memory location */

	memcpy( (paddr_t)0x600, physaddr(&binfo), sizeof(struct bootinfo) );

	debug(printf("\nCalling bstart(0x%x): \n", addr));

	bstart( addr, FALSE );
}


loadprog()
{
	printf("\n%s", bootmsg);
	return bload(bootstring, TRUE, 0);
}


hdpverify(bios_parm, hdparm)
	struct biosparm	*bios_parm;
	struct hdparams	*hdparm;
{
	int	ncyl, nsec, nhead;

	ncyl = bios_parm->maxcyl + (((int)bios_parm->maxsec & 0xC0) << 2) + 1;
	nsec = (bios_parm->maxsec & 0x3F);
	nhead = bios_parm->maxhd + 1;
	debug(printf("ncyl %d  ", ncyl));
	debug(printf("nsec %d  ", nsec));
	debug(printf("nhead %d\n", nhead));
	if ((hdparm->hdp_ncyl != ncyl && hdparm->hdp_ncyl != ncyl + 1) ||
	    hdparm->hdp_nsect != nsec ||
	    hdparm->hdp_nhead != nhead) {
		hdparm->hdp_ncyl = ncyl;
		hdparm->hdp_nsect = nsec;
		hdparm->hdp_nhead = nhead;
		hdparm->hdp_precomp = 0xFFFF;
		hdparm->hdp_lz = ncyl;
	}
}
