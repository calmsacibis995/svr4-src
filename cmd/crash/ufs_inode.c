/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:ufs_inode.c	1.2.9.1"

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/mntent.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
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
#include <sys/fs/ufs_inode.h>
#include <sys/cred.h>
#include <sys/stream.h>
#include "crash.h"

extern char *vnode_header;

extern struct syment	*Vfs, *File;			/* namelist symbol pointers */
struct syment		*UFSInode = NULL; 
struct syment		*UFSNinode = NULL;
struct syment		*UFSIfreelist = NULL;
struct syment		*UFSinodeNINODE = NULL;
long			iptr;
struct inode		ufs_ibuf;			/* buffer for UFS inode */
long			ufs_ninode;			/* size of UFS inode table */
/*
 * Get arguments for UFS inode.
 */
int
get_ufs_inode ()
{
	int	slot = -1;
	int	full = 0;
	int	list = 0;
	int	all = 0;
	int	phys = 0;
	long	addr = -1;
	long	arg1 = -1;
	long	arg2 = -1;
	int	free = 0;
	long	next;
	int	c;
	struct inode	*firstfree;
	char	*heading =
		"SLOT  MAJ/MIN   INUMB RCNT LINK   UID   GID     SIZE    MODE FLAGS\n";

	if(!Vfs)
		if(!(Vfs = symsrch("rootvfs")))
			error("vfs list not found in symbol table\n");

	if(!File)
		if(!(File = symsrch("file")))
			error("file table not found in symbol table\n");
/*	fprintf (stderr, "File: 0x%x 0x%x\n", File, File->n_value);*/

	if(!UFSInode)
		if(!(UFSInode = symsrch("ufs_inode")))
			error("UFS inode table not found in symbol table\n");
/*	fprintf (stderr, "UFSInode: 0x%x 0x%x\n", UFSInode, UFSInode->n_value);*/

	if(!UFSNinode)
		if(!(UFSNinode = symsrch("ufs_ninode")))
			error("cannot determine UFS inode table size\n");
/*	fprintf (stderr, "UFSNinode: 0x%x 0x%x\n", UFSNinode, UFSNinode->n_value);*/

	if(!UFSinodeNINODE)
		if(!(UFSinodeNINODE = symsrch("inodeNINODE")))
			error("UFS inode table end not found in symbol table\n");
/*	fprintf (stderr, "UFSinodeNINODE:  0x%x 0x%x\n",
		UFSinodeNINODE, UFSinodeNINODE->n_value); */

	optind = 1;
/*	fprintf (stderr, "optind 0x%x argcnt 0x%x args 0x%x *args 0x%x\n",
		optind, argcnt, args, *args); */
	while((c = getopt(argcnt, args, "efprlw:")) !=EOF) {
/*		fprintf (stderr, "c: 0x%x\n", c);*/
		switch(c) {

		case 'e':	all = 1;
				break;

		case 'f':	full =1;
				break;

		case 'l':	list = 1;
				break;

		case 'p':	phys = 1;
				break;

		case 'r':	free = 1;
				break;

		case 'w':	redirect();
				break;

		default:	longjmp(syn, 0);
		}
	}
	readmem(UFSNinode->n_value, 1, -1, (char *)&ufs_ninode, sizeof ufs_ninode,
		"size of UFS inode table");
/*	fprintf (stderr, "UFSNinode->n_value: 0x%x 0x%x\n", UFSNinode->n_value, ufs_ninode);*/
	readmem((long)(UFSInode->n_value), 1, -1, (char *)&iptr, sizeof iptr, "UFS inode");

/*	fprintf (stderr, "all 0x%x full list 0x%x phys 0x%x 0x%x free 0x%x\n",
		all, full, list, phys, free);*/
	if(list)
		list_ufs_inode ();
	else {
		fprintf(fp, "UFS INODE TABLE SIZE = %d\n", ufs_ninode);
		if(!full)
			(void) fprintf(fp, "%s", heading);
		if(free) {
			if(!(UFSIfreelist = symsrch("ifreeh")))
				error("ifreeh not found in symbol table\n");
			readmem((long)UFSIfreelist->n_value, 1, -1,
				(char *)&firstfree,
				sizeof(int), "ifreeh buffer");
			next = (long)firstfree;
			while(next) {
/*				fprintf (stderr, "next 0x%x\n", next);*/
				print_ufs_inode(1, full, slot, phys, next, heading);
				next = (long)ufs_ibuf.i_freef;
				if(next == (long)firstfree)
					next = 0;
			}
		} else if(args[optind]) {
			all = 1;
			do {
				getargs(ufs_ninode, &arg1, &arg2);
/*				fprintf (stderr, "arg1 0x%x arg2 0x%x\n",
					arg1, arg2);*/
				if(arg1 == -1)
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						print_ufs_inode (all, full, slot,
							phys, addr, heading);
				else {
					if(arg1 < ufs_ninode)
						slot = arg1;
					else addr = arg1;
						print_ufs_inode (all, full, slot,
							phys, addr, heading);
				}
				slot = addr = arg1 = arg2 = -1;
			} while(args[++optind]);
		} else for(slot = 0; slot < ufs_ninode; slot++) {
			print_ufs_inode (all, full, slot, phys, addr, heading);
		}
			

	}
}

