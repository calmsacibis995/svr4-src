/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_fstatus.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_fstatus.c	3.16	LCC);	/* Modified: 16:29:16 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<sys/stat.h>
#include	<errno.h>

#if !defined(XENIX) && defined(SYS5) || defined(EXL316)
#	include <sys/statfs.h>
#elif defined(ULTRIX)
#	include <sys/types.h>
#	include <sys/param.h>
#	include <sys/mount.h>
#elif defined(LOCUS)
#	include <dstat.h>
#endif

#if	defined(CCI) || defined(LOCUS)
extern	long
	diskfree(),
	disksize();
#endif	/* CCI || LOCUS */

extern  char *fnQualify();

void
#ifdef	MULT_DRIVE
pci_fsize(filename, recordsize, drvNum, addr, request)
int
	drvNum;
#else
pci_fsize(filename, recordsize, addr, request)
#endif
    char	*filename;		/* Name of file */
    int		recordsize;		/* Return size in units of records */
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    register	int
	length;				/* Length of file in units */
	
    struct	stat
	filstat;
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */


	if (recordsize <= 0) {
	    addr->hdr.res = FAILURE;
	    return;
	}

    /* Translate filename */
	cvt2unix(filename);

	/* If unmapping filename returns collision return failure */
#ifdef HIDDEN_FILES
	attrib = 0;
	if ((unmapfilename(CurDir, filename,&attrib)) == DUP_FILE_IN_DIR)
#else
	if ((unmapfilename(CurDir, filename)) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */
	{
	    addr->hdr.res = FAILURE;
	    return;
	}

#ifdef	MULT_DRIVE
	if ((stat(fnQualify(filename, CurDir), &filstat)) < 0)
#else
	if ((stat(filename, &filstat)) < 0)
#endif	/* MULT_DRIVE */
	{
	    err_handler(&addr->hdr.res, request, filename);
	    return;
	}

	length = (filstat.st_size + recordsize - 1) / recordsize;

    /* Fill-in response header */
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
	addr->hdr.offset = length;
}



void
#ifdef	MULT_DRIVE
pci_setstatus(filename, mode, drvNum, addr, request)
int
	drvNum;
#else
pci_setstatus(filename, mode, addr, request)
#endif
char
	*filename;		/* Name of file */
int
	mode;			/* Mode to change file to */
struct output
	*addr;			/* Pointer to response buffer */
