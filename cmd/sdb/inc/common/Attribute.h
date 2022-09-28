/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Attribute.h	1.3"
#ifndef Attribute_h
#define Attribute_h

typedef enum	{
			an_nomore,
			an_tag,
			an_name,
			an_child,
			an_sibling,
			an_parent,
			an_count,
			an_type,
			an_elemtype,
			an_elemspan,
			an_subscrtype,
			an_lobound,
			an_hibound,
			an_basetype,
			an_resulttype,
			an_argtype,
			an_bytesize,
			an_bitsize,
			an_bitoffs,
			an_litvalue,
			an_stringlen,
			an_lineinfo,
			an_location,
			an_lopc,
			an_hipc,
			an_visibility,
			an_scansize,
		} Attr_name;

typedef enum	{
			af_none,
			af_tag,
			af_int,
			af_locdesc,
			af_stringndx,
			af_coffrecord,
			af_coffline,
			af_coffpc,
			af_spidoffs,
			af_fundamental_type,
			af_symndx,
			af_reg,
			af_addr,
			af_local,
			af_visibility,
			af_lineinfo,
			af_attrlist,
			af_cofffile,
			af_symbol,
			af_bdioffs,
			af_bdiline,
			af_elfoffs,
		} Attr_form;

union Attr_value {
		void *	ptr;
		long	word;
	};

struct Attribute {
			int		name : 16;	// cfront bug: Attr_name
			int		form : 16;	// cfront bug: Attr_form
			Attr_value	value;
		}; 
	
class NameEntry;

#endif

// end of Attribute.h

