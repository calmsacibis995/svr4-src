/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/mlisten.c	1.9.3.1"
# include	<unistd.h>
# include	<stdlib.h>
# include	<string.h>
# include	<poll.h>
# include	<stropts.h>
# include	<fcntl.h>
# include	<errno.h>

# include	"lp.h"
# include	"msgs.h"

#define TURN_ON(X,F)	(void)Fcntl(X, F_SETFL, (Fcntl(X, F_GETFL, 0)|(F)))

static int		NumEvents = 0;
static int		NumCons = 0;
static int		ConsSize= 0;
static int		NumNewCons = 0;
static MESG **		Connections = NULL;
static struct pollfd *	PollFdList = NULL;

#if	defined(__STDC__)
int
mlisteninit ( MESG * md )	/* funcdef */
#else
int
mlisteninit ( md )
MESG	*md;
#endif
{
    if (md == NULL)
    {
	errno = EINVAL;
	return(-1);
    }

    if (ConsSize > 0)
    {
	errno = EBUSY;
	return(-1);
    }

    ConsSize = 20;
    Connections = (MESG **) Malloc(ConsSize * MDSIZE);
    PollFdList = (struct pollfd*) Malloc(ConsSize * sizeof(struct pollfd));
    if (Connections == NULL || PollFdList == NULL)
    {
	errno = ENOMEM;
	return(-1);
    }
    Connections[0] = md;
    PollFdList[0].fd = md->readfd;
    PollFdList[0].events = POLLIN;
    PollFdList[0].revents = 0;
    NumCons = 1;
    return(0);
}

#if	defined(__STDC__)
int
mlistenadd ( MESG * md, short events )	/* funcdef */
#else
int
mlistenadd ( md, events )
MESG	*md;
short	events;
#endif
{
    int			slack;
    struct pollfd *	fdp;

    /*
    **	See if we have room in the connection table.
    **	Realloc(3) the table if the number of connections
    **	changes by more than 20.
    */

    slack = ConsSize - (NumCons + NumNewCons + 1);

    if (slack < 0)
    {
	ConsSize += 20;
	Connections = (MESG **) Realloc(Connections, ConsSize * MDSIZE);
	PollFdList = (struct pollfd*) Realloc(PollFdList, ConsSize * sizeof(struct pollfd));
	if (Connections == NULL || PollFdList == NULL)
	{
	    errno = ENOMEM;
	    return(-1);
	}
    }

    if (slack > 20)
    {
	ConsSize -= 20;
	Connections = (MESG **) Realloc(Connections, ConsSize * MDSIZE);
	PollFdList = (struct pollfd*) Realloc(PollFdList, ConsSize * sizeof(struct pollfd));
	if (Connections == NULL || PollFdList == NULL)
	{
	    errno = ENOMEM;
	    return(-1);
	}
    }

    fdp = PollFdList + (NumCons + NumNewCons);
    fdp->fd = md->readfd;
    fdp->events = events;
    fdp->revents = 0;

    /*
    **	Now add the entry to the connection table
    **	NumCons will be updated above.
    */
    Connections[NumCons + NumNewCons++] = md;
    return(0);
}

#if	defined(__STDC__)
MESG *
mlistenreset ( void )	/* funcdef */
#else
MESG *
mlistenreset ( )
#endif
{
    int		x;
    MESG *	md;

    if (ConsSize == 0)
	return(NULL);

    ConsSize = 0;

    for (x = 1; x < NumCons; x++)
	(void) mdisconnect(Connections[x]);

    md = Connections[0];

    Free(Connections);
    Free(PollFdList);

    Connections = NULL;
    PollFdList = NULL;
    NumCons = 0;
    NumNewCons = 0;
    NumEvents = 0;
    return(md);
}

