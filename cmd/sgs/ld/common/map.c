/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/map.c	1.48"
/*
 * Map file parsing and input section to output segment mapping.
 */

/************************************************************
 * Includes
 ***********************************************************/

#include	<fcntl.h>
#include	<string.h>
#include	"globals.h"
#include	<sys/stat.h>
#include	<ctype.h>

/************************************************************
 * Local macros
 ***********************************************************/

#define	SG_TEST(f, b)	((f) & (1 << (b))) /* test a bit in a segment flag */

#define	SG_SET(f, b)	((f) |= 1 << (b)) /* set a bit in a segment flag */

#define	SG_CLEAR(f, b)	((f) &= ~(1 << (b))) /* clear a bit in a segment flag */

/************************************************************
 * Local typedefs
 ***********************************************************/

typedef enum {
	TK_STRING,
	TK_COLON,
	TK_SEMICOLON,
	TK_EQUAL,
	TK_ATSIGN,
	TK_EOF
} Token;			/* Possible return values from gettoken. */

typedef enum {
	LD_PHDR,
	LD_INTERP,
	LD_TEXT,
	LD_DATA,
	LD_DYNA,
	LD_SHLIB,
	LD_NOTE,
	LD_EXTRA
} Segtype;

/************************************************************
 * Local constants
 ***********************************************************/

#define	PHDR_SEG	(&seg_descs[LD_PHDR])
#define	INTERP_SEG	(&seg_descs[LD_INTERP])
#define	TEXT_SEG	(&seg_descs[LD_TEXT])
#define	DATA_SEG	(&seg_descs[LD_DATA])
#define	DYNA_SEG	(&seg_descs[LD_DYNA])
#define	SHLIB_SEG	(&seg_descs[LD_SHLIB])
#define	NOTE_SEG	(&seg_descs[LD_NOTE])
#define	EXTRA_SEG	(&seg_descs[LD_EXTRA])
 
/************************************************************
 * Local variable definitions
 ***********************************************************/

/* Default output segment descriptors. */
static Sg_desc seg_descs[] = {
	{ { PT_PHDR, 0, 0, 0, 0, 0, PF_R + PF_X, 0}, "phdr", 0, {NULL, NULL },
		(1 << SGA_FLAGS) | (1 << SGA_TYPE), NULL },
	{ { PT_INTERP, 0, 0, 0, 0, 0, PF_R, 0 }, "interp", 0, { NULL, NULL }, 0, NULL },
	{ { PT_LOAD, 0, 0, 0, 0, 0, PF_R + PF_X, 0 }, "text", 0, { NULL, NULL },
		(1 << SGA_FLAGS) | (1 << SGA_TYPE), NULL },
	{ { PT_LOAD, 0, 0, 0, 0, 0, PF_R + PF_W + PF_X, 0 }, "data", 0, { NULL, NULL },
		(1 << SGA_FLAGS) | (1 << SGA_TYPE), NULL },
	{ { PT_DYNAMIC, 0, 0, 0, 0, 0, PF_R + PF_W + PF_X, 0 }, "dynamic", 0, { NULL, NULL }, 0, NULL },
	{ { PT_SHLIB, 0, 0, 0, 0, 0, 0, 0 }, "shlib", 0, { NULL, NULL }, 0, NULL },
	{ { PT_NOTE, 0, 0, 0, 0, 0, 0, 0 }, "note", 0, { NULL, NULL }, (1 << SGA_TYPE), NULL },
	{ { PT_NULL, 0, 0, 0, 0, 0, 0, 0 }, "", 0, { NULL, NULL }, 0, NULL }
};

static Listnode segnodes[]={
	{ (VOID*) PHDR_SEG, &segnodes[LD_PHDR+1] },
	{ (VOID*) INTERP_SEG, &segnodes[LD_INTERP+1] },
	{ (VOID*) TEXT_SEG, &segnodes[LD_TEXT+1] },
	{ (VOID*) DATA_SEG, &segnodes[LD_DATA+1] },
	{ (VOID*) DYNA_SEG, &segnodes[LD_DYNA+1] },
	{ (VOID*) SHLIB_SEG, &segnodes[LD_SHLIB+1] },
	{ (VOID*) NOTE_SEG, &segnodes[LD_NOTE+1] },
	{ (VOID*) EXTRA_SEG, NULL },
};

/* Segment descriptors. */
List seg_list = { segnodes, &segnodes[LD_EXTRA] };


/* Default entrance criteria. */
static Ent_crit stat_ent_crit[] = {
	/* Static case. */
	{ { NULL, NULL}, ".interp", SHT_PROGBITS, 0, 0, INTERP_SEG },
	{ { NULL, NULL}, NULL, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC, TEXT_SEG },
	{ { NULL, NULL}, NULL, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC + SHF_WRITE, DATA_SEG },
	{ { NULL, NULL}, NULL, SHT_NOBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC + SHF_WRITE, DATA_SEG },
	{ { NULL, NULL}, NULL, SHT_SHLIB, 0, 0, SHLIB_SEG },
	{ { NULL, NULL}, NULL, SHT_NOTE, 0, 0, NOTE_SEG },
	{ { NULL, NULL}, NULL, 0, 0, 0, EXTRA_SEG },
};

	/* Dynamic case. */
static Ent_crit dyn_ent_crit[] = {
	{ { NULL, NULL}, ".interp", SHT_PROGBITS, 0, 0, INTERP_SEG },
	{ { NULL, NULL}, NULL, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC, TEXT_SEG },
	{ { NULL, NULL}, ".symtab", SHT_SYMTAB, 0, 0, TEXT_SEG },
	{ { NULL, NULL}, ".strtab", SHT_STRTAB, 0, 0, TEXT_SEG },
	{ { NULL, NULL}, ".hash", SHT_HASH, 0, 0, TEXT_SEG },
	{ { NULL, NULL}, NULL, SHT_REL, SHF_ALLOC + SHF_WRITE, SHF_ALLOC, TEXT_SEG },
	{ { NULL, NULL}, NULL, SHT_RELA, SHF_ALLOC + SHF_WRITE, SHF_ALLOC, TEXT_SEG },
	{ { NULL, NULL}, ".dynamic", SHT_DYNAMIC, 0, 0, DATA_SEG },
	{ { NULL, NULL}, ".plt", SHT_PROGBITS, 0, 0, TEXT_SEG },
	{ { NULL, NULL}, ".got", SHT_PROGBITS, 0, 0, DATA_SEG },
	{ { NULL, NULL}, NULL, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC + SHF_WRITE, DATA_SEG },
	{ { NULL, NULL}, NULL, SHT_NOBITS, SHF_ALLOC + SHF_WRITE,
		  SHF_ALLOC + SHF_WRITE, DATA_SEG },
	{ { NULL, NULL}, NULL, SHT_SHLIB, 0, 0, SHLIB_SEG },
	{ { NULL, NULL}, NULL, SHT_NOTE, 0, 0, NOTE_SEG },
	{ { NULL, NULL}, NULL, 0, 0, 0, EXTRA_SEG }
};

