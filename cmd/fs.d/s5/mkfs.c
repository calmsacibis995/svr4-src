/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)s5.cmds:mkfs.c	1.55"
/*	mkfs	COMPILE:	cc -O mkfs.c -s -i -o mkfs
 * mkfs - with support for 512, 1KB and 2KB logical block sizes
 * Make a file system prototype.
 */

#include <stdio.h>
#include <a.out.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/fcntl.h>
#include <sys/param.h>	/* included for definition of USIZE */
#include <sys/fs/s5param.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5ino.h>
#include <sys/stat.h>
#include <sys/fs/s5inode.h>
#include <sys/fs/s5macros.h>

#ifdef ELF_BOOT
#include <libelf.h>
#endif


#define MAX_FSBSIZE 	2048
/* boot-block size */
#define BBSIZE	512
/* super-block size */
#define SBSIZE	512
#define	LADDR	10
#define CLRSIZE 16384


#ifdef u3b15
#define STEPSIZE	24
#define CYLSIZE		32
#endif

#ifdef i386
#define	STEPSIZE	7
#define	CYLSIZE		400
#else
#define	STEPSIZE	10
#define	CYLSIZE		162
#endif

#define	MAXFN	1500

#define itod(inopb, inoshift, x) (daddr_t) ((x+(2*inopb-1)) >> inoshift) 
#define itoo(inopb, x) (daddr_t) ((x+2*inopb-1)& (inopb-1))


FILE 	*fin;
int	fsi;
int	fso;
int 	fd;
char	*charp;
char	buf[MAX_FSBSIZE];
char	zerobuf[CLRSIZE];

int work0[MAX_FSBSIZE/sizeof(int)];
struct fblk *fbuf = (struct fblk *)work0;

#if u3b || u3b15 || u3b2 || vax || i386
struct aouthdr head;
FILHDR filhdr;
#else
struct exec head;
#endif
char	string[512];

int work1[MAX_FSBSIZE/sizeof(int)];
struct filsys *filsys = (struct filsys *)work1;
struct filsys sblock;
char	*fsys;
char	*proto;
int	f_n = CYLSIZE;
int	f_m = STEPSIZE;
int	error;
ino_t	ino;
int blocksize;		/* logical block size */
long bsize;
int inoshift;
int sectpb;
int nidir;
int nfb;
int ndirect;
int nbinode;
long	getnum();
daddr_t	alloc();
time_t	cur_time;
extern char *optarg;		/* getopt(3c) specific */
extern int optind;


