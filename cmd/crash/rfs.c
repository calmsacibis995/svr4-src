/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:rfs.c	1.16.17.1"

/*
 * This file contains code for the crash functions:  gdp, rcvd, sndd and rduser.
 */

#include <stdio.h>
#include <sys/param.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#define _KERNEL
#include <sys/stream.h>
#undef _KERNEL
#include <sys/list.h>
#include <sys/rf_messg.h>
#include <sys/rf_comm.h>
#include <sys/nserve.h>
#include <sys/rf_cirmgr.h>
#include <sys/vfs.h>
#include <sys/rf_adv.h>
#include <sys/var.h>
#include "crash.h"

extern struct syment *File,*Vnode;	/* namelist symbol */

/* Symbol entries of number of various dynamically allocated RFS structures */
static struct syment *Nrcvd,*Nsndd,*Nrduser,*Maxgdp;

/* Start Address of the allocated RFS tables */
static unsigned long	Gdp_addr, Rcvd_addr, Sndd_addr, Rduser_addr;	



/* check for rfs activity */
int
checkboot()
{
	int rf_state;
	struct syment *rf_boot;

	if(!(rf_boot = symsrch("rf_state"))) 
		error("rf_state not found in symbol table\n");

	readmem((long)rf_boot->n_value,1,-1,(char *)&rf_state,
		sizeof rf_state,"rf_state");

	if(rf_state != RF_UP) {
		if(rf_state == RF_DOWN)
			prerrmes("rfs not started\n\n");
		else prerrmes("rfs in process of starting\n\n");
	}
}


/* get arguments for gdp function */
int
getgdp()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	char *heading = "SLOT MNT SYSID ISTATE 1SHOT  HDRADDR IDATADDR HL DLEN TIDU CONSTATE\n";
	struct syment *gdpsym;
	int maxgdp;

	if(!Gdp_addr) 
		if (gdpsym = symsrch("gdp"))  {
			readmem(gdpsym->n_value, 1, -1, (char *)&Gdp_addr, 
			  sizeof Gdp_addr, "Address of gift descriptor table");
		} else {
			error("gdp table not found\n");
		}

	if(!Maxgdp)
		if(!(Maxgdp = symsrch("maxgdp"))) 
			error("cannot determine size of gift descriptor table\n");
	readmem((long)Maxgdp->n_value,1,-1,(char *)&maxgdp,
		sizeof maxgdp,"size of gift descriptor table");
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
	checkboot();
	fprintf(fp,"GDP MAX SIZE = %d\n",maxgdp);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(maxgdp,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prgdp(full,all,slot,phys,addr,heading,
						maxgdp);
			else {
				if(arg1 < maxgdp)
					slot = arg1;
				else addr = arg1;
				prgdp(full,all,slot,phys,addr,heading,maxgdp);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < maxgdp; slot++)
		prgdp(full,all,slot,phys,addr,heading,maxgdp);
}

/* print gdp table */
int
prgdp(full,all,slot,phys,addr,heading,max)
int full,all,slot,phys,max;
unsigned long addr;
char *heading;
{
	struct gdp gdpbuf;
	char temp[MAXDNAME+1];

	readbuf(addr,(long)(Gdp_addr + slot*sizeof gdpbuf),phys,-1,
		(char *)&gdpbuf,sizeof gdpbuf,"gdp structures");
	if (!gdpbuf.queue && !all)
		return;
	if(full)
		fprintf(fp,"%s",heading);
	if(addr > -1) 
		slot = getslot(addr, Gdp_addr,sizeof gdpbuf,phys,max);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);
	fprintf(fp,"  %2d  %4x      %1d     ",
		gdpbuf.mntcnt,
		gdpbuf.sysid,
		gdpbuf.input.istate);
	if(gdpbuf.input.oneshot)
		fprintf(fp,"1");
	else	fprintf(fp,"0");
	if (gdpbuf.hdr)
		fprintf(fp," %8x",gdpbuf.hdr);
	else fprintf(fp,"   -     ");	
	if (gdpbuf.idata)
		fprintf(fp," %8x",gdpbuf.idata);
	else fprintf(fp,"   -     ");	
	fprintf(fp," %2d %4d %4d %s%s%s\n",
		gdpbuf.hlen,
		gdpbuf.dlen,
		gdpbuf.maxpsz,
		(gdpbuf.constate & GDPDISCONN) ? " dis" : "",
		(gdpbuf.constate & GDPRECOVER) ? " rec" : "",
		(gdpbuf.constate & GDPCONNECT) ? " con" : "");
	if(full) {
		fprintf(fp,"\tQUEUEADDR  FILEADDR\n");
		fprintf(fp,"\t%#.8x %#.8x\n",gdpbuf.queue,gdpbuf.file);
		fprintf(fp,"\tHET VER   UID   GID       TIME\n");
		fprintf(fp,"\t%3x  %2d %5x %5x %10d\n",
			gdpbuf.hetero,
			gdpbuf.version,
			gdpbuf.idmap[0],
			gdpbuf.idmap[1],
			gdpbuf.timeskew_sec);
		strncpy(temp,gdpbuf.token.t_uname,MAXDNAME);
		fprintf(fp,"\tTOKEN ID:  %4x    TOKEN NAME:  %s\n",
			gdpbuf.token.t_id,
			temp);
		fprintf(fp,"\n");
	}
}


