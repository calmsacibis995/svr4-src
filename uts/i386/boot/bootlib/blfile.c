/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/bootlib/blfile.c	1.1.3.2"

/*
 *  This is the AT version of disk only BL_file_xxx interface
 *  exists solely for interface compatibility.
 *
 *  The interface of BL_file_xxx is IDENTICAL to the MB2 
 */
 
#include "../sys/boot.h"
#include "../sys/dib.h"
#include "../sys/error.h"
#include "../sys/libfm.h"

#ifdef MB2
extern
#endif
off_t	boot_delta;
off_t	disk_file_offset;
extern	bfstyp_t	root_fs_type;

/*
 *  BL_init for the disk must initialize the disk params and alttrack 
 *  mapping so that subsequent open and reads will work.
 *
 *  get_fs() then checks for boot file system (BFS) and initializes parameters
 *  for BFS and root files sytem.  It returns the root_delta: physical block 
 *  number  for the beginning of the root filesystem.
 */

BL_init()
{
	off_t	rootdelta;

	/* returns offset for root partition */
	rootdelta=get_fs();

	/* check if root is s5 and initialize */
	if ( s5_init(rootdelta) == 0 ) 
		root_fs_type = s5;
			
	/* code can be added to check for other file system types later */
}

/*
 *  BL_file_open for AT ignores dib.  *status gets error code from open
 *  and translates to either E_OK or ~E_OK.  see open().
 *  In addition, with file pointer disk_file_off is set to 0; subsequent
 *  read updates this.
 *
 *  Others will need to add more error checking as needed.
 */

BL_file_open(path, dib, status)
register char	*path;
register struct	dib	*dib;
register ulong	*status;
{
	if (open(path) <= 0)
		*status = E_FNEXIST;
	else {
		disk_file_offset = 0;
		*status = E_OK;
	}
}


BL_file_read(buffer, buffer_sel, buffer_size, actual, status)
register char 	*buffer;
register ushort	buffer_sel;
register ulong	buffer_size;
register ulong	*actual;
register ulong	*status;
{
	*actual = (ulong) read(disk_file_offset, buffer, 
				buffer_sel, buffer_size);
	if (*actual == buffer_size)
		*status = E_OK;
	else {
		debug(printf("BL_file_read failed: %d %d\n",  \
				disk_file_offset,*actual));

		*status = (*actual==0) ? E_FNEXIST : E_EOF;
	}
	disk_file_offset += *actual;
}


BL_file_close(status)
register ulong	*status;
{
	/* in.i_number = 0;		/* close of file indicator */
	*status = E_OK;
}