#if	defined(__STDC__)
MESG * mlisten ( void )	/* funcdef */
#else
MESG * mlisten ( )
#endif
{
    extern uid_t	Lp_Uid;

    MESG *		mdp;
    MESG *		md;
    MQUE *		p;
    char		cbuff[MSGMAX];
    int			flag = 0;
    int			disconacts;
    int			x;
    int			y;
    struct pollfd *	fdp;
    struct strbuf	ctl;
    struct strrecvfd	recbuf;
    /*
    **	Set up buffer for receiving messages.
    */
    ctl.buf = cbuff;
    ctl.maxlen = sizeof (cbuff);

    /*
    **	This loop exists to return control to poll after the
    **	result of poll yeilds no new connections or serviceable
    **	messages.
    */
    for (;;)
    {
	/*
	**	If there are no unserviced events pending, call poll(2)
	**	and wait for a message or connection.
	**	NumEvents may be -1 in the event of an interrupt, hence
	**	<= 0
	*/
	if (NumEvents <= 0)
	{
	    /*
	    **	Add new connections, if any, reset connection counter
	    */
	    NumCons += NumNewCons;
	    NumNewCons = 0;

	    if (NumCons <= 0)
	    {
		errno = EINTR;
		return(NULL);
	    }
	    
	    /*
	    **	Scan the connection table and remove any holes
	    */
	    for (x = 0; x < NumCons; x++)
	    {
		mdp = Connections[x];

		/*
		**	Disconnected, clear the node and compress the
		**	tables.  If the disconnect called any
		**	on_discon functions (disconacts > 0), return
		**	because there may be something to clean up.
		**	Finally, decrement <x> so that the next node
		**	doesn't get missed.
		*/
		if (mdp->readfd == -1)
		{
		    disconacts = mdisconnect(mdp);
		    NumCons--;
		    for (y = x; y < (NumCons + NumNewCons); y++)
		    {
			Connections[y] = Connections[y + 1];
			PollFdList[y] = PollFdList[y + 1];
		    }
		    if (disconacts > 0)
		    {
			errno = EINTR;
			return(NULL);
		    }
		    else
			x--;
		}
		else
		    if (mdp->mque)
			PollFdList[x].events = POLLOUT;
		    else
			PollFdList[x].events = POLLIN;
	    }

	    /*
	    **	Wait for a message or a connection.
	    **	This call may be interrupted by alarms used
	    **	elsewhere, so if poll fails, return NULL and
	    **	set errno to EAGAIN.
	    */
	    if ((NumEvents = poll(PollFdList, NumCons, -1)) < 0)
	    {
		errno = EAGAIN;
		return(NULL);
	    }
	}

	for (x = 0; x < NumCons; x++)
	{
	    mdp = Connections[x];
	    fdp = PollFdList + x;

	    if (fdp->revents == 0)
		continue;

	    if (mdp->type == MD_MASTER)
	    {
		/*
		**	Only valid revent is: POLLIN
		*/
		if (fdp->revents != POLLIN)
		{
		    errno = EINVAL;
		    return(NULL);
		}

		/*
		**	Retrieve the file descriptor
		*/
		if (ioctl(mdp->readfd, I_RECVFD, &recbuf) != 0)
		{
		    if (errno == EINTR)
		    {
			errno = EAGAIN;
			return(NULL);
		    }
		    if (errno == ENXIO)
		    {
			fdp->revents = 0;
			NumEvents--;
			continue;
		    }
#if	defined(NOCONNLD)
		    if (errno == EBADMSG)
			while (Getmsg(mdp, &ctl, &ctl, &flag) >= 0);
#endif
		    return(NULL);
		}

		TURN_ON(recbuf.fd, O_NDELAY);
		/*
		**	Now, create the message descriptor
		**	and populate it with what we know.
		*/
		if ((md = (MESG *)Malloc(MDSIZE)) == NULL)
		{
		    errno = ENOMEM;
		    return(NULL);
		}


		md->file = NULL;
		md->gid = recbuf.gid;
		md->mque = NULL;
		md->on_discon = NULL;
		md->readfd = md->writefd = recbuf.fd;
		md->state = MDS_IDLE;
		md->type = MD_UNKNOWN;
		md->uid = recbuf.uid;

		md->admin = (md->uid == 0 || md->uid == Lp_Uid);

		if (mlistenadd(md, POLLIN) != 0)
		    return(NULL);

		ResetFifoBuffer (md->readfd);
		/*
		**	Reset fdp because mlistenadd may have
		**	realloc()ed PollFdList and changed its
		**	physical location.
		*/
		fdp = PollFdList + x;

		/*
		**	Clear the event that brought us here,
		**	decrement the event counter, and get the
		**	next event.
		*/
		fdp->revents = 0;
		NumEvents--;
		continue;
	    }
	    else
		/*
		**	If this connection is a child process, just
		**	save the event and return the message descriptor
		*/
		if (mdp->type == MD_CHILD)
		{
		    mdp->event = fdp->revents;
		    NumEvents--;
		    fdp->revents = 0;
		    return(mdp);
		}
		else
		{

		    /*
		    **	POLLNVAL means this client disconnected and
		    **	all messages have been processed.
		    */
		    if (fdp->revents & POLLNVAL) /* disconnected & no msg */
		    {
			if (mdp->readfd >= 0) {
				Close (mdp->readfd);
				if (mdp->writefd == mdp->readfd)
					mdp->writefd = -1;
				mdp->readfd = -1;
			}
			fdp->revents = 0;
			NumEvents--;
			continue;
		    }

		    /*
		    **	POLLERR means an error message is on the 
		    **	stream.  Since this is totally unexpected,
		    **	the assumption is made that this stream will
		    **	be flagged POLLNVAL next time through poll
		    **	and will be removed at that time.
		    */
		    if (fdp->revents & POLLERR)	/* uh, oh! */
		    {
			if (mdp->readfd >= 0) {
				Close (mdp->readfd);
				if (mdp->writefd == mdp->readfd)
					mdp->writefd = -1;
				mdp->readfd = -1;
			}
			NumEvents--;
			fdp->revents = 0;
			continue;
		    }


		    /*
		    **	POLLHUP means the client aborted the call.
		    **	The client is not removed, because there may
		    **	still be messages on the stream.
		    */
		    if (fdp->revents & POLLHUP)	/* disconnected */
		    {
			NumEvents--;
			fdp->revents = 0;
			/*
			 * MORE: This is odd. Why are we closing the
			 * stream if there ``may still be messages''???
			 */
			if (mdp->readfd >= 0) {
				Close (mdp->readfd);
				if (mdp->writefd == mdp->readfd)
					mdp->writefd = -1;
				mdp->readfd = -1;
			}
			continue;

			/*
			 * MORE: Why is this here??
			 *
			if (mdp->type == MD_SYS_FIFO)
			    (void) Close(mdp->writefd);

			mdp->writefd = -1;

			if (fdp->revents == POLLHUP)
			{
			    NumEvents--;
			    fdp->revents = 0;
			    (void) Close(mdp->readfd);
			    mdp->readfd = -1;
			    continue;
			}
			 *
			 */
		    }
		    /*
		    **	POLLOUT means that the client had a full
		    **	stream and messages became backlogged and
		    **	now the stream is empty.  So the queued msgs
		    **	are sent with putmsg(2)
		    */
		    if (fdp->revents & POLLOUT)
		    {
			if (mdp->mque == NULL)
			{
			    NumEvents--;
			    fdp->revents = 0;
			    continue;
			}
			while(mdp->mque)
			{
			    if(Putmsg(mdp, NULL, mdp->mque->dat, 0))
				break;
			    p = mdp->mque;
			    mdp->mque = p->next;
			    Free(p->dat->buf);
			    Free(p->dat);
			    Free(p);
			}
			NumEvents--;
			fdp->revents = 0;
			continue;
		    }

		    /*
		    **	POLLIN means that there is a message on the
		    **	stream.
		    **	Return the message descriptor to the caller
		    **	so that the message may be received and
		    **	processed.
		    */
		    if (fdp->revents & POLLIN)	/* got a message */
		    {
			NumEvents--;
			mdp->event = fdp->revents;
			fdp->revents = 0;
			if (mdp->type == MD_UNKNOWN)
			{
/*
**			    if (Getmsg(mdp, &ctl, NULL, &flag) < 0)
**				return(NULL);
**
**			    if (ctl.len == -1)
**				mdp->type = MD_SYS_FIFO;
**			    else
**				mdp->type = MD_STREAM;
*/
			    mdp->type = MD_STREAM;
			}
			return(mdp);
		    }
		}
	}
    }
}

# define	VOID_FUNC_PTR		void (*)()
# define	PTR_TO_VOID_FUNC_PTR	void (**)()

#if	defined(__STDC__)
int mon_discon ( MESG * md, void (*fn)() )
#else
int mon_discon ( md, fn )
MESG	*md;
void	(*fn)();
#endif
{
    int		size = 2;
    void	(**fnp) ();

    if (md->on_discon)
    {
	for (fnp = md->on_discon; *fnp; fnp++)
	    size++;
	if ((md->on_discon = (PTR_TO_VOID_FUNC_PTR) Realloc (md->on_discon, size * sizeof(VOID_FUNC_PTR))) == NULL)
	{
	    errno = ENOMEM;
	    return(-1);
	}
    }
    else
	if ((md->on_discon = (PTR_TO_VOID_FUNC_PTR) Malloc (size * sizeof(VOID_FUNC_PTR))) == NULL)
	{
	    errno = ENOMEM;
	    return(-1);
	}
    
    size--;
    md->on_discon[size] = NULL;
    size--;
    md->on_discon[size] = fn;
    return(0);
}
