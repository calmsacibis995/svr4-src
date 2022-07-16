/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libadm:memory.c	1.1.4.1"

/*
 *  memory.c
 *	sysmem()	Get the amount of memory on the system
 *	asysmem()	Get the amount of available memory on the system
 */

/*
 *  G L O B A L   D E F I N I T I O N S
 *	- Header files referenced
 *	- Global functions referenced
 */


/*
 * Header files included:
 *	<sys/types.h>	    Data types known to the kernel (needed by sysi86.h)
 *	<sys/sysi86.h>	    Definitions for the sysi86() kernel call
 *	<sys/sysinfo.h>	    Internal Kernel definitions
 *	<sys/param.h>	    Internal Kernel Parameters
 *	<sys/sysmacros.h>   Internal Kernel Macros
 *	<fcntl.h>	    File control definitions
 *	<unistd.h>	    UNIX Standard definitions
 */

#include	<sys/types.h>
#include	<sys/sysi86.h>
#include	<sys/sysinfo.h>
#include	<sys/param.h>
#include	<sys/sysmacros.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<string.h>

/*
 * External references
 *	malloc()	Allocate a block of main memory
 *	free()		Free malloc()ed memory
 *	lseek()		Seek an open file
 *	read()		Read from an open file	
 *	close()		Close an open file
 */

extern	char           *malloc();
extern	void		free();
extern	long		lseek();
extern	int			read();
extern	int			close();

/*
 *  L O C A L   D E F I N I T I O N S
 *	- Local constants
 *	- Local function definitions
 *	- Static data
 */

/*
 * Local Constants
 *	TRUE		Boolean value, true
 *	FALSE		Boolean value, false
 *	NULL		No such address (nil)
 */

#ifndef	TRUE
#define	TRUE	(1)
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	NULL
#define	NULL	(0)
#endif


/*
 * Local Macros
 *	strend(p)	Find the end of the string pointed to by "p"
 */

#define	strend(p)	strrchr(p,'\0')

/*
 * sysmem()
 *
 *	Return the amount of memory configured on the system.
 *	On the 386, this is whatever is returned by the SI86MEM
 *	option to the sysi86() function.
 *
 *  Arguments: None
 * 	
 *  Returns:  long
 *	Whatever sysi86(SI86MEM) returns.
 */

long 
sysmem()
{
	return ((long) sysi86(SI86MEM));
}

/*
 * int asysmem()
 *
 *	This function returns the amount of available memory on the system.
 *	This is defined by
 *
 *  Arguments:  None
 *
 *  Returns:  long
 *	The amount of available memory or -1 with "errno" set if the value
 *	is not available.  On the 3B2, this is governed by file system access 
 *	to kernel memory.
 */

long
asysmem()
{

/**************
***************
***************
***************  A quick hack to compile the rest cleanly.
***************
***************  This MUST be removed when the following functionality
***************     is replaced with 386-specific code.
***************
***************
**************/

	return( -1 );
/***
/***	/* Automatic data */
/***	struct s3bsym  *symbtbl;	/* Ptr to symbol table */
/***	char	       *p;		/* Temporary pointer */
/***	int		freemem;	/* "freemem" value in the kernel */
/***	int	        memfd;		/* File desc for opening memory */
/***	long		rtnval;		/* Value to return */
/***	int		symbtblsz;	/* Size of the symbol table */
/***	int		okflag;		/* TRUE if all's well (so far) */
/***	int		i;		/* Counter of symbols */
/***	unsigned int	freemem_addr;	/* Vaddr of "freemem" in kernel */
/***
/***
/***
/***	/* 
/***	 * Get the virtual address of the memory info variable "int freemem"
/***	 *	1.  Get the size of the kernel symbol table
/***	 *	2.  malloc() space for the symbol table based on its size
/***	 *	3.  Read the symbol table into malloc()ed space
/***	 *	4.  Find the symbol "freemem" in the table
/***	 *	5.  Get the value of the symbol (it's the virtual address)
/***	 *	6.  Free the symbol table
/***	 */
/***
/***	okflag = TRUE;
/***	if ((sys3b(S3BSYM, (struct s3bsym *) &symbtblsz, sizeof(symbtblsz)) == 0) &&
/***	    (symbtbl = (struct s3bsym *) malloc(symbtblsz))) {
/***
/***	    (void) sys3b(S3BSYM, symbtbl, symbtblsz);
/***	    p = (char *) symbtbl;
/***	    for (i = symbtbl->count; i-- && (strcmp(p, "freemem") != 0) ; p = S3BNXTSYM(p)) ;
/***	    if (i >= 0) freemem_addr = S3BSVAL(p);
/***	    else okflag = FALSE;
/***
/***	    free((void *) symbtbl);
/***
/***	} else okflag = FALSE;
/***
/***	/* If an error occurred, we're through */
/***	if (!okflag) return(-1);
/***
/***	/* Open kernel memory */
/***	rtnval = -1;
/***	if ((memfd = open("/dev/kmem", O_RDONLY, 0)) > 0) {
/***
/***	    if ((lseek(memfd, freemem_addr, SEEK_SET) != -1) && 
/***	        (read(memfd, &freemem, sizeof(freemem)) == sizeof(freemem))) {
/***		rtnval = ctob(freemem);
/***	    }
/***
/***	    /* Close kernel memory */
/***	    (void) close(memfd);		
/***
/***	}  /* If successfully opened /dev/kmem */
/***
/***	/* Fini */
/***	return(rtnval);
***/
}
