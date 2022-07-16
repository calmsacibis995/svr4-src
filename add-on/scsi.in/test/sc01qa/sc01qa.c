/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*		All Rights Reserved			*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*		All Rights Reserved			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF		*/
/*	TOSHIBA CORPORATION and SORD COMPUTER CORPORATION	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi.in:test/sc01qa/sc01qa.c	1.3"

/*
 *	SCSI CD-ROM TARGET DRIVER TEST COMMAND
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "sys/cdrom.h"

struct command {
	char	*cmd;
	int	(*func)();
};
struct cdrom_audio	audio;
char		bcdbuf[2];
int		bcdret;

int	cdtestunit(),cdreqsense(),cdmodsense();
int	cdstrtunit(),cdstopunit();
int	cdrezeunit(),cdinquiry(),cdreadcapa();
int	cdprevmv(),cdallomv(),cdseek();
int	cdreadt(),cdtrayopen(),cdtrayclose();
int	cdsearch(),cdaudio(),cdstill();
int	cdtestdev,cddevflag;
int	shell(),menuprint();
int	quit();

struct command command[] = {
	"cdtestunit",	cdtestunit,
	"cdrezeunit",	cdrezeunit,
	"cdreqsense",	cdreqsense,
	"cdinquiry",	cdinquiry,
	"cdmodsense",	cdmodsense,
	"cdstrtunit",	cdstrtunit,
	"cdstopunit",	cdstopunit,
	"cdprevmv",	cdprevmv,
	"cdallomv",	cdallomv,
	"cdreadcapa",	cdreadcapa,
	"cdseek",	cdseek,
	"cdread",	cdreadt,
	"cdsearch",	cdsearch,
	"cdaudio",	cdaudio,
	"cdstill",	cdstill,
	"cdtrayopen",	cdtrayopen,
	"cdtrayclose",	cdtrayclose,

	"!",		shell,
	"?",		menuprint,
	"q",		quit,
};

char    cmd[10];
char	shcmd[100];
unsigned char	buf[64*1024];
char	*mesg;
int     add,data;
int	flag;
int	scfd;		/* SCSI device */
int	scfduse;
jmp_buf	jump1;
jmp_buf	jump2;

main(argc,argv)
char    **argv;
{
        int     n,i;

	if(argc != 2){
		fprintf(stderr,"usage : %s devicename\n",argv[0]);
		exit(1);
	}
	if((scfd = open(argv[1],2)) < 0){
		if((scfd = open(argv[1],0)) < 0){
			perror("open ");
			exit(1);
		}
	}
	scfduse++;
	setjmp(jump1);
	/* if user driver, release I/O port */
	menuprint(0);
	for(;;){
		fprintf(stderr,"\nEnter command ---> ");
		if((n = scanf("%s",cmd)) <= 0)
			break;
		fprintf(stderr,"%s command executing\n",cmd);
		for( i = 0 ; i < sizeof(command)/sizeof(struct command) ; ++i){
			if(strcmp(command[i].cmd,cmd) == 0){
				(*command[i].func)(1);
				break;
			}
		}
		if( i == sizeof(command)/sizeof(struct command))
			fprintf(stderr,"%s : unknown command\n",cmd);
        }
        exit(0);
}

/*
 *	OPTICAL DISK Ioctl
 */
cdtestunit()
{
	if(ioctl(scfd,C_TESTUNIT,0) < 0)
		perror("TESTUNIT");
}

cdrezeunit()
{
	if(ioctl(scfd,C_REZERO,0) < 0)
		perror("REZEROUNIT");
}

cdreqsense()
{
	printf("Not supported\n");
}

cdinquiry()
{
	int	dev;
	struct cdrom_inq inq;

	inq.length = 98;
	inq.addr = (char *)buf;
	if(ioctl(scfd,C_INQUIR,&inq) < 0){
		perror("INQUIRY");
		return(-1);
	}
	fprintf(stdout,"periferal device type(0x5) = 0x%x\n",buf[0]);
	fprintf(stdout,"RMB & device version(0x80) = 0x%x\n",buf[1]);
	fprintf(stdout,"ISO & ECMA & ANSI version(0x01) = 0x%x\n",buf[2]);
	fprintf(stdout,"response data format(0x01) = 0x%x\n",buf[3]);
	fprintf(stdout,"additional length(0x5d) = 0x%x\n",buf[4]);
	printent("vendor identification",8,15,buf,1);
	printent("product identification",16,31,buf,1);
	printent("revision level",32,35,buf,1);
	printent("SCSI firmware date",36,43,buf,1);
}

cdmodsense()
{
	printf("Not supported\n");
}

cdstrtunit()
{
	if(ioctl(scfd,C_STARTUNIT,0) < 0)
		perror("C_STARTUNIT");
}

