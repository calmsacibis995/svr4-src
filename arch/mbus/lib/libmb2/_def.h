/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:lib/libmb2/_def.h	1.3"

#define MAX_PSIZE	128

static char *a_dev_name = "/dev/mb2-tai"; 
static char *s_dev_name = "/dev/mb2-tai"; 
static char *mod_name = "mb2tsm"; 
char mb2_user[MAX_PSIZE];
extern int errno;

#define min(a,b)	(a > b? b : a)
#define RNDUP(a)	((a + 0x3)&~0x3)

/* 	procedures uses in the library			*/

extern int _mb2_check_fd ();
extern int _mb2_check_range ();
extern void _mb2_do_sync ();
extern int _mb2_do_bind ();
extern int _mb2_do_unbind ();
extern int _mb2_do_optmgmt ();
extern int _mb2_do_info ();
extern int _mb2_do_send ();
extern int _mb2_do_second_recv ();
extern int _mb2_do_receive ();
extern int _mb2_do_fragreq ();
extern int _mb2_do_sendrsvp ();
extern int _mb2_do_sendreply ();
extern int _mb2_do_sendcancel ();
