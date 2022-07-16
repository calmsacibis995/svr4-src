/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/sys/setreid.c	1.2.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 


setreuid(ruid, euid) 
        int     ruid;
        int     euid;
{
        /*
         * Priv access checking are done by the system calls
	 * If either setuid or seteuid fails, it returns with -1.
         */

	int	error = 0;

        if ( ruid != -1 ) { 
		error = setuid(ruid);
		if ( error < 0 )
			return (error);
	} 
	if ( euid != -1 ) {
		error = seteuid(euid);
		if ( error < 0 )
			return (error);
	}
	return (0);
}


setregid(rgid, egid) 
        int     rgid;
        int     egid;
{
        /*
         * Priv access checking are done by the system calls
         * If either setgid or setegid fails, it returns with -1. 
         */
 
	int	error = 0;
        if ( rgid != -1 ) { 
                error = setgid(rgid);    
                if ( error < 0 ) 
                        return (error);
        } 
        if ( egid != -1 ) {
                error = setegid(egid);    
                if ( error < 0 ) 
                        return (error);
        }
        return (0);
} 
