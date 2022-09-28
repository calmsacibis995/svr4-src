/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)32ld:32ld.c	2.23.3.1"

/*
 *	MAC-32 (down)loader - release 2.0 software specific
 */


/* 
 * Normally, use the new, fast relocator.  (3B5's get the dregs...)
 * But users can compile with -DNORELOCATOR to use the old m32ld.
 * NOTE: We now look for m32ld in $DMD/bin, not $DMDSGS/<type>/bin
 */
#ifndef NORELOCATOR
#define RELOCATOR	"/bin/32reloc"
#else
#define M32LD		"/bin/m32ld"
#endif

/*
 * swapw	words must be swapped between host and MAC-32
 * swapb	bytes must be swapped between host and MAC-32
 */
#ifdef	pdp11
#define	swapb	1
#define	swapw	0
#endif
#ifdef	vax
#define	swapb	1
#define	swapw	1
#endif
#ifdef	u3b
#define	swapb	0
#define	swapw	0
#endif
#ifdef	u3b2
#define	swapb	0
#define	swapw	0
#endif
#ifdef	u3b15
#define	swapb	0
#define	swapw	0
#endif

#include <fcntl.h>
#include <sys/termio.h>
#include "a.out.h"
#include <stdio.h>
#include <errno.h>
#include <sys/jioctl.h>
#include "proto.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


#define	MAXRETRIES	10
#define	DATASIZE	512

#define NSECTS  12

#define MPX_VER 0x5620
#define FBOMAGIC 0560
#define SENDTERMID "\033[c"
#define TERM_1_0 "\033[?8;7;1c"
#define TERMB_1_0 "\033[?8;7;2c"
#define TERM_DMD "\033[?8;"
#define TERMIDSIZE 9
#define STR_EQUAL 0
#define ENC_CHK "\033[F"
#define	ENC_LEN	4

#define BINARY_LOAD	0
#define HEX_LOAD	1
#define	ENC_ENABLE	2		/* Value that enables terminal encoding */

char *getenv(), *strcpy(), *malloc();
#ifdef SVR32
extern int open();
#endif /* SVR32 */
int access();


char	scr_buf[30];
char	termid[TERMIDSIZE+1];
char	Load_str[] = "\033[0;0v";  /* default download (binary & standalone*/
int	Layerflag = 0;
int lpindex;	/* for getting rom version */
int count;	/* for getting rom version */
int Loadtype = BINARY_LOAD;

char	Usage[]		= "Usage: %s [-d] [-p] objectfile";
/* also [-v, -l, -z, -s size] flags, but the user never sees these */

char *name;
struct stat	Statbuf, *Statptr;
struct filehdr fileheader;
struct aouthdr aoutheader;
struct scnhdr secthdrs[NSECTS];
char	*errname;	/* name of error file for relocator */

struct termio	ttysave,/* save the state of tty */
		ttyraw;
int	obj;		/* File descriptor for object file */
long	location;
char	file[128];	/* Name of file */
char	reloc[128];	/* path name to reloc command */
int	nargchars;	/* Number of characters, including nulls, in args */
long	longbuf[3];
int	debug;		/* Show sizes etc. on stderr */
int	psflag;		/* Print error detection statistics */
short	maxpktdsize;
int	zflag;		/* Do a JZOMBOOT */
int	vflag;		/* is firmware version known? */
int	booted;
int	errfile;
int	retries;
int	stkflag;	/* changing stack size -- value is new size */
char	romversion;	/* last digit of rom version id string */


char	*cp,*dcp;
int	forbgs;	
int	mychan;		/* xt channel number download being done on */
char	cntlf[] = "/dev/xt/000";
int	cntlfd;			/* FD for xt control channel */

short speeds[16]={
	 1,	5,	7,	10,	13,	15,	20,	30,
	60,	120,	180,	240,	480,	960,	1920,	1
};

unsigned char sizes[16]={
	 16,	16,	16,	16,	16,	16,	16,	16,
	 16,	32,	32,	56,	56,	120,	120,	16
};

void	Psend();
void	Precv();
void doswab();	/* swap bytes no matter what */

extern int   optind;
extern char *optarg;
extern int   errno;
extern char *getenv();

timeout_id()
{
	error(0, "Error: Invalid terminal type:\n\t not a DMD terminal or DMD failed to respond",
		(char *) 0);
}

