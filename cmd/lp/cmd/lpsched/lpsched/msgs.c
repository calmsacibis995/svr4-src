/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/msgs.c	1.9.4.1"

# include	<stdarg.h>
# include	<limits.h>
# include	<sys/types.h>
# include	<poll.h>
# include	<stropts.h>
# include	<unistd.h>

# include	"lpsched.h"

#define TURN_OFF(X,F)	(void)Fcntl(X, F_SETFL, (Fcntl(X, F_GETFL, 0) & ~(F)))

#if	defined(DEBUG)
static char time_buf[50];
#endif

#if	defined(__STDC__)
static void	log_message_fact(char *, ...);
static void	do_msg3_2 ( MESG * md );
#else
static void	log_message_fact();
static void	do_msg3_2 ();
#endif

static void	net_shutdown();
static void	conn_shutdown();

extern int		Filter_Status;
extern void		dispatch();
extern int		Waitrequest;
void			shutdown_messages();
static char		*Message;
static int		MaxClients		= 0,
			do_msg();
extern int		Reserve_Fds;
extern int		Shutdown;

MESG			*Net_md;
int			Net_fd;

/*
** take_message() - WAIT FOR INTERRUPT OR ONE MESSAGE FROM USER PROCESS
*/

#if	defined(__STDC__)
void take_message ( void )	/* funcdef */
#else
void take_message ( )
#endif
{
    ENTRY ("take_message")

    int		bytes;
    int		slot;
    MESG *	md;

    for (EVER)	/* not really forever...returns are in the loop */
    {
	if ((md = mlisten()) == NULL)
	    switch(errno)
	    {
	      case EAGAIN:
	      case EINTR:
		return;

	      case ENOMEM:
		mallocfail();
		/* NOTREACHED */

	      default:
		fail ("Unexpected streams error in mlisten (%s).\n" , PERROR);
	    }
	
	/*
	 * Check for a dropped connection to a child.
	 * Normally a child should tell us that it is dying
	 * (with S_SHUTDOWN or S_SEND_CHILD), but it may have
	 * died a fast death. We'll simulate the message we
	 * wanted to get so we can use the same code to clean up.
	 */
	if (
		(md->event & POLLHUP) && !(md->event & POLLIN)
	     || (md->event & (POLLERR|POLLNVAL))
	) {
		switch (md->type) {

		case MD_CHILD:
			/*
			 * If the message descriptor is found in the
			 * exec table, it must be an interface pgm,
			 * notification, etc. Otherwise, it must be
			 * a network child.
			 */
			for (slot = 0; slot < ET_Size; slot++)
				if (Exec_Table[slot].md == md)
					break;
			if (slot < ET_Size) {
				(void)putmessage (
					Message,
					S_CHILD_DONE,
					Exec_Table[slot].key,
					slot,
					0,
					0
				);
#if	defined(DEBUG)
				if (debug & (DB_EXEC|DB_DONE)) {
					execlog (
						"DROP! slot %d, pid %d\n",
						slot,
						Exec_Table[slot].pid
					);
					execlog ("%e", &Exec_Table[slot]);
				}
#endif
			} else
				(void)putmessage (Message, S_SHUTDOWN, 1);
			bytes = 1;
			break;

		default:
			bytes = -1;
			break;

		}

	} else
		bytes = mread(md, Message, MSGMAX);

	switch (bytes)
	{
	  case -1:
	    if (errno == EINTR)
		return;
	    else
		fail ("Unexpected streams error (%s).\n" , PERROR);
	    break;

	  case 0:
	    break;

	  default:
	    if (do_msg(md))
		return;
	    break;
	}
    }
}

/*
** do_msg() - HANDLE AN INCOMING MESSAGE
*/

#if	defined(__STDC__)
static int do_msg ( MESG * md )	/* funcdef */
#else
static int do_msg (md)
    MESG	*md;
#endif
{
    ENTRY ("do_msg")

    int			type;

#if	defined(DEBUG)
    if (debug & DB_MESSAGES)
    {
	FILE	*fp = open_logfile("messages");

	if (fp)
	{
		time_t	now = time((time_t *)0);
		char	buffer[BUFSIZ];
		int	size	= stoh(Message + MESG_SIZE);
		int	type	= stoh(Message + MESG_TYPE);

		setbuf(fp, buffer);
		cftime(time_buf, NULL, &now);
		fprintf(
		    fp,
		    "RECV: %24.24s type %d size %d\n",
		    time_buf,
		    type,
		    size
		);
		fputs("      ", fp);
		fwrite(Message, 1, size, fp);
		fputs("\n", fp);
		close_logfile(fp);
	}
    }
# endif

    switch (type = mtype(Message))
    {
      case S_NEW_QUEUE:
	do_msg3_2(md);
	break;

      default:
#if	defined(DEBUG)
	log_message_fact ("      MESSAGE ACCEPTED: client %#0x", md);
#endif
	if (type != S_GOODBYE)
	{
	    md->wait = 0;
	    dispatch (type, Message, md);
	    /*
	     * The message may have caused the need to
	     * schedule something, so go back and check.
	     */
	    return(1);
	}
	break;
    }
    return(0);
}

