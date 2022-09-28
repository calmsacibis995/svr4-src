/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/code.c	1.18"
#include <stdio.h>
#include <string.h>
#include <libelf.h>
#include "systems.h"
#include "symbols.h"
#include "section.h"
#include "gendefs.h"
#include "expr.h"

int previous = 0;
int counter = -1;
int seccnt = 1;
short bitpos = 0;
#if DEBUG
extern short Oflag;	/* debugging flag */
#endif
extern long newdot;	/* up-to-date value of "." */

extern symbol	*dot;
extern unsigned short line;
extern char *filenames[];
extern symbol *lookup();
extern int txtsec;

struct scninfo *sectab;			/* section info table */
static int nsections = NSECTIONS;	/* number of allocated sections */



/* table for predefined sections  and their flags */
struct predef_sect_attributes
{
	char  *	sect_name;
	int 	sect_flag;
	long 	sect_type;
};

static struct predef_sect_attributes predefined_sections[] = {
	{ ".text",     SHF_EXECINSTR | SHF_ALLOC, SHT_PROGBITS },
	{ ".bss",      SHF_WRITE | SHF_ALLOC, SHT_NOBITS },
	{ ".data",     SHF_WRITE | SHF_ALLOC, SHT_PROGBITS },
	{ ".data1",    SHF_WRITE | SHF_ALLOC, SHT_PROGBITS },
	{ ".debug",    0, SHT_PROGBITS },
	{ ".comment",  0, SHT_PROGBITS },
	{ ".fini",     SHF_EXECINSTR | SHF_ALLOC, SHT_PROGBITS },
	{ ".init",     SHF_EXECINSTR | SHF_ALLOC, SHT_PROGBITS },
	{ ".line",     0, SHT_PROGBITS },
	{ ".rodata",   SHF_ALLOC, SHT_PROGBITS },
	{ ".rodata1",  SHF_ALLOC, SHT_PROGBITS },
	{ ".note",     0, SHT_NOTE },
	{ ".dynamic",  NOT_USER, SHT_DYNAMIC },
	{ ".got",      NOT_USER, SHT_PROGBITS },
	{ ".hash",     NOT_USER, SHT_HASH },
	{ ".interp",   NOT_USER, SHT_STRTAB },
	{ ".multi",    NOT_USER, SHT_PROGBITS },
	{ ".opthdr",   NOT_USER, SHT_NULL },
	{ ".pic",      NOT_USER, SHT_NULL },
	{ ".plt",      NOT_USER, SHT_PROGBITS },
	{ ".postld",   NOT_USER, SHT_NULL },
	{ ".shstrtab", NOT_USER, SHT_STRTAB },
	{ ".strtab",   NOT_USER, SHT_STRTAB },
	{ ".dynstr",   NOT_USER, SHT_STRTAB },
	{ ".symtab",   NOT_USER, SHT_SYMTAB },
	{ ".dynsym",   NOT_USER, SHT_DYNSYM },
	{ ".rela",     NOT_USER, SHT_RELA },
	{ ".rel",      NOT_USER, SHT_REL },
	{ "",          0, 0 }
};


static CODEBOX	*newcode();

CODEBOX	*Codebox;
codebuf	*Code;
int	Genline = 1;


/*
 *	Generate creates an intermediate representation from the info
 *	supplied thru its args. The intermediate representation consists
 *	of linked lists, one per user section, of CODEBOXES each of which 
 *	contain an array of codebuf structures.  
 *	Each codebuf contains information for either an opcode or an 
 *	instruction operand.  	(see codeout.h). 
 *
 *	These entries represent the encoded user program and
 *	will be used as input to the second pass of the assembler.
 */
