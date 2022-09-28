/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:vfssw.c	1.1.4.1"

/*
 * This file contains code for the crash function vfssw.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/var.h>
#include <sys/fstyp.h>
#include <sys/vfs.h>
#include "crash.h"

struct syment  *Nfstyp;		/*namelist symbol pointers */
struct syment *Vfssw;
extern struct syment *Vfs;

/* get arguments for vfssw function */
int
getvfssw()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	int nfstypes;

	if(!Vfssw)
		if((Vfssw = symsrch("vfssw")) == NULL)
			error("vfssw not found in symbol table\n");
	if(!Nfstyp)
		if((Nfstyp = symsrch("nfstype")) == NULL)
			error("nfstyp not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"pw:")) !=EOF) {
		switch(c) {
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	readmem((long)Nfstyp->n_value,1,-1,(char *)&nfstypes,
		sizeof (int), "number of file systems types");
	fprintf(fp,"FILE SYSTEM SWITCH TABLE SIZE = %d\n",nfstypes-1);
	fprintf(fp,"SLOT   NAME     FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nfstypes,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg1 == 0) {
				fprintf(fp,"0 is out of range\n");
				continue;
			}
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prvfssw(all,slot,phys,addr,nfstypes);
			else {
				if(arg1 < nfstypes)
					slot = arg1;
				else addr = arg1;
				prvfssw(all,slot,phys,addr,nfstypes);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 1; slot < nfstypes; slot++)
		prvfssw(all,slot,phys,addr,nfstypes);
}

/* print vfs switch table */
int
prvfssw(all,slot,phys,addr,max)
int all,slot,phys,max;
long addr;
{
	struct vfssw vfsswbuf;
	char name[FSTYPSZ+1];

	readbuf(addr,(long)(Vfssw->n_value+slot*sizeof vfsswbuf),phys,-1,
		(char *)&vfsswbuf,sizeof vfsswbuf,"file system switch table");
	if(!vfsswbuf.vsw_name && !all)
		return; 
	if(addr > -1)
		slot = getslot(addr,(long)Vfssw->n_value,sizeof vfsswbuf,phys,max);
	if(slot == -1)
		fprintf(fp,"  - ");
	else
		fprintf(fp, "%4d", slot);
	readmem((long)vfsswbuf.vsw_name,1,-1,name,sizeof name,"fs_name");
	fprintf(fp,"   %-10s", name); 
	fprintf(fp," %x\n",
		vfsswbuf.vsw_flag);
}
