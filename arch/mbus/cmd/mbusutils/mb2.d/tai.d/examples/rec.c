/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/examples/rec.c	1.3"

/* :se tabstop=4 shiftwidth=4 */
/*************************************************************/
/*	rec.c  - receive files transferred across MBII using TAI */
/*************************************************************/
#include "ft.h"

int transport_fd;	/*	This is external so we can */
					/*	use it in interrupt routine */

main(argc, argv)
int argc;			/* UNUSED */
char *argv[];		/* UNUSED */
{
	FILE *file;		/*	The FILE pointer used to create */
					/*	and write the transferred file */

	unsigned short dest_port;	/*	The well known port id */
								/*	of this server */

	/*******************************************************/
	/* These next 3 declarations are allocating the buffer */
	/* structures for the various parts of the messages.   */
	/*******************************************************/

	mb2_buf req_cbuf;			/* control part of request */
	mb2_buf req_dbuf;			/* data part of request */
	mb2_buf reply_cbuf;			/* control part of response */
	/* No data part of the response - it is unsolicited. */

	/*******************************************************/
	/* These two declarations allocate the space for the   */
	/* control parts of the request and response.  The     */
	/* data part of the request will be malloced when we   */
	/* know how much space we need to hold the message.    */
	/*                                                     */
	/* A pointer in the appropriate mb2_buf structure will */
	/* set to point to these later.                        */
	/*******************************************************/

	char response[UNSOL_LENGTH];/* response control buffer */
	char request[UNSOL_LENGTH];	/* This holds the control */
								/* part of the request    */
								/* message.  It is long   */
								/* enough for an          */
								/* unsolicited message so */
								/* that the case of zero  */
								/* length files is covered.*/

	mb2_msginfo msg_info;	/* the message info structure */

	/**************************************************/
	/*	This is used to store the filename.  The only */
	/*	reason we need this so that we have room for  */
	/*	a 14 character filename plus a NULL.          */
	/*	The layout of the control buffer of the       */
	/*	request message has 14 bytes for the filename.*/
	/**************************************************/
	char filename[15];

	/* Catch SIGINT, SIGHUP, and SIGTERM signals. */
	/* on_intr() will clean up and exit */
	sigset (SIGINT, on_intr);
	sigset (SIGHUP, on_intr);
	sigset (SIGTERM, on_intr);

	/*	Received files are placed in a spool directory.
	*	Change directory to the spool directory.
	*/
	if (chdir(DEST_DIR) < 0)
	{
		perror("chdir");
		exit(-1);
	}

	/*	Get destination port from environment.
	*	See d_port() in common.c
	*/
	dest_port = d_port();


	/*	In order to receive a message, we need to first
	*	obtain a transport endpoint.  mb2s_openport
	*	is used for this.
	*	We use a well known port number (dest_port)
	*	so that the clients will know who to talk to.
	*	The NULL as the second argument says set
	*	the endpoint option parameters to default values.
	*/
	
	if ((transport_fd = mb2s_openport(dest_port, NULL)) < 0)
	{
		perror("mb2s_openport");
		exit(-1);
	}

	fprintf(stderr, 
			"Host: %u\nReady to receive a file on port %u\n",
			mb2_gethostid(),
			dest_port);


	/*	Before receiving the message, we fill in the
	*	various structures with the correct values
	*/

	/*	Set up the buffer structure for the control part
	*	of the request message.
	*	req_cbuf.len will be set by the mb2s_receive
	*	to indicate the amount of data actually received
	*/
	req_cbuf.maxlen = UNSOL_LENGTH;
	req_cbuf.buf = request;

	/*	Set the buffer structure for the data part
	*	of the request message.
	*	This is needed even though we are receiving no
	*	solicited data right now, so that the transport
	*	can fill in the .len field and tell us how much
	*	data we should receive.
	*/
	req_dbuf.maxlen = 0;
	req_dbuf.buf = NULL;


	/* Receive the message. */

	/*	We receive a 0 length message to simplify the logic
	*	later in the program.  This means we follow the
	*	same logic no mater what the size of the file is.
	*/
	if (0 != mb2s_receive(
		transport_fd,	/* transport endpoint */
		&msg_info,		/* message info pointer */
		&req_cbuf,		/* control part of request message */
		&req_dbuf))		/* data psrt of request message */
	{
		if (errno == MB2_MORE_DATA)
		/**********************************************/
		/*	This is what we expect, since we set the  */
		/*	maxlen field of req_dbuf to 0.            */
		/**********************************************/
		{
			/*	Now we set up the req_dbuf to receive the file.
			*	First we allocate the space.
			*/
			if ((req_dbuf.buf = malloc(req_dbuf.len)) == 0)
			{
				fprintf(stderr, "malloc failed\n");
				/*	Can't be in fragmentation state here,
				*	so we don't terminate()
				*/
				cleanup(-1);
				/* NOTREACHED */
			}
			req_dbuf.maxlen = req_dbuf.len;
			if (0 != mb2s_receive(
				transport_fd,		/* transport endpoint */
				NULL,				/* This must be NULL  */
									/* after MB2_MORE_DATA */
				NULL,				/* This must be NULL  */
									/* after MB2_MORE_DATA */
				&req_dbuf))
			{
			/*	If this receive fails
			*	we have to close the port and exit.
			*	cleanup takes care of that.
			*/
			perror("mb2s_receive");
			cleanup(-1);
			/* NOTREACHED */
			}
		}
		else
		{
			/* If the receive failed for another reason
			*	we have to close the port and exit.
			*	cleanup takes care of that.
			*/
			perror("mb2s_receive");
			cleanup(-1);
			/* NOTREACHED */
		}
	}
	/*	We fall through to here if:
	*		1) The first receive worked.  That means we got an unsolicited
	*			message or a request message requiring fragmentation.
	*			Note that we did not do anything to cause fragmentation.
	*			The transport may have fragmented the message if
	*			there was not enough buffer space to hold the message
	*			until the receive gets called.
	*		2) The second receive worked.  That means we got a
	*			solicited message.
	*
	*/
	switch (msg_info.msgtyp)
	{

		/*	These are for keeping track of how much          */
		/*	data we've received in the case of fragmentation */
		int whats_left, total_got;		

		/*	This is used to point to the buffer that   */
		/*	is malloc'd for a fragmented message       */
		char *buffer;

		case MB2_REQFRAG_MSG:

			/*	Since only 32 KBytes -1 of data can be received from the
			*	TAI, we request fragments in 32 KByte chunks or 
			*	whatever's left.
			*	Note that we may not get what we requested, so we
			*	have to look at req_dbuf.len to see what we got
			*	after the receive.  It may not match req_dbuf.max_len.
			*
			*	First we allocate enough memory for the whole file.
			*/

			if ((buffer = malloc(req_dbuf.len)) == 0)
			{
				fprintf(stderr, "malloc failed\n");
				/* need to end fragmentation state here */
				terminate(&msg_info);
				send_fail(&msg_info);
				/* NOTREACHED */
			}
			whats_left = req_dbuf.len;
			total_got = 0;
			while (whats_left > 0)	/* we still have to   */
									/* receive more frags */
			{
				/*******************************************/
				/*	Before receiving the message, we fill  */
				/*	in the request data buf structure with */
				/*	the correct values. We set the buffer  */
				/*	pointer in the field to point into the */
				/*	memory we allocated just past what     */
				/*	we've already received.                */
				/*******************************************/
				req_dbuf.buf = buffer + total_got;
				req_dbuf.maxlen = req_dbuf.len =
						min(whats_left, 32767);
				if (0 != mb2s_getreqfrag(
					transport_fd,	/* transport endpoint */
					&msg_info,		/* message info struct */
					&req_dbuf))
				{

					/****************************************/
					/*	If this fails there isn't much that */
					/*	can be done. Any legal cleanup path */
					/*	involves doing a getreqfrag(), so   */
					/*	we just give up and cleanup().      */
					/****************************************/
					perror("mb2s_getreqfrag");
					cleanup(-1);
					/* NOTREACHED */
				}
				/*	Now subtract what we got from
				*	whats_left and add it to the total_got
				*/
				whats_left -= req_dbuf.len;
				total_got += req_dbuf.len;
			}
			/*	We've finished receiving the fragments.
			*	This next assignments are so that when we get
			*	to the next switch statement we can use the
			*	req_dbuf structure to get the information
			*	we need to write the file.
			*	This makes req_dbuf look the same whether
			*	we fragmented or not.
			*/
			req_dbuf.buf = buffer;
			req_dbuf.len = total_got;
			break;
		case MB2_REQ_MSG:
			/* This is the normal case with no fragmentation */
			break;
		case MB2_NTRAN_MSG:
			fprintf(stderr,
				"Received non-transaction message\n");
			cleanup(-1);
			/* NOTREACHED */
		default:
			fprintf(stderr,
			  "Got a message I don't know what to do with\n");
			cancel(&msg_info);
			/* NOTREACHED */
	}
	/*	The switch statement is used so that the
	*	application can easily be extended later.
	*	Right now there is only one command - SEND_FILE
	*/
	switch (user_command(req_cbuf.buf))
	{
		case SEND_FILE:
			/*	Open the file.
			*	The only reason we can't use req_cbuf.buf
			*	directly in the open call is that it
			*	points to a string that may not be null
			*	terminated if the filename is 14 characters.
			*
			*	NOTE: we really should check here that the
			*	filename is valid and not a pathname, so
			*	that we don't create a file with non-ASCII
			*	characters or overwrite /unix or something.
			*/
			strncpy(filename, user_filename(req_cbuf.buf), 14);
			filename[14] = '\0'; /*for the case where
									filename is 14 chars*/
			if (filename[0] == '\0')
			{
				/*	No filename was given, use standard out. */
				file = stdout;
			}
			else
			{
				if ((file = fopen(filename, "w")) == NULL)
				{
					perror("fopen");
					send_fail(&msg_info);
					/* NOTREACHED */
				}
				
			}
			if (req_dbuf.len > fwrite(
				req_dbuf.buf,	/* pointer to the data */
				1,				/* size of element (char) */
				req_dbuf.len,	/* length of this segment */
				file))			/* the open file */
			{
				/*	For some reason, we cannot write the
				*	file correctly.
				*	We may have reached the ULIMIT, or be
				*	out of disk space.  Whatever the reason,
				*	we have to tell the sender that the file
				*	transfer did not work.  But first,
				*	remove the file that we have created.
				*/
				perror("fwrite");
				if (unlink(filename) != 0)	/* exit() will
											close the file */
				{
					perror("unlink");
				}
				send_fail(&msg_info);
				/* NOTREACHED */
			}
			/* Notify the user that the file was received */
			fprintf(stderr,
					"Received file: \"%s\", %u bytes\n",
					filename, req_dbuf.len);
			/* Prepare the response */
			reply_cbuf.len = RESP_SIZE;
			reply_cbuf.maxlen = RESP_SIZE;
			reply_cbuf.buf = response;
			user_response(reply_cbuf.buf) = SUCCESS;
			user_count(reply_cbuf.buf) = req_dbuf.len;

			/* Now we send the reply. It is unsolicited. */
			if ( 0 != mb2s_sendreply(
				transport_fd,		/* transport endpoint */
				&msg_info.socketid,	/* remote socket id ptr*/
				&reply_cbuf,		/* control part of reply */
				NULL,				/* reply is unsolicited */
				msg_info.tid,		/* transaction id */
				MB2_EOT))			/* Last fragment */
		 	{
		 		perror("mb2s_sendreply");
				cancel(&msg_info);
				/* NOTREACHED */
		 	}
			break;
		default:
			fprintf(stderr,
				"Received message with unknown command: %u\n",
				user_command(req_cbuf.buf));

			/*	We have received a message that we do not know
			*	how to interpret.  We don't send a failure
			*	message because whatever sent us this message
			*	is not talking my language, so who knows what
			*	it would do with the response.
			*	Just cancel the transaction.
			*/

			cancel(&msg_info);
			/* NOTREACHED */
	}
	cleanup(1);
	/* NOTREACHED */
}




