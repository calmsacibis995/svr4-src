/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/talk/get_addrs.c	1.3.2.1"

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

#ifdef SYSV
#define	bcmp(a,b,c)	memcmp((a),(b),(c))
#define	bcopy(a,b,c)	memcpy((b),(a),(c))
#endif /* SYSV */

struct hostent *gethostbyname();
struct servent *getservbyname();

get_addrs(my_machine_name, rem_machine_name)
char *my_machine_name;
char *rem_machine_name;
{
    struct hostent *hp;
    struct servent *sp;

    msg.pid = getpid();

	/* look up the address of the local host */

    hp = gethostbyname(my_machine_name);

    if (hp == (struct hostent *) 0) {
	printf("This machine doesn't exist. Boy, am I confused!\n");
	exit(-1);
    }

    if (hp->h_addrtype != AF_INET) {
	printf("Protocal mix up with local machine address\n");
	exit(-1);
    }

    bcopy(hp->h_addr, (char *)&my_machine_addr, hp->h_length);

	/* if on the same machine, then simply copy */

    if ( bcmp((char *)&rem_machine_name, (char *)&my_machine_name,
		sizeof(rem_machine_name)) == 0) {
	bcopy((char *)&my_machine_addr, (char *)&rem_machine_addr,
		sizeof(rem_machine_name));
    } else {

	if ((rem_machine_addr.s_addr = inet_addr (rem_machine_name)) == -1) {

	    /* look up the address of the recipient's machine */

	    hp = gethostbyname(rem_machine_name);

	    if (hp == (struct hostent *) 0 ) {
	        printf("%s is an unknown host\n", rem_machine_name);
	        exit(-1);
	    }

	    if (hp->h_addrtype != AF_INET) {
	        printf("Protocol mix up with remote machine address\n");
	        exit(-1);
	    }

	    bcopy(hp->h_addr, (char *) &rem_machine_addr, hp->h_length);
	}
    }


    /* find the daemon portal */

#ifdef NTALK
    sp = getservbyname("ntalk", "udp");
#else
    sp = getservbyname("talk", "udp");
#endif

    if (strcmp(sp->s_proto, "udp") != 0) {
	printf("Protocol mix up with talk daemon\n");
	exit(-1);
    }

    if (sp == 0) {
	    p_error("This machine doesn't support a tcp talk daemon");
	    exit(-1);
    }

    daemon_port = sp->s_port;
}

