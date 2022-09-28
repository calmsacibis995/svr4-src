/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sadp.c	1.14.1.16"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/buf.h>
#include <sys/elog.h>
#include <a.out.h>
#include <sys/iobuf.h>

#include "sys/pdi.h" 
#include "sys/scsi.h"
#include "sys/sdi.h"
#include <sys/vtoc.h>
#include "sys/disktd.h"  


#include <time.h>
#include <sys/utsname.h>
#include <sys/var.h>
#include <ctype.h>

#include <stdio.h>
#include <sys/immu.h>
#include <sys/bootinfo.h>

/* cylinder profiling */
#define BLANK ' '
#define BLOB '*'
#define TRACE '.'
#define BRK '='
#define FOOT '-'
#define CYLNO   1
#define SEEKD   2
#define	CHPERCYL	103	/*the number of 8 cylinder chunks on a disk*/


#define cylin b_resid;
#define NDRIVE	3
#define SNDRIVE	56
#define V	0
#define ID	1
#define SD01	2
#define IF	3
#define	IDTAB	4
#define	HDU	4
#define L_SCSI 	5
#define	FDU	6
#define BOOTINFO	7
#define SBF	8
#define PBF	9
#define SWP1	10
#define SWP2	11

struct nlist setup[] = {
	{"v"},
	{"hdstat"},	/*  idtime"},  */
	{"junk"},
	{"ifstat"},
	{"hdutab"}, 	/*	HARD {"idtab"}, 	*/
	{"Sd01_d"},
	{"iftab"},
	{"bootinfo"},
	{"buf"},
	{"pbuf"},
	{"swbuf1"},
	{"swbuf2"},
	{0}
};

int idcnt;
#define	IDNDRV	2

#ifdef i386
struct bootinfo bootinfo;
#else
struct idstatstruct idstatus[IDNDRV];
#endif

struct iobuf dkp[NDRIVE];
struct iotime ib[NDRIVE];
struct var tbl;
char *sbuf, *phybuf;
struct buf bp[2];		/*  for swap buffers  */
int nonblk;
int index;
int index1;
unsigned temp1;

char devnm[3][5] = {
	"hdsk", 		/* integral hard disk */
	"sdsk",			/* SCSI disk */
	"fdsk"			/* integral floppy disk	*/
};
#define CHUNKS	100
#define CYLS_PER_CHUNK	20

int	Sd01_diskcnt;
struct	last_time_info {
	int	end_index;
	long	b_blkno;
};

struct	drive_information {
	int	monitored;
	long	io_ops;
	struct	last_time_info last_time;
	long	chunk_accessed[CHUNKS];
	long	seek_dist[CHUNKS];
	long	total_accesses;
	long	sum_seeks;
};
struct drive_information *drive_info;
struct disk *dsd01;



 

int fflg,dflg,tflg,hflg,errflg;
int s,n,ct;
static int ub = 8;
int sdist;
int m;
int dev;
int temp;
int f;
int i;
int n1,dleng,dashb,k;
int dashf;
int dn;
int drvlist[NDRIVE];
int Sdrvlist[SNDRIVE];	/* SCSI */
struct HISTDATA {
	long    hdata[CHPERCYL];
};
struct utsname name;
 
char *nopt;
char empty[30];
char drive[30];
char *malloc();
int  SCSI;    			/*         SCSI*/
int  ALL;
long lseek();
long dkcyl[8][CHPERCYL];
long skcyl[8][CHPERCYL];
long iocnt[8];
 
