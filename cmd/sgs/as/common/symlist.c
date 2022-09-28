/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symlist.c	1.3"

#include <stdio.h>
#include "gendefs.h"
#include "symbols.h"

extern void aerror();

/*
 *
 *	"alloc" is a function that permanently allocates blocks of
 *	storage.  The storage allocated by this routine is more
 *	economical than that provided by "malloc", however it cannot be
 *	returned to free storage.  Blocks allocated by "alloc" should
 *	never be passed to "free".
 *
 *	This function operates by obtaining large blocks of storage
 *	from "malloc", and then doles it out in small pieces without
 *	keeping around extra bookkeeping information for each small
 *	block.
 *
 *	The size of the blocks obtained from "malloc" is determined by
 *	the static array "incrs".  This contains successively smaller
 *	sizes of blocks to be obtained.  When a block of a given size
 *	cannot be obtained from "malloc", the next smaller size is
 *	tried.  The static variable "incptr" indexes the element of
 *	"incrs" that is currently being used for the size of the block
 *	to be allocated.
 *
 *	The current block from which storage is being allocated is
 *	addressed by the static variable "block".  The number of bytes
 *	remaining to be allocated in this block is given by the static
 *	variable "length".
 *
 *	Storage is allocated by first rounding the size up to an even
 *	number of bytes.  This avoids alignment problems if the storage
 *	is to be used for something other than characters.  If "length"
 *	is insufficient for allocating "size" bytes, another block of
 *	storage is obtained and "length" is set to the size of that
 *	block.  If necessary, a smaller size will be used to allocate
 *	this new block of storage.  Once a block of sufficient size is
 *	present, the "size" is subtracted from the remaining length,
 *	the variable "block" is incremented by the "size", and the
 *	pevious value of "block" is returned.
 *
 */

VOID *
alloc(size)
register int size;
{
	register int mod;
	register char *ptr;

	static unsigned incrs[] = { 2048, 1024, 512, 256, 128, 64, 0 };
	static short
		incptr = 0,
		length = 0;

	static char *block;

	/*
	 * Round up "size" to multiple of the wordsize
	 * of the machine the assembler is running on
	 */
	if ((mod = size % sizeof(int)) != 0)
		size += sizeof(int) - mod;

	if (size > length){
		while((block = (char *)malloc(incrs[incptr])) == 0){
			if (incrs[++incptr] == 0)
				aerror("Cannot allocate storage");
		}
		length = incrs[incptr];
	}

	ptr = block;
	block += size;
	length -= size;
	return(ptr);
}

