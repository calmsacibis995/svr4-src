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

#ident	"@(#)mbus:uts/i386/boot/msa/tapemgr.c	1.3"

#ifndef lint
static char tapemgr_copyright[] = "Copyright 1988 Intel Corporation 462802";
#endif

/*
 *  This is the tape manager module for Multibus II System V/386 R3.1 (or
 *	greater) protected mode	bootstrap loader.
 */
#include "../sys/boot.h"
#include "../sys/error.h"
#include "../sys/s2main.h"
#include "../sys/dib.h"

extern	ushort	ourDS;
char	gbuf[4096];		/* generic buffer */
extern	int		dev_gran;

ushort	gbuf_offset;
ushort	end_of_file;
long	whole_blocks;
ulong	bytes_to_read;
long	read_blocks;


/******************************************************************************
*
* TITLE: BL_init
*
* CALLING SEQUENCE:
*	BL_init;
*
* INTERFACE VARIABLES:
*	None
*
* CALLING PROCEDURES:
* 	pload
*
* CALLS:
*	None
*
* ABSTRACT:
*	Just a stub to satisfy an i/f requirement
*
******************************************************************************/
BL_init()
{
	/* dummy stub for tape device */
	return;
}

/******************************************************************************
*
* TITLE: BL_file_open
*
* CALLING SEQUENCE:
* 	BL_file_open(path, dib, status);
*
* INTERFACE VARIABLES:
* 	path
* 		is a POINTER to a null terminated string containing the
* 		pathname for the file.
* 	dib
* 		is a POINTER to the Driver Interface Block for the boot device.
* 		This parameter is passed from the Stage 1 loader to Stage 2.
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to
* 		be returned.
*
* CALLING PROCEDURES:
* 	pboot
*
* CALLS:
* 	None
*
* ABSTRACT:
* 	This procedure implements the EPS level file open interface.
*
******************************************************************************/
/* ARGSUSED */
BL_file_open(path, dib, status)
register char	*path[];
register struct	dib	*dib;
register int	*status;
{
	/* Validate the Device Type as a Tape Device Driver.
	 */
	if ((dib->hdr.device_type != TAPE_DIB) &&
		(dib->hdr.device_type != PCI_TAPE_DIB))
		*status = E_DEVICE_TYPE;
	else {
		if (dev_gran > MAX_DEV_GRAN)
		   *status = E_DEVICE_GRANULARITY;
		else {
		   /* Initialize the variables.
		    */
		   gbuf_offset = dev_gran; /* indicates nothing has been read */
		   end_of_file = FALSE;
		   *status = E_OK;
		}
	}
}

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
* 	tape_read
* 	iomove
*
* ABSTRACT:
* 	This procedure implements the EPS level file read interface.
*
******************************************************************************/
BL_file_read(buffer, buffer_sel, buffer_size, actual, status)
register char	*buffer[];
register ushort	buffer_sel;
register ulong	buffer_size;
register ulong	*actual;
register int	*status;
{	register ulong	c;
	register ulong	t;

	/* Initialize count of bytes read and status */

	*actual = 0;
	
	if (end_of_file)
		*status = E_EOF;
	else
		*status = E_OK;

	/* Check for read of zero bytes */
	
	if ( buffer_size > 0 ) {
	
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
		
		whole_blocks = buffer_size / dev_gran;
		*status = E_OK;

		while ((whole_blocks > 0) && (*status == E_OK) &&
			(!end_of_file)) {
			c = *actual;		/* save that for now */
			read_blocks = whole_blocks > 255 ? 255 : whole_blocks;
			t = (ulong)buffer ;
			t += c;
			tape_read(read_blocks, t, buffer_sel,
					 &bytes_to_read, status);
			*actual = bytes_to_read + c;
			buffer_size = buffer_size - bytes_to_read;
		 	if (*status == E_OK) {
				whole_blocks = whole_blocks - read_blocks;
			}
			else {
				if ((*status == E_EOF) ||
				    (*status == E_FILE_MARK) ||
				    (*status == E_END_OF_MEDIUM)) {
					/* Don't try to read any more */
					end_of_file = TRUE;
					gbuf_offset = dev_gran;
					*status = E_EOF;
				}
			}
		}
		
		/* Read a block into manager's buffer and copy data from it.
		 * (Last Partial Block).
		 */

		if ((buffer_size > 0) && (!end_of_file)) {

			 /* Read a block into the global buffer */

			tape_read(ONE_BLOCK, &gbuf[0], ourDS, &bytes_to_read,
					status);
			
		 	/* Check for and save end of file condition */
		 	
			if ((*status == E_FILE_MARK) ||
			    (*status == E_EOF) ||
			    (*status == E_END_OF_MEDIUM)) {
				end_of_file = TRUE;
				*status = E_EOF;
			}
			if (bytes_to_read >= buffer_size) {

				/* Copy data from gbuf into the
				 * caller's buffer
				 */
			
				c = *actual;	
				t = (ulong)buffer;
				t += c;
				iomove(&gbuf[0], ourDS, t,
					buffer_sel, buffer_size);
				gbuf_offset = buffer_size;
				*actual = *actual + buffer_size;
			}
		}
	}
}

/******************************************************************************
*
* TITLE: BL_file_close
*
* CALLING SEQUENCE:
* 	BL_file_close(status);
*
* INTERFACE VARIABLES:
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to
* 		be returned.
*
* CALLING PROCEDURES:
* 	pload
*
* CALLS:
* 	tape_seek
*
* ABSTRACT:
* 	This procedure implements the EPS level file close interface.
*
******************************************************************************/
BL_file_close(status)
register int	*status;
{
	/* set status */
	*status = E_OK;
	gbuf_offset = dev_gran;

	/* check to see whether EOF was reached since file was opened.
	 * If not, seek forward to just past the next file mark.
	 */
	if (end_of_file)
		end_of_file = FALSE;
	else
		tape_seek(status);
}

/******************************************************************************
*
* TITLE: BL_seek
*
* CALLING SEQUENCE:
* 	BL_seek(status);
*
* INTERFACE VARIABLES:
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to
* 		be returned.
*
* CALLING PROCEDURES:
* 	pload
*
* CALLS:
* 	tape_seek
*
* ABSTRACT:
* 	This procedure implements the EPS level seek interface.
*
******************************************************************************/
BL_seek(status)
register int	*status;
{
	tape_seek(status);
}