main(argc,argv)
int argc;
char **argv;
{
	unsigned sleep();
	unsigned actlast,actcurr;
	extern  int     optind;
	extern  char    *optarg;
	int c,j;
	char *ctime(),*stime;
	long curt;
	extern time_t time();
	long skdist[8];
	long disk[8];


	while ((c = getopt(argc,argv,"thd:")) != EOF)
		switch(c) {
		case 't':
			tflg++;
			break;
		case 'h':
			hflg++;
			break;
		case 'd':
			dleng = strlen(optarg);
			if (dleng == 5){
				errflg++;
				break;
			}
			if ( strncmp(optarg,"sdsk",4) == 0)
				SCSI = 1;
			if (dleng > 4){
			for (i=5,n1=5;i<dleng;i++){
				if (optarg[i] == ','){
					if (n1 == i){
					errflg++;
					break;
					}
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					if (dashf != 0) {
						if (dashb >= dn){
							errflg++;
							break;
						}
						for (j=dashb;j<dn+1;j++){
							if (SCSI) /*  SCSI */
							 	Sdrvlist[j] = 1;
							else
								drvlist[j] = 1;
						}
						dashb = 0;
						dashf = 0;
					}
					else
					{
						if (SCSI)
						    	Sdrvlist[dn] = 1;
						else
							drvlist[dn] = 1;
					}
				n1 = i+1;
				}
				else {
				if (optarg[i] == '-'){
					if (dashf != 0) {
						errflg++;
						break;
					}
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					if(SCSI)
						Sdrvlist[dn] = 1;
					else
						drvlist[dn] = 1;
					dashb = dn;
					dashf = 1;
					n1=i+1;
				}
				else { 
					if (i == dleng-1){
					i++;
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					if (dashf != 0)
						for (j=dashb;j<dn+1;j++){
							if (SCSI)
								Sdrvlist[j] =1;
							else
								drvlist[j] = 1;
						}
					else
					{
						if(SCSI)
							Sdrvlist[dn] = 1;
						else
							drvlist[dn] =1;
					}
					}
				}
				}
			}
			}
			else {
				if (dleng != 4){
					errflg++;
					break;
				}
				if (SCSI)
					ALL++;
				else
					for (i=0;i<8;i++) 
						drvlist[i] = 1;
			}
			if (errflg)
				break;
			temp = 3;

			for (i=0;i<temp;i++)
				if (strncmp(optarg,devnm[i],4) == 0){
					dev = i;
					break;
				}
			if (i == temp){
				errflg++;
				break;
			}
			dflg++;
			break;
		case '?':
			errflg++;
			break;
		}
	if (errflg)
		exit1();

/*      if no frequency arguments present, exit */
	if (optind == argc)
		exit1();
/*      if a non-dash field is presented as an argument,
	check if it is a numerical arg.
*/
	nopt = argv[optind];
	if (tstdigit(nopt) != 0)
		exit1();
/*      for frequency arguments, if only s is presented , set n to 1
*/
	if ((optind +1) == argc) {
		s = atoi(argv[optind]);
		n = 1;
	}
/*      if both s and n are specified, check if 
	arg n is numeric.
*/
	if ((optind +1) < argc) {
		nopt = argv[optind + 1];
		if (tstdigit(nopt) != 0)
			exit1();
		s = atoi(argv[optind]);
		n = atoi(argv[optind+1]);
	}
	if ( s <= 0 )
		extmsg("bad value of s");
	if ( n <= 0 )
		extmsg("bad value of n");
	ct = s;
/*      get entries defined in setup from /stand/unix */

	nlist("/stand/unix",setup);


/*      open /dev/kmem  */

	if((f= open("/dev/kmem",0)) == -1)
		extmsg("cannot open /dev/kmem");








































fprintf (stderr, "dev = %d	dflg = %d\n", dev, dflg);
	if (dflg == 0){
		for (i=0;i<3;i++){
			if (setup[i+IDTAB].n_value != 0 ){
				dev = i;
				break;
			}
		}

		if (i == 3)
			extmsg("Neither hdsk, scsi nor fdsk is defined");

		for (i=0;i<NDRIVE;i++)
			drvlist[i] =1;
	}
	else 
		if (setup[dev+IDTAB].n_value == 0){
			fprintf(stderr,"Device %s is not defined\n",devnm[dev]);
			exit(2);
		}


/*      set upper bound on number on drive
	if hard drive is device of interest - find out
	the number of drives present (idcnt)
*/

	if (dev == 0){
		lseek(f,(long)setup[BOOTINFO].n_value,0);

		if (read(f,&bootinfo,sizeof bootinfo) == -1)
			extmsg("cannot read hard disk count");
		for (i = 0; i < IDNDRV; i++)
			if (bootinfo.hdparams[i].hdp_ncyl)
				idcnt++;
		ub = idcnt;

	} else {
/*		Its a floppy - only one of these  */
		ub = 1;
	}

fprintf (stderr, "ub = %d or %d hard drives\n",ub, ub);

	if (dflg && dn >= ub) { 
		fprintf(stderr, "Device %s, drive %2d is not equipped\n",
				devnm[dev], dn);
		exit(2);
	}
/*      get storage from memory for sysbuf pool and physical buy pool      */
fprintf (stderr, "get storage from memory\n");

	lseek(f,(long)setup[V].n_value,0);
	read(f,&tbl,sizeof tbl);
	sbuf = malloc( sizeof(struct buf) * tbl.v_buf );
	if (sbuf == NULL)
		exit2();
	phybuf = malloc( sizeof(struct buf) * tbl.v_pbuf );
	if (phybuf == NULL)
		exit2();

/*      get current I/O count for each drive    */
fprintf (stderr, "get current I/O count for each drive  \n");
	for (;;){
		s = ct;
		for (k=0,i=0;k<ub;k++){

fprintf (stderr, "devlist = %d\n", drvlist[k]);
			if (drvlist[k] == 0)
				continue;
			for (j=0;j<CHPERCYL;j++){
				dkcyl[i][j] = 0;
				skcyl[i][j] = 0;
			}
			iocnt[i] = 0;
			disk[i] = 0;
			skdist[i] = 0;
			i++;
			if (i == 8){
				ub = k+1;  /*  only 8 drives are allowed */
				break;
			}
		}
/*      if no drives are selected or illegal drive number is specified, exit    */
		if (i == 0) 
			exit1();

fprintf (stderr, "get i/o count for each disk   dev=%d \n", dev);
/*      get i/o count for each disk     */
		lseek(f,(long)setup[dev+ID].n_value,0);
		if(read(f,ib,(sizeof (struct iotime)*ub)) == -1)
			extmsg("cannot read device status table");
		for(k=0,i=0;k<ub;k++)
		{
fprintf (stderr, "--> devlist = %d  %d\n", drvlist[k], k);
			if (drvlist[k] == 0)
				continue;
			iocnt[i] = ib[k].io_cnt;
fprintf (stderr, "--> %d\n", iocnt[i]);
			i++;
		}

	for (;;){

/*      take a snapshot of buffer header pool , swap buffer pool and
	physical buffer header  */

fprintf (stderr, "Reading hard driver iobuf\n");
		lseek(f,(long)setup[IDTAB+dev].n_value,0);
		if( read(f,dkp,(sizeof (struct iobuf)* ub)) == -1){
			perror("sadp");
			extmsg("cannot read disk drive iobuf");
		}
 
/*      read system buffer header pool   */
fprintf (stderr, "read system buffer header pool \n");
		lseek(f,(long)setup[SBF].n_value,0);
		if (read(f,sbuf,tbl.v_buf*sizeof(struct buf)) == -1){
			perror("sadp");
			extmsg("cannot read system buffer pool");
		}


fprintf (stderr, "read physical buffer header pool   \n");
		lseek(f,(long)setup[PBF].n_value,0);
		if (read(f,phybuf,tbl.v_pbuf*sizeof(struct buf))== -1){
			perror("sadp");
			extmsg("cannot read physical buffer pool");
		}


fprintf (stderr, "reading V, ID \n");
		for (m=SWP1;m<SWP2+1;m++){
			lseek(f,(long)setup[m].n_value,0);
fprintf (stderr, "%d   %d\n", m, m-SWP1);
			if (read(f,bp[m-SWP1],sizeof (struct buf)) == -1){
				fprintf(stderr,"cannot read phy bufhdr - %d\n",m);
				perror("sadp");
				exit(1);
			}
		}

#ifndef i386
		lseek(f,(long)setup[SWP].n_value,0);
		if(read(f,bp,sizeof bp) == -1){
			fprintf(stderr,"cannot read swap bufhdr  %o\n",
			setup[5].n_value);
			perror("sadp");
			exit(1);
		}
#endif





fprintf (stderr, "trace disk queue for I/O location, seek distan\n");
		for (k=0,i=0;k<ub;k++){
			if (drvlist[k] == 0)
				continue;

/*      trace disk queue for I/O location, seek distance	*/

			nonblk = 0;

fprintf (stderr, "dkp[k] -- %d\n", dkp[k].b_actf);
			if(dkp[k].b_actf != NULL ) {
				temp1 = (unsigned)dkp[k].b_actf; 
				actlast = (unsigned)dkp[k].b_actl;

				actlast = temp1 - 1020 ;
fprintf (stderr, "temp1 = %d          actlast = %d\n", temp1, actlast);

				index1 = 0;
				index = (int)(temp1 -setup[SBF].n_value)/
					(sizeof (struct buf));

fprintf (stderr, "index = %d\n", index);

				if ((testbuf() == -1) ||
					 (testdev() == -1) ||
					 ((unsigned)dkp[k].b_actf ==
					    (unsigned)dkp[k].b_actl)) {
					i++;
					continue;
				}
				sdist = temp;

				while (temp1 != NULL){
					nonblk = 0;
					actcurr =temp1;
					index1 =index;
					index =(int)(temp1 -setup[SBF].n_value)/
						(sizeof (struct buf));
					if ((testbuf() == -1) ||
						 (testdev() == -1))
						break;
					sdist = temp - sdist;
					if (sdist < 0)
						sdist = -sdist;
					skcyl[i][(sdist+7) >> 3]++;
					sdist = temp;

					if (actcurr == actlast)
						break;
				}
			}
		i++;
		}

		if (--s)
			sleep(1);
		else{

/*      at the end of sampling, get the present I/O count,
	and system name */

			lseek(f,(long)setup[dev+ID].n_value,0);
			read(f,ib,(sizeof (struct iotime)*ub));

			uname(&name);

/*      print the report, there are two parts:
	cylinder profile, seeking distance profile */
			curt = time((long *) 0);
			stime = ctime (&curt);
			printf("\n\n%s\n",stime);
			printf("%s %s %s %s %s\n",
				name.sysname,
				name.nodename,
				name.release,
				name.version,
				name.machine);
			for (k=0,i=0;k<ub;k++){
			if (drvlist[k] == 0)
				continue;
				for (j=0;j<CHPERCYL;j++){
					disk[i] = disk[i] +dkcyl[i][j];
					skdist[i] = skdist[i] + skcyl[i][j];

				}
			i++;
			}
			if ((tflg == 0) && (hflg == 0))
				tflg = 1;
			if (tflg){
				printf("\nCYLINDER ACCESS PROFILE\n");
				for (k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
					continue;
					if (disk[i] != 0){
#if u3b || u3b15 || u370
						iocnt[i] = iocnt[i] - (long)linfo[i].iocnt;
#else
						iocnt[i] = ib[k].io_cnt - iocnt[i];
#endif
						printf("\n%s-%d:\n",
							devnm[dev],k);
						printf("Cylinders\tTransfers\n");
						for (j=0;j<CHPERCYL;j++){
							if (dkcyl[i][j] > 0)
								printf("%3d - %3d\t%ld\n",
								j*8,j*8+7,dkcyl[i][j]);
						}
						printf("\nSampled I/O = %ld, Actual I/O = %ld\n",
						disk[i],iocnt[i]);
						if (iocnt[i] > 0)
						printf("Percentage of I/O sampled = %2.2f\n",
						((float)disk[i] /(float)iocnt[i]) * 100.0);
					}
					i++;
				}

				printf("\n\n\nSEEK DISTANCE PROFILE\n");
				for (k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (skdist[i] != 0){
						printf("\n%s-%d:\n",
							devnm[dev],k);
						printf("Seek Distance\tSeeks\n");
						for(j=0;j<CHPERCYL;j++)
	
							if (skcyl[i][j] > 0){
								if (j == 0)
									printf("        0\t%ld\n",
									skcyl[i][j]);
								else
									printf("%3d - %3d\t%ld\n",
								j*8-7,j*8,skcyl[i][j]);
							}
						printf("Total Seeks = %ld\n",skdist[i]);
					}
					i++;
				}
			}
			if (hflg){
				for(k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (disk[i] != 0) {
						cylhdr(CYLNO,disk[i]);
						cylhist(disk[i],dkcyl[i]);
						cylftr(CYLNO);
					}
					i++;
				}
				for(k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (skdist[i] != 0){
						cylhdr(SEEKD,skdist[i]);
						cylhist(skdist[i],skcyl[i]);
						cylftr(SEEKD);
					}
					i++;
				}
			}

			break;
		}
	}
	if (--n)
		continue;
	exit(0);
	}
}


