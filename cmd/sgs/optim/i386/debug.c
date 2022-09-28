/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/debug.c	1.6"

/* ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *

**	This module gives the optimizer the ability to read elf-style
**	debugging information and to write this information out
**	to the optimized assembly file, suitably modified.
**	
**	During pass 1, the debugging information is scanned and
**	and saved in a temporary file, Dfile.  Also a queue is
**	built which contains pointers into Dfile ( right now these
**	pointers are just line numbers ). The queue contains
**	sufficient information to find the start and end in Dfile
**	of the debugging information for each function and the
**	lines in Dfile which may need to be changed after optimization.
**	
**	Also during pass 1 all information in the .line section 
**	is discarded: the optimizer keeps track of this information
**	using the ..LN labelling convention and a new version of
**	.line is created as the text nodes are printed.
**	
**	Debugging entries of importance to the optimizer have the
**	following tags ( see the header dwarf.h ).
**
**		1) TAG_global_subroutine, TAG_subroutine: these will
**		  get changed to TAG_inline_subroutine if the
**		  function is inline expanded
**
**		2) TAG_local_variable, TAG_formal_parameter: these may
**		  get their location attributes changed, if the optimizer
**		  puts them into registers.  In the future, we may
**		  also be able to handle  global ( or file static )
**		  variables which get put into registers for the
**		  duration of a function call.
**
**		3) TAG_label: user defined label. These may get
**		  removed by the optimizer, so we must be sure
**		  no reference to them remains in the .debug section.
**		
**	At the end of pass 2, pseudo variables in the .debug section
**	are given values using .set directives, so that the debugging
**	entries for each function properly reflect whether or not that
**	function has been inline expanded.

***** ***** ***** ***** ***** ***** ***** ***** ***** ***** */

#include <stdio.h>
#include <ctype.h>
#include <dwarf.h>
#include "defs"
#include "regal.h"
#include "debug.h" /* declarations for access functions to this module */
#include <malloc.h>
extern putasm();
extern unlink();
extern void fatal();

/* global data structures and variables */

typedef enum admode { Disp, CPUReg, other } AdMode; /* a la HALO */
typedef char * FN_Id;
typedef struct d_entry {	/* In memory rep of debug info
				   entry that could be of interest. */
	short TAG;	/* TAG_global_subroutine, TAG_subroutine,
			   TAG_inline_subroutine,
			   TAG_local_variable, TAG_formal_parameter,
			   TAG_label */
	FN_Id fnid;	/* For subroutine TAGs: type should be FN_Id */
	long loc_head, loc_tail;     /* Offsets in Dfile for lines 
					which may need to be changed. 
					For variables, this is the
					location attribute.  For
					functions, the TAG ( which
					may get changed to inline ). */
	AdMode mode; /* Disp or CPUReg or other */
	int regid;  /* e.g., reg_ebx */
	int displacement; /* value of displacement off of frame pointer
			     read from location entry */
	struct d_entry *next;	/* Next "interesting" entry */
} D_entry, *D_ptr;

static D_ptr D_head=NULL, D_tail, current_fn; /* Queue of "interesting"
						 entries. */
static FILE *Dfile;	/* Temporary file, created on pass 1, written
			   out in modified form on pass 2. */

/* private declarations for pass1 */
typedef enum tok {tok_label, tok_ps_byte, tok_ps_2byte, tok_ps_4byte, 
		  tok_ps_string, tok_dont_care, tok_end} token;
	/* In the optimizer's simplified view of the world,
	   there are only 6 assembly language tokens:
		tok_label: a label definition, value is pointer to first
			char
		tok_ps_byte, tok_ps_2byte,tok_ps_4byte,tok_ps_string: 
		    pseudo_ops returned by plookup()
		    value is the string operand of the pseudo-op
		tok_end: EOF or a section change directive
		tok_dont_care: what it says
	*/

