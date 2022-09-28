/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cpiopc:cpiopc.c	1.3"

/*
** The -o option has been removed in 4.0.  Compile w/ -DVALID_O to allow the -o
** option to work.
*/

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/dir.h>

#define	NEW	0
#define	OLD	1
#define EQ(x,y)	(strcmp(x,y)==0)
#define MKSHORT(v,lv) {U.l=1L;if(U.c[0]) U.l=lv,v[0]=U.s[1],v[1]=U.s[0]; else U.l=lv,v[0]=U.s[0],v[1]=U.s[1];}
#define MAGIC	070707		/* cpio magic number */
#define IN	1		/* copy in */
#define OUT	2		/* copy out */
#define HDRSIZE	(Hdr.h_name - (char *)&Hdr)	/* header size minus filename field */
#define LINKS	1000		/* max no. of links allowed */
#define CHARS	76		/* ASCII header size minus filename field */
#define BUFSIZE 512
#define CPIOBSZ 4096		/* file read/write */

#ifdef VALID_O
struct	stat	Statb, Xstatb;
#else
struct	stat	Xstatb;
#endif

/*
 *      data describing UNIX PC disk specific information
 *      Volume Header Format
 */

#define VHBMAGIC        0x51565155      /* magic number in disk vhb */
struct vhb {
        long    magic;          /* miniframe disk format code */
        long     chksum;         /* adjustment so that the 32 bit sum starting
                                   from magic for 512 bytes sums to -1 */
        char    name[6];        /* printf name */
        short  cyls;           /* the number of cylinders for this disk */
        short  heads;          /* number of heads per cylinder */
        short  psectrk;        /* number of physical sectors per track */
        short  pseccyl;        /* number of physical sectors per cylinder */
        char    flags;          /* floppy density and high tech drive flags */
        char    step;           /* stepper motor rate to controller */
        short  sectorsz;       /* physical sector size in bytes */
        char   fill[4069];     /* Enough bytes to complete 8 sectors */
};

	/* Cpio header format */
struct header {
	short	h_magic,
		h_dev;
	ushort	h_ino,
		h_mode,
		h_uid,
		h_gid;
	short	h_nlink,
		h_rdev,
		h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[256];
} Hdr;

unsigned	Bufsize = BUFSIZE;		/* default record size */
short	Buf[CPIOBSZ/2], *Dbuf;
char	BBuf[CPIOBSZ], *Cbuf;
int	Wct, Wc;
short	*Wp;
char	*Cp;

short	Option,
	Dir,
	Uncond,
	Rename,
	Toc,
	Verbose,
	Select,
	Mod_time,
	Acc_time,
	Abort,
	Cflag,
	fflag;

struct passwd *npw;
struct passwd *getpwnam();
char	nuser[9];
char	nflag, nerr, wxyzflg, wflag, xflag, yflag, zflag;
char	ioflg, devflag;
short	fpcnt = 1;
char	floprerr[]="The floppy disk is misinserted or has an error.\nPlease check the floppy disk.\n";
char	flopwerr[]="The floppy disk is write-protected, misinserted or has an error.\nPlease check the floppy disk.\n";

int	Ofile,
#ifdef VALID_O
	Ifile,
#endif
	Input = 0,
	Output = 1;
long	Blocks,
	Longfile,
	Longtime;

char	Fullname[256],
	pcdev[40],
#ifdef VALID_O
	Name[256],
#endif
	pcbuf[CPIOBSZ];

int	Pathend;

FILE	*Rtty,
	*Wtty,
	*devtty;

char	*Pattern[100];
char	Strhdr[500];
char	*Chdr = Strhdr;
short	Uid,
	Gid,
	A_directory,
	A_special,
	Filetype = (short) S_IFMT;

extern	errno;
char	*malloc();
/*	char	*Cd_name;	*/
FILE 	*popen();

union { long l; short s[2]; char c[4]; } U;

long mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0])
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return (long) U.l;
}