#if pdp11 || vax || u3b2 ||  i386
/*      determine if the I/O is from system buffer pool,
	or swap buffer pool or physical buffer  */
 
int testbuf()
{
#if u3b2 || i386
	if ((temp1 <setup[SBF].n_value) || (index > tbl.v_buf)){
#else
	if ((temp1 <setup[SBF].n_value) || (index > (tbl.v_buf + tbl.v_sabuf))){
#endif
		index = (int)(temp1 -setup[PBF].n_value)/
			(sizeof (struct buf));
		if (index < tbl.v_pbuf){
			nonblk = 1;
			return(0);

		}
#if vax || u3b2 || i386
		for (m=SWP1;m<SWP2;m++){
			if (temp1 == setup[m].n_value){
				m = m-SWP1;
				nonblk =2;
				return(0);
			}
		}
	return(-1);

#else
	index = (int)(temp1 -setup[SWP].n_value)/
		(sizeof (struct buf));
	if (index < NSWP) {
		m = index;
		nonblk = 2;

		return(0);
	}
	else return(-1);
#endif
	}
	return(0);
}

/*      varify the I/O, get the cylinder number */

ckbits(x)
	register struct buf *x;
{
	register p;
	for (p=0;p<index;p++,x++)
		continue;
	if((x->b_flags & B_BUSY) &&
	    ((x->b_flags & B_DONE) == 0)){
		temp = x->cylin;
#ifdef vax
		temp1 = (unsigned)x->av_forw -
		    0x80000000;
#else
		temp1 = (unsigned)x->av_forw;
#endif
		return(0);
	}
	else
		return(-1);

}
int testdev()
{
	if((nonblk == 0) && (ckbits(sbuf) != -1))
		goto endtest;
	else {
		if ((nonblk == 1) && (ckbits(phybuf) != -1))
			goto endtest;

		else {

			if ((nonblk == 2) &&
			    ((bp[m].b_flags & B_BUSY) &&
			    ((bp[m].b_flags & B_DONE) == 0))){
				temp = bp[m].cylin;
#ifdef vax
				temp1 = (unsigned)bp[m].av_forw - 0x80000000;
#else
				temp1 = (unsigned)bp[m].av_forw;
#endif
			}
			else
				return(-1);
		}
	}
endtest:
	dkcyl[i][temp >> 3]++;
	return(0);
}
#endif

#if u3b || u370
dsktbl(x)
register struct dskinfo *x;
{
	lseek (f,(long)setup[DSKINFO].n_value,0);
	if (read(f,x,dskcnt*sizeof (struct dskinfo)) == -1)
		return(-1);
	return(0);
}
getiocnt(x)
register struct dskinfo *x;
{
	register p,i;
	for (p=0,i=0;p<ub;p++,x++)
		if (drvlist[p] == 0)
			continue;
		else{
#ifdef u3b
			iocnt[i] = x->devinfo.io_bcnt;
#else
			iocnt[i] = x->blkcnt;
#endif
			i++;
		}
	return;
}
 
ldlinfo(x)
register struct dskinfo *x;
{
	register i,k;
	for (i=0,k=0;k<ub;k++,x++)
		if (drvlist[k] == 0)
			continue;
		else {
			linfo[i].lstptr = (int)x->index == 0? 9: (x->index -1);
			linfo[i].bknum = x->outq[linfo[i].lstptr].blknum;
			i++;
		}
}

getsample(x)
register struct dskinfo *x;
{
	register i,k;
	int cntflg=0;
register j;
	for (i=0,k=0;k<ub;k++,x++){
		if (drvlist[k] == 0)
			continue;
		if (linfo[i].bknum == 0 && x->index == 0 &&
			x->outq[x->index].blknum == 0){
			i++;
			continue;
		}
		if (linfo[i].bknum == x->outq[linfo[i].lstptr].blknum)
			if (x->index == ((linfo[i].lstptr + 1) % 10)){
				i++;
				continue;
			} else {
				cptr = linfo[i].lstptr;
				cntflg = 1;
			}
		else
			cptr = x->index;
		blkno = x->outq[cptr].blknum;
		getcyl(blkno,x);
		if (cntflg)
			cntflg = 0;
		else
		dkcyl[i][temp >> 3]++;
		sdist = temp;
		while (cptr != x->index){
			blkno = x->outq[cptr].blknum;
			getcyl(blkno,x);
			dkcyl[i][temp >> 3]++;
			sdist = temp -sdist;
			if (sdist < 0)
				sdist = -sdist;
			skcyl[i][(sdist+7) >> 3]++;
			sdist = temp;
		}
		linfo[i].lstptr = (int)cptr == 0? 9: (cptr - 1);
		linfo[i].bknum = blkno;
		i++;
	}
}

getcyl(y,x)
register struct dskinfo *x;
register uint y;
{
	int res;
	int bpercyl;	/*blocks per cylinder*/
/* Calculate blks per cyl for 3b disk drive type.
   There are 608 blks per cyl on a 300Mb disk,
   768 blks per cyl on a 340Mb disk and 1280
   blocks per cyl on a 675Mb disk drive. */

	switch(x->numtrks) {

	case TRACKS300:
		bpercyl = (HEADS300 * SECTSIZ);
		break;

	case TRACKS340:
		bpercyl = (HEADS340 * SECTSIZ);
		break;

	case TRACKS675:
		bpercyl = (HEADS675 * SECTSIZ);
		break;

	default:
		bpercyl = (HEADS300 * SECTSIZ);
		break;
	}
	res = (y % bpercyl) == 0? 0: 1;
	temp = (int)(blkno / bpercyl) + res;
	cptr = (cptr +1) % 10;
	return(0);
}
exit3()
{
	fprintf(stderr,"cannot read dskinfo table\n");
	exit(3);
}
#endif
 
#ifdef u3b15
dftbl(x)
register struct dfc *x;
{
	int i, j, delta;

	lseek (f,(long)setup[DFDFC].n_value,0);
	if (read(f,x,dfcnt*sizeof (struct dfc)) == -1)
		return(-1);
	/* 
		In Release 1.1, the disk performance queue is managed by
		a pointer to the end of the queue and a pointer to the
		next entry to be used.  After the disk block is copied from
		/dev/kmem to the malloc area, these pointers are garbage since
		they point back to /dev/kmem locations.  To update these
		pointers, compute the difference between the new end of queue
		and the old end of queue pointers (delta) and update the
		two pointers by this delta amount.
	*/
	for (i = 0; i < dfcnt; ++i) {	/* for each Disk controller */
		for (j = 0; j < NDRV; ++j) {	/* for each drive on the controller */
			delta = (int)(&x->df_stat[j].ptrackq[NTRACK]) - (int)x->df_stat[j].endptrack;
			x->df_stat[j].pttrack = (struct df_ptrack *)(((int)x->df_stat[j].pttrack) + delta);
			x->df_stat[j].endptrack = (struct df_ptrack *)(((int)x->df_stat[j].endptrack) + delta);
		}
		++x;	/* advance pointer to next controller structure */
	}
	/* end of release 1.1 pointer mod */
	return(0);
}
getiocnt(x)
register struct dfc *x;
{
	register p,i,q;

	for (p=0, i=0, q=0; q<ub; q++) {
		if (drvlist[q] != 0) {
			iocnt[i] = x->df_stat[p].io_bcnt;
			i++;
		}
		p++;
		if ((p % NDRV) == 0) {
			p=0;
			x++;
		}
	}
	return;
}
 
/* END_INDEX produces index of first entry beyond last used
 *	range of values is 0 to NTRACK - can only be 0 if no entries
 *	have been made in the ptrack queue
 */
#define  END_INDEX(drive) (x->df_stat[drive].pttrack - x->df_stat[drive].ptrackq)

ldlinfo(x)
register struct dfc *x;
{
	register i,		/* index to linfo table */
		 k,		/* physical unit on dfc indicated by x */
		 l,		/* system-wide drive number */
		 index;		/* index into ptrackq */

	for (i=0, k=0, l=0; k<ub; k++) {
		if (drvlist[k] != 0) {
			index = END_INDEX(l) - 1;
			if (index < 0)  /* check for no entries */
				index = 0;
			linfo[i].lstptr = index;
			linfo[i].bknum = x->df_stat[l].ptrackq[index].b_blkno;
			i++;
		}

		if ( ++l % NDRV == 0) {
			l = 0;
			x++;
		}
	}
}

getsample(x)
register struct dfc *x;
{
	register	i,	/* index into linfo array */
			k,	/* system-wide drive number */
			l;	/* physical unit on dfc represented by x */
	int cntflg=0;
	int index;		/* index into ptrackq array */
	int cylinder;
	struct df_ptrack *pptr; /* pointer to the first entry in ptrackq */

	for (i=0,k=0,l=0 ;k<ub; k++){
		if (drvlist[k] == 0)
			goto next;
		index = END_INDEX(l) % NTRACK;
		pptr = x->df_stat[l].ptrackq;

		/* if no entries have been left in the list of completed
		   blocks, skip it */

		if (linfo[i].bknum == 0 && index == 0 &&
			pptr[index].b_blkno == 0){
			i++;
			goto next;
		}

		/* if the entry indicated by linfo[i].lstptr is unchanged
		   since our last probe, then either nothing has happened
		   or entries have been added to the circular list without
		   overwriting the entry last spied.
		   If the entry has changed, then we know that all entries
		   in the list have been writen since we were here last */
		if (linfo[i].bknum == pptr[linfo[i].lstptr].b_blkno)
			if (index == ((linfo[i].lstptr + 1) % 10)){
				i++;
				goto next;
			} else {
				cptr = linfo[i].lstptr;
				cntflg = 1;
			}
		else
			cptr = index;
		blkno = pptr[cptr].b_blkno;
		cylinder = getcyl(blkno);
		cptr = (cptr + 1) % NTRACK;
		if (cntflg)
			cntflg = 0;
		else
			dkcyl[i][cylinder >> 3]++;
		sdist = cylinder;

		/* walk around the circle until you reach the point at 
		   which records are being added */
		while (cptr != index){
			blkno = pptr[cptr].b_blkno;
			cylinder = getcyl(blkno);
			cptr = (cptr + 1) % NTRACK;
			dkcyl[i][cylinder >> 3]++;
			sdist = cylinder -sdist;
			if (sdist < 0)
				sdist = -sdist;
			skcyl[i][(sdist+7) >> 3]++;
			sdist = cylinder;
		}
		linfo[i].lstptr = (int)cptr == 0? NTRACK-1: (cptr - 1);
		linfo[i].bknum = blkno;
		i++;
next :		if ((++l % NDRV) == 0) {
			l = 0;
			x++;
		}
	}
}

int getcyl(block)
register uint block;
{
/* This block number to cylinder conversion is valid only for the
   Lark 1 drives. If it's valid for any other drives, it's pure 
   coincidence */

	int track, cyl;

	track = block / 32;		/* remainder = sector */
	if (track > 399)
		track -= 400;
	else
		track += 400;
	cyl = track / 2;		/* remainder = head */
	if (cyl > 199)
		cyl -= 200;
	return(cyl);
}



exit3()
{
	fprintf(stderr,"cannot read dskinfo table\n");
	exit(3);
}
#endif
 
/*      get drive number routine	*/
getdrvn()
{
	extern char *optarg;
	char *strcpy();
	char *strncat();
 
	strcpy(drive,empty);
	strncat(drive,&optarg[n1],i-n1);
	if (tstdigit(drive) != 0)
		return (-1);
	dn =atoi(drive);
	if(SCSI) {
		if (dn >= SNDRIVE)
			return(-1);
	}
	else {
		if (dn >= NDRIVE)
			return(-1);
	}
	return(0);
}

exit1()
{
	fprintf(stderr,"usage:  sadp [-th][-d device[-drive]] s [n]\n");
	exit(1);
}

exit2()
{
	fprintf(stderr,"sadp: can't get memory, TRY AGAIN!!\n");
	exit(2);
}

extmsg(msg)
char	*msg;
{
	fprintf(stderr, "sadp: %s\n", msg);
	exit(4);
}

int tstdigit(ss)
char *ss;
{
	int kk,cc;
	kk=0;
	while ((cc = ss[kk]) != '\0' ){
		if (isdigit(cc) == 0)
			return(-1);
		kk++;
	}
	return(0);
}

/*      the following routines are obtained from iostat */
/* cylinder profiling histogram */
/*.s'cylhist'Output Cylinder Histogram'*/
cylhist(at, dp)
long at;
register struct HISTDATA *dp;
{
	register ii;
	int maxrow;
	long graph[CHPERCYL];
	long    max, max2;
	long    data;
	long    scale;

	max = 0;
	for(ii = 0; ii < CHPERCYL; ii++) {
		if(data = dp->hdata[ii]) {
			maxrow = ii;
			if(data > max) {
				max2 = max;
				max = data;
			} 
			else if (data > max2 && data != max)
				max2 = data;
		}
	}
	maxrow++;

	/* determine scaling */
	scale = 1;
	if ( max2 ) {
		scale = at / ( max2 * 2 );
		if ( scale > 48 )
			scale = 48;
		}

	for(ii = 0; ii < maxrow; ii++) {
		if(dp->hdata[ii])
			graph[ii] = (scale * 100 * dp->hdata[ii]) / at;
		else
			graph[ii] = -1;
	}

	prthist(graph, maxrow, scale, (long) (max*100*scale/at));
}
/*.s'prthist'Print Histogram'*/

prthist(array, mrow, scale, gmax)
	long array[], scale, gmax;
register mrow;
{
	long    line;

	line = 50;
	/* handle overflow in scaling */
	if(gmax > 51) {
		line = 52;
		printf("\n%2ld%% -|", gmax/scale);
		pline(line--, array, mrow, BLOB);
		printf("\n     %c", BRK);
		pline(line--, array, mrow, BRK);
	} 
	else if ( gmax = 51 )
		line = 51;
	while( line > 0) {
		if((line & 07) == 0) {
			printf("\n%2ld%% -|", line/scale);
		} 
		else {
			printf("\n     |");
		}
		pline(line--, array, mrow, BLOB);
	}
	printf("\n 0%% -+");
	line = -1;
	pline( line, array, mrow, FOOT);
}
/*.s'pline'Print Histogram Line'*/
pline(line, array, mrow, dot)
	long line, array[];
int mrow;
char dot;
{
	register ii;
	register char *lp;
	char lbuff[132];

	lp = lbuff;
	for(ii = 0; ii < mrow; ii++)
		if(array[ii] < line)
			if(line == 1 && array[ii] == 0)
				*lp++ = TRACE;
			else
				*lp++ = BLANK;
		else
			*lp++ = dot;
	*lp++ = 0;
	printf("%s", lbuff);
}
/*.s'cylhdr'Print Cylinder Profiling Headers'*/

cylhdr( flag, total)
	long total;
{
	if(fflg)
		printf("\014\n");
	if( flag == CYLNO)
		printf("\nCYLINDER ACCESS HISTOGRAM\n");
	if (flag == SEEKD)
		printf("\nSEEK DISTANCE HISTOGRAM\n");
	printf("\n%s-%d:\n",
		devnm[dev],k);
	printf("Total %s = %ld\n", flag==CYLNO ? "transfers" : "seeks", total);
}
/*.s'cylftr'Print Histogram Footers'*/

cylftr( flag )
{
	if (flag == CYLNO)
		printf("\n      \t\t\tCylinder number, granularity=8");
	else
		printf("\n      =<<\t<\t<\t<\t<\t<\t<\t<\t<");
	printf("\n      081\t8\t1\t2\t2\t3\t4\t4\t5");
	printf("\n        6\t0\t4\t0\t7\t3\t0\t6\t2");
	printf("\n         \t \t4\t8\t2\t6\t0\t4\t8");
	printf("\n");
}
SD01_profiler()
{
        void populate(); 
	void initialize_info();
	void extract_info();
	void print_results();
	unsigned sleep();

	register int j;			/* Scratch loop counter.	*/
	register int time_left;		/* Time left in this interval.	*/

	while (n--)	{
		initialize_info();
		time_left = s;
		while(time_left--) {
			populate();
			extract_info();
		if (time_left)
			sleep(1);
		}

		/*
		 * At the end of the interval, get the present I/O count
		 * and subtract from it the I/O count of the beginning of
		 * the interval to get the number of blocks transferred
		 * to and from each disk drive during the interval.
		 */
		populate();
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;	/* Skip this drive. */
			drive_info[j].io_ops = dsd01[j].dk_stat.ios.io_ops - drive_info[j].io_ops;
		}

		print_results();
	}
	exit(0);
}



