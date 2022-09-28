/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mkpart:scan.c	1.1.1.2"

/*
 * Scan.c
 *
 * Scan.c contains the lexical analyzer for tokenizing the partition file.
 */

#include <stdio.h>
#include "mkpart.h"
#include "parse.h"
#include "y.tab.h"
#include <ctype.h>

/*
 * Reserved is a structure for reserved words and punctuation.  The first
 * element points at a literal rendition of the reserved word and the second
 * is the token number to be returned to the parser.
 */
struct reserved {
	char	*name;	/* token name */
	int	id;	/* token id */
} punct[] =	{
/* WARNING! THESE ENTRIES MUST BE IN ASCII COLLATING SEQUENCE! */
	{	"(",		LPAREN		},
	{	")",		RPAREN		},
	{	",",		COMMA		},
	{	"-",		DASH		},
	{	":",		COLON		},
	{	"=",		EQ		},
},

keywords[] = {
/* WARNING! THESE ENTRIES MUST BE IN ASCII COLLATING SEQUENCE! */
	{       "ALTS",         ALTS_T          },
	{	"ALTTRK",	ALTTRK_T	},
	{       "BACKUP",       BACKUP_T        },
	{       "BOOT",         BOOT_T          },
	{       "DUMP",         DUMP_T          },
	{       "HOME",         HOME_T          },
	{       "NOMOUNT",      NOMOUNT_P       },
	{       "NONUNIX",      OTHER_T         },
	{       "OTHER",        OTHER_T         },
	{       "RO",           RO_P            },
	{       "ROOT",         ROOT_T          },
	{       "STAND",        STAND_T         },
	{       "SWAP",         SWAP_T          },
	{       "USR",          USR_T           },
	{       "VALID",        VALID_P         },
	{       "VAR",          VAR_T           },
	{	"altsec",	ALTSEC		},
	{       "badsec",       BADSEC          },
	{	"badtrk",	BADTRK		},
	{	"boot",		BOOT		},
	{	"bpsec",	BPSEC		},
	{	"cyls",		CYLS		},
	{	"device",	DEVICE		},
	{	"dserial",	DSERIAL		},
	{	"heads",	HEADS		},
	{	"partition",	PARTITION	},
	{	"perm",		PERM		},
	{	"sectors",	SECTORS		},
	{	"size",		SIZE		},
	{	"start",	START		},
	{	"tag",		TAG		},
	{	"usedevice",	USEDEVICE	},
	{	"usepart",	USEPART		},
	{	"vtocsec",	VTOCSEC		},
	{	0,		0		}
};

extern FILE *	input;			/* our input stream */
symbol		*hashbuckets[NHASH];	/* for hashing into the symbol table */
static int	seen_nonwhite;		/* true for non-BLANK_LINEs */
static long	line_number = 1;	/* current input line number */
static char	line_buffer[MAXLINE];	/* current input buffer */
static char	*buffer_pos = line_buffer-1;	/* current lex position */

#define GETCHAR(x) ((x)= *++buffer_pos=getc(input))
#define UNGETCHAR(x) (--buffer_pos,ungetc((x),input))

unsigned long	number();
char		*string();
int		iskeyword();
int		iskeypunct();

/*
 * Yylex()
 * returns the token number of the next token seen.  Causes yylval to be
 * set to the actual value of a token, when such a value is needed (e.g.,
 * if yylex returns token NAME, yylval is a pointer into the symbol table
 * for the entry of this name).
 */
