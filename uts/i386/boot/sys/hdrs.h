/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/sys/hdrs.h	1.1.2.1"

#include "filehdr.h"
#include "aouthdr.h"
#include "scnhdr.h"

struct coffhdrs {
	struct filehdr	mainhdr;
	struct aouthdr	secondhdr;
};

