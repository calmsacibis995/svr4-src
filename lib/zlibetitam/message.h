/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:message.h	1.1"
	/*
	 *  message.h - V1.1
	 *
	 *	Message type definitions for help and error message support
	 */

#define  MT_HELP	0		/* Help message */
#define  MT_ERROR	1		/* Error message */
#define  MT_QUIT	2		/* Error message with quit option */
#define  MT_POPUP	3		/* Disappearing error message */
#define  MT_CONFIRM	4		/* Confirm/deny message */
#define  MT_INFO	5		/* Informational message (no label) */