int
yylex()
{
int c;

	/*
	 * Loop through whitespace and comments.  Watch for EOF and
	 * BLANKLINEs (a BLANKLINE is '[ \t]*\n'; BLANKLINEs don't have
	 * comments.
	 */
	while (1) {

		if ( (GETCHAR(c)) == '#') {	/* comment to end of line */
			seen_nonwhite++;
			while ( (GETCHAR(c)) != '\n' && c != EOF ) /* null */;
		}

		if (c == EOF ) {
#ifdef DEBUG
			fprintf(stderr,"lex NULLTOKEN\n");
#endif
			return yylval.token = NULLTOKEN;
		}

		if (c == '\n') {
			line_number++;	/* keep track of input line number */
			buffer_pos = line_buffer-1; /* reset line buffer */
			if (!seen_nonwhite) {
#ifdef DEBUG
				fprintf(stderr,"lex BLANK_LINE\n");
#endif
				return yylval.token = BLANK_LINE;
			} else {
				seen_nonwhite = 0;
			}
			continue;
		}
		if (isspace(c)) {
			continue;
		}

		break;	/* getting here means we see a real character to lex */
	}

	seen_nonwhite++;
	
	if ( isalpha(c) || c == '_' ) {	/* identifier or keyword */
		char	*start;
		int	len=1;
		int	t;

		/*
		 * Gather following identifier-type characters into buffer
		 * and keep track of identifier length.
		 */
		start = buffer_pos;
		for( ;
			c != EOF && c != '\n' &&
				(isalpha(c) || isdigit(c) || c == '_');
					GETCHAR(c)) {
			len++;
		}
		UNGETCHAR(c);

			/* check for keyword */
		if ( (t = iskeyword(start,--len)) != NULLTOKEN ) {
#ifdef DEBUG
			fprintf(stderr,"lex keyword %d\n",t);
#endif
			return yylval.token = t;
		} else {
			char	*space;

			/*
			 * Otherwise it's an identifier.  Copy it into
			 * another buffer to save it for future use.
			 * Get a parse node to point at it, and make yylval
			 * point at the node.  Let the parser enter it into
			 * the symbol table.
			 */
			if ( !(space = malloc(len + 1)) ) {
				myerror("Out of name space",0);
			}
			strncpy(space, start, len);
			space[len] = '\0';
			(yylval.node = newnode(NAME))->Name = space;
#ifdef DEBUG
			fprintf(stderr,"lex NAME '%s'\n",space);
#endif
			return NAME;
		}

		/* If not an identifier type of thing, perhaps punctuation? */
	} else if ( (yylval.token = iskeypunct()) != NULLTOKEN ) {
#ifdef DEBUG
		fprintf(stderr,"lex '%c'\n",c);
#endif
		return yylval.token;

		/* Or a number? */
	} else if ( isdigit(c) ) {
			/* get number into a node; yylval points to node */
		(yylval.node = newnode(NUMBER))->Number = number();
#ifdef DEBUG
		fprintf(stderr,"lex NUMBER 0x%x\n",yylval.node->Number);
#endif
		return NUMBER;

		/* String */
	} else if ( c == '"' ) {
			/* get string into a node that yylval points to */
		(yylval.node = newnode(STRING))->String = string();
#ifdef DEBUG
		fprintf(stderr,"lex STRING \"%s\"\n",yylval.node->String);
#endif
		return STRING;
	}
	myerror("Unknown or invalid token\n",1);
}

void
myerror(s,d)
char *s;
int d;
{
	if (d) {
		*++buffer_pos = '\0';
		fprintf(stderr,"Line %ld at '%s'\n%s\n",
			line_number,line_buffer,s);
	} else {
		fprintf(stderr,"Line %ld, %s\n",
			line_number,s);
	}
	exit(10);
}

void
yyerror(s)
char *s;
{
	myerror(s,1);
}

/*
 * Number ()
 * returns the unsigned long number that we just lexed.  The first digit is
 * already in the line buffer.  Watch for octal and hex numbers as well as
 * decimal.
 */
unsigned long
number()
{
int c = *buffer_pos;
int base = 10;
unsigned long result = 0;

	/* If 1st digit was zero, determine new base */
	if (c == '0') {
		base = 8;
		if ( (GETCHAR(c)) == 'x' || c == 'X' ) {
			base = 16;
		} else {
			UNGETCHAR(c);
		}
	} else {
		result = c - '0';
	}

	/*
	 * Loop on each digit (or hex digit if base is 16) until end of number
	 * add and multiply by base.
	 */
	for (GETCHAR(c); isdigit(c) || (base == 16 && isxdigit(c)); GETCHAR(c)) {
		result *= base;
		if (isdigit(c)) {
			result += c - '0';
		} else {
			result += 10 + tolower(c) - 'a';
		}
	}
	UNGETCHAR(c);

	return result;
}

/*
 * String ()
 * returns a copy of the string lexed.  Strings do not cross newlines.
 */
char *
string()
{
char	*start;
char	*space;
int c;
int len;

	/*
	 * Line buffer has 1st ", which we don't need.  Save position of 1st
	 * "ed char in buffer, read chars until another " or error.
	 */
	GETCHAR(c);
	start = buffer_pos;
	for( ;
		c != EOF && c != '\n' && (c != '"' || (buffer_pos[-1] == '\\'));
			GETCHAR(c)) {
		/* null */;
	}

		/* check for error termination */
	if (c == EOF || c == '\n') {
		myerror("Unterminated string",1);
	}

		/* copy string to malloc'ed space and return */
	if ( !(space = malloc((len = buffer_pos - start) + 1)) ) {
		myerror("Out of string space",0);
	}
	strncpy(space, start, len);
	space[len] = '\0';
	return space;
}

