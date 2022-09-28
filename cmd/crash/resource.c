/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:resource.c	1.3.9.1"

/*
 * This file contains code for the crash functions:  resource, srmount.
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
#include <sys/rf_adv.h>
#include "crash.h"

extern struct syment *Vnode, *Vfs, *Rcvd, *Nrcvd;	/* namelist symbol */
static struct syment *Resources;			/* namelist symbol */



/* get arguments for resource function */
int
getresrc()
{
	int slot = -1;
	int phys = 0;
	int c;


	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();

	prresrc();

}
int
prresrc()
{
	int slot = -1;
	int i;
	struct rf_resource_head rh;
	struct rf_resource resrc, *next;

	if(!Resources)
		if(!(Resources = symsrch("rf_resource_head")))
			error("rf_resource_head not found\n");

	readmem((long)Resources->n_value,1,-1,(char *)&rh,sizeof rh,
		"rf_resource_head");
	next = rh.rh_nextp;

	fprintf(fp, "          NAME   RMOUNTP   ROOTVP   RCVDP   FLAGS \n");

	while(next != (rf_resource_t *)Resources->n_value){
		readmem((long)next,1,-1,(char *)&resrc,sizeof resrc,"resource");

		fprintf(fp, "%14s  %8x %8x %8x", resrc.r_name, resrc.r_mountp,
			resrc.r_rootvp, resrc.r_queuep);

		fprintf(fp,"  %s%s%s\n",
			(resrc.r_flags & R_RDONLY) ? "r" : "-",
			(resrc.r_flags & R_CACHEOK) ? "c" : "-",
			(resrc.r_flags & R_UNADV) ? "u" : "-");

		next = resrc.r_nextp;
	}
}

/* get arguments for srmount function */
int
getsrmount()
{
	int slot = -1;
	int phys = 0;
	long addr = -1;
	int c;


	optind = 1;
	while((c = getopt(argcnt,args,"pw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();

	if(!args[optind])
		longjmp(syn,0);
	fprintf(fp,"SYSID   MNDX RCNT  BCOUNT  SLPCNT FLAGS \n");
	do{
		if((addr = strcon(args[optind++], 'h')) == -1)
			error("\n");
		prsrmount(phys,addr);
	}while(args[optind]);

	fprintf(fp, "\n");
}

/* print server mount list */
int
prsrmount(phys,addr)
int phys;
long addr;
{
	struct sr_mount srmntbuf, *sptr;

	sptr = (struct sr_mount *)addr;
	while(1){
		readbuf((long)sptr,0,phys,-1,(char *)&srmntbuf,sizeof srmntbuf,
			"server mount list");
		fprintf(fp," %4x %4u %4u  %6u",
			srmntbuf.srm_sysid,
			srmntbuf.srm_mntid,
			srmntbuf.srm_refcnt,
			srmntbuf.srm_slpcnt);
		fprintf(fp,"  %s",(srmntbuf.srm_flags & SRM_RDONLY) ? "ro" : "rw");
		fprintf(fp,"%s%s\n",
			(srmntbuf.srm_flags & SRM_LINKDOWN) ? " ldown" : "",
			(srmntbuf.srm_flags & SRM_FUMOUNT) ? " fumnt" : "");
		sptr = srmntbuf.srm_nextp;
		if(sptr == NULL)
			break;
	}

}
