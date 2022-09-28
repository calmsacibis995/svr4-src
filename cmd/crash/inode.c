/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:inode.c	1.14.15.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 * This file contains code for the crash functions:  vnode, inode, file.
 */
#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/var.h>
#include <sys/vfs.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/list.h>
#include <sys/rf_messg.h>
#include <sys/rf_comm.h>
#include <sys/fs/s5inode.h>
#include <sys/fs/xnamnode.h>
#include <sys/cred.h>
#define _KERNEL
#include <sys/stream.h>
#undef _KERNEL
#include "crash.h"

char *vnode_header = "VCNT VFSMNTED     VFSP  STREAMP VTYPE  RDEV        VDATA VFILOCKS VFLAG\n";

extern struct syment *Vnode, *Streams, *Vfs, *File;	/* namelist symbol pointers */
extern struct syment *Inode, *Ninode, *Ifreelist, *Snode, *Sndd, *Rcvd, *Nrcvd, *Ngrps;
extern struct syment *S5vnodeops, *Ufs_vnodeops, *Duvnodeops, *Spec_vnodeops, *Fifo_vnodeops, *Prvnodeops;

struct vnode vnbuf;			/* buffer for vnode */

long iptr;
struct inode ibuf;		/* buffer for s5inode */
long ninode;			/* size of S5 inode table */

