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

#ident	"@(#)scsi.in:test/sw01qa/sw01qa.c	1.3"

/*
 *	SCSI WORM TARGET DRIVER TEST COMMAND
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "sys/worm.h"

struct command {
	char	*cmd;
	int	(*func)();
};


int	wmtestunit(),wmreqsense(),wmmodsense(),wmstrtunit(),wmstopunit();
int	wmrezeunit(),wminquiry(),wmreadcapa();
int	wmprevmv(),wmallomv(),wmseek(),wmstncheck(),wmreadcb();
int	wmreadt(),wmwritet(),wmloadcart(),wmunloadca();
int	wmcheck(),wmccheck(),wmverify();
int	shell(),menuprint();
int	quit();
int	wmtestdev,wmdevflag;

struct command command[] = {
/* 36 */ "wmtestunit",	wmtestunit,
/* 36 */ "wmrezeunit",	wmrezeunit,
/* 37 */ "wmreqsense",	wmreqsense,
/* 37 */ "wminquiry",	wminquiry,
/* 38 */ "wmmodsense",	wmmodsense,
/* 39 */ "wmstrtunit",	wmstrtunit,
/* 40 */ "wmstopunit",	wmstopunit,
/* 41 */ "wmprevmv",	wmprevmv,
/* 42 */ "wmallomv",	wmallomv,
/* 42 */ "wmreadcapa",	wmreadcapa,
/* 43 */ "wmseek",	wmseek,
/* 43 */ "wmread",	wmreadt,
/* 43 */ "wmwrite",	wmwritet,
/* 44 */ "wmstncheck",	wmstncheck,
/* 44 */ "wmloadcart",	wmloadcart,
/* 44 */ "wmunloadca",	wmunloadca,
/* 45 */ "wmreadcb",	wmreadcb,
/* 46 */ "wmcheck",	wmcheck,
/* 47 */ "wmccheck",	wmccheck,
/* 49 */ "wmverify",	wmverify,

/* 97 */ "!",		shell,
/* 99 */ "?",		menuprint,
/* 99 */ "q",		quit,
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
wmtestunit()
{
	if(ioctl(scfd,W_TESTUNIT,0) < 0)
		perror("TESTUNIT");
}

wmrezeunit()
{
	if(ioctl(scfd,W_REZERO,0) < 0)
		perror("REZEROUNIT");
}

wmreqsense()
{
	printf("Not supported\n");
}

wminquiry()
{
	int	dev;
	struct worm_inq inq;

	inq.length = 126;
	inq.addr = (char *)buf;
	if(ioctl(scfd,W_INQUIR,&inq) < 0){
		perror("INQUIRY");
		return(-1);
	}
	fprintf(stdout,"periferal device type = %x\n",buf[0]);
	fprintf(stdout,"RMB & device version = %x\n",buf[1]);
	fprintf(stdout,"ISO & ECMA & ANSI version = %x\n",buf[2]);
	fprintf(stdout,"response data format = %x\n",buf[3]);
	fprintf(stdout,"additional length = %x\n",buf[4]);
	printent("vendor identification",8,15,buf,1);
	printent("product identification",16,31,buf,1);
	printent("revision level",32,35,buf,1);
	printent("SCSI firmware date",36,43,buf,1);
}

wmmodsense()
{
	printf("Not supported\n");
}

wmstrtunit()
{
	if(ioctl(scfd,W_STRTUNIT,0) < 0)
		perror("W_STRTUNIT");
}

wmstopunit()
{
	if(ioctl(scfd,W_STOPUNIT,0) < 0)
		perror("W_STOPUNIT");
}

wmprevmv()
{
	if(ioctl(scfd,W_PREVMV,0) < 0)
		perror("W_PREVMV");
}

wmallomv()
{
	if(ioctl(scfd,W_ALLOMV,0) < 0)
		perror("W_ALLOMV");
}

wmreadcapa()
{
	struct	worm_capacity	cap;

	if(ioctl(scfd,W_READCAPA,&cap) < 0){
		perror("READCAPA");
		return(-1);
	}
	printf("logical block address = %x\n",cap.addr);
	printf("block length = %x\n",cap.len);
}

wmseek()
{
	int	dev,add;

	add = getparam("address");
	if(ioctl(scfd,W_SEEK,add) < 0)
		perror("SEEK");
}

