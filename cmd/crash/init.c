/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:init.c	1.7.9.1"

/*
 * This file contains code for the crash initialization.
 */

#include <sys/param.h>
#include <a.out.h>
#include <signal.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef i386
#include <sys/psw.h>
#include <sys/pcb.h>
#endif
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#ifndef i386
#include <sys/sbd.h>
#endif
#include <sys/cdump.h>
#include <sys/immu.h>
#ifndef i386
#include <sys/nvram.h> 
#endif
#include <sys/var.h>
#ifdef i386
#include <sys/sysi86.h>
#else
#include <sys/sys3b.h>
#endif
#include <sys/sysmacros.h>
#include "crash.h"

int	nmlst_tstamp ,		/* timestamps for namelist and cdumpfiles */
	dmp_tstamp ;

#ifdef __STDC__
struct syment dummyent = { (long)0, (long)0, (long)0, (short)0,
				(unsigned short)0, (char)0, (char)0 };
#else
struct syment dummyent;
#endif

extern char *strtbl ;		/* pointer to string table in symtab.c */
extern char *dumpfile;		
extern char *namelist;
#ifndef i386
	extern struct fw_nvr fwnvram;	/* nvram buffers */
	extern struct unx_nvr unxnvram;
	extern struct xtra_nvr xtranvram;
	extern char *mmusrama, *mmusramb;
	extern char * sramapt[4];
	extern SRAMB srambpt[4];
	long fltcrptr;			/* configuration registers */
	long fltarptr;
	long conrptr;
	struct nvparams nvp;			/* structure for sys3b call  */
#endif
extern int active;		/* flag for active system */
extern unsigned long vtop();
extern long lseek();
extern char *malloc();
struct syment *sp;		/* pointer to symbol table */
struct user *ubp;		/* pointer to ublock buffer */
extern struct syment *File, *Inode, *Vfs, *V,
	*Panic, *Curproc;	/* namelist symbol pointers */
struct syment *Inode, *Ninode, *Ifreelist, *Snode, *Sndd, *Rcvd, *Nrcvd, *Ngrps;
struct syment *S5vnodeops, *Duvnodeops, *Spec_vnodeops, *Fifo_vnodeops, *Prvnodeops, *Ufs_vnodeops;

