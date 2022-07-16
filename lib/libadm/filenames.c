/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/
#ident	"@(#)libadm:filenames.c	1.1.2.1"

/*
 */

#include	<devmgmt.h>

#ifndef	NULL
#define	NULL		0
#endif

char	       *getenv();

char *_getdevtabfilename() 
{
	/* Automatic data */
	char	       *devtablename;


	/* 
	 *  Get the value for the device table environment variable.  If there's not one defined,
	 *  get the default name.
	 */

	if ((devtablename = getenv(OAM_DEVTAB)) == (char *) NULL) devtablename = DTAB_PATH;

	/* Return a pointer to the retrieved name */
	return(devtablename);
}

char *_getdgroupfilename() 
{
	/* Automatic data */
	char	       *dgroupfilename;


	/* 
	 *  Get the value for the device group file environment variable.  If there's not one defined,
	 *  get the default name.
	 */

	if ((dgroupfilename = getenv(OAM_DGROUP)) == (char *) NULL) dgroupfilename = DGRP_PATH;

	/* Return a pointer to the retrieved name */
	return(dgroupfilename);
}
