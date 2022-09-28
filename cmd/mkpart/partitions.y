/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)mkpart:partitions.y	1.1"

#include "mkpart.h"
#include "parse.h"
#include "sys/types.h"
#include "sys/vtoc.h"
#include <stdio.h>
%}

%start file

/*
 *	Partition File Tokens
 */
%token NULLTOKEN	0
	/* basic tokens */
%token NAME		10	/* identifier: [a-zA-Z_][a-zA-Z_0-9]* */
%token NUMBER		11	/* whole number */
%token STRING		12	/* double quoted string */

	/* punctuation */
%token BLANK_LINE	20	/* \n[ \t]*\n */
%token COLON		21
%token COMMA		22
%token EQ		23	/* = */
%token LPAREN		24
%token RPAREN		25
%token DASH		26

	/* keywords */

		/* device keywords */
%token BOOT		40	/* specifies a.out filename of boot code */
%token HEADS		41	/* number of heads on physical device */
%token CYLS		42	/* number of cylinders on device */
%token SECTORS		43	/* number of sectors per track */
%token BPSEC		44	/* number of bytes per sector */
%token DSERIAL		45	/* device serial number */
%token VTOCSEC		46	/* sector number of vtoc */
%token ALTSEC		47	/* sector number of alternate block table */
%token DEVICE		48	/* unix raw device filename for entire device */
%token BADSEC		49	/* bad sector list */
%token BADTRK		50	/* bad track list */
%token USEDEVICE	51

		/* partition keywords */
%token TAG		60
%token TAGNAME		61
%token PERM		62	/* partition permission value */
%token PERMNAME		63
%token START		64	/* first sector in partition */
%token SIZE		65	/* number of sectors in partition */
%token PARTITION	66	/* unix raw device filename for this partition */
%token USEPART		67
%token ALTS_T           68      /* Partition Tag Values */
%token ALTTRK_T		69		/*  |   */
%token BACKUP_T         70              /*  |   */
%token BOOT_T           71              /*  |   */
%token OTHER_T          72              /*  |   */
%token ROOT_T           73              /*  |   */
%token SWAP_T           74              /*  |   */
%token USR_T            75              /*  V   */
%token RO_P             76      /* Partition Permission Values */
%token NOMOUNT_P        77              /*  |   */
%token VALID_P          78              /*  V   */

		/* nonterminal nodes */
%token RANGE		80	/* a range of numbers for bad block list */
%token LIST		81	/* a list of nodes */
		/* New 4.0 tag values */
%token DUMP_T		82
%token VAR_T		83
%token HOME_T		84
%token STAND_T		85



%type	< symbol >	label

%type	< stanza >	stanza stanza_body device_stanza partition_stanza

%type	< node >	NAME NUMBER STRING
%type	< node >	device_parameter part_parameter number_list nl_element
%type	< node >		range

%type	< token >	NULLTOKEN BLANK_LINE COLON COMMA EQ LPAREN RPAREN
%type	< token >		DASH BOOT HEADS CYLS SECTORS BPSEC DSERIAL
%type	< token >		VTOCSEC ALTSEC BADSEC BADTRK DEVICE USEDEVICE
%type	< token >		TAGNAME TAG PERM PERMNAME START SIZE PARTITION
%type   < token >               USEPART ALTS_T ALTTRK_T BACKUP_T BOOT_T DUMP_T 
%type	< token >		HOME_T OTHER_T ROOT_T STAND_T SWAP_T USR_T VAR_T
%type   < token >               RO_P NOMOUNT_P VALID_P
%type   < token >       tagname permname

%%
file :           blank_line partition_file NULLTOKEN
		| partition_file NULLTOKEN
		;

partition_file :  /* empty */
		| partition_file stanza
		;

blank_line :      BLANK_LINE
		| blank_line BLANK_LINE
		;

stanza_end :	  blank_line
		| NULLTOKEN /* eof */
		;

stanza :	label stanza_body stanza_end	{ $1->ref = (char *)$2;
						if ($2) {
						    $2->dev.ds_name = $1;
						}
						}
		;

label :         NAME COLON              { $$ = lookup($1->Name);
					$$->flags |= SY_DEFN;   }
		;

stanza_body :           /* empty */     { $$ = 0; }
		| device_stanza         { $$ = $1; }
		| partition_stanza      { $$ = $1; }
		;

device_stanza :   device_parameter      { $$ = newdstanza($1); }
		| device_stanza COMMA device_parameter { $$ = add2dstanza($1,$3); }
		;


