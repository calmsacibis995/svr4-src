/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sysdef-3b2:sysdef.c	1.37.3.1"

	/*
	** This command can now print the value of data items
	** from (1) the file (/unix is default), and (2) from
	** /dev/kmem.  If the read is from /dev/kmem, we can
	** also print the value of BSS symbols, (e.g., nadvertise).
	** The logic to support this is: if read is from file,
	** (1) find the section number of .bss, (2) look through
	** nlist for symbols that are in .bss section and zero
	** the n_value field.  At print time, if the n_value field
	** is non-zero, print the info.
	**
	** This protects us from trying to read a bss symbol from
	** the file and, possibly, droping core.
	**
	** When reading from /dev/kmem, the n_value field is the
	** seek address, and the contents are read from that address.
	**
	** NOTE: when reading from /dev/kmem, the actual, incore
	** values will be printed, for example: the current nodename
	** will be printed, and as mentioned above, nadvertise (number
	** of current advertised resources), etc.
	**
	** the cmn line usage is: sysdef -i -n namelist -m master.d directory
	** (-i for incore.)
	*/
#include	<stdio.h>
#include	<nlist.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>

#ifdef i386
#include	<sys/sysi86.h>
#else
#include	<sys/sys3b.h>
#endif

#include	<sys/var.h>
#include	<sys/tuneable.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<sys/fcntl.h>
#include	<sys/utsname.h>
#include	<sys/resource.h>
#include	<sys/conf.h>
#include	<sys/stat.h>
#include	<sys/signal.h>
#include	<sys/evecb.h>
#include	<sys/hrtcntl.h>
#include	<sys/priocntl.h>
#include	<sys/procset.h>
#include	<sys/events.h>
#include	<dirent.h>
#include	<ctype.h>

#include	<libelf.h>

#ifdef i386
#include	<sys/elf_386.h>
#else
#include	<sys/elf_M32.h>
#endif

#ifdef u3b15
#include	<sys/mmu.h>
#define	ONLBE(d)	((d)&0x30)
#endif

#ifdef u3b2
#include	<sys/param.h>
#define ONLBE(d)	(0)
#define KVIOBASE	0x60000
#endif

#ifdef i386
#include	<sys/param.h>
#define ONLBE(d)	(0)
#define KVIOBASE	0x60000
#endif

extern char *ctime();
extern char *strcat();
extern char *strcpy();
extern char *strncpy();
extern char *strncat();
extern char *malloc();

extern char *optarg;
extern int optind;

#define	SYM_VALUE(sym)	(nl[(sym)].n_value)
#define MEMSEEK(sym)	memseek(sym)
#define MEMREAD(var)	fread((char*)&var, sizeof(var), 1, \
				(incore ? memfile : sysfile))


#ifndef i386
struct	s3bboot	bootname;
#endif

struct	var	v;
struct  tune	tune;
struct	msginfo	minfo;
struct	seminfo	sinfo;
struct	shminfo	shinfo;
struct evcinfo evinfo;

int incore = 0;		/* 0 == read values from /dev/kmem, 1 == from file */
int bss;		/* if read from file, don't read bss symbols */
char	*os ="/stand/unix";
char	*mr = "/etc/master.d";
char	*mem = "/dev/kmem";
char 	*rlimitnames[] = {
	"cpu time",
	"file size",
	"heap size",
	"stack size",
	"core file size",
	"file descriptors",
	"mapped memory"
};

char	line[256], flag[8], pre[8], pre_addr[20], rtn[20];
#ifdef  u3b15
struct	mmuseg	addr;
#endif
#ifdef  u3b2
long	addr;
#endif

#ifdef  i386
long	addr;
#endif

int	nsp;
long	strthresh;
int	nstrpush, strmsgsz, strctlsz;
int	nadvertise, nrcvd, nrduser, nsndd, minserve, maxserve, maxgdp,
	rfsize, rfs_vhigh, rfs_vlow, nsrmount;
int	nremote, nlocal, rcache_time;
int	hrtimes_size, itimes_size,
	aio_size, min_aio_servers, max_aio_servers, aio_server_timeout;
short	naioproc, ts_maxupri;
char 	sys_name[10], intcls[10];
dev_t	root, dump;
char	MAJ[256];
daddr_t	spl;
int	nlsize, lnsize;
FILE	*sysfile, *mastf, *memfile;
DIR	*mast;

void	memseek(), getnlist(), setln();

struct nlist	*nl, *nlptr;

#ifdef i386
int rootdev, dumpdev, MAJOR, 
#else
int rootdev, dumpdev, MAJOR, sys3bboot,
#endif

#ifdef i386
	com2cons,
	do387cr3,
	do386b1,
	maxminor,
	nclass,
	ninode,
	nstrphash,
	piomaxsz,
	putbufsz,
#endif

	vs, tu, msginfo, seminfo,
	shminfo, FLckinfo, utsnm, bdev, evcinfo,
	pnstrpush, pstrthresh, pstrmsgsz, pstrctlsz, pmaxgdp,
	pnadvertise, pnrcvd, pnrduser, pnsndd, pminserve,
	pmaxserve, prfsize, prfs_vhigh, prfs_vlow, pnsrmount,
	pnremote, pnlocal, prcache_time, endnm,
	phrtimes_size, pitimes_size, pnaiosys, pminaios, 
	pmaxaios, paiotimeout, pnaioproc, pts_maxupri, 
	psys_name, pinitclass, prlimits;

