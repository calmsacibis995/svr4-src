/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:devtab.c	1.2.7.1"
/*LINTLIBRARY*/

/*
 * devtab.c
 *
 *  Contains functions that deal with the device table and are not for
 *  consumption by the general user population.
 *
 *  Functions defined:
 *	_opendevtab()		Opens the device table for commands
 *	_setdevtab()		Rewinds the open device table
 *	_enddevtab()		Closes the open device table
 *	_getdevtabent()		Gets the next entry in the device table
 *	_freedevtabent()	Frees memory allocated to a device-table entry
 *	_getdevrec()		Gets a specific record from the device table
 *	_devtabpath()		Get the pathname of the device table file
 *	_validalias()		Is a value a valid alias?
 */

/*
 *  Header files
 *
 *	<sys/sysmacros.h>	System macro definitions
 *	<sys/types.h>		System data types
 *	<sys/mkdev.h>		Device Macros
 *	<unistd.h>		System Symbolic Constants
 *	<stdio.h>		Standard I/O definitions
 *	<string.h>		String handling definitions
 *	<ctype.h>		Character types and macros
 *	<errno.h>		Error codes 
 *	<sys/stat.h>		File status information
 *	<devmgmt.h>		Global Device Management definitions
 *	"devtab.h"		Local Device Management definitions
 */

#include	<sys/sysmacros.h>
#include	<sys/types.h>
#include	<sys/mkdev.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<devmgmt.h>
#include	"devtab.h"


/*
 *  Externals referenced:
 *	
 *	malloc()	Allocate a chunk of memory
 *	realloc()	Enlarge or shrink a malloc()ed piece of memory
 *	free()		Release a malloc()ed piece of memory
 *	getenv()	Get data from the program environemnt
 *	oam_devtab	File descriptor of the open device table
 */

extern	void	       *malloc();
extern	void	       *realloc();
extern	void		free();
extern	char	       *getenv();
extern	FILE	       *oam_devtab;


/*
 *  Static data definitions:
 *	dtabrecnum	Record number of the current record (0 to n-1)
 *	leftoff		Addr of char to begin next parse using
 *			getfld(), getattrval(), getquoted()
 *	recbufsz	The size of the buffer used for reading records
 *	recbuf		Addr of malloc() buffer for reading records
 *	xtndcnt		Number of malloc()/realloc() calls on record buffer
 */

static	int		xtndcnt = 0;
static	char	       *recbuf = (char *) NULL;
static	int		recbufsz = 0;

static	char	       *leftoff = (char *) NULL;
static	int		dtabrecnum = 0;

/*
 * int samedev(x, y)
 *	struct stat	x, y
 *
 *	Compares pertinent information in a stat() structure
 *	to see if the two structures describe the same device.
 *	If the file modes are the same and they have the same
 *	file system and i-node (i.e. they're links) or they
 *	are block or character devices and have the same major
 *	and minor device numbers (i.e. "mknod"s for the same
 *	device), it's the same device.
 *
 *   Returns:  int
 *	TRUE if the two structures describe the same device
 *	FALSE otherwise
 */

static int
samedev(x, y)
	struct stat	x;
	struct stat 	y;
{
	int	same;


	/* If the devices are of the same type ... */
	if ((x.st_mode & 0170000) == (y.st_mode & 0170000)) {

	    /* 
	     * If they are described by the same inode on the same device,
	     * the two devices are the same.  Otherwise, if the devices are 
	     * character-special or block-special devices, try to match by 
	     * device type and major and minor device numbers.
	     */

	    if ((x.st_dev == y.st_dev) && (x.st_ino == y.st_ino)) same = TRUE;
	    else
		if (((x.st_mode & 0170000) == 0020000) || 
		    ((x.st_mode & 0170000) == 0060000)) {
		    if ((major(x.st_rdev) == major(y.st_rdev)) &&
			(minor(x.st_rdev) == minor(y.st_rdev))) same = TRUE;
		    else same = FALSE;
		}
		else same = FALSE;

	}
	else same = FALSE;

	return(same);
}

/*
 *  void _setdevtab()
 *
 *	This function rewinds the open device table so that the next
 *	_getdevtabent() returns the first record in the device table.
 *
 *  Arguments:  None
 *
 *  Returns:  Void
 */