main(argc, argv)
int argc;
char *argv[];
{
	int f, c, i;
	int errflag = 0;
	int bflag = 0;
	int bpos = 0;
	int mflag = 0;
	int colon = 0;
	int arg;
	long n, nb;
	struct stat statarea;
	struct {
		daddr_t tfree;
		ino_t tinode;
		char fname[6];
		char fpack[6];
	} ustatarea;


	time(&cur_time);

	/* Process the Options */ 
	while ((arg = getopt(argc,argv,"?b:m")) != -1) {
		switch(arg) {
		case 'b':	
			bflag++;
			blocksize = atoi(optarg);
			break;
		case 'm':	
			mflag++;
			break;
		case '?':	/* print usage message */
			errusage();
		}
	}
	if (mflag) {
		fsys = argv[optind];
		dumpfs();
	}
	if (bflag) {
		if ((blocksize != 512) && (blocksize != 1024) && (blocksize != 2048)) {
			fprintf(stderr, "s5 mkfs: %d is invalid logical block size\n", blocksize);
			exit(31+1);
		}
	} else
		blocksize = 1024;	

	sectpb = blocksize / 512;
	nidir = blocksize / sizeof(daddr_t);
	nfb = nidir + 1300;
	ndirect = blocksize /sizeof(struct direct);
	nbinode = blocksize /sizeof(struct dinode);

	for ( i = nbinode, inoshift = 0; i > 1; i >>= 1)
		inoshift++;

	/* get the other arguments */
	fsys = argv[optind++];
	proto = argv[optind++];
	if ((bflag && argc >= 6) || (!bflag && argc >= 5)) {
		f_m = atoi(argv[optind++]);
		f_n = atoi(argv[optind++]);
		if (f_n <= 0 || f_n >= MAXFN)
			f_n = CYLSIZE;
		if (f_m <= 0 || f_m > f_n)
			f_m = STEPSIZE;
	}
	
	if(stat(fsys, &statarea) < 0) {
		fprintf(stderr, "s5 mkfs: cannot stat %s\n", fsys);
		exit(31+1);
	}
	fsi = open(fsys, 0);
	if(fsi < 0) {
		fprintf(stderr, "s5 mkfs: cannot open %s\n", fsys);
		exit(31+1);
	}
	if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
		fprintf(stderr, "s5 mkfs: %s: *** MOUNTED FILE SYSTEM\n", fsys);
		exit(31+1);
	}

	
	fso = creat(fsys, 0666);
	if(fso < 0) {
		fprintf(stderr, "s5 mkfs: cannot create %s\n", fsys);
		exit(31+1);
	}
	clr_fs();
	fin = fopen(proto, "r");
	if(fin == NULL) {	/* setup from command line (or bad proto file) */
		nb = n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				if(c == ':') {
					if (colon)
						errusage();
					else
						colon++;
					nb = n;
					n = 0;
					continue;
				}
				fprintf(stderr, "s5 mkfs: cannot open proto file '%s'\n", proto);
				exit(31+1);
			}
			n = n*10 + (c-'0');
		}
		if(!nb) {
			nb = n / sectpb;
			n = nb/4;
		} else {
			nb /= sectpb;
		}

				/* nb is number of logical blocks in fs,
				   n is number of inodes */

		filsys->s_fsize = nb;
		charp = "d--777 0 0\nlost+found d--777 0 0 $ $ ";
	}
	else {				/* proto file setup */

		/*
		 * get name of boot load program
		 * and read onto block 0
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) 
			fprintf(stderr, "s5 mkfs: cannot  open boot program '%s'\n", string);
		else {
			read(f, (char *)&filhdr, sizeof(filhdr));

			if (filhdr.f_magic == FBOMAGIC) {
				read(f, (char *)&head, sizeof head);

#if u3b || u3b15 || u3b2 || vax || i386
				c = head.tsize + head.dsize;
#else
				c = head.a_text + head.a_data;
#endif
				if(c > BBSIZE) 
					fprintf(stderr, "s5 mkfs: '%s' too big\n", string);
				else {
					read(f, buf, c);

					/* write boot-block to file system */
					lseek(fso, 0L, 0);
					if(write(fso, buf, BBSIZE) != BBSIZE) {
						fprintf(stderr, "s5 mkfs: error writing boot-block\n");
						exit(31+1);
					}
				}

				close(f);

