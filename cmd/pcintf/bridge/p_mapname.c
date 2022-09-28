/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_mapname.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_mapname.c	3.4	LCC);	/* Modified: 15:36:08 11/27/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"

/*
 * The MAPFILE function maps a dos filename to the corresponding unix
 * filename if mode is zero and performs the inverse operation if mode   
 * is one.  There is an extended IOCTL function on the bridge that makes    
 * use of this service.
 *    
 */

void
pci_mapname(name, mode, addr)
char *name;
int mode;
struct output *addr;
{
    char nametemp[128];
    int flag;
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */
    
    strcpy(nametemp,name);
    bkslash(nametemp);
    if (mode) 
	{ /* map unix name to dos name */
		log("\nmapfilename: %s\n",nametemp);
 		flag = (mapfilename(CurDir,nametemp) != -1);
		ftslash(nametemp,strlen(nametemp));
		uppercase(nametemp,strlen(nametemp));
    }
    else  /* map dos name to unix name */
	{
		lowercase(nametemp,strlen(nametemp));
		log("\nunmapfilename: %s\n",nametemp);
#ifdef HIDDEN_FILES
		attrib = 0xff;
 		flag = (unmapfilename(CurDir,nametemp,&attrib) == FILE_IN_DIR);    
#else
 		flag = unmapfilename(CurDir,nametemp);    
#endif /* HIDDEN_FILES */
    }
    log("nametemp out: %s  flag:%d\n",nametemp, flag);
    if (flag) 
	{
		strcpy(addr->text,nametemp);
		addr->hdr.t_cnt = strlen(nametemp) + 1;
		addr->hdr.res = SUCCESS;
    }
    else 
	{
		addr->hdr.t_cnt = 0;
		addr->hdr.res = FILE_NOT_FOUND; 
    }
}