static Listnode node_pool[MAX_ENTCRIT_LEN];

static List ecrit_list = {&node_pool[0],NULL};	/* Selects static or dynamic map */

static char* mapspace;		/* Pointer to malloc'ed space holding map file. */

static unsigned long line_num;	/* Current line number in the map file. */

static char* start_tok;		/* First character of current token. */

/************************************************************
 * Local function declarations
 ***********************************************************/

LPROTO(void lowercase, (char*));
LPROTO(Token gettoken, (void));
LPROTO(void sort_seg_list, (void));

/************************************************************
 * Local function definitions
 ***********************************************************/

#define	PTYPE(S)	((S)->sg_phdr.p_type) /* Idiom to access type. */

/* Sort the segment list by increasing virtual address. */
static void
sort_seg_list()
{
    List 	seg1;
    List 	seg2;
    Listnode	*np1;
    Listnode	*np2;
    Listnode	*np3;
    Sg_desc	*sgp1;
    Sg_desc	*sgp2;

    seg1.head = seg1.tail = seg2.head = seg2.tail = NULL;

    /* add the interp segment to our list */
    for (LIST_TRAVERSE(&seg_list, np1, sgp1)) {
	if (PTYPE(sgp1) == PT_INTERP)
	    (void) list_append(&seg1, sgp1);
    }

    /* add the loadable segments to another list in sorted order */
    for (LIST_TRAVERSE(&seg_list, np1, sgp1)) {
	if (PTYPE(sgp1) != PT_LOAD)
	    continue;
	if (SG_TEST(sgp1->sg_flags, SGA_VADDR) == 0)
	    (void) list_append(&seg2, sgp1);
	else {
		if (seg2.head == NULL) {
			(void) list_append(&seg2, sgp1);
			continue;
		}
	    np3 = NULL;
	    for (LIST_TRAVERSE(&seg2, np2, sgp2)) {
		if (SG_TEST(sgp2->sg_flags, SGA_VADDR) == 0) {
		    if (np3 == NULL)
			(void) list_prepend(&seg2, sgp1);
		    else
			(void) list_insert(np3, sgp1);
		    np3 = NULL;
		    break;
		}
		if (sgp1->sg_phdr.p_vaddr < sgp2->sg_phdr.p_vaddr) {
		    if (np3 == NULL)
			(void) list_prepend(&seg2, sgp1);
		    else
			(void) list_insert(np3, sgp1);
		    np3 = NULL;
		    break;
		} else if (sgp1->sg_phdr.p_vaddr >
			   sgp2->sg_phdr.p_vaddr) {
		    np3 = np2;
		} else
		    lderror(MSG_FATAL,
			    "two segments %s and %s have the same assigned virtual address",
			    sgp1->sg_name, sgp2->sg_name);
	    }
	    if (np3 != NULL)
		(void) list_append(&seg2, sgp1);
	}
    }

    /* add the sorted loadable segments to our list */
    for (LIST_TRAVERSE(&seg2, np1, sgp1)) {
	(void) list_append(&seg1, sgp1);
    }

    /* add all other segments to our list */
    for (LIST_TRAVERSE(&seg_list, np1, sgp1)) {
	if (PTYPE(sgp1) != PT_INTERP && PTYPE(sgp1) != PT_LOAD)
	    (void) list_append(&seg1, sgp1);
    }

    seg_list.head = seg_list.tail = NULL;

    /* Now rebuild the original list. */
    for (LIST_TRAVERSE(&seg1, np1, sgp1))
	(void) list_append(&seg_list, sgp1);

}



/* Convert a string to lowercase. */
static void
lowercase(str)
	char* str;	/* The string to convert. */
{
	while (*str = tolower(*str))
		str++ ;
}

/* Get a token from the mapfile. */
static Token
gettoken()
{
	static char	*nextchr = NULL;	/* Next character to examine in the mapfile. */
	static char	oldchr = '\0';		/* Character at end of current token. */
	char		*end;			/* End of the current token. */

	/* First time through. */
	if (nextchr == NULL)
		nextchr = mapspace;

	/* Cycle through the characters looking for tokens. */
	for (;;) {
		if (oldchr != '\0') {
			*nextchr = oldchr;
			oldchr = '\0';
		}
		if (!isascii(*nextchr) ||
			(!isprint(*nextchr) && !isspace(*nextchr) &&
			*nextchr != '\0'))
			lderror(MSG_FATAL,
				"mapfile: %d: illegal character '\\%03o' in mapfile",
				line_num, *((unsigned char*) nextchr));
		switch (*nextchr) {
		case '\0':	/* End of file. */
			return TK_EOF;
		case ' ':	/* White space. */
		case '\t':
			nextchr++;
			break;
		case '\n':	/* White space too, but bump line number. */
			nextchr++;
			line_num++;
			break;
		case '#':	/* Comment. */
			while (*nextchr != '\n' && *nextchr != '\0')
				nextchr++;
			break;
		case ':':	/* COLON. */
			nextchr++;
			return TK_COLON;
		case ';':	/* SEMICOLON. */
			nextchr++;
			return TK_SEMICOLON;
		case '=':	/* EQUAL. */
			nextchr++;
			return TK_EQUAL;
		case '@':	/*  ATSIGN.  */
 			nextchr++;
 			return TK_ATSIGN;
		case '"':	/* STRING. */
			start_tok = ++nextchr;
			if ((end = strpbrk(nextchr, "\"\n")) == NULL)
				lderror(MSG_FATAL,
					"mapfile: %d: string does not terminate with a quote mark",
					line_num);
			if (*end != '"')
				lderror(MSG_FATAL,
                                        "mapfile: %d: string does not terminate with a quote mark",
                                        line_num);
			*end = '\0';
			nextchr = end + 1;
			return TK_STRING;
		default:	/* STRING. */
			start_tok = nextchr;
			end = strpbrk(nextchr, " \t\n:;=#\"");
			if (end == NULL)
				nextchr = start_tok + strlen(start_tok);
			else {
				nextchr = end;
				oldchr = *nextchr;
				*nextchr = '\0';
			}
                        return TK_STRING;
		}
	}
}