list_ufs_inode ()
{
	char		inodebuf[500];
	int		i, j;
	long		next;
	struct inode	*firstfree;

	if(!UFSIfreelist)
		if(!(UFSIfreelist = symsrch("ifreeh")))
			error("ifreeh not found in symbol table\n");
/*	fprintf (stderr, "UFSIfreelist: 0x%x 0x%x\n",
		UFSIfreelist, UFSIfreelist->n_value);*/
	for(i = 0; i < ufs_ninode; i++)
		inodebuf[i] = 'n';

	for(i = 0; i < ufs_ninode; i++) {
		readmem((long)(iptr+i*sizeof ufs_ibuf), 1, -1,
			(char *)&ufs_ibuf, sizeof ufs_ibuf, "inode table");
		if(ufs_ibuf.i_vnode.v_count != 0)
			inodebuf[i] = 'u';
	}
	readmem((long)UFSIfreelist->n_value, 1, -1, (char *)&firstfree,
		sizeof(long), "ifreeh buffer");
	next = (long)firstfree;
	while(next) {
		i = getslot((long)next, (long)iptr, sizeof ufs_ibuf, 0, ufs_ninode);
		readmem((long)next, 1, -1, (char *)&ufs_ibuf, sizeof ufs_ibuf, "UFS inode");
		inodebuf[i] = 'f';
		if(ufs_ibuf.i_vnode.v_count != 0)
			inodebuf[i] = 'b';
		next = (long)ufs_ibuf.i_freef;
		if(next == (long)firstfree)
			next = 0;
	}
	(void) fprintf(fp, "The following UFS inodes are in use:\n");
	for(i = 0, j = 0; i < ufs_ninode; i++) {
		if(inodebuf[i] == 'u') {
			if(j && (j % 10) == 0)
				fprintf(fp, "\n");
			fprintf(fp, "%3d    ", i);
			j++;
		}
	}
	fprintf(fp, "\n\nThe following UFS inodes are on the freelist:\n");
	for(i = 0, j=0; i < ufs_ninode; i++) {
		if(inodebuf[i] == 'f') {
			if(j && (j % 10) == 0)
				fprintf(fp, "\n");
			fprintf(fp, "%3d    ", i);
			j++;
		}
	}
	fprintf(fp, "\n\nThe following UFS inodes are on the freelist but have non-zero reference counts:\n");
	for(i = 0, j=0; i < ufs_ninode; i++) {
		if(inodebuf[i] == 'b') {
			if(j && (j % 10) == 0)
				fprintf(fp, "\n");
			fprintf(fp, "%3d    ", i);
			j++;
		}
	}
	fprintf(fp, "\n\nThe following UFS inodes are in unknown states:\n");
	for(i = 0, j = 0; i < ufs_ninode; i++) {
		if(inodebuf[i] == 'n') {
			if(j && (j % 10) == 0)
				fprintf(fp, "\n");
			fprintf(fp, "%3d    ", i);
			j++;
		}
	}
	fprintf(fp, "\n");
}

/*
 * Print UFS inode table.
 */