cdstopunit()
{
	if(ioctl(scfd,C_STOPUNIT,0) < 0)
		perror("C_STOPUNIT");
}

cdprevmv()
{
	if(ioctl(scfd,C_PREVMV,0) < 0)
		perror("C_PREVMV");
}

cdallomv()
{
	if(ioctl(scfd,C_ALLOMV,0) < 0)
		perror("C_ALLOMV");
}

cdreadcapa()
{
	struct	cdrom_capacity	cap;

	if(ioctl(scfd,C_READCAPA,&cap) < 0){
		perror("READCAPA");
		return(-1);
	}
	printf("logical block address = 0x%x\n",cap.addr);
	printf("block length = 0x%x (%d d)\n",cap.len, cap.len);
}

cdseek()
{
	int	dev,add;

	add = getparam("address");
	if(ioctl(scfd,C_SEEK,add) < 0)
		perror("SEEK");
}

cdreadt()
{
	int	fd,dev,i,p1,p2,p3;
	int	count,bs;

	p2 = getparam("block number");
	p3 = getparam("read size (block)");
	p1 = getparam("I/O size (block)");
	bs = getparam("block size (0x800)");
	fd = creatfile("filename ");
	if(fd < 0)
		return(-1);
	if(lseek(scfd,p2*bs,0) < 0){
		perror("lseek");
		return(-1);
	}
	for( i = 0 ; i < p3/p1 ; ++i){
		if((count = read(scfd,buf,p1*bs)) < 0){
			perror("read");
			return(-1);
		}
		if(write(fd,buf,count) < 0){
			perror("write");
			return(-1);
		}
		fprintf(stdout,"%d:read %d bytes\n",i,count);
	}
	close(fd);
}

cdsearch()
{

	audio.play = getparam("param: 1(play), 0(pause)");
	audio.type = getparam("type:  0(logical), 1(ATIME), 2(track)");
	audio.addr_logical = 0;
	if (audio.type == 0) {	/* type 0: logical block address */
		audio.addr_logical = getparam("logical block address");
	} else if (audio.type == 1) {	/* type 1: MIN/SEC/FRAME */
		audio.addr_min = itobcd(getparam("MIN (00-99)"));
		audio.addr_sec = itobcd(getparam("SEC (00-59)"));
		audio.addr_frame = itobcd(getparam("FRAME (00-74)"));
	} else if (audio.type == 2) {	/* type 2: track number */
		audio.addr_track = itobcd(getparam("track No."));
	}
	if(ioctl(scfd,C_AUDIOSEARCH,&audio) < 0){
		perror("C_AUDIOSEARCH");
		return(-1);
	}
}

cdaudio()
{
	audio.play = getparam("param: 0(muting on), 1(L), 2(R), 3(stereo)");
	audio.type = getparam("type:  0(logical), 1(ATIME), 2(track)");
	audio.addr_logical = 0;
	if (audio.type == 0) {	/* type 0: logical block address */
		audio.addr_logical = getparam("logical block address");
	} else if (audio.type == 1) {	/* type 1: MIN/SEC/FRAME */
		audio.addr_min = itobcd(getparam("MIN (00-99)"));
		audio.addr_sec = itobcd(getparam("SEC (00-59)"));
		audio.addr_frame = itobcd(getparam("FRAME (00-74)"));
	} else if (audio.type == 2) {	/* type 2: track number */
		audio.addr_track = itobcd(getparam("track No."));
	}
	if(ioctl(scfd,C_PLAYAUDIO,&audio) < 0){
		perror("C_PLAYAUDIO");
		return(-1);
	}
}

cdstill()
{
	if(ioctl(scfd,C_STILL,0) < 0){
		perror("C_STILL");
		return(-1);
	}
}

cdtrayopen()
{
	int	param;

	param = getparam("param: 0(normal), 1(immed)");
	if(ioctl(scfd,C_TRAYOPEN,param) < 0){
		perror("C_TRAYOPEN");
		return(-1);
	}
}

cdtrayclose()
{
	int	param;
	param = getparam("param: 0(normal), 1(immed)");
	if(ioctl(scfd,C_TRAYCLOSE,param) < 0){
		perror("C_TRAYCLOSE");
		return(-1);
	}
}

getparam(s)
char	*s;
{
	int	param;

	fprintf(stderr,"enter %s ---> ",s);
	scanf("%x",&param);
	return(param);
}

char	filename[20];
getfile(s)
char	*s;
{
	int	fd;

	fprintf(stderr,"enter %s ---> ",s);
	scanf("%s",filename);
	if((fd = open(filename,0)) < 0){
		fprintf(stderr,"cannot open %s",filename);
		return(-1);
	}
	return(fd);
}
creatfile(s)
char	*s;
{
	int	fd;

	fprintf(stderr,"enter %s ---> ",s);
	scanf("%s",filename);
	if((fd = creat(filename,0666)) < 0){
		fprintf(stderr,"cannot creat %s",filename);
		return(-1);
	}
	return(fd);
}
/*
 *	execute shell command
 */
