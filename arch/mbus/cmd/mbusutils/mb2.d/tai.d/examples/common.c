/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/examples/common.c	1.3"

/********************************************************/
/*  common.c  --  routines used by send.c and rec.c     */
/********************************************************/
#include "ft.h"

/************************************************************************/
/*                                                                      */
/* common.c contains routines that are used by both send.c and rec.c.   */
/*                                                                      */
/************************************************************************/


/************************************************************************/
/* d_port returns the value found in the environmental variable         */
/* DEST_PORT or the #defined DEST_PORT if the environment variable is   */
/* not set. DEST_PORT is #defined in ft.h.                              */
/*                                                                      */
/* send.c uses this routine to determine the destination port           */
/* id.  This allows the port id to be changed at run-time.              */
/*                                                                      */
/* Note that the destination port id must be the same for both the send */
/* and the receive applications.  The send application (client) sends   */
/* the file to the receive (server) application.  The port id is the    */
/* software address where the message is sent. rec.c uses get_socket    */
/* to get the destination port id.                                      */
/*                                                                      */
/* The environment variables are read as unsigned decimal integers.     */
/*                                                                      */
/************************************************************************/
unsigned short d_port()
{
	char *portstr;
	/* Get destination port from environment */
	portstr = getenv("DEST_PORT");
	if (portstr == 0) /* DEST_PORT not defined in environment*/
	{
		/* Assign default value to portid */
		return(DEST_PORT);
		/* NOTREACHED */
	}
	return((unsigned short)atoi(portstr));
	/* NOTREACHED */
}

/************************************************************************/
/* get_socket takes a pointer to a message info structure and fills     */
/* in the socketid structure with the host id and port id obtained      */
/* from the environmental variables DEST_HOST and DEST_PORT, if defined.*/
/* The port id is taken from the DEST_PORT definition in ft.h if it     */
/* is not defined in the environment.  If the host id is not defined in */
/* environment get_socket "guesses" the host id based upon the          */
/* most common configuration of hosts in slot 3 and slot 5.             */
/*                                                                      */
/* send.c uses get_socket to determine the "address" to send the file   */
/* to.  The host id is the address of the board (usually the same as the*/
/* slot number that the board occupies), and the port id is the address */
/* of the application on that board.                                    */
/*                                                                      */
/* The environment variables are read as unsigned decimal integers.     */
/*                                                                      */
/************************************************************************/
void get_socket(msg_info)
mb2_msginfo* msg_info;
{
	char *string_ptr;
	unsigned short local_host;

	/* Assign destination port id */
	msg_info->socketid.portid = d_port();

	/* Find the host that the application is */
	/* running on. (the local host) */
	local_host = mb2_gethostid();

	/* Get the host id from environment */
	/* If it is not defined, then "guess". */
	if ((string_ptr = getenv("DEST_HOST")) == 0)
	{
		/* Assign default value to hostid */
		/* Processors are usually in slots 3 and 5, so
		*	if local host is 5, assign 3 to dest,
		*	and vice versa.  Otherwise print error
		*	message and exit.
		*/
		switch (local_host)
		{
			case 5:
				msg_info->socketid.hostid = 3;
				break;
			case 3:
				msg_info->socketid.hostid = 5;
				break;
			default:
				fprintf(stderr,
					"Can't guess dest host.\n");
				exit(-1);
		}
	}
	else
	{
		/* Assign value from environment to hostid */
		msg_info->socketid.hostid =
					(unsigned short)atoi(string_ptr);
		if (msg_info->socketid.hostid == local_host)
		{
			fprintf(stderr,
				"Can't send to local host, %d\n", local_host);
			exit(-1);
		}
	}
}

/************************************************************************/
/*	on_intr() is called when any of the set signals are called. 		*/
/*	It just prints a message that notifies the user which signal was    */
/*  caught and calls cleanup().                                         */
/*                                                                      */
/************************************************************************/

void
on_intr(sig)
int sig;
{
	fprintf(stderr, "\n--- INTERRUPT %d---\n", sig);
	cleanup(-1);
	/* NOTREACHED */
}



/************************************************************************/
/* cleanup() closes the open transport endpoint and exit()'s.           */
/* It passes it's argument to exit().  This allows the same             */
/* routine to be used for normal and abnormal termination.              */
/* transport_fd is external so we didn't have to pass it to cleanup().  */
/*                                                                      */
/************************************************************************/

void cleanup(ret_val)
int ret_val;
{
	if (mb2s_closeport(transport_fd) < 0)
	{
		perror("mb2s_closeport");
	}
	exit(ret_val);
}
