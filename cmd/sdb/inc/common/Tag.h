/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Tag.h	1.1"

#ifndef Tag_h
#define Tag_h

#undef DEFTAG
#define DEFTAG(VAL, NAME) VAL,

enum Tag {
#include "Tag1.h"
};

char *tag_string(Tag);  // see xxx MORE.

#endif