/*
 * Iskeyword ( possible keyword name, length of name )
 * returns a token number.  NULLTOKEN means that the name was not, in fact,
 * a keyword.
 */
int
iskeyword (name, len)
char *name;
int len;
{
int				cmp;
register struct reserved	*p = keywords;

	/*
	 * Zero terminate string (just to be sure), and then walk through
	 * the keyword table, linear searching (yech!) for a match.  Stop
	 * when found or the name is less than the current entry.  THIS
	 * ASSUMES THAT THE KEYWORD TABLE IS MAINTAINED IN COLLATING SEQUENCE
	 * ORDER.
	 */
	name[len] = '\0';
	while ( p->name && (cmp = strcmp(name,p->name)) > 0 ) {
		p++;
	}

	if (!p->name || cmp) {
		return NULLTOKEN;
	} else {
		return p->id;
	}
}

/*
 * Iskeypunct ()
 * returns the token number of the punctuation matched or NULLTOKEN.
 * (I would've called this ispunct(), corresponding to iskeyword() above,
 * but ctype has already used that!)
 */
int
iskeypunct()
{
register struct reserved *p = punct;

	/*
	 * Linear search for puntuation match until our char is less than
	 * table entry.  THIS NOT ONLY ASSUMES COLLATING SEQUENCE, BUT THAT
	 * ALL PUNCTUATION IS ONE CHARACTER IN LENGTH.
	 */
	for ( ; p->name; p++ ) {
		if (*p->name == *buffer_pos) {
			return p->id;
		}
	}
	return NULLTOKEN;
}

/*
 * Hash ( identifier name )
 * returns a hashed value derived from the supplied name that is in the range
 * [0..NHASH-1].  Strictly an empirical guess function that seemed to do ok
 * in (very) limited trials.  It takes up to 6 characters, starting ~.25 into
 * the name, and shifts and xors them into an int.  The result is then %NHASH
 * and returned.
 */
unsigned int
hash (name)
char *name;
{
unsigned int len, i, val;

#ifdef DEBUG
	fprintf(stderr,"hash for name '%s' is",name);
#endif
	if ( (len = strlen(name)) < 2) {
		val = name[0];
	} else {
		i = (len >> 1) - (len >> 2);
		name += i;
		if ( (len -= i) > 6) {
			len = 6;
		}
		for( val = i = 0; i < len; i ++) {
			val ^= *name++ << i;
		}
	}
#ifdef DEBUG
	fprintf(stderr,"0x%x\n",val % NHASH);
#endif
	return (val % NHASH);
}

/*
 * Lookup ( identifier name )
 * returns a pointer into the symbol table for the current name.  If the name
 * was already in the table, that entry is returned, otherwise a newly added
 * symbol entry is returned.  Symbol entries have a flags field, currently
 * with 2 of the bits defined.  New entries returned here are guaranteed to
 * have flags == 0.
 *
 * The symbol table structure is an array of list headers.  The array
 * (hashbuckets[]) is indexed by hash(name), and the symbol is then found
 * by linear searching from the selected entry.  If hash() does a reasonable
 * job, the linear search should be very short.  New entries are placed at
 * the front of the hashbucket list, for convenience.
 */
symbol *
lookup (name)
char *name;
{
register unsigned int	hashidx = hash(name);
register symbol		*sym = hashbuckets[hashidx];
symbol *tmp;

#ifdef DEBUG
	fprintf(stderr,"lookup(%s)...",name);
#endif

		/* linear search for the symbol name */
	for( ; sym && strcmp(sym->name, name); sym = sym->next) {
#ifdef DEBUG
		fprintf(stderr,"not '%s'\n",sym->name);
#endif
		/* null */;
	}

	if (!sym) {	/* Not found, add a new one */
		sym = hashbuckets[hashidx];
		if ( !(hashbuckets[hashidx] = (symbol *)malloc(sizeof(symbol)) ) ) {
			myerror("Out of symbol table space\n",0);
		}
		hashbuckets[hashidx]->next = sym;
		( sym = hashbuckets[hashidx] )->flags = 0;
		sym->name = name;
		sym->ref = 0;

		/*
		 * MaxUseDepth is used during stanza processing.  It counts
		 * the number of symbols in the symbol table, but is used to
		 * detect mutually recursive stanza references.
		 */
		MaxUseDepth++;
#ifdef DEBUG
		fprintf(stderr,"not found, assigning to 0x%x\n",sym);
#endif
	}
#ifdef DEBUG
	else {
		fprintf(stderr,"found at 0x%x\n",sym);
	}
#endif

	return sym;
}