static char *current_line, *tok_val, *next_char;
    /* pointers to beginning of line, value of token
       current_line is initialized by parse_debug,
       otherwise only get_token() sets them */

static init_debug_flag = 0; /* if parse_debug is never called we don't want
		       to produce any debugging info. */
static long linenumber;
static token current_token, get_token();

static D_ptr new_entry(); /* Allocates and returns pointer to new entry */
static void put_entry();  /* Push the given entry on the end of the queue */
static void init_debug();
static void set_offsets();
static void put_Dfile(); /* Write to Dfile */
static void skip_past_label(); /* skip this entry */
static void skip_attribute(); /* skip attribute based on form as
				 defined in dwarf.h */
static int parse_location(); /* parse the location entry, checking to
		       * see if it's a parameter or auto variable.
		       * If it is, update the new entry and return
		       * 1, otherwise return 0 ( we are not interested
		       * in globals, file or block statics */

	void
parse_debug(s)
char *s; /* passes in the current input pointer */
{

    extern long strtol();
    short tag_value;
    short target_attribute, attribute;
    int found_attribute;
    char *label,*tmpstr;

    D_ptr tmp_ptr;
    long starting_line;

    if(! init_debug_flag) {
	init_debug_flag = 1;
	init_debug();
    }

    next_char=current_line=s;
    current_token=get_token();
    while(current_token != tok_end) { /* Look for debug entries
					 until end of section. */

	target_attribute=0;
	while(target_attribute==0) { /* loop exits after the tag of an
		      interesting entry has been found */

	    do { /* until the .4byte following a label has
		    been read */

	        while(current_token != tok_label) {
	            current_token=get_token();
		    if (current_token==tok_end) return;
		}
			
		do { /* keep looking */
		    current_token=get_token();
		    if (current_token==tok_end) return;
		} while(current_token == tok_label);
		
	    } while(current_token != tok_ps_4byte);

	    /* .4byte: expect a length in one of the forms
		constant ( skip the entry )
		label ( skip the entry )
	        label-save_label
	    */
	    if(isdigit(*tok_val)){ /* It's a constant, skip entry */
		continue;
	    }
	    else {
		if((label=strdup(tok_val))==(char *) NULL)
		    fatal("parse_debug(): strdup failed\n");
		{ 
		    char *temp;
		    if(temp=strchr(label,'-')) *temp='\0';
		    else {
			skip_past_label(label);
			if(current_token==tok_end) return;
			continue;
		    }
		}
		current_token=get_token();
		if (current_token == tok_end) return;
	    }

	    /* Now look for TAG for this entry */
	    if(current_token != tok_ps_2byte) {
		skip_past_label(label);	
		if(current_token==tok_end) return;
		continue;
	    }
	    tag_value=(short)strtol(tok_val,(char **)NULL,0);

	    switch(tag_value) {
	    case TAG_subroutine:
	    case TAG_global_subroutine:
		target_attribute=AT_name;
		starting_line=linenumber; /* location of tag: may have to 
				  change if fn is inline expanded */
		current_token=get_token();
		tmp_ptr=new_entry(tag_value);
		set_offsets(tmp_ptr,starting_line,linenumber);
    	    	break;
	    case TAG_formal_parameter:
	    case TAG_local_variable:
		target_attribute=AT_location;
		current_token=get_token();
    	    	break; /* desired exit from loop */
	    case TAG_label:
		target_attribute=AT_low_pc;
		current_token=get_token();
		break;
	    default:
    	    	skip_past_label(label);
		if(current_token==tok_end) return;
    	    	continue;
	    } /* switch */
	} /* end of while(target_attribute==0) loop, we've found a tag */

	/* Now skip thru attribute list.  The only
	   attributes of interest are AT_location for
	   the variables, AT_name for subroutines,
	   AT_low_pc for user defined labels. */

	found_attribute=0;
	while(current_token==tok_ps_2byte) {

	    /* Got an attribute: figure out what it is */	

	    attribute=(short)strtol(tok_val,(char **)NULL,0);
	    if(attribute==target_attribute) {
		found_attribute=1;
		switch(attribute) {
		case AT_location:
		    starting_line=linenumber+1; /* current offset */
		    tmp_ptr=new_entry(tag_value);
		    if(parse_location(tmp_ptr)) {
		        set_offsets(tmp_ptr,starting_line,linenumber);
			put_entry(tmp_ptr);
		    }
		    else
			free(tmp_ptr);
		    break;
		case AT_name:
		    current_token=get_token();
		    if (current_token != tok_ps_string)
			fatal("parse_debug(): expected \".string\"\n");
			/* don't care about name, but could save
			   it here to check it jibes with FN_Id for
			   the function we are about to optimize. */
		    tmpstr=strdup(tok_val);
		    tmp_ptr->fnid = (FN_Id)tmpstr++; /* scan past " */
		    tmpstr = strchr(tmpstr,'\"');
		    current_fn=tmp_ptr;	/* remember function header */
		    if(tmpstr) {
			*tmpstr = '\0';
		   	 put_entry(tmp_ptr);
		    }
		    else
			fatal("parse_debug(): coudn't parse fn name\n");
		    current_token=get_token();
		    break;	
		case AT_low_pc:
		    starting_line=linenumber; /* offset of .2byte AT_low_pc */
		    tmp_ptr=new_entry(tag_value);
		    current_token=get_token();
		    if(current_token != tok_ps_4byte)
			fatal("parse_debug(): expected \".4byte\t<label>\"\n");
		    set_offsets(tmp_ptr,starting_line,linenumber+1);
		    put_entry(tmp_ptr);
		    break;
		default: fatal("parse_debug(): unexpected target_attribute\n");
		}
	    }
	    else { /* skip this attribute and keep looking */
		skip_attribute(attribute&FORM_MASK);
		continue;
	    }
	} /* while(current_token ... */
	if( found_attribute || (target_attribute == AT_location) ) {

		/* If we didn't find the target AT_location, then
		   this formal parm or variable has no location
		   so we don't care about it, e.g. in typedefs. */

	        skip_past_label(label);
		if(current_token==tok_end) return;
	        continue;
	}
	else {
	    fatal("parse_debug(): can't find attribute. \
		   current_line: %s\n",current_line);
        }
    }
} /* parse_debug() */

	static void