/*
 * initialize_info()
 */

void
initialize_info()

{
	register int  j, k;   /* Loop counters 	*/
	register int index;	/* Index into ptrackq.	*/
	void populate();

	if (setup[SD01].n_value != 0) {
		lseek(f,(long)setup[SD01].n_value, 0);
		if (read(f, &Sd01_diskcnt, sizeof Sd01_diskcnt) != sizeof Sd01_diskcnt)
			extmsg("cannot read SD01 disk count");
		drive_info = (struct drive_information *) malloc(sizeof(struct drive_information) * Sd01_diskcnt);
			if(drive_info == NULL)
				extmsg("cannot malloc");
		dsd01 = (struct disk *) malloc(sizeof (struct disk) * Sd01_diskcnt);
				if (dsd01 == NULL)
					extmsg("cannot malloc");
}

		populate();
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;   /* Not interested in this drive.*/
			for (k = 0; k < CHUNKS; k++)	{
				drive_info[j].chunk_accessed[k] = 0;
				drive_info[j].seek_dist[k] = 0;
			}
			drive_info[j].total_accesses = 0;
			drive_info[j].sum_seeks = 0;

			drive_info[j].io_ops = dsd01[j].dk_stat.ios.io_ops;
			/*
			 * Calculate the index of the last-used entry (the
			 * entry immediately before the head of the queue)
			 * in the performance tracking queue.  The head
			 * of the queue can be at index 0 iff no access
			 * has been made to the disk since the last boot.
			 */
			index = dsd01[j].dk_stat.pttrack - dsd01[j].dk_stat.ptrackq - 1;	
			if (index < 0)	/* Fresh queue.	*/
				index = 0;
			drive_info[j].last_time.end_index = index;
			drive_info[j].last_time.b_blkno = dsd01[j].dk_stat.ptrackq[index].b_blkno;

		}
}