main(argc, argv)
int argc;
register char *argv[];
{
	register int retval;
	char    *dwnldflag;

	/* Start out by checking that download is going */
	/* to a DMD with at least 1.1 firmware (not 1.0) */
	for (lpindex=0; lpindex<=TERMIDSIZE; lpindex++)		/* should move this down */
		termid[lpindex] = 0;

	(void)ioctl(1, TCGETA, &ttysave); /* get the current state */

	name = *argv;
	while ((retval = getopt (argc, argv, "deplv:")) != EOF) {
		switch ( retval ) {
		case 'd':
			debug++;
			break;
		case 'e':
			fprintf(stderr,"-e flag no longer used: always assumed\n");
			break;
		case 'p':
			psflag++;
			break;
/*
		case 'z':
			zflag++;
			break;
*/
		case 'l':
			Layerflag++;
			break;
/*
		case 's':
			if ((stkflag = strtol(optarg, (char *)0 ,0)) == 0) {
				fprintf(stderr,"%s: Illegal stack size: %s",
				        name, optarg);
				error(0, Usage, name);
				return 1;
			}
			break;
*/
		case 'v':
			vflag++;
			romversion = optarg[0];
			Loadtype = optarg[1] - '0';
			break;
		default:
			error(0, Usage, name);
			exit(1);
		}
	}

	if (optind != argc-1) {
		fprintf(stderr,Usage, name);
		exit(1);
	}

	Load_str[4] = Loadtype + '0';
	
/* This uses the obsolete sgtty ioctls...how do we replace them? */
#ifdef SAFE
	(void)ioctl(1, TIOCEXCL, 0);
#endif

	ttyraw.c_iflag = IGNBRK;
	ttyraw.c_cflag = (ttysave.c_cflag & CBAUD) | (ttysave.c_cflag & CLOCAL) | CS8 | CREAD;
	ttyraw.c_cc[VMIN] = 1;
/*	(void)ioctl(0, TCFLSH, (struct termio *)0); /* testing...*/
	(void)ioctl(1, TCSETA, &ttyraw);
/*	(void)ioctl(1, TCSETAW, &ttyraw); */
	if ( ioctl(1, JMPX, 0) != -1 ) {
		error(0,"Generic Windowing does not support layers downloads","");
	}
	errno = 0;

	/* identify ROM version */
	if (!vflag) {
		getromvers();
		if( stkflag)
			error(0,"'-s' flag supported in layers only.");

	}
	if( (romversion < '5') && (stkflag != 0) )
		error(0, "Error: Stacksize not supported by your terminal firmware.\n", (char *)0);

	if( !vflag ) {		/* check encoding */

		/* NOTE: version 8;7;5 roms may not properly check
		   for encoding from the keyboard */

		if( romversion >= '5' ) {
			write(1,ENC_CHK,strlen(ENC_CHK));
			count = 0;
			while(count < ENC_LEN){
				lpindex = read(0,&scr_buf[count],ENC_LEN);
				if(lpindex > 0)count += lpindex;
			}
			if( scr_buf[2] == '1' ) {
				Loadtype = ENC_ENABLE;
			}
			else {
			    if (((dwnldflag = getenv("DMDLOAD")) != NULL) && (dwnldflag[0] != NULL)) {
				if(strcmp(dwnldflag, "hex") == 0)
				    Loadtype = ENC_ENABLE;
			    }
			}
		} else
			if(((dwnldflag = getenv("DMDLOAD")) != NULL) &&
					(dwnldflag[0] != NULL)) {
			if(strcmp(dwnldflag, "hex") == 0)
				Loadtype = HEX_LOAD;
		}
	}
	if (Loadtype != BINARY_LOAD) {
		Load_str[4] = Loadtype + '0';
		ttyraw.c_iflag |= IXON;
		(void)ioctl(1, TCSETAW, &ttyraw);
	}
	if(jpath(argv[optind], access, 4)!=0)
		error(1, "no such file '%s'", argv[optind]);

	/* check for an empty file before download */
	Statptr= &Statbuf;
	stat(argv[optind], Statptr);
	if(Layerflag && (Statbuf.st_size == 0)) {
		Load_str[2] = '2';
		write(1,Load_str, 6);
		goto cleanup;
	}

	obj=jpath(argv[optind], open, 0);
	if(obj<0)
		error(1, "cannot open '%s'", file);
/********************************************************/
/*							*/
/*	reads the headers for the m32a.out		*/
/*	file and stores the data read into the global	*/
/*	structures declared for this purpose		*/
/*							*/
/********************************************************/

	Read (&fileheader, sizeof(struct filehdr));
	if(fileheader.f_magic!=FBOMAGIC)	/* FBOMAGIC is 0560 */
		error(0, "'%s' is not a WE-32000 family a.out", file);
	Read (&aoutheader, fileheader.f_opthdr);

	if (fileheader.f_nscns > NSECTS)
		error(0,"%s: exceeded max number of sections -- see system administrator", name);
	if(aoutheader.vstamp==MPX_VER)	/* MPX_VER is 0x5620 */
		error(0, "'%s' compiled for layers", file);
	
	/* does this a.out have empty text/data/bss sections? */
	if(((aoutheader.tsize + aoutheader.dsize + aoutheader.bsize) == 0) && Layerflag) {
		if (debug)
			fprintf(stderr,"Downloading type 2 (no patch)\n");
		Load_str[2] = '2';
		write(1,Load_str, 6);
		goto cleanup;
	}
	if(Layerflag) {
		if (debug)
			fprintf(stderr,"Downloading type 1 (a patch)\n");
		/*
		* if it got here then we want to download layers with a patch
		*/
		Load_str[2] = '1';
	}

/*	signal(SIGHUP,error);
	signal(SIGINT,error);	Doing this tends to be more buggy than it's worth
	signal(SIGQUIT,error); 	This has been taken out several times before! */

	boot();
	maxpktdsize = min(sizes[ttysave.c_cflag&CBAUD], (long)MAXPKTDSIZE);
	pinit(speeds[ttysave.c_cflag&CBAUD], maxpktdsize, ACKON);

	load(/*argv[1],*/ optarg, argv);
	buzz();
	(void)ioctl(0, TCFLSH, (struct termio *)0);

/* This uses the obsolete sgtty ioctls...how do we replace them? */
#ifdef SAFE
	(void)ioctl(1, TIOCNXCL, 0);
#endif
cleanup:
/*	(void)ioctl(1, TCSETA, &ttysave);*/
#ifdef u3b
	sleep(1);				/* layers -f problem */
#endif
	(void)ioctl(1, TCSETAW, &ttysave);
	if(psflag)
		pstats(stderr);
	return(0);
}