main(argc, argv)
char **argv;
{
	register ct;
	long	filesz;
	register i, j;
	char *nptr;

	signal(SIGSYS, SIG_IGN);
	if(*argv[1] != '-')
		usage();
	Uid = getuid();
	umask(0);
	Gid = getgid();
	Pattern[0] = "*";

	while(*++argv[1]) {
		switch(*argv[1]) {
		case 'a':		/* reset access time */
			Acc_time++;
			break;
		case 'B':		/* change record size to 5120 bytes */
			Bufsize = 5120;
			break;
		case 'i':
			Option = IN;
			ioflg++;
			if(argc > 2 ) {	/* check for -n; save patterns,*/
				i=2;
				if ((strncmp(argv[2], "-n", 2)==0)){
					nflag++;
					nptr=argv[2];
					if (nptr[2] != 0){
						i=3;
						strncpy(nuser, &nptr[2], sizeof(nuser));
					}
					else
						if(argc < 4){
							fprintf(stderr,"cpiopc: -n option requires a user name\n");
							usage();
						}
						else{
							i=4;
							strncpy(nuser, argv[3], sizeof(nuser));
						}
				}
				if (argc > i){	 /* save patterns, if any */
					for(j=0; i <= argc; i++, j++)
						Pattern[j] = argv[i];
				}
			}
			break;
		case 'f':	/* do not consider patterns in cmd line */
			fflag++;
			break;
#ifdef VALID_O
		case 'o':
			if(argc != 2)
				usage();
			Option = OUT;
			ioflg++;
			break;
#endif /* VALID_O */
		case 'c':		/* ASCII header */
			Cflag++;
			break;
		case 'd':		/* create directories when needed */
			Dir++;
			break;
		case 'm':		/* retain mod time */
			Mod_time++;
			break;
		case 'r':		/* rename files interactively */
			Rename++;
			Rtty = fopen("/dev/tty", "r");
			Wtty = fopen("/dev/tty", "w");
			if(Rtty==NULL || Wtty==NULL) {
				fprintf(stderr,
				  "Cannot rename (/dev/tty missing)\n");
				exit(2);
			}
			break;
		case 't':		/* table of contents */
			Toc++;
			break;
		case 'u':		/* copy unconditionally */
			Uncond++;
			break;
		case 'v':		/* verbose table of contents */
			Verbose++;
			break;
		case 'w':		/* Use S5 Dual Format & raw dev */
			wflag++;
			wxyzflg++;
			break;
		case 'x':		/* Use UNIX PC Flop Format & raw dev */
			xflag++;
			wxyzflg++;
			break;
		case 'y':		/* Use S5 Floppy 9 Track 48 TPI */
			yflag++;
			wxyzflg++;
			break;
		case 'z':		/* Use S5 Floppy 15 Track 96 TPI */
			zflag++;
			wxyzflg++;
			break;
		case '0':		/* 0 drive is default */
			devflag = '0';
			break;
		case '1':
			devflag = '1';
			break;
		case '2':
			devflag = '2';
			break;
		case '3':
			devflag = '3';
			break;
		case '4':
			devflag = '4';
			break;
		case '5':
			devflag = '5';
			break;
		case 'K':
			Abort++;
			break;
		case 'n':		/* Can't group -n option here */
			nerr++;
			break;
		default:
			usage();
		}
	}
	if(nerr) {
		fprintf(stderr,"cpiopc: -n option must follow -i, and be grouped separately\n");
		usage();
	}
	if(nflag){
		if (Uid != 0){
			fprintf(stderr,"cpiopc: The -n option is restricted to superuser.\n");
			exit(2);
		}
		setpwent();
		if ( (npw = getpwnam(nuser) ) == 0){
			fprintf(stderr,"User name %s not valid\n", nuser);
			exit(2);
		}
	}
	if(ioflg != 1) {
		fprintf(stderr,"Can only specify -i.\n");
		exit(2);
	}
	if (wxyzflg || devflag){
		if( wxyzflg!=1 ){
			fprintf(stderr,"Only one of the w, x, y or z options is allowed along with\na device specifier 0, 1 or 2 (0 is default).\n");
		exit(2);
		}
		wxyz();
	}
	if(Cflag)
	    Cp = Cbuf = (char *)malloc(Bufsize);
	else
	    Wp = Dbuf = (short *)malloc(Bufsize);
	Wct = Bufsize >> 1;
	Wc = Bufsize;

	switch(Option) {

#ifdef VALID_O
	case OUT:
		if (wxyzflg){
			close(Output);
			if((Output = open(pcdev, 1)) < 0) {
				if (errno == ENOENT)
					fprintf(stderr,"No such floppy device.\n");
				else
					fprintf(stderr,flopwerr);
				exit(2);
			}
			devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
			fprintf(devtty,"Transfer in progress - Do not remove the floppy disk.\n");
			fclose(devtty);
		}
		if (xflag){	/* Write past first track for UNIX PC */
			upchead();
			write(Output, pcbuf, CPIOBSZ);
		}
		/* get filename, copy header and file out */
		while(getname()) {
			if( mklong(Hdr.h_filesize) == 0L) {
			   if( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			   else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
				continue;
			}
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fprintf(stderr,"<%s> ?\n", Hdr.h_name);
				continue;
			}
			if ( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				if(read(Ifile, Cflag? BBuf: (char *)Buf, ct) < 0) {
					fprintf(stderr,"Cannot read %s\n", Hdr.h_name);
					continue;
				}
				Cflag? writehdr(BBuf,ct): bwrite(Buf,ct);
			}
			close(Ifile);
			if(Acc_time)
				utime(Hdr.h_name, &Statb.st_atime);
			if(Verbose)
				fprintf(stderr,"%s\n", Hdr.h_name);
		}

	/* copy trailer, after all files have been copied */
		strcpy(Hdr.h_name, "TRAILER!!!");
		Hdr.h_magic = MAGIC;
		MKSHORT(Hdr.h_filesize, 0L);
		Hdr.h_namesize = strlen("TRAILER!!!") + 1;
		if ( Cflag )  {
			bintochar(0L);
			writehdr(Chdr,CHARS+Hdr.h_namesize);
		}
		else
			bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
		Cflag? writehdr(Cbuf, Bufsize): bwrite(Dbuf, Bufsize);
		break;
#endif /* VALID_O */

	case IN:
		if (wxyzflg){
			close(Input);
			if((Input = open(pcdev, 0)) < 0) {
				if (errno == ENOENT || errno == ENXIO)
					fprintf(stderr,"No such floppy device.\n");
				else
					fprintf(stderr, floprerr);
				exit(2);
			}
			devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
			fprintf(devtty,"Transfer in progress - Do not remove the floppy disk.\n");
			fclose(devtty);
			if (xflag)	/* Go past first track for UNIX PC */
				read(Input, pcbuf, CPIOBSZ);
		}
		pwd();
		while(gethdr()) {
			Ofile = ckname(Hdr.h_name)? openout(Hdr.h_name): 0;
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				Cflag? readhdr(BBuf,ct): bread(Buf, ct);
				if(Ofile) {
					if(write(Ofile, Cflag? BBuf: (char *)Buf, ct) < 0) {
					 fprintf(stderr,"Cannot create or write %s\n", Hdr.h_name);
					 continue;
					}
				}
			}
			if(Ofile) {
				close(Ofile);
				if(chmod(Hdr.h_name, Hdr.h_mode) < 0) {
					fprintf(stderr,"Cannot chmod <%s> (errno:%d)\n", Hdr.h_name, errno);
				}
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			}
			if(!Select)
				continue;
			if(Verbose)
				if(Toc)
					pentry(Hdr.h_name);
				else
					puts(Hdr.h_name);
			else if(Toc)
				puts(Hdr.h_name);
			if(Abort)
				exit(0);
		}
	}


	if (wxyzflg){
		devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
		fprintf(devtty,"It is safe to remove the floppy disk.\n");
		fclose(devtty);
	}
	/* print number of blocks actually copied */
	   fprintf(stderr,"%ld blocks\n", Blocks * (Bufsize>>9));
	exit(0);
}
usage()
{
#ifdef VALID_O
	fprintf(stderr,"Usage: cpiopc -o[acvBwxyz0..5] <name-list [>collection]\n%s\n",
	"       cpiopc -i[cdmrtuvfBKwxyz0..5] [-n user] [pattern ...] [<collection]");
#else
	fprintf(stderr,"Usage: cpiopc -i[cdmrtuvfBKwxyz0..5] [-n user] [pattern ...] [<collection]\n");
#endif /* VALID_O */
	exit(2);
}