/* initialize buffers, symbols, and global variables for crash session */
int
init()
{
	int offset ;
	struct crash_hdr crash_hdr ;
	struct syment	*ts_symb = NULL;
	struct stat	fstatb, memstatb;
	extern void sigint();
	
	if((mem = open(dumpfile, 0)) < 0)	/* open dump file, if error print */
		fatal("cannot open dump file %s\n",dumpfile);
	/*
	 * Set a flag if the dumpfile is of an active system.
	 */
	fstat(mem, &fstatb);
	stat("/dev/mem", &memstatb);
	if ((fstatb.st_mode & S_IFMT) == S_IFCHR &&
			major(fstatb.st_rdev) == major(memstatb.st_rdev))
		active = 1;
#ifndef i386
	else {
		offset = SPMEM - MAINSTORE - CHDR_OFFSET ;
		if(lseek(mem,(long)offset, 0) == -1
		  || read(mem,(char *)(&crash_hdr),sizeof (crash_hdr)) !=
			 sizeof (crash_hdr)
		  || strcmp(crash_hdr.sanity,CRASHSANITY) != 0)
			active = 1;
	}
	if(active) {
		/* read the nvram info */
		
		/* firmware nvram */
		/* load up the nvparams structure */
		nvp.addr = (char *) FW_NVR;
		nvp.data = (char *) &fwnvram;
		nvp.cnt = sizeof(struct fw_nvr);
		sys3b( RNVR, &nvp, 0);


		/* unix nvram */
		nvp.addr = (char *) UNX_NVR;
		nvp.data = (char *) &unxnvram;
		nvp.cnt = sizeof(struct unx_nvr);
		sys3b( RNVR, &nvp, 0);
		
		/* extra nvram */
		nvp.addr = (char *) XTRA_NVR;
		nvp.data = (char *) &xtranvram;
		nvp.cnt = sizeof(struct xtra_nvr);
		sys3b( RNVR, &nvp, 0);

		/* Read the kernel section srama's and sramb's.  */
		sramapt[0] = (char *) *srama;
		sramapt[1] = (char *) *(srama + 1);
		srambpt[0] = *sramb;
		srambpt[1] = *(sramb + 1);
	}
	else
	{
		/* go get the mmu info */
		offset += sizeof(crash_hdr) ;
		if(lseek( mem, (long)offset, 0) == -1)
			fatal("seek error on mmu information\n");;
		if(read(mem,(char *)sramapt,4*(sizeof(char *))) != 4*(sizeof(char *)))
			fatal("read error on srama entries\n");
		if(read(mem,srambpt, 4*(sizeof(SRAMA))) != 4*(sizeof(SRAMA)))
				fatal("read error on sramb entries\n");
		/*
		 * Read the saved Configuration registers.
		 * fltcrptr and fltarptr are read here,
		 * to preserve ordering, for crash usage
		 * the values are read from nvram which more accurately
		 * relects the state of the machine at the time of the panic
		 */
		if(read(mem, (char *)&fltcrptr, sizeof(long)) != sizeof (long))
			fatal(" read error on Fault Code Register\n");
		if(read(mem, (char *)&fltarptr, sizeof(long)) != sizeof(long))
			fatal("read error on Fault Address Register\n");
		if(read(mem, (char *)&conrptr, sizeof(long)) != sizeof(long))
			fatal("read error on Configuration Register\n");
		/*
		 * "lseek" into memory and read the dumped
		 * contents of NVRAM and the mmu info
		 */

		/* read in the firmware nvram info */
		offset = SPMEM - MAINSTORE - NVR_OFFSET ;
		if(lseek(mem, (long)offset, 0) == -1)
			fatal("seek error on firmware nvram\n");
		if( read(mem, (char *)&fwnvram, sizeof(struct fw_nvr)) 
				!=  sizeof (struct fw_nvr) )
			fatal( "read error on firmware nvram\n");

		/* read in the unix nvram info */
		if(lseek( mem, (long)(offset + UNX_OFSET), 0) == -1)
			fatal("seek error on unix nvram\n");
		if( read(mem, &unxnvram, sizeof(struct unx_nvr)) 
				!=  sizeof (struct unx_nvr) )
			fatal( "read error on unix nvram\n");
		
		/* read in the xtra nvram info */
		if(lseek( mem, (long)(offset + XTRA_OFSET), 0) == -1)
			fatal("seek error on xtra nvram\n");
		if( read(mem, (char *)&xtranvram, sizeof(struct xtra_nvr)) 
				!=  sizeof (struct xtra_nvr) )
			fatal( "read error on unix nvram\n");

	}
#endif
	rdsymtab();			/* open and read the symbol table */

#ifndef i386	/* there is not a timestamp in the 386 kernel */

	/* check timestamps */
	if(nmlst_tstamp == 0 || !(ts_symb = symsrch("crash_sync")))
		fprintf(fp,"WARNING - could not find timestamps, timestamps not checked\n") ;
	else
	{
		if((lseek(mem,vtop((long)ts_symb->n_value,-1),0) == -1)
		  || read(mem, (char *)&dmp_tstamp,sizeof(int)) != sizeof(int))
			fatal("could not process dumpfile with supplied namelist %s\n",namelist) ;

		if(dmp_tstamp != nmlst_tstamp)
		{
			fprintf(fp,"WARNING - namelist and system image not of the same vintage\n") ;
			fprintf(fp,"          crash results unpredictable\n") ;
		}
	}

#endif

	if(!(V = symsrch("v")))
		fatal("var structure not found in symbol table\n");
	if(!(File = symsrch("file")))
		fatal("file not found in symbol table\n");
	if(!(Vfs = symsrch("rootvfs")))
		fatal("vfs not found in symbol table\n");
	if(!(Panic = symsrch("panicstr")))
		fatal("panicstr not found in symbol table\n");
	if(!(Curproc = symsrch("curproc")))
		fatal("curproc not found in symbol table\n");
	if(!(Inode = symsrch("inode")))
		error("S5 inode table not found in symbol table\n");
	if(!(Sndd = symsrch("sndd")))
		Sndd = &dummyent;
	if(!(Snode = symsrch("stable")))
		error("snode table not found\n");
	if(!(S5vnodeops = symsrch("s5vnodeops")))
		S5vnodeops = &dummyent;
	if(!(Ufs_vnodeops = symsrch("ufs_vnodeops")))
		Ufs_vnodeops = &dummyent;

	if(!(Duvnodeops = symsrch("duvnodeops")))
		Duvnodeops = &dummyent;
	if(!(Spec_vnodeops = symsrch("spec_vnodeops")))
		error("spec_vnodeops not found\n");
	if(!(Fifo_vnodeops = symsrch("fifo_vnodeops")))
		error("fifo_vnodeops not found\n");
	if(!(Prvnodeops = symsrch("prvnodeops")))
		Prvnodeops = &dummyent;
	if(!(Ngrps = symsrch("ngroups_max")))
		error("ngroups_max not found in symbol table\n");

	readmem((long)V->n_value,1,-1,(char *)&vbuf,
		sizeof vbuf,"var structure");

	/* Allocate ublock buffer */
	ubp = (user_t*)malloc((unsigned)(USIZE*NBPC));

	Procslot = getcurproc();
	/* setup break signal handling */
	if(signal(SIGINT,sigint) == SIG_IGN)
		signal(SIGINT,SIG_IGN);
}