shell()
{
	fprintf(stderr,"enter shell command ---> ");
	gets(shcmd);
	if(gets(shcmd))
		system(shcmd);
}

/*
 *	command menu
 */
menuprint(num)
{
	fprintf(stderr,"\n\n------------- CD-ROM menu -----------------------------------------------\n");
	fprintf(stderr,"cdtestunit\t");
	fprintf(stderr,"cdrezeunit\t");
	fprintf(stderr,"cdreqsense\t");
	fprintf(stderr,"cdinquiry \t");
	fprintf(stderr,"cdmodsense\n");
	fprintf(stderr,"cdstrtunit\t");
	fprintf(stderr,"cdstopunit\t");
	fprintf(stderr,"cdprevmv  \t");
	fprintf(stderr,"cdallomv  \t");
	fprintf(stderr,"cdreadcapa\n");
	fprintf(stderr,"cdseek    \t");
	fprintf(stderr,"cdread    \t");
	fprintf(stderr,"cdsearch\t");
	fprintf(stderr,"cdaudio   \t");
	fprintf(stderr,"cdstill\n");
	fprintf(stderr,"cdtrayopen\t");
	fprintf(stderr,"cdtrayclose\t");

	fprintf(stderr,"\n");

	fprintf(stderr,"!\t\t");
	fprintf(stderr,"?\t\t");
	fprintf(stderr,"q\t\t");

	fprintf(stderr,"\n");
}

printent(s,from,to,ptr,flag)
char	*s;
unsigned char	*ptr;
{
	unsigned char	*cp;
	int	i,n;

	switch(flag){
	case	0:
		fprintf(stdout,"	%s = ",s);
		for( i = 1,cp = ptr + from  ; cp <= ptr + to ; i++,cp++){
			fprintf(stdout,"%2x ",*cp);
			if(!(i%16))
				fprintf(stdout,"\n		");
		}
		if(i%16)
			fprintf(stdout,"\n");
		break;
	case	1:
		fprintf(stdout,"	%s = ",s);
		for( i = 1,cp = ptr + from  ; cp <= ptr + to ; i++,cp++){
			fprintf(stdout,"%c ",*cp);
			if(!(i%16))
				fprintf(stdout,"\n		");
		}
		if(i%16)
			fprintf(stdout,"\n");
		break;
	case	2:
		fprintf(stdout,"	%s = ",s);
		for( i = 1,cp = ptr + from  ; cp <= ptr + to ; i++,cp++){
			fprintf(stdout,"%2x:%c ",*cp,*cp);
			if(!(i%16))
				fprintf(stdout,"\n		");
		}
		if(i%16)
			fprintf(stdout,"\n");
		break;
	case	3:
		n = 0;
		cp = ptr + to ;
		for( i = 0 ; i < to - from +1 ; ++i,cp-- ){
			if(i > 3)
				break;
			n |= *cp << 8*i;
		}
		/***
		fprintf(stdout,"	%s = 0x%x\n",s,n);
		***/
		return(n);
		break;
	default:
		for( i = 1,cp = ptr + from  ; cp <= ptr + to ; i++,cp++){
			fprintf(stdout,"%2x:%c ",*cp,*cp);
			if(!(i%16))
				fprintf(stdout,"\n		");
		}
		break;
	}
}
dump()
{
	int	size,flag;

	size = getparam("size ");
	flag = getparam("flag ");
	cddump(buf,size,flag);
}

/**********************************************************************
 *
 *	cddump() : dump internal buf
 * 
 **********************************************************************/
cddump(dp,size,flag)
unsigned char *dp;
{
	int	i,fd;

	switch(flag){
	case	0:
	case	1:
		for( i = 1 ; i < size+1 ; ++i){
			if(flag)
				printf("%c  ",*dp++);
			else
				printf("0x%x  ",*dp++);
			if( i%10 == 0)
				printf("\n");
		}
		printf("\n");
		break;
	case	2:
		if((fd = creat("cddump",0666)) < 0){
			perror("cannot creat cddump");
			return(-1);
		}
		if(write(fd,dp,size) < 0){
			perror("cannot write cddump");
			return(-1);
		}
		break;
	}
}

itobcd(i)
int	i;
{
	if (i > 99) {
		i = 99;
	}
	sprintf(bcdbuf, "%2d", i);
	bcdret = (bcdbuf[0] << 4) & 0xf0 | bcdbuf[1] & 0x0f;
	return(bcdret);
}

/*
 * quit this test program
 */
quit() {
	exit(0);
}