void
_setdevtab()
{
	/*  If the device table file is open, rewind the file  */
	if (oam_devtab != (FILE *) NULL) {
	    rewind(oam_devtab);
	    dtabrecnum = 0;
	}
	return;
}

/*
 *  void _enddevtab()
 *
 *	This function closes the open device table.  It resets the
 *	open device table external variable to NULL.
 *
 *  Arguments:  None
 *
 *  Returns:  Void
 */

void
_enddevtab()
{
	/*  If the device table file is open, close it  */
	if (oam_devtab != (FILE *) NULL) {
	    (void) fclose(oam_devtab);
	    oam_devtab = (FILE *) NULL;
	    dtabrecnum = 0;
	}
	return;
}

/*
 *  char *getfld(ptr, delims)
 *	char   *ptr
 *	char   *delims
 *
 *  Notes:
 *    -	Can't use "strtok()" because of its use of static data.  The caller
 *	may be using strtok() and we'll really mess them up.
 *    - The function returns NULL if it didn't find any token -- '\0' can't
 *	be a delimiter using this algorithm.
 */

static char *
getfld(ptr, delims)
	char   *ptr;		/* String to parse */
	char   *delims;		/* List of delimiters */
{
	int	done;		/* TRUE if we're finished */
	char   *p, *q;		/* Temp pointers */

	/*
	 *  Figure out where to start.
	 *  If given a pointer, use that.
	 *  Otherwise, use where we left off.
	 */

	p = ptr ? ptr : leftoff;


	/*
	 *  If there's anything to parse, search the string for the first
	 *  occurrence of any of the delimiters.  If one is found, change it
	 *  to '\0' and remember the place to start for next time.  If not
	 *  found, forget the restart address and prepare to return NULL.
	 *  Don't terminate on "escaped" characters.
	 */

	if (p) {			            /* Anything to do ?? */
	    q = p;				    /* Where to begin */
	    done = FALSE;			    /* We're not done yet */
	    while (*q && !done) {		    /* Any more chars */
		if (*q == '\\') {		    /* Escaped ? */
		  if (*(++q)) q++;		    /* Skip escaped char */
		}
		else 				    /* Not escaped */
		    if (!strchr(delims, *q)) q++;   /* Skip non-delim */
		    else done = TRUE;		    /* Otherwise, done */
	    }
	    if (*q) {				    /* Terminator found? */
		*q++ = '\0';			    /* Null-terminate token */
		leftoff = q;			    /* Remember restart pt. */
	    }
	    else leftoff = p = (char *) NULL;	    /* Nothin found or left */
	}

	/*  Finished  */
	return(p);				    /* Return ptr to token */
}

/*
 *  char *getquoted(ptr)
 *	char   *ptr;
 *
 *	This function extracts a quoted string from the string pointed
 *	to by <ptr>, or, if <ptr> is (char *) NULL, wherever we left off
 *	last time.
 *
 *  Arguments:
 *	char *ptr	Pointer to the character-string to parse, or
 *			(char *) NULL if we're to pick up where we
 *			[getquoted(), getfld(), and getattrval()] left off.
 *
 *  Returns:  char *
 *	The address of malloc()ed space that contains the possibly quoted
 *	string.
 *
 *  Notes:
 *    -	This code only works if it can assume that the last character in
 *	the string it's parsing is a '\n', something that is guarenteed
 *	by the getnextrec() function.
 */

static char *
getquoted(ptr)
	char   *ptr;
{
	/*  Automatic data  */
	char   *rtn;		/* Value to return */
	char   *p, *q;		/* Temps */

	/* Figure out where to start off */
	p = ptr ? ptr : leftoff;

	/* If there's anything to parse and it's a quoted string ... */
	if ((p) && (*p == '"') && (p = getfld(p+1, "\""))) {

	    /* Copy string for the caller */
	    if (rtn = (char *) malloc(strlen(p)+1)) {	/* Malloc() space */
		q = rtn;			/* Set up temp ptr */
		do {
		    if (*p == '\\') p++;	/* Skip escape */
		    *q++ = *p;			/* Copy char */
		} while (*p++); 		/* While there's chars */
	    } else leftoff = rtn = (char *) NULL;
	} else leftoff = rtn = (char *) NULL;

	/* Fini */
	return(rtn);
}