#define ADDR	0	/* index for _addr array */
#define OPEN	1	/* index for open routine */
#define CLOSE	2	/* index for close routine */
#define READ	3	/* index for read routine */
#define WRITE	4	/* index for write routine */
#define IOCTL	5	/* index for ioctl routine */
#define STRAT	6	/* index for strategy routine */
#define MMAP	7	/* index for mmap routine */
#define SEGMAP	8	/* index for segmap routine */
#define POLL	9	/* index for poll routine */
#define SIZE	10	/* index for size routine */

#define EQ(x,y)		(strcmp(x, y)==0)  

#define	MAXI	300
#define	MAXL	MAXI/11+10
#define EXPAND	99

struct	link {
	char	*l_cfnm;	/* config name from master table */
	int l_funcidx;		/* index into name list structure */
	unsigned int l_soft :1;	/* software driver flag from master table */
	unsigned int l_dtype:1;	/* set if block device */
	unsigned int l_used :1;	/* set when device entry is printed */
} *ln, *lnptr, *majsrch();

	/* ELF Items */
Elf *elfd = NULL;
Elf32_Ehdr *ehdr = NULL;

main(argc, argv)
	int	argc;
	char	**argv;
{

	struct stat mfbuf;
	char mfname[MAXPATHLEN];
	struct	utsname utsname;
	struct	dirent *dp;
	struct rlimit rlimit;
	Elf_Scn *scn;
	Elf32_Shdr *shdr;
	char *name;
	int ndx;
	int i;

	while ((i = getopt(argc, argv, "im:n:?")) != EOF) {
		switch (i) {
		case 'i':
			incore++;
			break;
		case 'm':
			mr = optarg;
			break;
		case 'n':
			os = optarg;
			break;
		default:
			fprintf(stderr,

#ifdef i386
				"usage: %s [-i -n namelist]\n",
					argv[0]);
#else
				"usage: %s [-i -n namelist -m master.d_dir]\n",
					argv[0]);
#endif

			exit (1);
		}
	}

	if((sysfile = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot open %s\n",os);
		exit(1);
	}

	if (incore) {
		if((memfile = fopen(mem,"r")) == NULL) {
			fprintf(stderr,"cannot open %s\n",mem);
			exit(1);
		}
	}

	/*
	**	Use libelf to read both COFF and ELF namelists
	*/

        if ((elf_version(EV_CURRENT)) == EV_NONE) {
                fprintf(stderr, "ELF Access Library out of date\n");
		exit (1);
        }

        if ((elfd = elf_begin (fileno(sysfile), ELF_C_READ, NULL)) == NULL) {
                fprintf(stderr, "Unable to elf begin %s (%s)\n",
			os, elf_errmsg(-1));
		exit (1);
        }

	if ((ehdr = elf32_getehdr(elfd)) == NULL) {
		fprintf(stderr, "%s: Can't read Exec header (%s)\n",
			os, elf_errmsg(-1));
		exit (1);
	}

	if ( (((elf_kind(elfd)) != ELF_K_ELF) &&
			((elf_kind(elfd)) != ELF_K_COFF))
			|| (ehdr->e_type != ET_EXEC) )
	{
		fprintf(stderr, "%s: invalid file\n", os);
		elf_end(elfd);
		exit (1);
	}

	/*
	**	If this is a file read, look for .bss section
	*/

	if (!incore) {
		ndx = 1;
		scn = NULL;
		while ((scn = elf_nextscn(elfd, scn)) != NULL) {
			if ((shdr = elf32_getshdr(scn)) == NULL) {
				fprintf(stderr, "%s: Error reading Shdr (%s)\n",
					os, elf_errmsg(-1));
				exit (1);
			}
			name =
			elf_strptr(elfd, ehdr->e_shstrndx, (size_t)shdr->sh_name);
			if ((name) && ((strcmp(name, ".bss")) == 0)) {
				bss = ndx;
			}
			ndx++;
		}
	} /* (!incore) */

#ifndef i386
	stat(mr,&mfbuf);
	if ((mfbuf.st_mode & S_IFMT) != S_IFDIR)
	{
		fprintf(stderr," %s not a directory\n",mr);
		exit(1);
	}
	if((mast = opendir(mr)) == NULL) {
		fprintf(stderr, "cannot open %s\n", mr);
		exit(1);
	}
#endif

	uname(&utsname);
	printf("*\n* %s Configuration\n*\n",utsname.machine);

	nlsize = MAXI;
	lnsize = MAXL;
	nl=(struct nlist *)(calloc(nlsize, sizeof(struct nlist)));
	ln=(struct link *)(calloc(lnsize, sizeof(struct link)));
	nlptr = nl;
	lnptr = ln;

#ifndef i386
	bdev = setup("bdevsw");
	rootdev = setup("rootdev");
	dumpdev = setup("dumpdev");
	MAJOR = setup("MAJOR");
	sys3bboot = setup("sys3bboot");

	while (dp = readdir(mast)) {
		if (EQ(dp->d_name,".") ||  EQ(dp->d_name,".."))
			continue;
		strcpy(mfname,mr);
		strcat(mfname,"/");
		strncat(mfname, dp->d_name, strlen(dp->d_name));
		stat(mfname,&mfbuf);
		if ((mfbuf.st_mode & S_IFMT) != S_IFREG)
			continue;
		if ((mastf = fopen(mfname,"r")) == NULL) {
			fprintf(stderr,"cannot open %s\n",mfname);
			exit(1);
		}
		while(fgets(line, sizeof(line), mastf) != NULL) {
			if(line[0] == '$' && line[1] == '$')
				break;
			if(line[0] == '*' || line[0]=='\t' || line[0]==' ' || line[0]=='\n')
				continue;
			if( sscanf(line, " %s %*s %s ",
				     	flag,    pre) < 1)
				break;
			if (test(flag,'x'))
				break;
			if (strcmp(pre, "-") == 0)	/* kernel */
				break;
			strcat(strcpy(pre_addr,pre),"_addr");
			setln(dp->d_name, setup(pre_addr), test(flag,'b'), test(flag,'s') );
			setup(strcat(strcpy(rtn,pre),"open"));
			setup(strcat(strcpy(rtn,pre),"close"));
			setup(strcat(strcpy(rtn,pre),"read"));
			setup(strcat(strcpy(rtn,pre),"write"));
			setup(strcat(strcpy(rtn,pre),"ioctl"));
			setup(strcat(strcpy(rtn,pre),"strategy"));
			setup(strcat(strcpy(rtn,pre),"mmap"));
			setup(strcat(strcpy(rtn,pre),"segmap"));
			setup(strcat(strcpy(rtn,pre),"poll"));
			setup(strcat(strcpy(rtn,pre),"size"));
			break;
		}
		if(fclose(mastf) != 0){
			fprintf(stderr,"cannot close %s\n",mfname);
			exit(1);
		}
	}

	closedir(mast);		/* that's all folks */
#endif

	endnm = setup("");

	getnlist();


#ifndef i386
	/* boot program name */

	if (SYM_VALUE(sys3bboot) == 0)
		printf("\n  Boot program: *unknown*\n" );
	else
		{
		MEMSEEK(sys3bboot);	MEMREAD(bootname);
		printf("\n  Boot program: /%s", bootname.path[0]=='/'? bootname.path+1 : bootname.path );
		printf("\n  Time stamp:   %s", ctime(&bootname.timestamp) );
		}

	/* rootdev, dumpdev */

	MEMSEEK(rootdev);	MEMREAD(root);
	MEMSEEK(dumpdev);	MEMREAD(dump);

	printf("*\n* Devices\n*\n");
	devices();

	printf("*\n* Loadable Objects\n*\n");
	edtsect();

	printf("*\n* System Configuration\n*\n");

	MEMSEEK(MAJOR);
	MEMREAD(MAJ);

	sysdev();
#endif

	/* easy stuff */
	printf("*\n* Tunable Parameters\n*\n");
	nlptr = nl;
	vs = setup("v");
	tu = setup("tune");
	utsnm = setup("utsname");
	prlimits = setup("rlimits");
	FLckinfo = setup("flckinfo");
	endnm = msginfo = setup("msginfo");
	pnstrpush = setup("nstrpush");
	pstrthresh = setup("strthresh");
	pstrmsgsz = setup("strmsgsz");
	pstrctlsz = setup("strctlsz");
	pnadvertise = setup("nadvertise");
	pmaxgdp = setup("maxgdp");
	pnrcvd = setup("nrcvd");
	pnrduser = setup("nrduser");
	pnsndd = setup("nsndd");
	pminserve = setup("minserve");
	pmaxserve = setup("maxserve");
	prfsize = setup("rfsize");
	prfs_vhigh = setup("rfs_vhigh");
	prfs_vlow = setup("rfs_vlow");
	pnsrmount = setup("nsrmount");
	seminfo = setup("seminfo");
	shminfo = setup("shminfo");
	pnremote = setup("nremote");
	pnlocal = setup("nlocal");
	prcache_time = setup("rc_time");
	phrtimes_size = setup("hrtimes_size");
	pitimes_size = setup("itimes_size");
	pnaiosys = setup("aio_size");
	pminaios = setup("min_aio_servers");
	pmaxaios = setup("max_aio_servers");
	paiotimeout = setup("aio_server_timeout");
	pnaioproc = setup("naioproc");
	pts_maxupri = setup("ts_maxupri");
	psys_name = setup("sys_name");
	pinitclass = setup("intcls");
	evcinfo = setup("evcinfo");

#ifdef i386
	com2cons = setup("com2cons");
	do386b1 = setup("do386b1");
	do387cr3 = setup("do387cr3");
	maxminor = setup("maxminor");
	nclass = setup("nclass");
	ninode = setup("ninode");
	nstrphash = setup("nstrphash");
	piomaxsz = setup("piomaxsz");
	putbufsz = setup("putbufsz");
#endif

	setup("");

	getnlist();

	for(nlptr = &nl[vs]; nlptr != &nl[endnm]; nlptr++) {
		if(nlptr->n_value == 0) {
			fprintf(stderr, "namelist error\n");
			exit(1);
		}
	}
	MEMSEEK(vs);	MEMREAD(v);
	printf("%6d	buffers in buffer cache (NBUF)\n",v.v_buf);
	printf("%6d	entries in callout table (NCALL)\n",v.v_call);
	printf("%6d	entries in proc table (NPROC)\n",v.v_proc);
	printf("%6d	maximum global priority in sys class (MAXCLSYSPRI)\n",v.v_maxsyspri);
	printf("%6d	clist buffers (NCLIST)\n",v.v_clist);
	printf("%6d	processes per user id (MAXUP)\n",v.v_maxup);
	printf("%6d	hash slots for buffer cache (NHBUF)\n",v.v_hbuf);
	printf("%6d	buffers for physical I/O (NPBUF)\n",v.v_pbuf);
	printf("%6d	size of system virtual space map (SPTMAP)\n",
		v.v_sptmap);
	printf("%6d	maximum physical memory to use (MAXPMEM)\n",
		v.v_maxpmem);
	printf("%6d	auto update time limit in seconds (NAUTOUP)\n",
		v.v_autoup);
	MEMSEEK(tu);	MEMREAD(tune);
	printf("%6d  page stealing low water mark (GPGSLO)\n", tune.t_gpgslo);
	printf("%6d  fsflush run rate (FSFLUSHR)\n", tune.t_fsflushr);
	printf("%6d  minimum resident memory for avoiding deadlock (MINARMEM)\n",
		tune.t_minarmem);
	printf("%6d  minimum swapable memory for avoiding deadlock (MINASMEM)\n",
		tune.t_minasmem);

#ifdef i386
	MEMSEEK(nclass);	MEMREAD(nclass);
	printf("%6d  number of scheduler classes (NCLASS)\n", nclass);

	MEMSEEK(ninode);	MEMREAD(ninode);
	printf("%6d  number of inodes (NINODE)\n", ninode);

	printf("*\n* i386 Specific Tunables\n*\n");
	MEMSEEK(com2cons);	MEMREAD(com2cons);
	printf("%6d  Alternate console switch (COM2CONS)\n", com2cons);

	MEMSEEK(do386b1);	MEMREAD(do386b1);
	printf("%6d  80836 B1 stepping bug work around (DO386B1)\n", do386b1);

	MEMSEEK(do387cr3);	MEMREAD(do387cr3);
	printf("%6d  80387 errata #21 work around (DO387CR3)\n", do387cr3);

	MEMSEEK(maxminor);	MEMREAD(maxminor);
	printf("%6d  maximum value for major and minor numbers (MAXMINOR)\n", maxminor);

	MEMSEEK(nstrphash);	MEMREAD(nstrphash);
	printf("%6d  size of internal hash table (NSTRPHASH)\n", nstrphash);

	MEMSEEK(piomaxsz);	MEMREAD(piomaxsz);
	printf("%6d  size of virtual kernel address space for raw hard disk I/O (PIOMAXSZ)\n", piomaxsz);

	MEMSEEK(putbufsz);	MEMREAD(putbufsz);
	printf("%6d  size of buffer to record system messages (PUTBUFSZ)\n", putbufsz);
#endif

	printf("*\n* Utsname Tunables\n*\n");

#ifdef i386
	printf("%8s  release (REL)\n",utsname.release);
	printf("%8s  node name (NODE)\n",utsname.nodename);
	printf("%8s  system name (SYS)\n",utsname.sysname);
	printf("%8s  version (VER)\n",utsname.version);
#else
	MEMSEEK(utsnm);	MEMREAD(utsname);
	printf("%8s  release (REL)\n",utsname.release);
	printf("%8s  node name (NODE)\n",utsname.nodename);
	printf("%8s  system name (SYS)\n",utsname.sysname);
	printf("%8s  version (VER)\n",utsname.version);
#endif

	printf("*\n* Process Resource Limit Tunables (Current:Maximum)\n*\n");
	MEMSEEK(prlimits);	
	for (i = 0; i < RLIM_NLIMITS; i++) {
		MEMREAD(rlimit);
		if (rlimit.rlim_cur == RLIM_INFINITY)
			printf("Infinity  :");
		else

#ifdef i386
			printf("0x%8.8x:", rlimit.rlim_cur);
#else
			printf("%8x:", rlimit.rlim_cur);
#endif

		if (rlimit.rlim_max == RLIM_INFINITY)
			printf("Infinity");
		else

#ifdef i386
			printf("0x%8.8x", rlimit.rlim_max);
#else
			printf("%8x", rlimit.rlim_max);
#endif

		printf("\t%s\n", rlimitnames[i]);
	}

	printf("*\n* Streams Tunables\n*\n");
	if (SYM_VALUE(pnstrpush)) {
		MEMSEEK(pnstrpush);	MEMREAD(nstrpush);
		printf("%6d	maximum number of pushes allowed (NSTRPUSH)\n",
			nstrpush);
	}
	if (SYM_VALUE(pstrthresh)) {
		MEMSEEK(pstrthresh);	MEMREAD(strthresh);
		if (strthresh) {
			printf("%6ld	streams threshold in bytes (STRTHRESH)\n",
				strthresh);
		}
		else {
			printf("%6ld	no streams threshold (STRTHRESH)\n",
				strthresh);
		}
	}
	if (SYM_VALUE(pstrmsgsz)) {
		MEMSEEK(pstrmsgsz);	MEMREAD(strmsgsz);
		printf("%6d	maximum stream message size (STRMSGSZ)\n",
			strmsgsz);
	}
	if (SYM_VALUE(pstrctlsz)) {
		MEMSEEK(pstrctlsz);	MEMREAD(strctlsz);
		printf("%6d	max size of ctl part of message (STRCTLSZ)\n",
			strctlsz);
	}

	printf("*\n* RFS Tunables\n*\n");
	if (SYM_VALUE(pnadvertise)) {
		MEMSEEK(pnadvertise);	MEMREAD(nadvertise);
		printf("%6d	entries in advertise table (NADVERTISE)\n",
			nadvertise);
	}
	if (SYM_VALUE(pnrcvd)) {
		MEMSEEK(pnrcvd);	MEMREAD(nrcvd);
		printf("%6d	receive descriptors (NRCVD)\n",
			nrcvd);
	}
	if (SYM_VALUE(pnrduser)) {
		MEMSEEK(pnrduser);	MEMREAD(nrduser);
		printf("%6d	maximum number of rd_user structures (NRDUSER)\n",
			nrduser);
	}
	if (SYM_VALUE(pnsndd)) {
		MEMSEEK(pnsndd);	MEMREAD(nsndd);
		printf("%6d	send descriptors (NSNDD)\n",
			nsndd);
	}
	if (SYM_VALUE(pminserve)) {
		MEMSEEK(pminserve);	MEMREAD(minserve);
		printf("%6d	minimum number of server processes (MINSERVE)\n",
			minserve);
	}
	if (SYM_VALUE(pmaxserve)) {
		MEMSEEK(pmaxserve);	MEMREAD(maxserve);
		printf("%6d	maximum number of server processes (MAXSERVE)\n",
			maxserve);
	}
	if (SYM_VALUE(pmaxgdp)) {
		MEMSEEK(pmaxgdp);	MEMREAD(maxgdp);
		printf("%6d	maximum number of remote systems with active mounts (MAXGDP)\n",
			maxgdp);
	}
	if (SYM_VALUE(prfsize)) {
		MEMSEEK(prfsize);	MEMREAD(rfsize);
		printf("%6d	size of static RFS administrative storage area (RFHEAP)\n",
			rfsize);
	}
	if (SYM_VALUE(prfs_vhigh)) {
		MEMSEEK(prfs_vhigh);	MEMREAD(rfs_vhigh);
		printf("%6d	latest compatible RFS version (RFS_VHIGH)\n",
			rfs_vhigh);
	}
	if (SYM_VALUE(prfs_vlow)) {
		MEMSEEK(prfs_vlow);	MEMREAD(rfs_vlow);
		printf("%6d	earliest compatible RFS version (RFS_VLOW)\n",
			rfs_vlow);
	}
	if (SYM_VALUE(pnsrmount)) {
		MEMSEEK(pnsrmount);	MEMREAD(nsrmount);
		printf("%6d	entries in server mount table (NSRMOUNT)\n",
			nsrmount);
	}
	if (SYM_VALUE(prcache_time)) {
		MEMSEEK(prcache_time);	MEMREAD(rcache_time);
		printf("%6d	max interval for turning off RFS caching (RCACHE_TIME)\n",
			rcache_time);
	}
	if (SYM_VALUE(pnremote)) {
		MEMSEEK(pnremote);	MEMREAD(nremote);
		printf("%6d	minimum number of RFS buffers (NREMOTE)\n",
			nremote);
	}
	if (SYM_VALUE(pnlocal)) {
		MEMSEEK(pnlocal);	MEMREAD(nlocal);
		printf("%6d	minimum number of local buffers (NLOCAL)\n",
			nlocal);
	}
	if (SYM_VALUE(msginfo))
		{
		MEMSEEK(msginfo);	MEMREAD(minfo);
		printf("*\n* IPC Messages\n*\n");
		printf("%6d	entries in msg map (MSGMAP)\n",minfo.msgmap);
		printf("%6d	max message size (MSGMAX)\n",minfo.msgmax);
		printf("%6d	max bytes on queue (MSGMNB)\n",minfo.msgmnb);
		printf("%6d	message queue identifiers (MSGMNI)\n",minfo.msgmni);
		printf("%6d	message segment size (MSGSSZ)\n",minfo.msgssz);
		printf("%6d	system message headers (MSGTQL)\n",minfo.msgtql);
		printf("%6u	message segments (MSGSEG)\n",minfo.msgseg);
		}

	if (SYM_VALUE(seminfo))
		{
		MEMSEEK(seminfo);	MEMREAD(sinfo);
		printf("*\n* IPC Semaphores\n*\n");
		printf("%6d	entries in semaphore map (SEMMAP)\n",sinfo.semmap);
		printf("%6d	semaphore identifiers (SEMMNI)\n",sinfo.semmni);
		printf("%6d	semaphores in system (SEMMNS)\n",sinfo.semmns);
		printf("%6d	undo structures in system (SEMMNU)\n",sinfo.semmnu);
		printf("%6d	max semaphores per id (SEMMSL)\n",sinfo.semmsl);
		printf("%6d	max operations per semop call (SEMOPM)\n",sinfo.semopm);
		printf("%6d	max undo entries per process (SEMUME)\n",sinfo.semume);
		printf("%6d	semaphore maximum value (SEMVMX)\n",sinfo.semvmx);
		printf("%6d	adjust on exit max value (SEMAEM)\n",sinfo.semaem);
		}

	if (SYM_VALUE(shminfo))
		{
		MEMSEEK(shminfo);	MEMREAD(shinfo);
		printf("*\n* IPC Shared Memory\n*\n");
		printf("%6d	max shared memory segment size (SHMMAX)\n",shinfo.shmmax);
		printf("%6d	min shared memory segment size (SHMMIN)\n",shinfo.shmmin);
		printf("%6d	shared memory identifiers (SHMMNI)\n",shinfo.shmmni);
		printf("%6d	max attached shm segments per process (SHMSEG)\n",shinfo.shmseg);
		}

	printf("*\n* High Resolution Timer Tunables\n*\n");
	if (SYM_VALUE(phrtimes_size)) {
		MEMSEEK(phrtimes_size);	MEMREAD(hrtimes_size);
		printf("%6d	max number of timer structures for real-time clock (HRTIME)\n", hrtimes_size);
	}
	if (SYM_VALUE(pitimes_size)) {
		MEMSEEK(pitimes_size);	MEMREAD(itimes_size);
		printf("%6d	max number of timer structures for processes special clocks (HRVTIME)\n", itimes_size);
	}

	if (SYM_VALUE(pts_maxupri)) {
		printf("*\n* Time Sharing Scheduler Tunables\n*\n");
		MEMSEEK(pts_maxupri);	MEMREAD(ts_maxupri);
		printf("%d	maximum time sharing user priority (TSMAXUPRI)\n", ts_maxupri);
	}

	if (SYM_VALUE(psys_name)) {
		MEMSEEK(psys_name);	MEMREAD(sys_name);
		printf("%s	system class name (SYS_NAME)\n", sys_name);
	}

	if (SYM_VALUE(pinitclass)) {
		MEMSEEK(pinitclass);	MEMREAD(intcls);
		printf("%s	class of init process (INITCLASS)\n", intcls);
	}

	if (SYM_VALUE(pnaiosys)) {
		printf("*\n* Async I/O Tunables\n*\n");
		MEMSEEK(pnaiosys);	MEMREAD(aio_size);
		printf("%6d	outstanding async system calls(NAIOSYS)\n", aio_size);
	}
	if (SYM_VALUE(pminaios)) {
		MEMSEEK(pminaios);	MEMREAD(min_aio_servers);
		printf("%6d	minimum number of servers (MINAIOS)\n", min_aio_servers);
	}
	if (SYM_VALUE(pmaxaios)) {
		MEMSEEK(pmaxaios);	MEMREAD(max_aio_servers);
		printf("%6d	maximum number of servers (MAXAIOS)\n", max_aio_servers);
	}
	if (SYM_VALUE(paiotimeout)) {
		MEMSEEK(paiotimeout);	MEMREAD(aio_server_timeout);
		printf("%6d	number of secs an aio server will wait (AIOTIMEOUT)\n", aio_server_timeout);
	}
	if (SYM_VALUE(pnaioproc)) {
		MEMSEEK(pnaioproc);	MEMREAD(naioproc);
		printf("%6d	number of async requests per process (NAIOPROC)\n", naioproc);
	}


	if (SYM_VALUE(evcinfo)) {
		MEMSEEK(evcinfo);	MEMREAD(evinfo);
		printf("*\n* Events Tunables\n*\n");
		printf("%6d	max event queue structures (MEVQUEUES)\n", evinfo.evci_mevqueues);
		printf("%6d	max kernel event structures (MEVKEVS)\n", evinfo.evci_mevkevs);
		printf("%6d	max event expression reference structures (MEVEXREFS)\n", evinfo.evci_mevexrefs);
		printf("%6d	max event expression structures (MEVEXPRS)\n", evinfo.evci_mevexprs);
		printf("%6d	max event term structures (MEVTERMS)\n", evinfo.evci_mevterms);
		printf("%6d	max event satisfied expression structure (MEVSEXPRS)\n", evinfo.evci_mevsexprs);
		printf("%6d	max event satisfied term structures (MEVSTERMS)\n", evinfo.evci_mevsterms);
		printf("%6d	max trap identifier structures (MEVTIDS)\n", evinfo.evci_mevtids);
		printf("%6d	max retry structures (MEVRETRYS)\n", evinfo.evci_mevretrys);
		printf("%6d	max event exit structures (MEVEXITS)\n", evinfo.evci_mevexits);
		printf("%6d	max event signal structures (MEVSIGS)\n", evinfo.evci_mevsigs);
		printf("%6d	max stream data structures (MEVSTRDS)\n", evinfo.evci_mevstrds);
		printf("%6d	max directory entries (MEVDIRENTS)\n", evinfo.evci_mevdirents);
		printf("%6d	max bytes for holding data (EVDATA)\n", evinfo.evci_mevdata);
		printf("%6d	entries in the trap id hash table (EVTIDHTS)\n", evinfo.evci_tidhts);
		printf("%6d	entries in the file name hash table (EVFNHTS)\n", evinfo.evci_fnhts);
		printf("%6d	default max events on queue (EVMAXEV)\n", evinfo.evci_maxev);
		printf("%6d	default max bytes per event (EVMAXDPE)\n", evinfo.evci_maxdpe);
		printf("%6d	default max total bytes in memory (EVMAXMEM)\n", evinfo.evci_maxmem);
		printf("%6d	default max trap expression per process (EVMAXTRAPS)\n", evinfo.evci_maxtraps);
		printf("%6d	default max terms in an expression (EVMAXETERMS)\n", evinfo.evci_maxeterms);
	}

	if (elfd)
		elf_end(elfd);
	exit(0);
}

