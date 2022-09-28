/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:vfs.c	1.4.10.1"

/*
 * This file contains code for the crash functions:  vfs (mount).
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/var.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/nserve.h>
#include <sys/list.h>
#include <sys/rf_messg.h>
#include <sys/rf_comm.h>
#include "crash.h"


extern struct syment *Vfs, *Vfssw;	/* namelist symbol */

/* get arguments for vfs (mount) function */
int
getvfsarg()
{
	int all = 0;
	int phys = 0;
	int c;
	unsigned long vfsp;

	if(!Vfs)
		if(!(Vfs = symsrch("rootvfs")))
			error("vfs list not found\n");
	if(!Vfssw)
		if((Vfssw = symsrch("vfssw")) == NULL)
			error("vfssw not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}

	fprintf(fp," FSTYP  BSZ  MAJ/MIN      FSID    VNCOVERED   PDATA      BCOUNT  FLAGS\n");
	if(args[optind]){
		do {
			vfsp = (unsigned long)strcon(args[optind], 'h');
			if(vfsp == (unsigned long)-1) 
				continue;
			else
				prvfs(all,phys,vfsp);
			vfsp = (unsigned long)-1;
		}while(args[++optind]);
	} else {
		readmem(Vfs->n_value,1,-1,(long)&vfsp, sizeof vfsp,
			"head of vfs list");
		while (vfsp)
			vfsp = (unsigned long) prvfs(all,phys,vfsp);
	}
}

/* print vfs list */
prvfs(all,phys,addr)
int all,phys;
unsigned long addr;
{
	struct vfs vfsbuf;
	char fsname[MAXDNAME+1];
	extern long lseek();
	short i = 0;
	struct vfssw vfsswbuf;

	readbuf(addr,0,phys,-1,(char *)&vfsbuf,sizeof vfsbuf,"vfs list");
	readmem((long)(Vfssw->n_value+vfsbuf.vfs_fstype*sizeof vfsswbuf),1,-1,
		(char *)&vfsswbuf,sizeof vfsswbuf,"file system switch table");
	readmem((long)vfsswbuf.vsw_name,1,-1,fsname,sizeof fsname,"fs_name");

	fprintf(fp,"%6s %4u",
		fsname,
		vfsbuf.vfs_bsize);
	fprintf(fp," %4u,%-5u",
		getemajor(vfsbuf.vfs_dev),
		geteminor(vfsbuf.vfs_dev));
	fprintf(fp, " %8x   %8x  %8x %10d",
		vfsbuf.vfs_fsid.val[0],
		vfsbuf.vfs_vnodecovered,
		vfsbuf.vfs_data,
		vfsbuf.vfs_bcount);
	fprintf(fp,"  %s%s%s%s%s%s%s",
		(vfsbuf.vfs_flag & VFS_RDONLY) ? " rd" : "",
		(vfsbuf.vfs_flag & VFS_MLOCK) ? " lck" : "",
		(vfsbuf.vfs_flag & VFS_MWAIT) ? " wait" : "",
		(vfsbuf.vfs_flag & VFS_NOSUID) ? " nosu" : "",
		(vfsbuf.vfs_flag & VFS_REMOUNT) ? " remnt" : "",
		(vfsbuf.vfs_flag & VFS_NOTRUNC) ? " notr" : "",
		(vfsbuf.vfs_flag & VFS_UNLINKABLE) ? " nolnk" : "");
	fprintf(fp,"\n");
	return((long)vfsbuf.vfs_next);
}