/*
 *  struct attrval *getattrval(ptr)
 *	char   *ptr
 *
 *	This function extracts the next attr=val pair from <ptr> or wherever
 *	getfld() left off...
 *
 *  Arguments:
 *	char *ptr	The string to parse, or (char *) NULL if we're to
 *			begin wherever we left off last time.
 *
 *  Returns:  struct attrval *
 *	The address of a malloc()ed structure containing the attribute and the
 *	value of the attr=val pair extracted.
 */

static struct attrval *
getattrval(ptr)
	char   *ptr;
{
	/*  Automatic data  */
	struct attrval *rtn;		/* Ptr to struct to return */
	char	       *p, *q;		/* Temp pointers */


	/*  Use what's given to us or wherever we left off  */
	p = ptr ? ptr : leftoff;

	/*  If there's anything to parse, extract the next attr=val pair  */
	if (p) {

	    /*  Eat white space  */
	    while (*p && isspace(*p)) p++;

	    /*  Extract the attribute name, if any  */
	    if (*p && getfld(p, "=")) {

		/*  Allocate space for the structure we're building  */
		if (rtn = (struct attrval *) malloc(sizeof(struct attrval))) {

		    /*  Allocate space for the attribute name  */
		    if (rtn->attr = (char *) malloc(strlen(p)+1)) {

			/*  Copy the attribute name into alloc'd space  */
			q = rtn->attr;			/* Set up temp ptr */
			do {
			    if (*p == '\\') p++;	/* Skip escape */
			    *q++ = *p;			/* Copy char */
			} while (*p++); 		/* While more */

			/*  Extract the value  */
			if (!(rtn->val = getquoted((char *) NULL))) {
			    /* Error getting value, free resources */
			    free(rtn->attr);
			    free(rtn);
			    leftoff = (char *) NULL;
			    rtn = (struct attrval *) NULL;
			}
		    }
		    else {
			/* Error getting space for attribute, free resources */
			free(rtn);
			leftoff = (char *) NULL;
			rtn = (struct attrval *) NULL;
		    }

		}
		else {
		    /* No space for attr struct */
		    leftoff = (char *) NULL;
		    rtn = (struct attrval *) NULL;
		}

	    }
	    else {
		/* No attribute name */
		leftoff = (char *) NULL;
		rtn = (struct attrval *) NULL;
	    }

	}
	else {
	    /* Nothing to parse */
	    leftoff = (char *) NULL;
	    rtn = (struct attrval *) NULL;
	}

	/* Done */
	return(rtn);
}

/*
 *  char *getnextrec()
 *
 *	This function gets the next record from the input stream "oam_devtab"
 *	and puts it in the device-table record buffer (whose address is in
 *	"recbuf").  If the buffer is not allocated or is too small to
 *	accommodate the record, the function allocates more space to the
 *	buffer.
 *
 *  Arguments:  None
 *
 *  Returns:  char *
 *	The address of the buffer containing the record.
 *
 *  Static Data Referenced:
 *	recbuf		Address of the buffer containing records read from the
 *			device table file
 *	recbufsz	Current size of the record buffer
 *	xtndcnt		Number of times the record buffer has been extended
 *	oam_devtab	Device table stream, expected to be open for (at
 *			least) reading
 *
 *  Notes:
 *    - The string returned in the buffer <buf> ALWAYS end in a '\n' (newline)
 *	character followed by a '\0' (null).
 */