int
print_ufs_inode (all, full, slot, phys, addr, heading)
	int	all, full, slot, phys;
	long	addr;
	char	*heading;
{
	char		ch;
	int		i;
	extern long	lseek();

	readbuf(addr,(iptr+slot*sizeof ufs_ibuf),phys,-1,
		(char *)&ufs_ibuf,sizeof ufs_ibuf,"inode table");
	if(!ufs_ibuf.i_vnode.v_count && !all)
			return;
	if(full)
		fprintf(fp, "%s", heading);
	if(addr > -1) {
/*		fprintf (stderr,
		    "getslot(addr 0x%x iptr 0x%x size 0x%x phys 0x%x ufs_ninode 0x%x)\n",
			addr, (long)iptr, sizeof ufs_ibuf, phys, ufs_ninode);
*/
		slot = getslot(addr, (long)iptr, sizeof ufs_ibuf, phys, ufs_ninode);
	}
	if(slot == -1)
		fprintf(fp, "  - ");
	else
		fprintf(fp, "%4d", slot);
	fprintf(fp, " %4u,%-5u %5u %3d %5u %5d %5d %8ld",
		getemajor(ufs_ibuf.i_dev),
		geteminor(ufs_ibuf.i_dev),
		ufs_ibuf.i_number,
		ufs_ibuf.i_vnode.v_count,
		ufs_ibuf.i_nlink,
		ufs_ibuf.i_uid,
		ufs_ibuf.i_gid,
		ufs_ibuf.i_size);
	switch(ufs_ibuf.i_vnode.v_type) {
		case VDIR: ch = 'd'; break;
		case VCHR: ch = 'c'; break;
		case VBLK: ch = 'b'; break;
		case VREG: ch = 'f'; break;
		case VLNK: ch = 'l'; break;
		case VFIFO: ch = 'p'; break;
		default:    ch = '-'; break;
	}
	fprintf(fp, " %c", ch);
	fprintf(fp, "%s%s%s%03o",
		ufs_ibuf.i_mode & ISUID ? "u" : "-",
		ufs_ibuf.i_mode & ISGID ? "g" : "-",
		ufs_ibuf.i_mode & ISVTX ? "v" : "-",
		ufs_ibuf.i_mode & 0777);

	fprintf(fp, "%s%s%s%s%s%s%s%s%s%s\n",
		ufs_ibuf.i_flag & ILOCKED ? " lk" : "",
		ufs_ibuf.i_flag & IUPD ? " up" : "",
		ufs_ibuf.i_flag & IACC ? " ac" : "",
		ufs_ibuf.i_flag & IWANT ? " wt" : "",
		ufs_ibuf.i_flag & ICHG ? " ch" : "",
		ufs_ibuf.i_flag & ISYNC ? " sy" : "",
		ufs_ibuf.i_flag & ILWAIT ? " wt" : "",
		ufs_ibuf.i_flag & IREF ? " rf" : "",
		ufs_ibuf.i_flag & INOACC ? " na" : "",
		ufs_ibuf.i_flag & IMODTIME ? "md" : "",
		ufs_ibuf.i_flag & IMOD ? " md" : "");
	if(!full)
		return;
	fprintf(fp, "\tFORW BACK AFOR ABCK\n");
	slot = ((long)ufs_ibuf.i_forw - (long)iptr) / sizeof ufs_ibuf;
	if((slot >= 0) && (slot < ufs_ninode))
		fprintf(fp, "\t%4d", slot);
	else
		fprintf(fp, "\t  - ");
	slot = ((long)ufs_ibuf.i_back - (long)iptr) / sizeof ufs_ibuf;
	if((slot >= 0) && (slot < ufs_ninode))
		fprintf(fp, " %4d", slot);
	else
		fprintf(fp, "   - ");
	slot = ((long)ufs_ibuf.i_freef - (long)iptr) / sizeof ufs_ibuf;
	if((slot >= 0) && (slot < ufs_ninode))
		fprintf(fp, " %4d", slot);
	else
		fprintf(fp, "   - ");
	slot = ((long)ufs_ibuf.i_freeb - (long)iptr) / sizeof ufs_ibuf;
	if((slot >= 0) && (slot < ufs_ninode))
		fprintf(fp, " %4d\n", slot);
	else
		fprintf(fp, "   - \n");

	fprintf(fp, "\t  OWNER COUNT NEXTR \n");
	fprintf(fp, "\t%4d", ufs_ibuf.i_owner);
	fprintf(fp, " %4d", ufs_ibuf.i_count);
	fprintf(fp, " %8x\n", ufs_ibuf.i_nextr);

	if((ufs_ibuf.i_vnode.v_type == VDIR) || (ufs_ibuf.i_vnode.v_type == VREG)
		|| (ufs_ibuf.i_vnode.v_type == VLNK)) {
		for(i = 0; i < NADDR; i++) {
			if(!(i & 3))
				fprintf(fp, "\n\t");
			fprintf(fp, "[%2d]: %-10x", i, ufs_ibuf.i_db[i]);
		}
		fprintf(fp, "\n");
	} else
		fprintf(fp, "\n");

	/* print vnode info */
	fprintf(fp, "\nVNODE :\n");
	fprintf(fp, vnode_header);
	prvnode(&ufs_ibuf.i_vnode);
	fprintf(fp, "\n");
}
