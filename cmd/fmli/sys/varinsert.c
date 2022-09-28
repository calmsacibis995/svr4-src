/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/varinsert.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

/*
 * insert an element at position "position" in a v_array
 */
struct v_array *
array_insert(array, element, position)
struct v_array	array[];
char	*element;
unsigned	position;
{
	register struct v_array	*ptr;

	ptr = v_header(array);
	if (position > ptr->tot_used)
		return array;
	ptr = v_header(array_grow(array, 1));
	if (position < ptr->tot_used - 1)
		memshift(ptr_to_ele(ptr, position + 1), ptr_to_ele(ptr, position), ptr->ele_size * (ptr->tot_used - position - 1));
	if (element != NULL)
		memcpy(ptr_to_ele(ptr, position), element, ptr->ele_size);
	return v_body(ptr);
}
