/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash:async.c	1.2.10.1"
/*
*This file contains code for the crash function: async
*/

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/signal.h>
#include <sys/var.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/cred.h>
#include <sys/immu.h>
#include <sys/proc.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>
#include <sys/hrtsys.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>
#include <sys/procset.h>
#include <sys/events.h>
#include <sys/evsys.h>
#include <sys/asyncio.h>
#define _KERNEL 1
#include <sys/asyncsys.h>
#undef _KERNEL
#include "crash.h"

struct syment *Aioqueue, *Aioactive, *Free;
struct aioreq aiobuf, aioactive, freebuf;
struct syment *S5vnodeops, *Ufs_vnodeops, *Duvnodeops, *Spec_vnodeops, *Fifo_vnodeops, *Prvnodeops;
struct syment *Min, *Max, *Timeout, *Size, *Naio, *Curaio, *Sysent, *Syssize, *Ngrps;

int
getasync()
{

	int c;
	int full=0;

	if(!Aioqueue)
		if(!(Aioqueue = symsrch("aio_list")))
			error("aio_list not found in symbol table\n");
	if(!Aioactive)
		if(!(Aioactive = symsrch("aio_active_list")))
			error("aio_active_list not found in symbol table\n");
	if(!S5vnodeops)
		if(!(S5vnodeops = symsrch("s5vnodeops")))
			error("s5vnodeops not found\n");
	if(!Ufs_vnodeops)
		if(!(Ufs_vnodeops = symsrch("ufs_vnodeops")))
			error("ufs_vnodeops not found\n");
	if(!Duvnodeops)
		if(!(Duvnodeops = symsrch("duvnodeops")))
			error("duvnodeops not found\n");
	if(!Spec_vnodeops)
		if(!(Spec_vnodeops = symsrch("spec_vnodeops")))
			error("spec_vnodeops not found\n");
	if(!Fifo_vnodeops)
		if(!(Fifo_vnodeops = symsrch("fifo_vnodeops")))
			error("fifo_vnodeops not found\n");
	if(!Prvnodeops)
		if(!(Prvnodeops = symsrch("prvnodeops")))
			error("prvnodeops not found\n");
	optind=1;
	while((c=getopt(argcnt, args, "fw:")) != EOF) {
		switch(c) {
			case 'f' :	full=1;
			         	 break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}

	readmem((long)Aioqueue->n_value, 1, -1, (char *)&aiobuf,
		sizeof aiobuf, "aioserver request queue");
	readmem((long)Aioactive->n_value, 1, -1, (char *)&aioactive,
		sizeof aioactive, "active async queue");

	prasync(full);
}

/* print async information */
int
prasync(full)
int full;
{


	struct aioreq *saiop, *faiop;
	int minbuf, maxbuf, timebuf, sizebuf, curaiobuf, ngrpbuf;
	short naiobuf;
	struct cred *credbufp;
	struct sysent callbuf, syscallbuf;
	int fileslot;
	int i;
	long result;
	char fstyp[5];
	struct vnode vno;
	char *syscall;
	uint sysentbuf;
	int slot;

	if(!Sysent)
		if(!(Sysent = symsrch("sysent")))
			error("sysent not found in symbol table\n");
	if(!Syssize)
		if(!(Syssize = symsrch("sysentsize")))
			error("sysentsize not found in symbol table\n");


	readmem((long)Syssize->n_value, 1, -1, (char *)&sysentbuf,
		sizeof sysentbuf, "sysent size");

	fprintf(fp, "Aioserver Request Queue: \n");
	saiop=aiobuf.ar_sflink;
	if(saiop == (aioreq_t *)Aioqueue->n_value)
		fprintf(fp,"The aioserver request queue is empty\n");
	else
	for(slot=0; saiop != (aioreq_t *)Aioqueue->n_value; saiop=aiobuf.ar_sflink, slot++){
		readmem((long)saiop, 1, -1, (char *)&aiobuf,
			sizeof aiobuf, "aioserver request queue");
		fprintf(fp, "Slot %d\n", slot);
		fprintf(fp, "fflink: %x,  fblink: %x\n", aiobuf.ar_fflink,
			aiobuf.ar_fblink);
		fprintf(fp, "sflink: %x,  sblink: %x,  procp: %x\n",
 			aiobuf.ar_sflink, aiobuf.ar_sblink, aiobuf.ar_procp);

		/* Print system call name and arguments */

		readmem((long)aiobuf.ar_syscallp, 1, -1,
			(char *)&syscallbuf, sizeof syscallbuf,
			"current sysent structure");

		result=((long)aiobuf.ar_syscallp - Sysent->n_value) / sizeof (struct sysent);
		if(result < 0 || result > sysentbuf)
			error("system call entry out of range\n");

		fprintf(fp, "syscall: %ld,  args: ", result);
		for(i=0; i < syscallbuf.sy_narg; i++) 
			fprintf(fp, "%x ", aiobuf.ar_args[i]);
		fprintf(fp, "\n");

		

		fprintf(fp, "ulimit: %d,  ofilep: %x\n", aiobuf.ar_ulimit,
			 aiobuf.ar_ofilep);


		fprintf(fp,"\nFile Table Entry : \n");
		fprintf(fp,"\tcount: %3d, ", aiobuf.ar_aiofile.f_count);
	
	
		if(aiobuf.ar_aiofile.f_count && aiobuf.ar_aiofile.f_vnode != 0){
			/* read in vnode */
			readmem(((long)aiobuf.ar_aiofile.f_vnode),1,-1,
				(char *)&vno,sizeof vno,"vnode");
	
			if(vno.v_op == (struct vnodeops *)S5vnodeops->n_value){
				strcpy(fstyp, "S5  ");
			}
			else if(vno.v_op == (struct vnodeops *)Ufs_vnodeops->n_value){
				strcpy(fstyp, "UFS  ");
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
	
			fprintf(fp,"fstype/f_vnode: %s/%8x, ",fstyp,aiobuf.ar_aiofile.f_vnode);
		}
		fprintf(fp,"offset: %8x,",aiobuf.ar_aiofile.f_offset);
		fprintf(fp,"  %s%s%s%s%s%s%s%s\n",
			aiobuf.ar_aiofile.f_flag & FREAD ? " read" : "",
			aiobuf.ar_aiofile.f_flag & FWRITE ? " write" : "",  /* print the file flag */
			aiobuf.ar_aiofile.f_flag & FAPPEND ? " appen" : "",
			aiobuf.ar_aiofile.f_flag & FSYNC ? " sync" : "",
			aiobuf.ar_aiofile.f_flag & FCREAT ? " creat" : "",
			aiobuf.ar_aiofile.f_flag & FTRUNC ? " trunc" : "",
			aiobuf.ar_aiofile.f_flag & FEXCL ? " excl" : "",
			aiobuf.ar_aiofile.f_flag & FNDELAY ? " ndelay" : "");
	
	
		/* user credentials */
		fprintf(fp, "\nUser Credentials: \n");
		if(!Ngrps)
			if(!(Ngrps = symsrch("ngroups_max")))
				error("ngroups_max not found in symbol table\n");
		readmem((long)Ngrps->n_value, 1, -1, (char *)&ngrpbuf,
			sizeof ngrpbuf, "max groups");

		credbufp=(struct cred *)malloc(sizeof(struct cred) + sizeof(uid_t) * (ngrpbuf-1));
		readmem((long)aiobuf.ar_aiofile.f_cred,1,-1,(char *)credbufp,sizeof (struct cred) + sizeof(uid_t) * (ngrpbuf-1),"user cred");
		fprintf(fp,"\trcnt:%3d, uid:%d, gid:%d, ruid:%d, rgid:%d, ngroup:%4d",
			credbufp->cr_ref,
			credbufp->cr_uid,
			credbufp->cr_gid,
			credbufp->cr_ruid,
			credbufp->cr_rgid,
			credbufp->cr_ngroups);
		for(i=0; i < (short)credbufp->cr_ngroups; i++){
			if(!(i % 4))
				fprintf(fp, "\n");
			fprintf(fp,"\tgroup[%d]:%4d ", i, credbufp->cr_groups[i]);
		}
		fprintf(fp, "\n");
	
		fprintf(fp,"Process Class Parameters: \n");

		switch(aiobuf.ar_pri.pc_cid) {
		
		case 1:
			fprintf(fp,"\tcid: %d,  upri: %d\n", aiobuf.ar_pri.pc_cid, 
				((tsparms_t *)aiobuf.ar_pri.pc_clparms)->ts_upri);
			break;
	
		case 2:
			fprintf(fp, "\tcid: %d,  pri: %d,  tqsecs: %d,  tqnsecs: %d  \n",
			aiobuf.ar_pri.pc_cid,
 			((rtparms_t *)aiobuf.ar_pri.pc_clparms)->rt_pri,
			((rtparms_t *)aiobuf.ar_pri.pc_clparms)->rt_tqsecs,
			((rtparms_t *)aiobuf.ar_pri.pc_clparms)->rt_tqnsecs);
			break;
		}

		fprintf(fp, "Ecb: \n");
		fprintf(fp, "\teqd: %d,  flags: %d,  eid: %d,  evpri: %d\n",
			aiobuf.ar_ecb.ecb_eqd, aiobuf.ar_ecb.ecb_flags,
			aiobuf.ar_ecb.ecb_eid, aiobuf.ar_ecb.ecb_evpri);
	
		fprintf(fp, "\tevkevp: %x,  serverp: %x\n",
			aiobuf.ar_evkevp, aiobuf.ar_serverp);
	}


	fprintf(fp, "\nActive Async Queue: \n");
	saiop=aioactive.ar_sflink;
	if(saiop == (aioreq_t *)Aioactive->n_value)
		fprintf(fp, "The active async queue is empty\n");
	else
	for(slot=0; saiop != (aioreq_t *)Aioactive->n_value; saiop=aioactive.ar_sflink, slot++){
		readmem((long)saiop, 1, -1, (char *)&aioactive,
			sizeof aioactive, "aioserver request queue");
		fprintf(fp,"Slot %d\n", slot);
		fprintf(fp, "fflink: %x,  fblink: %x\n", aioactive.ar_fflink,
			aioactive.ar_fblink);
		fprintf(fp, "sflink: %x,  sblink: %x,  procp: %x\n",
 			aioactive.ar_sflink, aioactive.ar_sblink, aioactive.ar_procp);

		/* Print system call name and arguments */
		readmem((long)aioactive.ar_syscallp, 1, -1,
			(char *)&syscallbuf, sizeof syscallbuf, "current sysent structure");

		result=((long)aioactive.ar_syscallp - Sysent->n_value) / (sizeof (struct sysent));
		if(result < 0 || result > sysentbuf)
			error("system call entry out of range\n");

		fprintf(fp, "syscall: %d,  args: ", result);
		for(i=0; i < syscallbuf.sy_narg; i++) 
			fprintf(fp, "%x ", aioactive.ar_args[i]);

		fprintf(fp, "ulimit: %d,  ofilep: %x\n", aioactive.ar_ulimit,
			 aioactive.ar_ofilep);


		fprintf(fp,"\nFile Table Entry : \n");
		fprintf(fp,"\tcount: %3d, ", aioactive.ar_aiofile.f_count);
	
	
		if(aioactive.ar_aiofile.f_count && aioactive.ar_aiofile.f_vnode != 0){
			/* read in vnode */
			readmem(((long)aioactive.ar_aiofile.f_vnode),1,-1,
				(char *)&vno,sizeof vno,"vnode");
	
			if(vno.v_op == (struct vnodeops *)S5vnodeops->n_value){
				strcpy(fstyp, "S5  ");
			}
			else if(vno.v_op == (struct vnodeops *)Ufs_vnodeops->n_value){
				strcpy(fstyp, "UFS  ");
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
	
			fprintf(fp,"fstype/f_vnode: %s/%8x, ",fstyp,aioactive.ar_aiofile.f_vnode);
		}
		fprintf(fp,"offset: %8x,",aioactive.ar_aiofile.f_offset);
		fprintf(fp,"  %s%s%s%s%s%s%s%s\n",
			aioactive.ar_aiofile.f_flag & FREAD ? " read" : "",
			aioactive.ar_aiofile.f_flag & FWRITE ? " write" : "",  /* print the file flag */
			aioactive.ar_aiofile.f_flag & FAPPEND ? " appen" : "",
			aioactive.ar_aiofile.f_flag & FSYNC ? " sync" : "",
			aioactive.ar_aiofile.f_flag & FCREAT ? " creat" : "",
			aioactive.ar_aiofile.f_flag & FTRUNC ? " trunc" : "",
			aioactive.ar_aiofile.f_flag & FEXCL ? " excl" : "",
			aioactive.ar_aiofile.f_flag & FNDELAY ? " ndelay" : "");
	
	

		/* user credentials */
		fprintf(fp, "\nUser Credentials: \n");
		if(!Ngrps)
			if(!(Ngrps = symsrch("ngroups_max")))
				error("ngroups_max not found in symbol table\n");
		readmem((long)Ngrps->n_value, 1, -1, (char *)&ngrpbuf,
			sizeof ngrpbuf, "max groups");

		credbufp=(struct cred *)malloc(sizeof(struct cred) + sizeof(uid_t) * (ngrpbuf-1));
		readmem((long)aioactive.ar_aiofile.f_cred,1,-1,(char *)credbufp,sizeof (struct cred) + sizeof(uid_t) * (ngrpbuf-1),"user cred");
		fprintf(fp,"\trcnt:%3d, uid:%d, gid:%d, ruid:%d, rgid:%d, ngroup:%4d",
			credbufp->cr_ref,
			credbufp->cr_uid,
			credbufp->cr_gid,
			credbufp->cr_ruid,
			credbufp->cr_rgid,
			credbufp->cr_ngroups);
		for(i=0; i < (short)credbufp->cr_ngroups; i++){
			if(!(i % 4))
				fprintf(fp, "\n");
			fprintf(fp,"\tgroup[%d]:%4d ", i, credbufp->cr_groups[i]);
		}
		fprintf(fp, "\n");
		fprintf(fp,"Process Class Parameters: \n");

		switch(aioactive.ar_pri.pc_cid) {
		
		case 1:
			fprintf(fp,"\tcid: %d,  upri: %d\n", aioactive.ar_pri.pc_cid, 
				((tsparms_t *)aioactive.ar_pri.pc_clparms)->ts_upri);
			break;
	
		case 2:
			fprintf(fp, "\tcid: %d,  pri: %d,  tqsecs: %d,  tqnsecs: %d  \n",
			aioactive.ar_pri.pc_cid,
 			((rtparms_t *)aioactive.ar_pri.pc_clparms)->rt_pri,
			((rtparms_t *)aioactive.ar_pri.pc_clparms)->rt_tqsecs,
			((rtparms_t *)aioactive.ar_pri.pc_clparms)->rt_tqnsecs);
			break;
		}

		fprintf(fp, "Ecb: \n");
		fprintf(fp, "\teqd: %d,  flags: %d,  eid: %d,  evpri: %d\n",
			aioactive.ar_ecb.ecb_eqd, aioactive.ar_ecb.ecb_flags,
			aioactive.ar_ecb.ecb_eid, aioactive.ar_ecb.ecb_evpri);

		fprintf(fp, "\tevkevp: %x,  serverp: %x\n",
			 aioactive.ar_evkevp, aioactive.ar_serverp);
	}

	if(!full)
		return;

	if(!Min)
		if(!(Min = symsrch("min_aio_servers")))
			error("min_aio_servers not found in symbol table\n");
	if(!Max)
		if(!(Max = symsrch("max_aio_servers")))
			error("max_aio_servers not found in symbol table\n");
	if(!Timeout)
		if(!(Timeout = symsrch("aio_server_timeout")))
			error("aio_server_timeout not found in symbol table\n");
	if(!Size)
		if(!(Size = symsrch("aio_size")))
			error("aio_size not found in symbol table\n");
	if(!Naio)
		if(!(Naio = symsrch("naioproc")))
			error("naioproc not found in symbol table\n");
	if(!Curaio)
		if(!(Curaio = symsrch("cur_aio_servers")))
			error("cur_aio_servers not found in symbol table\n");

	readmem((long)Min->n_value, 1, -1, (char *)&minbuf,
		sizeof minbuf, "minimum aio servers");
	readmem((long)Max->n_value, 1, -1, (char *)&maxbuf,
		sizeof maxbuf, "max aio servers");
	readmem((long)Timeout->n_value, 1, -1, (char *)&timebuf,
		sizeof timebuf, "secs servers should wait");
	readmem((long)Size->n_value, 1, -1, (char *)&sizebuf,
		sizeof sizebuf, "aio requests in pool");
	readmem((long)Naio->n_value, 1, -1, (char *)&naiobuf,
		sizeof naiobuf, "async requests per process");
	readmem((long)Curaio->n_value, 1, -1, (char *)&curaiobuf,
		sizeof curaiobuf, "current # of aio servers");

	fprintf(fp, "\nSystem Configuration:  \n");
	fprintf(fp, "min_aio: %d  max_aio: %d  timeout: %d\n",
		minbuf, maxbuf, timebuf);
	fprintf(fp, "aio_size: %d  naioproc: %d  cur_aio: %d\n",
		sizebuf, naiobuf, curaiobuf);


	/* print the number of free list request structs */

	if(!Free)
		if(!(Free = symsrch("aio_free")))
			error("aio_free not found in symbol table\n");

	readmem((long)Free->n_value, 1, -1, (char *)&freebuf,
		sizeof freebuf, "free list of aio request structs");

	faiop=freebuf.ar_sflink;
	for(i=0; faiop != NULL; faiop=freebuf.ar_sflink, i++)
		readmem((long)faiop, 1, -1, (char *)&freebuf,
			sizeof freebuf, "free list of aio request structs");
		
	fprintf(fp, "free list count = %d\n", i);

}
