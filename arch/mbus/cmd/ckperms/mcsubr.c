/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/ckperms/mcsubr.c	1.3"

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<filehdr.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<ar.h>
#include	"defs.h"

/*
	check if file is a common object file, a stripped file or a
	archive file.  by checking the header info in the file
	and comparing it to the magic no. The default value returned
	is a regular file.
*/
char 
getfhdr (pname)
char 	*pname;
{
	FILE	*ifile;
	FILHDR	file_hdr;
	long	size;
	char	ret;
	if ((ifile = fopen(pname, "r")) == NULL) {
		errmsg (WARN, SAMLN, "cannot open file for reading header");
		return ('\0');
	}
	if (fseek(ifile,0L,2) != 0) {
		ret = '\0';
		goto endr;
	}
	if ((size = ftell(ifile)) == EOF || size == 0 || size < sizeof (FILHDR)) {
		ret = '\0';
		goto endr;
	}
	if (fseek(ifile,0L,0) != 0) {
		ret = '\0';
		goto endr;
	}
	if (fread(&file_hdr, sizeof(file_hdr), 1, ifile) != 1)  {
		ret = '\0';
		goto endr;
	}

	if (ISCOFF (file_hdr.f_magic)) {
		ret = (file_hdr.f_nsyms == 0 ? 's' : 'x');
		goto endr;
	}
	if (strncmp((char *) &file_hdr,ARMAG, SARMAG) == 0){
		ret = 'a';
		goto endr;
	}
	ret = 'r';
endr:
	fclose (ifile);
	return (ret);
}

/*
	checks if file is a directory or a special file and returns
	corressponding code.
*/
char	
getft (mode, pname)
ushort	mode;
char	*pname;
{
	char	ret;
	switch (mode&S_IFMT) {
	case S_IFDIR :
		return ('d');
	case S_IFBLK :
		return ('b');
	case S_IFCHR :
		return ('c');
	case S_IFIFO :
		return ('p');
	default :
		if ((ret = getfhdr (pname)) == '\0')
			ret = 'r'; /* 'r' is the default */
		return (ret);
	}
}
