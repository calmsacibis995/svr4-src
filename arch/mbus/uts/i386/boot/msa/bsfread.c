/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/boot/msa/bsfread.c	1.3"

/*
 *  This is the bootserver file read module for Multibus II System V/386 
 *  R3.1 (or greater) protected mode bootstrap loader.  
 */
#include "../sys/boot.h"
#include "../sys/error.h"
#include "../sys/s2main.h"
#include "../sys/dib.h"

extern	ushort	ourDS;
char	gbuf[4096];		/* generic buffer */
extern	int		dev_gran;
extern	ushort	gbuf_offset;
extern	ushort	end_of_file;

ulong	bytes_to_read;
ulong	read_blocks;

/******************************************************************************
*
* TITLE: BL_file_read
*
* CALLING SEQUENCE:
* 	BL_file_read(buffer, buffer_sel, buffer_size, actual, status);
*
* INTERFACE VARIABLES:
* 	 buffer
* 		is the 32-bit offset (for the selector given by buf_sel) to 
* 		memory where the data read from the file is to be returned.
* 	buf_sel
* 		is the SELECTOR of memory where the data read from the file 
* 		is to be returned.
* 	buffer_size
* 		is a 32-bit quantity containing the number of bytes to be read 
* 		from the file.
* 	actual
* 		is a POINTER to a 32-bit quantity where the actual number of 
* 		bytes read is to be returned.
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to 
* 		be returned.
*
* CALLING PROCEDURES:
* 	pboot
* 	pload
*
* CALLS:
* 	bs_file_read
* 	iomove
*
* ABSTRACT:
* 	This procedure implements the EPS level file read interface.
*
******************************************************************************/
BL_file_read(buffer, buffer_sel, buffer_size, actual, status)
register char	*buffer;
register ushort	buffer_sel;
register ulong	buffer_size;
register ulong	*actual;
register int	*status;
{	register ulong	c;

	/* Initialize count of bytes read and status */

	*actual = 0;
	*status = E_OK;
	
	/* Check for read of zero bytes */
	
	if (buffer_size > 0) {
	
		/* Copy from data remaining in manager's buffer (1st
		 * partial block) 
		 */

		if ((unsigned)dev_gran >= gbuf_offset) {
			bytes_to_read = dev_gran - gbuf_offset;
			if (buffer_size < bytes_to_read)
				bytes_to_read = buffer_size;
		}
		else
			bytes_to_read = buffer_size;

		if (bytes_to_read > 0) {
			iomove(&gbuf[gbuf_offset], ourDS, (char *)buffer, buffer_sel, 
				bytes_to_read) ;
			gbuf_offset = gbuf_offset + bytes_to_read;
		}
		*actual = bytes_to_read;
		buffer_size = buffer_size - bytes_to_read;
	
		/* Read a number of whole blocks into caller's 
		 * buffer (Full blocks) 
		 */
		
		*status = E_OK;
		bytes_to_read = buffer_size & 0xfffff000;
		if (bytes_to_read > 0) {
			c = *actual;		/* save that for now */
			bs_file_read(buffer+c, buffer_sel, 
					 bytes_to_read, actual, status);
			*actual = *actual + c;
			buffer_size = buffer_size - bytes_to_read;
	 		if (*status != E_OK) {
			/* Don't try to read any more */
				end_of_file = TRUE;  
				gbuf_offset = dev_gran;
			}
		}
		/* Read a block into manager's buffer and copy data from it.
		 * (Last Partial Block).
		 */

		if ((buffer_size > 0) && (!end_of_file)) {

			 /* Read a block into the global buffer */

			bs_file_read(&gbuf[0], ourDS, dev_gran, &bytes_to_read,
					status);
			 
		 	/* Check for and save end of file condition */
		 	
			if (*status != E_OK) 
				end_of_file = TRUE;
			else
				gbuf_offset = 0;

			if (bytes_to_read >= buffer_size) {

				/* Copy data from gbuf into the caller's buffer 
				 */
				c = *actual;	
				iomove(&gbuf[0], ourDS, (char *)&buffer[c], 
					buffer_sel, buffer_size);
				gbuf_offset = buffer_size;
				*actual = *actual + buffer_size;
			}
		}
	}
}
