/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/talk/ctl.h	1.2.2.1"

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


/* ctl.h describes the structure that talk and talkd pass back
   and forth
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define NAME_SIZE 9
#define TTY_SIZE 16
#define HOST_NAME_LENGTH 256

#define MAX_LIFE 60 /* maximum time an invitation is saved by the
			 talk daemons */
#define RING_WAIT 30  /* time to wait before refreshing invitation 
			 should be 10's of seconds less than MAX_LIFE */

    /* the values for type */

#define LEAVE_INVITE 0
#define LOOK_UP 1
#define DELETE 2
#define ANNOUNCE 3

    /* the values for answer */

#define SUCCESS 0
#define NOT_HERE 1
#define FAILED 2
#define MACHINE_UNKNOWN 3
#define PERMISSION_DENIED 4
#define UNKNOWN_REQUEST 5

typedef struct ctl_response CTL_RESPONSE;

struct ctl_response {
    char type;
    char answer;
    int id_num;
    struct sockaddr_in addr;
};

typedef struct ctl_msg CTL_MSG;

struct ctl_msg {
    char type;
    char l_name[NAME_SIZE];
    char r_name[NAME_SIZE];
    int id_num;
    pid_t pid;
    char r_tty[TTY_SIZE];
    struct sockaddr_in addr;
    struct sockaddr_in ctl_addr;
};
