/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Tag1.h	1.3"
//
// List of all of the tag values known to the debugger.
//

#ifndef DEFTAG
#define DEFTAG(X, Y)
#endif

DEFTAG(		t_none,			"t_none"		)

DEFTAG(		pt_startvars,		"pt_startvars"		)
// variable tags must go after here

DEFTAG(		t_argument,		"t_argument"		)
DEFTAG(		t_global,		"t_global"		)
DEFTAG(		t_variable,		"t_variable"		)
DEFTAG(		t_unionmem,		"t_unionmem"		)
DEFTAG(		t_structuremem,		"t_structuremem"	)

// variable tags must go before here
DEFTAG(		pt_endvars,		"t_endvars"		)

DEFTAG(		t_entry,		"t_entry"		)
DEFTAG(		t_extlabel,		"t_extlabel"		)

DEFTAG(		pt_starttypes,		"t_starttypes"		)
// type tags must go after here

DEFTAG(		t_arraytype,		"t_arraytype"		)
DEFTAG(		t_pointertype,		"t_pointertype"		)
DEFTAG(		t_reftype,		"t_reftype"		)
DEFTAG(		t_functiontype,		"t_functiontype"	)
DEFTAG(		t_structuretype,	"t_structuretype"	)
DEFTAG(		t_uniontype,		"t_uniontype"		)
DEFTAG(		t_enumtype,		"t_enumtype"		)
DEFTAG(		t_enumlittype,		"t_enumlittype"		)
DEFTAG(		t_functargtype,		"t_functargtype"	)
DEFTAG(		t_discsubrtype,		"t_discsubrtype"	)
DEFTAG(		t_bitfieldtype,		"t_bitfieldtype"	)
DEFTAG(		t_stringtype,		"t_stringtype"		)
DEFTAG(		t_bitfield,		"t_bitfield"		)
DEFTAG(		t_typedef,		"t_typedef"		)
DEFTAG(		t_unspecargs,		"t_unspecargs"		)

// type tags must go before here
DEFTAG(		pt_endtypes,		"t_endtypes"		)

DEFTAG(		t_sourcefile,		"t_sourcefile"		)
DEFTAG(		t_block,		"t_block"		)
DEFTAG(		t_label,		"t_label"		)