/*
 * setup - add an entry to a namelist structure array
 */
int
setup(nam)
	char	*nam;
{
	int idx;

	if(nlptr >= &nl[nlsize]) { 
		if ((nl=(struct nlist *)realloc(nl,(nlsize+EXPAND)*(sizeof(struct nlist)))) == NULL) {
			fprintf(stderr, "Namelist space allocation failed\n");
			exit(1);
		}
		nlptr=&nl[nlsize];
		nlsize+=EXPAND;
	}

	nlptr->n_name = malloc((unsigned)(strlen(nam)+1));	/* initialize pointer to next string */
	strcpy(nlptr->n_name,nam);	/* move name into string table */
	nlptr->n_type = 0;
	nlptr->n_value = 0;
	idx = nlptr++ - nl;
	return(idx);
}

#ifndef i386
/*
 * setln - set up internal link structure for later
 * function look-up.  Records useful information from the
 * /etc/master table description.
 */
void
setln(cf, nidx, block, software)
	char	*cf;
	int nidx;
	int block, software;
{
	if(lnptr >= &ln[lnsize]) {
		lnsize = lnptr - ln;
		if (( ln=(struct link *)realloc(ln, (lnsize+EXPAND)*(sizeof(struct link)))) == NULL ) {
			fprintf(stderr, "Internal Link space allocation failed\n");
			exit(1);
		}
		lnptr = &ln[lnsize];
		lnsize += EXPAND;
	}

	lnptr->l_cfnm = malloc((unsigned)strlen(cf)+2);	/* add space and null */
	if (!lnptr->l_cfnm)
		fprintf(stderr, "Internal Link name space allocation failed\n");
	strcat(strcpy(lnptr->l_cfnm, " "), cf);
	lnptr->l_funcidx = nidx;
	lnptr->l_soft = software;
	lnptr->l_dtype = block;
	lnptr->l_used = 0;
	lnptr++;
}

