/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_delete.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_delete.c	3.17	LCC);	/* Modified: 16:28:54 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <xdir.h>

/*				External Routines			*/


extern  void ftslash();         /* Translates frontslash to UNIX backslash */
extern  void getpath();         /* Extracts pathname portion from string */

extern  int match();            /* Match UNIX filename to DOS search pattern */
extern  int wildcard();         /* Determines if string has wildcards */
extern  int unmapfilename();    /* Unmaps a PCI filename if necessary */

extern struct temp_slot temp_slot[];

extern int
	errno;

void
#ifdef	MULT_DRIVE
pci_delete(filename, attr, drvNum, addr, request)
int
	drvNum;
#else
pci_delete(filename, attr, addr, request)
#endif	/* MULT_DRIVE */
    char	*filename;		/* Name of file to unlink */
    int		attr;			/* MS-DOS file attribute */
    struct	output	*addr;		/* Address of response buffer */
    int		request;		/* DOS request number simulated */
{
	register	int
		hflg,			/* Hidden attribute flag */
		found = FALSE;		/* Loop flag */
	int     reg_found = FALSE;		/* loop flag */
	int     tslot;                      /* temp file slot number */
	
	char *dotptr,			/* Pointer to "." in file name */
		pipefilename[MAX_PATH],	/* File name of pipe */
		pathcomponent[MAX_PATH],	/* Path component of filename */
		mappedname[MAX_FILENAME],	/* mapped version of candidate
									*  directory entries
									*/
		namecomponent[MAX_PATH];	/* File component of filename */
	
	DIR *dirdp;			/* Pointer to opendir() structure */
	
	struct	direct
		*direntryptr;		/* Pointer to directory entry */
	
	unsigned short userId;
	struct stat statb;
#ifdef HIDDEN_FILES
	int mapret;
	int attrib;
#endif /* HIDDEN_FILES */

	userId = getuid();

    /* Translate MS-DOS filename to UNIX filename */
	cvt2unix(filename);

	/*** Kludge to allow access to protected directories for tmps ***/
	if ((tslot = redirdir(filename, 0)) >= 0) {
	    strcpy(filename, "/tmp/");
	    strcat(filename, temp_slot[tslot].fname);
	    temp_slot[tslot].s_flags = 0;
	}

    /* If unmapping filename returns collision return failure */
#ifdef HIDDEN_FILES
	/* ignore hidden files (otherwise, since attrib is random,
		we'll sometimes get them and sometimes not. */
	attrib = 0;		
	if ((mapret = unmapfilename(CurDir, filename, &attrib)) != FILE_IN_DIR)
#else
	if ((unmapfilename(CurDir, filename)) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */
	{
	    debug(0, ("delete: unmap '%s','%s' unmap failed\n",
			CurDir, filename));
#ifdef HIDDEN_FILES
	    addr->hdr.res = (mapret == FILE_NOT_IN_DIR) ? FILE_NOT_FOUND :ACCESS_DENIED;
#else
	    addr->hdr.res = ACCESS_DENIED;
#endif
	    return;
	}

	hflg = (attr & HIDDEN) ? TRUE : FALSE;

#ifdef	MULT_DRIVE
	filename = fnQualify(filename, CurDir);
#endif

	debug(0, ("delete: %s\n", filename));
    /* Delete all files that match with "name" */
	if (wildcard(filename, strlen(filename)))
	{
	    getpath(filename, pathcomponent, namecomponent); 
	    if ((dirdp = opendir(pathcomponent)) == NULL)
	    {
			debug(0, ("delete: opendir %s failed\n",
			pathcomponent));
			addr->hdr.res = PATH_NOT_FOUND;
			return;
	    }

#ifdef	MULT_DRIVE
	    chdir(pathcomponent);
#endif	/* MULT_DRIVE */

	    while ((direntryptr = readdir(dirdp)) != NULL)
	    {
			if ((is_dot(direntryptr->d_name)) ||
		    	(is_dot_dot(direntryptr->d_name)) ||
		    	((!hflg) && (direntryptr->d_name[0] == '.')))
				continue;
			strcpy(mappedname,direntryptr->d_name);
			debug(0, ("%s:before mapfile:%s\n","pci_delete",mappedname));
			mapfilename(pathcomponent,mappedname);
			debug(0, ("%s:after mapfile:%s\n","pci_delete",mappedname));

			if (match(mappedname, namecomponent, MAPPED))
			{
		    	if (stat(direntryptr->d_name, &statb) >= 0)
		    	{
				/* these are type possible file types:	*/
				/*	S_IFDIR directory		*/
				/*	S_IFCHR character special	*/
				/*	S_IFBLK block special		*/
				/*	S_IFREG regular			*/
				/*	S_IFIFO fifo			*/
				/* only unlink "regular" files		*/

					if ((statb.st_mode & S_IFMT) == S_IFREG) 
					{
			    		reg_found = TRUE;
			    /* delete if root or have write permission */
			    		if ((!userId || !access(direntryptr->d_name, 2)) &&
							!unlink(direntryptr->d_name)) 
						{
							found = TRUE;
							del_fname(direntryptr->d_name);
			    		} 
						else 
							debug(0,("didn't unlink %s\n",direntryptr->d_name));
					} 
					else 
			    		debug(0, ("not reg file %s\n", direntryptr->d_name));
		    	} 
				else 
				{
					debug(0, ("unable to stat %s\n", direntryptr->d_name));
					reg_found = TRUE;	/* so we call err_handler() */
		    	}
			}/*endifmatch*/
	    }	/* end while */
	    closedir(dirdp);
	}
	else    /* no wild cards */
	{
	    debug(0, ("delete: no wild cards\n"));
	    /* If this is for a pipe open the pipe in /tmp */
	    if ((strncmp(filename, "/%pipe", PIPEPREFIXLEN)) == 0) 
		{
			strcpy(pipefilename, "/tmp");
			dotptr = strchr(filename, '.');
			sprintf(dotptr+1, "%d", getpid());
			strcat(pipefilename, filename);
			strcpy(filename, pipefilename);
	    }

	    if (stat(filename, &statb) >= 0)
	    {
		/* these are type possible file types:	*/
		/*	S_IFDIR directory		*/
		/*	S_IFCHR character special	*/
		/*	S_IFBLK block special		*/
		/*	S_IFREG regular			*/
		/*	S_IFIFO fifo			*/
		/* only unlink "regular" files		*/

			if ((statb.st_mode & S_IFMT) == S_IFREG) 
			{
		    	reg_found = TRUE;
		    /* delete if root or have write permission */
		    	if ((!userId || !access(filename, 2)) &&
					!unlink(filename)) 
				{
					found = TRUE;
					del_fname(filename);
		    	} 
				else 
					debug(0, ("didn't unlink %s\n", filename));
			} 
			else 
		    	debug(0, ("not reg file %s\n", filename));
	    } 
		else 
		{
			debug(0, ("unable to stat %s\n", filename));
			reg_found = TRUE;	/* so we call err_handler() */
	    }
	}
	/* fill in response header */
	if (found) 
	{
	    addr->hdr.res = SUCCESS;
	    addr->hdr.stat = NEW;
	} 
	else if (!reg_found) 
	{
	    debug(0, ("delete:  no regular files\n"));
	    addr->hdr.res = FILE_NOT_FOUND;
	    return;
	} 
	else 
	{
	    /* use the errno value from the last error encountered */
	    err_handler(&addr->hdr.res, request, filename);
	}
	return;
}

