/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_shminit.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_shminit.c	3.5	LCC);	/* Modified: 16:22:06 7/13/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

char  scratch[] = "\n\
# PC-Interface copyright (c) 1984, 1987 by Locus Computing Corporation.\n\
# All Rights Reserved.\n\
";

#ifdef SYS5
#define	PCI_KEY		0x160309
int	shmid;
int	*shmat();
#endif /* SYS5 */

char *myname;

main(argc, argv)
int	argc;
char	*argv[];
{

	myname = argv[0];
	if (*myname == '\0')
		myname = "unknown";

#ifdef SYS5
	/* Lock initialize --- */
	shmid = shmget(PCI_KEY, sizeof(int), 0777);
	if(shmid >= 0)
		*shmat(shmid, 0, 0) = 0;
#endif /* SYS5 */

}