/*
 * populate()
 *
 * Get the disk performance structure from /dev/kmem
 * and adjust the pointers within that structure.
 */

void
populate()

{
	int delta,i;

	if (setup[L_SCSI].n_value != 0) {
		lseek(f,(long)setup[L_SCSI].n_value, 0);
		if (read(f, dsd01, sizeof(struct disk) * Sd01_diskcnt)
				!= sizeof(struct disk) * Sd01_diskcnt) {
			extmsg("cannot read SD01 data structure");
		}
	}


	/*
	 * The disk performance queue is managed by pttrack, which
	 * points to the next entry to be used.  After the pdi
	 * structure is read from /dev/kmem
	 * pttrack is garbage since it points back to a
	 * location in the kernel data space.  Another pointer,
	 * endptrack, has been provided to assist in calculating
	 * the new value of pttrack.  Endptrack points to one element
	 * past the last element of ARRAY ptrackq (NOT the logical
	 * end of the CICULAR QUEUE that is based on that array).  To
	 * update pttrack, compute the difference between the new and
	 * old endptrack and increase pttrack by this delta amount.
	 */

	if (ALL) {
		for (i=0; i < Sd01_diskcnt; i++) {
			if (dsd01[i].dk_state > 0)
				Sdrvlist[i] = 1;
		}
	}

	for (i=0; i < SNDRIVE; i++) {
		if ( (Sdrvlist[i] == 1) && ( i < Sd01_diskcnt) ) {
			if (dsd01[i].dk_state == 0) {
				fprintf(stderr,"sdsk-%d is not equipped\n",i);
				exit(4);
			}
		}
		else 
		{
		if ( Sdrvlist[i] == 1) {
			fprintf(stderr,"sdsk-%d is not equipped\n",i);
			exit(4);
		}
		}
	}

	for (i=0; i < Sd01_diskcnt; i++) {
		if (Sdrvlist[i] == 0)
			continue;
	delta =  ((long) &(dsd01[i].dk_stat.ptrackq[NTRACK]) - (long)dsd01[i].dk_stat.endptrack);
	dsd01[i].dk_stat.pttrack = (pdi_ptrk_t *)(((int)dsd01[i].dk_stat.pttrack) + delta);
	}
}