int
	request;		/* DOS request number simulated */
{
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */
	/* Currently, only the mode of a file can be set */
	cvt2unix(filename);

#ifdef	MULT_DRIVE
	filename = fnQualify(filename, CurDir);
#endif

	/* If unmapping filename returns collision return failure */
#ifdef HIDDEN_FILES
	attrib = 0;
	if ((unmapfilename(CurDir, filename, &attrib)) == DUP_FILE_IN_DIR)
#else
	if ((unmapfilename(CurDir, filename)) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */
	{
	    addr->hdr.res = PATH_NOT_FOUND;
	    return;
	}

	if (chmod(filename, mode) < 0) {
		err_handler(&addr->hdr.res, request, filename);
		return;
	}

	/* Fill-in response header */
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
}



void
#ifdef	MULT_DRIVE
pci_fstatus(drvNum, addr, request)
int
	drvNum;
#else
pci_fstatus(addr, request)
#endif
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    struct	stat
	filstat;
#if	!defined(CCI) && !defined(LOCUS)
    long 
	freeblk,
	totlblk;
#endif	  /* not CCI and not LOCUS */

#if !defined(XENIX) && defined(SYS5) || defined(EXL316)
	struct statfs sfs;

	if (statfs(CurDir, &sfs, sizeof (struct statfs), 0) < 0) 
	{
	    err_handler(&addr->hdr.res, request, CurDir);
	    return;
	}
#elif defined(ULTRIX)
	struct fs_data sfs;

	if (statfs(CurDir, &sfs) < 0)
		err_handler(&addr->hdr.res, request, CurDir);
#elif defined(LOCUS)	/* VER_28X */
	struct dstat ds_buf;

	if (dstat(CurDir, &ds_buf, sizeof(ds_buf), 0) < 0) 
	{
	    log("pci_fstatus: dstat failed, errno: %d\n", errno);
	    err_handler(&addr->hdr.res, request, CurDir);
	    return;
	}
#else	/* not SYS5 (exept XENIX), EXL316, ULTRIX or LOCUS */
	if (stat((*CurDir == '\0') ? "/" : CurDir, &filstat) < 0) {
	    err_handler(&addr->hdr.res, request,
					(*CurDir == '\0') ? "/" : CurDir);
	    return;
	}
#endif

    /* Fill-in response header */
	addr->hdr.res = SUCCESS;
	addr->hdr.stat = NEW;
#if !defined(XENIX) && defined(SYS5) || defined(EXL316)
	addr->hdr.b_cnt = (short)((sfs.f_bfree * sfs.f_bsize)/FREESPACE_UNITS);
	addr->hdr.mode = (short)((sfs.f_blocks * sfs.f_bsize)/FREESPACE_UNITS);
#elif defined(ULTRIX)
	/* ULTRIX reports the space in numbers of 1K blocks even if the actual
	** blocksize is something else, so don't use bsize here.
	*/
	addr->hdr.b_cnt = (short)((sfs.fd_req.bfreen * 1024)/FREESPACE_UNITS);
	addr->hdr.mode = (short)((sfs.fd_req.btot * 1024)/FREESPACE_UNITS);
#elif defined(LOCUS)
	addr->hdr.b_cnt = diskfree(ds_buf.dst_gfs) / FREESPACE_UNITS;
	addr->hdr.mode = (short)(disksize(ds_buf.dst_gfs) / FREESPACE_UNITS);
#else

	addr->hdr.b_cnt = diskfree(filstat.st_dev) / FREESPACE_UNITS;

#ifdef	ICT
	if ( addr->hdr.b_cnt > 32000 )
		addr->hdr.b_cnt = 29000;
#endif	/* ICT */

	addr->hdr.mode = (short)(disksize(filstat.st_dev) / FREESPACE_UNITS);

#endif
	debug(4, ("pci_fstatus:dev %#x, b_cnt %d, mode %d\n",
		filstat.st_dev, addr->hdr.b_cnt, addr->hdr.mode));
}

void
pci_devinfo(vdescriptor, addr, request)
    int		vdescriptor;		/* PCI virtual file descriptor */
    struct	output	*addr;		/* Pointer to response buffer */
    int		request;		/* DOS request number simulated */
{
    int adescriptor;                    /* Actual UNIX descriptor */
    int status;

/* Get actual UNIX file descriptor */
    if ((adescriptor = swap_in(vdescriptor, 0)) < 0)
    {
		addr->hdr.res = (adescriptor == NO_FDESC) ? FILDES_INVALID : FAILURE;
		return;
    }

    if (isatty(adescriptor))
    {
	/* is Not a file */
	/* filling in DOS device info word */
	/* with CTRL=0 ISDEV=1 EOF=1 RAW=1 ISCLK=0 ISNUL=0 */
	/* "reserved" bits = 0 */
	/* decide on bits ISCOT and ISCIN ("console output","console input" */
	/* Am not sure What this really means, am just seeing if */
	/* stdin, or stdout/stderr */
	switch (adescriptor)
	{
	case 0:
	    status = 0x00e1;    /* console input */
	    break;
	case 1:
	case 2:
	    status = 0x00e2;    /* console output */
	    break;
	default:
	    status = 0x00e0;    /* not stdio */
	    break;
	}
    }
    else /* is a file */
    {
	/* only need to set two bits: ISDEV=0.  EOF=0 when */
	/* "channel has been written" */
		if (write_done(vdescriptor))
	    	status = 0x0000;
		else
	    	status = 0x0040;
    }
    /* Fill-in response buffer */
    addr->hdr.res = SUCCESS;
    addr->hdr.stat = NEW;
    addr->hdr.mode = status;
}
