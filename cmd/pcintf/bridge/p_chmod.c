/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_chmod.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_chmod.c	3.10	LCC);	/* Modified: 16:25:35 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<errno.h>
#include	<sys/stat.h>

/*				External Routines			*/


extern void 
	ftslash();		/* Translates frontslash to UNIX backslash */

extern int 
	errno,
	swap_in(),		/* Causes a virtual descriptor to be paged in */
	attribute(),		/* Set attribute bits in output frame */
	unmapfilename();	/* Unmaps a PCI filename if necessary */
#ifdef HIDDEN_FILES
	char *strrchr(),*strcpy();	/* NEEDS_WORK should use a header file */
#endif /* HIDDEN_FILES */


void
#ifdef	MULT_DRIVE
pci_chmod(in, drvNum, out)
int
	drvNum;
#else
pci_chmod(in, out)
#endif
struct input
	*in;				/* Request packet */
struct output
	*out;				/* Response packet */
{
char
	*fileName;			/* Name of file to chmod */
struct stat
	fileStat;			/* Unix file info */
int
	dosAttr = 0,			/* Returned DOS file attribute */
	newMode;			/* New Unix file modes */
#ifdef HIDDEN_FILES
	int attrib;
	int chg_hid_attr();
	char *tmp_fileP;
#define	MV		"mv"	/* for chmod of hidden files, a rename is req'd */
#endif /* HIDDEN_FILES */

	out->hdr.stat = NEW;
	out->hdr.res = SUCCESS;

	/* Massage MS-DOS pathname */
	fileName = in->text;
	cvt2unix(fileName);
#ifdef HIDDEN_FILES 
	attrib = 0xff;	/* search for everything */
	if (unmapfilename(CurDir, fileName, &attrib) == DUP_FILE_IN_DIR)
#else
	if (unmapfilename(CurDir, fileName) == DUP_FILE_IN_DIR)
#endif /* HIDDEN_FILES */ 
	{
	    out->hdr.res = ACCESS_DENIED;
	    return;
	}

#ifdef	MULT_DRIVE
	fileName = fnQualify(fileName, CurDir);
#endif	/* MULT_DRIVE */

	if (stat(fileName, &fileStat) < 0) {
		err_handler(&out->hdr.res, CHMOD, fileName);
		return;
	}

	/* mode != 0 ==> set modes otherwise get modes */
	if (in->hdr.mode != 0) {
		/* note: do not complain about archive bit */
		if (in->hdr.attr &
		    (SYSTEM | VOLUME_LABEL | SUB_DIRECTORY))
		{
		    out->hdr.res = ACCESS_DENIED;
		    return;
		}


		if (in->hdr.attr & READ_ONLY)
			newMode = fileStat.st_mode & ~ALL_WRITE;
		else
			newMode = fileStat.st_mode | O_WRITE;

		if (chmod(fileName, newMode) < 0) {
			err_handler(&out->hdr.res, CHMOD, fileName);
			return;
		}
#ifdef HIDDEN_FILES
		/* convert filename to *just* filename (no leading '/'s) */
		if ((tmp_fileP = strrchr(fileName,'/')) == NULL)
			tmp_fileP = fileName;
		else
			tmp_fileP++;			/* jump past the '/' */

		/* check if we're going to change the file attr */
		if ((((IS_HIDDEN(tmp_fileP)) && !(in->hdr.attr & HIDDEN)) ||
			(!(IS_HIDDEN(tmp_fileP)) && (in->hdr.attr & HIDDEN)) ) &&
			((fileStat.st_mode & S_IFMT) == S_IFREG) )	/* dirs not allowed */
	
					/* change it. (Requires moving the file) */
					if (chg_hid_attr(fileName,in->hdr.attr) < 0)
					{
						err_handler(&out->hdr.res, CHMOD, fileName);
						return;
					}
#endif /* HIDDEN_FILES */

	} 
	else
#ifdef HIDDEN_FILES
		out->hdr.attr = attribute(fileName,&fileStat);
#else
		out->hdr.attr = attribute(&fileStat);
#endif /* HIDDEN_FILES */

	return;
}

#ifdef HIDDEN_FILES
int
chg_hid_attr(oldfile,attr)
char *oldfile;
int attr;
{
char *only_file;	/* the actual base (input) filename (no leading /'s) */
char *newfile,*end_of_newfile;
static char *mvargs[] = {MV, NULL, NULL, NULL};	/* gets passed to exec_cmd() */
	
	debug(0xffff,("CHMOD: about to change %s %s a hidden file\n",oldfile,(attr&HIDDEN)?"FROM":"TO"));
	newfile = (char*) malloc(strlen(oldfile)+2);	
	if (newfile == NULL)
		return(-1);
	strcpy(newfile,oldfile);

	/* setup pointers to filename-only parts of the (possibly) full pathnames */
	if ((only_file = strrchr(oldfile,'/')) == NULL)
		only_file = oldfile;
	else
		only_file++;

	if ((end_of_newfile = strrchr(newfile,'/')) == NULL)
		end_of_newfile = newfile;
	else
		end_of_newfile++;

	*end_of_newfile = '\0';		/* we're going to append the new file to this */


	if (attr & HIDDEN)			/* we want to change TO a hidden file */
		strcat(end_of_newfile,".");		/* '.' is leading char of hidden file */

	else						/* change FROM a hidden file */
		while (*only_file == '.')		/* remove leading dots */
			only_file++;

	/* now just glue the filename onto the end of the new directory name */

	strcat(newfile,only_file);

	debug(0xffff,("Chmod: about to move <%s> to <%s>\n",oldfile,newfile));

	if (access(newfile, 0) == 0)  /* fail if target exists */
	{
		debug(0xffff,("Chmod: target file exists, can't move\n"));
		return (-1);
	}
	else
	{
		mvargs[1] = oldfile;
		mvargs[2] = newfile;
		if (exec_mv(MV, mvargs)) /* do the move */
		{
			/* update name/pathname in open file table */
			changename(oldfile, newfile);
		}
		else
			return (-1);
	}
	return(1);
}
#endif /* HIDDEN_FILES */
