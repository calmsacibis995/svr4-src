/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:eio.c	2.14.3.1"

#include "uucp.h"

#ifdef	E_PROTOCOL

#ifndef MIN
#define     MIN(a,b) (((a)<(b))?(a):(b))
#endif

extern long lseek();	/* Find offset into the file. */
static jmp_buf Failbuf;
extern int erdblk();

#define	EBUFSIZ	1024	/* "e" protocol buffer size */

/*	This constant MUST be the same on all machines running "e"
	uucico protocol!  It must remain this value forever!
*/
#define	CMSGSIZ	20	/* size of the initial file size message */

/*
 * error-free channel protocol
 */
/* ARGSUSED */
static void
ealarm(sig)
int sig;
{
	longjmp(Failbuf, 1);
}
static void (*esig)();

/*
 * turn on protocol timer
 */
int
eturnon()
{
	esig=signal(SIGALRM, ealarm);
	return(0);
}

int
eturnoff()
{
	signal(SIGALRM, esig);
	return(0);
}

/*
 * write message across link
 *	type	-> message type
 *	str	-> message body (ascii string)
 *	fn	-> link file descriptor
 * return
 *	FAIL	-> write failed
 *	0	-> write succeeded
 */
int
ewrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[EBUFSIZ];
	int	s1, s2;

	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	s1 = strlen(bufr) + 1;
	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrmsg write failed\n%s", "");
		return(FAIL);
	}
	alarm(60);
	s2 = (*Write)(fn, bufr, (unsigned) s1);
	alarm(0);
	if (s1 != s2)
		return(FAIL);
	return(0);
}

/*
 * read message from link
 *	str	-> message buffer
 *	fn	-> file descriptor
 * return
 *	FAIL	-> read timed out
 *	0	-> ok message in str
 */
int
erdmsg(str, fn)
register char *str;
{
	register int i;
	register int len;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdmsg read failed\n%s", "");
		return(FAIL);
	}

	i = EBUFSIZ;
	for (;;) {
		alarm(60);
		len = (*Read)(fn, str, i);
		alarm(0);
		if (len <= 0) return(FAIL);
		str += len; i -= len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}

/*
 * read data from file fp1 and write
 * on link
 *	fp1	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	FAIL	->failure in link
 *	0	-> ok
 */
int
ewrdata(fp1, fn)
register FILE *fp1;
int	fn;
{
	register int ret;
	int	fd1;
	int len;
	unsigned long bytes;
	char bufr[EBUFSIZ];
	struct stat	statbuf;
	off_t	msglen;
	char	cmsglen[CMSGSIZ];
	off_t	startPoint;	/* Offset from begining of the file in
				 *   case we are restarting from a check
				 *   point.
				 */

	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrdata failed\n%s", "");
		return(FAIL);
	}
	bytes = 0L;
	fd1 = fileno(fp1);
	fstat(fd1, &statbuf);
	startPoint = lseek(fd1, 0L, 1);
	if (startPoint < 0)
	{
		DEBUG(7, "ewrdata lseek failed.  Errno=%d\n", errno);
		return(FAIL);
	}
	msglen = statbuf.st_size - startPoint;
	if (msglen < 0)
	{
		DEBUG(7, "ewrdata: startPoint past end of file.\n%s", "");
		return(FAIL);
	}
	sprintf(cmsglen, "%ld", (long) msglen);
	alarm(60);
	ret = (*Write)(fn, cmsglen, sizeof(cmsglen));
	if (ret != sizeof(cmsglen))
		return(FAIL);
	DEBUG(7, "ewrdata planning to send %ld bytes to remote.\n", msglen);
	while ((len = read( fd1, bufr, EBUFSIZ )) > 0) {
		alarm(60);
		bytes += len;
		putfilesize(bytes);
		ret = (*Write)(fn, bufr, (unsigned) len);
		alarm(0);
		DEBUG(9, "ewrdata ret %d\n", ret);
		if (ret != len)
			return(FAIL);
		if ((msglen -= len) <= 0)
			break;
	}
	if (len < 0 || (len == 0 && msglen != 0)) return(FAIL);
	return(0);
}

/*
 * read data from link and
 * write into file
 *	fp2	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	0	-> ok
 *	FAIL	-> failure on link
 */
int
erddata(fn, fp2)
register FILE *fp2;
{
	register int len;
	register int ret = SUCCESS;
	int	fd2;
	unsigned long bytes;
	char bufr[EBUFSIZ];
	long	msglen;
	char	cmsglen[CMSGSIZ];

	bytes = 0L;
	len = erdblk(cmsglen, sizeof(cmsglen), fn);
	if (len < 0)
	    return(FAIL);
	sscanf(cmsglen, "%ld", &msglen);
	DEBUG(7, "erdblk msglen %ld\n", msglen);
	if ( ((msglen-1)/512 +1) > Ulimit )
		ret = EFBIG;
	fd2 = fileno( fp2 );
	for (;;) {
		len = erdblk(bufr, (int) MIN(msglen, EBUFSIZ), fn);
		DEBUG(9, "erdblk ret %d\n", len);
		if (len < 0) {
			DEBUG(7, "erdblk failed\n%s", "");
			return(FAIL);
		}
		bytes += len;
		putfilesize(bytes);
		if ((msglen -= len) < 0) {
			DEBUG(7, "erdblk read too much\n%s", "");
			return(FAIL);
		}
		/* this write is to file -- use write(2), not (*Write) */
		if ( ret == SUCCESS && write( fd2, bufr, len ) != len ) {
			ret = errno;
			DEBUG(7, "erddata: write to file failed, errno %d\n", ret);
		}
		if (msglen == 0)
			break;
	}
	return(ret);
}

/*
 * read block from link
 * reads are timed
 *	blk	-> address of buffer
 *	len	-> size to read
 *	fn	-> link descriptor
 * returns:
 *	FAIL	-> link error timeout on link
 *	i	-> # of bytes read
 */
int
erdblk(blk, len,  fn)
register char *blk;
{
	register int i, ret;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdblk timeout\n%s", "");
		return(FAIL);
	}

	for (i = 0; i < len; i += ret) {
		alarm(60);
		DEBUG(9, "erdblk ask %d ", len - i);
		if ((ret = (*Read)(fn, blk, (unsigned) len - i)) <= 0) {
			alarm(0);
			DEBUG(7, "erdblk read failed\n%s", "");
			return(FAIL);
		}
		DEBUG(9, "erdblk got %d\n", ret);
		blk += ret;
		if (ret == 0)
			break;
	}
	alarm(0);
	return(i);
}
#endif	/* E_PROTOCOL */
