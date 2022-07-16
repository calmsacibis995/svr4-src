/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PP_DEP_H
#define _SYS_PP_DEP_H

#ident	"@(#)head.sys:sys/pp_dep.h	11.2.7.1"
/*
 * PORTS CIO Definitions
 *
 ***** WARNING ***** WARNING ***** WARNING ***** WARNING
 *	This header file is shared by both sw and fw
 ***** WARNING ***** WARNING ***** WARNING ***** WARNING
 */

typedef struct
{
	unsigned char	pc[4];	/* ports codes */
} RAPP, CAPP;

#define CQSIZE		35		/* size one completion queue  */
#define RQSIZE		18		/* size all requeust queues   */
#define NUM_QUEUES	6		/* (5 output) + (1 supply buf)*/

#define NUM_ELEMENTS	10		/* max size rqueues 0->4      */

#define CENTRONICS	4		/* printer rqueue	      */
#define PPPUMP		5		/* ports pump		      */
#define SUPPLYBUF	5		/* input(read) supply buffer  */

#define RAM_START	0
#define RAM_END		32768

#endif	/* _SYS_PP_DEP_H */
