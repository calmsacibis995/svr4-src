/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)master:xnamfs/stubs.c	1.3"

#include <sys/vnode.h>
#include <sys/errno.h>

/* Stubs file for XENIX shared data */
/* Stubs file for XENIX semaphores  */

/* kern-fs:xnamfs/xsem.c.c */
int	creatsem(){ return(ENOSYS); }
int	opensem(){ return(ENOSYS); }
int	sigsem(){ return(ENOSYS); }
int	waitsem(){ return(ENOSYS); }
int	nbwaitsem(){ return(ENOSYS); }
void	closesem(){ }
int	xsemfork(){ return(0); }
void	xseminit(){ }
/* void	unxsem_alloc() { }		*/
/* int	xsem_alloc() {return(ENOSYS); }	*/

/* kern-fs:xnamfs/xnamsubr.c */
struct vnode *xnamvp(){ return((struct vnode *)0); }

/* kern-fs:xnamfs/xsd.c */
int	sdget(){ return(ENOSYS); }
int	xsdfree(){ return(ENOSYS); }
void	xsdinit(){ }
void	xsdexit(){ }
int	xsdfork(){ return(ENOSYS); }
int	sdenter(){ return(ENOSYS); }
int	sdleave(){ return(ENOSYS); }
int	sdgetv(){ return(ENOSYS); }
int	sdwaitv(){ return(ENOSYS); }
void	unxsd_alloc(){ }
int	sdsrch(){ return(0); }
/* int	xsd_alloc() {return (ENOSYS); }	*/

/* int	xsdwstch() {return (0); }	*/