/************************************************************
 * Global function definitions
 ***********************************************************/

/* Set entry criteria pointer to proper map; preserves static declarations in */
/* this file. */
/* ARGSUSED */
void 
ecrit_setup()
{
	Ent_crit	*nptr;
	int		i = 0;

	nptr = dmode ? dyn_ent_crit : stat_ent_crit;

	for (; nptr->ec_segment != EXTRA_SEG; nptr++, i++){
		if (dmode) {
			if (!Gflag) {
				/* dynsym is used; file symbol table will */
				/* land in EXTRA_SEG */
				if (nptr->ec_type == SHT_SYMTAB) {
					nptr->ec_name = ".dynsym";
					nptr->ec_type = SHT_DYNSYM;
				} else if (nptr->ec_type == SHT_STRTAB &&
					strcmp(nptr->ec_name, ".strtab") ==
					SAME)
					nptr->ec_name = ".dynstr";

				/* interp section must go into text */
				if (nptr->ec_name &&
					strcmp(nptr->ec_name, ".interp") ==
					SAME)
					nptr->ec_segment = TEXT_SEG;
			} else {
				if (nptr->ec_name &&
					strcmp(nptr->ec_name, ".dynamic") ==
					SAME)
					nptr->ec_segment = TEXT_SEG;
			}
		}

		/* establish links in ecrit_list */
		node_pool[i].data = (VOID*)nptr;
		node_pool[i].next = &node_pool[i+1];
	}
	node_pool[i].data = (VOID*)nptr;
	node_pool[i].next = NULL;
	ecrit_list.tail = &node_pool[i];

}