/*
 * extract_info()
 *
 * Sample the disk performance data and save it in the drive_info structure.
 */

void
extract_info()

{
	register long b_blkno;	/* Number of block that was accessed.	*/
	long cylinder;	/* Cylinder number calculated from b_blkno.	*/
	long prev_cyl;	/* The cylinder number of the previous entry;	*/
			/* used in calculating the seek distance.	*/
	int seek_dist;	/* Calculated seek distance.			*/
	int all_new_entries;	/* Are all queue entries new?		*/
	register int index;	/* Index into the ptrackq array		*/
	int front_index;	/* Index of the front of the queue	*/
	int i;

	for (i=0; i < Sd01_diskcnt; i++) {
		if (Sdrvlist[i] == 0)
			continue;

	front_index = dsd01[i].dk_stat.pttrack - dsd01[i].dk_stat.ptrackq;
	
	/* If the queue is still empty, skip this drive. */
	if (front_index == 0)
		continue;
	
	/*
	 * If the contents of the entry at the previous end of the
   	 * queue have been overwritten since our last probe, then
	 * the queue has completely wrapped around and all NTRACK
	 * entries are new.
	 */
	if (drive_info[i].last_time.b_blkno != dsd01[i].dk_stat.ptrackq[drive_info[i].last_time.end_index].b_blkno)
	{
		/*
		 * The contents of the previous end of the queue have changed.
		 * All entries are new.  Start at the present front of the
		 * queue and probe all NTRACK entries.
		 */
		index = front_index % NTRACK;
		all_new_entries = 1;
	}
	else	/* The contents of the last end of the queue are unchanged.  */
		if (front_index == (drive_info[i].last_time.end_index + 1))
			/*
			* The front of the queue is still at the same
			* index.  So, no new entries have been added.
			*/
			continue;	/* Skip this drive. */
		else	{
			/*
			 * Some entries have been added but not enough to over-
			 * write the whole queue. Start at the first new entry.
			 */
			index = drive_info[i].last_time.end_index;
			all_new_entries = 0;
		}
	/*
	 * Index now points to the last probed entry of last time.
	 * Find out the cylinder number of that entry so we can calculate
	 * the seek distance to first entry to be probed this time.
	 */
	b_blkno = dsd01[i].dk_stat.ptrackq[index].b_blkno;
	/*
	 * Di_sectors contains the number of sectors (blocks) per
	 * track and di_tracks has the number of tracks per cylinder.
	 */
	prev_cyl = (b_blkno / dsd01[i].dk_pdsec.sectors) / dsd01[i].dk_pdsec.tracks;

	/*
	 * Normally the front of the queue is an entry that is either
	 * already probed or is invalid.  But if all the entries are new,
	 * the front of the queue points to a valid entry that has not been
	 * seen.  So, increment the access count of the cylinder in the
	 * entry at the front of the queue even though we cannot calculate
	 * a seek distance to it since there is no valid previous entry for it.
	 */
	if (all_new_entries)
		drive_info[i].chunk_accessed[prev_cyl / CYLS_PER_CHUNK]++;
	
	while (++index != front_index)	{
		index = index % NTRACK;
		b_blkno = dsd01[i].dk_stat.ptrackq[index].b_blkno;
		cylinder = (b_blkno / dsd01[i].dk_pdsec.sectors) / dsd01[i].dk_pdsec.tracks;
		drive_info[i].chunk_accessed[cylinder / CYLS_PER_CHUNK]++;
		seek_dist = cylinder - prev_cyl;
		if (seek_dist < 0)
			seek_dist = -seek_dist;
		drive_info[i].seek_dist[(seek_dist + CYLS_PER_CHUNK - 1) / CYLS_PER_CHUNK]++;
		prev_cyl = cylinder;
	}
	drive_info[i].last_time.end_index = index - 1;
	drive_info[i].last_time.b_blkno = b_blkno;
	}
}


