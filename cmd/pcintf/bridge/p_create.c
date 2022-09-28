/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_create.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_create.c	3.21	LCC);	/* Modified: 16:28:27 2/26/90 */

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


/*				External Routines			*/


extern  void ftslash();         /* Translates frontslash to UNIX backslash */

extern	long get_dos_time();	/* returns the virtual DOS time stamp on file */
extern  int bdate();                /* Converts date to MS-DOS format */
extern  int btime();                /* Converts time into MS-DOS format */
extern  int swap_in();              /* Causes a virtual descriptor to be paged in */
extern  int attribute();            /* Set attribute bits in output frame */
extern  int create_file();          /* PCI access routine for creating files */

extern	char *memory();         /* Allocate a dynamic buffer */

#if	defined(XENIX) || !defined(SYS5)
extern char *mktemp();          /* Create a unique temporary filename */
#else
extern char *tempnam();		/* Create a unique temporary filename */
#endif


#ifdef ATT3B2
typedef unsigned short u_short;
#endif


extern  struct tm *localtime(); /* Load file date into tm structure */



/*			Imported Structures				*/


extern  int errno;              /* Contains error code from system calls */
extern  int print_desc[NPRINT];    /* File descriptor of print/spool file */

extern  char *print_name[NPRINT];  /* Filename of file to be printed */

struct temp_slot temp_slot[NSLOT];

void
#ifdef	MULT_DRIVE
pci_create(filename, mode, dosAttr, pid, drvNum, addr, request)
int drvNum;
#else
pci_create(filename, mode, dosAttr, pid, addr, request)
#endif
char
	*filename;			/* File name(with possible metachars) */
int
	mode,				/* Mode to open file with */
	dosAttr,			/* Dos file attribute */
	pid;				/* Process id of PC process */

struct output
	*addr;				/* Pointer to response buffer */
