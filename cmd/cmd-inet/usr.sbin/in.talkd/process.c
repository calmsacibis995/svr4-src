/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.talkd/process.c	1.2.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


    /* process.c handles the requests, which can be of three types:

		ANNOUNCE - announce to a user that a talk is wanted

		LEAVE_INVITE - insert the request into the table
		
		LOOK_UP - look up to see if a request is waiting in
			  in the table for the local user

		DELETE - delete invitation

     */

#include "ctl.h"
char *strcpy();
CTL_MSG *find_request();
CTL_MSG *find_match();

process_request(request, response)
CTL_MSG *request;
CTL_RESPONSE *response;
{
    CTL_MSG *ptr;

    response->type = request->type;
    response->id_num = 0;

    switch (request->type) {

	case ANNOUNCE :

	    do_announce(request, response);
	    break;

	case LEAVE_INVITE :

	    ptr = find_request(request);
	    if (ptr != (CTL_MSG *) 0) {
		response->id_num = ptr->id_num;
		response->answer = SUCCESS;
	    } else {
		insert_table(request, response);
	    }
	    break;

	case LOOK_UP :

	    ptr = find_match(request);
	    if (ptr != (CTL_MSG *) 0) {
		response->id_num = ptr->id_num;
		response->addr = ptr->addr;
		response->answer = SUCCESS;
	    } else {
		response->answer = NOT_HERE;
	    }
	    break;

	case DELETE :

	    response->answer = delete_invite(request->id_num);
	    break;

	default :

	    response->answer = UNKNOWN_REQUEST;
	    break;

    }
}

struct hostent *gethostbyaddr();

do_announce(request, response)
CTL_MSG *request;
CTL_RESPONSE *response;
{
    struct hostent *hp;
    CTL_MSG *ptr;
    int result;

	/* see if the user is logged */

    result = find_user(request->r_name, request->r_tty);

    if (result != SUCCESS) {
	response->answer = result;
	return;
    }

    hp = gethostbyaddr(&request->ctl_addr.sin_addr,
			  sizeof(struct in_addr), AF_INET);

    if ( hp == (struct hostent *) 0 ) {
	response->answer = MACHINE_UNKNOWN;
	return;
    }

    ptr = find_request(request);
    if (ptr == (CTL_MSG *) 0) {
	insert_table(request,response);
	response->answer = announce(request, hp->h_name);
    } else if (request->id_num > ptr->id_num) {
	    /*
	     * this is an explicit re-announce, so update the id_num
	     * field to avoid duplicates and re-announce the talk 
	     */
	ptr->id_num = response->id_num = new_id();
	response->answer = announce(request, hp->h_name);
    } else {
	    /* a duplicated request, so ignore it */
	response->id_num = ptr->id_num;
	response->answer = SUCCESS;
    }

    return;
}

#include <utmp.h>

/*
 * Search utmp for the local user
 */

find_user(name, tty)
char *name;
char *tty;
{
    struct utmp ubuf;

    int fd;

    if ((fd = open(UTMP_FILE, 0)) == -1) {
	print_error("Can't open /etc/utmp");
	return(FAILED);
    }

    while (read(fd, (char *) &ubuf, sizeof ubuf) == sizeof(ubuf)) {
	if (strncmp(ubuf.ut_name, name, sizeof ubuf.ut_name) == 0) {
	    if (*tty == '\0') {
		    /* no particular tty was requested */
		(void) strcpy(tty, ubuf.ut_line);
		close(fd);
		return(SUCCESS);
	    } else if (strcmp(ubuf.ut_line, tty) == 0) {
		close(fd);
		return(SUCCESS);
	    }
	}
    }

    close(fd);
    return(NOT_HERE);
}
