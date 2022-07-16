/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_GETPAGES_H
#define _SYS_GETPAGES_H

#ident	"@(#)head.sys:sys/getpages.h	11.1.4.1"

/*	The following list is used to keep track of regions locked
**	by getpages.
*/

typedef struct {
	reg_t	*gpr_rgptr;	/* Pointer to the region.	*/
	int	gpr_count;	/* Number of pages from this	*/
				/* region in spglist and 	*/
				/* fpglist.			*/
} gprgl_t;

extern gprgl_t	gprglst[];
extern int	gprgndx;


/*	The following structure is used to maintain a list of
 *	pages to be stolen by getpages.
 */

typedef struct {
	pte_t	*gp_ptptr;	/* Ptr to page table entry.	*/
	gprgl_t	*gp_rlptr;	/* Ptr to region list entry.	*/
} pglst_t;

/*	The following define sets the maximum size chunk which
 *	getpages can swap out at once.  The tuneable parameter
 *	tune.t_maxsc can never be made larger than this value.
 */

#define MAXSPGLST	100

/*	The following list is used to accumulate pages which we
**	wish to steal but which must be written to swap first.
*/

extern pglst_t	spglst[];
extern int	spglndx;

/*	The following list is used to accumulate pages which we
**	wish to steal and which have a current copy on swap or
**	a file and thus do not require any I/O to be done before
**	they can be stolen.  The reason for accummulating them
**	in a list instead of free each page as it is found is
**	to reduce the number of ATB purges required.
*/

#define	MAXFPGLST	100

extern pglst_t	fpglst[];
extern int	fpglndx;


/*	The following word is used to lock the getpages tables
**	described above so that vhand and sched can't both
**	use them at the same time.
*/

extern int	pglstlock;
extern int	pglstlk();
extern int	pglstunlk();

#endif	/* _SYS_GETPAGES_H */