skip_past_label(label) /* gets the next token after label */
char *label;
{
    char save_char;
    register char *t;
    int found=0;

    while(!found) {

	if(current_token==tok_end) 
	    fatal("skip_past_label(): premature EOI\n");
	else if(current_token==tok_label) {
		t=current_line;
		FindWhite(t);
		t--; save_char= *t; /* save ':' */
		*t='\0';
		if(strcmp(current_line,label)==0) found=1;
		*t=save_char; /* restore line */
	}
	current_token=get_token();

    } /* while(!found) */

}

	static int
parse_location(ptr)
D_ptr ptr; 		/* put info in here, if you find it */
{
	/* We expect a location of the form
		1) OP_BASEREG OP_CONST OP_ADD ( displ )
		2) OP_REG
		3) OP_ADDR
	   In cases 1 & 2 enter the info and return 1.
	   In case 3 return 0.
	   In all three cases, get the next token after the entry.
	   Anything else is fatal. */

    extern long strtol();
    int entry_length; 	/* Length in bytes of the
				   entry ( after assembly ). */
    short atom; 		/* values defined in dwarf.h */
    current_token=get_token();
    if ((current_token != tok_ps_2byte) || !isdigit(*tok_val))
        fatal("parse_location(): expected .2byte <const>\n");
    entry_length=(int)strtol(tok_val,(char **)NULL,0);

    current_token=get_token(); /* OP_REG, OP_BASEREG or OP_ADDR atom */
    if ((current_token != tok_ps_byte) || !isdigit(*tok_val))
        fatal("parse_location(): expected .byte <atom>\n");
    atom=(short)strtol(tok_val,(char **)NULL,0);
    current_token=get_token();

    switch(atom) {

    case OP_ADDR: /* just make sure it looks right 
		     in the future we might want to
		     communicate information to sdb
		     if we put this guy in a register */
	if (current_token != tok_ps_4byte)
	    fatal("parse_location(): OP_ADDR operand\n");
	break;

    case OP_REG:
    case OP_BASEREG:
        if ((current_token != tok_ps_4byte) || !isdigit(*tok_val))
            fatal("parse_location(): expected .4byte <reg#>\n");
    
        /* Convert the number to internal register format and enter it */
    
        ptr->regid= (int)strtol(tok_val,(char **)NULL,0);
    
        if(atom == OP_REG) {		/* Register parm or auto */
            ptr->mode=CPUReg;
	    if(entry_length != 5)
                fatal("parse_location(): expected 5 byte length\n");
        }
    
        else if (atom == OP_BASEREG) {	/* stack parm or auto */
            ptr->mode=Disp;
	    current_token=get_token();
            atom=(short)strtol(tok_val,(char **)NULL,0);
    	    if ((current_token != tok_ps_byte) || (atom != OP_CONST))
                fatal("parse_location(): expected .byte OP_CONST\n");
	    current_token=get_token();
    	    if (current_token != tok_ps_4byte)
                fatal("parse_location(): expected .4byte <displacement>\n");
	    else if ( !isdigit(*tok_val) && (*tok_val != '-'))
                fatal("parse_location(): bad operand to .4byte\n");
	    ptr->displacement=(int)strtol(tok_val,(char **)NULL,0);
	    current_token=get_token();
            atom=(int)strtol(tok_val,(char **)NULL,0);
    	    if ((current_token != tok_ps_byte) || (atom != OP_ADD))
                fatal("parse_location(): expected .byte OP_ADD\n");
	    if(entry_length != 11)
                fatal("parse_location(): expected 11 byte length\n");
        }
	break;

    default:
    	fatal("parse_location(): expected OP_REG or OP_BASEREG\n");
    } /* switch */

    current_token=get_token();
    return (atom != OP_ADDR);
}

	static void