#ifdef VALID_O
getname()		/* get file name, get info for header */
{
	register char *namep = Name;
	register ushort ftype;
	long tlong;

	for(;;) {
		if(gets(namep) == NULL)
			return 0;
		if(*namep == '.' && namep[1] == '/')
			namep += 2;
		strcpy(Hdr.h_name, namep);
		if(stat(namep, &Statb) < 0) {
			fprintf(stderr,"< %s > ?\n", Hdr.h_name);
			continue;
		}
		ftype = Statb.st_mode & Filetype;
		A_directory = (ftype == S_IFDIR);
		A_special = (ftype == S_IFBLK)
			|| (ftype == S_IFCHR)
			|| (ftype == S_IFIFO);
		Hdr.h_magic = MAGIC;
		Hdr.h_namesize = strlen(Hdr.h_name) + 1;
		Hdr.h_uid = Statb.st_uid;
		Hdr.h_gid = Statb.st_gid;
		Hdr.h_dev = Statb.st_dev;
		Hdr.h_ino = Statb.st_ino;
		Hdr.h_mode = Statb.st_mode;
		MKSHORT(Hdr.h_mtime, Statb.st_mtime);
		Hdr.h_nlink = Statb.st_nlink;
		tlong = (Hdr.h_mode&S_IFMT) == S_IFREG? Statb.st_size: 0L;
		MKSHORT(Hdr.h_filesize, tlong);
		Hdr.h_rdev = Statb.st_rdev;
		if( Cflag )
		   bintochar(tlong);
		return 1;
	}
}

