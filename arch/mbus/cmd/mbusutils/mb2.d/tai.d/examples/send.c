/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/examples/send.c	1.3"

/* :se tabstop=4 shiftwidth=4 */
/*******************************************************/
/*  send.c  --  send files across MBII using TAI       */
/*******************************************************/
#include "ft.h"

int transport_fd;	/*	This is external so we can use */
					/*	it in the interrupt routine. */

main(argc, argv)
int argc;
char *argv[];
{
	char *filename;				/* points to the file name */
	mb2_msginfo msg_info;		/* message info */

	/*******************************************************/
	/* These next 3 declarations are allocating the buffer */
	/* structures for the various parts of the messages.   */
	/*******************************************************/

	mb2_buf req_cbuf;			/* control part of request */
	mb2_buf *req_dbuf_ptr;		/* data part of request */
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

	char request[REQ_SIZE];		/* request control buffer */
	char response[RESP_SIZE];	/* response control buffer */

	/* Catch SIGINT, SIGHUP, and SIGTERM signals. */
	/* on_intr() will clean up and exit */
	sigset (SIGINT, on_intr);
	sigset (SIGHUP, on_intr);
	sigset (SIGTERM, on_intr);

	/*	Parse the command line arguments.
	*	The syntax of send is:
	*        send [filename]
	*	If no filename is specified,
	*	send reads from standard input.
	*/
	switch (argc)
	{
		case 1:
			filename = "";
			break;
		case 2:
			/************************************************/
			/*	If a filename was provided on the command   */
			/*	line, substitute that file for standard     */
			/*	input.  This way the rest of this           */
			/*	program can read from standard input.       */
			/************************************************/
			if (freopen(argv[1], "r", stdin) == NULL)
			{
				perror("freopen");
				fprintf(stderr, "Couldn't open %s\n", argv[1]);
				exit(-1);
			}
			filename = fname(argv[1]);
			break;
		default:
			fprintf(stderr,"Usage: %s [filename]\n",argv[0]);
			exit(-1);
	}

	/* Now we read in the file or standard input */
	req_dbuf_ptr = read_in(stdin);

	/*	In order to send a message, we need to first
	*	obtain a transport endpoint.  mb2s_openport
	*	is used for this.
	*	We don't care what the port id is because
	*	the only messages we will receive are responses to
	*	our requests, and
	*	the message that we will be sending to the server
	*	will contain what is effectively our "return address".
	*	Therefore, we call mb2s_openport with "0" as the
	*	first argument, which says assign any available port.
	*	The NULL as the second argument	says set the
	*	endpoint option parameters to default values.
	*/
	if ((transport_fd = mb2s_openport(0, NULL)) < 0)
	{
		perror("mb2s_openport");
		exit(-1);
	}

 	/* Fill in the control part of the request message */
 	user_command(request) = SEND_FILE;
 	strncpy(user_filename(request), filename, 14);

	/* set up control buf struct for the request message */
	req_cbuf.maxlen = REQ_SIZE;
	req_cbuf.len = REQ_SIZE;
	req_cbuf.buf = request;

	/* set up control buf struct for the response message */
	reply_cbuf.maxlen = RESP_SIZE;
	reply_cbuf.buf = response;
	
	/*	Get the socket ID from environment and put it
	*	in the msg_info structure.
	*	This is the "address" that we are sending the
	*	message to.
	*/
	get_socket(&msg_info);
	
	fprintf(stderr,
		"\nSending file \"%s\" to host %u, port %u\n",
		filename, msg_info.socketid.hostid, msg_info.socketid.portid);

	/* Now we send the message */
	if ( 0 != mb2s_sendrsvp(
			transport_fd,	/* transport endpoint */
			&msg_info,		/* pointer to message info */
			&req_cbuf,		/* pointer to request control buf*/
			req_dbuf_ptr,	/* pointer to request data buf */
			&reply_cbuf,	/* pointer to reply control buf */
			NULL))			/* reply will be unsolicited */
 	{
 		perror("mb2s_sendrsvp");
 		cleanup(-1);
		/* NOTREACHED */
 	}
	/*	The synchronous version of sendrsvp returns when the
	*	response has been received.  Therefore we do not have
	*	to do any kind of receive call.  All we have to do is
	*	examine the reply we received and see whether the
	*	transfer was successful.
	*/

 	/* get the information from the reply */
	switch (msg_info.msgtyp)
	{
		case MB2_CANCEL_MSG:
			/*	A cancel message can be received for several reasons.
			*	If there is no application that has opened the port
			*	that we sent the message to (assuming a UNIX host),
			*	then the transport software will send us a cancel message.
			*	The most likely scenario is that the rec program
			*	is not running or we sent the message to the wrong
			*	"address"
			*	
			*	Another possibility is that there is an application at
			*	the "address", but it is not the rec program and
			*	therefore does not understand the format of our message.
			*	The application would then send the cancel message.
			*/
			fprintf(stderr,
			  "Message was canceled.\nIs receiver running?\n");
			cleanup(-1);
			/* NOTREACHED */
		case MB2_RESP_MSG:
			/*	This is what we expect in the normal case.
			*	We have received a response message.
			*	Now we analyze the result code to see if
			*	the transfer was successful.
			*/
			if (user_response(reply_cbuf.buf) != 0)
			{
				/* response is an error indication */
				fprintf(stderr, "File transfer failed!\n");
				fprintf(stderr,
					"Error code returned from server: %u\n",
					user_response(reply_cbuf.buf));
				fprintf(stderr,
					"%u bytes out of %u total received \
					by server\n",
					user_count(reply_cbuf.buf),
					req_dbuf_ptr->len);
				cleanup(-1);
				/* NOTREACHED */
			}
			/* The result code indicated success */
			if (user_count(reply_cbuf.buf) !=
									req_dbuf_ptr->len)
			{
				/*********************************/
				/*	Result is success,           */
				/*	but count is not what I sent */
				/*********************************/
				fprintf(stderr, "This should never happen.\n");
				cleanup(-1);
				/* NOTREACHED */
			}
			fprintf(stderr, "File successfully sent.\n");
			cleanup(0);
			/* NOTREACHED */
		default:
			fprintf(stderr, "We should never see this\n");
			cleanup(1);
			/* NOTREACHED */
	}
}