skip_attribute(form)
short form; /* 1040A */
			/* Skips an attribute entry, gets
			   the first token after this attribute. */
{
    short bytes_to_skip=0; /* Number of ( object file ) bytes
		    	      we must account for to skip this
		              attribute. */
    short skipped=0; /* counter */
    extern long strtol();

    switch(form) {

    default:
    case FORM_BLOCK4: /* Unused */
    case FORM_NONE: /* error */ 
        fatal("skip_attribute(): bad attribute format code %d\n",form);
	/* FALLTHRU */
    case FORM_DATA2:
	bytes_to_skip=2;
	break;

    case FORM_DATA8:;
	bytes_to_skip=8;
	break;

    case FORM_ADDR:
    case FORM_REF:
    case FORM_DATA4:;
	bytes_to_skip=4;
	break;

    case FORM_BLOCK2: /* Have to look at next two bytes for length */
	current_token=get_token();
	if ((current_token != tok_ps_2byte) || !isdigit(*tok_val))
	    fatal("skip_attribute(): expected .2byte <const>\n");
	bytes_to_skip=(short)strtol(tok_val,(char **)NULL,0);
        break;

    case FORM_STRING:
	current_token=get_token(); 
	if (current_token != tok_ps_string)
	    fatal("skip_attribute(): expected .string\n");
	break;

    } /* switch */

    while(skipped < bytes_to_skip) {
	current_token=get_token();

	switch(current_token) {
	case tok_end:
	    fatal("skip_attribute(): premature end of entry\n");
	    /* FALLTHRU */
	case tok_ps_byte:
	    skipped++;
	    break;
	case tok_ps_2byte:
	    skipped+=2;
	    break;
	case tok_ps_4byte:
	    skipped+=4;
	    break;
	case tok_label:
	    break;
	default:
	    fatal("skip_attribute(): unexpected pseudo_op\n");
	} /* switch */
    } /* while(skipped ... */
    if(skipped != bytes_to_skip)
	fatal("skip_attribute(): weird byte count\n");
    current_token=get_token();
} /* skip_attribute() */

	static token