void
Generate (nbits,action,value,sym,unevaluated)
BYTE nbits, unevaluated;
BYTE action;
long value;
symbol *sym;	/* sym may hold pointer to expression tree,
		instead of pointer to symbol table entry. */
{
	register codebuf	*code;
	register CODEBOX	*tail;

#if DEBUG
	if (Oflag) {
		if (bitpos == 0)
			(void) printf("(%d:%6lx)	",dot->sectnum,newdot);
		else
			(void) printf("		");
		(void) printf("%hd	%d	%hd	%.13lx	%lx\n",
			line,(short)nbits,action,value,sym);
	}
#endif
	
	if (sectab[dot->sectnum].type == SHT_NOBITS) {
		if  (nbits && value)
		{
			yyerror("Attempt to initialize a no-bits section");
		}
		else
			bitpos += nbits;
			newdot += bitpos/BITSPBY;
			bitpos %= BITSPBY;
		return;
			
	}

	/* the following "if" statment */
	/* was introduced for software workarounds. */
	/* these functions use it to count */
	/* the number bits which are generated. */
	/* in the normal case "counter == -1" end does not */
	/* affect the code generation */ 

	if ((counter >=0) && (sectab[dot->sectnum].flags & SHF_EXECINSTR))
	{
	   counter += nbits;
	   return;
	}

	tail = sectab[dot->sectnum].Codetail;
	if ((code = tail->c_end) >= &tail->c_buf[NCODE])
	{
		sectab[dot->sectnum].Codetail = tail = newcode(tail);
		code = tail->c_end;
	}

	code->cvalue = value;
	code->csym = sym;
	code->cnbits = nbits;
	code->caction = action;
	if (unevaluated)
		code->caction |= CB_EFLAG;
	code->cline = (unsigned short) Genline;
	Genline = 0;
	code->errline = line;
	++tail->c_end;

	bitpos += nbits;
	newdot += bitpos/BITSPBY;
	bitpos %= BITSPBY;
}
    
static long
lookup_predef_section(name)
register char *name;
{
        struct predef_sect_attributes *p;
	for (p = predefined_sections; (*p).sect_name[0] != '\0'; p++)
                if ( !strcmp(name, (*p).sect_name) )
		        return (((*p).sect_flag << 8) | (*p).sect_type);
	return -1;
}


/*
 *	cgsect changes the section into which the assembler is 
 *	generating info
 */
void
cgsect(newsec)
register int newsec;
{
	/*
	 * save current section "."
	 */

	sectab[dot->sectnum].size = newdot;

	/*
	 * save current section number
	 */

	previous = dot->sectnum;

	/*
	 * change to new section
	 */

	dot->sectnum = (short) newsec;
	dot->value = newdot = sectab[newsec].size;
}


/*
 * search for (and possibly define) a section by name
 */


int
mksect(sym,flags,type)
register symbol	*sym;
register int	flags;
register long	type;
{
	register struct scninfo	*sect;
	register int check_attrib = 0;

	if (seccnt >= nsections - 1)
		if ((sectab =(struct scninfo *) 
		    	realloc((struct scninfo *)sectab,
			(nsections *= 2) * sizeof(struct scninfo))) == NULL)
				aerror("cannot malloc section table");
	if ((sym->type & STT_SECTION))	/* is symbol a previously
					defined section? */
	{
		if ((flags) && (sectab[sym->sectnum].flags != flags))
			yyerror("Section attributes do not match");
		if ((type) && (sectab[sym->sectnum].type != type))
			yyerror("Section type does not match");

	} else 	if (!UNDEF_SYM(sym))
			yyerror("Section name already defined");
	else {
		if ((check_attrib = lookup_predef_section(sym->name)) >= 0) {
			/* predefined section*/
			if ((check_attrib >> 8) == NOT_USER)
				yyerror("Predefined Non-User Section");
			else {
				if (flags && (flags != (check_attrib >> 8)))
					yyerror("Section attributes do not match");
				if (type && (type != (check_attrib & 0xff)))
					yyerror("Section types do not match");
			}
		
			
		}
		

			seccnt++;
			sect = &sectab[seccnt];
			if (type != SHT_NOBITS)
				sectab[seccnt].Codehead = sectab[seccnt].Codetail
					= newcode((CODEBOX *)0);
			if (check_attrib >= 0){		/* predefined section */
				sect->flags = check_attrib >> 8;
				sect->type = check_attrib & 0xff;
			}
			else
				{
				sect->flags = flags;
				sect->type = (type) ? type : SHT_PROGBITS;
			}
			sect->name = sym->name;
			sym->type = STT_SECTION;
			sym->binding = STB_LOCAL;
			sym->sectnum = (short) seccnt;
		}
	return(sym->sectnum);
}

