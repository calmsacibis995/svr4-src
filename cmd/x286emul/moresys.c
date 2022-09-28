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

#ident	"@(#)x286emul:moresys.c	1.1"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysi86.h>
#include "vars.h"

extern int errno;

Exec( N, A )
	unsigned long N, A;
{
	return exec_common( OFF(N), SEL(N), OFF(A), SEL(A), 0, 0 );
}

Exece( N, A, E )
	unsigned long N, A, E;
{
	return exec_common( OFF(N), SEL(N), OFF(A), SEL(A), OFF(E), SEL(E) );
}

exec_common( offPath, selPath, offArgv, selArgv, offEnvv, selEnvv )
	unsigned offPath, selPath;      /* the program */
	unsigned offArgv, selArgv;      /* the args */
	unsigned offEnvv, selEnvv;      /* the environment */
{
	char **argv386;                 /* pointers to arg strings */
	char **envv386;                 /* pointers to env strings */
	int    argc;                    /* how many args */
	int    envc;                    /* how many envs */
	char  *prog;                    /* program to exec */
	unsigned short	* Argv,		/* pointer to 286 arg pointers */
			* Envv;		/* pointer to 286 env pointers */
	unsigned short * p;             /* temporary pointer */
	int i;                          /* for counting and indexing */
	int save_errno;                 /* holds errno after bad exec */

	prog = (char *)cvtchkptr(MAKEPTR(selPath,offPath));
	Argv = (unsigned short *)cvtchkptr(MAKEPTR(selArgv,offArgv));
	Envv = (unsigned short *)cvtchkptr(MAKEPTR(selEnvv,offEnvv));

	/*
	 * count argument pointers
	 */
	argc = 0;
	if ( Argv ) {
		p = Argv;
		while ( 1 ) {
			if ( Ldata ) {
				if ( p[0] == 0 && p[1] == 0 )
					break;
				else
					p++;
			} else {
				if ( p[0] == 0 )
					break;
			}
			p++; argc++;
		}
	}

	/*
	 * count environment pointers
	 */
	envc = 0;
	if ( Envv ) {
		p = Envv;
		while ( 1 ) {
			if ( Ldata ) {
				if ( p[0] == 0 && p[1] == 0 )
					break;
				else
					p++;
			} else {
				if ( p[0] == 0 )
					break;
			}
			p++; envc++;
		}
	}

	/*
	 * allocate space for arg pointers and env pointers
	 */
	argv386 = (char **)getmem( argc * 4 + 4 );
	envv386 = (char **)getmem( envc * 4 + 4 );

	/*
	 * set up the arg pointers
	 */
	for ( i = 0; i < argc; i++ ) {
		int ptr286;

		if ( Ldata )
			ptr286 = MAKEPTR(Argv[2*i+1],Argv[2*i]);
		else
			ptr286 = MAKEPTR(Stacksel,Argv[i]);
		argv386[i] = cvtchkptr(ptr286);
	}
	argv386[argc] = 0;

	/*
	 * set up the env pointers
	 */
	for ( i = 0; i < envc; i++ ) {
		int ptr286;

		if ( Ldata )
			ptr286 = MAKEPTR(Envv[2*i+1],Envv[2*i]);
		else
			ptr286 = MAKEPTR(Stacksel,Envv[i]);
		envv386[i] = cvtchkptr(ptr286);
	}
	envv386[envc] = 0;

	execve( prog, argv386, envv386 );

	save_errno = errno;             /* free may change errno */
	free( argv386 );
	free( envv386 );
	errno = save_errno;
	return -1;
}

Break( where )
	int where;
{
	int ind;

	ind = SEL(where);
	where = OFF(where);
#define NO_BOZOS
#ifdef NO_BOZOS
	/* ind now contains the selector for the requested data segment.
	 * This is as good a place as any to make sure user is not trying
	 * to trash the stack */
	if ( ! ( ind > Stacksel ) ) {
		if ( ind < Stacksel ) {
			/*
			 * user is being a jerk
			 */
			errno = EINVAL;
			return -1;
		} else {
			/*
			 * user might be being a jerk
			 */
	/* SVR4.0 */	if ((BADVISE_flags & SI86B_PRE_SV) != SI86B_PRE_SV) {
			    if ( where < (char *)&where - (char *)TheStack ) {
				/* jerk... */
				errno = EINVAL;
				return -1;
			    }
	/* SVR4.0 */	}
		}
	}
#endif
	ind = SELTOIDX(ind);
	if ( ind >= Numdsegs ) {
		errno = EINVAL;
		return -1;
	}
	if (Dsegs[ind].base == BAD_ADDR ||
		(Dsegs[ind].type & 0xffff) != 1 || (Dsegs[ind].type >> 16) > 1){
		errno = EINVAL;
		return -1;
	}
	/*
	 * At this point, we have decided that the address specified by
	 * the user is somewhere in a valid segment, and that it is not
	 * below the stack.  Our task now is to make this segment be the
	 * needed size, and to throw away any "normal" segments above
	 * this address
	 */
	if ( where > Dsegs[ind].lsize ) {
		GrowSeg( ind, where );		/* make segment big enough */
		setsegdscr( IDXTOSEL(ind), Dsegs[ind].base, Dsegs[ind].lsize,
			Dsegs[ind].psize, 1 );
	} else {
/* SVR4.0 */
	    if ((BADVISE_flags & SI86B_PRE_SV) != SI86B_PRE_SV) {
		/*
		 * shrink segment down to correct size
		 */
		setsegdscr( IDXTOSEL(ind), Dsegs[ind].base, where,
			Dsegs[ind].psize, 1 );
		/*
		 * throw away any data segments above this one
		 */
		while ( ++ind < Numdsegs ) {
			if ( Dsegs[ind].psize != 0 ) {
				if ( Dsegs[ind].type == 1 ) {
					setsegdscr( IDXTOSEL(ind), 0, 0, 0, 2 );
				}
			}
		}
	    }
/* SVR4.0 */
	}
	return 0;
}
