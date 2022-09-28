/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:gio.c	2.10.3.1"

#include "uucp.h"

#include "pk.h"

struct pack *Pk;
extern int gwrblk(), grdblk(), pkread(), pkwrite();
extern void pkclose();

extern int packsize, xpacksize;

jmp_buf Getjbuf, Gfailbuf;

static void (*gsig)();

/* ARGSUSED */
static void
galarm(sig)
int sig;
{
	signal(SIGALRM, galarm);
	longjmp(Getjbuf, 1);
}

void
pkfail()
{
	longjmp(Gfailbuf, 1);
}

int
gturnon()
{
	struct pack *pkopen();
	if (setjmp(Gfailbuf))
		return(FAIL);
	gsig=signal(SIGALRM, galarm);
	if (Debug > 4)
		pkdebug = 1;
	Pk = pkopen(Ifn, Ofn);
	if ((int) Pk == NULL)
		return(FAIL);
	return(0);
}

int
gturnoff()
{
	if(setjmp(Gfailbuf))
		return(FAIL);
	pkclose(Pk);
	(void) signal(SIGALRM, gsig);
	return(0);
}

int
gwrmsg(type, str, fn)
char type, *str;
{
	char bufr[BUFSIZ], *s;
	int len, i;

	if(setjmp(Gfailbuf))
		return(FAIL);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	if ((i = len % xpacksize) != 0) {
		len = len + xpacksize - i;
		bufr[len - 1] = '\0';
	}
	gwrblk(bufr, len, fn);
	return(0);
}


/*ARGSUSED*/
int
grdmsg(str, fn)
char *str;
{
	unsigned len;

	if(setjmp(Gfailbuf))
		return(FAIL);
	for (;;) {
		len = pkread(Pk, str, packsize);
		if (len == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}

int
gwrdata(fp1, fn)
FILE *fp1;
{
	char bufr[BUFSIZ];
	int fd1;
	int len;
	int ret;
	unsigned long bytes;

	if(setjmp(Gfailbuf))
		return(FAIL);
	bytes = 0L;
	fd1 = fileno( fp1 );
	while ((len = read( fd1, bufr, BUFSIZ )) > 0) {
		bytes += len;
		putfilesize(bytes);
		ret = gwrblk(bufr, len, fn);
		if (ret != len) {
			return(FAIL);
		}
		if (len != BUFSIZ)
			break;
	}
	ret = gwrblk(bufr, 0, fn);
	return(0);
}

int
grddata(fn, fp2)
FILE *fp2;
{
	register int ret = SUCCESS;
	int fd2;
	int len;
	char bufr[BUFSIZ];
	unsigned long bytes;

	if(setjmp(Gfailbuf))
		return(FAIL);
	bytes = 0L;
	fd2 = fileno( fp2 );
	for (;;) {
		len = grdblk(bufr, BUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		putfilesize(bytes);
		if ( ret == SUCCESS && write( fd2, bufr, len ) != len) {
			ret = errno;
			DEBUG(7, "grddata: write to file failed, errno %d\n", errno);
		}
		if (len < BUFSIZ)
			break;
	}
	return(ret);
}


/*ARGSUSED*/
int
grdblk(blk, len,  fn)
char *blk;
{
	int i, ret;

	for (i = 0; i < len; i += ret) {
		ret = pkread(Pk, blk, len - i);
		if (ret < 0)
			return(FAIL);
		blk += ret;
		if (ret == 0)
			return(i);
	}
	return(i);
}


/*ARGSUSED*/
int
gwrblk(blk, len, fn)
char *blk;
{
	int ret;

	ret = pkwrite(Pk, blk, len);
	return(ret);
}