#ifdef ELF_BOOT
		/* The following code is used to read ELF boot files.
		** If this code is used, you must link with libelf, where
		** some of these functions reside.
		*/
			} else {
				Elf *elfd;
				Elf32_Phdr *phdr;

				if ((elfd = elf_begin(f, ELF_C_READ, NULL)) == NULL) {
					fprintf(stderr, "Can't elf_begin %s\n", string);
					exit (31+1);
				}

				if ((phdr = elf32_getphdr(elfd)) == NULL) {
					fprintf(stderr, "Can't get Phdr %s\n", string);
					exit (31+1);
				}

				if (phdr->p_type == PT_LOAD) {
					if (phdr->p_filesz < BBSIZE) {
						lseek(f, (long)phdr->p_offset, 0);
						read(f, buf, phdr->p_filesz);
						lseek(fso, 0L, 0);
						if(write(fso, buf, BBSIZE) != BBSIZE) {
						fprintf(stderr, "s5 mkfs: error writing boot-block\n");
						exit(31+1);
						}
					} else {
						fprintf(stderr, "s5 mkfs: '%s' too big\n", string);
					}
				}
				elf_end(elfd);
				close(f);
#endif
			}
		}
		/*
		 * get total disk size
		 * and inode block size
		*/

		nb = getnum();
		filsys->s_fsize = nb / sectpb;
		n = getnum();

	}
	n /= nbinode;		/* number of logical blocs for inodes */
	if(n <= 0)
		n = 1;
	if(n > 65500/nbinode)
		n = 65500/nbinode;
	filsys->s_isize = n + 2;

	/* set magic number for file system type */
	filsys->s_magic = FsMAGIC;
	if (blocksize == 512)
		filsys->s_type = Fs1b;
	else if (blocksize == 1024)
		filsys->s_type = Fs2b;
	else if (blocksize == 2048)
		filsys->s_type = Fs4b;
	else {
		fprintf(stderr, "s5 mkfs: unknown block size\n");
		exit(31+1);
	}
	filsys->s_dinfo[0] = f_m;
	filsys->s_dinfo[1] = f_n;
	f_n /= sectpb;
	f_m = (f_m +(sectpb -1))/sectpb;  /* gap rounded up to the next block */

	printf("bytes per logical block = %d\n", blocksize);
	printf("total logical blocks = %ld\n", filsys->s_fsize);
	printf("total inodes = %ld\n", n*nbinode);
	printf("gap (physical blocks) = %d\n", filsys->s_dinfo[0]);
	printf("cylinder size (physical blocks) = %d \n", filsys->s_dinfo[1]);

	if((daddr_t)filsys->s_isize >= filsys->s_fsize) {
		fprintf(stderr, "s5 mkfs: %ld/%ld: bad ratio\n", filsys->s_fsize, filsys->s_isize-2);
		exit(31+1);
	}
	filsys->s_tinode = 0;
	for(c=0; c<blocksize; c++)
		buf[c] = 0;
	for(n=2; n!=filsys->s_isize; n++) {
		wtfs(n, buf);
		filsys->s_tinode += nbinode;
	}
	ino = 0;

	bflist();

	cfile((struct inode *)0);

	filsys->s_time = cur_time;
#if u3b2 || u3b15 || i386
	filsys->s_state = FsOKAY - (long)filsys->s_time;
#endif

/* write super-block onto file system */
	lseek(fso, (long)SUPERBOFF, 0);
	if(write(fso, (char *)filsys, SBSIZE) != SBSIZE) {
		fprintf(stderr, "s5 mkfs: write error: super-block\n");
		exit(31+1);
	}

	printf("mkfs: Available blocks = %ld\n",filsys->s_tfree);
	exit(error);
}

cfile(par)
struct inode *par;
{
	struct inode in;
	daddr_t bn, nblk;
	int dbc, ibc;
	char db[MAX_FSBSIZE];
	daddr_t ib[1812]; /* NIDIR + 1300 */
	int i, f, c, n;
	daddr_t blockno;

	/*
	 * get mode, uid and gid
	 */

	getstr();
	in.i_mode  = gmode(string[0], "-bcdl", IFREG, IFBLK, IFCHR, IFDIR, IFLNK, 0, 0);
	in.i_mode |= gmode(string[1], "-u", 0, ISUID, 0, 0, 0, 0, 0);
	in.i_mode |= gmode(string[2], "-g", 0, ISGID, 0, 0, 0, 0, 0);
	for(i=3; i<6; i++) {
		c = string[i];
		if(c<'0' || c>'7') {
			fprintf(stderr, "s5 mkfs: %c/%s: bad octal mode digit\n", c, string);
			error = 1;
			c = 0;
		}
		in.i_mode |= (c-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<blocksize; i++)
		db[i] = 0;
	for(i=0; i<nfb; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;
	if(par == (struct inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode & IFMT) {

 	case IFLNK:
 		/* symbolic link *
 		/* path represents the link it should point to */
 		getstr();
 		n = strlen(string);
 		in.i_size += n;
 		blockno = alloc();
 		lseek(fso, (long)(blockno*blocksize), 0);
 		write(fso, string, n);
 		ib[ibc++] = blockno;
 		if (ibc > nfb)
 			fprintf(stderr, "s5 mkfs: file too large\n");
 		break;

	case IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			fprintf(stderr, "s5 mkfs: cannot open %s\n", string);
			error = 1;
			break;
		}
		while((i=read(f, db, blocksize)) > 0) {
			in.i_size += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.i_addr[0] = (i<<8) | f;
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct direct);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct direct);
			cfile(&in);
		}
		break;

	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

gmode(c, s, m0, m1, m2, m3, m4, m5, m6)
char c, *s;
{
	int i;

	for(i=0; s[i]; i++)
		if(c == s[i])
			return((&m0)[i]);
	fprintf(stderr, "s5 mkfs: %c/%s: bad mode\n", c, string);
	error = 1;
	return(0);
}

long 
getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, "s5 mkfs: %s: bad number\n", string);
			error = 1;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		fprintf(stderr, "s5 mkfs: EOF\n");
		exit(31+1);