/* get arguments for vnode function */
int
getvnode()
{
	unsigned long addr = -1;
	int phys = 0;
	int c;


	optind = 1;
	while((c = getopt(argcnt,args,"pw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
					break;
		}
	}
	if(args[optind]){
		fprintf(fp, vnode_header);
		do{
			if((addr = strcon(args[optind++], 'h')) == -1)
				error("\n");
			readbuf(addr,0,phys,-1,(char *)&vnbuf,sizeof vnbuf,"vnode");
			prvnode(&vnbuf);
		}while(args[optind]);

		fprintf(fp, "\n");
	}
	else longjmp(syn,0);

}

prvnode(vnptr)
struct vnode *vnptr;
{
	

	fprintf(fp,"%3d  %8x %8x %8x ",
		vnptr->v_count,
		vnptr->v_vfsmountedhere,
		vnptr->v_vfsp,
		vnptr->v_stream);
	switch(vnptr->v_type){
		case VREG :	fprintf(fp, "  f      -     "); break;
		case VDIR :	fprintf(fp, "  d      -     "); break;
		case VLNK :	fprintf(fp, "  l      -     "); break;
		case VCHR :
				fprintf(fp, "  c  %4u,%-5u",
					getemajor(vnptr->v_rdev),
					geteminor(vnptr->v_rdev));
				break;
		case VBLK :
				fprintf(fp, "  b  %4u,%-5u",
					getemajor(vnptr->v_rdev),
					geteminor(vnptr->v_rdev));
				break;
		case VFIFO :	fprintf(fp, "  p      -     "); break;
		case VNON :	fprintf(fp, "  n      -     "); break;
		default :	fprintf(fp, "  -      -     "); break;
	}
	fprintf(fp," %8x %8x",
		vnptr->v_data,
		vnptr->v_filocks);
	fprintf(fp,"%s%s\n",
		vnptr->v_flag & VROOT ? " root" : "");
}

/* get arguments for S5 inode function */
int
getinode()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int free = 0;
	long next;
	int list = 0;
	struct ifreelist freebuf;
	int c;
	char *heading = "SLOT  MAJ/MIN   INUMB RCNT LINK   UID   GID     SIZE    MODE FLAGS\n";


	if(!Inode)
		if(!(Inode = symsrch("inode")))
			error("S5 inode table not found in symbol table\n");
	if(!Ninode)
		if(!(Ninode = symsrch("ninode")))
			error("cannot determine S5 inode table size\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efprlw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'r' :	free = 1;
					break;
			case 'l' :	list = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	readmem(Ninode->n_value,1,-1,(char *)&ninode, sizeof ninode,
		"size of S5 inode table");
	readmem((long)(Inode->n_value),1,-1,(char *)&iptr, sizeof iptr, "S5 inode");

	if(list)
		listinode();
	else {
		fprintf(fp,"INODE TABLE SIZE = %d\n", ninode);
		if(!full)
			fprintf(fp,"%s",heading);
		if(free) {
			if(!Ifreelist)
				if(!(Ifreelist = symsrch("ifreelist")))
					error("ifreelist not found in symbol table\n");
			readmem((long)Ifreelist->n_value,1,-1,(char *)&freebuf,
			sizeof freebuf,"ifreelist buffer");
			next = (long)freebuf.av_forw;
			while(next) {
				prinode(1,full,slot,phys,next,heading);
				next = (long)ibuf.av_forw;
				if(next == (long)Ifreelist->n_value)
					next = 0;
			}
		}	
		else if(args[optind]) {
			all = 1;
			do {
				getargs(ninode,&arg1,&arg2);
				if(arg1 == -1) 
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						prinode(all,full,slot,phys,addr,heading);
				else {
					if(arg1 < ninode)
						slot = arg1;
					else addr = arg1;
					prinode(all,full,slot,phys,addr,heading);
				}
				slot = addr = arg1 = arg2 = -1;
			}while(args[++optind]);
		}
		else for(slot = 0; slot < ninode; slot++)
			prinode(all,full,slot,phys,addr,heading);
	}
}


int
listinode()
{
	char inodebuf[500];
	int i,j;
	long next;
	struct ifreelist freebuf;

	if(!Ifreelist)
		if(!(Ifreelist = symsrch("ifreelist")))
			error("ifreelist not found in symbol table\n");
	for(i = 0; i < ninode; i++)
		inodebuf[i] = 'n';

	for(i = 0; i < ninode; i++) {
		readmem((long)(iptr+i*sizeof ibuf),1,-1,
			(char *)&ibuf,sizeof ibuf,"inode table");
		if(ibuf.i_vnode.v_count != 0)
			inodebuf[i] = 'u';
	}
	readmem((long)Ifreelist->n_value,1,-1,(char *)&freebuf,
		sizeof freebuf,"ifreelist buffer");
	next = (long)freebuf.av_forw;
	while(next) {
		i = getslot((long)next,(long)iptr,sizeof ibuf,0,ninode);
		readmem((long)next,1,-1,(char *)&ibuf,sizeof ibuf,"S5 inode");
		inodebuf[i] = 'f';
		if(ibuf.i_vnode.v_count != 0)
			inodebuf[i] = 'b';
		next = (long)ibuf.av_forw;
		if(next == (long)Ifreelist->n_value)
			next = 0;
	}
	fprintf(fp,"The following inodes are in use:\n");
	for(i = 0,j = 0; i < ninode; i++) {
		if(inodebuf[i] == 'u') {
			if(j && (j % 10) == 0)
				fprintf(fp,"\n");
			fprintf(fp,"%3d    ",i);
			j++;
		}
	}
	fprintf(fp,"\n\nThe following inodes are on the freelist:\n");
	for(i = 0,j=0; i < ninode; i++) {
		if(inodebuf[i] == 'f') {
			if(j && (j % 10) == 0)
				fprintf(fp,"\n");
			fprintf(fp,"%3d    ",i);
			j++;
		}
	}
	fprintf(fp,"\n\nThe following inodes are on the freelist but have non-zero reference counts:\n");
	for(i = 0,j=0; i < ninode; i++) {
		if(inodebuf[i] == 'b') {
			if(j && (j % 10) == 0)
				fprintf(fp,"\n");
			fprintf(fp,"%3d    ",i);
			j++;
		}
	}
	fprintf(fp,"\n\nThe following inodes are in unknown states:\n");
	for(i = 0,j = 0; i < ninode; i++) {
		if(inodebuf[i] == 'n') {
			if(j && (j % 10) == 0)
				fprintf(fp,"\n");
			fprintf(fp,"%3d    ",i);
			j++;
		}
	}
	fprintf(fp,"\n");
}

/* print inode table */
int
prinode(all,full,slot,phys,addr,heading)
int all,full,slot,phys;
unsigned long addr;
char *heading;
{
	char ch;
	int i;

	readbuf(addr,(iptr+slot*sizeof ibuf),phys,-1,
		(char *)&ibuf,sizeof ibuf,"inode table");

	if(!ibuf.i_vnode.v_count && !all)
			return ;
	if(full)
		fprintf(fp,"%s",heading);
	if(addr > -1) 
		slot = getslot(addr,(long)iptr,sizeof ibuf,phys,ninode);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);
	fprintf(fp," %4u,%-5u %5u %3d %5d %5d %5d %8ld",
		getemajor(ibuf.i_dev),
		geteminor(ibuf.i_dev),
		ibuf.i_number,
		ibuf.i_vnode.v_count,
		ibuf.i_nlink,
		ibuf.i_uid,
		ibuf.i_gid,
		ibuf.i_size);
	switch(ibuf.i_vnode.v_type) {
		case VDIR: ch = 'd'; break;
		case VCHR: ch = 'c'; break;
		case VBLK: ch = 'b'; break;
		case VREG: ch = 'f'; break;
		case VLNK: ch = 'l'; break;
		case VFIFO: ch = 'p'; break;
		case VXNAM:
                        switch(ibuf.i_rdev) {
                                case XNAM_SEM: ch = 's'; break;
                                case XNAM_SD: ch = 'm'; break;
                                default: ch = '-'; break;
                        };
                        break;
		default:    ch = '-'; break;
	}
	fprintf(fp," %c",ch);
	fprintf(fp,"%s%s%s%03o",
		ibuf.i_mode & ISUID ? "u" : "-",
		ibuf.i_mode & ISGID ? "g" : "-",
		ibuf.i_mode & ISVTX ? "v" : "-",
		ibuf.i_mode & 0777);

	fprintf(fp,"%s%s%s%s%s%s%s\n",
		ibuf.i_flag & ILOCKED ? " lk" : "",
		ibuf.i_flag & IUPD ? " up" : "",
		ibuf.i_flag & IACC ? " ac" : "",
		ibuf.i_flag & IWANT ? " wt" : "",
		ibuf.i_flag & ICHG ? " ch" : "",
		ibuf.i_flag & ISYN ? " sy" : "",
		ibuf.i_flag & IMOD ? " md" : "");
	if(!full)
		return;
	fprintf(fp,"\tFORW BACK AFOR ABCK\n");
	slot = ((long)ibuf.i_forw - (long)iptr) / sizeof ibuf;
	if((slot >= 0) && (slot < ninode))
		fprintf(fp,"\t%4d",slot);
	else fprintf(fp,"\t  - ");
	slot = ((long)ibuf.i_back - (long)iptr) / sizeof ibuf;
	if((slot >= 0) && (slot < ninode))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");
	slot = ((long)ibuf.av_forw - (long)iptr) / sizeof ibuf;
	if((slot >= 0) && (slot < ninode))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");
	slot = ((long)ibuf.av_back - (long)iptr) / sizeof ibuf;
	if((slot >= 0) && (slot < ninode))
		fprintf(fp," %4d\n",slot);
	else fprintf(fp,"   - \n");

	fprintf(fp,"\t  NEXTR\n");
	fprintf(fp,"\t%8x",
		ibuf.i_nextr);

	if((ibuf.i_vnode.v_type == VDIR) || (ibuf.i_vnode.v_type == VREG)
		|| (ibuf.i_vnode.v_type == VLNK)) {
		for(i = 0; i < NADDR; i++) {
			if(!(i & 3))
				fprintf(fp,"\n\t");
			fprintf(fp,"[%2d]: %-10x",i,ibuf.i_addr[i]);
		}
		fprintf(fp,"\n");
	}
	else
		fprintf(fp,"\n");

	/* print vnode info */
	fprintf(fp,"\nVNODE :\n");
	fprintf(fp, vnode_header);
	prvnode(&ibuf.i_vnode);
	fprintf(fp,"\n");
}

