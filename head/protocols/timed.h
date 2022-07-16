/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)/usr/src/head/protocols/timed.h.sl 1.1 4.0 12/08/90 60501 AT&T-USL"

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


/*
 * Time Synchronization Protocol
 */

#ifndef _PROTOCOLS_TIMED_H
#define _PROTOCOLS_TIMED_H

#define	TSPVERSION	1
#define ANYADDR 	NULL

struct tsp {
	u_char	tsp_type;
	u_char	tsp_vers;
	short	tsp_seq;
	struct timeval tsp_time;
	char tsp_name[MAXHOSTNAMELEN];
};
 
/*
 * Command types.
 */
#define	TSP_ANY			0	/* match any types */
#define	TSP_ADJTIME		1	/* send adjtime */
#define	TSP_ACK			2	/* generic acknowledgement */
#define	TSP_MASTERREQ		3	/* ask for master's name */ 
#define TSP_MASTERACK		4	/* acknowledge master request */
#define	TSP_SETTIME		5	/* send network time */
#define TSP_MASTERUP		6	/* inform slaves that master is up */
#define	TSP_SLAVEUP		7	/* slave is up but not polled */
#define	TSP_ELECTION		8	/* advance candidature for master */
#define	TSP_ACCEPT		9	/* support candidature of master */
#define	TSP_REFUSE		10	/* reject candidature of master */
#define TSP_CONFLICT		11	/* two or more masters present */
#define	TSP_RESOLVE		12	/* masters' conflict resolution */
#define	TSP_QUIT		13	/* reject candidature if master is up */
#define	TSP_DATE		14	/* reset the time (date command) */
#define TSP_DATEREQ		15	/* remote request to reset the time */
#define TSP_DATEACK		16	/* acknowledge time setting  */
#define	TSP_TRACEON		17	/* turn tracing on */
#define	TSP_TRACEOFF		18	/* turn tracing off */
#define TSP_MSITE		19	/* find out master's site */
#define TSP_MSITEREQ		20	/* remote master's site request */
#define	TSP_TEST		21	/* for testing election algo */

#define	TSPTYPENUMBER		22

#ifdef TSPTYPES
char *tsptype[TSPTYPENUMBER] =
  { "ANY", "ADJTIME", "ACK", "MASTERREQ", "MASTERACK", "SETTIME", "MASTERUP", 
  "SLAVEUP", "ELECTION", "ACCEPT", "REFUSE", "CONFLICT", "RESOLVE", "QUIT", 
  "DATE", "DATEREQ", "DATEACK", "TRACEON", "TRACEOFF", "MSITE", 
  "MSITEREQ", "TEST" };
#endif

#endif /*!_PROTOCOLS_TIMED_H*/