/* get arguments for rcvd function */
int
getrcvd()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	struct syment *rcvdsym, *snddsym;
	int nrcvd;
	char *heading = "SLOT  QTYPE   VN/SD   QCNT  RCNT STATE\n";

	if(!Rcvd_addr)
		if (rcvdsym  = symsrch("rcvd")) {
			readmem(rcvdsym->n_value, 1, -1, (char *)&Rcvd_addr, 
			  sizeof Rcvd_addr, "Address of receive descriptor table");
		} else {
			error("receive descriptor table not found\n");
		}

	if(!Sndd_addr)
		if (snddsym  = symsrch("sndd")) {
			readmem(snddsym->n_value, 1, -1, (char *)&Sndd_addr, 
			  sizeof Sndd_addr, "Address of send descriptor table");
		} else {
			error("send descriptor table not found\n");
		}

	if(!Nrcvd)
		if(!(Nrcvd = symsrch("nrcvd"))) 
			error("cannot determine size of receive descriptor table\n");

	if(!Nsndd)
		if(!(Nsndd = symsrch("nsndd"))) 
			error("cannot determine size of send descriptor table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();
	readmem((long)Nrcvd->n_value,1,-1,(char *)&nrcvd,
		sizeof nrcvd,"size of receive descriptor table");
	fprintf(fp,"RECEIVE DESCRIPTOR TABLE SIZE = %d\n",nrcvd);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(nrcvd,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prrcvd(all,full,slot,phys,addr,
						heading,nrcvd);
			else {
				if(arg1 < nrcvd)
					slot = arg1;
				else addr = arg1;
				prrcvd(all,full,slot,phys,addr,heading,nrcvd);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nrcvd; slot++)
		prrcvd(all,full,slot,phys,addr,heading,nrcvd);
}

/* print rcvd table */
int
prrcvd(all,full,slot,phys,addr,heading,size)
int all,full,slot,phys,size;
unsigned long addr;
char *heading;
{
	struct rcvd rcvdbuf;
	struct rd_user userbuf;
	struct rd_user *next;
	int nsndd;

	readbuf(addr,(long)(Rcvd_addr + slot*sizeof rcvdbuf),phys,-1,
		(char *)&rcvdbuf,sizeof rcvdbuf,"received descriptor table");
	readmem((long)Nsndd->n_value,1,-1,(char *)&nsndd,
		sizeof nsndd,"size of send descriptor table");
	if(((rcvdbuf.rd_stat == RDUNUSED)) && !all)
		return;
	if(full) {
		streaminit();
        	fprintf(fp,"%s",heading);
	}
	if(addr > -1) 
		slot = getslot(addr, Rcvd_addr,sizeof rcvdbuf,phys,size);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);
	if(rcvdbuf.rd_qtype & RDGENERAL) {
		if(rcvdbuf.rd_vp)
			fprintf(fp,"   G   %8x ",rcvdbuf.rd_vp);
		else
			fprintf(fp,"   G       -    ");
	}
	else if(rcvdbuf.rd_qtype & RDSPECIFIC) {
		slot = ((long)rcvdbuf.rd_sdp - Sndd_addr)/
			sizeof (struct sndd);
		if((slot >= 0) && (slot < nsndd))
			fprintf(fp,"   S   %4d     ",slot);
		else fprintf(fp,"   S       -     ");
	}
	else
		fprintf(fp,"   ?      -     ");

	fprintf(fp," %5u %4u",
		rcvdbuf.rd_qcnt,
		rcvdbuf.rd_refcnt);
	fprintf(fp,"%s%s",
		(rcvdbuf.rd_stat & RDUSED) ? " used" : "",
		(rcvdbuf.rd_stat & RDUNUSED) ? " unused" : "",
		(rcvdbuf.rd_stat & RDLINKDOWN) ? " lnkdown" : "",
		(rcvdbuf.rd_stat & RDWANT) ? " rdwant" : "");
	fprintf(fp,"\n");
	if(full) {
		fprintf(fp,"\tGEN   NEXT     QSLP   RHEAD RTAIL\n");
		fprintf(fp,"\t%5d", rcvdbuf.rd_gen);
		slot = ((unsigned long)rcvdbuf.rd_next - Rcvd_addr)/
			sizeof (struct rcvd);
		if((slot >= 0) && (slot < size))
			fprintf(fp," %4d",slot);
		else fprintf(fp,"  -  ");
		fprintf(fp," %8x", rcvdbuf.rd_qslp);
		/*
		if (rcvdbuf.rd_rcvdq.qc_head)
			fprintf(fp,"   %8x",rcvdbuf.rd_rcvdq.qc_head);
		else fprintf(fp,"   -  ");
		if (rcvdbuf.rd_rcvdq.qc_tail)
			fprintf(fp,"   %8x",rcvdbuf.rd_rcvdq.qc_tail);
		else fprintf(fp,"   -  ");
		*/
		fprintf(fp, "        %8x", rcvdbuf.rd_rcvdq);
		if(rcvdbuf.rd_user_list) {
			fprintf(fp,"   USER_LIST: %8x",rcvdbuf.rd_user_list);
			if(rcvdbuf.rd_qtype & RDGENERAL) {
				next = rcvdbuf.rd_user_list;
				if(next)
					fprintf(fp,"\n\tQUEUEADDR SRMNT  VCNT  RWCNT  RCNT  WCNT   NEXT   CWCNT FLAG \n");
				while(next) {
					readmem((long)next,1,-1,(char *)&userbuf,
						sizeof userbuf,"user list");
					if (userbuf.ru_queue)
						fprintf(fp,"\t%9x",userbuf.ru_queue);
					else fprintf(fp,"\t   - ");
					fprintf(fp," %5d", userbuf.ru_srmntid);
					fprintf(fp," %5d %5d %5d %5d",
						userbuf.ru_vcount,
						userbuf.ru_frwcnt,
						userbuf.ru_frcnt,
						userbuf.ru_fwcnt);
					next = userbuf.ru_next;
					if(next)
						fprintf(fp," %8x",next);
					else fprintf(fp,"    -    ");
					fprintf(fp," %s%s%s\n",
					  (userbuf.ru_flag) ? "" : " off",
					  (userbuf.ru_flag & RU_CACHE_ON) ?
					   " ena" : "",
					  (userbuf.ru_flag & RU_CACHE_DISABLE)
					   ? " dis" : "");
					if(userbuf.ru_vcount == 0)
						break;
				}
			}
			else fprintf(fp,"\n");
		}
		else fprintf(fp,"\n");
		fprintf(fp,"\n");
	}
}