char *
bldargs(argc, argv)
	char *argv[];
{
	register i;
	register char *argp, *p, *q;
	for(nargchars=0, i=0; i<argc; i++)
		nargchars+=strlen(argv[i])+1;
	if((argp=malloc(nargchars))==0)
		error("can't allocate argument chars", "");
	/* this loop is probably not necessary, but it's safe */
	for(i=0, q=argp; i<argc; i++){
		p=argv[i];
		do; while(*q++ = *p++);
	}
	return argp;
}


load(/*f,*/ argc, argv)
/*	char *f;*/
	char *argv[];
{
	char *argp;
	long largc;
	int i;
	location = aoutheader.entry;
	for (i = 0; i < (int)fileheader.f_nscns; ++i)     /* read section header array */
		Read (&secthdrs[i], sizeof(struct scnhdr));

	if(debug){
		fprintf(stderr, "%s:\nSection:\taddr:\tsize:\n", file);
		for ( i = 0; i < (int)fileheader.f_nscns; ++i)
			fprintf(stderr,"%s\t\t0x%lx\t0x%lx\n",
			secthdrs[i].s_name,secthdrs[i].s_paddr,secthdrs[i].s_size);
		buzz();
	}
	sendfile();
	{
		long	startaddr;

		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
		location = aoutheader.entry;
		swaw(&location, &startaddr, PKTASIZE);
		psend((char *)&startaddr, PKTASIZE);
		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
	}
}

jpath(f, fn, a)
	register char *f;
	register int (*fn)();
{
	char *getenv(), *strcpy();
	register char *jp, *p;
	register o;
	if (*f != '/' && strncmp(f, "./", 2) && strncmp(f, "../", 3) && 
	    (jp=getenv("JPATH"))!=0){
		while(*jp){
			for(p=file; *jp && *jp!=':'; p++,jp++)
				*p= *jp;
			if(p!=file)
				*p++='/';
			if(*jp)
				jp++;
			(void)strcpy(p, f);
			if((o=(*fn)(file, a))!=-1)
				return o;
		}
	}
	return((*fn)(strcpy(file, f), a));
}