wmreadt()
{
	int	fd,dev,i,p1,p2,p3;
	int	count,bs;

	p2 = getparam("block number");
	p3 = getparam("read size (block)");
	p1 = getparam("I/O size (block)");
	bs = getparam("block size (0x400)");
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

wmwritet()
{
	int	dev,i,p1,p2,p3,fd;
	int	rc,wc;

	fd = getfile("filename ");
	if(fd < 0)
		return(-1);
	p2 = getparam("block number");
	p3 = getparam("write size (block)");
	p1 = getparam("I/O size (block)");
	if(lseek(scfd,p2*1024,0) < 0){
		perror("lseek");
		return(-1);
	}
	for( i = 0 ; i < p3/p1 ; ++i){
		if((rc = read(fd,buf,p1*1024)) < 0){
			perror("read");
			return(-1);
		}
		if((wc = write(scfd,buf,p1*1024)) < 0){
			perror("write");
			return(-1);
		}
		printf("%d:read count=%d, write count=%d\n",i,rc,wc);
	}
	close(fd);
}
wmstncheck()
{
	int	dev,param[3];

	if(ioctl(scfd,W_STNCHECK,buf) < 0){
		perror("WMSTNCHECK");
		return(-1);
	}
	switch(buf[3]){
	case	0:
		printf("no cartridge\n");
		break;
	case	1:
		printf("cartridge exist\n");
		break;
	case	2:
		printf("cartridge set\n");
		break;
	case	3:
		printf("cartridge error\n");
		break;
	default:
		printf("invalid code \n");
		break;
	}
	cddump(buf,4,0);
}

wmloadcart()
{
	if(ioctl(scfd,W_LOADCART,0) < 0)
		perror("W_LOADCART");
}

wmunloadca()
{
	if(ioctl(scfd,W_UNLOADCA,0) < 0)
		perror("W_UNLOADCA");
}

wmreadcb()
{
	if(ioctl(scfd,W_READCB,buf) < 0){
		perror("READCB");
		return(-1);
	}
	cddump(buf,1024,0);
}

wmcheck()
{
	int	dev,block,size;
	struct worm_check ck;

	ck.start = getparam("block");
	ck.num = getparam("size");
	if(ioctl(scfd,W_CHECK,&ck) < 0){
		perror("CHECK");
		return(-1);
	}
	printf("block = %x , result = %x\n",ck.block,ck.result);
}

wmccheck()
{
	int	dev,block,size;
	struct worm_check ck;

	ck.start = getparam("block");
	ck.num = getparam("size");
	if(ioctl(scfd,W_CCHECK,&ck) < 0){
		perror("CCHECK");
		return(-1);
	}
	printf("block = %x , result = %x\n",ck.block,ck.result);
}

wmverify()
{
	int	dev,block,size;
	struct worm_verify vy;

	vy.start = getparam("block");
	vy.num = getparam("size");
	if(ioctl(scfd,W_VERIFY,&vy) < 0)
		perror("VERIFY");
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
	fprintf(stderr,"\n\n------------- WORM menu -------------------------------------------------\n");
	fprintf(stderr,"wmtestunit\t");
	fprintf(stderr,"wmrezeunit\t");
	fprintf(stderr,"wmreqsense\t");
	fprintf(stderr,"wminquiry \t");
	fprintf(stderr,"wmmodsense\n");
	fprintf(stderr,"wmstrtunit\t");
	fprintf(stderr,"wmstopunit\t");
	fprintf(stderr,"wmprevmv  \t");
	fprintf(stderr,"wmallomv  \t");
	fprintf(stderr,"wmreadcapa\n");
	fprintf(stderr,"wmseek    \t");
	fprintf(stderr,"wmread    \t");
	fprintf(stderr,"wmwrite   \t");
	fprintf(stderr,"wmstncheck\t");
	fprintf(stderr,"wmreadcb  \n");
	fprintf(stderr,"wmcheck   \t");
	fprintf(stderr,"wmccheck  \t");
	fprintf(stderr,"wmread    \t");
	fprintf(stderr,"wmloadcart\t");
	fprintf(stderr,"wmunloadca\n");
	fprintf(stderr,"wmwritever\t");
	fprintf(stderr,"wmverify  \t");

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
		fprintf(stdout,"	%s = %x\n",s,n);
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
				printf("%x  ",*dp++);
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

quit(){
	exit(0);
}