static char *
getnextrec()
{
	/* Automatic data */
	char	       *recp;		/* Value to return */
	char	       *p;		/* Temp pointer */
	int		done;		/* TRUE if we're finished */
	int		reclen;		/* Number of chars in record */


	/* If there's no buffer for records, try to get one */
	if (!recbuf) {
	    if (recbuf = (char *) malloc(DTAB_BUFSIZ)) {
	    	recbufsz = DTAB_BUFSIZ;
		xtndcnt = 0;
	    } else return((char *) NULL);
	}


	/* Get the next record */
	recp = fgets(recbuf, recbufsz, oam_devtab);
	done = FALSE;

	/* While we've something to return and we're not finished ... */
	while (recp && !done) {

	    /* If our return string isn't a null-string ... */
	    if ((reclen = strlen(recp)) != 0) {

		/* If we have a complete record, we're finished */
		if ((*(recp+reclen-1) == '\n') &&
		    ((reclen == 1) || (*(recp+reclen-2) != '\\'))) done = TRUE;
		else while (!done) {

		    /*
		     * Need to complete the record.  A complete record is one
		     * which is terminated by an unescaped new-line character
		     */

		    /* If the buffer is full, expand it and continue reading */
		    if (reclen == recbufsz-1) {

			/* Have we reached our maximum extension count? */
			if (xtndcnt < XTND_MAXCNT) {

			    /* Expand the record buffer */
			    if (p = (char *) realloc(recbuf, recbufsz+DTAB_BUFINC)) {

				/* Update buffer information */
				xtndcnt++;
				recbuf = p;
				recbufsz += DTAB_BUFINC;

			    } else {

				/* Expansion failed */
				recp = (char *) NULL;
				done = TRUE;
			    }

			} else {

			    /* Maximum extend count exceeded.  Insane table */
			    recp = (char *) NULL;
			    done = TRUE;
			}

		    }

		    /* Complete the record */
		    if (!done) {

			/* Read stuff into the expanded space */
			if (fgets(recbuf+reclen, recbufsz-reclen, oam_devtab)) {
			    reclen = strlen(recbuf);
			    recp = recbuf;
			    if ((*(recp+reclen-1) == '\n') &&
				((reclen == 1) || (*(recp+reclen-2) != '\\'))) done = TRUE;
			}
			else {
			    /* Read failed, corrupt record? */
			    recp = (char *) NULL;
			    done = TRUE;
			}
		    }

		}   /* End incomplete record handling */

	    } else {

		/* Read a null string?  (corrupt table) */
		recp = (char *) NULL;
		done = TRUE;
	    }

	}   /* while (recp && !done) */

	/* Return what we've got (if anything) */
	return(recp);
}

/*
 *  char *_devtabpath()
 *
 *	Get the pathname of the device table
 *
 *  Arguments:  None
 *
 *  Returns:  char *
 *	Returns the pathname to the device table of (char *) NULL if
 *	there was a problem getting the memory needed to contain the
 *	pathname.
 *
 *  Algorithm:
 *	1.  If OAM_DEVTAB is defined in the environment and is not
 *	    defined as "", it returns the value of that environment
 *	    variable.
 *	2.  Otherwise, use the value of the environment variable DTAB_PATH.
 */


char *
_devtabpath()
{

	/* Automatic data */
#ifdef	DEBUG
	char	       *path;		/* Ptr to path in environment */
#endif
	char	       *rtnval;		/* Ptr to value to return */


	/*
	 * If compiled with -DDEBUG=1,
	 * look for the pathname in the environment
	 */

#ifdef	DEBUG
	if ((path = getenv(OAM_DEVTAB)) && (*path)) {
	    if (rtnval = (char *) malloc(strlen(path)+1)) (void) strcpy(rtnval, path);
	}
	else {
#endif
	    /*
	     * Use the standard device table.
	     */

	    if (rtnval = (char *) malloc(strlen(DTAB_PATH)+1)) (void) strcpy(rtnval, DTAB_PATH);

#ifdef	DEBUG
	}
#endif

	/* Finished */
	return(rtnval);
}

/*
 *  int _opendevtab(mode)
 *	char   *mode
 *
 *	The _opendevtab() function opens a device table for a command.
 *
 *  Arguments:
 *	mode	The open mode to use to open the file.  (i.e. "r" for
 *		reading, "w" for writing.  See FOPEN(BA_OS) in SVID.)
 *
 *  Returns:  int
 *	TRUE if it successfully opens the device table file, FALSE otherwise
 */

int
_opendevtab(mode)
	char   *mode;
{
	/*
	 *  Automatic data
	 */

	char   *devtabname;		/* Ptr to the device table name */
	int	rtnval;			/* Value to return */


	rtnval = TRUE;
	if (devtabname = _devtabpath()) {
	    if (oam_devtab) (void) fclose(oam_devtab);
	    if (oam_devtab = fopen(devtabname, mode))
		dtabrecnum = 0;  /* :-) */
	    else rtnval = FALSE; /* :-( */
	} else rtnval = FALSE;   /* :-( */
	return(rtnval);
}

/*
 *  int _validalias(alias)
 *	char   *alias
 *
 *	Determine if <alias> is a valid alias.  Returns TRUE if it is
 *	a valid alias, FALSE otherwise.
 *
 *  Arguments:
 *	alias		Value to check out
 *
 *  Returns:  int
 *	TRUE if <alias> is a valid alias, FALSE otherwise.
 */