/************************************************************************/
/*  fname() takes a pointer to a path name and returns a pointer to     */
/*  the filename component of the path.  It uses a static array to      */
/*  hold the pathname and returns a pointer to the right place in that  */
/*  array.  Therefore, the data is overwritten by subsequent instances  */
/*  of this call.  The pathname is limited to 255 characters.           */
/*                                                                      */
/************************************************************************/
char *fname(path)
char *path;
{
	static char temp[256];
	int i,file_name;
	strncpy(temp, path, 255);
	file_name = 0;
	i = 0;
	while (temp[i] != 0)
	{
		if (temp[i] == '/')
		{
			file_name = i+1;
		}
	i++;
	}
	return(&temp[file_name]);
}

/************************************************************************/
/*  read_in() mallocs the space for a mb2_buf structure and reads       */
/*  in a file READ_CHUNK bytes at a time, allocating the space for      */
/*  the file as it goes.  It sets up the mb2_buf structure correctly    */
/*  with the buf field pointing to the file buffer and the max_len      */
/*  and len fields set to the length of the file. It returns a          */
/*  pointer to the mb2_buf structure.                                   */
/*                                                                      */
/************************************************************************/

mb2_buf* read_in(file)
FILE *file;
{
	mb2_buf* data_buf;
	int amount_read;
	/********************************************************/
	/*	First we allocate memory for the mb2_buf structure  */
	/*	(so far we've only declared pointers to mb2_buf     */
	/*	structures)                                         */
	/********************************************************/
	if ((data_buf = (mb2_buf *)malloc(sizeof(mb2_buf))) == 0)
	{
		perror("malloc");
		exit(-1);
	}

	/*	Now we read in the file by READ_CHUNK blocks,
	*	allocating memory as we go along.
	*	We keep track of the amount read and the location
	*	of the memory in the mb2_buf structure.
	*/
	if ((data_buf->buf = malloc(READ_CHUNK)) == 0)
	{
		perror("malloc");
		exit(-1);
	}
	while (0 != (amount_read = fread(
		data_buf->buf + data_buf->len,
		1,
		READ_CHUNK,
		file)))
	{
		data_buf->len += amount_read;
		if (amount_read < READ_CHUNK)
		{
			break;
		}
		if (0 == (data_buf->buf = realloc(
			data_buf->buf,
			data_buf->len + READ_CHUNK)))
		{
			perror("realloc");
			exit(-1);
		}
	}
	data_buf->maxlen = data_buf->len;
	return (data_buf);
}