/* Parse the mapfile. */
void
map_parse(mapfile_name)
	char	*mapfile_name;		/* The name of the map file. */
{
	struct stat	stat_buf;	/* Need to stat(2) the mapfile. */
	int		mapfile_fd;	/* File descriptor for mapfile. */
	Sg_desc		*segment;	/* Segment descriptor being manipulated. */
	Listnode	*np1;		/* Node pointer. */
	Listnode	*np2;		/* Node pointer. */
	Sg_desc		*sgp;		/* Segment descriptor pointer. */
	Boolean		 place_seg;	/* TRUE if we have to place the current segment. */
	Ent_crit	*entcr;		/* Entrance criteria for a segment. */
	Token		tok;		/* Current token. */
	Listnode	*ecrit_next = NULL; /* Next place for entrance criterion. */

	DPRINTF(DBG_MAP, (MSG_DEBUG, "entering map_parse"));
	/* we read the entire mapfile into memory. */
	if (stat(mapfile_name, &stat_buf) == -1)
		lderror(MSG_FATAL, "cannot stat(2) mapfile \"%s\"", mapfile_name);
	mapspace = (char*) mymalloc(stat_buf.st_size + 1);
	if ((mapfile_fd = open(mapfile_name, O_RDONLY)) == -1)
		lderror(MSG_FATAL, "cannot open mapfile \"%s\"", mapfile_name);
	if (read(mapfile_fd, mapspace, stat_buf.st_size) != stat_buf.st_size)
		lderror(MSG_FATAL, "%s: read failed", mapfile_name);
	mapspace[stat_buf.st_size] = '\0';

	/* Set up the line number counter. */
	line_num = 1;
	
	/* We now parse the mapfile until the gettoken routine returns EOF. */
	while ((tok = gettoken()) != TK_EOF) {

		/* We don't know which segment yet. */
		segment = NULL;

		/* At this point we are at the beginning of a line. */
		if (tok != TK_STRING)
			lderror(MSG_FATAL,
				"%s: %d: expected a segment name at the beginning of a line",
				mapfile_name, line_num);

		/* Find the segment named in the token. */
		for (LIST_TRAVERSE(&seg_list, np1, sgp))
			if (strcmp(sgp->sg_name, start_tok) == SAME) {
				/* Found it. */
				segment = sgp;
				place_seg = FALSE;
				break;
			}

		/* If segment is still NULL our segment does not exist. */
		if (segment == NULL) {
			segment = NEWZERO(Sg_desc);
			segment->sg_phdr.p_type = PT_NULL;
			segment->sg_name = (char*) mymalloc(strlen(start_tok) + 1);
			(void) strcpy(segment->sg_name, start_tok);
			segment->sg_length = 0;
			segment->sg_osectlist.head = segment->sg_osectlist.tail
				= NULL;
			segment->sg_flags = 0;
			place_seg = TRUE;
		}

		if (strcmp(segment->sg_name, "interp") == SAME ||
		    strcmp(segment->sg_name, "dynamic") == SAME)
			lderror(MSG_FATAL,
				"%s: %d: segments 'interp' and 'dynamic' may not be changed",
				mapfile_name, line_num);

		/* Now check the second character on the line. */
		if ((tok = gettoken()) == TK_EQUAL) { /* A segment declaration. */
			Word
				temp_flags = 0,
				temp_type = segment->sg_phdr.p_type,
				temp_vaddr = segment->sg_phdr.p_vaddr,
				temp_paddr = segment->sg_phdr.p_paddr,
				temp_length = segment->sg_length,
				temp_align = segment->sg_phdr.p_align;
			unsigned long flags = 0;

			while ((tok = gettoken()) != TK_SEMICOLON) {
				if (tok != TK_STRING)
					lderror(MSG_FATAL,
						"%s: %d: expected one or more segment attributes after an '='",
						mapfile_name, line_num);

				/* Put the token into a canonical form. */
				lowercase(start_tok);
				
				if (*start_tok == '?') {
					char* p = start_tok + 1;

					if (SG_TEST(flags, SGA_FLAGS))
						lderror(MSG_FATAL,
							"%s: %d: flags set more than once on same line",
							mapfile_name, line_num);
					while (*p)
						switch (*p++) {
						case 'r':
							temp_flags |= PF_R;
							break;
						case 'w':
							temp_flags |= PF_W;
							break;
						case 'x':
							temp_flags |= PF_X;
							break;
						default:
							lderror(MSG_FATAL,
								"%s: %d: unknown segment flags '%s'",
								mapfile_name,
								line_num,
								start_tok);
						}
					SG_SET(flags, SGA_FLAGS);
				} else if (strcmp(start_tok, "load") == SAME) {
					if (SG_TEST(flags, SGA_TYPE))
						lderror(MSG_FATAL,
							"%s: %d: type set mode than once on same line",
							mapfile_name, line_num);
					temp_type = PT_LOAD;
					if (temp_type != segment->sg_phdr.p_type)
						SG_SET(flags, SGA_TYPE);
				} else if (strcmp(start_tok, "note") == SAME) {
                                        if (SG_TEST(flags, SGA_TYPE))
                                                lderror(MSG_FATAL, 
                                                        "%s: %d: type set mode than once on same line",
                                                        mapfile_name, line_num);
                                        temp_type = PT_NOTE;
					if (temp_type != segment->sg_phdr.p_type)
						SG_SET(flags, SGA_TYPE);
				} else if (*start_tok == 'l' ||
					   *start_tok == 'v' ||
					   *start_tok == 'a' ||
					   *start_tok == 'p') {
					char* p;
					long l;
					
#ifdef					__STDC__
					l = strtoul(&start_tok[1], &p, 0);
#else					/* __STDC__ */
					l = strtol(&start_tok[1], &p, 0);
#endif					/* __STDC__ */
					if (p == &start_tok[1])
						lderror(MSG_FATAL,
							"%s: %d: badly formed number in segment address or length '%s'",
							mapfile_name, line_num, start_tok);
					switch (*start_tok) {
					case 'l':
						if (SG_TEST(flags, SGA_LENGTH))
							lderror(MSG_FATAL,
								"%s: %d: length set more than once one same line",
								mapfile_name,
								line_num);
						temp_length = l;
						SG_SET(flags, SGA_LENGTH);
						break;
					case 'v':
						if (SG_TEST(flags, SGA_VADDR))
							lderror(MSG_FATAL,
								"%s: %d: virtual address set more than once one same line",
								mapfile_name,
								line_num);
						temp_vaddr = l;
						SG_SET(flags, SGA_VADDR);
						break;
					case 'p':
						if (SG_TEST(flags, SGA_PADDR))
							lderror(MSG_FATAL,
								"%s: %d: physical address set more than once one same line",
								mapfile_name,
								line_num);
						temp_paddr = l;
						SG_SET(flags, SGA_PADDR);
						break;
					case 'a':
						if (SG_TEST(flags, SGA_ALIGN))
							lderror(MSG_FATAL,
								"%s: %d: alignment set more than once one same line",
								mapfile_name,
								line_num);
						temp_align = l;
						SG_SET(flags, SGA_ALIGN);
						break;
					}
				} else
					lderror(MSG_FATAL,
						"%s: %d: unknown segment attribute '%s'",
						mapfile_name, line_num, start_tok);
			}
			if ((segment->sg_flags & (flags & (1 << SGA_FLAGS))) &&
			    (temp_flags != segment->sg_phdr.p_flags))
				lderror(MSG_WARNING,
					"%s: %d: changing existing attributes",
					mapfile_name, line_num);
			else if (segment->sg_flags & (flags & ~(1 << SGA_FLAGS)))
				lderror(MSG_WARNING,
					"%s: %d: changing existing attributes",
					mapfile_name, line_num);
			else if (place_seg) { /* New segment */
				if (SG_TEST(flags, SGA_TYPE) == 0) { /* default */
								     /* type */
					temp_type = PT_LOAD;
					SG_SET(flags, SGA_TYPE);
				}
				if (SG_TEST(flags, SGA_FLAGS) == 0 &&
				    temp_type == PT_LOAD) {
					temp_flags = PF_R | PF_W | PF_X;
					SG_SET(flags, SGA_FLAGS);
				}
			}
			if (temp_type != PT_LOAD) {
				if (SG_TEST(flags, SGA_FLAGS)) {
					lderror(MSG_WARNING,
						"%s: %d: flags not allowed on non-LOAD segments",
						mapfile_name, line_num);
					SG_CLEAR(flags, SGA_FLAGS);
					SG_CLEAR(segment->sg_flags, SGA_FLAGS);
				}
				if (SG_TEST(flags, SGA_VADDR)) {
					lderror(MSG_WARNING,
						"%s: %d: virtual address not allowed on non-LOAD segments",
						mapfile_name, line_num);
					SG_CLEAR(flags, SGA_VADDR);
					SG_CLEAR(segment->sg_flags, SGA_VADDR);
				}
				if (SG_TEST(flags, SGA_PADDR)) {
					lderror(MSG_WARNING,
						"%s: %d: physical address not allowed on non-LOAD segments",
						mapfile_name, line_num);
					SG_CLEAR(flags, SGA_PADDR);
					SG_CLEAR(segment->sg_flags, SGA_PADDR);
				}
				if (SG_TEST(flags, SGA_LENGTH)) {
					lderror(MSG_WARNING,
						"%s: %d: length not allowed on non-LOAD segments",
						mapfile_name, line_num);
					SG_CLEAR(flags, SGA_LENGTH);
					SG_CLEAR(segment->sg_flags, SGA_LENGTH);
				}
				if (SG_TEST(flags, SGA_ALIGN)) {
					lderror(MSG_WARNING,
						"%s: %d: alignment not allowed on non-LOAD segments",
						mapfile_name, line_num);
					SG_CLEAR(flags, SGA_ALIGN);
					SG_CLEAR(segment->sg_flags, SGA_ALIGN);
				}
			}
			if (SG_TEST(flags, SGA_TYPE))
				segment->sg_phdr.p_type = temp_type;
			if (SG_TEST(flags, SGA_FLAGS))
				segment->sg_phdr.p_flags = temp_flags;
			if (SG_TEST(flags, SGA_VADDR))
				segment->sg_phdr.p_vaddr = temp_vaddr;
			if (SG_TEST(flags, SGA_PADDR))
				segment->sg_phdr.p_paddr = temp_paddr;
			if (SG_TEST(flags, SGA_LENGTH))
				segment->sg_length = temp_length;
			if (SG_TEST(flags, SGA_ALIGN))
				segment->sg_phdr.p_align = temp_align;
			segment->sg_flags |=  flags;
		} else if (tok == TK_COLON) {
			Boolean name_seen = FALSE, type_seen = FALSE,
				attr_seen = FALSE;
			
			/* If we get this far and the segment is new (place_seg */
			/* == TRUE) then we need to set the segment fields to */
			/* default values. */
			if (place_seg) {
				segment->sg_phdr.p_type = PT_LOAD;
				segment->sg_phdr.p_flags = PF_R + PF_W + PF_X;
				segment->sg_flags |= (1 << SGA_TYPE) |
					(1 << SGA_FLAGS) | (1 << SGA_LENGTH) |
					(1 << SGA_ALIGN);
			}

			/* We are looking at a new entrance criteria line. */
			entcr = NEWZERO(Ent_crit);
			entcr->ec_segment = segment;
			if (ecrit_next == NULL)
			    ecrit_next = list_prepend(&ecrit_list, entcr);
			else
			    ecrit_next = list_insert(ecrit_next, entcr);
			
			while ((tok = gettoken()) != TK_COLON &&
			       tok != TK_SEMICOLON) {
				if (tok == TK_EOF)
					lderror(MSG_FATAL,
						"%s: %d: premature EOF",
						mapfile_name, line_num);
				if (*start_tok == '$') {
					if (type_seen)
						lderror(MSG_FATAL,
							"%s: %d: more than one section type specified",
							mapfile_name, line_num);
					type_seen = TRUE;
					start_tok++;
					lowercase(start_tok);
					if (strcmp(start_tok, "progbits") == SAME)
						entcr->ec_type = SHT_PROGBITS;
					else if (strcmp(start_tok, "symtab") == SAME)
						entcr->ec_type = SHT_SYMTAB;
					else if (strcmp(start_tok, "dynsym") == SAME)
						entcr->ec_type = SHT_DYNSYM;
					else if (strcmp(start_tok, "strtab") == SAME)
						entcr->ec_type = SHT_STRTAB;
					else if (strcmp(start_tok, "rela") == SAME)
						entcr->ec_type = SHT_RELA;
					else if (strcmp(start_tok, "rel") == SAME)
						entcr->ec_type = SHT_REL;
					else if (strcmp(start_tok, "hash") == SAME)
						entcr->ec_type = SHT_HASH;
					else if (strcmp(start_tok, "lib") == SAME)
						entcr->ec_type = SHT_SHLIB;
					else if (strcmp(start_tok, "dynamic") == SAME)
						entcr->ec_type = SHT_DYNAMIC;
					else if (strcmp(start_tok, "note") == SAME)
						entcr->ec_type = SHT_NOTE;
					else if (strcmp(start_tok, "nobits") == SAME)
						entcr->ec_type = SHT_NOBITS;
					else
						lderror(MSG_FATAL,
							"%s: %d: unknown section type '%s'",
							mapfile_name, line_num, start_tok);
				} else if (*start_tok == '?') {
					Boolean bang_seen;

					if (attr_seen)
						lderror(MSG_FATAL,
                                                        "%s: %d: more than one section attribute group specified",
                                                        mapfile_name, line_num);
                                        attr_seen = TRUE;
					start_tok++;
					lowercase(start_tok);
					bang_seen = FALSE;
					for ( ; *start_tok != '\0'; start_tok++)
						switch (*start_tok) {
						case '!':
							if (bang_seen)
								lderror(MSG_FATAL,
									"%s: %d: badly formed attribute group '%s'",
									mapfile_name, line_num,
									start_tok);
							bang_seen = TRUE;
							break;
						case 'a':
							if (entcr->ec_attrmask &
							    SHF_ALLOC)
								lderror(MSG_FATAL,
									"%s: %d: badly formed attribute group '%s'",
                                                                        mapfile_name, line_num,
									start_tok);
							entcr->ec_attrmask |= SHF_ALLOC;
							if (!bang_seen)
								entcr->ec_attrbits |= SHF_ALLOC;
							bang_seen = FALSE;
							break;
						case 'w':
							if (entcr->ec_attrmask &
							    SHF_WRITE)
								lderror(MSG_FATAL,
									"%s: %d: badly formed attribute group '%s'",
                                                                        mapfile_name, line_num,
									start_tok);
							entcr->ec_attrmask |= SHF_WRITE;
							if (!bang_seen)
								entcr->ec_attrbits |= SHF_WRITE;
							bang_seen = FALSE;
							break;
						case 'x':
							if (entcr->ec_attrmask &
							    SHF_EXECINSTR)
								lderror(MSG_FATAL,
									"%s: %d: badly formed attribute group '%s'",
                                                                        mapfile_name, line_num,
									start_tok);
							entcr->ec_attrmask |= SHF_EXECINSTR;
							if (!bang_seen)
								entcr->ec_attrbits |= SHF_EXECINSTR;
							bang_seen = FALSE;
							break;
						default:
							lderror(MSG_FATAL,
								"%s: %d: badly formed attribute group '%s'",
                                                                        mapfile_name, line_num,
                                                                        start_tok);
							break;
						}
				} else {
					if (name_seen)
						lderror(MSG_FATAL,
							"%s: %d: more than one section name specified",
							mapfile_name, line_num);
					name_seen = TRUE;
					entcr->ec_name = (char*)
						mymalloc(strlen(start_tok) + 1);
					(void) strcpy(entcr->ec_name, start_tok);
				}
			}
			if (tok == TK_COLON)
				while ((tok = gettoken()) != TK_SEMICOLON) {
					char* file;

					if (tok == TK_EOF)
						lderror(MSG_FATAL,
							"%s: %d: premature EOF",
							mapfile_name, line_num);
					file = (char*)
						mymalloc(strlen(start_tok) + 1);
					(void) strcpy(file, start_tok);
					(void) list_append(&(entcr->ec_files), file);
				}
		} else if (tok == TK_ATSIGN) {
			if ((tok = gettoken()) == TK_STRING) {
				Sym* nsym;
				Ldsym* lsym;
				char* name = (char*) mymalloc(strlen(start_tok) + 1);
				(void) strcpy(name, start_tok);
				if (segment->sg_sizesym != NULL)
					lderror(MSG_FATAL,
						"%s: segment %s already has a size symbol",
						mapfile_name, segment->sg_name);
				if ((lsym = sym_find(name,NOHASH)) != NULL)
					lderror(MSG_FATAL,
						"%s: symbol %s already defined",
						mapfile_name, name);
				nsym = NEWZERO(Sym);
				nsym->st_shndx = SHN_ABS;
				nsym->st_size = 0;
				nsym->st_info = ELF_ST_INFO(STB_GLOBAL, STT_OBJECT);
				lsym = sym_enter(nsym, name, NOHASH);
				lsym->ls_deftag = REF_RELOBJ;
				segment->sg_sizesym = lsym;
				count_outglobs++;
				count_strsize += strlen(name) + 1;
			} else
				lderror(MSG_FATAL,
					"%s: expected a symbol name after '$'",
					mapfile_name);
			if ((tok = gettoken()) != TK_SEMICOLON) {
				lderror(MSG_FATAL,
					"%s: %d: expected a SEMICOLON",
					mapfile_name, line_num);
			}
		} else
			lderror(MSG_FATAL,
				"%s: %d: expected a '=', ':', or '$'", mapfile_name, line_num);
		if (segment->sg_phdr.p_type == PT_NULL) {
			segment->sg_phdr.p_type = PT_LOAD;
			segment->sg_phdr.p_flags = PF_R + PF_W + PF_X;
		}
		if (place_seg) {
			int src_type, dst_type;

			DPRINTF(DBG_MAP, (MSG_DEBUG, "placing segment %s",
					  segment->sg_name));
			switch (segment->sg_phdr.p_type) {
			case PT_PHDR:
				src_type = 0;
				break;
			case PT_LOAD:
				src_type = 2;
				break;
			case PT_SHLIB:
				src_type = 4;
				break;
			case PT_NOTE:
				src_type = 5;
				break;
			default:
				lderror(MSG_SYSTEM,
					"%s: %d: internal segment type %d",
					mapfile_name, line_num, segment->sg_phdr.p_type);
			}
			np2 = NULL;
			for (LIST_TRAVERSE(&seg_list, np1, sgp)) {
				switch (sgp->sg_phdr.p_type) {
				case PT_PHDR:
					dst_type = 0;
					break;
				case PT_INTERP:
					dst_type = 1;
					break;
				case PT_LOAD:
					dst_type = 2;
					break;
				case PT_DYNAMIC:
					dst_type = 3;
					break;
				case PT_SHLIB:
					dst_type = 4;
					break;
				case PT_NOTE:
					dst_type = 5;
					break;
				case PT_NULL:
					dst_type = 6;
					break;
				default:
					lderror(MSG_SYSTEM,
						"%s: %d: internal segment type %d",
						mapfile_name, line_num, sgp->sg_phdr.p_type);
				}
				if (src_type <= dst_type) {
					if (dst_type == 1)
						lderror(MSG_FATAL,
							"%s: %d: only one INTERP segment allowed",
							mapfile_name, line_num);
					if (np2 == NULL) {
						DPRINTF(DBG_MAP, (MSG_DEBUG,
								  "placing first"));
						(void) list_prepend(&seg_list,
								    segment);
					 } else {
						DPRINTF(DBG_MAP, (MSG_DEBUG,
								  "placing after %s",
								  ((Sg_desc*)
								  (np2->data))->sg_name));
						(void) list_insert(np2, segment);
					}
					break;
				}
				np2 = np1;
			}
		}
	}
	sort_seg_list();
}

