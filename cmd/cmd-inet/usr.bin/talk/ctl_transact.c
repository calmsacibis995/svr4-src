/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/talk/ctl_transact.c	1.1.2.1"

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

#include "talk_ctl.h"
#include <sys/time.h>

#define CTL_WAIT 2	/* the amount of time to wait for a 
			   response, in seconds */


    /*
     * SOCKDGRAM is unreliable, so we must repeat messages if we have
     * not recieved an acknowledgement within a reasonable amount
     * of time
     */

ctl_transact(target, msg, type, response)
struct in_addr target;
CTL_MSG msg;
int type;
CTL_RESPONSE *response;
{
    struct sockaddr junk;
    int read_mask;
    int ctl_mask;
    int nready;
    int cc;
    int junk_size;
    struct timeval wait;

    wait.tv_sec = CTL_WAIT;
    wait.tv_usec = 0;

    msg.type = type;

    daemon_addr.sin_addr = target;
    daemon_addr.sin_port = daemon_port;

    ctl_mask = 1 << ctl_sockt;

	/*
	 * keep sending the message until a response of the right
	 * type is obtained
	 */
    do {
	    /* keep sending the message until a response is obtained */

	do {
	    cc = sendto(ctl_sockt, (char *)&msg, sizeof(CTL_MSG), 0,
			&daemon_addr, sizeof(daemon_addr));

	    if (cc != sizeof(CTL_MSG)) {
		if (errno == EINTR) {
			/* we are returning from an interupt */
		    continue;
		} else {
		    p_error("Error on write to talk daemon");
		}
	    }

	    read_mask = ctl_mask;

	    while ((nready = select(32, &read_mask, 0, 0, &wait)) < 0) {
		if (errno == EINTR) {
			/* we are returning from an interupt */
		    continue;
		} else {
		    p_error("Error on waiting for response from daemon");
		}
	    }
	} while (nready == 0);

	    /* keep reading while there are queued messages 
	       (this is not necessary, it just saves extra
	       request/acknowledgements being sent)
	     */

	do {

	    junk_size = sizeof(junk);
	    cc = recvfrom(ctl_sockt, (char *) response,
			   sizeof(CTL_RESPONSE), 0, &junk, &junk_size );
	    if (cc < 0) {
		if (errno == EINTR) {
		    continue;
		}
		p_error("Error on read from talk daemon");
	    }

	    read_mask = ctl_mask;

		/* an immediate poll */

	    timerclear(&wait);
	    nready = select(32, &read_mask, 0, 0, &wait);

	} while ( nready > 0 && response->type != type);

    } while (response->type != type);
}
