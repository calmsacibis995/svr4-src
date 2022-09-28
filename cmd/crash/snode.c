/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:snode.c	1.2.9.1"

/*
 * This file contains code for the crash functions:  snode.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/var.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/conf.h>
#include <sys/fs/snode.h>
#include "crash.h"

extern char *vnode_header;

extern struct syment *Vfs, *Vfssw, *File;	/* namelist symbol pointers */ 
struct syment *Snode;

/* get arguments for snode function */
int
getsnode()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	char *heading = "SLOT  MAJ/MIN    REALVP     COMMONVP  NEXTR  SIZE    COUNT FLAGS\n";


	if(!Snode)
		if(!(Snode = symsrch("stable")))
			error("snode table not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	fprintf(fp,"SNODE TABLE SIZE = %d\n", STABLESIZE);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(STABLESIZE,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prsnode(all,full,slot,phys,addr,heading);
			else {
				if(arg1 < STABLESIZE)
					slot = arg1;
				else addr = arg1;
				prsnode(all,full,slot,phys,addr,heading);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < STABLESIZE; slot++)
		prsnode(all,full,slot,phys,addr,heading);
}



/* print snode table */
int
prsnode(all,full,slot,phys,addr,heading)
int all,full,slot,phys;
long addr;
char *heading;
{
	struct snode *snp, snbuf;
	extern long lseek();

	if(addr == -1)
		readbuf(addr,(long)(Snode->n_value+slot*sizeof snp),phys,-1,
			(char *)&snp,sizeof snp,"snode address");
	if(snp == 0)
		return;
	readbuf(addr,(long)snp,phys,-1,(char *)&snbuf,sizeof snbuf,"snode table");

	while( 1 )
	{
		if(!snbuf.s_count && !all)
				return ;
		if(full)
			fprintf(fp,"%s",heading);

		if(slot == -1)
			fprintf(fp,"  - ");
		else fprintf(fp,"%4d",slot);
		fprintf(fp," %4u,%-5u %8x    %8x %4d %5d    %5d ",
			getemajor(snbuf.s_dev),
			geteminor(snbuf.s_dev),
			snbuf.s_realvp,
			snbuf.s_commonvp,
			snbuf.s_nextr,
			snbuf.s_size,
			snbuf.s_count);

		fprintf(fp,"%s%s%s%s%s\n",
			snbuf.s_flag & SLOCKED ? " lk" : "",
			snbuf.s_flag & SUPD ? " up" : "",
			snbuf.s_flag & SACC ? " ac" : "",
			snbuf.s_flag & SWANT ? " wt" : "",
			snbuf.s_flag & SCHG ? " ch" : "");
		if(full)
		{
			/* print vnode info */
			fprintf(fp,"VNODE :\n");
			fprintf(fp, vnode_header);
			prvnode(&snbuf.s_vnode);
			fprintf(fp,"\n");
		}

		if((addr != -1) || (snbuf.s_next == NULL))
			return;
		snp = snbuf.s_next;
		readbuf(addr,(long)snp,phys,-1,(char *)&snbuf,sizeof snbuf,
			"snode table");
	}
}