/*
 * print_results()
 *
 * Print the results of monitoring in histogram or tabular form.  The
 * report has two parts: cylinder profile and seeking distance profile.
 */

void
print_results()

{

	int uname();
	char *ctime();
	long time();
	
	long current_time;
	struct utsname name;
	int max_chunk;		/* The highest chunk number (or seek	*/					/* distance) for which data exists.	*/
	register struct drive_information *drive_ptr;
	register int j, k;   /* Loop counters for PEs, drives, and	*/
				/* chunks of cylinders, respectively.	*/

	current_time = time((long *) 0);
	printf("\n\n%s\n",ctime(&current_time));
	if (uname(&name) == -1)
		extmsg("Cannot get the name of the system from uname()");
	else
		printf("%s %s %s %s %s\n", name.sysname, name.nodename, name.release, name.version, name.machine);
	/*
	 * Sum up the number of accesses and seek distances for all
	 * chunks of each drive under investigation.
	 */
	for (j = 0; j < Sd01_diskcnt; j++)	{
		if (Sdrvlist[j] == 0)
			continue;   /* Not interested in this drive.*/
		for (k = 0; k < CHUNKS; k++)	{
			drive_info[j].total_accesses += drive_info[j].chunk_accessed[k];
			drive_info[j].sum_seeks += drive_info[j].seek_dist[k];
			}
	}
	if ((tflg == 0) && (hflg == 0))
		tflg = 1;
	if (tflg)	{
		printf("\nCYLINDER ACCESS PROFILE\n");
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;
			drive_ptr = &(drive_info[j]);
			if (drive_ptr->total_accesses != 0)	{
				printf("\n%s-%d:\n", 
				    devnm[1],j);
				printf("Cylinders\tTransfers\n");
				for (k = 0; k < CHUNKS; k++)
					if (drive_ptr->chunk_accessed[k] > 0)
						printf("%3d - %3d\t%ld\n", k * CYLS_PER_CHUNK, k * CYLS_PER_CHUNK + CYLS_PER_CHUNK - 1, drive_ptr->chunk_accessed[k]);
				printf("\nSampled I/O = %ld, Actual I/O = %ld\n",
					drive_ptr->total_accesses,drive_ptr->io_ops);
				if (drive_ptr->io_ops > 0)
					printf("Percentage of I/O sampled = %2.2f\n",
					((float)drive_ptr->total_accesses /(float)drive_ptr->io_ops) * 100.0);
				}
			}

		printf("\n\n\nSEEK DISTANCE PROFILE\n");
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;
			drive_ptr = &(drive_info[j]);
			if (drive_ptr->sum_seeks != 0)	{
				printf("\n%s-%d:\n", 
					devnm[1],j);
				printf("Seek Distance\tSeeks\n");
				for (k = 0; k < CHUNKS; k++)
					if (drive_ptr->seek_dist[k] > 0)	{
						if (k == 0)
							printf("        0\t%ld\n", drive_ptr->seek_dist[k]);
						else
							printf("%3d - %3d\t%ld\n", k * CYLS_PER_CHUNK - (CYLS_PER_CHUNK - 1), k * CYLS_PER_CHUNK, drive_ptr->seek_dist[k]);
					}
				printf("Total Seeks = %ld\n",drive_ptr->sum_seeks);
				}
			}
	}
	if (hflg)	{
		/* Print a histogram of the results.	*/
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;
			drive_ptr = &(drive_info[j]);
			if (drive_ptr->total_accesses != 0) 	{
				Scylhdr(CYLNO, drive_ptr->total_accesses, j);
				max_chunk = Scylhist(drive_ptr->total_accesses, drive_ptr->chunk_accessed);
				Scylftr(CYLNO, max_chunk);
			}
		}
		for (j = 0; j < Sd01_diskcnt; j++)	{
			if (Sdrvlist[j] == 0)
				continue;
			drive_ptr = &(drive_info[j]);
			if (drive_ptr->sum_seeks != 0)	{
				Scylhdr(SEEKD, drive_ptr->sum_seeks, j);
				max_chunk = Scylhist(drive_ptr->sum_seeks, drive_ptr->seek_dist);
				Scylftr(SEEKD, max_chunk);
			}
		}
	}
}


