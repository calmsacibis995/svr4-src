/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexp/common/format.h	1.3"

#include "TYPE.h"
#include "Vector.h"

extern char *fmt_str( unsigned char *p, int count, int count_was_explicit );

extern char *default_fmt( TYPE );

extern int parse_fmt( char *fmt, int &count, int &length,
						char *&mode, int &explicit );

extern int format_bytes( Vector &raw, char *label, char* sep, char *fmt,
						TYPE & );

extern unsigned long sext( unsigned long l, int size );
