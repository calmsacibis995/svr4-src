/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/examples/ft.h	1.3"

/*************************************************************/
/* ft.h  --  include file for file transfer application      */
/*************************************************************/

/************************************************************************/
/* This include file is used by all the modules of the file transfer    */
/* application - common.c, send.c and rec.c.   It #includes all header  */
/* files needed by these modules and #defines various symbolic constants*/
/* and macros that are used by the modules.                             */
/*                                                                      */
/* The forward declarations for the various functions used by the       */
/* modules are placed here.                                             */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <malloc.h>
#include <macros.h>
#include <errno.h>
#include <sys/mb2taiusr.h>	/* Needed in order to use TAI. */

/* Default values for destination */
#define DEST_PORT 2049
#define DEST_HOST 5

/* Definitions of commands (only one implemented for now) */
#define SEND_FILE 1

/* Definition of response code */
#define SUCCESS 0
#define FAILURE 1

/* Miscellaneous defines */
#define READ_CHUNK 4096		/* The client will read files in
								READ_CHUNK sized pieces */
#define DEST_DIR "/tmp"		/* This is where the server
								will place received files */


/*	These are macros for accessing the information in the
*	user data portion of our messages
*/

/*	The request message is 16 bytes which is the most data that can be
*	sent in the control part of a solicited request message.
*	The request message is laid out as follows:
*		The first two bytes are the request "command".
*		Initially there is only one command, SENDFILE.
*		This field allows easy future expansion without too
*		much trouble.
*
*		The next 14 bytes depend upon the command, but in
*		the case of SENDFILE, they are the filename of the
*		file being sent.  The filename is NULL terminated
*		except in the case where it is 14 characters long.
*		A NULL in the first character is valid and means there
*		is no filename.
*		The send program will read from standard input
* 		if it is invoked with no filename on the command line.
*		In this case, the server will write
*		the file to it's standard output.
*
*	If structures were guaranteed to not be padded this user data
*	would be defined as:
*			struct _send_dbuf {
*				short command;
*				char filename[14];
*			};
*		
*	Since structures can be padded the following macros are used to
*	access the elements of the request message.
*/

#define REQ_SIZE 16
#define COMMAND_OFFSET 0
#define FILE_OFFSET 2
/* user_command() returns unsigned short */
#define user_command(p) *(unsigned short *)(&p[COMMAND_OFFSET])
#define user_filename(p) &p[FILE_OFFSET] /*returns pointer*/

/*	The response message is 20 bytes which is the most data that can be
*	sent in the control part of a unsolicited response message.
*	The response message is laid out as follows:
*		The first two bytes are the response result code.
*		0 indicates a successful file transfer and other
*		values are error conditions.
*
*		The next 4 bytes are a count of the number of bytes written.
*		If the result code is 0, this should be the same as the number
*		of bytes sent.  If there is an error, this may be 0 or some
*		number less than the amount sent.
*
*		The next 14 bytes are not defined for the SEND_FILE
*		transaction.
*
*	If structures were guaranteed to not be padded this user data
*	would be defined as:
*			struct _reply_dbuf {
*				short response;
*				int count;
*				char fill[14];
*			};
*		
*	Since structures can be padded the following macros are used to
*	access the elements of the response message.
*/

#define RESP_SIZE 20
#define RESP_OFFSET 0
#define COUNT_OFFSET 2
/* user_response returns unsigned short */
#define user_response(p) *(unsigned short *)(&p[RESP_OFFSET])
/* user_count returns unsigned */
#define user_count(p) *(unsigned *)(&p[COUNT_OFFSET])


/* Externals */
extern int transport_fd;	/*	Need this so routines that */
							/*	use this can be in common.c */


/* Forward declarations */
extern char *getenv();
extern unsigned short d_port();
extern void on_intr();
mb2_buf *read_in();
char *fname();
void cleanup();
void terminate();
void send_fail();
void cancel();
void get_socket();
