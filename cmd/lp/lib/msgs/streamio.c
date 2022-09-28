/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:lib/msgs/streamio.c	1.1.2.2"

#include	<unistd.h>
#include	<signal.h>
#include	<stropts.h>
#include	<errno.h>
#include	"lp.h"
#include	"msgs.h"
#include	"debug.h"

extern	int	errno;

#if	__STDC__
int
Putmsg (MESG *mdp, strbuf_t *ctlp, strbuf_t *datap, int flags)
#else
int
Putmsg (mdp, ctlp, datap, flags)

MESG	 *mdp;
strbuf_t *ctlp;
strbuf_t *datap;
int	 flags;
#endif
{
	int	rtncode, count;
	struct pollfd fds;

	DEFINE_FNNAME (Putmsg)
	ENTRYP
	fds.fd=mdp->writefd;
	fds.events=POLLOUT;
	poll(&fds, 1, 1000);
	while(!(fds.revents && POLLOUT)){
	    sleep(1);
	    poll(&fds, 1, 1000);
	}
	rtncode = putmsg (mdp->writefd, ctlp, datap, flags);
	if (rtncode == 0 || errno != ENOSTR || !datap)
	{
		TRACEP ("putmsg succeeded.")
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (datap)
		EXITP
		return	rtncode;
	}
#ifdef	DEBUG
	{
		int	arg;

		TRACEP ("ioctls checks")
		TRACEP ("I_GWROPT")
		errno = 0;
		arg = 0;
		rtncode = ioctl (mdp->writefd, I_GWROPT, &arg);
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (arg)

		TRACEP ("I_SWROPT")
		errno = 0;
		rtncode = ioctl (mdp->writefd, I_SWROPT, SNDZERO);
		TRACEd (rtncode)
		TRACEd (errno)

		TRACEP ("I_GWROPT")
		errno = 0;
		arg = 0;
		rtncode = ioctl (mdp->writefd, I_GWROPT, &arg);
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (arg)
	}
#endif
/*
**	TRACEP ("write3_2")
**	count = write3_2 (mdp, datap->buf, datap->len);
**	if (count < 0 || count < datap->len)
**	{
**		TRACEP ("write3_2 failed.")
**		TRACEd (count)
**		TRACEd (datap->len)
**		EXITP
**		return	-1;
**	}
**	TRACEP ("write3_2 succeeded.")
*/
	TRACEP ("write")
	count = write (mdp->writefd, datap->buf, datap->len);
	if (count < 0 || count < datap->len)
	{
		TRACEP ("write failed.")
		TRACEd (count)
		TRACEd (datap->len)
		EXITP
		return	-1;
	}
	TRACEP ("write succeeded.")
	EXITP
	return	0;
}

#if	__STDC__
int
Getmsg (MESG *mdp, strbuf_t *ctlp, strbuf_t *datap, int *flagsp)
#else
int
Getmsg (mdp, ctlp, datap, flagsp)

MESG	 *mdp;
strbuf_t *ctlp;
strbuf_t *datap;
int	 *flagsp;
#endif
{
	int	rtncode;

	DEFINE_FNNAME (Getmsg)
	ENTRYP
	rtncode = getmsg (mdp->readfd, ctlp, datap, flagsp);
	if (rtncode >= 0 || errno != ENOSTR || !datap)
	{
		TRACEP ("getmsg succeeded.")
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (datap)
		EXITP
		return	rtncode;
	}
#ifdef	DEBUG
	{
		int	arg;

		TRACEP ("ioctls checks")
		TRACEP ("I_GRDOPT")
		errno = 0;
		arg = 0;
		rtncode = ioctl (mdp->readfd, I_GRDOPT, &arg);
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (arg)
	}
#endif

	TRACEP ("I_SRDOPT")
	errno = 0;
	rtncode = ioctl (mdp->readfd, I_SRDOPT, RMSGN|RPROTDIS);
	TRACEd (rtncode)
	TRACEd (errno)

#ifdef	DEBUG
	{
		int	arg;

		TRACEP ("I_GRDOPT")
		errno = 0;
		arg = 0;
		rtncode = ioctl (mdp->readfd, I_GRDOPT, &arg);
		TRACEd (rtncode)
		TRACEd (errno)
		TRACE  (arg)
	}
#endif
/*
**	TRACEP ("read3_2")
**	if (ctlp)
**		ctlp->len = -1;
**	*flagsp = 0;
**	datap->len = read3_2 (mdp, datap->buf, datap->maxlen);
**	if (datap->len < 0)
**	{
**		TRACEP ("read3_2 failed.")
**		EXITP
**		return	-1;
**	}
**	TRACEP ("read3_2 succeeded.")
*/
	TRACEP ("read")
	if (ctlp)
		ctlp->len = -1;
	*flagsp = 0;
	datap->len = read (mdp->readfd, datap->buf, datap->maxlen);
	if (datap->len < 0)
	{
		TRACEP ("read failed.")
		EXITP
		return	-1;
	}
	TRACEP ("read succeeded.")
	EXITP
	return	0;
}