/* Place a section into the mapfile structure. */
void 
place_section(isect)
	Insect	*isect;		/* The section to place. */
{
	Listnode	*np1;		/* Node pointer. */
	Listnode	*np2;		/* Node pointer. */
	Ent_crit	*entcr;		/* Entrance criteria node pointer. */
	Sg_desc		*sgp;		/* Segment descriptor pointer. */
	Os_desc		*osp;		/* Output section descriptor pointer. */
	char		*file;		/* File names. */
	Os_desc		*osect;		/* New output section descriptor. */
	int		src_type;	/* Sorting type of the input section. */
	int		dst_type;	/* Sorting type of an existing section. */

	/* Traverse the entrance criteria list searching for a segment that */
	/* matches the input section we have.  If an entrance criterion is set */
	/* then there must be an exact match. If we complete the loop without */
	/* finding a segment, then sgp will be NULL. */

	DPRINTF(DBG_MAP,(MSG_DEBUG,"place_section: isect: %s, size = %#x",
	isect->is_name, isect->is_shdr->sh_size));

	sgp = NULL;
	for (LIST_TRAVERSE(&ecrit_list, np1, entcr)) {
		if (entcr->ec_name != NULL &&
		    strcmp(entcr->ec_name, isect->is_name) != SAME)
			continue;
		if (entcr->ec_type != SHT_NULL &&
		    entcr->ec_type != isect->is_shdr->sh_type)
			continue;
		if (entcr->ec_attrmask != 0 &&
		    (entcr->ec_attrmask & entcr->ec_attrbits) !=
		    (entcr->ec_attrmask & isect->is_shdr->sh_flags))
			continue;
		if (entcr->ec_files.head != NULL) {
			int ffound = 0;
			if (isect->is_file_ptr != NULL)
				for (LIST_TRAVERSE(&(entcr->ec_files), np2, file)) {
					if (file[0] == '*') {
						OLD_C(extern char* strrchr();)
						CONST char* basename;
						if ((basename =
						     strrchr(isect->is_file_ptr->
							     fl_name, '/')) == NULL)
							basename = isect->is_file_ptr->
								fl_name;
						else if (basename[1] != '\0')
							basename++;
						if (strcmp(&file[1], basename) == SAME)
							ffound = 1;
					} else {
						if (strcmp(file, isect->is_file_ptr->
							   fl_name) == SAME)
							ffound = 1;
					}
				}
			if (!ffound)
				goto contin;
		}
		sgp = entcr->ec_segment;
		break;
contin:		;
	}
	if (sgp == NULL)
		sgp = ((Ent_crit*) (ecrit_list.tail->data))->ec_segment;

	if (sgp->sg_phdr.p_type == PT_LOAD)
		isect->is_shdr->sh_flags |= SHF_ALLOC;
	if (sgp->sg_phdr.p_type != PT_LOAD)
		isect->is_shdr->sh_flags = 0;
	switch (isect->is_shdr->sh_type) {
	case SHT_DYNAMIC:
		src_type = 11;
		break;
	case SHT_PROGBITS:
if (strcmp(isect->is_name, ".init") == SAME)
	src_type = 7;
else if (strcmp(isect->is_name, ".got") == SAME)
	src_type = 10;
else if (strcmp(isect->is_name, ".plt") == SAME)
	src_type = 8;
else if (strcmp(isect->is_name, ".interp") == SAME)
	src_type = 0;
else
	src_type = 9;
		break;
	case SHT_SYMTAB:
	case SHT_DYNSYM:
		src_type = 2;
		break;
	case SHT_STRTAB:
		if(strcmp(isect->is_name, ".strtab") == SAME)
			src_type = 3;
		else
			src_type = 4;
		break;
	case SHT_RELA:
		src_type = 5;
		break;
	case SHT_REL:
		src_type = 6;
		break;
	case SHT_HASH:
		src_type = 1;
		break;
	case SHT_SHLIB:
		src_type = 12;
		break;
	case SHT_NOTE:
		src_type = 13;
		break;
	case SHT_NOBITS:
		src_type = 14;
		break;
	default:
		if ((isect->is_shdr->sh_type >= SHT_LOUSER) &&
		    (isect->is_shdr->sh_type <= SHT_HIUSER))
			src_type = 1;
		else
			lderror(MSG_FATAL,
				"illegal section type for map file structure");
	}

	np2 = NULL;
	for (LIST_TRAVERSE(&(sgp->sg_osectlist), np1, osp)) {
		if (strcmp(isect->is_name, osp->os_name) == SAME &&
		    isect->is_shdr->sh_type == osp->os_shdr->sh_type &&
		    isect->is_shdr->sh_flags == osp->os_shdr->sh_flags) {
			(void) list_append(&(osp->os_insects), isect);
			isect->is_outsect_ptr = osp;
			DPRINTF(DBG_MAP, (MSG_DEBUG, "\tplaced in section %s ", osp->os_name));
			return;
		}

		switch (osp->os_shdr->sh_type) {
		case SHT_DYNAMIC:
			dst_type = 11;
			break;
		case SHT_PROGBITS:
if (strcmp(osp->os_name, ".init") == SAME)
	dst_type = 7;
else if (strcmp(osp->os_name, ".got") == SAME)
	dst_type = 10;
else if (strcmp(osp->os_name, ".plt") == SAME)
	dst_type = 8;
else if (strcmp(osp->os_name, ".interp") == SAME)
	dst_type = 0;
else
	dst_type = 9;
			break;
		case SHT_SYMTAB:
		case SHT_DYNSYM:
			dst_type = 2;
			break;
		case SHT_STRTAB:
			if (strcmp(osp->os_name, ".strtab") == SAME)
				dst_type = 3;
			else
				dst_type = 4;
			break;
		case SHT_RELA:
			dst_type = 5;
			break;
		case SHT_REL:
			dst_type = 6;
			break;
		case SHT_HASH:
			dst_type = 1;
			break;
		case SHT_SHLIB:
			dst_type = 12;
			break;
		case SHT_NOTE:
			dst_type = 13;
			break;
		case SHT_NOBITS:
			dst_type = 14;
			break;
		default:
			if ((osp->os_shdr->sh_type >= SHT_LOUSER) &&
			    (osp->os_shdr->sh_type <= SHT_HIUSER))
				dst_type = 1;
			else
				lderror(MSG_SYSTEM,
					"illegal section type for map file structure");
		}

		if (src_type < dst_type)
			break;

		np2 = np1;
	}

	count_osect++;
	count_outlocs++;
	osect = NEWZERO(Os_desc);
	osect->os_shdr = NEWZERO(Shdr);
	osect->os_shdr->sh_type = isect->is_shdr->sh_type;
	osect->os_shdr->sh_flags = isect->is_shdr->sh_flags;
	osect->os_shdr->sh_entsize = isect->is_shdr->sh_entsize;
	osect->os_name = isect->is_name;
	count_namelen += strlen(isect->is_name)+1;
	(void) list_append(&(osect->os_insects), isect);
	DPRINTF(DBG_MAP, (MSG_DEBUG, "\tplaced in section %s ", osect->os_name));
	isect->is_outsect_ptr = osect;

	if (np2 == NULL)
		(void) list_prepend(&(sgp->sg_osectlist), osect);
	else
		(void) list_insert(np2, osect);
}