int
_validalias(alias)
	char   *alias;			/* Alias to validate */
{
	/* Automatic data */
	char	       *p;		/* Temp pointer */
	int		len;		/* Length of <alias> */
	int		rtn;		/* Value to return */


	/* Assume the worst */
	rtn = FALSE;

	/*
	 * A valid alias contains 0 < i <= 14 characters.  The first
	 * must be alphanumeric or "@$_." and the rest must be alphanumeric
	 * or "@#$_+-."
	 */

	/* Check length */
	if (((len = strlen(alias)) > 0) && (len <= 14)) {

	    /* Check the first character */
	    p = alias;
	    if (isalnum(*p) || strchr("@$_.", *p)) {

		/* Check the rest of the characters */
		for (p++ ; *p && (isalnum(*p) || strchr("@#$_-+.", *p)) ; p++);
		if (!(*p)) rtn = TRUE;
	    }
	}

	/* Return indicator... */
	return(rtn);

}   /* int _validalias() */

/*
 *  struct devtabent *_getdevtabent()
 *
 *  	This function returns the next entry in the device table.
 *	If no device table is open, it opens the standard device table
 *	and returns the first record in the table.
 *
 *  Arguments:  None.
 *
 *  Returns:  struct devtabent *
 *	Pointer to the next record in the device table, or
 *	(struct devtabent *) NULL if it was unable to open the file or there
 *	are no more records to read.  "errno" reflects the situation.  If
 *	errno is not changed and the function returns NULL, there are no more
 *	records to read.  If errno is set, it indicates the error.
 *
 *  Notes:
 *    - The caller should set "errno" to 0 before calling this function.
 */

struct devtabent *
_getdevtabent()
{
	/*  Automatic data  */
	struct devtabent       *ent;	/* Ptr to dev table entry structure */
	struct attrval	       *attr;	/* Ptr to struct for attr/val pair */
	struct attrval	       *t;	/* Tmp ptr to attr/val struct */
	char		       *record;	/* Ptr to the record just read */
	char		       *p, *q;	/* Tmp char ptrs */
	int			done;	/* TRUE if we've built an entry */


	/*  Open the device table if it's not already open */
	if (oam_devtab == (FILE *) NULL) {
	    if (!_opendevtab("r")) return ((struct devtabent *) NULL);
	}

	/*  Get space for the structure we're returning  */
	if (!(ent = (struct devtabent *) malloc(sizeof(struct devtabent)))) {
	    return((struct devtabent *) NULL);
	}

	done = FALSE;
	while (!done && (record = getnextrec())) {

	    /* Save record number in structure */
	    ent->entryno = dtabrecnum++;

	    /* Comment record?  If so, just save the value and we're through */
	    if (strchr("#\n", *record) || isspace(*record)) {
		ent->comment = TRUE;
		done = TRUE;
		if (ent->attrstr = (char *) malloc((unsigned) strlen(record)+1)) {
		    q = ent->attrstr;
		    p = record;
		    do {
			if (*p == '\\') p++;
			*q++ = *p;
		    } while (*p++);
		}
		else {
		    free((char *) ent);
		    ent = (struct devtabent *) NULL;
		}
	    }

	    else {

		/* Record is a data record.   Parse it. */
		ent->comment = FALSE;
		ent->attrstr = (char *) NULL;  /* For now */

		/* Extract the device alias */
		if (p = getfld(record, ":")) {
		    if (*p) {
			if (ent->alias = (char *) malloc((unsigned) strlen(p)+1)) {
			    q = ent->alias;
			    do {
				if (*p == '\\') p++;
				*q++ = *p;
			    } while (*p++);
			}
		    } else ent->alias = (char *) NULL;

		    /* Extract the character-device name */
		    if ((p = getfld((char *) NULL, ":")) == (char *) NULL) {
			if (ent->alias) free(ent->alias);
		    } else {
			if (*p) {
			    if (ent->cdevice = (char *) malloc((unsigned) strlen(p)+1)) {
				q = ent->cdevice;
				do {
				    if (*p == '\\') p++;
				    *q++ = *p;
				} while (*p++);
			    }
			} else ent->cdevice = (char *) NULL;

			/* Extract the block-device name */
			if (!(p = getfld((char *) NULL, ":"))) {
			    if (ent->alias) free(ent->alias);
			    if (ent->cdevice) free(ent->cdevice);
			} else {
			    if (*p) {
				if (ent->bdevice = (char *) malloc((unsigned) strlen(p)+1)) {
				    q = ent->bdevice;
				    do {
					if (*p == '\\') p++;
					*q++ = *p;
				    } while (*p++);
				}
			    } else ent->bdevice = (char *) NULL;

			    /* Extract the pathname */
			    if ((p = getfld((char *) NULL, ":\n")) == (char *) NULL) {
				if (ent->alias) free(ent->alias);
				if (ent->cdevice) free(ent->cdevice);
				if (ent->bdevice) free(ent->bdevice);
			    } else {
				if (*p) {
				    if (ent->pathname = (char *) malloc((unsigned) strlen(p)+1)) {
					q = ent->pathname;
					do {
					    if (*p == '\\') p++;
					    *q++ = *p;
					} while (*p++);
				    }
				} else ent->pathname = (char *) NULL;

				/* Found a valid record */
				done = TRUE;

				/* Extract attributes, build a linked list of 'em (may be none) */
				if (attr = getattrval((char *) NULL)) {
				    ent->attrlist = attr;
				    t = attr;
				    while (attr = getattrval((char *) NULL)) {
					t->next = attr;
					t = attr;
				    }
				    t->next = (struct attrval *) NULL;
				}
				else ent->attrlist = (struct attrval *) NULL;

			    } /* pathname extracted */
			} /* bdevice extracted */
		    } /* cdevice extracted */
		} /* alias extracted */
	    }
	} /* !done && record read */

	/*  If no entry was read, free space allocated to the structure  */
	if (!done) {
	    free(ent);
	    ent = (struct devtabent *) NULL;
	}

	return(ent);
}