/************************************************************************/
/*	terminate() terminates the request phase of a transaction.  		*/
/*	It is used when we don't want to receive the rest of a message      */
/*  that is in fragmentation state.                                     */
/*                                                                      */
/************************************************************************/
void terminate(msg_info_ptr)
mb2_msginfo *msg_info_ptr;
{
	/************************************************************************/
 	/*	A get request fragment call with a 0 length buffer                  */
	/*	tells the sender that we don't need the rest of the data			*/
	/*                                                                      */
	/*  Note that the result of canceling a message that is in              */
	/*  fragmented state is not defined by the transport specification.     */
	/************************************************************************/
	mb2_buf data_buffer;
	data_buffer.len = 0;
	if (0 != mb2s_getreqfrag(
		transport_fd,	/*	transport endpoint */
		msg_info_ptr,	/*	ptr to message info structure */
		&data_buffer))	/* .len =0 terminates the request */
						/*	phase of transaction          */
	{
		/*	If this fails we try to cancel. */
		perror("mb2s_getreqfrag");
		cancel(msg_info_ptr);
		/* NOTREACHED */
	}
}


/************************************************************************/
/*	send_fail() sends a response message back to the client referred    */
/*	to by msg_info_ptr.  The response message is in the format defined  */
/*  in ft.h.  A FAILURE indication is placed in the proper place        */
/*  in the unsolicited response and the count field is set to 0.        */
/*                                                                      */
/*  Note that the contents of reply_cbuf.buf are defined by the         */
/*  application, not the transport protocol or interface.               */
/*                                                                      */
/************************************************************************/