void
genstring ( p )
char *p ;
{
	int c;
	int val;

	while( (c = *p++) ){
		switch( c ) {

		case '\\':
			switch( c = *p++ ){

			case '\n':
				continue;

			default:
				val = c;
				goto mkcc;

			case 'n':
				val = '\n';
				goto mkcc;

			case 'r':
				val = '\r';
				goto mkcc;

			case 'b':
				val = '\b';
				goto mkcc;

			case 't':
				val = '\t';
				goto mkcc;

			case 'f':
				val = '\f';
				goto mkcc;

			case 'v':
				val = '\013';
				goto mkcc;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = c-'0';
				c = *p++;  /* try for 2 */
				if( (c>='0') && (c<='7') ){
					val = (val<<3) | (c-'0');
					c = *p++;  /* try for 3 */
					if((c>='0') && (c<='7') ){
						val = (val<<3) | (c-'0');
						}
					else --p;
					}
				else --p;

				goto mkcc;

				}
		default:
			val =c;
		mkcc:
			generate ( 8 , NOACTION , (long) val , NULLSYM) ;
			}
		}
	/* end of string or  char constant */

	/*
	 * and , finally, a NULL character to mark the end of the string
	 */

	generate ( 8 , NOACTION , (long) 0 , NULLSYM) ;


}

void
comment(string)
char *string;
{
	int prevsec;
	static int comsec = -1;
	
	if (comsec < 0) 
		comsec = mksect(lookup(".comment", INSTALL), 0, (long) SHT_PROGBITS);
	prevsec = previous;
	cgsect(comsec);
	/* It looks like a string, so treat it like one */
	genstring(string);

	cgsect(previous);
	previous = prevsec;
}


static CODEBOX *
newcode(old)
	CODEBOX	*old;
{
	CODEBOX	*new;

	if ((new = (CODEBOX *)malloc(sizeof(*new))) == 0)
		aerror("out of code memory");
	new->c_link = 0;
	new->c_end = &new->c_buf[0];
	if (old)
		old->c_link = new;
	return new;
}

void
bss(sym,val,alignment)
register symbol *sym;
long val,alignment;
{
	long mod;
	extern int bsssec;

	if (bsssec < 0)
		bsssec = mksect( lookup(_BSS, INSTALL),
				(SHF_ALLOC | SHF_WRITE), (long) SHT_NOBITS);
	if (alignment) {
		if (mod = sectab[bsssec].size % alignment)
			sectab[bsssec].size += alignment - mod;
		if (alignment > sectab[bsssec].addralign)
		  	sectab[bsssec].addralign = alignment;
	      }
	sym->value = sectab[bsssec].size;
	sectab[bsssec].size += val;
	sym->size = val;
	sym->sectnum = bsssec;
	sym->type = STT_OBJECT;
	if (!BIT_IS_ON(sym,BOUND))
		sym->binding = STB_LOCAL;
	if (TEMP_LABEL(sym))
		sym->flags &= ~GO_IN_SYMTAB;
	if (BIT_IS_ON(sym,SET_SYM)) {
		yyerror("error: multiply defined symbol");
		(void) fprintf(stderr,"\t\t... %s\n",sym->name);
	}

} /* bss */

void
sectabinit()
{
	if ((sectab = (struct scninfo *)calloc((unsigned) nsections,
			nsections * (sizeof(struct scninfo)))) == NULL)
		aerror("cannot malloc section table");
}

