/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_open.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_open.c	3.19	LCC);	/* Modified: 16:29:56 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<sys/stat.h>
#include	<time.h>
#include	<errno.h>
#include	<string.h>
#include	<xdir.h>
#ifdef RLOCK
#include	<rlock.h>
#endif	/* RLOCK */



/*				External Routines			*/


extern  void ftslash();         /* Translates frontslash to UNIX backslash */
extern  void getpath();         /* Extracts pathname portion from string */

extern	long get_dos_time();	/* Returns virtual DOS time stamp on file */

extern  int bdate();            /* Converts date to MS-DOS format */
extern  int btime();            /* Converts time into MS-DOS format */
extern  int match();            /* Match UNIX filename to DOS search pattern */
extern  int swap_in();          /* Causes a virtual descriptor to be paged in */
extern  int wildcard();         /* Determines if string has wildcards */
extern  int open_file();        /* Add a new file to file open table */
extern  int attribute();        /* Set attribute bits in output frame */
extern  int unmapfilename();    /* Unmaps a PCI filename if necessary */

extern  struct tm *localtime(); /* Load file date into tm structure */

extern struct temp_slot temp_slot[];

#ifdef ATT3B2
typedef unsigned short u_short;
#endif

/*			Imported Structures				*/


extern	int	
	errno;			/* Contains error code from system calls */
void
#ifdef	MULT_DRIVE
pci_open(filename, mode, attr, pid, drvNum, addr, request)
int
	drvNum;
#else
pci_open(filename, mode, attr, pid, addr, request)
#endif
    char	*filename;		/* File name(with possible metachars) */
    int		mode;			/* Mode to open file with */
    int		attr;			/* MS-DOS file attribute */
    int		pid;			/* Process id of PC process */
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    register int
	adescriptor,			/* Actual UNIX file descriptor */
	vdescriptor;			/* PCI virtual file descriptor */

    int
	tslot,
	hflg,				/* Hidden MS-DOS file attribute */
	allowDir,			/* Let open succeed on directory */
	found;				/* Loop flag */

    long
	dos_time_stamp;			/* virtual DOS time stamp on file */

    char
	*dotptr,			/* Pointer to "." in filename */
	pathcomponent[MAX_PATH],	/* Pathname component of filename */
	namecomponent[MAX_PATH],	/* Filename component of filename */
        mappedname[MAX_FILENAME],	/* mapped version of candidate
					*  directory entries
					*/
	pipefilename[MAX_PATH];         /* Filename of pipe file in /tmp */

    struct	tm 
	*timeptr;			/* Pointer for locattime() */

    struct	stat
	filstat;			/* File status structure for stat() */

    DIR
	*dirdp;				/* Pointer to opendir struct */

    struct	direct
	*direntryptr;			/* Pointer to directory entry */

#ifdef RLOCK  /* record locking */
    int		share;			/* file share access mode */
#endif  /* RLOCK */

#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

#ifdef RLOCK  /* record locking */
	/* At this point "mode" holds both the share (upper 4 bits) and
	   the open mode (lower four bits).
	*/
	/* Pull out the share bits */
	share = SHR_BITS(mode);

	/* Pull out the r/w open bits */
	mode = RW_BITS(mode);
#endif  /* RLOCK */

    /* "Clean-up" MS-DOS pathname */
	cvt2unix(filename);

	/*** Kludge to allow access to protected directories for tmps ***/
	if ((tslot = redirdir(filename, 0)) >= 0) {
	    strcpy(filename, "/tmp/");
	    strcat(filename, temp_slot[tslot].fname);
	}

#ifdef	JANUS
	fromfunny(filename); /* this translates funny names, as well as */
#endif  /* JANUS                  autoexec.bat, con, ibmbio.com, ibmdos.com  */

#ifdef	MULT_DRIVE
	filename = fnQualify(filename, CurDir);
#endif	/* MULT_DRIVE */

	/* If unmapping filename returns collision return failure */
#ifdef HIDDEN_FILES
	attrib = HIDDEN;		/* open returns hidden files, too */
	if ((unmapfilename(CurDir, filename, &attrib)) == DUP_FILE_IN_DIR) 
#else
	if ((unmapfilename(CurDir, filename)) == DUP_FILE_IN_DIR) 
