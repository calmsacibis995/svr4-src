/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/truncate.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)truncate.c	3.7	LCC);	/* Modified: 17:01:59 12/1/88 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"
#include <errno.h>
#include <fcntl.h>

extern	int
	errno;

extern	char
	*memory();		/* Allocate a dynamic buffer */

extern long
	lseek();

extern void
	free();

extern int
	desc_used;

#if	defined(XENIX) || !defined(SYS5)
extern char	*mktemp();		/* Create a unique for tmp file name */
#else
extern char	*tempnam();		/* Create a unique for tmp file name */
#endif

extern	struct vFile *vfCache;

extern	int
	errno;



/*
 * Truncate -	causes a file to be truncated to length nbytes.
 */

int
truncate(nbytes, descriptor)
register long
	nbytes;			/* Length to truncate to */
int
	descriptor;		/* MS-DOS virtual file descriptor */
{
register int
	status,                 /* Return value from system calls */
	status2,		/* Return value from system calls */
	length;                 /* Number of bytes to read() */
register long
	count;			/* Number of bytes read */
int
	act_desc,		/* Actual descriptor */
	tmp_desc;		/* File descriptor of tmp file */
char
	tmpName[20],		/* Stores pointer to unique tmp filename */
	*buf;			/* Temporary buffer for file transfer */
register struct vFile
	*vdSlot = &vfCache[descriptor];

#ifdef	BERKELEY42

	/* If this file is currently opened for READ_ONLY reopen it for
	 * write access.
	 */
	if (!(vdSlot->flags && VF_OMODE)) {
		close(vdSlot->uDesc);
		do
			act_desc = open(vdSlot->pathName, O_RDWR, 0666);
		while (act_desc == -1 && errno == EINTR);
		if (act_desc < 0) {
			log("truncate: can't open `%s' %d\n", 
				vdSlot->pathName, errno);
			vdSlot->flags |= VF_INACTV;
			desc_used--;
			return FALSE;
		}

		vdSlot->uDesc = act_desc;
	}

	if (ftruncate(vdSlot->uDesc,(int)nbytes) < 0) {
		log("truncate: can't truncate `%s' %d\n", 
			vdSlot->pathName, errno);
		vdSlot->flags |= VF_INACTV;
		desc_used--;
		return FALSE;
	}

	vdSlot->rwPtr = nbytes;
	return TRUE;
#else	/* BERKELEY42 */
	close(vdSlot->uDesc);
	do
		act_desc = open(vdSlot->pathName, O_RDWR, 0666);
	while (act_desc == -1 && errno == EINTR);
	if (act_desc < 0) {
		log("truncate: can't open `%s' %d\n", vdSlot->pathName, errno);
		vdSlot->flags |= VF_INACTV;
		desc_used--;
		return FALSE;
	}

	vdSlot->uDesc = act_desc;
	(void) lseek(act_desc, 0L, 0);

	strcpy(tmpName, "/tmp/br_XXXXXX");
	mktemp(tmpName);

	do
		tmp_desc = open(tmpName, O_RDWR | O_CREAT, 0666);
	while (tmp_desc == -1 && errno == EINTR);
	if (tmp_desc < 0) {
		log("truncate: can't open %s %d\n", tmpName, errno);
		return FALSE;
	}

	/* Allocate a temporary buffer for copying */
	buf = memory(MAX_OUTPUT);

	/* Copy data into tmp file */
	for (count = 0; nbytes > count; count += status) {
		length = (nbytes > MAX_OUTPUT) ? MAX_OUTPUT : nbytes;
		do
			status = read(act_desc, buf, (unsigned) length);
		while (status == -1 && errno == EINTR);
		if (status < 0)
			goto error1;
		do
			status2 = write(tmp_desc, buf, (unsigned) status);
		while (status2 == -1 && errno == EINTR);
		if (status2 != status)
			goto error1;
	}

	/* Truncate file to zero length */
	close(vdSlot->uDesc);
	do
	    act_desc = open(vdSlot->pathName, O_RDWR | O_TRUNC, 0666);
	while (act_desc == -1 && errno == EINTR);
	if (act_desc < 0) {
	    vdSlot->flags |= VF_INACTV;
	    desc_used--;
	    goto error1;
	}

	/* Remember actual descriptor */
	vdSlot->uDesc = act_desc;
	vdSlot->flags &= ~VF_INACTV;

	/* Write nbytes of the tmp file back to the original file */
	(void) lseek(tmp_desc, 0L, 0);
	for (count = 0; nbytes > count; count += status) {
		length = (nbytes-count > MAX_OUTPUT) ? MAX_OUTPUT : nbytes-count;
		do
			status = read(tmp_desc, buf, (unsigned) length);
		while (status == -1 && errno == EINTR);
		if (status < 0)
			goto error1;
		do
			status2 = write(act_desc, buf, (unsigned) status);
		while (status2 == -1 && errno == EINTR);
		if (status2 != status)
			goto error1;
	}
	vdSlot->rwPtr = count;

	/* Free buffer, filename, and unlink tmp file */
	close(tmp_desc);
	unlink(tmpName);
	free(buf);
	return TRUE;

error1:
	close(tmp_desc);
	unlink(tmpName);
	free(buf);
	return FALSE;
#endif	/* BERKELEY42 */
}