error(pflag, s1, s2)
	char *s1, *s2;
{
	long flushval = 0L;
	register int	n;
	register int	saverrno;
	char		buf[BUFSIZ];
	extern int	errno;

	saverrno = errno;
	if(booted){
		psend((char *)(&flushval),sizeof(long));	
		if(errfile>0){
			buzz();
			while((n=read(errfile, buf, sizeof buf))>0)
				write(2, buf, n);
			unlink(errname);
		}
	}
#ifdef SAFE
	(void)ioctl(1, TIOCNXCL, 0);
#endif
	(void)ioctl(1, TCSETAW, &ttysave);
	if(pflag){
		errno=saverrno;
		perror(s2);
	}
	fprintf(stderr, "\n%s: ", name);
	fprintf(stderr, s1, s2);
	fprintf(stderr, "\n");
	if(psflag)
		pstats(stderr);
	exit(1);
}
int
Read(a, n)
	char *a;
{
	register i;
	i=read(obj, a, n);
	if(i<0)
		error(1, "read error on '%s'", file);
	return(i);
}
void
Write(a, n)
	char *a;
{
	if(realwrite(a, n)!=n)
		error(1, "write error to DMD", (char *)0);
	if(psflag )
		trace(a);
}
writeswap(a, n)
	char *a;
{
	char buf1[DATASIZE+PKTASIZE], buf2[DATASIZE+PKTASIZE];
	swaw(a, buf1, n);
	swab(buf1, buf2, n);
	Write(buf2, n);
}
trace(a)
	char *a;
{
	register int	i;

	for(i=0; i<(PKTHDRSIZE+PKTASIZE); i++)
		fprintf(stderr, "<%o>", a[i]&0xff);
	fprintf(stderr, "\n");
}

sendfile()
{
	register int i;
	for ( i = 0; i < (int)fileheader.f_nscns; ++i) {
		if(secthdrs[i].s_scnptr > 0)  {
			if ((secthdrs[i].s_flags & STYP_NOLOAD) ||
			    (secthdrs[i].s_flags & STYP_DSECT))
				continue;
			lseek(obj,secthdrs[i].s_scnptr,0);
			sendseg(secthdrs[i].s_paddr,secthdrs[i].s_paddr+secthdrs[i].s_size);
		}
	}
}

sendseg(strloc,endloc)
long strloc;
long endloc;
{
	char buf[DATASIZE+PKTASIZE], buf2[DATASIZE];
	char tmpbuf[DATASIZE+PKTASIZE];
	register n;
	while((n=Read(&buf[PKTASIZE], min(maxpktdsize, endloc-strloc)))>0){
/*local swab?*/	swab(&buf[PKTASIZE], &tmpbuf[PKTASIZE], n);
		swaw((short *)&strloc, (short *)&tmpbuf[0], PKTASIZE);
		Psend(tmpbuf, n+PKTASIZE);
		strloc+=n;
	}
}
void
Psend(bufp, count)
	char *bufp;
	int count;
{
	retries = 0;
	while(freepkts == 0)
		Precv();
	psend(bufp, count);
}
void
Precv()
{
	char c;

	alarm(3);		/* sleep at least 2 seconds */
	if(realread(&c, 1) == 1){
		alarm(0);
		if(psflag)
			fprintf(stderr, "recv <%o>\n", c&0xff);
		precv(c);
	}else if(errno != EINTR )
		error(1, "read error", (char *)0);
	else if(++retries >= MAXRETRIES)
		error(0, "load protocol failed", (char *)0);
	else if(psflag)
			fprintf(stderr, "recv timeout.. retries=%d\n",retries);
}

min(a, b)
	long b;	/* not your average min() */
{
	return(a<b? a : (int)b);
}

/* why duplicate a routine in the library? Because it seems to be needed */
/* bgs: changed the name, because I need the REAL swab()		 */
swab(a, b, n)
	register char *a, *b;
	register n;
{
#	if(swapb)
	register char *s, *t;
	n/=2;	/* n in bytes */
	s=b+1;
	t=b;
	while(n--){
		*s= *a++;
		*t= *a++;
		s+=2;
		t+=2;
	}
#	else
	while(n--)
		*b++= *a++;
#	endif
}