struct SHISTDATA	{
	long    hdata[CHUNKS];
};	/* Used by the histogram printing functions.	*/

/*      the following routines are obtained from iostat */
/* cylinder profiling histogram */
/*.s'cylhist'Output Cylinder Histogram'*/
Scylhist(at, dp)
long at;
register struct SHISTDATA *dp;

{
	register ii;
	int maxrow;
	long graph[CHUNKS];
	long    max, max2;
	long    data;
	long    scale;

	max = 0;
	for (ii = 0; ii < CHUNKS; ii++)	{
		if (data = dp->hdata[ii])	{
			maxrow = ii;
			if (data > max)	{
				max2 = max;
				max = data;
			} 
			else if (data > max2 && data != max)
				max2 = data;
		}
	}
	maxrow++;

	/* determine scaling */
	scale = 1;
	if ( max2 )	{
		scale = at / ( max2 * 2 );
		if ( scale > 48 )
			scale = 48;
		}

	for (ii = 0; ii < maxrow; ii++)	{
		if (dp->hdata[ii])
			graph[ii] = (scale * 100 * dp->hdata[ii]) / at;
		else
			graph[ii] = -1;
	}

	Sprthist(graph, maxrow, scale, (long) (max*100*scale/at));
	return (maxrow);
}
/*.s'prthist'Print Histogram'*/

Sprthist(array, mrow, scale, gmax)
	long array[], scale, gmax;
register mrow;

{
	long    line;

	line = 50;
	/* handle overflow in scaling */
	if (gmax > 51)	{
		line = 52;
		printf("\n%2ld%% -|", gmax/scale);
		pline(line--, array, mrow, BLOB);
		printf("\n     %c", BRK);
		pline(line--, array, mrow, BRK);
	} 
	else if ( gmax = 51 )
		line = 51;
	while ( line > 0)	{
		if ((line & 07) == 0)	{
			printf("\n%2ld%% -|", line/scale);
		} 
		else	{
			printf("\n     |");
		}
		pline(line--, array, mrow, BLOB);
	}
	printf("\n 0%% -+");
	line = -1;
	pline( line, array, mrow, FOOT);
}
/*.s'cylhdr'Print Cylinder Profiling Headers'*/

Scylhdr( flag, total, drive_num)
int flag;
long total;
int drive_num;

{
	if ( flag == CYLNO)
		printf("\nCYLINDER ACCESS HISTOGRAM\n");
	if (flag == SEEKD)
		printf("\nSEEK DISTANCE HISTOGRAM\n");
	printf("\n%s-%d:\n", 
	    devnm[1],drive_num);
	printf("Total %s = %ld\n", flag == CYLNO ? "transfers" : "seeks", total);
}
/*.s'cylftr'Print Histogram Footers'*/

Scylftr(flag, max_chunk)
register int flag, max_chunk;

{
	register int i;
	char number[5];

	if (flag == CYLNO)
		printf("\n      \t\t\tCylinder number, granularity=%d", CYLS_PER_CHUNK);
	else	{
		printf("\n      =<<  <");
		for (i = 200; i <= max_chunk * CYLS_PER_CHUNK; i += 100)
			printf("    <");
		}
	/*
	 * The following code works only for CYLS_PER_CHUNK = 20 and
	 * has to be slightly modified for other CYLS_PER_CHUNK sizes.
	 */
	printf("\n      024  1");
	for (i = 200; i <= max_chunk * CYLS_PER_CHUNK; i += 100)	{
		sprintf(number, "%d", i);
		printf("    %c", number[0]);
	}
	printf("\n       00  0");
	for (i = 200; i <= max_chunk * CYLS_PER_CHUNK; i += 100)	{
		sprintf(number, "%d", i);
		printf("    %c", number[1]);
	}
	printf("\n           0");
	for (i = 200; i <= max_chunk * CYLS_PER_CHUNK; i += 100)	{
		sprintf(number, "%d", i);
		printf("    %c", number[2]);
	}
	printf("\n            ");
	for (i = 200; i <= max_chunk * CYLS_PER_CHUNK; i += 100)	{
		sprintf(number, "%d", i);
		printf("    %c", (number[3] == '\0') ? ' ' : number[3]);
	}
	printf("\n");
}