bintochar(t)		/* ASCII header write */
long t;
{
	sprintf(Chdr,"%.6o%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		MAGIC,Statb.st_dev,Statb.st_ino,Statb.st_mode,Statb.st_uid,
		Statb.st_gid,Statb.st_nlink,Statb.st_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(Hdr.h_name)+1,t,Hdr.h_name);
}
#endif /* VALID_O */

chartobin()		/* ASCII header read */
{
	register int rv;
	rv = sscanf(Chdr,"%6ho%6ho%6ho%6ho%6ho%6ho%6ho%6ho%11lo%6ho%11lo",
		&Hdr.h_magic,&Hdr.h_dev,&Hdr.h_ino,&Hdr.h_mode,&Hdr.h_uid,
		&Hdr.h_gid,&Hdr.h_nlink,&Hdr.h_rdev,&Longtime,&Hdr.h_namesize,
		&Longfile);
	if (rv != 11)
		/* bad convert force out of phase */
		Hdr.h_magic = 0;
	MKSHORT(Hdr.h_filesize, Longfile);
	MKSHORT(Hdr.h_mtime, Longtime);
}

gethdr()		/* get file headers */
{
	register ushort ftype;

	if (Cflag)  {
		readhdr(Chdr,CHARS);
		chartobin();
	}
	else
		bread(&Hdr, HDRSIZE);

	if(Hdr.h_magic != MAGIC) {
		fprintf(stderr,"Cannot read file header information");
		if(wxyzflg)
			fprintf(stderr,"--please check the floppy disk.\n");
		else
			fprintf(stderr,".\n");
		exit(2);
	}
	if(Cflag)
		readhdr(Hdr.h_name, Hdr.h_namesize);
	else
		bread(Hdr.h_name, Hdr.h_namesize);
	if(EQ(Hdr.h_name, "TRAILER!!!"))
		return 0;
	ftype = Hdr.h_mode & Filetype;
	A_directory = (ftype == S_IFDIR);
	A_special =(ftype == S_IFBLK)
		|| (ftype == S_IFCHR)
		|| (ftype == S_IFIFO);
	return 1;
}