swaw(a, b, n)
	register short *a, *b;
	register n;
{
#	if(swapw)
	register short *s, *t;
	n/=4;	/* n in bytes */
	s=b+1;
	t=b;
	while(n--){
		*s= *a++;
		*t= *a++;
		s+=2;
		t+=2;
	}
#	else
	n>>=1;
	while(n--)
		*b++= *a++;
#	endif
}

/*
 * Swap bytes in 16-bit [half-]words
 * Historical...should be merged with the other swab() above
 * NOTE: the reason it isn't is because other places call the swab above
 * e.g. proto.c, etc.  The only difference here is that it always does it.
 */

void
doswab(pf, pt, n)
register short *pf, *pt;
register int n;
{
	n /= 2;
	while(--n >= 0) {
		*pt++ = (*pf << 8) + ((*pf >> 8) & 0377);
		pf++;
	}
}


boot(){
	char c = 0;

	write(1, Load_str,6);	/* esc sequence for download */
	while(c != 'a')
		read(0, &c, 1);	/* wait for terminal to be ready */
	booted++;
	return 0;
}

buzz(){
	/* sleep for a time >~0.5 sec; nice if we had nap! */
	sleep(2);	/* sleep(1) not necessarily long enough */
}
getdmd(str)
char *str;
{
	strcpy(str,getenv("DMD"));
	if (str[0] == '\0')
	{
		fprintf(stderr,"Must have DMD set in your environment\n");
		return(1);
	}
	return(0);
}

getdmdsgs(str)
char *str;
{
	strcpy(str,getenv("DMDSGS"));
	if (str[0] == '\0')
	{
		fprintf(stderr,"Must have DMDSGS set in your environment\n");
		fprintf(stderr,"DMDSGS is the root directory of the DMD sgs\n");
		return(1);
	}
	return(0);
}

/*
* This routine is the lowest level write routine to the dmd.  It provides a 
* simple way to implement a safer download protocol through networks. 
* This requires that a shell varariable will be set if this extra precaution
* is to be taken.
*/

realwrite(a,n)
char *a;
{
	char cbuf[(MAXPKTSIZE + PKTASIZE) * 2], c;
	int i, j, maxsize;

	if(Loadtype == BINARY_LOAD) {
		return(write(1,a,n));
	}
	else {
		/*
		* do a hex load
		*/
		j = n;
		maxsize = ((MAXPKTSIZE + PKTASIZE) / 2);
		for(i = 0;i < n*2; i++){
			c = *a++;
			cbuf[i]=(c & 0xf) | 0x40;
			cbuf[++i]=((c >> 4) & 0xf) | 0x40;
		}
		i = 0;
		while(n > 0) {
			if(n > maxsize) {
				if(write(1, &cbuf[i*maxsize*2], maxsize*2) != maxsize*2)
					return(-1);
				n -= maxsize;
				i++;
			}
			else {
				if(write(1, &cbuf[i*maxsize*2], n*2) != n*2)
					return(-1);
				n=0;	/* last buffer so don't loop anymore */
			}		
		}
		return(j); /* all correct so return number of actual characters sent */
	}
}

realread(a, n)
char *a;
{
	char	cbuf[2];
	int 	i;

	if(Loadtype == BINARY_LOAD) {
		return(read(0, a, n));
	}
	else {
		for(i = 0; i < n ; i++) {
			if(read(0, cbuf, 2) != 2)
				return(-1);
			*a++ = (cbuf[0] & 0xf) | ((cbuf[1] & 0xf) << 4);
		}
		return(n);
	}
}

/*	Had to get the rom version the old way (by escape sequence).
 *	Either pre-2.0 roms or a standalone download without the -v option
 */
	
getromvers() 
{
		write(1,SENDTERMID,strlen(SENDTERMID));
		count = 0;
		while(count < TERMIDSIZE){
			lpindex = read(0,&termid[count],TERMIDSIZE);
			if(lpindex > 0)count += lpindex;
		}
		if ((strcmp(termid,TERM_1_0) == STR_EQUAL) ||	/* equal strings */
			(strcmp(termid,TERMB_1_0) == STR_EQUAL))
			error(0,"Error: Firmware not updated to 1.1 or greater\n",
				(char *) 0);
		if (strncmp(termid,TERM_DMD,strlen(TERM_DMD)) != STR_EQUAL)
			error(0, "Error: %s must be run on a DMD terminal\n",
				(char *) name);
		romversion = termid[strlen(termid)-2];
		if (debug) {
			fprintf(stderr,"ROM version (by escape seq) is %s\n",termid);
		}
}