/*
** calculate_nopen() - DETERMINE # FILE DESCRIPTORS AVAILABLE FOR QUEUES
*/

#if	defined(__STDC__)
static void calculate_nopen ( void )	/* funcdef */
#else
static void calculate_nopen()
#endif
{
    ENTRY ("calculate_nopen")

    int		fd, nopen;

    /*
     * How many file descriptorss are currently being used?
     */
    for (fd = nopen = 0; fd < OpenMax; fd++)
	if (fcntl(fd, F_GETFL, 0) != -1)
	    nopen++;

    /*
     * How many file descriptors are available for use
     * as open FIFOs? Leave one spare as a way to tell
     * clients we don't have any to spare (hmmm....) and
     * one for the incoming fifo.
     */

    MaxClients = OpenMax;
    MaxClients -= nopen;	/* current overhead */
    MaxClients -= Reserve_Fds;
    MaxClients -= 2;		/* incoming FIFO and spare outgoing */
    MaxClients--;		/* the requests log */
    MaxClients--;		/* HPI routines and lpsched log */

    return;
}

static void net_shutdown ( )
{
    ENTRY ("net_shutdown")

    note ("The network process has terminated.\n");
    Net_md = NULL;
    lpshut(1);
}

static void conn_shutdown ( )
{
    ENTRY ("conn_shutdown")

    if (!Shutdown)
	note ("The public connection \"%s\", has failed.\n", Lp_FIFO);
    lpshut(1);
}

/*
** init_messages() - INITIALIZE MAIN MESSAGE QUEUE
*/

#if	defined(__STDC__)
void init_messages ( void )	/* funcdef */
#else
void init_messages()
#endif
{
    ENTRY ("init_messages")

    char	*cmd;
    MESG *	md;

    (void) signal(SIGPIPE, SIG_IGN);

    calculate_nopen ();

    if (cmd = makestr(RMCMD, " ", Lp_Public_FIFOs, "/*", (char *)0))
    {
	(void) system(cmd);
	Free(cmd);
    }
    if (cmd = makestr(RMCMD, " ", Lp_Private_FIFOs, "/*", (char *)0))
    {
	(void) system(cmd);
	Free(cmd);
    }

    Message = (char *)Malloc(MSGMAX);

    (void) Chmod(Lp_Public_FIFOs, 0773);
    (void) Chmod(Lp_Private_FIFOs, 0771);
    (void) Chmod(Lp_Tmp, 0711);
    
    if ((md = mcreate(Lp_FIFO)) == NULL)
	fail ("Can't create public message device (%s).\n", PERROR);
    mon_discon(md, conn_shutdown);
    
    if (mlisteninit(md) != 0)
	if (errno == ENOMEM)
	    mallocfail();
	else
	    fail ("Unexpected streams error (%s).\n" , PERROR);

    (void) Chmod(Lp_FIFO, 0666);
    return;
}

void
#if	defined(__STDC_)
init_network ( void )
#else
init_network()
#endif
{
    ENTRY ("init_network")

    int		i;
    int		fds[2];

    if (Net_md)
	return;
    
    if (pipe(fds) == -1)
	fail ("Failed to create pipe for network process (%s).\n", PERROR);
    switch (fork())
    {
      case -1:	/* Failure */
	note ("The fork for the network server has failed.\n");
	lpshut(1);
	break;
	
      case 0:	/* The Child */
	(void) Close(0);
	if (Fcntl(fds[1], F_DUPFD, 0) != 0)
	    exit(99);
	for (i = 1; i <= OpenMax; i++)
		Close(i);
	execl(LPNET, "lpNet", 0);
	exit(100);
	/* NOTREACHED */

      default:	/* The Parent */
	(void) Close(fds[1]);
	Net_fd = fds[0];
	if ((Net_md = mconnect(NULL, fds[0], fds[0])) == NULL)
	{
	    note ("Failed to bind the network process (%s).\n", PERROR);
	    (void) Close(fds[0]);
	    return;
	}
	Net_md->type = MD_CHILD;
	if (mlistenadd(Net_md, POLLIN) == -1)
	{
	    note ("Failed to attach the network process (%s).\n", PERROR);
	    mdisconnect(Net_md);
	    Net_md = NULL;
	    return;
	}
	mon_discon(Net_md, net_shutdown);
    }
}

    
void
#if	defined(__STDC__)
shutdown_messages ( void )	/* funcdef */
#else
shutdown_messages()
#endif
{
    ENTRY ("shutdown_messages")

    MESG	*md;
    
    (void) Chmod(Lp_Public_FIFOs, 0770);
    (void) Chmod(Lp_Tmp, 0700);
    (void) Chmod(Lp_FIFO, 0600);
    md = mlistenreset();
    mdestroy(md);
}