partition_stanza :   part_parameter     { $$ = newpstanza($1); }
		| partition_stanza COMMA part_parameter { $$ = add2pstanza($1,$3); }
		;

device_parameter :
		  USEDEVICE EQ NAME     { ($$ = $3)->token = USEDEVICE; }
		| BOOT EQ STRING        { ($$ = $3)->token = BOOT; }
	/* Device Characteristics parameters */
		| DEVICE EQ STRING      { ($$ = $3)->token = DEVICE; }
		| HEADS EQ NUMBER       { ($$ = $3)->token = HEADS; }
		| CYLS EQ NUMBER        { ($$ = $3)->token = CYLS; }
		| SECTORS EQ NUMBER     { ($$ = $3)->token = SECTORS; }
		| BPSEC EQ NUMBER       { ($$ = $3)->token = BPSEC; }
		| DSERIAL EQ STRING     { ($$ = $3)->token = DSERIAL; }
		| VTOCSEC EQ NUMBER     { ($$ = $3)->token = VTOCSEC; }
		| ALTSEC EQ NUMBER      { ($$ = $3)->token = ALTSEC; }
		| BADSEC EQ LPAREN number_list RPAREN   { ($$ = $4)->token = BADSEC; }
		| BADTRK EQ LPAREN number_list RPAREN   { ($$ = $4)->token = BADTRK; }
		;

part_parameter :
	/* Partition Characteristics parameters */
		  USEPART EQ NAME       { ($$ = $3)->token = USEPART; }
		| PARTITION EQ NUMBER   { ($$ = $3)->token = PARTITION; }
		| TAG EQ tagname        { ($$ = newnode(TAG))->Number = $3; }
		| PERM EQ permname      { ($$ = newnode(PERM))->Number = $3; }
		| START EQ NUMBER       { ($$ = $3)->token = START; }
		| SIZE EQ NUMBER        { ($$ = $3)->token = SIZE; }
		;

tagname :         ALTS_T                { $$ = V_ALTS;   }
		| ALTTRK_T		{ $$ = V_ALTTRK; }
		| BACKUP_T              { $$ = V_BACKUP; }
		| BOOT_T                { $$ = V_BOOT;   }
		| DUMP_T                { $$ = V_DUMP;   }
		| HOME_T                { $$ = V_HOME;   }
		| OTHER_T               { $$ = V_OTHER;  }
		| ROOT_T                { $$ = V_ROOT;   }
		| SWAP_T                { $$ = V_SWAP;   }
		| STAND_T               { $$ = V_STAND;  }
		| USR_T                 { $$ = V_USR;    }
		| VAR_T                 { $$ = V_VAR;    }
		;

permname:         RO_P                  { $$ = V_RONLY;  }
		| NOMOUNT_P             { $$ = V_UNMNT;  }
		| VALID_P               { $$ = V_VALID;  }
		;

number_list :     nl_element    { $$ = $1; }
		| number_list COMMA nl_element { add_nl_elem($1,$3); $$ = $1; }
		;

nl_element :      NUMBER        { ($$ = newnode(LIST))->ListElem = $1; }
		| range         { ($$ = newnode(LIST))->ListElem = $1; }
		;

range :           NUMBER DASH NUMBER    { ($$ = newnode(RANGE))->RangeLo =
							$1->Number;
					$$->RangeHi = $3->Number;
					freenode($1); freenode($3); }
		;
%%

/*
 * Newpartstanza ()
 * returns a pointer to a new partstanza struct, cast to a stanza *.  All
 * of the fields are guaranteed to be initialized to something reasonable
 * and null.
 */
stanza *
newpartstanza ()
{
	register stanza *s;

	if ( !(s=(stanza *)malloc(sizeof(stanza))) ) {
		myerror("Out of stanza space\n",1);
	}
#ifdef DEBUG
	fprintf(stderr,"allocating new partition stanza at 0x%x\n",s);
#endif
	s->part.ps_tag = T_STANZA;
	s->part.ps_use = 0;
	s->part.ps_type = S_PART;
	s->part.ps_partno = UNDEFINED_NUMBER;
	s->part.ps_ptag = s->part.ps_perm = 0;
	s->part.ps_start = s->part.ps_size = UNDEFINED_SECTOR;
	return s;
}

/*
 * Newpstanza ( node pointer )
 * returns a pointer to a partstanza, cast to a stanza *.  It fills in the
 * field corresponding to node *n before returning;  all other fields are
 * reasonably null.
 */
stanza *
newpstanza (n)
node *n;
{
	return (add2pstanza(newpartstanza(),n));
}