char		AuthCode[HEAD_AUTHCODE_LEN];
static void	(*callers_sigpipe_trap)() = SIG_DFL;


/*
**	Function:	static int read3_2( MESG *, char *, int)
**	Args:		message descriptor
**			message buffer (var)
**			buffer size
**	Return:		0 for sucess, -1 for failure
**
**	This performs a 3.2 HPI style read_fifo on the pipe referanced
**	in the message descriptor.  If a message is found, it is returned
**	in message buffer.
*/
#if	defined(__STDC__)
int read3_2 ( MESG * md, char *msgbuf, int size )
#else
int read3_2 ( md, msgbuf, size )
MESG	*md;
char	*msgbuf;
int	size;
#endif
{
    short	type;

    if (md->type == MD_USR_FIFO)
	(void) Close (Open(md->file, O_RDONLY, 0));

    do
    {
	switch (read_fifo(md->readfd, msgbuf, size))
	{
	  case -1:
	    return (-1);

	  case 0:
	    /*
	     ** The fifo was empty and we have O_NDELAY set,
	     ** or the Spooler closed our FIFO.
	     ** We don't set O_NDELAY in the user process,
	     ** so that should never happen. But be warned
	     ** that we can't tell the difference in some versions
	     ** of the UNIX op. sys.!!
	     **
	     */
	    errno = EPIPE;
	    return (-1);
	}

	if ((type = stoh(msgbuf + HEAD_TYPE)) < 0 || LAST_MESSAGE < type)
	{
	    errno = ENOMSG;
	    return (-1);
	}
    }
    while (type == I_QUEUE_CHK);

    (void)memcpy (AuthCode, msgbuf + HEAD_AUTHCODE, HEAD_AUTHCODE_LEN);

    /*
    **	Get the size from the 3.2 HPI message
    **	minus the size of the control data
    **	Copy the actual message
    **	Reset the message size.
    */
    size = stoh(msgbuf + HEAD_SIZE) - EXCESS_3_2_LEN;
    memmove(msgbuf, msgbuf + HEAD_SIZE, size);
    (void) htos(msgbuf + MESG_SIZE, size);
    return(0);
}

#if	defined(__STDC__)
int write3_2 ( MESG * md, char * msgbuf, int size )
#else
int write3_2 (md, msgbuf, size)
    MESG	*md;
    char	*msgbuf;
    int		size;
#endif
{
    char	tmpbuf [MSGMAX + EXCESS_3_2_LEN];
    int		rval;


    (void) memmove(tmpbuf + HEAD_SIZE, msgbuf, size);
    (void) htos(tmpbuf + HEAD_SIZE, size + EXCESS_3_2_LEN);
    (void) memcpy (tmpbuf + HEAD_AUTHCODE, AuthCode, HEAD_AUTHCODE_LEN);

    callers_sigpipe_trap = signal(SIGPIPE, SIG_IGN);

    rval = write_fifo(md->writefd, tmpbuf, size + EXCESS_3_2_LEN);

    (void) signal(SIGPIPE, callers_sigpipe_trap);


    return (rval);
}