/*
 * Handle the configured devices
 */
devices()
{
	register struct link *lnkptr;
	register int idx;

	/*
	 * for each dev_addr array found, read the
	 * addresses and calculate the board slot
	 */
	for( lnkptr=ln; lnkptr < lnptr; ++lnkptr ) {
		idx = lnkptr->l_funcidx + ADDR;
		if ( ! (nl[idx]).n_value || lnkptr->l_soft )
			/* dev_addr undefined */
			continue;

		MEMSEEK(idx);
		for ( ; ; ) {
			MEMREAD( addr );

#ifdef u3b15
			if ( ! addr.valid )
				break;

			if ( addr.sys != 0 )
				printf( " %s\tboard=%d\t(on LBE)\n", lnkptr->l_cfnm, addr.base/256 );
			else
				printf( " %s\tboard=%d\n", lnkptr->l_cfnm, addr.base/256 );
#endif

#ifdef u3b2
			if ( addr == 0 )
				break;

			printf( " %s\tboard=%d\n", lnkptr->l_cfnm, ((addr-KVIOBASE)/2228224) + 1 );
#endif
		}
		lnkptr->l_used = 1;
	}
	/*
	 * for each remaining device, print the device if it has not
	 * already been used and it is found to be in the symbol table
	 */
	for( lnkptr=ln; lnkptr < lnptr; ++lnkptr ) {
		idx = lnkptr->l_funcidx;
		if ( ! lnkptr->l_used )
			if ( nl[idx+OPEN].n_value
			  || nl[idx+CLOSE].n_value
			  || nl[idx+READ].n_value
			  || nl[idx+WRITE].n_value
			  || nl[idx+IOCTL].n_value
			  || nl[idx+STRAT].n_value
			  || nl[idx+MMAP].n_value
			  || nl[idx+SEGMAP].n_value
			  || nl[idx+POLL].n_value
			  || nl[idx+SIZE].n_value ) {
					printf( " %s\n", lnkptr->l_cfnm );
					lnkptr->l_used = 1;
			}
	}
}
/*
 * Handle loadable objects
 */
