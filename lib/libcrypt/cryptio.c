/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libcrypt:cryptio.c	1.19"

#ifdef __STDC__
	#pragma weak run_setkey = _run_setkey
	#pragma weak run_crypt = _run_crypt
	#pragma weak crypt_close = _crypt_close
	#pragma weak makekey = _makekey
#endif

#include "synonyms.h"
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#define	READER		0
#define	WRITER		1
#define KSIZE 	8

static pid_t	popen_pid[256];
static char key[KSIZE+1];
/*  Global Variables  */
static struct header {
	long offset;
	unsigned int count;
};

static int cryptopen();
static int writekey();
static int _cp2close(), _cp2open();

extern uid_t getuid();
extern gid_t getgid(); 
extern int read(), write();
extern int kill();
extern char *strncpy();
extern int	close(),
		execl(),
		pipe();
extern pid_t	fork();
void	_exit();

int crypt_close(); 

int run_setkey(p, keyparam)
int   p[2];
const char *keyparam;
{
	if(cryptopen(p) == -1)
		return -1;
	(void)strncpy(key, keyparam, KSIZE);
	if(*key == 0) {
		(void)crypt_close(p);
		return(0);
	}
	if(writekey(p,key) == -1)
		return -1;
	return(1);
}

static char cmd[] = "exec /usr/bin/crypt -p 2>/dev/null";
static int
cryptopen(p)
int	p[2];
{
	char c;	

	if(_cp2open(cmd, p) < 0) 
		return(-1);
	if(read(p[WRITER], &c, 1)!= 1) { /* check that crypt is working on
					   other end */
		(void)crypt_close(p); /* remove defunct process */
		return(-1); 
	}
	return(1);
}

static int writekey(p,keyarg)
int p[2];
char *keyarg;
{
	void (*pstat) ();
	pstat = signal(SIGPIPE, SIG_IGN); /* don't want pipe errors to cause
					     death */
	if(write(p[READER], keyarg, KSIZE) != KSIZE) {
		(void)crypt_close(p); /* remove defunct process */
		(void)signal(SIGPIPE,pstat);
		return(-1);
	}
	(void)signal(SIGPIPE,pstat);
	return(1);
}


int
run_crypt(offset, buffer, count, p)
long offset;
char	*buffer;
unsigned int count;
int p[2];
{
	struct header header;
	void (*pstat) ();

	header.count = count;
	header.offset = offset;
	pstat = signal(SIGPIPE, SIG_IGN);
	if(write(p[READER], (char *)&header, sizeof(header))!=sizeof(header)) {
		(void)crypt_close(p);
		(void)signal(SIGPIPE, pstat);
		return -1;
	}
	if(write(p[READER], buffer, count) < count) {
		(void)crypt_close(p);
		(void)signal(SIGPIPE, pstat);
		return(-1);
	}
	if(read(p[WRITER], buffer,  count) < count) {
		(void)crypt_close(p);
		(void)signal(SIGPIPE, pstat);
		return(-1);
	}
	(void)signal(SIGPIPE, pstat);
	return(0);
}

makekey(b)
int b[2];
{
	register int i;
	long gorp;
	char tempbuf[KSIZE], *a, *temp;

	a = key;
	temp = tempbuf;
	for(i = 0; i < KSIZE; i++)
		temp[i] = *a++; 
	gorp = getuid() + getgid();

	for(i = 0; i < 4; i++)
		temp[i] ^= (char)((gorp>>(8*i))&0377); 

	if (cryptopen(b) == -1)
		return(-1);
	if (writekey(b,temp) == -1)
		return(-1);
	return(0);
}


crypt_close(p)
int p[2];
{
	pid_t pid;
	int ret;
	if(p[0] == 0 && p[1] == 0 || p[0] < 0 || p[1] < 0)
		return -1;
	pid = popen_pid[p[0]];
	if(pid != popen_pid[p[1]])
		return -1;
	if(!pid)
		return -1;
	(void) kill(pid, 9);
	ret = _cp2close(p);
	return ret;
}

/*
	Similar to popen(3S) but with pipe to cmd's stdin and from stdout.
*/

static int _cp2open(cmd, p)
const char	*cmd;
int	p[2]; /* file descriptor array to cmd stdin and stdout */
{
	int		tocmd[2];
	int		fromcmd[2];
	register int	pid;

	if(pipe(tocmd) < 0  ||  pipe(fromcmd) < 0 )
		return  -1;
	/* be consistent with stdio */
	if(tocmd[1] >= 256 || fromcmd[0] >= 256) {
		(void) close(tocmd[0]);
		(void) close(tocmd[1]);
		(void) close(fromcmd[0]);
		(void) close(fromcmd[1]);
		return -1;
	}
	if( (pid = fork()) == 0 ) {
		(void) close( tocmd[1] );
		(void) close( 0 );
		(void) fcntl( tocmd[0], F_DUPFD, 0 );
		(void) close( tocmd[0] );
		(void) close( fromcmd[0] );
		(void) close( 1 );
		(void) fcntl( fromcmd[1], F_DUPFD, 1 );
		(void) close( fromcmd[1] );
		(void) execl("/sbin/sh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return  -1;
	popen_pid[ tocmd[1] ] = pid;
	popen_pid[ fromcmd[0] ] = pid;
	(void) close( tocmd[0] );
	(void) close( fromcmd[1] );
	p[0] = tocmd[1];
	p[1] = fromcmd[0];
	return  0;
}

static int
_cp2close(p)
int	p[2];
{
	register pid_t	r;
	int		status;
	pid_t		waitpid();
	void		(*hstat)(),
			(*istat)(),
			(*qstat)();
	
	pid_t pid;

	if(p[0] < 0 || p[0] >= 256 || p[1] < 0 || p[1] >= 256)
		return -1;
	pid = popen_pid[p[0]];
	if(pid != popen_pid[p[1]])
		return -1;
	(void)close(p[0]);
	(void)close(p[1]);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while ((r = waitpid(pid, &status, 0)) == (pid_t)-1 && errno == EINTR)
		;
	if (r == (pid_t)-1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	return  status;
}
