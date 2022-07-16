/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:devattr.c	1.2.6.1"

/*LINTLIBRARY*/

/*
 *  devattr.c
 *
 *  Contents:
 *	devattr()	Get the value of a attribute for a specific device
 */

/*
 *  Header files needed
 *	<sys/types.h>		System Data Types
 *	<stdio.h>		Standard I/O Definitions
 *	<errno.h>		Error-value definitions
 *	<string.h>		String function and constant definitions
 *	<devmgmt.h>		Device table definitions available to the world
 *	"devtab.h"		Local device table definitions
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<devmgmt.h>
#include	"devtab.h"


/*
 *  Externals referenced
 *	malloc()		Allocates space from main memory
 *	oam_devtab		FILE * to the opened device table
 */

extern	void   *malloc();
extern	FILE   *oam_devtab;


/*
 *  Local constant definitions
 */


/*
 *  Local static data
 */

/*
 *  char *devattr(device, attr)
 *
 *	This function searches the device table, looking for the device
 *	specified by <device>.  If it finds a record corresponding to that
 *	device (see below for a definition of that correspondence), it
 *	extracts the value of the field <attr> from that record, if any.
 *	It returns a pointer to that value, or (char *) NULL if none.
 *
 *  Arguments:
 *	device		Pointer to the character-string that describes the
 *			device whose record is to be looked for
 *	attr		The device's attribute to be looked for
 *
 *  Returns:  char *
 *	A pointer to the character-string containing the value of the
 *	attribute <attr> for the device <device>, or (char *) NULL if none
 *	was found.  If the function returns (char *) NULL and the error was
 *	detected by this function, it sets "errno" to indicate the problem.
 *
 *  "errno" Values:
 *	EPERM		Permissions deny reading access of the device-table
 *			file
 *	ENOENT		The specified device-table file could not be found
 *	ENODEV		Device not found in the device table
 *	EINVAL		The device does not have that attribute defined
 *	ENOMEM		No memory available
 */

char *
devattr(device, attribute)
	char   *device;		/* The device ) we're to look for */
	char   *attribute;	/* The attribute to extract */
{
	/* Automatic data */
	struct devtabent       *record;		/* Retrieved record */
	struct attrval	       *p;		/* attr/val records */
	char	    	       *val;		/* Extracted value */
	char		       *rtnval;		/* Value to return */
	int			found;		/* TRUE if attribute found */


	/* Get the record for the specified device */
	if (!(record = _getdevrec(device))) {
		_enddevtab();
		return((char *) NULL);
	}

	/* Search the record for the specified attribute */
	found = FALSE;

	/* Did they ask for the device alias? */
	if (strcmp(attribute, DTAB_ALIAS) == 0) {
	    val = (record->alias != (char *) NULL) ? record->alias : "" ;
	    found = TRUE;
	}

	/* Did they ask for the character-special device? */
	else if (strcmp(attribute, DTAB_CDEVICE) == 0) {
	    val = (record->cdevice != (char *) NULL) ? record->cdevice : "" ;
	    found = TRUE;
	}

	/* Did they ask for the block-special device? */
	else if (strcmp(attribute, DTAB_BDEVICE) == 0)
	{
	    val = (record->bdevice != (char *) NULL) ? record->bdevice : "" ;
	    found = TRUE;
	}

	/* Did they ask for the pathname? */
	else if (strcmp(attribute, DTAB_PATHNAME) == 0)
	{
	    val = (record->pathname != (char *) NULL) ? record->pathname : "" ;
	    found = TRUE;
	}

	else {

	    /*
	     * Didn't ask for one of the easy ones, search the attr/val
	     * structure
	     */

	    p = record->attrlist;
	    while (!found && (p)) {
		if (strcmp(p->attr, attribute) == 0) {
		    val = p->val;
		    found = TRUE;
		}
		else p = p->next;
	    }
	}

	/*
	 * If the attribute was found, copy it into malloc()ed space.
	 * If not, set errno appropriately; we'll return NULL
	 */

	if (found) {
	    if (rtnval = (char *) malloc((unsigned) strlen(val)+1))
		(void) strcpy(rtnval, val);
	    else errno = ENOMEM;
	} else {
	    rtnval = (char *) NULL;
	    errno = EINVAL;
	}

	/* Free the space allocated to the struct devtabent structure */
	_freedevtabent(record);

	_enddevtab();

	/* Fini */
	return(rtnval);
}
