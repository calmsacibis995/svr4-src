/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ctl87.c	1.3"

/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */
/*
*
*  This file implements the C procedure
*        int  get87();            fetches the 8087 status word
*
* It has been modified from the code distributed by WEITEK in the following
* ways:
*
* 1.      asm() statements are used so as to avoid having to worry about
*         generating the correct C entry and exit code.
*
* 2.      Int results are returned in eax, sign extended.
*
*/

#ifdef WEITEK
int
get87 ()
{
        asm ("subl $2, %esp");		/*  Make Space on the stack */
        asm ("fnstsw -2(%ebp)");	/* Save the Status word on the stack */
        asm ("movswl -2(%ebp), %eax");	/* Load the return value */
}
#endif /* WEITEK */