/*
 * Newdevstanza ()
 * returns a pointer to a new devstanza struct, cast to a stanza *.  All
 * of the fields are guaranteed to be initialized to something reasonable
 * and null.
 */
stanza *
newdevstanza ()
{
	register stanza *s;

	if ( !(s=(stanza *)malloc(sizeof(stanza))) ) {
		myerror("Out of stanza space\n",1);
	}
#ifdef DEBUG
	fprintf(stderr,"allocating new device stanza at 0x%x, node 0x%x\n",s,n);
#endif
	s->dev.ds_tag = T_STANZA;
	s->dev.ds_type = S_DEVICE;
	s->dev.ds_use = 0;
	s->dev.ds_boot = s->dev.ds_device = s->dev.ds_dserial = NULL;
	s->dev.ds_heads = s->dev.ds_cyls = s->dev.ds_sectors = s->dev.ds_bpsec = 0;
	s->dev.ds_vtocsec = s->dev.ds_altsec = UNDEFINED_SECTOR;
	s->dev.ds_badsec = s->dev.ds_badtrk = NULL;
	return s;
}

/*
 * Newdstanza ( node pointer )
 * returns a pointer to a devstanza, cast to a stanza *.  It fills in the
 * field corresponding to node *n before returning;  all other fields are
 * reasonably null.
 */
stanza *
newdstanza (n)
node *n;
{
	return (add2dstanza(newdevstanza(),n));
}

/*
 * Add2pstanza ( stanza pointer, node pointer )
 * returns a stanza pointer.  Adds the value contained by the node to the
 * partition stanza passed in and then returns it.  The node passed in is
 * released or pointed to by the stanza, so it should not be used again
 * (directly).
 */
stanza *
add2pstanza (s,n)
stanza *s;
node *n;
{
	if (!ISPART(n->token)) {
		myerror("Unknown or inappropriate keyword\n",1);
	}

	switch (n->token) {
	case USEPART:
		(s->part.ps_use = lookup(n->Name))->flags |= SY_DECL;
		freenode(n);
		break;
	case PARTITION:
		s->part.ps_partno = n->Number;
		freenode(n);
		break;
	case TAG:
		s->part.ps_ptag |= n->Number;
		freenode(n);
		break;
	case PERM:
		s->part.ps_perm |= n->Number;
		freenode(n);
		break;
	case START:
		s->part.ps_start = n->Number;
		freenode(n);
		break;
	case SIZE:
		s->part.ps_size = n->Number;
		freenode(n);
		break;
	}
	return s;
}

/*
 * Add2dstanza ( stanza pointer, node pointer )
 * returns a stanza pointer.  Adds the value contained by the node to the
 * device stanza passed in and then returns it.  The node passed in is
 * released or pointed to by the stanza, so it should not be used again
 * (directly).
 */
stanza *
add2dstanza (s,n)
stanza *s;
node *n;
{
	if (!ISDEVICE(n->token)) {
		myerror("Unknown or inappropriate keyword\n",1);
	}

	switch (n->token) {
	case USEDEVICE:
		(s->dev.ds_use = lookup(n->Name))->flags |= SY_DECL;
		freenode(n);
		break;
	case BOOT:
		s->dev.ds_boot = n->String;
		freenode(n);
		break;
	case DEVICE:
		s->dev.ds_device = n->String;
		freenode(n);
		break;
	case HEADS:
		s->dev.ds_heads = n->Number;
		freenode(n);
		break;
	case CYLS:
		s->dev.ds_cyls = n->Number;
		freenode(n);
		break;
	case SECTORS:
		s->dev.ds_sectors = n->Number;
		freenode(n);
		break;
	case BPSEC:
		s->dev.ds_bpsec = n->Number;
		freenode(n);
		break;
	case DSERIAL:
		s->dev.ds_dserial = n->String;
		freenode(n);
		break;
	case VTOCSEC:
		s->dev.ds_vtocsec = n->Number;
		freenode(n);
		break;
	case ALTSEC:
		s->dev.ds_altsec = n->Number;
		freenode(n);
		break;
	case BADSEC:
		s->dev.ds_badsec = n;
		break;
	case BADTRK:
		s->dev.ds_badtrk = n;
		break;
	}
	return s;
}

/*
 * Overlap ( node pointer, node pointer )
 * returns boolean	true -> the range or number in one node is covered by
 *				the other node.
 *			false-> nodes are for nonoverlapping ranges.
 * The nodes are assumed to be either RANGE or NUMBER nodes.  This function
 * checks if the number or range of numbers overlaps.
 */