get_token()

/* Side effects:
	1) Handles update of next_char
	2) Calls getstmnt at end of line and sets current_line
	3) Handles EOF and section change nasties
	4) Calls plookup() to figure out a pseudo-op
	5) Outputs lines to the temporary Dfile
	6) Prints the section change on stdout

	This is the only function in debug.c which 
		a) does any reads
		b) knows about assembly directives
		c) knows anything about input characters
		d) assigns to next_char and current_line
		   and tok_val 
*/

{
    char savechar; 
    register char *t;
    unsigned int pop; /* returned by plookup() */
    extern char *getstmnt();	/* defined in i386/local.c */
    extern plookup(); 		/* also */
#define NEWSECTION(x) prev_section=section; section=(x);

    /* find a label or regular instruction */

    t=next_char;
    SkipWhite(t);
    while((*t=='\0') ||(*t==CC)) {
	/* No more on this line, save it. */
	put_Dfile(current_line);
	current_line=getstmnt();
	if(current_line==NULL) {
	    return tok_end;
	}
	t=current_line;
	SkipWhite(t);
    } /* while */

    /* t now points at the next non white character
       in the input stream */

    if (t==current_line) { /* It's a label */
	tok_val=current_line;
	FindWhite(t); /* Get next_char ready for next call */
	next_char=t;
	return tok_label;
    }
    next_char=t;
    FindWhite(t);
    savechar= *t;			/* Mark end of pseudo-op */
    *t = '\0';
    pop=plookup(next_char);			/* Identify pseudo-op. */
    *t = savechar;			/* Restore saved character. */
    SkipWhite(t);

    /* t now points at the operand, if there is one */

    switch(pop){	/* dispatch on pseudo-op */
    case POTHER:	/* pseudo-op not found */
	fatal("get_token(): illegal directive \"%s\".\n",next_char);
	break;
	
/* Next three cases are all section changes.  */

    case DATA:
	NEWSECTION(CSdata);
	printf("%s\n",current_line);  /* Don't print section changes to Dfile */
	if(aflag) putasm(current_line);
	return tok_end;

    case PREVIOUS:
	{ enum Section temp; temp=section;
	section = prev_section; prev_section = temp; }
	printf("%s\n",current_line); /* Don't print section changes to Dfile */
	if(aflag) putasm(current_line);
	return tok_end;

    case SECTION: /* check this */
    	next_char=t;	
	while((*t!= '\0') && (*t != ',' ) && ~isspace(*t))
	    t++;
	savechar = *t;
	*t='\0';
	prev_section = section;
	if(strcmp(next_char, ".rodata") == 0)
	    section = CSrodata;
	else if(strcmp(next_char, ".data1") == 0)
	    section = CSdata1;
	else if(strcmp(next_char, ".data") == 0)
	     section = CSdata;
	else if(strcmp(next_char, ".text") == 0)
	    section = CStext;
	else if(strcmp(next_char, ".debug") == 0)
	    section=CSdebug; /* does this work ?? */
	else if(strcmp(next_char, ".line") == 0)
	    section=CSline;
	else if(strcmp(next_char, ".bss") == 0)
	    section=CSbss;
	else	/* unknown section */
	    section = CSother;
	*t = savechar;
	printf("%s\n",current_line); /* Don't print section changes to Dfile */
	if(aflag) putasm(current_line);
	return tok_end; /* Don't care about next_char */
	/* NOTREACHED */
	break;
    case TEXT:
	NEWSECTION(CStext);
	printf("%s\n",current_line); /* Don't print section changes to Dfile */
	if(aflag) putasm(current_line);
	return tok_end;
	/* NOTREACHED */
	break;

    /* Data for .debug section */

    default: /* pseudo-op with operand */
	tok_val=t;
	FindWhite(t);
	next_char=t; /* ready for next call */

	switch(pop) {
        case BYTE:
	    return tok_ps_byte;
        case TWOBYTE:
	    return tok_ps_2byte;
        case FOURBYTE:
	    return tok_ps_4byte;
	case STRING:	
	    return tok_ps_string;
        default:
	    return tok_dont_care;
	}
    }
/* NOTREACHED */
}

	static void
