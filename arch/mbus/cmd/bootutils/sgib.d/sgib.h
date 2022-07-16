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

#ident	"@(#)mbus:cmd/bootutils/sgib.d/sgib.h	1.3"

#define ERR	 	(void)fprintf
#define TAPEBUFF	512
#define DISKBUF		1024

/* boot types */

#define BTAPE		0x1
#define BSERVER		0x2
#define BDISK		0x4
#define BFLOP		0x8
 

struct drtab {
	ushort	dr_ncyl;
	char	dr_nfhead;
	char	dr_nrhead;
	char	dr_nsec;
	char	dr_lsecsiz;
	char	dr_hsecsiz;
	char	dr_nalt;
};
