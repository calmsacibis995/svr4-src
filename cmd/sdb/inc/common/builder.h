/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/builder.h	1.1"
#ifndef builder_h
#define builder_h

#include	"Attribute.h"
#include	"Type.h"

Attribute *	find_attr(Attribute *, Attr_name);

char * 		name_string( Attr_name );

char * 		form_string( Attr_form );

char * 		type_string( Fund_type );

void		print_attr( Attribute * );

void		print_attrlist( Attribute * );

#endif

// end of builder.h