init_debug() 
{ /* Open intermediate file Dfile */

    char *filename;
    extern int errno; /* UNIX system errno */

#ifdef DEBUGDEBUG
    filename = tempnam(".","optmD"); 
#else
    filename = tempnam("/usr/tmp","optmD"); 
#endif
    if(filename == NULL)
	fatal("init_debug: couldn't make tempname.\n");

    Dfile = fopen(filename,"w+");
    if(Dfile == (FILE *)NULL)
	fatal("init_debug: couldn't open file: %s (%d).\n",filename,errno);

#ifndef DEBUGDEBUG
    if(unlink(filename) == -1)		/* Unlink: file will be removed
					   when program ends.	*/
    fatal("init_debug: couldn't unlink file: %s (%d).\n",filename,errno);
    free(filename);
#endif
}

	static void
put_Dfile(str)
/* register */ char *str;
{
	if (isspace(*str)) {
		SkipWhite(str);
		*--str='\t';
	}
	if(fprintf(Dfile,"%s\n",str)<0)
		fatal("put_Dfile: fprintf failed\n"); /* for now */
	linenumber++;
}



/* queue handling routines */

	static D_ptr
new_entry(tag_val) /* malloc space for a new entry, enter the tag */
short tag_val;
{
    D_ptr tmp_ptr;
    tmp_ptr=(D_ptr)malloc(sizeof(D_entry));
    if(tmp_ptr==NULL)
        fatal("new_queue_entry(): malloc failed\n");
    tmp_ptr->TAG=tag_val;
    return tmp_ptr;
}

	static void
put_entry(ptr)
D_ptr ptr;
{
    ptr->next=NULL;
    if(D_head==NULL) {
        D_head=ptr;
        D_tail=ptr;
    }
    else {
        D_tail->next=ptr;
        D_tail=ptr;
    }
}

	static void
set_offsets(ptr,start,end)
D_ptr ptr;
long start, end;
{
    if(ptr==NULL)
	fatal("set_offsets(): nobody home\n");
    ptr->loc_head=start;
    ptr->loc_tail=end;
}


static void mod_location();

	void
mod_debug()	/* Modify debugging info for parms and autos which
		   have been allocated to registers by the optimizer. 
		   This function is called only by raoptim() in 
		   regal.c. */
{
    D_ptr p;

    if (! init_debug_flag ) return;
    if (! current_fn ) fatal("mod_debug(): no debugging info for fn\n");
    p = current_fn->next; /* skip the header */
    while(p) { /* scan thru this fn's entries */
	switch(p->TAG){
	    case TAG_global_subroutine:
	    case TAG_subroutine:
	    case TAG_inline_subroutine:
	        fatal("mod_debug(): weird debug queue\n");
		/* We are assuming that no other entries follow
		   the current function's entries. */
		break;
	    case TAG_local_variable:
	    case TAG_formal_parameter:
		mod_location(p);
		break;
	    case TAG_label:
		break; /* for now, later check
				      this lable is still defined */
	    default: fatal("mod_debug(): unknown TAG.");
	}
	p=p->next;
    }
}

	static void
mod_location(p) 	/* Reconstitute a location entry. 
			   This code is equivalent to the
			   old radef in regal.c for coff style
			   debugging info */
D_ptr p; 		/* Points to location entry */

