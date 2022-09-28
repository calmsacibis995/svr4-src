/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symbols.h	1.8"
/*
 */

#include	"systems.h"
#include	<libelf.h>

#ifdef __STDC__
typedef void VOID;
#else
typedef char VOID;
#endif

extern VOID * malloc();
extern VOID * calloc();
extern VOID * realloc();
extern VOID * alloc();

typedef unsigned char BYTE;

#define BINDINGS	(STB_GLOBAL | STB_WEAK | STB_LOCAL)
#define	BOUND		0x8

#define UNDEF_SYM(sym)	((sym)->sectnum == SHN_UNDEF)
#define COMMON(sym)	((sym)->sectnum == SHN_COMMON)
#define ABS_SYM(sym)	((sym)->sectnum == SHN_ABS)

#define BSS_SYM(sym)	(sectab[(sym)->sectnum].type == SHT_NOBITS)
#define TEXT_SYM(sym)	(sectab[(sym)->sectnum].flags & SHF_EXECINSTR)


#define LOCAL(sym)	((sym->binding & BINDINGS  ) == STB_LOCAL) 
#define GLOBAL(sym)	((sym->binding & BINDINGS  ) == STB_GLOBAL) 
#define WEAK(sym)	((sym->binding & BINDINGS  ) == STB_WEAK)
#define	SECTION(sym)		(sym->type == STT_SECTION)
#define TEMP_LABEL(sym) (LOCAL(sym) && (sym->name[0] == '.'))

#define GO_IN_SYMTAB	1	/* symbol should go in symbol table */
#define SIZE_SET	2	/* symbol already has a size */
#define TYPE_SET	4	/* symbol already has a type */
#define SET_SYM		16	/* symbol defined by .set */

#define BIT_IS_ON(symbol,flag)	((symbol->flags) & (flag))

typedef	union
	{
		char	name[9];
		struct
		{
			long	zeroes;
			long	offset;
		} tabentry;
	} name_union;

typedef struct {
	char 	*tab_entry;
	long	currindex;
	long	size;
} string_table;
	


typedef	struct symtag
	{
		char	*name;
		long 	size;
		long	value;
		short	flags; 		/* internal flags */
		long	stindex;	/* index in object file symbol table */
		BYTE	type;
		BYTE 	binding;
		short	align;
		long	sectnum;	/* number of section where defined */
		struct symtag	*next;
	} symbol;

#define SYMBOLL	sizeof(symbol)

#define INSTALL	1
#define N_INSTALL	0


/* macros to convert between symbol table indices and symbol pointers */
#define GETSYMPTR(indx,sym) \
	{ extern symbol *symtab[];\
	extern unsigned long tablesize;\
	sym = &symtab[(indx)/tablesize][(indx)%tablesize]; }
