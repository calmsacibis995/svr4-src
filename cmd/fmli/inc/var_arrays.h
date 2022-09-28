/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)fmli:inc/var_arrays.h	1.2"

struct v_array {
	unsigned short	tot_used;	/* number of elements used */
	unsigned short	tot_left;	/* number of elements unused */
	unsigned short	ele_size;	/* size of an element   */
	unsigned short	step_size;	/* size to increment by */
};

#define v_header(X)	(&((struct v_array *)(X))[-1])
#define v_body(X)	(&((struct v_array *)(X))[1])

/*
 * free the space used by a v_array
 */
#define array_destroy(X)	if (X) free(v_header(X))

/*
 * the length of a v_array to zero without actually freeing any space
 * this is useful for a v_array which is to be emptied and filled up again
 */
#define array_trunc(X)	(v_header(X)->tot_used += v_header(X)->tot_used, v_header(X)->tot_used = 0)

/*
 * returns the number of elements actually stored in the v_array
 */
#define array_len(X)	((X) ? v_header(X)->tot_used : 0)

/*
 * this allows the caller to specify the granularity with which
 * space is allocated for a v_array, based on how close the original
 * estimate of the array's size should be
 * Initially, the step size for incrementing a v_array's size is set
 * to the maximum of 10% of the initially allocated size and 16
 * (the size of a v_array header structure)
 */
#define array_ctl(X, Y)	(v_header(X)->step_size = (Y))

/*
 * slightly easier to use versions of array_append and array_delete
 */
#define var_append(T, A, E)	((A) = (T *) array_check_append(sizeof(T), (struct v_array *) (A), (E)))
/* #define var_append(T, A, E)	((A) = (T *) array_append(((A) == NULL) ? array_create(sizeof(T), 8) : (struct v_array *) (A), (E))) */
#define var_delete(T, A, I)	((A) = (T *) array_delete((struct v_array *) (A), (I)))

#define ptr_to_ele(X, Y)	((char *) v_body(X) + (Y) * (X)->ele_size)

extern struct v_array	*array_create();
extern struct v_array	*array_delete();
extern struct v_array	*array_append();
extern struct v_array	*array_insert();
extern struct v_array	*array_shrink();
extern struct v_array	*array_grow();