	case EOF:
		fprintf(stderr, "s5 mkfs: EOF\n");
		exit(31+1);

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} 
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fsi, (long)(bno*blocksize), 0);
	n = read(fsi, bf, blocksize);
	if(n != blocksize) {
		fprintf(stderr, "s5 mkfs: read error: %ld\n", bno);
		exit(31+1);
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fso, (long)(bno*blocksize), 0);
	n = write(fso, bf, blocksize);
	if(n != blocksize) {
		fprintf(stderr, "s5 mkfs: write error: %ld\n", bno);
		exit(31+1);
	}
}

clr_fs()
{
	int n;

	if (lseek(fso, (long)0, 0) <0) {
		fprintf(stderr, "s5 mkfs: seek error: %ld\n", (long)0);
		exit(31+1);
	}
	n = write(fso, zerobuf, CLRSIZE);
	if(n != CLRSIZE) {
		fprintf(stderr, "s5 mkfs: write error: %ld\n", (long)0);
		exit(31+1);
	}
}

daddr_t 
alloc()
{
	int i;
	daddr_t bno;

	filsys->s_tfree--;
	bno = filsys->s_free[--filsys->s_nfree];
	if(bno == 0) {
		fprintf(stderr, "s5 mkfs: out of free space\n");
		exit(31+1);
	}
	if(filsys->s_nfree <= 0) {
		rdfs(bno, (char *)fbuf);
		filsys->s_nfree = fbuf->df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys->s_free[i] = fbuf->df_free[i];
	}
	return(bno);
}


bfree(bno)
daddr_t bno;
{
	int i;

	filsys->s_tfree++;
	if(filsys->s_nfree >= NICFREE) {
		fbuf->df_nfree = filsys->s_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf->df_free[i] = filsys->s_free[i];
		wtfs(bno, (char *)fbuf);
		filsys->s_nfree = 0;
	}
	filsys->s_free[filsys->s_nfree++] = bno;
}

entry(in, str, adbc, db, aibc, ib)
ino_t in;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct direct *dp;
	int i;

	dp = (struct direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = in;
	for(i=0; i<DIRSIZ; i++)
		dp->d_name[i] = 0;
	for(i=0; i<DIRSIZ; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= ndirect)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<blocksize; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= nfb) {
		fprintf(stderr, "s5 mkfs: file too large\n");
		error = 1;
		*aibc = 0;
	}
}

getch()
{

	if(charp)
		return(*charp++);
	return(getc(fin));
}

