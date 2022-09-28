/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/pckframe.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)pckframe.c	3.8	LCC);	/* Modified: 16:33:10 2/26/90 */

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


extern  struct tm 
*localtime();

extern	long
time();

extern	int
date();


/*		Routines Used for Packing Output Packets		*/


/*
 * btime() -		Takes the file creation time from a stat() call
 *			and converts it to MS-DOS time.
 */

int
btime(ptr)
register struct tm *ptr;
{
	register int 
	    hour,
	    min;

	    hour = ptr->tm_hour;
	    hour <<= 11;
	    min = ptr->tm_min;
	    min <<= 5;
	    return(hour | min | ptr->tm_sec/2);
}


/*
 * bdate() -		Takes file creation date and converts into an DOS date.
 */

int 
bdate(ptr)
register struct tm *ptr;
{
	register int 
	    year,
	    month;

	    year = ptr->tm_year - 80;
	    year <<= 9;
	    month = ptr->tm_mon + 1;
	    month <<= 5;
	    return(year | month | ptr->tm_mday);
}



/*
 * attribute() -	Returns a  file attribute byte in a packet for MS-DOS.
 *		If we do hidden file processing, we need to see the filename to
 *		determine if it's a hidden file.  The same scheme could be used for
 *		the "system" file type.  EricP.
 */

int 
#ifdef HIDDEN_FILES
attribute(filename,statBuf)
char *filename;
#else
attribute(statBuf)
#endif /* HIDDEN_FILES */
struct stat	*statBuf;
{
	register int dosAttr = 0;			/* MS-DOS File attributes */
#ifdef HIDDEN_FILES
	char *strrchr();
	char *tmp_fileP;

	if ((tmp_fileP = strrchr(filename,'/')) == NULL)	/* get only filename */
		tmp_fileP = filename;
	else
		tmp_fileP++;		/* advance past "/" */

	debug(0,("attribute:checking <%s> for hidden attrib\n",tmp_fileP));
	if (IS_HIDDEN(tmp_fileP))
		dosAttr |= HIDDEN;
#endif /* HIDDEN_FILES */


	if ((statBuf->st_mode & S_IFMT) == S_IFDIR)
		dosAttr |= SUB_DIRECTORY;
	else {
		/* Plain files always appear un-archived */
		dosAttr |= ARCHIVE;

		if (statBuf->st_uid == getuid()) {
			if (!(statBuf->st_mode & O_WRITE))
				dosAttr |= READ_ONLY;
		}
		else if (statBuf->st_gid == getgid()) {
			if (!(statBuf->st_mode & G_WRITE))
				dosAttr |= READ_ONLY;
		}
		else
			if (!(statBuf->st_mode & W_WRITE))
				dosAttr |= READ_ONLY;
	}
	return dosAttr;
}


/*
 *	pckframe() -	Packs frame header data into an output buffer.
 *			NOTE:  At present (7/30/86), this routine is never
 *			called with anything but a null file descriptor (fds)
 *			field, and thus, it is OK to pack the current UNIX
 *			clock time into the header.  If this routine is ever
 *			used in the future with an actual fds, pckframe should
 *			be modified to use the virtual DOS time stamp for the
 *			given file descriptor (taken from the vfCache kept in
 *			vfile.c.  Please see routines get_dos_time() and 
 *			get_vdescriptor() and module vfile.c).
 */

void
pckframe(addr, sel, seq, req, stat, res, fds, bcnt, tcnt, mode,
size, off, attr, date, fptr)
register struct output *addr;
int sel, seq, stat, res, fds, bcnt, tcnt, mode, size, off, attr,
date;
unsigned char req;
register struct	stat *fptr;
{
	register struct tm 
	    *tptr;

	    addr->pre.select = (char)sel;
	    addr->hdr.seq    = (char)seq;
	    addr->hdr.req    = (char)req;
	    addr->hdr.stat   = (char)stat;
	    addr->hdr.res    = (char)res;
	    addr->hdr.b_cnt  = (short)bcnt;
	    addr->hdr.t_cnt  = (short)tcnt;
	    addr->hdr.fdsc   = (short)fds;
	    addr->hdr.offset = (long)off;
	    addr->hdr.mode   = (mode) ? fptr->st_mode : 0;
	    addr->hdr.f_size = (size) ? fptr->st_size : 0;
	    addr->hdr.attr   = (attr) ? attribute(fptr) : 0;
	    if (date)
	    	tptr = localtime(&(fptr->st_mtime));
	    addr->hdr.date = (date) ? 0xffff & bdate(tptr) : 0;
	    addr->hdr.time = (date) ? 0xffff & btime(tptr) : 0;
}

/*
 * Name: pckRD
 * Purpose: packs a reliable delivery header with appropriate stuff
 * Arguments: addr,    Address of header.
 *            code,    Command code
 *            seq1,    Sequence number of command
 *            seq2,    Sequence number ack number
 *            options: Options flags
 *            version: Current version of RD
 */

void
pckRD( addr, code, seq1, seq2, options, tcbn, version )
register struct emhead *addr;
unsigned char code, options, tcbn, version;
unsigned short seq1, seq2;
{
            addr->code = code;
	    addr->dnum = seq1;
            addr->anum = seq2;
	    addr->options = options;
	    addr->tcbn    = tcbn;
	    addr->version = version;
}

/* rdtest: test whether a == b, which returns 0
 *                       a < b, which returns -1
 *                       a > b, which returns 1
 * This function is used for testing protocol state variables in the
 * reliable delivery code, which are subject to wrap-around.
 */

rdtest( a, b )
unsigned short a, b;
{
	if ( a == b) return 0;

	if ( !( a == RD_SEQ_MAX || b == RD_SEQ_MAX) ) /* not wrapped */
	{
		if (a < b)  return -1;
	        if (a > b)  return 1;
	}
	else           /* one of the two is wrapped */
	{
		if ( a == RD_SEQ_MAX )       /* a is wrapped */
		{
			if (b==0) return -1; /* a<b */
			return 1;            /* a>b */
		}
		else /* b is wrapped */
		{
			if (a==0) return 1; /* a>b */
			return -1;          /* a<b */
		}
	}
}