{
    extern ra_assigned_to();
    int r;
    int dscl;	/* Unfortunately, debugging info and storage
		   classes used by regal.c are different, so
		   we have to convert from one to the other:

		     TAG    |    mode	| dscl
	===============================================
	TAG_local_variable  | CPUReg   -> REGISTER (4)
			    | Disp     -> AUTO (1)
	TAG_formal_parameter| CPUReg   -> REGISTER (4)
			    | Disp     -> PARAM (9)	 */

    if(p->mode == CPUReg)
	dscl = REGISTER;
    else
	dscl = ( p->TAG == TAG_local_variable ) ? AUTO : PARAM;

    if( r = ra_assigned_to(dscl,p->regid,p->displacement) ) {
	p->mode = CPUReg;
	p->regid = r;
    }
}

/* Output routines */
static void print_lines();
static void skip_lines();
static void print_location();

extern FILE * outfp;

	void
print_debug() /* Called once per .s file from wrapup() in local.c. */
{
    static long linenum;	/* next line to print */
    if( (!init_debug_flag) || (D_head == NULL)) return ; /* Nothing seen */
    fflush(Dfile);
    rewind(Dfile);
#ifdef IMPL_PUSHSECTION
    fprintf(outfp,"\t.pushsection\n");
#endif
    fprintf(outfp,"\t.section\t .debug\n"); 
    while(D_head != NULL) {
        switch(D_head->TAG) {
        case TAG_global_subroutine:
        case TAG_subroutine:
        case TAG_inline_subroutine:
	    print_lines(D_head->loc_head, &linenum);
	    fprintf(outfp,"\t.2byte\t%#x\n",D_head->TAG);
	    skip_lines(D_head->loc_tail, &linenum);
	    break;
	case TAG_local_variable:
	case TAG_formal_parameter:
	    print_lines(D_head->loc_head, &linenum);
	    print_location(D_head);
	    skip_lines(D_head->loc_tail, &linenum);
	    break;
	case TAG_label:
	    /* Don't print pc_low info .. later modify this
	       to print info if available. */
	    print_lines(D_head->loc_head,&linenum);
	    skip_lines(D_head->loc_tail,&linenum);
	    break;
	}
	D_head=D_head->next;
    }
    print_lines(0,&linenum);
#ifdef IMPL_PUSHSECTION
    fprintf(outfp,"\t.popsection\n");
#else
    fprintf(outfp,"\t.previous\n");
#endif
}

	static void
print_lines(limit,line_num)

/* print all lines of Dfile in the range *line_num to limit-1,
   reset *line_num to limit, unless limit is 0, in which case
   just print all the remaining lines in the file;
*/

long limit;
long *line_num;
{
    char s[132]; /* for now */
    int i;
    if(limit == 0)
	while(fgets(s,132,Dfile)) fprintf(outfp,"%s",s);
    else {
        for(i= *line_num; i<limit; i++)
	    if(fgets(s, 132, Dfile)) {
	        fprintf(outfp,"%s",s);
	    }
	    else fatal("print_lines(): fgets() failed\n"); 
        *line_num=limit;
    }
}

	static void
skip_lines(limit,line_num)	/* same as print_lines, but no output,
				   except in debugging mode */

/* skip all lines of Dfile in the range *line_num to limit-1,
   reset *line_num to limit, unless limit is 0, in which case
   just print all the remaining lines in the file;
*/

long limit;
long *line_num;
{
    char s[132]; /* for now */
    int i;

#ifdef DEBUGDEBUG
    fprintf(outfp,"%c old debugging info (start):\n",CC);
#endif

    if(limit == 0)
	while(fgets(s,132,Dfile)) fprintf(outfp,"%s",s);
    else {
        for(i= *line_num; i<limit; i++) {
	    if(fgets(s, 132, Dfile) == NULL)
	        fatal("skip_lines(): fgets() failed\n"); 
#ifdef DEBUGDEBUG
	    fprintf(outfp,"%c%s",CC,s);
#endif
	}
        *line_num=limit;
#ifdef DEBUGDEBUG
	fprintf(outfp,"%c old debugging info (end):\n",CC);
#endif
    }
}

    static void 