/* get arguments for sndd function */
int
getsndd()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	struct syment *snddsym;
	int nsndd;
        char *heading =
	  "SLOT SIZE   MNDX CONID/GEN PROC SQUEADDR NEXT  STATE\n";

	if(!Sndd_addr)
		if (snddsym  = symsrch("sndd")) {
			readmem(snddsym->n_value, 1, -1, (char *)&Sndd_addr, 
			  sizeof Sndd_addr, "Address of send descriptor table");
		} else {
			error("send descriptor table not found\n");
		}
	if(!Nsndd)
		if(!(Nsndd = symsrch("nsndd"))) 
			error("cannot determine size of send descriptor table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();
	readmem((long)Nsndd->n_value,1,-1,(char *)&nsndd,
		sizeof nsndd,"size of send descriptor table");
	fprintf(fp,"SEND DESCRIPTOR TABLE SIZE = %d\n",nsndd);
        if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(nsndd,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prsndd(all,full,slot,phys,addr,heading,nsndd);
			else {
				if(arg1 < nsndd)
					slot = arg1;
				else addr = arg1;
				prsndd(all,full,slot,phys,addr,heading,nsndd);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nsndd; slot++)
		prsndd(all,full,slot,phys,addr,heading,nsndd);
}

/* print sndd function */
int
prsndd(all,full,slot,phys,addr,heading,size)
int all,full,slot,phys,size;
unsigned long addr;
char *heading;
{
	struct sndd snddbuf;

	readbuf(addr,(long)(Sndd_addr + slot*sizeof snddbuf),phys,-1,
		(char *)&snddbuf,sizeof snddbuf,"send descriptor table");
	if((snddbuf.sd_stat == SDUNUSED) && !all)
		return;
        if(full)
		fprintf(fp,"%s",heading);
	if(addr > -1) 
		slot = getslot(addr, Sndd_addr,sizeof snddbuf,phys,size);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);
	fprintf(fp," %6d %4d %4d %4d",
		snddbuf.sd_size,
		snddbuf.sd_mntid,
		snddbuf.sd_gift.gift_id,
		snddbuf.sd_gift.gift_gen);
	slot = proc_to_slot((long)snddbuf.sd_srvproc);
	if (slot == -1)
		fprintf(fp,"  -  ");
	else
		fprintf(fp," %4d",slot);
	if (snddbuf.sd_queue)
		fprintf(fp," %8x",snddbuf.sd_queue);
	else fprintf(fp,"  -      ");
	slot = ((unsigned long)snddbuf.sd_free.ls_next - Sndd_addr)/
		sizeof (struct sndd);
	if((slot >= 0) && (slot < size))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"  -  ");
	fprintf(fp,"%s%s%s%s%s%s%s",
		(snddbuf.sd_stat & SDUSED) ? " used" : "",
		(snddbuf.sd_stat & SDLOCKED) ? " lck" : "",
		(snddbuf.sd_stat & SDSERVE) ? " serve" : "",
		(snddbuf.sd_stat & SDLINKDOWN) ? " ldown" : "",
		(snddbuf.sd_stat & SDWANT) ? " want" : "",
		(snddbuf.sd_stat & SDCACHE) ? " cache" : "",
		(snddbuf.sd_stat & SDMNDLCK) ? " mndlck" : "");
	fprintf(fp,"\n");
	if(full) {
		fprintf(fp,"\t FHANDLE   VCODE   STASHP \n");
		fprintf(fp,"\t%8x  %6d %8x \n",
			snddbuf.sd_fhandle,
			snddbuf.sd_vcode,
			snddbuf.sd_stashp);

		fprintf(fp,"\tVCNT RMAJ/MIN  VFSPTR VFILOCKS    \n");
		fprintf(fp,"\t %3d %3u,%-3u",
			snddbuf.sd_vn.v_count,
			getemajor(snddbuf.sd_vn.v_rdev),
			geteminor(snddbuf.sd_vn.v_rdev));
		fprintf(fp," %8x %8x",
			snddbuf.sd_vn.v_vfsp,
			snddbuf.sd_vn.v_filocks);
		fprintf(fp,"\n");

	}
}