/*
 *  void _freedevtabent(devtabent)
 *	struct devtabent       *devtabent;
 *
 *	This function frees space allocated to a device table entry.
 *
 *  Arguments:
 *	struct devtabent *devtabent	The structure whose space is to be
 *					freed.
 *
 *  Returns:  void
 */

void
_freedevtabent(ent)
	struct devtabent       *ent;
{
	/*
	 * Automatic data
	 */

	struct attrval *p;		/* Structure being freed */
	struct attrval *q;		/* Next structure to free */

	if (!ent->comment) {

	    /*
	     *  Free the attribute list.  For each item in the attribute list,
	     *    1.  Free the attribute name (always defined),
	     *    2.  Free the value (if any -- it's not defined if we're changing
	     *	      an existing attribute),
	     *    3.  Free the space allocated to the structure.
	     */

	    if ((q = ent->attrlist)) do {
		p = q;
		q = p->next;
		free(p->attr);
		if (p->val) free(p->val);
		free(p);
	    } while (q);

	    /* Free the standard fields (alias, cdevice, bdevice, pathname) */
	    if (ent->alias) free(ent->alias);
	    if (ent->cdevice) free(ent->cdevice);
	    if (ent->bdevice) free(ent->bdevice);
	    if (ent->pathname) free(ent->pathname);
	}

	/* Free the attribute string */
	if (ent->attrstr) free(ent->attrstr);

	/* Free the space allocated to the structure */
	free(ent);

	/* We're through */
	return;
}

/*
 *  struct devtabent *_getdevrec(device)
 *	char *device
 *
 *	Thie _getdevrec() function returns a pointer to a structure that
 *	contains the information in the device-table entry that describes
 *	the device <device>.
 *
 *	The device <device> can be a device alias, a pathname contained in
 *	the entry as the "cdevice", "bdevice", or "pathname" attribute,
 *	or a pathname to a device described using the "cdevice", "bdevice",
 *	or "pathname" attribute (depending on whether the pathname references
 *	a character-special file, block-special file, or something else,
 *	respectively.
 *
 *  Arguments:
 *	char *device	A character-string describing the device whose record
 *			is to be retrieved from the device table.
 *
 *  Returns:  struct devtabent *
 *	A pointer to a structure describing the device.
 *
 *  Notes:
 *    -	Someday, add a cache so that repeated requests for the same record
 *	don't require going to the filesystem.  (Maybe -- this might belong
 *	in devattr()...)
 */