edtsect()
{
	int i,l;
	struct s3bconf *s3bconf = NULL;
	Elf_Scn *scn;
	Elf32_Shdr *eshdr;
	char *strtab, *name;

	/*
	 * read <EDT> section header
	 */
	if ((scn = elf_getscn(elfd, ehdr->e_shstrndx)) == NULL) {
		fprintf(stderr, "%s: Error reading shstrndx (%s)\n",
			os, elf_errmsg(-1));
		exit (1);
	}

	if ((eshdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "%s: Error reading string Shdr header (%s)\n",
			os, elf_errmsg(-1));
		exit (1);
	}

	if ((strtab = malloc(eshdr->sh_size)) == NULL) {
		fprintf(stderr, "%s: Out of memory for string table\n", os);
		exit (1);
	}

	if ((fseek(sysfile, eshdr->sh_offset, 0)) != 0) {
		fprintf(stderr, "%s: string table seek error\n", os);
		exit (1);
	}

	if ((fread(strtab, eshdr->sh_size, 1, sysfile)) != (size_t)1) {
		fprintf(stderr, "%s: string table read error\n", os);
		exit (1);
	}

	scn = NULL;
	while ((scn = elf_nextscn(elfd, scn)) != NULL) {

		if ((eshdr = elf32_getshdr(scn)) == NULL) {
			fprintf(stderr, "%s: Error reading EDT Shdr\n", os);
			exit (1);
		}

		name = strtab + eshdr->sh_name;

		if ((name) && ((strcmp(name, "<EDT>")) == 0)) {

			if ((fseek(sysfile, eshdr->sh_offset, 0)) != 0) {
				fprintf(stderr, "Error seeking to EDT Section %s\n", os);
				exit (1);
			}

			s3bconf = (struct s3bconf *)malloc((unsigned)eshdr->sh_size);
			if (!s3bconf) {
				fprintf(stderr, "s3bconf: no memory\n");
				exit (1);
			}

			if ((fread((char *)s3bconf, eshdr->sh_size, 1, sysfile)) != (size_t)1) {
				fprintf(stderr, "fread error on EDT %s\n", os);
				exit (1);
			}

			break;
		}
	}

	if (strtab)
		free(strtab);

	if (!s3bconf)
		return (0);

	/*
	 * loop to extract loadable object names
	 */
	for (i=0; i < s3bconf->count; ++i)
	{
		if (s3bconf->driver[i].flag & S3BC_MOD)
		{
			lcase(s3bconf->driver[i].name);
			printf("  %s\n", s3bconf->driver[i].name);
		}
	}

	return (0);
}