print_location(dptr)
D_ptr dptr;
{
    int reg;

    /* Now print the entry */
#ifdef DEBUGDEBUG
	fprintf(outfp,"%c optimized location:\n",CC);
#endif

    reg = dptr->regid;
    if(dptr->mode == CPUReg) {
#ifdef DEBUGDEBUG
	fprintf(outfp,"\t.2byte\t5; .byte %#x; .4byte %d\t%c OP_REG\n", OP_REG, reg,CC);
#else
	fprintf(outfp,"\t.2byte\t5; .byte %#x; .4byte %d\n", OP_REG, reg);
#endif

    }
    else {
	fprintf(outfp,"\t.2byte\t11; .byte %#x; .4byte %d; .byte %#x; .4byte %#x; \
.byte %#x", OP_BASEREG, reg,OP_CONST,dptr->displacement,OP_ADD);
#ifdef DEBUGDEBUG
	fprintf(outfp,"\t%c OP_BASEREG OP_CONST OP_ADD\n",CC);
#else
	putc('\n',outfp);
#endif
    }
}

/* .line section routines */

static char *text_begin;
int init_line_flag = 0; /* Did we see any line info in the input?
			  If so, we assume the compiler emitted
			  some debug info, hence must have defined
			  a .line section, so we need one too.
			  It's global, because we check this flag
			  every time we see .previous when 
			  section == CSline in local.c yylex() */

	void
save_text_begin_label(s)		/* remember the compiler's label */
char * s;
{
	text_begin = strdup(s);
}

	void
init_line_section()

/* Defines the line number table in .section .line. */

{
    printf("..line.b:\n");
    printf("\t.4byte ..line.e-..line.b; .4byte %s\n",text_begin);
    init_line_flag = 1;
}

	void
exit_line_section()

/* Define the terminal source line info entry ( reqt 120, June 2,
   DWARF debugging info reqts. )
*/

{
    if(init_line_flag) {
#ifdef IMPL_PUSHSECTION
        fprintf(outfp,"\t.pushsection\n");
#endif
        fprintf(outfp,"\t.section\t.line\n");
        fprintf(outfp,"\t.4byte  0; .2byte 0xffff; .4byte ..text.e-%s\n",text_begin);
        fprintf(outfp,"..line.e:\n");
#ifdef IMPL_PUSHSECTION
    	fprintf(outfp,"\t.popsection\n");
#else
    	fprintf(outfp,"\t.previous\n");
#endif
    }
}

	void
print_line_info(UniqueId)
int UniqueId;
{
    /* output a source line description in the .line section */

    if(init_debug_flag) {
#ifdef IMPL_PUSHSECTION
    	printf("\t.pushsection\n");
#endif
    	printf("\t.section\t.line\n"); 
    	printf("\t.4byte %d;",UniqueId); /* source line */
    	printf(" .2byte 0xffff;"); /* position in line */
    	printf(" .4byte ..LN%d-%s\n",UniqueId,text_begin); /* address delta */
#ifdef IMPL_PUSHSECTION
    	printf("\t.popsection\n");
#else
    	printf("\t.previous\n");
#endif
    /* now output a corresponding ..LN label in .text */
    printf("..LN%d:\n",UniqueId); 

    }
}
	void
print_FS_line_info(first_src_line,fname)
int first_src_line;
char *fname;
{
    if(!init_debug_flag)
	fatal("print_FS_line_info(): no debugging info seen\n");
    printf("\t.section\t.line\n"); 
    printf("\t.4byte %ld; .2byte 0xffff; .4byte %s-%s\n",
		first_src_line,fname,text_begin);
    printf("\t.previous\n");

/* Typical entry:
	.section	.line
	.4byte 5; .2byte 0xffff; .4byte foo-..Otext.b.0
	.previous
*/

}
