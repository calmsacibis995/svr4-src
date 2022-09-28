/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/macros.h	1.5"

/*
** Global macros
*/

/****************************************
** imports
****************************************/

#include	"paths.h"

/***************************************/

#define	DYN_SYM		"DYNAMIC"

#define	DYN_USYM	"_DYNAMIC"

#define	EDATA_SYM	"edata"

#define	EDATA_USYM	"_edata"

#define END_SYM		"end"

#define	END_USYM	"_end"

#define	ETEXT_SYM	"etext"

#define ETEXT_USYM	"_etext"

#define FINI_SYM	"_fini"

#define	GOT_SYM		"GLOBAL_OFFSET_TABLE_"

#define GOT_USYM        "_GLOBAL_OFFSET_TABLE_"

#define INIT_SYM	"_init"

#define	INTERP_DEFAULT	LIBCSO_NAME