#ifdef	DEBUG
void
mapprint(name)
	char	*name;
{
	Listnode	*np1;
	Listnode	*np2;
	Listnode	*np3;
	Ent_crit	*ecp;
	char		*cp;
	Sg_desc		*sgp;
	Phdr		phdr;
	Os_desc		*osp;
	Shdr		*shdr;
	Insect		*isp;

	lderror(MSG_DEBUG, "\n\n***Entrance criteria List");
	if (name == NULL) {
	    for (LIST_TRAVERSE(&ecrit_list, np1, ecp)) {
		lderror(MSG_DEBUG, " %#x(%u):%#x(%u)", np1, np1, ecp, ecp);
		lderror(MSG_DEBUG, "  Files:");
		for (LIST_TRAVERSE(&(ecp->ec_files), np2, cp))
		    lderror(MSG_DEBUG, "   %#x(%u):\"%s\"", np2, np2, cp);
		lderror(MSG_DEBUG, "  Name: \"%s\"", ecp->ec_name);
		lderror(MSG_DEBUG, "  Type: %d", ecp->ec_type);
		lderror(MSG_DEBUG, "  Attrmask: %d, bits: %d", ecp->ec_attrmask,
			ecp->ec_attrbits);
		lderror(MSG_DEBUG, "  Segment (%s): %#x(%u)",
			ecp->ec_segment->sg_name, ecp->ec_segment, ecp->ec_segment);
	    }
	}
	lderror(MSG_DEBUG, "***Segment List");
	for (LIST_TRAVERSE(&seg_list, np1, sgp)) {
		lderror(MSG_DEBUG, " %#x(%u):%#x(%u)", np1, np1, sgp, sgp);
		phdr = sgp->sg_phdr;
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_type: %d", phdr.p_type);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_offset: %d", phdr.p_offset);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_vaddr: %#x(%u)",
			phdr.p_vaddr, phdr.p_vaddr);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_paddr: %#x(%u)",
			phdr.p_paddr, phdr.p_paddr);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_filesz: %#x(%u)",
			phdr.p_filesz, phdr.p_filesz);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_memsz: %#x(%u)",
			phdr.p_memsz, phdr.p_memsz);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_flags: %d", phdr.p_flags);
		lderror(MSG_DEBUG, "  ...->sg_phdr.p_align: %d", phdr.p_align);
		lderror(MSG_DEBUG, "  ...->sg_name: \"%s\"", sgp->sg_name);
		lderror(MSG_DEBUG, "  ...->sg_length: %#x(%u)",
			sgp->sg_length, sgp->sg_length);
		lderror(MSG_DEBUG, "  ...->sg_flags: %d", sgp->sg_flags);
		lderror(MSG_DEBUG, "  ...->sg_osectlist:");
		for (LIST_TRAVERSE(&(sgp->sg_osectlist), np2, osp)) {
			if (name != NULL && strcmp(name, osp->os_name) != SAME)
				continue;
			lderror(MSG_DEBUG, "    %#x(%u):%#x(%u)", np2, np2,
				osp, osp);
			shdr = osp->os_shdr;
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_name: %d",
				shdr->sh_name);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_type: %d",
				shdr->sh_type);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_flags: %d",
				shdr->sh_flags);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_addr: %#x(%u)",
				shdr->sh_addr, shdr->sh_addr);
			lderror(MSG_DEBUG,
				"     ...->os_shdr->sh_offset: %#x(%u)",
				shdr->sh_offset, shdr->sh_offset);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_size: %#x(%u)",
				shdr->sh_size, shdr->sh_size);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_link: %d",
				shdr->sh_link);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_info: %d",
				shdr->sh_info);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_addralign: %d",
				shdr->sh_addralign);
			lderror(MSG_DEBUG, "     ...->os_shdr->sh_entsize: %d",
				shdr->sh_entsize);
			lderror(MSG_DEBUG, "     ...->os_scn: %#x(%u)",
				osp->os_scn, osp->os_scn);
			lderror(MSG_DEBUG, "     ...->os_name: \"%s\"",
				osp->os_name);
			lderror(MSG_DEBUG, "     ...->os_szinrel: %#x(%u)",
				osp->os_szinrel, osp->os_szinrel);
			lderror(MSG_DEBUG, "     ...->os_szoutrel: %#x(%u)",
				osp->os_szoutrel, osp->os_szoutrel);
			lderror(MSG_DEBUG, "     ...->os_insects:");
			for (LIST_TRAVERSE(&(osp->os_insects), np3, isp)) {
				lderror(MSG_DEBUG, "      %#x(%u):%#x(%u)",
					np3, np3, isp, isp);
				lderror(MSG_DEBUG, "       ...->is_scn: %#x(%u)",
					isp->is_scn, isp->is_scn);
				lderror(MSG_DEBUG, "       ...->is_name: \"%s\"",
					isp->is_name);
				shdr = isp->is_shdr;
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_name: %d",
					shdr->sh_name);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_type: %d",
					shdr->sh_type);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_flags: %d",
					shdr->sh_flags);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_addr: %#x(%u)",
					shdr->sh_addr, shdr->sh_addr);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_offset: %#x(%u)",
					shdr->sh_offset, shdr->sh_offset);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_size: %#x(%u)",
					shdr->sh_size, shdr->sh_size);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_link: %d",
					shdr->sh_link);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_info: %d",
					shdr->sh_info);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_addralign: %d",
					shdr->sh_addralign);
				lderror(MSG_DEBUG,
					"       ...->is_shdr->sh_entsize: %d",
					shdr->sh_entsize);
				lderror(MSG_DEBUG, "       ...->is_index: %d",
					isp->is_index);
				lderror(MSG_DEBUG,
					"       ...->is_file_ptr: %#x(%u)",
					isp->is_file_ptr, isp->is_file_ptr);
				lderror(MSG_DEBUG,
					"       ...->is_outsect_ptr: %#x(%u)",
					isp->is_outsect_ptr, isp->is_outsect_ptr);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_buf: %#x(%u)",
					isp->is_rawbits->d_buf,
					isp->is_rawbits->d_buf);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_type: %d",
					isp->is_rawbits->d_type);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_size: %#x(%u)",
					isp->is_rawbits->d_size,
					isp->is_rawbits->d_size);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_off: %#x(%u)",
					isp->is_rawbits->d_off,
					isp->is_rawbits->d_off);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_align: %#x(%u)",
					isp->is_rawbits->d_align,
					isp->is_rawbits->d_align);
				lderror(MSG_DEBUG,
					"       ...->is_rawbits->d_version: %d",
					isp->is_rawbits->d_version);
				lderror(MSG_DEBUG,
					"       ...->is_rela_list: NOT PRINTED");
				lderror(MSG_DEBUG,
					"       ...->is_displ: %#x(%u)",
					isp->is_displ, isp->is_displ);
				lderror(MSG_DEBUG,
					"       ...->is_newFOffset: %#x(%u)",
					isp->is_newFOffset, isp->is_newFOffset);
				lderror(MSG_DEBUG,
					"       ...->is_newVAddr: %#x(%u)",
					isp->is_newVAddr, isp->is_newVAddr);
			}
			lderror(MSG_DEBUG, "     ...->os_outrels: NOT PRINTED");
			lderror(MSG_DEBUG, "     ...->os_sectsym: NOT PRINTED");
		}
	}
}

