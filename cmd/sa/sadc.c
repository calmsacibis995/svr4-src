/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sadc.c	1.30.1.30"

/*
 *	sadc.c - writes system activity binary data from /dev/kmem to a
 *		file or stdout.
 *
 *	Usage: sadc [t n] [file]
 *		if t and n are not specified, it writes
 *		a dummy record to data file. This usage is
 *		particularly used at system booting.
 *		If t and n are specified, it writes system data n times to
 *		file every t seconds.
 *		In both cases, if file is not specified, it writes
 *		data to stdout.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/var.h>
#include <sys/iobuf.h>
#include <sys/stat.h>
#include <sys/elog.h>
#include <sys/vnode.h>
#include <sys/fs/s5inode.h>
#include <sys/file.h>

#ifdef u3b2
#include <sys/sbd.h>
#include <sys/immu.h>
#include <sys/id.h>
#endif

#include <sys/file.h>

#ifdef i386
#include <sys/buf.h>
#include <sys/vtoc.h>
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/bootinfo.h>

#include "sys/scsi.h"
#include "sys/sdi.h"
#include "sys/sd01.h" 
#endif

#include <sys/sysinfo.h>
#include <sys/fs/rf_acct.h>
#include <sys/proc.h>

#include <sys/fcntl.h>
#include <sys/flock.h>
#include <sys/tuneable.h>

#include "sa.h"

#ifdef u3b
#include <sys/mtc.h>
#include <sys/dma.h>
#include <sys/vtoc.h>
#include <sys/dfc.h>
#define ctob(x) ptob(x)
int 	dskcnt, mtccnt;
char 	*taploc, *dskloc;
int 	apstate;
#endif

#ifdef u3b2
int 	idcnt;
struct 	idstatstruct idstatus[IDNDRV];
struct 	iotime idstat[IDNDRV];
struct 	iotime ifstat;
#endif 

#ifdef i386
int 	idcnt = 0;
int 	sd01_diskcnt;
#define MAX_LUN	8
#define NDRV    2       /* maximum number of drives on 386 (2 per 546 board) */
#define NBRD    1       /* maximum number of 546 boards on 386 */
struct 	disk *dsd01;
struct 	iotime hdstat[NDRV];
struct 	bootinfo bootinfo;
int 	bdcnt; 		/* number of boards on this machine */
#endif 

#ifdef u3b5
#include <sys/dfdrv.h>
int 	dfcnt;
char 	*dfloc;
#endif 

#ifdef u370
#include <sys/times.h>
#include <sst.h>
time_t 	times();
#endif

struct 	stat ubuf,syb;
struct 	var tbl;
struct 	syserr err;

#if pdp11 || vax
struct 	iotime ia[NCTRA][NDRA];
struct 	iotime ib[NCTRB][NDRB];
struct 	iotime ic[NCTRC][NDRC];
struct 	iotime id[NCTRC][NDRD];
int 	rlcnt;
int 	gdcnt;
#endif

#ifdef i286
struct 	iotime ia[NCTL][NDPC];
int 	wncnt;
#endif

struct 	sa d;
struct 	flckinfo flckinfo;
struct 	tune tune;
char 	*flnm = "/tmp/sa.adrfl";
char 	*loc;
static 	int fa = 0;
unsigned tblloc;
char 	*malloc();

#ifndef u370
static 	int tblmap[SINFO];
#endif

#ifdef u370
static 	int tblmap[10];	/* A kludge for the 370 -not really used */
#endif

static 	int recsz,tmpsz;
extern	time_t time();
int 	i,j,k;
long 	lseek();
int 	f;
long 	ninode;


