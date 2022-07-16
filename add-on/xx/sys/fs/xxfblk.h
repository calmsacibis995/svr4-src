/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ifndef _FS_XXFBLK_H
#define _FS_XXFBLK_H

#ident	"@(#)xx:sys/fs/xxfblk.h	1.2.1.1"

#pragma pack(2)

struct	fblk
{
	short	df_nfree;
	daddr_t	df_free[NICFREE];
};

#pragma pack()
#endif	/* _FS_XXFBLK_H */
