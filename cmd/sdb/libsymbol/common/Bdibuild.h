/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Bdibuild.h	1.4"
#ifndef Bdibuild_h
#define Bdibuild_h

#include	"Bdi.h"
#include	"Fetalrec.h"
#include	"Itype.h"
#include	"Reflist.h"
#include	"Fetalline.h"

enum	Tag;
struct	Syminfo;

class Bdibuild {
	Bdi		bdi;
	Fetalrec	fetalrec;
	Fetalrec	fetaltype;
	Fetalline	fetalline;
	Reflist		reflist;
	long		entry_offset, entry_base;
	long		stmt_offset, stmt_base;
	char *		ptr;
	long		length;
	long		entryoff;
	long		nextoff;
	Tag		tag;
	void		skip_attribute( short );
	int		find_attribute( int );
	Attribute *	build_record( long );
	char		get_byte();
	short		get_short();
	long		get_long();
	void *		make_chunk( void *, int );
	char *		get_string();
	void		next_item( Attribute * );
	int		subscr_list( Attribute * );
	void		get_location( Attr_form &, Attr_value & );
	void		get_ft( Attr_form &, Attr_value & );
	void		get_udt( Attr_form &, Attr_value & );
	void		get_mft( Attr_form &, Attr_value & );
	void		get_mudt( Attr_form &, Attr_value & );
	void		sibling();
	void		location();
	void		name();
	void		fund_type();
	void		mod_fund_type();
	void		user_def_type();
	void		mod_u_d_type();
	void		byte_size();
	void		bit_offset();
	void		bit_size();
	void		stmt_list();
	void		low_pc();
	void		high_pc();
	void		element_list();
	void		subscr_data();
public:
			Bdibuild( int );
	long		first_file();
	Attribute *	make_record( long );
	Lineinfo *	line_info( long );
	int		cache( long, long );
	long		find_global( long, char ** );
	int		get_syminfo( long, Syminfo & );
	char *		get_name( long );
};

#endif

// end of Bdibuild.h