int
overlap(n1,n2)
node *n1, *n2;
{
	node *t;
#define ISIN(a,x) ((a)>=(x)->RangeLo && (a)<=(x)->RangeHi)

	if (n1->token == RANGE && n2->token == NUMBER) {
		t = n1; n1 = n2; n2 = t;
	}
	if (n1->token == NUMBER) {
		if (n2->token == NUMBER) {
			return n1->Number == n2->Number;
		} else {
			return ISIN(n1->Number,n2);
		}
	} else {
		return ISIN(n1->RangeLo,n2) || ISIN(n1->RangeHi,n2);
	}
}

/*
 * Add_nl_elem ( node pointer, node pointer )
 * Add the NUMBER or RANGE of numbers pointed to by the second node to the
 * LIST that the first node points to.  The range is added at the end of the
 * list.  CURRENTLY WE ONLY DETECT OVERLAPS; WE SHOULD REMOVE DUPLICATES.
 */
void
add_nl_elem (l,n)
node *l, *n;
{
	for ( ; l->ListNext; l = l->ListNext )  {
		if (overlap(n,l->ListElem)) {
			fprintf(stderr,"warning:  bad sector list has duplicates\n");
		}
	}

	l->ListNext = n;
}

/*
 * Mergeranges ( node pointer, node pointer )
 * returns a LIST node pointer to a list of NUMBERs and/or RANGEs which is
 * the catenation of the original two lists.  Actually, the second list is
 * added to the end of the first.  All of the LIST nodes in the 2nd list
 * are freed.
 */
node *
mergeranges(r,l)
node *r, *l;
{
	node *t;

	while (l) {
		add_nl_elem(r,l->ListElem);
		t = l->ListNext;
		freenode(l);
		l = t;
	}
	return r;
}


/*
 * Parse node maintenance routines.  Mkpart starts off with a static allocation
 * of parse nodes in the array freenodes[].  These are linked together by
 * initnodes() at startup and are allocated from the front as needed.  If the
 * static list should ever become exhausted, new nodes are malloc'ed.  When
 * a node is no longer needed, it can be freenode'ed, which puts the node
 * on the front of the free list.
 */

#define INITFREENODES   500
node freenodes[INITFREENODES];
node *freelist = freenodes;

/*
 * Initnodes ()
 * walks the array freenode[] and links each element to its successor.  The
 * last node has a null ListNext.
 */
void
initnodes ()
{
int i;

	for (i = 0; i < INITFREENODES; i++) {
		freenodes[i].token = LIST;
		freenodes[i].ListElem = NULL;
		freenodes[i].ListNext = &freenodes[i+1];
	}
	freenodes[INITFREENODES-1].ListNext = 0;
}

/*
 * Newnode ( node type )
 * returns a pointer to a parse node that has been initialized to values
 * appropriate for a node of the given type.
 */
node *
newnode (t)
int t;
{
	node *n;

	if (!freelist) { /* freelist exhausted */
		if ( !(n=(node *)malloc(sizeof(node))) ) {
			myerror("Out of node space\n",1);
		}
	} else {
		n = freelist;
		freelist = n->ListNext;
	}
	n->ListNext = n->ListElem = NULL;

	n->token = t;
	switch (t) {
	case NAME:
	case USEDEVICE:	case USEPART:
		n->Name = NULL;
		break;
	case NUMBER:
	case HEADS:	case CYLS:	case SECTORS:	case VTOCSEC:
	case ALTSEC:	case BPSEC:	case START:	case SIZE:
	case PARTITION:
		n->Number = UNDEFINED_NUMBER;	/* a recognizable pattern */
		break;
	case STRING:
	case BOOT:	case DSERIAL:	case DEVICE:
		n->String = NULL;
		break;
	case NULLTOKEN:
	case BLANK_LINE:	case COLON:	case COMMA:	case EQ:
	case LPAREN:	case RPAREN:	case DASH:
		myerror("Attempting to newnode() punctuation\n",1);
	case TAGNAME:
	case PERMNAME:
		myerror("Attempting to newnode() tag or perm data\n",1);
	case RANGE:
		n->RangeLo = UNDEFINED_SECTOR;
		n->RangeHi = 0;
		break;
	case LIST:
	case TAG:	case PERM:
		n->ListNext = n->ListElem = NULL;
		break;
	default:
		fprintf(stderr,"type = %d\n",t);
		myerror("Unknown node type\n",1);
	}

	return n;
}

/*
 * Freenode ( node pointer )
 * places the node onto the head of the free node list.
 */
void
freenode (n)
node *n;
{
	n->token = LIST;
	n->ListElem = NULL;
	n->ListNext = freelist;
	freelist = n;
}