/* get arguments for rduser function */
int
getrduser()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	struct syment *rdusersym;
	int nuser;
        char *heading = "SLOT     NEXT  QUEUE  SRMNTINDX FCNT VCNT FRCNT  FWCNT  CWCNT CFLAG \n";

	if(!Rduser_addr)
		if (rdusersym = symsrch("rd_user"))  {
			readmem(rdusersym->n_value, 1, -1, (char *)&Rduser_addr, 
			  sizeof Rduser_addr, "Address of rcvd user table");
		} else {
			error("rcvd user table not found\n");
		}
	if(!Nrduser)
		if(!(Nrduser = symsrch("nrduser"))) 
			error("cannot determine size of rduser table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();
	readmem((long)Nrduser->n_value,1,-1,(char *)&nuser,
		sizeof nuser,"size of rcvd user table");
	fprintf(fp,"RCVD USER TABLE SIZE = %d\n",nuser);
        if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(nuser,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prrduser(all,full,slot,phys,addr,
						heading,nuser);
			else {
				if(arg1 < nuser)
					slot = arg1;
				else addr = arg1;
				prrduser(all,full,slot,phys,addr,heading,nuser);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nuser; slot++)
		prrduser(all,full,slot,phys,addr,heading,nuser);
}

/* print rd_user function */
int
prrduser(all,full,slot,phys,addr,heading,size)
int all,full,slot,phys,size;
unsigned long addr;
char *heading;
{
	struct rd_user rduser;

	readbuf(addr,(long)(Rduser_addr + slot*sizeof rduser),phys,-1,
		(char *)&rduser,sizeof rduser,"rcvd user table");
	if((rduser.ru_vcount == 0) && !all)
		return;
        if(full)
		fprintf(fp,"%s",heading);
	if(addr > -1) 
		slot = getslot(addr, Rduser_addr,sizeof rduser,phys,size);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d",slot);
	fprintf(fp," %8x %8x   %4u   %3u %3u %6u %6u",
		rduser.ru_next,
		rduser.ru_queue,
		rduser.ru_srmntid,
		rduser.ru_frwcnt,
		rduser.ru_vcount,
		rduser.ru_frcnt,
		rduser.ru_fwcnt);
	fprintf(fp, "   %c%c", rduser.ru_vcount ? '-' : 'f',
	  rduser.ru_vcount ? 'u' : '-');
	fprintf(fp, "      %c%c%c", rduser.ru_flag ? '-' : 'o',
	  rduser.ru_flag & RU_CACHE_ON ? 'e' : '-',
	  rduser.ru_flag & RU_CACHE_DISABLE ? 'd' : '-');
	fprintf(fp, "\n");
}