/* get arguments for file function */
int
getfile()
{
	int all = 0;
	int full = 0;
	int phys = 0;
	int c;
	long filep;
	char *heading = "ADDRESS  RCNT    TYPE/ADDR      OFFSET   FLAGS\n";
	long prfile();

	optind = 1;
	while((c = getopt(argcnt,args,"epfw:")) !=EOF) {
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

	readmem((long)(Inode->n_value),1,-1,(char *)&iptr,sizeof iptr,"inode table");
	if(!Ninode)
		Ninode = symsrch("ninode");
	if (Ninode)
		readmem(Ninode->n_value,1,-1,(char *)&ninode, sizeof ninode,
		    "size of S5 inode table");

	if(!full)
		fprintf(fp,"%s", heading);
	if(args[optind]) {
		all = 1;
		do {
			filep = strcon(args[optind],'h');
			if(filep == -1) 
				continue;
			else
				(void) prfile(all,full,phys,filep,heading);
			filep = -1;
		} while(args[++optind]);
	} else {
		readmem(File->n_value, 1, -1, (char *)&filep, sizeof filep,
		    "file table");
		while (filep)
			filep = prfile(all,full,phys,filep,heading);
	}
}


/* print file table */
long
prfile(all,full,phys,addr,heading)
int all,full,phys;
unsigned long addr;
char *heading;
{
	struct file fbuf;
	struct cred *credbufp;
	int fileslot;
	int ngrpbuf;
	short i;
	char fstyp[5];
	struct vnode vno;

	readbuf(addr,0,phys,-1,(char *)&fbuf,sizeof fbuf,"file table");
	if(!fbuf.f_count && !all)
		return(0);
	if(full)
		fprintf(fp,"\n%s", heading);
	fprintf(fp,"%.8x", addr);
	fprintf(fp," %3d", fbuf.f_count);


	if(fbuf.f_count && fbuf.f_vnode != 0){
		/* read in vnode */
		readmem(((long)fbuf.f_vnode),1,-1,(char *)&vno,sizeof vno,"vnode");

		if(vno.v_op == (struct vnodeops *)S5vnodeops->n_value){
			strcpy(fstyp, "S5  ");
		}
		else if(vno.v_op == (struct vnodeops *)Ufs_vnodeops->n_value){
			strcpy(fstyp, "UFS ");
		}
		else if(vno.v_op == (struct vnodeops *)Spec_vnodeops->n_value){
			strcpy(fstyp, "SPEC");
		}
		else if(vno.v_op == (struct vnodeops *)Fifo_vnodeops->n_value){
			strcpy(fstyp, "FIFO");
		}
		else if(vno.v_op == (struct vnodeops *)Duvnodeops->n_value){
			strcpy(fstyp, "DU  ");
		}
		else if(vno.v_op == (struct vnodeops *)Prvnodeops->n_value){
			strcpy(fstyp, "PROC");
		}
		else{
			strcpy(fstyp, " ?  ");
		}

	} else
		strcpy(fstyp, " ?  ");
	fprintf(fp,"    %s/%8x",fstyp,fbuf.f_vnode);
	fprintf(fp," %8x",fbuf.f_offset);
	fprintf(fp,"  %s%s%s%s%s%s%s%s\n",
		fbuf.f_flag & FREAD ? " read" : "",
		fbuf.f_flag & FWRITE ? " write" : "",  /* print the file flag */
		fbuf.f_flag & FAPPEND ? " appen" : "",
		fbuf.f_flag & FSYNC ? " sync" : "",
		fbuf.f_flag & FCREAT ? " creat" : "",
		fbuf.f_flag & FTRUNC ? " trunc" : "",
		fbuf.f_flag & FEXCL ? " excl" : "",
		fbuf.f_flag & FNDELAY ? " ndelay" : "");

	if(!full)
		return((long)fbuf.f_next);

	/* user credentials */
	if(!Ngrps)
		if(!(Ngrps = symsrch("ngroups_max")))
			error("ngroups_max not found in symbol table\n");
	readmem((long)Ngrps->n_value, 1, -1, (char *)&ngrpbuf,
		sizeof ngrpbuf, "max groups");

	credbufp=(struct cred *)malloc(sizeof(struct cred) + sizeof(uid_t) * (ngrpbuf-1));
	readmem((long)fbuf.f_cred,1,-1,(char *)credbufp,sizeof (struct cred) + sizeof(uid_t) * (ngrpbuf-1),"user cred");
	fprintf(fp,"User Credential : \n");
	fprintf(fp,"rcnt:%3d, uid:%-10d, gid:%-10d, ruid:%-10d, rgid:%-10d, ngroup:%4d",
		credbufp->cr_ref,
		credbufp->cr_uid,
		credbufp->cr_gid,
		credbufp->cr_ruid,
		credbufp->cr_rgid,
		credbufp->cr_ngroups);
	for(i=0; i < (short)credbufp->cr_ngroups; i++){
		if(!(i % 4))
			fprintf(fp, "\n");
		fprintf(fp,"group[%d]:%4d ", i, credbufp->cr_groups[i]);
	}
	fprintf(fp, "\n");

	/* Asyncio links */

	fprintf(fp, "Asyncio Links: \n");
	fprintf(fp, "Forward: %x  Backward: %x\n", fbuf.f_aiof, fbuf.f_aiob);
	return((long)fbuf.f_next);
}