main(argc, argv)
char 	**argv;
int 	argc;
{

#ifdef u370
	long 	tmstmp;
	struct 	pxs *pxs;
	struct 	stv *stv;
	int 	sst;
	int 	n;
	DEC 	dp;
	struct 	tms buff;
#endif	

	int 	ct;
	unsigned ti;
	int 	fp;
	long 	min;
	long 	rem;
	mode_t 	u;
	struct 	stat buf;
	char 	*fname;
	char 	*ptr;

	ct = argc >= 3? atoi(argv[2]): 0;
	min = time((long *) 0);
	ti = argc >= 3? atoi(argv[1]): 0;

/*	
 *	check if the address file is there and not older than /stand/unix, 
 *	if so, read the offsets in.  Otherwise, search /stand/unix name list 
 *	to get the offsets and create an address file for later use.
 */
	stat ("/stand/unix",&syb);
	if ((stat(flnm,&ubuf) == -1) || (ubuf.st_mtime <= syb.st_ctime))
		goto creafl;
	fa = open(flnm,2);
	if (read(fa, setup, sizeof(setup)) != sizeof(setup)) {
		close(fa);
		unlink(flnm);
creafl:
		u = umask(0);
		fa = open(flnm, O_RDWR|O_CREAT|O_TRUNC, 0664);
		umask(u);
		chown(flnm, 0, getegid());

		/*    search name list to get offsets	*/
		if (nlist("/stand/unix", setup) == -1)
			perrexit( "sadc" );

#ifdef vax
		for (i=0; i<SERR+1; i++)
			setup[i].n_value &= ~(0x80000000);
#endif

#ifdef u370
		/*    open /dev/mem - get system table addresses	*/
		if ((f = open("/dev/mem", 0)) == -1)
			exit(2);
		for (i = INO; i < V; i++){
			lseek(f, setup[i].n_value, 0);
			read(f, &setup[i].n_value, 4);
		}
		close(f);
#endif

		/*    write offsets to address file	*/
		write(fa, setup, sizeof(setup));
		close (fa);
	}

#ifndef u370
	/*    open /dev/kmem	*/
	if ((f = open("/dev/kmem", 0)) == -1)
		perrexit();
#endif

#ifdef u370
	/*    open /dev/mem	*/
	if ((f = open("/dev/mem", 0)) == -1)
		exit(2);

	/*    open tss system statistics table	*/
	if ((sst = open("/dev/sst",0)) == -1)
		exit(2);
#endif

#if pdp11 || vax
	/*    get number of disk drives on rl, ra, and general disk drivers.	*/
	if (setup[RLCNT].n_value != 0) {
		lseek(f,(long)setup[RLCNT].n_value, 0);
		if (read(f,&rlcnt,sizeof rlcnt) == -1)
			perrexit();
	}

	if (setup[GDCNT].n_value != 0){
		lseek(f,(long)setup[GDCNT].n_value, 0);
		if (read(f,&gdcnt,sizeof gdcnt) == -1)
			perrexit();
	}
#endif

#ifdef u3b
	/*    get numbers of tape controller and disk drives for 3B	*/
	if (setup[MTCCNT].n_value !=0){
		lseek(f,(long)setup[MTCCNT].n_value, 0);
		if (read(f,&mtccnt,sizeof mtccnt) == -1)
			perrexit();
		taploc = malloc(sizeof (struct mtc)*mtccnt*4);
		if (taploc == NULL)
			perrexit();
	}

	if (setup[DSKCNT].n_value != 0) {
		lseek(f, (long)setup[DSKCNT].n_value, 0);
		if (read(f, &dskcnt, sizeof(dskcnt)) == -1)
			perrexit();
		dskloc = malloc(sizeof (struct dskinfo)*dskcnt);
		if (dskloc == NULL)
			perrexit();
	}
#endif

#ifdef u3b5
	/*    get numbers of disk drives for 3B5	*/
	if (setup[DFCNT].n_value !=0){
		lseek(f, (long)setup[DFCNT].n_value, 0);
		if (read(f, &dfcnt, sizeof(dfcnt)) == -1)
			perrexit();
		dfloc = malloc(sizeof (struct dfc)*dfcnt);
		if (dfloc == NULL)
			perrexit();
	}
#endif

#ifdef u3b2
	/*    get number of hard disk drives for 3B2	*/
	if (setup[IDEQ].n_value !=0){
		lseek(f,(long)setup[IDEQ].n_value, 0);
		if (read(f, idstatus,sizeof idstatus) != sizeof idstatus)
			perrexit();
		for (i = 0; i < DNDRV; i++)
			if (idstatus[i].equipped)
				idcnt++;
	  }
#endif 

#ifdef i386
	/*    get number of hard disk and scsi drives for 386	*/
	if (setup[BOOTINFO].n_value != 0) {
		lseek(f,(long)setup[BOOTINFO].n_value, 0);
		if (read(f, &bootinfo, sizeof(bootinfo)) != sizeof(bootinfo))
			perrexit();
		for (i=0; i<NDRV; i++)
			if (bootinfo.hdparams[i].hdp_ncyl)
				idcnt++;
	}	

	if (setup[SD01].n_value != 0) {
		lseek(f, (long)setup[SD01].n_value, 0);
		if (read(f, &sd01_diskcnt, sizeof(sd01_diskcnt)) != sizeof(sd01_diskcnt))
			sd01_diskcnt = 0;
		else {
                	dsd01 = (struct disk *) malloc(sizeof (struct disk) * sd01_diskcnt);
			if (dsd01 == NULL)
                                perrexit("sadc can't malloc");
		}
	}
#endif 

#ifndef u370
	/*    construct tblmap and compute record size	*/
	for (i=0; i<SINFO; i++) {

#if pdp11 || vax
		if (setup[i].n_value != 0) {
			if (i == GDS)
				tblmap[i] = gdcnt * 8;
			else {
				if (i == RLS)
					tblmap[i] = rlcnt;
				else
					tblmap[i] =iotbsz[i];
			}
#endif

#ifdef i286
		if (setup[i].n_value != 0L) {
			if (i == WNS)
				tblmap[i] = wncnt;
			recsz += tblmap[i];
		}
#endif

#ifdef u3b
		if (setup[i].n_value != 0) {
			if (i == MTC)
				tblmap[i] = mtccnt * 4;
			else {
				if (i == DSKINFO)
					tblmap[i] = dskcnt;
				else
					tblmap[i] = iotbsz[i];
			}
#endif

#ifdef u3b5
		if(setup[i].n_value != 0) {
			if(i == DFDFC)
				tblmap[i] = dfcnt * NDRV;
			else
				tblmap[i] = iotbsz[i];
#endif 

#ifdef u3b2
		if(setup[i].n_value != 0) {
			if(i == ID)
				tblmap[i] = idcnt;
			else
				tblmap[i] = iotbsz[i];
#endif 

#ifdef i386
		if(setup[i].n_value != 0) {
			if(i == ID) {
				tblmap[i] = idcnt;
			}
                        else if (i == SD01) {
                                tblmap[i] = sd01_diskcnt;
			}
                        else
                            	tblmap[i] = iotbsz[i];
#endif 

#ifndef i286
			recsz += tblmap[i];
		}
		else
			tblmap[i] = 0;
#endif
	}
	recsz = sizeof (struct sa) - sizeof d.devio 
 		+ recsz * sizeof(d.devio[0]);
#endif

#ifdef u370
	/*    assign record size	*/
	recsz = sizeof(struct sa);
#endif

	if (argc == 3 || argc == 1) {
		/*    no data file is specified, direct data to stdout	*/
		fp = 1;
		/*    write header record	*/
		write(fp, tblmap, sizeof(tblmap));
	}
	else {
		fname = argc ==2 ? argv[1]: argv[3];
		/*    
		 *    check if the data file is there	
		 *    check if data file is too old	
		 */
		if ((stat(fname,&buf) == -1) || ((min - buf.st_mtime) > 86400))
			goto credfl;
		if ((fp = open(fname,2)) == -1) {
credfl:
			/*    data file does not exist: create one and write 
			 *    the header record.	
			 */
			if ((fp = creat(fname,00644)) == -1)
				perrexit();
			close(fp);
			fp = open(fname,2);
			lseek(fp, 0L, 0);
			write (fp, tblmap, sizeof(tblmap));
		}
		else {
			/*    data file exist: position the write pointer to 
			 *    the last good record.  
			 */
			lseek(fp,-(long)((buf.st_size - sizeof tblmap) % recsz),2);
		}
	}

	/*    if n=0, write the additional dummy record	*/
	if (ct == 0) {
		for (i=0; i<4; i++)
			d.si.cpu[i] = -300;
		d.ts = min;
		write(fp, &d, recsz);
	}

	/*    get memory for tables	*/
	if (lseek(f, (long)setup[NINODE].n_value, 0) != -1)
		if (read(f, &ninode, sizeof(ninode)) == -1)
			perrexit();
	if (lseek(f, (long)setup[V].n_value, 0) == -1)
		perrexit();
	if (read(f, &tbl, sizeof(tbl)) == -1)
		perrexit();
	if (ninode)
		if (tblloc < sizeof(struct inode)*ninode)
			tblloc = sizeof(struct inode)*ninode;

	if (tblloc < sizeof(struct file))
		tblloc = sizeof (struct file);

	/*    Proc table is a pointer now */
	if (tblloc < sizeof (int)*tbl.v_proc)
		tblloc = sizeof (int)*tbl.v_proc;

	loc = malloc(tblloc);
	if (loc == NULL){
		perrexit();
	}

#ifdef u370
	pxs = (struct pxs *)malloc(sizeof(*pxs));
	stv = (struct stv *)malloc(sizeof(*stv));
#endif

	for(;;) {
		/*    read data from /dev/kmem to structure d	*/
		lseek(f, (long)setup[SINFO].n_value, 0);
		if (read(f, &d.si, sizeof(d.si)) == -1)
			perrexit();

#ifdef i286
		d.si.bswapin = ctod(d.si.bswapin);
		d.si.bswapout = ctod(d.si.bswapout);
		for (i=WNS; i<SINFO; i++) {
			if (setup[i].n_value != 0L){
				lseek(f, (long)setup[i].n_value, 0);
				if (i == WNS) {
				if (read(f, ia[i], sizeof(ia[i])) == -1)
					perrexit();
				}
			}
		}
#endif

#ifndef i286
		/*    Distributed Unix info	*/
		if (setup[RF_SRV].n_value != 0) {
			lseek(f, (long)setup[RF_SRV].n_value, 0);
			if (read(f, &d.rf_srv, sizeof(d.rf_srv)) == -1)
				perrexit();
		}

		if (setup[RFSR].n_value != 0)  {
			lseek(f, (long)setup[RFSR].n_value, 0);
			if (read(f, &d.rfs_in, sizeof(d.rfs_in)) == -1)
				perrexit();
		}

		if (setup[RFCL].n_value != 0)  {
			lseek(f, (long)setup[RFCL].n_value, 0);
			if (read(f, &d.rfs_out, sizeof(d.rfs_out)) == -1)
				perrexit("sadc");
		}

		if (setup[MINSERVE].n_value != 0) {
			lseek(f, (long)setup[MINSERVE].n_value, 0);
			if (read(f, &d.minserve, sizeof(d.minserve)) == -1)
				perrexit();
		}

		if (setup[MAXSERVE].n_value != 0) {
			lseek(f, (long)setup[MAXSERVE].n_value, 0);
			if (read(f, &d.maxserve, sizeof(d.maxserve)) == -1)
				perrexit();
		}
#endif

		/*    Client Caching info	*/
		if (setup[RFC_INFO].n_value != 0) {
			lseek(f, (long)setup[RFC_INFO].n_value, 0);
			if (read(f, &d.rfc, sizeof(d.rfc)) == -1)
				perrexit();
		}

		/*    Kernel Memory Allocation info  */
		if (setup[KMEMINFO].n_value != 0) {
			lseek(f, (long)setup[KMEMINFO].n_value, 0);
			if (read(f, &d.km, sizeof(d.km)) == -1)
				perrexit();
		}

#if     vax || u3b || u3b5 || u3b2 || i386
		/*    Virtual memory	*/
		if (setup[MINFO].n_value != 0)	{
			lseek(f, (long)setup[MINFO].n_value, 0);
			if (read(f, &d.mi, sizeof(d.mi)) == -1)
				perrexit();
		}

		if (setup[VMINFO].n_value != 0)	{
			lseek(f, (long)setup[VMINFO].n_value, 0);
			if (read(f, &d.vmi, sizeof(d.vmi)) == -1)
				perrexit("sadc");
		}
#endif

#if pdp11 || vax 
		d.si.bswapin = ctod(d.si.bswapin);
		d.si.bswapout = ctod(d.si.bswapout);
		for (i=HPS; i<SINFO; i++)
			if (setup[i].n_value != 0) {
				lseek(f,(long)setup[i].n_value,0);
				if (i < RLS) {
					if (read(f, ia[i], sizeof(ia[i])) == -1)
						perrexit();
				}
				else {
					if (i == TSS) {
						if (read(f, id[i-TSS], sizeof(id[i-TSS])) == -1)
							perrexit();
					}
				}
				else {
					if (i == GDS) {
						if (read(f, ic[i-GDS],(sizeof (struct iotime)*gdcnt*8)) == -1)
							perrexit();
					}
				}
				else
					if (read(f,ib[i-RLS],sizeof ib[i-RLS]) == -1)
					perrexit();
				}
			}
#endif 

#ifdef u3b
		d.si.bswapin=ctob(d.si.bswapin)/NBPSCTR; /* BSIZE to NBPSCTR */
		d.si.bswapout=ctob(d.si.bswapout)/NBPSCTR; /* BSIZE to NBPSCTR */
		tapetbl(taploc);
		dsktbl(dskloc);
#endif 

#ifdef u3b5
		dftbl(dfloc);
#endif 

#if u3b2 || i386
		/*    translate from clicks to disk blocks */
		d.si.bswapin=ctob(d.si.bswapin)/NBPSCTR; /* BSIZE to NBPSCTR */
		d.si.bswapout=ctob(d.si.bswapout)/NBPSCTR; /* BSIZE to NBPSCTR */

#ifdef i386
		if (setup[ID].n_value != 0 ) {
			lseek(f, (long) setup[ID].n_value, 0 );
			if (read(f, hdstat, sizeof(struct iotime) * idcnt) == -1) {
				perrexit();
			}
		}

                if ((setup[SD01_D].n_value != 0) && (sd01_diskcnt > 0)) {
                        lseek(f, (long)setup[SD01_D].n_value, 0);
			if (read(f, &ptr, 4) != 4) {
				perror("sadc can't read SD01_D");
				exit(2);
			}
			lseek(f, (long)ptr, 0);
                        if (read(f, dsd01, sizeof(struct disk)*sd01_diskcnt)
                       	    != sizeof(struct disk) * sd01_diskcnt) {
                                perror("sadc can't read SD01_D structure");
				exit(2);
                        }
		}
#else
		if ( setup[ID].n_value != 0 ) {
			lseek(f, (long)setup[ID].n_value, 0);
			if (read(f, idstat, sizeof(struct iotime) * idcnt) == -1) {
				perrexit();
			}
		}

		if (setup[IF].n_value != 0 ) {
			lseek(f, (long)setup[IF].n_value, 0);
			if (read(f, &ifstat, sizeof(struct iotime)) == -1) {
				perrexit();
			}
		}
#endif 
#endif 

#ifdef u370
		if (read(sst, pxs, sizeof(*pxs)) == -1)
			exit(2);
		lseek(sst, 4096L, 0);
		if (read(sst, stv, sizeof(*stv)) == -1)
			exit(2);
		lseek(sst, 0, 0);

		/*    Get performance statistics from tss sst tables.
		 *    Get cpu usage info	
		 */
		d.nap = pxs->nap;
		d.vmtm = stv->vmb;
		d.usrtm = stv->nvmb;
		d.usuptm = d.vmtm - d.usrtm;
		d.idletm = stv->wtm1 + stv->wtm2;
		d.ccv = stv->ccv;

		/*    Get scheduling info	*/
		dp = stv->tsend.alt;
		n = cvb(dp);
		d.tsend = n;
		d.intsched = stv->sched.kie;
		d.mkdisp = stv->sched.kid;

		/*    Get drum and disk info	*/
		for (i = 0; i < NDEV; i++) {
			n = cvb(stv->dd[i].ddrs);
			d.io[i].io_sread = n;
			n = cvb(stv->dd[i].ddrp);
			d.io[i].io_pread = n;
			n = cvb(stv->dd[i].ddws);
			d.io[i].io_swrite = n;
			n = cvb(stv->dd[i].ddwp);
			d.io[i].io_pwrite = n;
			d.io[i].io_total = d.io[i].io_sread +
					d.io[i].io_pread +
					d.io[i].io_swrite +
					d.io[i].io_pwrite;
		}
		d.elpstm = times(&buff);
		d.curtm = time((long *) 0);
#endif

		/*    compute size of system table	*/
		if (ninode)
			d.szinode = inodetbl((struct inode*)loc);
		else
			d.szinode = 0;

		if (setup[FILCT].n_value != 0) {
			lseek(f, (long) setup[FILCT].n_value, 0);
			if (read(f, &d.szfile, sizeof(d.szfile)) == -1) 
				perrexit();
		}

#ifndef u370
		if (setup[PRO].n_value != 0) {
			lseek(f, (long) setup[PRO].n_value, 0);
			if (read(f, &d.szproc, sizeof(d.szproc)) == -1) 
				perrexit();
		}
#endif

#ifdef u370
		proctbl(loc);
		d.szproc = d.pi.sz;
#endif

#if vax || u3b || u3b5 || u3b2 || i386
		if ( setup[FLCK].n_value != 0 ) {
			lseek( f, (long) setup[FLCK].n_value, 0 );
			if (read(f, &flckinfo, sizeof(struct flckinfo)) == -1) 
				perrexit();
		}

		/*    record system table sizes */
		d.szlckr = flckinfo.reccnt;

		if (setup[TUNE].n_value != 0 ) {
			lseek( f, (long) setup[TUNE].n_value, 0 );
			if (read(f, &tune, sizeof(struct tune)) == -1) 
				perrexit();
		}

		/*    record maximum sizes of system tables */
		d.mszlckr = tune.t_flckrec;
#endif

		/*    record maximum sizes of system tables	*/
		d.mszinode = ninode;
		d.mszfile = 0;
		d.mszproc = tbl.v_proc;

		/*    record system tables overflows	*/
		lseek(f, (long)setup[SERR].n_value, 0);
		if (read(f, &err, sizeof(struct syserr)) == -1)
			perrexit();
		d.inodeovf = err.inodeovf;
		d.procovf = err.procovf;
		d.fileovf = 0;

		/*    get time stamp	*/
		d.ts = time ((long *)0);

#ifdef u3b
		/*    Ascertain whether the AP is online or not */
		if (setup[APSTATE].n_value != 0) {
			lseek(f, (long)setup[APSTATE].n_value, 0);
			if (read(f, &apstate,sizeof apstate) == -1)
				perrexit();
			d.apstate = apstate;
		}
#endif

		i=0;

#ifdef i286
		for (j=0; j<SINFO; j++) {
			if (setup[j].n_value != 0) {
				for (k=0; k<tblmap[j];k ++) {
					d.devio[i][0] = ia[j][k].io_cnt;
					d.devio[i][1] = ctod(ia[j][k].io_bcnt);
					d.devio[i][2] = ia[j][k].io_act;
					d.devio[i][3] = ia[j][k].io_resp;
					++i;
				}
			}
		}
#else

#ifndef u370
		for (j=0;j <SINFO; j++) {
#if pdp11 || vax
			if (setup[j].n_value != 0) {
				for (k=0; k<tblmap[j]; k++) {
					if (j == TSS) {
						d.devio[i][0] = id[j-TSS][k].io_cnt;
						d.devio[i][1] = ctod(id[j-TSS][k].io_bcnt);
						d.devio[i][2] = id[j-TSS][k].io_act;
						d.devio[i][3] = id[j-TSS][k].io_resp;
					}
					else {
						if (j == GDS) {
							d.devio[i][0] = ic[j-GDS][k].io_cnt;
							d.devio[i][1] = ctod(ic[j-GDS][k].io_bcnt);
							d.devio[i][2] = ic[j-GDS][k].io_act;
							d.devio[i][3] = ic[j-GDS][k].io_resp;
						}
					}
					else {
						if (j >= RLS && j < GDS) {
							d.devio[i][0] = ib[j-RLS][k].io_cnt;
							d.devio[i][1] = ctod(ib[j-RLS][k].io_bcnt);
							d.devio[i][2] = ib[j-RLS][k].io_act;
							d.devio[i][3] = ib[j-RLS][k].io_resp;
						}
					}
					else {
						d.devio[i][0] = ia[j][k].io_cnt;
						d.devio[i][1] = ctod(ia[j][k].io_bcnt);
						d.devio[i][2] = ia[j][k].io_act;
						d.devio[i][3] = ia[j][k].io_resp;
					}
					i++;
				}
			}
#endif 

#ifdef u3b
			if (setup[j].n_value != 0)
				if (j == MTC)
					tapemv(taploc);
				else
					dskmv(dskloc);
#endif 

#ifdef u3b5
			dfmv(dfloc);
#endif 

#ifdef u3b2
			for (j=0; j<SINFO; j++) {
				if (setup[j].n_value != 0) {
					for (k=0; k<tblmap[j]; k++) {
						if (j == ID) {
							d.devio[i][0] = idstat[k].io_cnt;
							d.devio[i][1] = idstat[k].io_bcnt;
							d.devio[i][2] = idstat[k].io_act;
							d.devio[i][3] = idstat[k].io_resp;
						} else if (j == IF) {
							d.devio[i][0] = ifstat.io_cnt;
							d.devio[i][1] = ifstat.io_bcnt;
							d.devio[i][2] = ifstat.io_act;
							d.devio[i][3] = ifstat.io_resp;
						}
						i++;
					}
				}
			}
#endif 

		}

#ifdef i386
		if (setup[BOOTINFO].n_value != 0) {
			for (k=0; k < idcnt; k++) {
				d.devio[i][0] = hdstat[k].io_cnt;
				d.devio[i][1] = hdstat[k].io_bcnt;
				d.devio[i][2] = hdstat[k].io_act;
				d.devio[i][3] = hdstat[k].io_resp;
				i++;
			}
		}

        	if (setup[SD01].n_value != 0) {
			register struct disk *dp;
                 	for (dp=dsd01, k=0; k < sd01_diskcnt; k++) {
                        	if (dp->dk_state & (DKINIT|DKFDISK)) {
                                	d.devio[i][0] = dp->dk_stat.tnrreq +
							dp->dk_stat.tnwreq;
                                	d.devio[i][1] = dp->dk_stat.io_bcnt;
                                	d.devio[i][2] = dp->dk_stat.io_act;
                                	d.devio[i][3] = dp->dk_stat.io_resp;
                                	d.devio[i][4] = k;
                                	i++;
                        	}
				dp++;
                	}
		}

#endif
#endif
#endif

		/*    write data to data file from structure d	*/
		write(fp, &d, recsz);
		if(--ct > 0)
			sleep(ti);
		else {
			close(fp);
			exit(0);
		}
	}
}


inodetbl(x)
register struct inode *x;
{
	unsigned long	addr;
	register i, n = 0;

	if ((lseek(f, (long)setup[INO].n_value, 0) == -1) ||
	    (read(f, &addr, sizeof(long)) != sizeof(long))) {
		perror("sadc cannot read address of inode table");
		exit(2);
	}

	if ((lseek(f, addr, 0) == -1) || 
	    (read(f, x, sizeof(struct inode) * ninode) != 
            sizeof(struct inode) * ninode)) {
		perror("sadc cannot copy inode table");
		exit(2);
	}

 	for (i=n=0; i<ninode; i++, x++)
		if (x->i_vnode.v_count != 0)
			n++;

	return(n);
}


#ifdef u370
proctbl(x)
register struct proc *x;
{
	register i;
	d.pi.sz = 0; 	/*	reinitialize procinfo structure	*/
	d.pi.run = 0;
	d.pi.wtsem = 0;
	d.pi.wtio = 0;
	d.pi.wtsemtm = 0;
	d.pi.wtiotm = 0;

	lseek(f, (long)setup[PRO].n_value, 0);
	read(f, x, tbl.v_proc*sizeof(struct proc));
	for (i=0; i < tbl.v_proc; i++, x++) {
		if (x->p_stat == NULL)
			continue;
		d.pi.sz++;
		if (x->p_stat == SRUN) {
			d.pi.run++;
			continue;
		}
		else
			if (x->p_stat == SSLEEP && x->p_ppid !=1)
				if (x->p_wchan == 0) {
					d.pi.wtio++;
					d.pi.wtiotm += d.curtm - x->p_start;
				}
				else {
					d.pi.wtsem++;
					d.pi.wtsemtm += d.curtm - x->p_start;
				}
	}

	/*    The following prevents a 0 divide in sar -q	*/
	if (d.pi.wtio == 0)
		d.pi.wtio=1;
	if (d.pi.wtsem == 0)
		d.pi.wtsem=1;
}
#endif


#ifdef u3b
tapetbl(x)
register struct mtc *x;
{
	lseek(f, (long)setup[MTC].n_value, 0);
	read(f, x, mtccnt*sizeof(struct mtc)*4);
}


dsktbl(x)
register struct dskinfo *x;
{
	register n;

	lseek(f, (long)setup[DSKINFO].n_value, 0);
	read(f, x, dskcnt*sizeof(struct dskinfo));
}


long tapemv(x)
register struct mtc *x;
{
	register p;

	for(p=0; p<mtccnt*4; p++, x++, i++) {
		d.devio[i][0] = x->iotime.ios.io_ops;
		d.devio[i][1] = x->iotime.io_bcnt/NBPSCTR; 
	}
}


long dskmv(x)
register struct dskinfo *x;
{
	register p;

	for(p=0; p<dskcnt; p++, x++, i++) {
		d.devio[i][0] = x->devinfo.io_cnt;
		d.devio[i][1] = x->devinfo.io_bcnt;
		d.devio[i][2] = x->devinfo.io_act;
		d.devio[i][3] = x->devinfo.io_resp;
	}
}
#endif 


#ifdef u3b5
dftbl(x)
register struct dfc *x;
{
	lseek(f, (long)setup[DFDFC].n_value, 0);
	read(f, x, dfcnt*sizeof(struct dfc));
}


dfmv(x)
register struct dfc *x;
{
	register p;
	int 	q;

	for (q=0; q<dfcnt; q++) {
		for (p=0; p<NDRV; p++, i++) {
			d.devio[i][0] = x->df_stat[p].io_cnt;
			d.devio[i][1] = x->df_stat[p].io_bcnt;
			/* 
			 *    job response time is in millisecond units
			 *    convert drive active time from milliseconds to HZ
			 */
			/* d.devio[i][2] = (x->df_stat[p].io_act * HZ) / 1000; */
			d.devio[i][2] = x->df_stat[p].io_act;
			d.devio[i][3] = x->df_stat[p].io_resp;
		}
		x++;
	}
	return;
}
#endif 


perrexit()
{
	perror("sadc");
	exit(2);
}
