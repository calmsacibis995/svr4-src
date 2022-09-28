/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:var.c	1.5.9.1"

/*
 * This file contains code for the crash function var.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#ifdef i386
#else
#include <sys/sbd.h>
#include <sys/psw.h>
#include <sys/pcb.h>
#endif
#include <sys/user.h>
#include <sys/var.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/vfs.h>
#include <sys/utsname.h>
#include "crash.h"

extern struct syment *Inode,*File,*Mount;   /* namelist symbol pointers */

/* get arguments for var function */
int
getvar()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		longjmp(syn,0);
	else prvar();
}

/* print var structure */
int
prvar()
{
	extern struct syment *V;

	readmem((long)V->n_value,1,-1,(char *)&vbuf,
		sizeof vbuf,"var structure");

	fprintf(fp,"v_buf: %3d\nv_call: %3d\nv_maxsyspri: %3d\nv_clist: %3d\nv_maxup: %3d\n",
		vbuf.v_buf,
		vbuf.v_call,
		vbuf.v_maxsyspri,
		vbuf.v_clist,
		vbuf.v_maxup);
	fprintf(fp,"v_hbuf: %3d\nv_hmask: %3d\nv_pbuf: %3d\n",
		vbuf.v_hbuf,
		vbuf.v_hmask,
		vbuf.v_pbuf);
	fprintf(fp,"v_sptmap: %3d\n",
		vbuf.v_sptmap);
	fprintf(fp,"v_maxpmem: %d\nv_autoup: %d\n",
		vbuf.v_maxpmem,
		vbuf.v_autoup);
}
