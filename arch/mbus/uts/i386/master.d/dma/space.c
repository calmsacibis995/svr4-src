/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/dma/space.c	1.1"

/*
 *	V.4 can run out of memory for short periods of time when it
 *	gets busy.  The dma_cb_lowat variable should be set to the
 *	number of dma_cb structures likely to be needed by interrupt
 *	threads during these brief periods.
*/
int dma_cb_lowat = 50;

/*
 *	The dma_buf_lowat variable should be set to the number
 *	of dma_buf structures likely to be needed by interrupt
 *	threads during these brief periods.
*/
int dma_buf_lowat = 50;