lcase(s)
	char *s;
{
	int c;

	while (c = *s)
	{
		if (isascii(c) && isupper(c))
			*s = tolower(c);
		++s;
	}
}

sysdev()
{
	register struct link *lptr;
	major_t m;

	/* rootdev, dumpdev */

	if((lptr = majsrch(root)) == NULL)
		fprintf(stderr, "unknown root device\n");
	else
		{
		m = getemajor(root);
		if ( ONLBE(m) )
			printf("  rootdev\t%s(%d)\tminor=%d\ton LBE(%d)\n", lptr->l_cfnm, m&0x0F, geteminor(root), 13+((m&0x30)>>4) );
		else
			if (lptr->l_soft)
				printf("  rootdev\t%s\tminor=%d\n", lptr->l_cfnm, geteminor(root) );
			else
				printf("  rootdev\t%s(%d)\tminor=%d\n", lptr->l_cfnm, m, geteminor(root) );
		}
	printf("  swap files\n");
	if (system("/usr/sbin/swap -l") < 0)
		fprintf(stderr, "unknown swap file(s)\n");


#ifdef u3b15
	if((lptr = majsrch(dump)) == NULL)
		fprintf(stderr, "unknown dump device\n");
	else
		{
		m = getemajor(dump);
		if ( ONLBE(m) )
			printf("  dumpdev\t%s(%d)\tminor=%d\ton LBE(%d)\n", lptr->l_cfnm, m&0x0F, geteminor(dump), 13+((m&0x30)>>4) );
		else
			if (lptr->l_soft)
				printf("  dumpdev\t%s\tminor=%d\n", lptr->l_cfnm, geteminor(dump) );
			else
				printf("  dumpdev\t%s(%d)\tminor=%d\n", lptr->l_cfnm, m, geteminor(dump) );
		}
#endif
}