int
	request;			/* DOS request number simulated */
{
    register int
	vdescriptor, 			/* PCI virtual file descriptor */
	adescriptor;			/* Actual UNIX file descriptor */

    int
	stringlen, 			/* Length of filename */
	ddev,	 			/* dev of directory */
	tslot,                          /* temp file slot number */
	printx = -1,                    /* printer table index */
	unlink_on_sig = FALSE;
#ifdef HIDDEN_FILES 
	int attrib;
#endif /* HIDDEN_FILES */

    long
	dos_time_stamp;			/* Virtual DOS time stamp on file */

    ino_t 
	dino;				/*Inode of directory */ 

    char
	*dotptr, 			/* Pointer to "." in filename */
	pipefilename[MAX_PATH],         /* Filename of pipe file in /tmp */
	pathcomponent[MAX_PATH],	/* Path part of full filename */
	oldname[MAX_PATH];		/* DOS style name path */

    struct tm 
	*timeptr;			/* Pointer for locattime() */

    struct	stat
	filstat;			/* File status structure for stat() */


	dino = 0;
      /* Transform filename if need be */

	if (strpbrk(filename, "?*")) {		/* disallow wildcards */
	    addr->hdr.res = FILE_NOT_FOUND;	/* sic */
	    return;
	}
	if (dosAttr & SUB_DIRECTORY ||
				(mode == TEMPFILE && dosAttr & VOLUME_LABEL)) {
	    addr->hdr.res = ACCESS_DENIED;
	    return;
	}
	if (mode == PRINT)
	{
	  /* Create a tmp file for print spooling */
	    if ((printx = find_printx(-1)) == -1)
	    {
			log("Print file table full!\n\n");
			addr->hdr.res = TOO_MANY_FILES;
			return;
	    }
#if	defined(XENIX) || !defined(SYS5)
	    print_name[printx] = memory(MAX_FNAME + 1);
	    strcpy(print_name[printx], "/tmp/cr_XXXXXX");
	    print_name[printx] = mktemp(print_name[printx]);
#else
	    print_name[printx] = tempnam("", "pci");
#endif
	    strcpy(filename, print_name[printx]);
	    unlink_on_sig = TRUE;
	    log("Opened print file.  printx = %d, name = %s\n\n",printx,
		print_name[printx]);
	}
	else  /* not PRINT */
	{
	    if (mode == TEMPFILE)
		    strcpy(oldname, filename);
	    else if (!legal_dosname(filename))
			{
				addr->hdr.res = ACCESS_DENIED;
				return;
			}


	  /* Translate MS-DOS pathname into UNIX filename */
	    cvt2unix(filename);

	    if (mode != TEMPFILE && strncmp(filename, "/%pipe", PIPEPREFIXLEN) != 0)
		{
			if ((tslot = redirdir(filename, 0)) >= 0) 
			{
		    	strcpy(filename, "/tmp/");
		    	strcat(filename, temp_slot[tslot].fname);
			} /*endif "redirected" */
			strcpy(pathcomponent, "/");
	    }

#ifdef  JANUS
	    fromfunny(filename); /* this translates funny names, as well as */
			      /* autoexec.bat, con, ibmbio.com, ibmdos.com  */
#endif  /* JANUS */

#ifdef	MULT_DRIVE
	    filename = fnQualify(filename, CurDir);
#endif  /* MULT_DRIVE */

	    addr->hdr.t_cnt = 0;
	    /* If unmapping filename returns collision return failure */
#ifdef HIDDEN_FILES 
		attrib = HIDDEN;	
	    if ((unmapfilename(CurDir, filename,&attrib) == DUP_FILE_IN_DIR) ||
			((attrib & HIDDEN) && !(dosAttr & HIDDEN)))
#else
	    if ((unmapfilename(CurDir, filename)) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */ 
	    {
			log("create: filename collision:<%s>\n",filename);
			addr->hdr.res = ACCESS_DENIED;
			return;
	    }

	    if (mode == TEMPFILE)
	    {
		if (strlen(oldname) == 0)   /* null string becomes root */
		    strcpy(filename, "/");

		dotptr = strlen(filename) + filename; /* point at ending null */

		if ((*(dotptr-1) != '/') && (*(dotptr-1) != '\\'))
		{
		    *(dotptr++) = '/';
		    *dotptr = '\0';         /* add in terminator */
		}

		/*** Check that directory is writeable ***/
		if (access(filename, 2) && ((errno == EACCES) || (errno == EROFS)))
		{
		    /* Not writeable */
		    if (stat(filename, &filstat))
		    {
				err_handler(&addr->hdr.res, request, filename);
				return;     /* Directory has serious problems */
		    }
		    dino = filstat.st_ino;  /* Remember directory inode */
		    ddev = filstat.st_dev;
		    strcpy(filename, "/tmp/00XXXXXX");
		    dotptr = &filename[5];
		}
		else
		{
		    /* add in the temp string */
		    strcat(filename, "00XXXXXX");
		}
		dotptr[1] --;
		do
		{
		    dotptr[1]++;
		    mktemp(filename);
		} while (stat(filename, &filstat) >= 0);

	    /* just return name portion to DOS */

		unlink_on_sig = TRUE;
		strcpy(addr->text, dotptr);
		strncpy(pathcomponent, filename, (dotptr - filename));
		pathcomponent[dotptr - filename] = '\0';
		mapfilename(pathcomponent, addr->text);
		addr->hdr.t_cnt = MAX_PATH;
	    } /*endif TEMP */
	    /* If this is for a pipe open the pipe in /tmp */
	    else if ((strncmp(filename, "/%pipe", PIPEPREFIXLEN)) == 0)
	    {
		    strcpy(pipefilename, "/tmp");
		    dotptr = strchr(filename, '.');
		    sprintf(dotptr+1, "%d", getpid());
		    strcat(pipefilename, filename);
		    strcpy(filename, pipefilename);
		    unlink_on_sig = TRUE;
	    } /*endif PIPE */
	}

      /* At this point am done converting filename. */
      /* Now, create the file, and get a virtual descriptor. */

	if (mode == CREATENEW && stat(filename, &filstat) >= 0)
	{
		addr->hdr.res = FILE_EXISTS;
		return;
	}

	vdescriptor = create_file(filename, 2, pid, unlink_on_sig, dosAttr, mode);

	if (vdescriptor < 0)
	{
	    addr->hdr.res = ACCESS_DENIED;
	    return;
	}

	/* Get the actual descriptor */
	if ((adescriptor = swap_in(vdescriptor, DONT_CARE)) < 0)
	{
	    addr->hdr.res =
			(adescriptor == NO_FDESC) ? FILDES_INVALID : ACCESS_DENIED;
	    return;
	}

	if ((fstat(adescriptor, &filstat)) < 0)
	{
	    err_handler(&addr->hdr.res, request, NULL);
	    return;
	}

    /* If for print/spooling, store file descriptor */
	if (printx != -1)
	    print_desc[printx] = vdescriptor;

	if ((mode == TEMPFILE) && dino)  /* if put the temp file in /tmp */
	{
	    /* then put filename in temp slot array, so can find later. */
	    for(tslot=0; tslot<NSLOT; tslot++)
	    {
		if (temp_slot[tslot].s_flags == 0)
		{
		    strcpy(temp_slot[tslot].fname, addr->text);
		    temp_slot[tslot].s_ino = dino;
		    temp_slot[tslot].s_dev = ddev;
		    temp_slot[tslot].s_flags = 1;
		    break;
		}
	    }/*endfor*/
	}

    /* Fill-in response header */
	addr->hdr.res    = SUCCESS;
	addr->hdr.stat   = NEW;
	addr->hdr.fdsc   = vdescriptor;
	addr->hdr.offset = lseek(adescriptor, 0L, 1);
	addr->hdr.inode = (u_short)filstat.st_ino;
	addr->hdr.f_size = filstat.st_size;
#ifdef HIDDEN_FILES
	addr->hdr.attr   = attribute(filename,&filstat);
#else
	addr->hdr.attr   = attribute(&filstat);
#endif /* HIDDEN_FILES */

	dos_time_stamp = get_dos_time (vdescriptor);
	timeptr = localtime(&(dos_time_stamp));
	addr->hdr.date = bdate(timeptr);
	addr->hdr.time = btime(timeptr);

	addr->hdr.mode = ((filstat.st_mode & 070000) == 0) ? 1 : 0;
}



legal_dosname(path)	/* return true iff legal DOS path syntax */
	char *path;
{
	char path2[MAX_FILENAME];	/* copy of path we can trash */
	char *filename;			/* filename base             */
	char *ext;			/* filename extension        */
	char *next;			/* next path component       */
	register char *c;
	register short i;

	static char illegal[] = ".\"/\\[]:|<>+=;,";

	if (!path)
		return 0;
	if (*path == '\\')
		path++;
	strcpy(path2, path);
	filename = path2;
	while (filename) {
		if (next = strchr(filename, '\\'))
			*next++ = '\0';
		if (ext = strchr(filename, '.')) {
			*ext++ = '\0';
			for (c = ext, i = 1; *c; c++, i++)
				if (i > 3 || *c < 0x20 || strchr(illegal, *c))
					return 0;
		}
		if (!(*filename))
			return 0;
		for (c = filename, i = 1; *c; c++, i++)
			if (i > 8 || *c < 0x20 || strchr(illegal, *c))
				return 0;
		filename = next;
	}
	return 1;
}



/*--------------------------------------------------------*/
/* Test for match of filename in relocated filename table
/* Return zero if no match.
/* Return index into temp filename table plus 1.
/*--------------------------------------------------------*/
int redirdir(filename, first_tslot)
	char *filename;
	int first_tslot;
{
	char dirname[MAX_PATH];
	char fname[14];
	struct stat filstat;
	register char *sp, *cp;
	register int ddev, tslot;
	register ino_t dino;

	for(cp=sp=filename; *cp; cp++)
		if ( *cp == '/') sp = cp;

	/* full pathname */
	if (*sp == '/')
	{	strcpy(fname, sp+1);
		*sp = 0;
		strcpy(dirname, filename);
		*sp = '/';
		if (dirname[0] == 0) strcpy(dirname, "/");
	} 
	else	/* relative path */
	{	
		strcpy(fname, sp);
		strcpy(dirname, ".");
	}
	if (fname[0] != '0')
	    return -1;
	if (stat(dirname, &filstat))
		return -1;

	dino = filstat.st_ino;
	ddev = filstat.st_dev;

	for (tslot = first_tslot; tslot < NSLOT; tslot++)
	{
	    if (temp_slot[tslot].s_flags
		&& temp_slot[tslot].s_ino == dino
		&& temp_slot[tslot].s_dev == ddev
		&& match(temp_slot[tslot].fname, fname, MAPPED))
		    return tslot;
	}
	return -1;
}
