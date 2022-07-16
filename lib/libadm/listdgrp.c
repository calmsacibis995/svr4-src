/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:listdgrp.c	1.1.6.1"
/*LINTLIBRARY*/

/*
 *  listdgrp.c
 *
 *  Contents:
 *	listdgrp()	List devices that belong to a device group.
 */


/*
 * Header files referenced:
 *	<sys/types.h>	System Data Types
 *	<errno.h>	UNIX and C error definitions
 *	<string.h>	String handling definitions
 *	<devmgmt.h>	Device management definitions
 *	"devtab.h"	Local device table definitions
 */

#include	<sys/types.h>
#include	<errno.h>
#include	<string.h>
#include	<devmgmt.h>
#include	"devtab.h"

/*
 * Local definitions
 */


/*
 *  Structure definitions:
 */

/*
 * Externals referenced
 * 	malloc()	Allocate space from main memory.
 *	free()		Free space allocated using malloc().
 *	_getdgrprec()	Get the next dev
 */

extern	void		       *malloc();	/* Alloc memory */
extern	void			free();		/* Free alloc'ed memory */
extern	struct dgrptabent      *_getdgrprec();	/* Get dgroup record */


/*
 * Local functions referenced
 */

/*
 * Global Data
 */

/*
 * Static Data
 */

/*
 * char **listdgrp(dgroup)
 *	char   *dgroup
 *
 *	List the members of a device group.
 *
 *  Arguments:
 *	char *dgroup	The device group needed
 *
 *  Returns:  char **
 *	A pointer to a list of pointers to char-strings containing
 *	the members of the device group.
 *
 *  Notes:
 *    -	malloc()ed space containing addresses 
 */

char  **
listdgrp(dgroup)
	char		       *dgroup;		/* The device group to list */
{
	/* Automatic data */
	struct dgrptabent      *dgrpent;	/* Device group description */
	struct member	       *member;		/* Device group member */
	char		      **listbuf;	/* Buffer allocated for addrs */
	char		      **rtnval;		/* Value to return */
	char		      **pp;		/* Running ptr through addrs */
	int			noerror;	/* Flag, TRUE if all's well */
	int			n;		/* Counter */


	/*
	 *  Initializations
	 */

	/*
	 *  Get the record for this device group
	 */

	if (dgrpent = _getdgrprec(dgroup)) {

	    /*  Count the number of members in the device group  */ 
	    n = 1;
	    for (member = dgrpent->membership ; member ; member = member->next) n++;

	    /*  Get space for the list to return  */
	    if (listbuf = (char **) malloc(n*sizeof(char **))) {

		/*  
		 *  For each member in the device group, add that device
		 *  name to the list of devices we're building
		 */

		pp = listbuf;
		noerror = TRUE;
		for (member = dgrpent->membership ; noerror && member ; member = member->next) {
		    if (*pp = (char *) malloc((unsigned) strlen(member->name)+1)) 
			(void) strcpy(*pp++, member->name);
		    else noerror = FALSE;
		}


		/*  
		 *  If there's no error, terminate the list we've built.
		 *  Otherwise, free the space allocated to the stuff we've built
		 */

		if (noerror) {
		    *pp = (char *) NULL;
		    rtnval = listbuf;
		}
		else {
		    /*  Some error occurred.  Clean up allocations  */
		    for (pp = listbuf ; *pp ; pp++) free(*pp);
		    free((char *) listbuf);
		    rtnval = (char **) NULL;
		}

	    }  /* if (malloc()) */

	    /*  Free space alloced to the device group entry  */
	    _freedgrptabent(dgrpent);

	}  /* if (_getdgrprec()) */
	else rtnval = (char **) NULL;


	/*  Finished -- wasn't that simple?  */
	return(rtnval);
}