#if	defined(DEBUG)
/*VARARGS1*/
#if	defined(__STDC__)
static void log_message_fact ( char * format , ... )	/* funcdef */
#else
static void log_message_fact (format, va_alist)
    char	*format;
    va_dcl
#endif
{
    ENTRY ("log_message_fact")

    va_list	ap;

    if (debug & DB_MESSAGES)
    {
	FILE		*fp = open_logfile("messages");
	char		buffer[BUFSIZ];

#if	defined(__STDC__)
	va_start (ap, format);
#else
	va_start (ap);
#endif
	if (fp) {
		setbuf (fp, buffer);
		vfprintf (fp, format, ap);
		fputs ("\n", fp);
		close_logfile (fp);
	}
	va_end (ap);
    }
    return;
}
#endif

/*
**	GROAN!!
** A pre-4.0 client wishes to talk. The protocol:
**
**	client			Spooler (us)
**
**	S_NEW_QUEUE  ------->
**				Create 2nd return fifo
**		     <-------   R_NEW_QUEUE on 1st fifo
**	close 1st fifo
**	prepare 2nd fifo
**	S_NEW_QUEUE  ------->
**				R_NEW_QUEUE with authcode
**				on 2nd fifo
**
** Thus this may be the first OR second S_NEW_QUEUE
** message for this client. We tell the difference
** by the "stage" field.
*/

#if	defined(__STDC__)
static void do_msg3_2 ( MESG * md )	/* funcdef */
#else
static void do_msg3_2();
    MESG *	md;
#endif
{
    ENTRY ("do_msg3_2")

    char *	pub_fifo;
    short	stage;
    char *	msg_fifo;
    char *	system;
    int		madenode;

    (void)getmessage (Message, S_NEW_QUEUE, &stage, &msg_fifo,&system);

    if (stage == 0)
    {
	/*
	** Make a new fifo, in the private dir rather
	** than the public dir. Use the original
	** for the ACK or NAK of this message.
	** Make the client the owner (user and group)
	** of the fifo, and make it readable only by the
	** client.
	*/

	md->file = makepath(Lp_Private_FIFOs, msg_fifo, (char *)0);
	pub_fifo = makepath(Lp_Public_FIFOs, msg_fifo, (char *)0);
	if (strlen(md->file)
	    && (madenode = (Mknod(md->file, (S_IFIFO|S_IRUSR|S_IWUSR), 0) == 0))
	    && Chown(md->file, md->uid, md->gid) == 0)
	{
	    /*
	    ** For "mknod()" to have succeeded, the
	    ** desired fifo couldn't have existed
	    ** already. Our creation gave modes
	    ** and owner that should prevent
	    ** anyone except who the client claims
	    ** to be from using the fifo. Thus
	    ** this fifo is ``safe''--anything we put
	    ** in can only be read by the client.
	    */

	    md->state = MDS_32PROTO;
	    if ((md->writefd = Open(pub_fifo, O_WRONLY | O_NDELAY, 0)) < 0)
	    {
		(void) Close(md->readfd);
		(void) Unlink(pub_fifo);
		Free(pub_fifo);
		return;
	    }
	    mputm (md, R_NEW_QUEUE, MOK);
	    Free(pub_fifo);
	    
#if	defined(DEBUG)
	    log_message_fact (
	"      NEW_QUEUE: client %#0x (%s) accepted at stage 0",
		md,
		msg_fifo
	    );
#endif
	}
	else
	{

	    /*
	    ** Another wise guy.
	    **
	    ** Before removing the second fifo, make sure
	    ** we actually created it!  If we did not get far
	    ** enough to set <madenode>, we never created the
	    ** fifo, so don't remove it!  The wise guy may be
	    ** trying to con us!
	    */

	    if (strlen(md->file) && madenode)
		(void)unlink (md->file);
	    Free (pub_fifo);
	    (void) Close(md->readfd);
#if	defined(DEBUG)
	    log_message_fact (
"      NEW_QUEUE: client %#0x (%s) rejected at stage 0 (%s)",
		md,
		msg_fifo,
		PERROR
	    );
#endif
	}
    }
    else
    {
	if (md->state != MDS_32PROTO)
	    return;

	/*
	** The client has just sent the second S_NEW_QUEUE
	** so we will now send back another ACK on the
	** safe fifo.
	*/
	(void) Close(md->writefd);
	
	if ((md->writefd = Open(md->file, O_WRONLY | O_NDELAY, 0)) >= 0)
	{
	    md->state = MDS_32CONNECT;
	    mputm (md, R_NEW_QUEUE, MOK);
#if	defined(DEBUG)
	    log_message_fact (
	"      NEW_QUEUE: client %#0x (%s) accepted at stage 1",
		md,
		msg_fifo
	    );
#endif
	}
	else
	{
	    (void)unlink (md->file);
	    (void) Close(md->readfd);
#if	defined(DEBUG)
	    log_message_fact (
"      NEW_QUEUE: client %#0x (%s) rejected at stage 1 (%s)",
		md,
		msg_fifo,
		PERROR
	    );
#endif
	}
    }
}