struct devtabent *
_getdevrec(device)
	char	       *device;			/* The device to search for */
{
	/*
	 *  Automatic data
	 */

	struct stat		devstatbuf;	/* Stat struct, <device> */
	struct stat		tblstatbuf;	/* Stat struct, tbl entry */
	struct devtabent       *devrec;		/* Pointer to current record */
	int			found;		/* TRUE if record found */
	int			olderrno;	/* Old value of errno */


	/*
	 *  Search the device table looking for the requested device
	 */

	_setdevtab();
	olderrno = errno;
	found = FALSE;
	if (!_validalias(device)) {
	    while (!found && (devrec = _getdevtabent())) {
		if (!devrec->comment &&
		    ((strcmp(device, devrec->cdevice) == 0) ||
		     (strcmp(device, devrec->bdevice) == 0) ||
		     (strcmp(device, devrec->pathname) == 0))) found = TRUE;
		else _freedevtabent(devrec);
	    }

	    /*
	     *  If the device <device> wasn't named explicitly in the device
	     *  table, compare it against like entries by comparing file-
	     *  system, major device number, and minor device number
	     */

	    if (!found) {
		_setdevtab();

		/*  Status the file <device>.  If fails, invalid device */
		if (stat(device, &devstatbuf) != 0) errno = ENODEV;
		else {

		    /*
		     *  If <device> is a block-special device.  See if it is
		     *  in the table by matching its file-system indicator and
		     *  major/minor device numbers against the file-system and
		     *  major/minor device numbers of the "bdevice" entries.
		     */

		    if ((devstatbuf.st_mode & 0170000) == 0020000) {
			while (!found && (devrec = _getdevtabent())) {
			    if (!devrec->comment && (devrec->cdevice != (char *) NULL))
				if (stat(devrec->cdevice, &tblstatbuf) == 0) {
				    if (samedev(tblstatbuf, devstatbuf))
					found = TRUE;
				}
				else errno = olderrno;	/* Ignore stat() errs */
			    if (!found) _freedevtabent(devrec);
			}
		    }

		    /*
		     * If <device> is a block-special device.  See if it is
		     * in the table by matching its file-system indicator and
		     * major/minor device numbers against the file-system
		     * and major/minor device numbers of the "bdevice" entries.
		     */

		    else if ((devstatbuf.st_mode & 0170000) == 0060000) {
			while (!found && (devrec = _getdevtabent())) {
			    if (!devrec->comment && (devrec->bdevice != (char *) NULL))
				if (stat(devrec->bdevice, &tblstatbuf) == 0) {
				    if (samedev(tblstatbuf, devstatbuf))
					found = TRUE;
				}
				else errno = olderrno;	/* Ignore stat() errs */
			    if (!found) _freedevtabent(devrec);
			}
		    }

		    /*
		     * If <device> is neither a block-special or character-
		     * special device.  See if it is in the table by matching
		     * its file-system indicator and major/minor device
		     * numbers against the file-system and major/minor device
		     * numbers of the "pathname" entries.
		     */

		    else {
			while (!found && (devrec = _getdevtabent())) {
			    if (!devrec->comment && (devrec->pathname != (char *) NULL))
				if (stat(devrec->pathname, &tblstatbuf) == 0) {
				    if (samedev(tblstatbuf, devstatbuf))
					found = TRUE;
				}
				else errno = olderrno;	/* Ignore stat() errs */
			    if (!found) _freedevtabent(devrec);
			}
		    }

		    if (!found) {
			devrec = (struct devtabent *) NULL;
			errno = ENODEV;
		    }

		} /* End case where stat() on the <device> succeeded */

	    } /* End case handling pathname not explicitly in device table */

	} /* End case handling <device> as a fully-qualified pathname */


	/*
	 *  Otherwise the device <device> is an alias.
	 *  Search the table for a record that has as the "alias" attribute
	 *  the value <device>.
	 */

	else {
	    while (!found && (devrec = _getdevtabent())) {
		if (!devrec->comment && strcmp(device, devrec->alias) == 0)
		    found = TRUE;
		else _freedevtabent(devrec);
	    }
	    if (!found) {
		devrec = (struct devtabent *) NULL;
		errno = ENODEV;
	    }
	}

	/* Fini */
	return(devrec);
}