/*
 * return true if the flags from /etc/master contain the character "c"
 */
test( flags, c )
	register char *flags;
	char c;
	{
	char t;

	while( t = *flags++ )
		if ( t == c )
			return( 1 );
	return( 0 );
	}


/*
 * majsrch - search for a link structure given the device
 * number of the device in question.
 */
struct	link *
majsrch(device)
	dev_t	device;
{
	register struct link *lnkptr;
	register major_t maj;
	register struct nlist *nlp;
	struct bdevsw one_bdevsw;
	int int_maj;


	maj = getemajor(device);
	int_maj = (int)MAJ[(int)maj];

	MEMSEEK(bdev);
	fseek((incore ? memfile : sysfile), (int_maj*sizeof(struct bdevsw)), 1);
	MEMREAD(one_bdevsw);

	for(lnkptr = ln; lnkptr < lnptr; lnkptr++)
		if(lnkptr->l_dtype) {
			nlp = &nl[lnkptr->l_funcidx];
			if((nlp[STRAT]).n_value == (long)one_bdevsw.d_strategy)
				return(lnkptr);
		}
	return(NULL);
}
#endif

void
memseek(sym)
int sym;
{
	Elf_Scn *scn;
	Elf32_Shdr *eshdr;
	long eoff;

	if (incore) {
		if ((fseek(memfile, nl[sym].n_value, 0)) != 0) {
			fprintf(stderr, "%s: fseek error (in memseek)\n", mem);
			exit (1);
		}
	} else {
		if ((scn = elf_getscn(elfd, nl[sym].n_scnum)) == NULL) {
			fprintf(stderr, "%s: Error reading Scn %d (%s)\n",
				os, nl[sym].n_scnum, elf_errmsg(-1));
			exit (1);
		}

		if ((eshdr = elf32_getshdr(scn)) == NULL) {
			fprintf(stderr, "%s: Error reading Shdr %d (%s)\n",
				os, nl[sym].n_scnum, elf_errmsg(-1));
			exit (1);
		}

		eoff = (long)(nl[sym].n_value - eshdr->sh_addr + eshdr->sh_offset);

		if ((fseek(sysfile, eoff, 0)) != 0) {
			fprintf(stderr, "%s: fseek error (in memseek)\n", os);
			exit (1);
		}
	}
}

/*
**	filter out bss symbols if the reads are from the file
*/
void
getnlist()
{
	register struct nlist *p;

	nlist(os, nl);

		/*
		**	The nlist is done.  If any symbol is a bss
		**	and we are not reading from incore, zero
		**	the n_value field.  (Won't be printed if
		**	n_value == 0.)
		*/
	if (!incore) {
		for (p = nl; p->n_name && p->n_name[0]; p++) {
			if (p->n_scnum == bss) {
				p->n_value = 0;
			}
		}
	}
}