bflist()
{
	struct inode in;
	daddr_t *ib; /* NIDIR + 1300 */
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	if (( ib = (daddr_t *)malloc(nfb*sizeof(daddr_t))) == NULL)
		fprintf(stderr, "s5 mkfs: cannot allocate spaace for ib\n");
	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_number = ino;
	in.i_mode = IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;

	for(i=0; i<nfb; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	filsys->s_tfree = 0;
	d = filsys->s_fsize;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
		for(i=0; i<f_n; i++) {
			f = d - adr[i];
			if(f < filsys->s_fsize && f >= (daddr_t)filsys->s_isize)
				if(badblk(f)) {
					if(ibc >= nidir) {
						fprintf(stderr, "s5 mkfs: too many bad blocks\n");
						error = 1;
						ibc = 0;
					}
					ib[ibc] = f;
					ibc++;
				} else {
					bfree(f);
				}
		}
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
register struct inode *ip;
register int *aibc;
daddr_t *ib;
{
	register struct dinode *dp;
	daddr_t d;
	register int i,j,k;
	daddr_t ib2[512];	/* a double indirect block */
	daddr_t ib3[512];	/* a triple indirect block */

	filsys->s_tinode--;
	d = itod(nbinode, inoshift, ip->i_number);
	if(d >= (daddr_t)filsys->s_isize) {
		if(error == 0)
			fprintf(stderr, "s5 mkfs: ilist too small\n");
		error = 1;
		return;
	}
	rdfs(d, buf);
	dp = (struct dinode *)buf;
	dp += itoo(nbinode, ip->i_number);

	dp->di_mode = ((ip->i_mode & IFMT )|ip->i_mode) ;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = cur_time;
	dp->di_mtime = cur_time;
	dp->di_ctime = cur_time;

	switch(ip->i_mode & IFMT) {

	case IFLNK:
	case IFDIR:
	case IFREG:
		/* handle direct pointers */
		for(i=0; i<*aibc && i<LADDR; i++) {
			ip->i_addr[i] = ib[i];
			ib[i] = 0;
		}
		/* handle single indirect block */
		if(i < *aibc)
		{
			for(j=0; i<*aibc && j<nidir; j++, i++)
				ib[j] = ib[i];
			for(; j<nidir; j++)
				ib[j] = 0;
			ip->i_addr[LADDR] = alloc();
			wtfs(ip->i_addr[LADDR], (char *)ib);
		}
		/* handle double indirect block */
		if(i < *aibc)
		{
			for(k=0; k<nidir && i<*aibc; k++)
			{
				for(j=0; i<*aibc && j<nidir; j++, i++)
					ib[j] = ib[i];
				for(; j<nidir; j++)
					ib[j] = 0;
				ib2[k] = alloc();
				wtfs(ib2[k], (char *)ib);
			}
			for(; k<nidir; k++)
				ib2[k] = 0;
			ip->i_addr[LADDR+1] = alloc();
			wtfs(ip->i_addr[LADDR+1], (char *)ib2);
		}
		/* handle triple indirect block */
		if(i < *aibc)
		{
			fprintf(stderr, "s5 mkfs: triple indirect blocks not handled\n");
		}
		break;

	case IFBLK:
		break;

	case IFCHR:
		break;


	default:
		fprintf(stderr, "s5 mkfs: bad ftype %o\n", (ip->i_mode &IFMT));
		exit(31+1);
	}

	ltol3(dp->di_addr, ip->i_addr, NADDR);
	wtfs(d, buf);
}

badblk(bno)
daddr_t bno;
{

	return(0);
}
errusage()
{
	fprintf(stderr, "s5 Usage:\n");
	fprintf(stderr, "mkfs [-F s5] [generic_options] special\n");
	fprintf(stderr, "mkfs [-F s5] [generic_options] [-b block_size] special blocks[:inodes] [gap blocks/cyl]\n");
        fprintf(stderr, "mkfs [-F s5] [generic_options] [-b block_size] special proto [gap blocks/cyl]\n");
	exit(31+1);
}
	
dumpfs()	
{
	/*
	 *	Read the super block associated with the fsys. 
	 */

	if ((fd = open(fsys, O_RDONLY)) < 0) {
		fprintf(stderr,"s5 mkfs: cannot open %s\n",fsys);
		exit(31+1);
	}
	
	if (lseek(fd, (long)SUPERBOFF, 0) < 0 
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		fprintf(stderr,"s5 mkfs: cannot read superblock\n");
		close(fd);
		exit(31+1);
	}
	
	if (sblock.s_magic != FsMAGIC) {
		fprintf(stderr,"s5 mkfs: %s is not an s5 file system\n",fsys);
		close(fd);
		exit(31+1);
	}
	switch(sblock.s_type) {
		case Fs1b:
			bsize = 512;
			break;
		case Fs2b:
			bsize = 1024;
			break;
		case Fs4b:
			bsize = 2048;
			break;
		default:
			exit(31+1);
	}
	printf("mkfs -F s5  [-b %d] %s %d[:%d] [%d %d]\n", 
			bsize, 
			fsys, 
			(sblock.s_fsize*bsize)/512, 
			((sblock.s_isize -2)*(bsize/(sizeof(struct dinode)))),
			sblock.s_dinfo[0], 
			sblock.s_dinfo[1]);
	close(fd);
	exit(0);
}