#endif


/* print a virtual address map of input and output sections */
void 
ldmap_out()
{
	
	Listnode *lptr1, *lptr2, *lptr3;
	register Os_desc *osect; 
	register Sg_desc *seg;
	register Insect *isect;

	/* print headers */
	printf("\t\tLINK EDITOR MEMORY MAP\n\n");
	if(rflag){
		printf("\noutput\t\tinput\t\tnew");
		printf("\nsection\t\tsection\t\tdisplacement\tsize\n");
	} else {
		printf("\noutput\t\tinput\t\tvirtual");
		printf("\nsection\t\tsection\t\taddress\t\tsize\n");
	}

	for (LIST_TRAVERSE(&seg_list, lptr1, seg)) {
		if (seg->sg_phdr.p_type != PT_LOAD)
			continue;
		for (LIST_TRAVERSE(&(seg->sg_osectlist),lptr2, osect)) {
			printf("%-8.8s\t\t\t%08.2lx\t%08.2lx\n",
				osect->os_name, osect->os_shdr->sh_addr,
					osect->os_shdr->sh_size);
			for (LIST_TRAVERSE(&(osect->os_insects),lptr3,isect)) {
				printf( "\t\t%-8.8s\t%08.2lx\t%08.2lx %s\n",isect->is_name,(rflag) ? isect->is_displ : isect->is_newVAddr,isect->is_shdr->sh_size,((isect->is_file_ptr != NULL) ? (char *)(isect->is_file_ptr->fl_name) : "(null)"));
			}
		}
	}
}
