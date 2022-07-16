/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/xprt.h	1.3"

#ifdef MBIIU
#define NULL_ST   
#define alien NULL_ST 
#endif

extern int ics_read();
extern int ics_write();
extern int ics_find_rec();
extern int mb2s_getinfo();
extern int mb2s_openport();
extern int mb2s_send();
extern unsigned short mb2_gethostid();
extern int mb2s_closeport();

#ifdef RMX286
extern char alien *cstr();
extern void alien rq$sleep();
extern TOKEN alien rq$create$port();
extern WORD alien rq$get$host$id();
extern char alien rq$get$interconnect();
extern void alien rq$set$interconnect();
extern void alien rq$get$port$attributes();
extern WORD alien rq$send();
extern void alien rq$sleep();
#endif