ckname(namep)	/* check filenames with patterns given on cmd line */
register char *namep;
{
	++Select;
	if(fflag ^ !nmatch(namep, Pattern)) {
		Select = 0;
		return 0;
	}
	if(Rename && !A_directory) {	/* rename interactively */
		fprintf(Wtty, "Rename <%s>\n", namep);
		fflush(Wtty);
		fgets(namep, 128, Rtty);
		if(feof(Rtty))
			exit(2);
		namep[strlen(namep) - 1] = '\0';
		if(EQ(namep, "")) {
			printf("Skipped\n");
			return 0;
		}
	}
	return !Toc;
}

openout(namep)	/* open files for writing, set all necessary info */
register char *namep;
{
	register f;
	register char *np;
	int ans;

	if(!strncmp(namep, "./", 2))
		namep += 2;
	np = namep;
	if(A_directory) {
		if(!Dir
		|| Rename
		|| EQ(namep, ".")
		|| EQ(namep, ".."))	/* do not consider . or .. files */
			return 0;
		if(stat(namep, &Xstatb) == -1) {

/* try creating (only twice) */
			ans = 0;
			do {
				if(makdir(namep) != 0) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(namep) == 0);
			if(ans == 1) {
				fprintf(stderr,"Cannot create directory for <%s> (errno:%d)\n", namep, errno);
				return(0);
			}else if(ans == 2) {
				fprintf(stderr,"Cannot create directory <%s> (errno:%d)\n", namep, errno);
				return(0);
			}
		}

ret:
		if(chmod(namep, Hdr.h_mode) < 0) {
			fprintf(stderr,"Cannot chmod <%s> (errno:%d)\n", namep, errno);
		}
		if (Uid == 0){
			if(nflag){
				if(chown(namep, npw->pw_uid, npw->pw_gid) < 0) 
					fprintf(stderr,"Cannot chown <%s> (errno:%d)\n", namep, errno);
			}
			else{
				if(chown(namep, Hdr.h_uid, Hdr.h_gid) < 0) 
					fprintf(stderr,"Cannot chown <%s> (errno:%d)\n", namep, errno);
			}
		}
		set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
		return 0;
	}
	if(Hdr.h_nlink > 1)
		if(!postml(namep, np))
			return 0;
	if(stat(namep, &Xstatb) == 0) {
		if(Uncond && !((!(Xstatb.st_mode & S_IWRITE) || A_special) && (Uid != 0))) {
			if(unlink(namep) < 0) {
				fprintf(stderr,"cannot unlink current <%s> (errno:%d)\n", namep, errno);
			}
		}
		if(!Uncond && (mklong(Hdr.h_mtime) <= Xstatb.st_mtime)) {
		/* There's a newer version of file on destination */
			if(mklong(Hdr.h_mtime) < Xstatb.st_mtime)
				fprintf(stderr,"current <%s> newer\n", np);
			return 0;
		}
	}
	if(A_special) {
		if((Hdr.h_mode & Filetype) == S_IFIFO)
			Hdr.h_rdev = 0;

/* try creating (only twice) */
		ans = 0;
		do {
			if(mknod(namep, Hdr.h_mode, Hdr.h_rdev) < 0) {
				ans += 1;
			}else {
				ans = 0;
				break;
			}
		}while(ans < 2 && missdir(np) == 0);
		if(ans == 1) {
			fprintf(stderr,"Cannot create directory for <%s> (errno:%d)\n", namep, errno);
			return(0);
		}else if(ans == 2) {
			fprintf(stderr,"Cannot mknod <%s> (errno:%d)\n", namep, errno);
			return(0);
		}

		goto ret;
	}

/* try creating (only twice) */
	ans = 0;
	do {
		if((f = creat(namep, Hdr.h_mode)) < 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	}while(ans < 2 && missdir(np) == 0);
	if(ans == 1) {
		fprintf(stderr,"Cannot create directory for <%s> (errno:%d)\n", namep, errno);
		return(0);
	}else if(ans == 2) {
		fprintf(stderr,"Cannot create <%s> (errno:%d)\n", namep, errno);
		return(0);
	}

	if(Uid == 0){
		if(nflag){
			if(chown(namep, npw->pw_uid, npw->pw_gid) < 0) 
				fprintf(stderr,"Cannot chown <%s> (errno:%d)\n", namep, errno);
		}
		else{
			if(chown(namep, Hdr.h_uid, Hdr.h_gid) < 0) 
				fprintf(stderr,"Cannot chown <%s> (errno:%d)\n", namep, errno);
		}
	}
	return f;
}

bread(b, c)
register c;
register short *b;
{
	int rdflg;
	static nleft = 0;
	static short *ip;
	register int rv;
	register short *p = ip;
	register int in;

	c = (c+1)>>1;
	while(c--) {
		if(nleft == 0) {
			rdflg=NEW;
			in = 0;
			while((rv=read(Input, &(((char *)Dbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if ((rv < 0)&&(errno != ENXIO)) {
				    if (rdflg == NEW){
					Input = chgreel(0, Input, 0);
					continue;
				    }
				    else{
					perror("I/O failure");
					fprintf(stderr,"Can't %s; aborting.\n",
					    "read input");
					exit(2);
				    }
				}
				if(rv <= 0) {
					Input = chgreel(0, Input, 1);
					/* re-start block */
					in = 0;
					nleft = 0;
					rdflg=NEW;
					continue;
				}
				rdflg=OLD; /* Some data is readable */
				in += rv;
				nleft += (rv >> 1);
			}
			nleft += (rv >> 1);
			p = Dbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

readhdr(b, c)
register c;
register char *b;
{
	int rdflg;
	static nleft = 0;
	static char *ip;
	register int rv;
	register char *p = ip;
	register int in;

	while(c--)  {
		if(nleft == 0) {
			in = 0;
			rdflg=NEW;
			while((rv=read(Input, &(((char *)Cbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if ((rv < 0)&&(errno != ENXIO)) {
				    if (rdflg == NEW){
					Input = chgreel(0, Input, 0);
					continue;
				    }
				    else{
					perror("I/O failure");
					fprintf(stderr,"Can't %s; aborting.\n",
					    "read input");
					exit(2);
				    }
				}
				if(rv <= 0) {
					Input = chgreel(0, Input, 1);
					/* re-start block */
					in = 0;
					nleft = 0;
					rdflg=NEW;
					continue;
				}
				rdflg=OLD;
				in += rv;
				nleft += rv;
			}
			nleft += rv;
			p = Cbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

#ifdef VALID_O
bwrite(rp, c)
register short *rp;
register c;
{
	register short *wp = Wp;
	register int rv;
	int wrflg;

	c = (c+1) >> 1;
	wrflg=NEW;
	while(c--) {
		if(!Wct) {
again:
			if((rv = write(Output, Dbuf, Bufsize)) != Bufsize) {
				if ((rv < 0)&&(errno != ENXIO)) {
				    if (wrflg == NEW){
					Output = chgreel(1, Output, 0);
					continue;
				    }
				    else{
					perror("I/O failure");
					fprintf(stderr,"Can't write output; aborting.\n");
					exit(2);
				    }
				}
				Output = chgreel(1, Output, 1);
				wrflg=NEW;
				goto again;
			}
			wrflg=OLD;
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

writehdr(rp, c)
register char *rp;
register c;
{
	register char *cp = Cp;
	register int rv;
	int wrflg;

	wrflg=NEW;
	while(c--)  {
		if(!Wc)  {
again:
			if((rv = write(Output,Cbuf,Bufsize)) != Bufsize)  {
				if ((rv < 0)&&(errno != ENXIO)) {
				    if (wrflg == NEW){
					Output = chgreel(1, Output, 0);
					continue;
				    }
				    else{
					perror("I/O failure");
					fprintf(stderr,"Can't write output; aborting.\n");
					exit(2);
				    }
				}
				Output = chgreel(1, Output, 1);
				wrflg=NEW;
				goto again;
			}
			wrflg=OLD;
			Wc = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Wc;
	}
	Cp = cp;
}
#endif /* VALID_O */

postml(namep, np)		/* linking funtion */
register char *namep, *np;
{
	register i;
	static struct ml {
		short	m_dev,
			m_ino;
		char	m_name[2];
	} *ml[LINKS];
	static	mlinks = 0;
	char *mlp;
	int ans;

	for(i = 0; i < mlinks; ++i) {
		if(mlinks == LINKS) break;
		if(ml[i]->m_ino==Hdr.h_ino &&
			ml[i]->m_dev==Hdr.h_dev) {
			if(Verbose)
			  printf("%s linked to %s\n", ml[i]->m_name,
				np);
			unlink(namep);
			if(Option == IN && *ml[i]->m_name != '/') {
				Fullname[Pathend] = '\0';
				strcat(Fullname, ml[i]->m_name);
				mlp = Fullname;
			}
			mlp = ml[i]->m_name;

/* try linking (only twice) */
			ans = 0;
			do {
				if(link(mlp, namep) < 0) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(np) == 0);
			if(ans == 1) {
				fprintf(stderr,"Cannot create directory for <%s> (errno:%d)\n", np, errno);
				return(0);
			}else if(ans == 2) {
				fprintf(stderr,"Cannot link <%s> & <%s>.\n", ml[i]->m_name, np);
				return(0);
			}

			set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			return 0;
		}
	}
	if(mlinks == LINKS
	|| !(ml[mlinks] = (struct ml *)malloc(strlen(np) + 2 + sizeof(struct ml)))) {
		static int first=1;

		if(first)
			if(mlinks == LINKS)
				fprintf(stderr,"Too many links\n");
			else
				fprintf(stderr,"No memory for links\n");
		mlinks = LINKS;
		first = 0;
		return 1;
	}
	ml[mlinks]->m_dev = Hdr.h_dev;
	ml[mlinks]->m_ino = Hdr.h_ino;
	strcpy(ml[mlinks]->m_name, np);
	++mlinks;
	return 1;
}

pentry(namep)		/* print verbose table of contents */
register char *namep;
{

	static short lastid = -1;
	static struct passwd *pw;
	struct passwd *getpwuid();
	static char tbuf[32];
	char *ctime();

	printf("%-7o", Hdr.h_mode & 0177777);
	if(lastid == Hdr.h_uid)
		printf("%-6s", pw->pw_name);
	else {
		setpwent();
		if(pw = getpwuid((int)Hdr.h_uid)) {
			printf("%-6s", pw->pw_name);
			lastid = Hdr.h_uid;
		} else {
			printf("%-6d", Hdr.h_uid);
			lastid = -1;
		}
	}
	printf("%7ld ", mklong(Hdr.h_filesize));
	U.l = mklong(Hdr.h_mtime);
	strcpy(tbuf, ctime((long *)&U.l));
	tbuf[24] = '\0';
	printf(" %s  %s\n", &tbuf[4], namep);
}

		/* pattern matching functions */
nmatch(s, pat)
char *s, **pat;
{
	if(EQ(*pat, "*"))
		return 1;
	while(*pat) {
		if((**pat == '!' && !gmatch(s, *pat+1))
		|| gmatch(s, *pat))
			return 1;
		++pat;
	}
	return 0;
}
gmatch(s, p)
register char *s, *p;
{
	register int c;
	register cc, ok, lc, scc;

	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		ok = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (ok)
					return(gmatch(++s, ++p));
				else
					return(0);

			case '-':
				ok |= ((lc <= scc) && (scc <= (cc=p[1])));
			}
			if (scc==(lc=cc)) ok++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(gmatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
register char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (gmatch(s++,p)) return(1);
	return(0);
}

makdir(namep)		/* make needed directories */
register char *namep;
{
	static status;
	register pid;

	if(pid = fork())
		while(wait(&status) != pid);
	else {
		close(2);
		execl("/bin/mkdir", "mkdir", namep, 0);
		exit(2);
	}
	return ((status>>8) & 0377)? 1: 0;
}

set_time(namep, atime, mtime)	/* set access and modification times */
register *namep;
long atime, mtime;
{
	static long timevec[2];

	if(!Mod_time)
		return;
	timevec[0] = atime;
	timevec[1] = mtime;
	utime(namep, timevec);
}

/* chgreel - WARNING!!! stdin (file desc 0) MUST ALWAYS be the input
 * archive; as various pieces of code test a valid file descritor as >0
 *
 * When you enter with inc=0, it means retry the current floppy.
 */

chgreel(x, fl, inc)
{
	register f;
	char str[22];

	if(!wxyzflg){
		fprintf(stderr,"errno: %d, ", errno);
		fprintf(stderr,"Can't %s\n", x? "write output": "read input");
		exit(2);
	}
	close(fl);
	if (Abort){
		fprintf(stderr,"cpiopc: Cannot find file on first diskette.\n");
		exit(2);
	}
	devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
	if (inc == 0){
		fprintf(devtty,"Bad access to floppy disk number %d.\n",fpcnt);
		fprintf(devtty, x? flopwerr: floprerr);
	}
	else
		fprintf(devtty,"You may remove floppy disk number %d.\n",fpcnt++);
again:
	fprintf(devtty,"To EXIT - press <E> followed by <RETURN>.\nTo continue - insert floppy disk number %d and press the <RETURN> key. ",fpcnt);
	f = 0;
	strcpy(str, "\n");	/* something in case fgets fails */
	fgets(str, 20, devtty);
	str[strlen(str) - 1] = '\0';
	if ((*str=='e')||(*str=='E'))
		exit(4);
	/* close terminal NOW so new open get fd 0 */
	fclose(devtty);
	if((f = open(pcdev, x? 1: 0)) < 0) {
		devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
		fprintf(devtty,"Cannot open floppy disk number %d.\n",fpcnt);
		fprintf(devtty, x? flopwerr: floprerr);
		goto again;
	}
	devtty = fopen("/dev/tty", "r+");	/* devtty is fd 0 */
	fprintf(devtty,"Transfer in progress - Do not remove the floppy disk.\n");
	fclose(devtty);
	if (xflag){
		if (x==Input)
			read(Input, pcbuf, CPIOBSZ);
		else
			write(Output, pcbuf, CPIOBSZ);
	}
	return(f);
}
missdir(namep)
register char *namep;
{
	register char *np;
	register ct = 2;

	for(np = namep; *np; ++np)
		if(*np == '/') {
			if(np == namep) continue;	/* skip over 'root slash' */
			*np = '\0';
			if(stat(namep, &Xstatb) == -1) {
				if(Dir) {
					if((ct = makdir(namep)) != 0) {
						*np = '/';
						return(ct);
					}
				}else {
					fprintf(stderr,"missing 'd' option\n");
					return(-1);
				}
			}
			*np = '/';
		}
	if (ct == 2) ct = 0;		/* the file already exists */
	return ct;
}

pwd()		/* get working directory */
{
	FILE *dir;

	dir = popen("pwd", "r");
	fgets(Fullname, 256, dir);
	if(pclose(dir))
		exit(2);
	Pathend = strlen(Fullname);
	Fullname[Pathend - 1] = '/';
}

#ifdef VALID_O
upchead()
{
	struct vhb *v;

	v=(struct vhb *)pcbuf;
	v->magic=VHBMAGIC;
	v->chksum=0x9d70a0d4;	
	strcpy( v->name, "Floppy");
	v->cyls=40*256;
        v->heads=2*256;       
        v->psectrk=8*256;     
        v->pseccyl=16*256;     
        v->flags=8;       
        v->step=0;        
        v->sectorsz=2;    	/* really 512 -- 512/512 = 2 */

	v->fill[7]=1;		/* Set necessary parameters to */
	v->fill[754]=035;	/* something that works   */
	v->fill[755]=025;	/*	"     "     "     */
	v->fill[756]=0301;	/*	"     "     "     */
	v->fill[757]=0161;	/*	"     "     "     */

	write(1, &v, sizeof(v));
}
#endif /* VALID_O */

wxyz()
{
	char	fdev[2];

	fdev[0] = 0;
	fdev[1] = 0;

	strcpy(pcdev, "/dev/rdsk/f");
	if(devflag){
		fdev[0]=devflag;
		strcat(pcdev, fdev);
	}
	else
		strcat(pcdev, "0");

	/* if (wflag) device /dev/rdsk/f0 (or f1, f2...f5) is correct */

	if (xflag)
		strcat(pcdev, "d8dt");
	if (yflag)
		strcat(pcdev, "d9d");
	if (zflag)
		strcat(pcdev, "q15d");
}