void send_fail(msg_info_ptr)
mb2_msginfo *msg_info_ptr;
{
	char response[RESP_SIZE];
	mb2_buf reply_cbuf;
	/* Prepare the response */
	reply_cbuf.len = RESP_SIZE;
	reply_cbuf.maxlen = RESP_SIZE;
	reply_cbuf.buf = response;
	user_response(reply_cbuf.buf) = FAILURE;
	user_count(reply_cbuf.buf) = 0;
	/* Now we send the reply. It is unsolicited. */
	if ( 0 != mb2s_sendreply(
		transport_fd,				/* transport endpoint */
		&msg_info_ptr->socketid,	/* remote socket id */
		&reply_cbuf,				/* reply control part */
		NULL,						/* reply is unsolicited */
		msg_info_ptr->tid,			/* transaction id */
		MB2_EOT))					/* Last fragment */
 	{
 		perror("mb2s_sendreply");
		cancel(msg_info_ptr);
		/* NOTREACHED */
 	}
	cleanup(-1);
	/* NOTREACHED */
}
/************************************************************************/
/*	cancel() sends a cancel message to the client referred to by        */
/*	msg_info_ptr.   It assumes that the request phase of the transaction*/
/*  is already completed.   It also calls cleanup(), since this         */
/*  application is always finished when the transaction is finished.    */
/*                                                                      */
/************************************************************************/
void cancel(msg_info_ptr)
mb2_msginfo *msg_info_ptr;
{
	if ( 0 != mb2s_sendcancel(		/* if cancel unsuccessful*/
		transport_fd,				/* transport endpoint */
		&msg_info_ptr->socketid,	/* remote socket id */
		msg_info_ptr->tid))			/* transaction id */
	{
		perror("mb2s_sendcancel");
	}
	cleanup(-1);
	/* NOTREACHED */
}
