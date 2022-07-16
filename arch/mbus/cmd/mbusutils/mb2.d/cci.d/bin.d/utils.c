/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/utils.c	1.3"

/****************************************************************************/
/*                                                                          */
/*                          MOV BYTE	                                    */
/*                          --------                                        */
/*                                                                          */
/****************************************************************************/

void movb(src_p, dest_p, len)

	char			*src_p;
	char			*dest_p;
	unsigned short	len;


	{
			/* Local Variables */

	char	*src_tmp_p;
	char	*dst_tmp_p;

	src_tmp_p = src_p;
	dst_tmp_p = dest_p;
	
	if (src_p == NULL || dest_p == NULL) 
		return;

	while (len > 0) {
		*dst_tmp_p = *src_tmp_p;
		++src_tmp_p;
		++dst_tmp_p;
		--len;
	}

}
