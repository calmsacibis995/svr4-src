/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/otserror.h	1.3"

/*
** ABSTRACT:	OTS protocol errors
**
** MODIFICATIONS:
*/

/* OTS Exception Codes */

#define E_OK			0	/* successful OTS transaction */
#define E_NO_PASSIVE_OPEN	1	/* no passive open for connect */
#define E_NO_RESOURCES		2	/* no resources for connection */
#define E_NO_CONNECTION		3	/* nothing to disconnect */
#define E_PROTOCOL_VERSION	4	/* unsupported OTS protocol version */
#define E_DATAGRAM		5	/* couldn't deliver datagram */