#endif /* HIDDEN_FILES */
	{
	    addr->hdr.res = FILE_EXISTS;
	    return;
	}
 
	allowDir = (attr & SUB_DIRECTORY) ? TRUE : FALSE;
	hflg = (attr & HIDDEN) ? TRUE : FALSE;

    /* Split input string into pathname and filename */
	getpath(filename, pathcomponent, namecomponent); 

    /* If this is for a pipe open the pipe in /tmp */
	if ((strncmp(filename, "/%pipe", PIPEPREFIXLEN)) == 0) 
	{
	    strcpy(pipefilename, "/tmp");
	    dotptr = strchr(filename, '.');
	    sprintf(dotptr+1, "%d", getpid());
	    strcat(pipefilename, filename);
	    strcpy(filename, pipefilename);
	}

    /* If filename has wildcards search the directory for a matching file */
	else if (!wildcard(filename, strlen(filename))) 
	{
	    if (stat(filename, &filstat) < 0) 
		{
	        err_handler(&addr->hdr.res, request, filename);
			return;
	    }
	    if ((filstat.st_mode & S_IFMT) == S_IFDIR && !allowDir) 
		{
			addr->hdr.res = ACCESS_DENIED;
			return;
	    }
	} 
	else 
	{
#ifndef	WILDCARD_OPEN

		debug(0, ("pci_open:ERR wildcard file %s\n", filename));
		addr->hdr.res = FILE_NOT_FOUND;
		return;

#endif	/* !WILDCARD_OPEN */
#ifdef	WILDCARD_OPEN
	    if (strcmp(namecomponent, "????????.???") == 0) 
		{
			strcpy(filename, pathcomponent); /* Use Current directory */
			mode = 0;			/* With read only access */
	    }
	    else 
		{
			found = FALSE;

#ifdef	MULT_DRIVE
			chdir(pathcomponent);
#endif

		if ((dirdp = opendir(pathcomponent)) == NULL)
		{
		    addr->hdr.res = PATH_NOT_FOUND;
		    return;
		}
		while (direntryptr = readdir(dirdp)) 
		{
			if ((is_dot(direntryptr->d_name)) ||		/* ignore '.', */
			    (is_dot_dot(direntryptr->d_name)) ||	/* and '..', */
			    ((!hflg) && (direntryptr->d_name[0] == '.')))
				continue;
		        if ((stat(direntryptr->d_name, &filstat) < 0) 
					|| ((filstat.st_mode & S_IFMT) == S_IFDIR && !allowDir)) 
						continue;
	
			strcpy(mappedname,direntryptr->d_name);
			debug(0, ("%s:before mapfile:%s\n", "pci_open",
				mappedname));
			mapfilename(pathcomponent,mappedname);
			debug(0, ("%s:after mapfile:%s\n", "pci_open",
				mappedname));
		        if (match(mappedname, namecomponent, MAPPED)) 
				{
					strcpy(filename, direntryptr->d_name);
					closedir(dirdp);
					found = TRUE;
					break;
		        }
		}	/* end dir-search while */
		closedir(dirdp);
		if (!found) 
		{
		    addr->hdr.res = FILE_NOT_FOUND;
		    return;
		}
	  }
#endif	/* WILDCARD_OPEN */
	}

	if ((vdescriptor = open_file(filename, mode,
#ifdef RLOCK  /* record locking */
						share,
#endif  /* RLOCK */
						pid, request)) < 0) {
	    addr->hdr.res = -vdescriptor;	/* error code */
	    return;
	}

    /* Get the actual file descriptor from the virtual descriptor */
	if ((adescriptor = swap_in(vdescriptor, DONT_CARE)) < 0) 
	{
	    addr->hdr.res=(adescriptor==NO_FDESC) ? FILDES_INVALID : ACCESS_DENIED;
	    return;
	}

	if ((fstat(adescriptor, &filstat)) < 0) 
	{
	    err_handler(&addr->hdr.res, request, NULL);
	    return;
	}


    /* Fill-in response buffer */
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
	addr->hdr.fdsc = vdescriptor;
	addr->hdr.inode = (u_short)filstat.st_ino;
	addr->hdr.f_size = filstat.st_size;
#ifdef HIDDEN_FILES
	addr->hdr.attr = attribute(filename,&filstat);
#else
	addr->hdr.attr = attribute(&filstat);
#endif /* HIDDEN_FILES */

	dos_time_stamp = get_dos_time (vdescriptor);
	timeptr = localtime(&(dos_time_stamp));
	addr->hdr.date = bdate(timeptr);
	addr->hdr.time = btime(timeptr);

	addr->hdr.mode = ((filstat.st_mode & 070000) == 0) ? 1 : 0;
	return;
}
